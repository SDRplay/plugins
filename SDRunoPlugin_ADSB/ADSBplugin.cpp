#include <sstream>
#include <vector>
#include <sstream>
#include <chrono>
#include <cstdlib>

#include <unoevent.h>
#include <iunoplugincontroller.h>

#include "ADSBplugin.h"
#include "ADSBprint.h"
#include "dump1090.h"

#include "Shlobj.h"
#include "shlwapi.h"
#pragma comment(lib,"shlwapi.lib")

struct modes Modes;

ADSBplugin::ADSBplugin(IUnoPluginController& controller) :
	IUnoPlugin(controller),
	m_form(*this, controller),
	m_worker(nullptr),
	m_started(false),
	m_stream_started(false),
	m_data_ready(false),
	m_dropping(0),
	m_hold_off(0),
	m_next_stats_display(0),
	m_next_stats_update(0),
	m_next_json(0),
	m_next_history(0),
	m_next_update(0),
	m_gain_control(true),
	m_request_gr(0),
	m_update_gr(0),
	m_z1_I(0.0f),
	m_z1_Q(0.0f),
	m_dc_a(0.0f),
	m_dc_b(0.0f)
{
#if defined(DEBUGCONSOLE)
	ADSBprint::open();	
#endif

	DEBUG_PRINT("ADSBplugin++\n");

	// Init defaults
	m_gui_config.oversample = 0;
	m_gui_config.net_http_ports = "8080";
	m_gui_config.net_output_raw_ports = "30002";
	m_gui_config.net_output_sbs_ports = "30003";
	m_gui_config.net_output_beast_ports = "30005";
}

ADSBplugin::~ADSBplugin()
{
	if (m_started)
	{
		StopAdsb();
	}

#if defined(DEBUGCONSOLE)
	ADSBprint::close();
#endif 
	
	DEBUG_PRINT("ADSBplugin::~ADSBplugin\n");
}

void ADSBplugin::StartAdsb()
{
	DEBUG_PRINT("ADSBplugin::StartAdsb - register stream processor\n");

	m_controller.RegisterStreamProcessor(0, this);
	std::lock_guard<std::mutex> l(m_lock);
	if (m_started)
	{
		return;
	}

	m_update_gr = MODES_RSP_INITIAL_GR;
	m_request_gr = m_update_gr;
	m_hold_off = 0;

	StartUp();
	m_started = true;

	if (!m_worker)
	{
		m_worker = new std::thread(&ADSBplugin::WorkerFunction, this);
	}
}

void ADSBplugin::StopAdsb()
{
	DEBUG_PRINT("ADSBplugin::StopAdsb - unregister stream processor\n");

	m_controller.UnregisterStreamProcessor(0, this);
	std::lock_guard<std::mutex> l(m_lock);
	if (!m_started)
	{
		return;
	}

	DEBUG_PRINT("ADSBplugin::Stop Thread\n");

	m_stream_started = false;
	m_started = false;
	m_worker->join();
	delete m_worker;
	m_worker = nullptr;

	DEBUG_PRINT("ADSBplugin::StopAdsb end\n");

	if (Modes.net)
	{
		modesUninitNet();
	}

	ModesUninit();
}

void ADSBplugin::HandleEvent(const UnoEvent& ev)
{
	m_form.HandleEvent(ev);
}

void ADSBplugin::SetGainControl(int enable)
{
	std::lock_guard<std::mutex> l(m_lock);
	m_gain_control = enable;
}

void ADSBplugin::SetAdsbMode(bool mode)
{
	DEBUG_PRINT("ADSBplugin::SetAdsbMode(%s)\n", (mode == true) ? "Oversample" : "Normal");

	std::lock_guard<std::mutex> l(m_lock);
	if (!m_started)
	{
		m_gui_config.oversample = (mode == true) ? 1 : 0;
		
		// Select demodulator mode
		if (mode)
		{
			if (m_controller.GetDemodulatorType(0) != IUnoPluginController::DemodulatorADSB8)
			{
				m_controller.SetDemodulatorType(0, IUnoPluginController::DemodulatorNone);
				nana::system::sleep(100);
				m_controller.SetDemodulatorType(0, IUnoPluginController::DemodulatorADSB8);
			}
		}
		else
		{
			if (m_controller.GetDemodulatorType(0) != IUnoPluginController::DemodulatorADSB2)
			{
				m_controller.SetDemodulatorType(0, IUnoPluginController::DemodulatorNone);
				nana::system::sleep(100);
				m_controller.SetDemodulatorType(0, IUnoPluginController::DemodulatorADSB2);
			}
		}
	}
}

void ADSBplugin::GrChanged(void)
{
	m_update_gr = m_request_gr;
	m_hold_off = 0;
	
	DEBUG_PRINT("grChanged() -> %d\n", m_request_gr);
}

