#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <iunoplugincontroller.h>
#include <iunoplugin.h>
#include <iunostreamobserver.h>
#include <iunoaudioobserver.h>
#include <iunoaudioprocessor.h>
#include <iunostreamobserver.h>
#include <iunoannotator.h>

#include "ADSBpluginUi.h"
#include "winstubs.h"

#define PUBLIC_HTML_DATA_PATH "\\SDRplay\\public_html\\data"
#define PUBLIC_HTML_PATH "\\SDRplay\\public_html"

typedef struct adsb_config_t
{
	int oversample;

	// Other possible options that could be configured from the GUI

	std::string net_output_raw_ports;
	std::string net_output_beast_ports;
	std::string net_http_ports;
	std::string net_output_sbs_ports;

} AdsbConfigT;

class ADSBplugin : public IUnoPlugin,
	public IUnoStreamProcessor
{

public:

	ADSBplugin(IUnoPluginController& controller);
	virtual ~ADSBplugin();

	// IUnoPlugin
	virtual void HandleEvent(const UnoEvent& ev) override;

	// IUnoStreamProcessor
	virtual void StreamProcessorProcess(channel_t channel, Complex* buffer, int length, bool& modified) override;

	void StartAdsb();
	void StopAdsb();
	void SetAdsbMode(bool mode);
	void SetGainControl(int enable);
	void GrChanged(void);
	void SetPorts(const std::string& http, const std::string& raw, const std::string& sbs, const std::string& beast);

private:

	void ModesInitConfig(void);
	int ModesInit(void);
	void ModesUninit(void);

	void InitConverter(double sample_rate, int filter_dc);
	void ConvertComplex(void* iq_data, uint16_t* mag_data, unsigned nsamples, double* out_power);

	void SamplesCallback(Complex* buf, uint32_t len);

	void BackgroundTasks(char** interactive_data);
	void ShowData(void);

	int ConvertAltitude(int ft);
	int ConvertSpeed(int kts);

	void StartUp();
	void WorkerFunction();

	void StartCpuTiming(struct timespec* start_time)
	{
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, start_time);
	}

	void EndCpuTiming(const struct timespec* start_time, struct timespec* add_to)
	{
		struct timespec end_time;
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end_time);

		add_to->tv_sec += (end_time.tv_sec - start_time->tv_sec - 1);
		add_to->tv_nsec += (1000000000L + end_time.tv_nsec - start_time->tv_nsec);
		add_to->tv_sec += add_to->tv_nsec / 1000000000L;
		add_to->tv_nsec = add_to->tv_nsec % 1000000000L;
	}

	AdsbConfigT m_gui_config;
	bool m_gain_control;

	struct timespec m_reader_thread_start;

	std::mutex m_lock;

	std::timed_mutex m_data_mutex;
	std::mutex m_cond_mutex;
	std::condition_variable m_data_cond;

	float m_dc_a;
	float m_dc_b;
	float m_z1_I;
	float m_z1_Q;
	int m_update_gr;
	int m_request_gr;

	ADSBpluginUi m_form;
	std::thread* m_worker;
	bool m_started;
	bool m_stream_started;
	bool m_data_ready;
	int m_dropping;
	int m_hold_off;

	uint64_t m_next_stats_display;
	uint64_t m_next_stats_update;
	uint64_t m_next_json;
	uint64_t m_next_history;
	uint64_t m_next_update;
};