void ADSBplugin::StreamProcessorProcess(channel_t channel, Complex* buffer, int length, bool& modified)
{	
	if ((channel == 0) && m_stream_started)
	{
		int i, count1, count2, new_buf_flag;
		int max_sig;
		float max_sig_d;
		unsigned int end, input_index;
	
		// Initialise heavily-used locals from Modes struct
		Complex* dptr = (Complex*)Modes.sdrplay_data;

		int max_sig_acc = Modes.max_sig;
		unsigned int data_index = Modes.data_index;

		// Assumptions; numSamples is smaller than MODES_RSP_BUF_SIZE, so either 0 or 1 buffers handed off
		// Think about what's going to happen, will we overrun end, will we fill a buffer?

		// count1 is lesser of input samples and samples to end of buffer 
		// count2 is the remainder, generally zero 

		end = data_index + length;
		count2 = end - (MODES_RSP_BUF_SIZE * MODES_RSP_BUFFERS);
		// count2 is samples wrapping around to start of buf 
		if (count2 < 0)
		{
			count2 = 0;
		}

		// count1 is samples fitting before the end of buf
		count1 = length - count2;              

		// Flag is set if this packet takes us past a multiple of MODES_RSP_BUF_SIZE 
		new_buf_flag = ((data_index & (MODES_RSP_BUF_SIZE - 1)) < (end & (MODES_RSP_BUF_SIZE - 1))) ? 0 : 1;

		// Now interleave data from I/Q into circular buffer, and note max I value 
		input_index = 0;
		max_sig_d = 0.0;

		int numsamples = (Modes.oversample) ? 1344 : 336;
		for (i = 0; i < (count1 - 1); i++)
		{
			if (buffer[input_index + i].real > max_sig_d) max_sig_d = buffer[input_index + i].real;

			if ((i != 0) && ((i % numsamples) == 0))
			{
				// Apply slowly decaying filter to max signal value 
				max_sig = (int)(max_sig_d * 32767.0) - 127;
				max_sig_acc += max_sig;
				max_sig = max_sig_acc >> RSP_ACC_SHIFT;
				max_sig_acc -= max_sig;
				max_sig_d = 0.0;
			}
		}
		max_sig = max_sig_acc >> RSP_ACC_SHIFT;

		memcpy(&dptr[data_index], &buffer[input_index], (count1 * sizeof(Complex)));
		data_index += count1;
		input_index += count1;

		// This code is triggered as we reach the end of our circular buffer 
		if (data_index >= (MODES_RSP_BUF_SIZE * MODES_RSP_BUFFERS))
		{
			// Pointer back to start of buffer 
			data_index = 0;  

			DEBUG_PRINT("max_sig %d reqGr %d updGr = %d\n", max_sig, m_request_gr, m_update_gr);
			
			if (m_update_gr == m_request_gr)
			{
				// Adjust gain if required 
				if (max_sig > RSP_MAX_GAIN_THRESH)
				{
					if (m_request_gr < 59)
					{
						m_request_gr += 1;
					}
				}
				else if (max_sig < RSP_MIN_GAIN_THRESH)
				{
					if (m_request_gr > 0)
					{
						m_request_gr -= 1;
					}
				}
			}
		}

		if (count2)
		{
			// Insert any remaining signal at start of buffer 
			memcpy(&dptr[data_index], &buffer[input_index], (count2 * sizeof(Complex)));
			data_index += count2;
		}

		// Send buffer downstream if enough available 
		if (new_buf_flag)
		{
			// Go back by one buffer length, then round down further to start of buffer 
			end = data_index + MODES_RSP_BUF_SIZE * (MODES_RSP_BUFFERS - 1);
			end &= MODES_RSP_BUF_SIZE * MODES_RSP_BUFFERS - 1;
			end &= ~(MODES_RSP_BUF_SIZE - 1);
			
			Complex* tmp = (Complex*)Modes.sdrplay_data;
			SamplesCallback(&tmp[end], MODES_RSP_BUF_SIZE);
		}

		// Stash static values in Modes struct 
		Modes.max_sig = max_sig_acc;
		Modes.data_index = data_index;		
	}
}

void ADSBplugin::SamplesCallback(Complex* buf, uint32_t len) 
{
	struct mag_buf *outbuf, *lastbuf;		
	unsigned free_bufs, block_duration;
	unsigned next_free_buffer;
	uint32_t slen;

	// Lock the data buffer variables before accessing them
	if (!m_data_mutex.try_lock_for(std::chrono::milliseconds(100)))
	{
		DEBUG_PRINT("SamplesCallback(1): dropping buffer as no lock for 100ms\n");
		return;
	}

	next_free_buffer = (Modes.first_free_buffer + 1) % Modes.mag_buf_num;
	outbuf = &Modes.mag_buffers[Modes.first_free_buffer];
	lastbuf = &Modes.mag_buffers[(Modes.first_free_buffer + Modes.mag_buf_num - 1) % Modes.mag_buf_num];
	free_bufs = (Modes.first_filled_buffer - next_free_buffer + Modes.mag_buf_num) % Modes.mag_buf_num;

	slen = len;

	if (free_bufs == 0 || (m_dropping && free_bufs < (unsigned)Modes.mag_buf_num / 2))
	{
		// FIFO is full. Drop this block.
		m_dropping = 1;
		outbuf->dropped += slen;
		m_data_mutex.unlock();
		return;
	}

	m_dropping = 0;
	m_data_mutex.unlock();

	// Compute the sample timestamp and system timestamp for the start of the block
	outbuf->sampleTimestamp = (uint64_t)(lastbuf->sampleTimestamp + 12e6 * (lastbuf->length + outbuf->dropped) / Modes.sample_rate);
	block_duration = (unsigned int)(1e9 * slen / Modes.sample_rate);

	// Get the approx system time for the start of this block
	clock_gettime(CLOCK_REALTIME, &outbuf->sysTimestamp);

	outbuf->sysTimestamp.tv_nsec -= block_duration;
	normalize_timespec(&outbuf->sysTimestamp);

	// Copy trailing data from last block (or reset if not valid)
	if (outbuf->dropped == 0 && lastbuf->length >= Modes.trailing_samples)
	{
		memcpy(outbuf->data, lastbuf->data + lastbuf->length - Modes.trailing_samples, Modes.trailing_samples * sizeof(uint16_t));
	}
	else
	{
		memset(outbuf->data, 0, Modes.trailing_samples * sizeof(uint16_t));
	}

	// Convert the new data
	outbuf->length = slen;
	ConvertComplex(buf, &outbuf->data[Modes.trailing_samples], slen, &outbuf->total_power);

	// Push the new data to the demodulation thread
	if (m_data_mutex.try_lock_for(std::chrono::milliseconds(100)) == false)
	{
		DEBUG_PRINT("SamplesCallback(2): dropping buffer as no lock for 100ms\n");
		return;
	}
	Modes.mag_buffers[next_free_buffer].dropped = 0;
	Modes.mag_buffers[next_free_buffer].length = 0;  // just in case
	Modes.first_free_buffer = next_free_buffer;

	// Accumulate CPU while holding the mutex, and restart measurement
	EndCpuTiming(&m_reader_thread_start, &Modes.reader_cpu_accumulator);
	StartCpuTiming(&m_reader_thread_start);
	m_data_mutex.unlock();

	{
		std::lock_guard<std::mutex> lock(m_cond_mutex);
		m_data_ready = true;
	}
	m_data_cond.notify_one();
}

void ADSBplugin::StartUp()
{
	char path[1024];	

	ModesInitConfig();

	// Hard coded options
	Modes.net = 1;
	Modes.interactive = 1;

	// Modes configured from GUI
	Modes.oversample = m_gui_config.oversample;
	Modes.net_http_ports = strdup(m_gui_config.net_http_ports.c_str());
	Modes.net_output_raw_ports = strdup(m_gui_config.net_output_raw_ports.c_str());
	Modes.net_output_sbs_ports = strdup(m_gui_config.net_output_sbs_ports.c_str());
	Modes.net_output_beast_ports = strdup(m_gui_config.net_output_beast_ports.c_str());

	// Changed from SHGetKnownFolderPath to continue support for Windows XP
	SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path);
	PathAppendA(path, PUBLIC_HTML_DATA_PATH);
	Modes.json_dir = strdup(path);

	ModesInit();

	if (Modes.net)
	{
		modesInitNet();
	}

	// Init stats
	uint64_t t = mstime();
	Modes.stats_current.start = t;
	Modes.stats_current.end = t;
	Modes.stats_alltime.start = t;
	Modes.stats_alltime.end = t;
	Modes.stats_periodic.start = t;
	Modes.stats_periodic.end = t;
	Modes.stats_5min.start = t;
	Modes.stats_5min.end = t;
	Modes.stats_15min.start = t;
	Modes.stats_15min.end = t;

	for (int j = 0; j < 15; ++j)
	{
		Modes.stats_1min[j].start = Modes.stats_current.start;
		Modes.stats_1min[j].end = Modes.stats_current.start;
	}

	// Write initial json files so they're not missing
	writeJsonToFile("receiver.json", generateReceiverJson);
	writeJsonToFile("stats.json", generateStatsJson);
	writeJsonToFile("aircraft.json", generateAircraftJson);	
}

void ADSBplugin::WorkerFunction()
{
	DEBUG_PRINT("Worker++\n");
	
	char** interactive_data = (char**)malloc((Modes.interactive_rows + 2) * sizeof(char*));
	for (int i = 0; i < (Modes.interactive_rows + 2); ++i)
	{
		interactive_data[i] = (char*)calloc(255, sizeof(char));
	}

	m_data_ready = false;

	// Initialization
	DEBUG_PRINT("%s %s starting up.\n", MODES_DUMP1090_VARIANT, MODES_DUMP1090_VERSION);

	// About 1 second
	int watchdog_counter = 10; 
	m_stream_started = true;

	while (m_started)
	{
		struct timespec start_time;

		if (m_data_mutex.try_lock_for(std::chrono::milliseconds(100)) == false)
		{
			DEBUG_PRINT("WorkerFunction(1): dropping buffer as no lock for 100ms\n");
			continue;
		}
			
		if (Modes.first_free_buffer == Modes.first_filled_buffer)
		{
			m_data_mutex.unlock();
			{
				std::unique_lock<std::mutex> lock(m_cond_mutex);
				
				// Wait for more data.
				// We should be getting data every 50-60ms. wait for max 100ms before we give up and do some background work.
				// This is fairly aggressive as all our network I/O runs out of the background work!				
				m_data_cond.wait_for(lock, std::chrono::milliseconds(100), [&] {return m_data_ready; });
				m_data_ready = false;
			}
			if (m_data_mutex.try_lock_for(std::chrono::milliseconds(100)) == false)
			{
				DEBUG_PRINT("WorkerFunction(2): dropping buffer as no lock for 100ms\n");
				continue;
		}
}
		// data_mutex is locked, and possibly we have data.

		// Copy out reader CPU time and reset it
		add_timespecs(&Modes.reader_cpu_accumulator, &Modes.stats_current.reader_cpu, &Modes.stats_current.reader_cpu);
		Modes.reader_cpu_accumulator.tv_sec = 0;
		Modes.reader_cpu_accumulator.tv_nsec = 0;

		if (Modes.first_free_buffer != Modes.first_filled_buffer)
		{
			m_data_mutex.unlock();
			// FIFO is not empty, process one buffer.

			// Gain updates first if required
			if (m_hold_off == 0)
			{
				if (m_update_gr < m_request_gr)
				{
					if (m_gain_control)
					{
						if (m_controller.SetIFGRRelative(0, 1) == 0)
						{
							DEBUG_PRINT("Failed to increment gain reduction\n");
							m_request_gr -= 1;
						}						
					}
				}
				else if (m_update_gr > m_request_gr)
				{
					if (m_gain_control)
					{
						if (m_controller.SetIFGRRelative(0, -1) == 0)
						{
							DEBUG_PRINT("Failed to decrement gain reduction\n");
							m_request_gr += 1;
						}
					}
				}
					
				if (m_update_gr != m_request_gr)
				{
					m_hold_off = 1;
				}
			}
				
			if (m_data_mutex.try_lock_for(std::chrono::milliseconds(100)) == false)
			{
				DEBUG_PRINT("WorkerFunction(3): dropping buffer as no lock for 100ms\n");
				continue;
			}
				
			struct mag_buf* buf;

			StartCpuTiming(&start_time);
			buf = &Modes.mag_buffers[Modes.first_filled_buffer];

			// Process data after releasing the lock, so that the capturing
			// thread can read data while we perform computationally expensive
			// stuff at the same time.
			m_data_mutex.unlock();

			if (Modes.oversample)
			{
				demodulate8000(buf);
			}
			else
			{
				demodulate2000(buf);
			}

			Modes.stats_current.samples_processed += buf->length;
			Modes.stats_current.samples_dropped += buf->dropped;
			EndCpuTiming(&start_time, &Modes.stats_current.demod_cpu);

			// Mark the buffer we just processed as completed.
			if (m_data_mutex.try_lock_for(std::chrono::milliseconds(100)) == false)
			{
				DEBUG_PRINT("WorkerFunction(4): dropping buffer as no lock for 100ms\n");
				continue;
			}
				
			Modes.first_filled_buffer = (Modes.first_filled_buffer + 1) % Modes.mag_buf_num;
			m_data_mutex.unlock();
			watchdog_counter = 10;
		}
		else
		{
			// Nothing to process this time around.
			m_data_mutex.unlock();
			if (--watchdog_counter <= 0)
			{
				DEBUG_PRINT("No data received for a long time\n");
				watchdog_counter = 600;
			}
		}
			
		StartCpuTiming(&start_time);
		BackgroundTasks(interactive_data);
		EndCpuTiming(&start_time, &Modes.stats_current.background_cpu);
	}
		
	while (m_stream_started == true)
	{
		DEBUG_PRINT("Waiting for stream to stop\n");
		nana::system::sleep(100);
	}		

	DEBUG_PRINT("Normal exit\n");

	if (interactive_data)
	{
	   for (int i = 0; i < (Modes.interactive_rows + 2); ++i)
	   {
		   if (interactive_data[i])
		   {
 			   free(interactive_data[i]);
		   }
	   }

	   free(interactive_data);
	   interactive_data = NULL;
	}

	DEBUG_PRINT("Worker--\n");
}

void ADSBplugin::ModesUninit(void)
{
	// Strings
	if (Modes.net_input_raw_ports)
	{
		free(Modes.net_input_raw_ports);
		Modes.net_input_raw_ports = NULL;
	}	
	if (Modes.net_output_raw_ports)
	{
		free(Modes.net_output_raw_ports);
		Modes.net_output_raw_ports = NULL;
	}
	if (Modes.net_output_sbs_ports)
	{
		free(Modes.net_output_sbs_ports);
		Modes.net_output_sbs_ports = NULL;
	}
	if (Modes.net_input_beast_ports)
	{
		free(Modes.net_input_beast_ports);
		Modes.net_input_beast_ports = NULL;
	}
	if (Modes.net_output_beast_ports)
	{
		free(Modes.net_output_beast_ports);
		Modes.net_output_beast_ports = NULL;
	}
	if (Modes.net_http_ports)
	{
		free(Modes.net_http_ports);
		Modes.net_http_ports = NULL;
	}
	if (Modes.html_dir)
	{
		free(Modes.html_dir);
		Modes.html_dir = NULL;
	}
	if (Modes.json_dir)
	{
		free(Modes.json_dir);
		Modes.json_dir = NULL;
	}

	// Buffers
	if (Modes.sdrplay_data)
	{
		free(Modes.sdrplay_data);
		Modes.sdrplay_data = NULL;
	}

	if (Modes.d8m_match_ar)
	{
		free(Modes.d8m_match_ar);
		Modes.d8m_match_ar = NULL;
	}

	if (Modes.d8m_phase_ar)
	{
		free(Modes.d8m_phase_ar);
		Modes.d8m_phase_ar = NULL;
	}

	if (Modes.d8m_dbuf)
	{
		free(Modes.d8m_dbuf);
		Modes.d8m_dbuf = NULL;
	}

	if (Modes.log10lut)
	{
		free(Modes.log10lut);
		Modes.log10lut = NULL;
	}

	if (Modes.mag_buffers)
	{
		for (int i = 0; i < Modes.mag_buf_num; ++i)
		{
			if (Modes.mag_buffers[i].data)
			{
				free(Modes.mag_buffers[i].data);
			}
		}

		free(Modes.mag_buffers);
		Modes.mag_buffers = NULL;
	}
}

void ADSBplugin::ModesInitConfig(void)
{
	char path[1024];	

	// Default everything to zero/NULL
	memset(&Modes, 0, sizeof(Modes));

	// Now initialise things that should not be 0/NULL to their defaults
	Modes.freq = MODES_DEFAULT_FREQ;
	Modes.check_crc = 1;
	Modes.net_heartbeat_interval = MODES_NET_HEARTBEAT_INTERVAL;
	Modes.net_input_raw_ports = strdup("30001");
	Modes.net_input_beast_ports = strdup("30004,30104");

	Modes.interactive_rows = MODES_INTERACTIVE_ROWS;
	Modes.interactive_display_ttl = MODES_INTERACTIVE_DISPLAY_TTL;
	
	// changed from SHGetKnownFolderPath to continue support for Windows XP
	SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path);
	PathAppendA(path, PUBLIC_HTML_PATH);
	Modes.html_dir = strdup(path);

	Modes.json_interval = 1000;
	Modes.json_location_accuracy = 1;
	Modes.maxRange = 1852 * 300; // 300NM default max range
	Modes.isSocket = 1;

	Modes.max_sig = RSP_MIN_GAIN_THRESH << RSP_ACC_SHIFT;
}

void ADSBplugin::SetPorts(const std::string& http, const std::string& raw, const std::string& sbs, const std::string& beast)
{
	m_gui_config.net_http_ports = http;
	m_gui_config.net_output_raw_ports = raw;
	m_gui_config.net_output_sbs_ports = sbs;
	m_gui_config.net_output_beast_ports = beast;
}

int ADSBplugin::ModesInit(void)
{
	DEBUG_PRINT("ADSBplugin::modesInit\n");

	Modes.sample_rate = Modes.oversample ? 8000000.0 : 2000000.0;
	
	// Allocate data buffers
	Modes.sdrplay_data = (Complex*)malloc(MODES_RSP_BUF_SIZE * MODES_RSP_BUFFERS * sizeof(Complex));

	if (Modes.sdrplay_data == NULL)
	{
		if (Modes.interactive)
		{
			Modes.interactive = 0;
		}

		DEBUG_PRINT("Insufficient memory for buffers\n");
		return 1;
	}

	Modes.sdr_buf_size = MODES_RSP_BUF_SIZE;
	Modes.mag_buf_samples = MODES_MAG_BUF_SAMPLES;
	Modes.mag_buf_num = MODES_MAG_BUFFERS;

	// arrays for demod_8000 if used.
	if (Modes.sample_rate == 8000000.0)
	{
		Modes.d8m_match_ar = (int*)malloc(D8M_WIN_LEN * sizeof(int));
		Modes.d8m_phase_ar = (int*)malloc(D8M_WIN_LEN * sizeof(int));
		Modes.d8m_dbuf = (int*)malloc((D8M_BUF_OVERLAP + MODES_MAG_BUF_SAMPLES) * sizeof(int));

		if ((Modes.d8m_match_ar == NULL) || (Modes.d8m_phase_ar == NULL) || (Modes.d8m_dbuf == NULL))
		{
			DEBUG_PRINT("Out of memory allocating data buffer.\n");
			return 1;
		}

		memset(Modes.d8m_dbuf, 0, (D8M_BUF_OVERLAP + MODES_MAG_BUF_SAMPLES) * sizeof(int));
	}

	Modes.mag_buffers = (struct mag_buf*)malloc(sizeof(struct mag_buf) * Modes.mag_buf_num);
	memset(Modes.mag_buffers, 0, sizeof(struct mag_buf) * Modes.mag_buf_num);

	// Allocate the various buffers used by Modes
	Modes.trailing_samples = (unsigned int)((MODES_PREAMBLE_US + MODES_LONG_MSG_BITS + 16) * 1e-6 * Modes.sample_rate);

	if ((Modes.log10lut = (uint16_t*)malloc(sizeof(uint16_t) * 256 * 256)) == NULL)
	{
		DEBUG_PRINT("Out of memory allocating data buffer.\n");
		return 1;
	}

	for (int i = 0; i < Modes.mag_buf_num; ++i)
	{
		if ((Modes.mag_buffers[i].data = (uint16_t*)calloc(Modes.mag_buf_samples + Modes.trailing_samples, sizeof(uint16_t))) == NULL)
		{
			DEBUG_PRINT("Out of memory allocating magnitude buffer.\n");
			return 1;
		}

		Modes.mag_buffers[i].length = 0;
		Modes.mag_buffers[i].dropped = 0;
		Modes.mag_buffers[i].sampleTimestamp = 0;
	}

	// Validate the users Lat/Lon home location inputs
	if ((Modes.fUserLat > 90.0)  // Latitude must be -90 to +90
		|| (Modes.fUserLat < -90.0)  // and 
		|| (Modes.fUserLon > 360.0)  // Longitude must be -180 to +360
		|| (Modes.fUserLon < -180.0))
	{
		Modes.fUserLat = Modes.fUserLon = 0.0;
	}
	else 
	if (Modes.fUserLon > 180.0)
	{ 
		// If Longitude is +180 to +360, make it -180 to 0
		Modes.fUserLon -= 360.0;
	}
	
	// If both Lat and Lon are 0.0 then the users location is either invalid/not-set, or (s)he's in the 
	// Atlantic ocean off the west coast of Africa. This is unlikely to be correct. 
	// Set the user LatLon valid flag only if either Lat or Lon are non zero. Note the Greenwich meridian 
	// is at 0.0 Lon,so we must check for either fLat or fLon being non zero not both. 
	// Testing the flag at runtime will be much quicker than ((fLon != 0.0) || (fLat != 0.0))
	Modes.bUserFlags &= ~MODES_USER_LATLON_VALID;
	if ((Modes.fUserLat != 0.0) || (Modes.fUserLon != 0.0))
	{
		Modes.bUserFlags |= MODES_USER_LATLON_VALID;
	}

	// Limit the maximum requested raw output size to less than one Ethernet Block 
	if (Modes.net_output_flush_size > (MODES_OUT_FLUSH_SIZE))
	{
		Modes.net_output_flush_size = MODES_OUT_FLUSH_SIZE;
	}
	if (Modes.net_output_flush_interval > (MODES_OUT_FLUSH_INTERVAL))
	{
		Modes.net_output_flush_interval = MODES_OUT_FLUSH_INTERVAL;
	}
	if (Modes.net_sndbuf_size > (MODES_NET_SNDBUF_MAX))
	{
		Modes.net_sndbuf_size = MODES_NET_SNDBUF_MAX;
	}

	// Prepare the log10 lookup table: 100log10(x)
	Modes.log10lut[0] = 0; // poorly defined..
	for (int i = 1; i <= 65535; i++)
	{
		Modes.log10lut[i] = (uint16_t)round(100.0 * log10(i));
	}

	// Prepare error correction tables
	modesChecksumInit(Modes.nfix_crc);
	icaoFilterInit();
	
	// Prepare sample conversion
	InitConverter(Modes.sample_rate, Modes.dc_filter);

	DEBUG_PRINT("ADSBplugin::modesInit end\n");
	return 0;
}

void ADSBplugin::InitConverter(double sample_rate, int filter_dc)
{
	m_z1_I = 0.0f;
	m_z1_Q = 0.0f;

	if (filter_dc)
	{
		// Init DC block @ 1Hz
		m_dc_b = (float)(exp(-2.0 * M_PI * 1.0 / sample_rate));
		m_dc_a = (float)(1.0 - m_dc_b);
	}
	else
	{
		// If the converter does filtering, make sure it has no effect
		m_dc_b = 1.0f;
		m_dc_a = 0.0f;
	}

	return;
}

void ADSBplugin::ConvertComplex(void* iq_data, uint16_t* mag_data, unsigned nsamples, double* out_power)
{
	Complex* in = (Complex*)iq_data;
	double power = 0.0;

	unsigned i;
	float fI, fQ, magsq;

	for (i = 0; i < nsamples; ++i)
	{
		fI = in[i].real;;
		fQ = in[i].imag;

		// DC block
		m_z1_I = fI * m_dc_a + m_z1_I * m_dc_b;
		m_z1_Q = fQ * m_dc_a + m_z1_Q * m_dc_b;
		fI -= m_z1_I;
		fQ -= m_z1_Q;

		magsq = fI * fI + fQ * fQ;
		if (magsq > 1.0)
		{
			magsq = 1.0;
		}

		power += (double)magsq;
		*mag_data++ = (uint16_t)(sqrtf(magsq) * 65535.0 + 0.5);
	}

	if (out_power)
	{
		*out_power = power;
	}
}

void ADSBplugin::BackgroundTasks(char** interactive_data)
{
	uint64_t now = mstime();

	icaoFilterExpire();
	trackPeriodicUpdate();

	if (Modes.net)
	{
		modesNetPeriodicWork();
	}

	// Refresh screen when in interactive mode
	if (Modes.interactive)
	{
		if (interactive_data)
		{
			for (int i = 0; i < (Modes.interactive_rows + 2); ++i)
			{
				memset(interactive_data[i], 0, 80 * sizeof(char));
			}
		}

		if (GetInteractiveData(interactive_data))
		{
			if (m_started)
			{
				m_form.ClearDisplay();
			}
			
			for (int i = 0; i < (Modes.interactive_rows + 2); ++i)
			{
				if (m_started)
				{
					m_form.PutDisplayString(interactive_data[i]);
				}
			}
		}
	}

	// always update end time so it is current when requests arrive
	Modes.stats_current.end = now;

	if (now >= m_next_stats_update)
	{
		if (m_next_stats_update == 0)
		{
			m_next_stats_update = now + 60000;
		}
		else
		{
			Modes.stats_latest_1min = (Modes.stats_latest_1min + 1) % 15;
			Modes.stats_1min[Modes.stats_latest_1min] = Modes.stats_current;

			add_stats(&Modes.stats_current, &Modes.stats_alltime, &Modes.stats_alltime);
			add_stats(&Modes.stats_current, &Modes.stats_periodic, &Modes.stats_periodic);

			reset_stats(&Modes.stats_5min);
			for (int i = 0; i < 5; ++i)
			{
				add_stats(&Modes.stats_1min[(Modes.stats_latest_1min - i + 15) % 15], &Modes.stats_5min, &Modes.stats_5min);
			}

			reset_stats(&Modes.stats_15min);
			for (int i = 0; i < 15; ++i)
			{
				add_stats(&Modes.stats_1min[i], &Modes.stats_15min, &Modes.stats_15min);
			}

			reset_stats(&Modes.stats_current);
			Modes.stats_current.start = Modes.stats_current.end = now;

			if (Modes.json_dir)
			{
				writeJsonToFile("stats.json", generateStatsJson);
			}

			m_next_stats_update += 60000;
		}
	}

	if (Modes.json_dir && now >= m_next_json)
	{
		writeJsonToFile("aircraft.json", generateAircraftJson);
		m_next_json = now + Modes.json_interval;
	}

	if (now >= m_next_history)
	{
		int rewrite_receiver_json = (Modes.json_dir && Modes.json_aircraft_history[HISTORY_SIZE - 1].content == NULL);

		free(Modes.json_aircraft_history[Modes.json_aircraft_history_next].content); // might be NULL, that's OK.
		Modes.json_aircraft_history[Modes.json_aircraft_history_next].content =
			generateAircraftJson("/data/aircraft.json", &Modes.json_aircraft_history[Modes.json_aircraft_history_next].clen);

		if (Modes.json_dir)
		{
			char filebuf[PATH_MAX];
			snprintf(filebuf, PATH_MAX, "history_%d.json", Modes.json_aircraft_history_next);
			writeJsonToFile(filebuf, generateHistoryJson);
		}

		Modes.json_aircraft_history_next = (Modes.json_aircraft_history_next + 1) % HISTORY_SIZE;

		if (rewrite_receiver_json)
		{
			writeJsonToFile("receiver.json", generateReceiverJson); // number of history entries changed
		}

		m_next_history = now + HISTORY_INTERVAL;
	}
}

int ADSBplugin::ConvertAltitude(int ft)
{
	if (Modes.metric)
	{
		return (int)(ft * 0.3048);
	}
	
	return ft;
}

int ADSBplugin::ConvertSpeed(int kts)
{
	if (Modes.metric)
	{
		return (int)(kts * 1.852);
	}

	return kts;
}

void ADSBplugin::ShowData(void)
{
	struct aircraft* a = Modes.aircrafts;
	uint64_t now = mstime();
	int count = 0;

	// Refresh screen every (MODES_INTERACTIVE_REFRESH_TIME) miliseconde
	if (now < m_next_update)
	{
		return;
	}

	m_next_update = now + MODES_INTERACTIVE_REFRESH_TIME;

#if DEBUGCONSOLE
	char progress;
	const char spinner[5] = "|/-\\";
	progress = spinner[(now / 1000) % 4];
	
	if (Modes.interactive_rtl1090 == 0)
	{
		DEBUG_PRINT("Hex    Mode  Sqwk  Flight   Alt    Spd  Hdg    Lat      Long    RSSI  Msgs  Ti%c\n", progress);
	}
	else
	{
		DEBUG_PRINT("Hex   Flight   Alt      V/S GS  TT  SSR  G*456^ Msgs    Seen %c\n", progress);
	}
	
	DEBUG_PRINT(
		"--------------------------------------------------------------------------------\n");
#endif

	while (a && (count < Modes.interactive_rows))
	{

		if ((now - a->seen) < Modes.interactive_display_ttl)
		{
			int msgs = a->messages;
			int flags = a->modeACflags;

			if ((((flags & (MODEAC_MSG_FLAG)) == 0) && (msgs > 1))
				|| (((flags & (MODEAC_MSG_MODES_HIT | MODEAC_MSG_MODEA_ONLY)) == MODEAC_MSG_MODEA_ONLY) && (msgs > 4))
				|| (((flags & (MODEAC_MSG_MODES_HIT | MODEAC_MSG_MODEC_OLD)) == 0) && (msgs > 127))
				)
			{
				char strSquawk[5] = " ";
				char strFl[7] = " ";
				char strTt[5] = " ";
				char strGs[5] = " ";

				if (a->bFlags & MODES_ACFLAGS_SQUAWK_VALID)
				{
					snprintf(strSquawk, 5, "%04x", a->modeA);
				}

				if (a->bFlags & MODES_ACFLAGS_SPEED_VALID)
				{
					snprintf(strGs, 5, "%3d", ConvertSpeed(a->speed));
				}

				if (a->bFlags & MODES_ACFLAGS_HEADING_VALID)
				{
					snprintf(strTt, 5, "%03d", a->track);
				}

				if (msgs > 99999)
				{
					msgs = 99999;
				}
				                       				
				char strMode[5] = "    ";
				char strLat[8] = " ";
				char strLon[9] = " ";
				double* pSig = a->signalLevel;
				double signalAverage = (pSig[0] + pSig[1] + pSig[2] + pSig[3] + pSig[4] + pSig[5] + pSig[6] + pSig[7]) / 8.0;

				if ((flags & MODEAC_MSG_FLAG) == 0)
				{
					strMode[0] = 'S';
				}
				else if (flags & MODEAC_MSG_MODEA_ONLY)
				{
					strMode[0] = 'A';
				}
				if (flags & MODEAC_MSG_MODEA_HIT) 
				{ 
					strMode[2] = 'a'; 
				}
				if (flags & MODEAC_MSG_MODEC_HIT) 
				{ 
					strMode[3] = 'c'; 
				}

				if (a->bFlags & MODES_ACFLAGS_LATLON_VALID)
				{
					snprintf(strLat, 8, "%7.03f", a->lat);
					snprintf(strLon, 9, "%8.03f", a->lon);
				}

				if (a->bFlags & MODES_ACFLAGS_AOG)
				{
					snprintf(strFl, 7, " grnd ");
				}
				else if (Modes.use_hae && (a->bFlags & MODES_ACFLAGS_ALTITUDE_HAE_VALID))
				{
					snprintf(strFl, 7, "%5dH", ConvertAltitude(a->altitude_hae));
				}
				else if (a->bFlags & MODES_ACFLAGS_ALTITUDE_VALID)
				{
					snprintf(strFl, 7, "%5d ", ConvertAltitude(a->altitude));
				}
					
				DEBUG_PRINT("%s%06X %-4s  %-4s  %-8s %6s %3s  %3s  %7s %8s  %5.1f %5d  %2.0f\n",
					(a->addr & MODES_NON_ICAO_ADDRESS) ? "~" : " ", (a->addr & 0xffffff),
					strMode, strSquawk, a->flight, strFl, strGs, strTt,
					strLat, strLon, 10 * log10(signalAverage), msgs, (now - a->seen) / 1000.0);

				count++;
			}
		}
		
		a = a->next;
	}
}
