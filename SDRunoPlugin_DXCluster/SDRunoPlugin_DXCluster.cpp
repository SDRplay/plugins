#include <sstream>
#include <unoevent.h>
#include <iunoplugincontroller.h>
#include <vector>
#include <sstream>
#include <chrono>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib") // Winsock library

#include "SDRunoPlugin_DXCluster.h"
#include "SDRunoPlugin_DXClusterUi.h"

#define MAXBUF 4096
#define bzero(x,y) memset(x,0,y);

#if defined(_M_X64) || defined(_M_IX86)
WSADATA wsa;
SOCKET sock;
#endif

#if defined(__GNUC__)
int socket;
#endif

SDRunoPlugin_DXCluster::SDRunoPlugin_DXCluster(IUnoPluginController& controller) :
	IUnoPlugin(controller),
	m_form(*this, controller),
	m_worker(nullptr),
	head(NULL),
	tail(NULL),
	current(NULL),
	prev(NULL),
	freqcurr(NULL),
	annocurr(NULL),
	dxCount(0),
	sampleRate(2000000),
	m_started(false),
	responseMatched(false)
{
}

SDRunoPlugin_DXCluster::~SDRunoPlugin_DXCluster()
{	
}

void SDRunoPlugin_DXCluster::AnnotatorProcess(std::vector<IUnoAnnotatorItem>& items)
{
	const uint32_t colors[] = {
		0x00ff0000, // red
		0x00ffff00, // yellow
		0x00ff00ff, // purple
		0x00aaff00, // green
		0x0030a0ff, // light blue
		0x00ffffff  // white
	};

	annocurr = head;
	int count;
	while (annocurr != NULL)
	{
		IUnoAnnotatorItem im;
		im.frequency = annocurr->freq;
		im.text = annocurr->callsign;
		count = checkLocalFreq(im.frequency, im.text);
		im.power = m_form.GetBaseline() + (count * 5);
		im.rgb = colors[m_form.GetColourIndex()];
		if (im.power == m_form.GetBaseline())
		{
			im.style = IUnoAnnotatorStyle::AnnotatorStyleMarkerAndLine;
		}
		else
		{
			im.style = IUnoAnnotatorStyle::AnnotatorStyleMarker;
		}
			
		items.push_back(im);
		if (annocurr->next != NULL)
		{
			annocurr = annocurr->next;
		}
		else
		{
			break;
		}
	}
	dxCount = items.size();
}

int SDRunoPlugin_DXCluster::checkLocalFreq(long long freq, std::string callsign)
{
	freqcurr = head;
	int count = 0;
	int window;
	if (sampleRate >= 2000000)
	{
		window = (int)(sampleRate / 100.);
	}
	else
	{
		window = (int)(sampleRate / 20.);
	}
	while (freqcurr != NULL)
	{
		if (callsign.compare(freqcurr->callsign) == 0)
		{
			break;
		}
		else
		{
			if (abs(freq - freqcurr->freq) < window)
			{
				count++;
			}
		}
		freqcurr = freqcurr->next;
	}
	return count;
}

void SDRunoPlugin_DXCluster::StartAnnotator()
{
	m_controller.RegisterAnnotator(this);
}

void SDRunoPlugin_DXCluster::StopAnnotator()
{
	m_controller.UnregisterAnnotator(this);
}

void SDRunoPlugin_DXCluster::StartDXCluster(std::string addr, std::string port, std::string callsign, int timeMins, std::string response)
{
	cAddr = addr;
	cPort = port;
	cCallsign = callsign;
	timeMinutes = timeMins;
	cResponse = response;

	std::lock_guard<std::mutex> l(m_lock);
	if (m_started)
	{
		return;
	}

	m_started = true;
	StartAnnotator();
	m_worker = new std::thread(&SDRunoPlugin_DXCluster::WorkerFunction, this);
}

void SDRunoPlugin_DXCluster::StopDXCluster()
{
	if (!m_started)
	{
		return;
	}

	StopAnnotator();
	m_started = false;
	closesocket(sock);
	m_worker->join();
	delete m_worker;
	m_worker = nullptr;
}

void SDRunoPlugin_DXCluster::HandleEvent(const UnoEvent& ev)
{
	m_form.HandleEvent(ev);	
}

// read data until get string
bool SDRunoPlugin_DXCluster::waitforstring(int sockfd, const char *s)
{
	int res, pos = 0;
	char buffer[MAXBUF];
	bzero(buffer, MAXBUF);

	if (responseMatched)
	{
		return true;
	}

	while (res = recv(sockfd, buffer + pos, sizeof(buffer) - pos, 0) > 0)
	{
		if (!m_started)
		{
			return false;
		}

		if (responseMatched)
		{
			return true;
		}

		if (res > 0)
		{
			pos += res;
			buffer[MAXBUF - 1] = 0;
#ifdef DEBUG
			char outbuf[MAXBUF];
			sprintf(outbuf, "SDRuno wait %s", buffer);
			OutputDebugStringA(outbuf);
#endif
			if (strstr(buffer, s))
			{
#ifdef DEBUG
				OutputDebugStringA("SDRuno Response Matched");
#endif
				responseMatched = true;
				return true;
			}
		}
	}
	return false;
}

void SDRunoPlugin_DXCluster::sendstring(int sockfd, const char *s)
{
	int len = strlen(s);
	send(sockfd, s, len, 0);
}

void SDRunoPlugin_DXCluster::WorkerFunction()
{
	struct hostent *server;
	struct sockaddr_in serv_addr;

	int port = stoi(cPort);

#if defined(_M_X64) || defined(_M_IX86)
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		m_started = false;
		return;
	}
#endif

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
#if defined(_M_X64) || defined(_M_IX86)
		WSACleanup();
#endif
		m_started = false;
		return;
	}

	server = gethostbyname(cAddr.c_str());
	serv_addr.sin_addr.s_addr = inet_addr(server->h_addr);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

	// Connect to server
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
#if defined(_M_X64) || defined(_M_IX86)
		closesocket(sock);
		WSACleanup();
#else
		close(s);
#endif
		m_started = false;
		return;
	}

	// Login to Telnet Server
	if (cCallsign.length() > 1)
	{
		waitforstring(sock, "ogin: ");
		responseMatched = false;
		char csTmp[16];
		sprintf(csTmp, "%s\r\n", cCallsign.c_str());
		sendstring(sock, csTmp);
		char wsTmp[255];
		sprintf(wsTmp, "%s", cResponse.c_str());
		if (cResponse.length() > 1)
		{
			if (!waitforstring(sock, wsTmp))
			{
				m_started = false;
			}
		}
		else
		{
			responseMatched = true;
		}
	}
	else
	{
		responseMatched = true;
	}

	int res, pos = 0;
	char buffer[MAXBUF];

	// Now process DX records
	while (m_started && responseMatched)
	{
		pos = 0;
		bzero(buffer, MAXBUF);
		while (1)
		{
			res = recv(sock, buffer + pos, sizeof(buffer) - pos, 0);

			if (res == SOCKET_ERROR)
			{
				m_started = false;
			}

			if (!m_started)
			{
				break;
			}

			if (res > 0)
			{
				pos += res;
				buffer[MAXBUF - 1] = 0;
#ifdef DEBUG
				char out[MAXBUF];
				sprintf(out, "SDRuno in %s", buffer);
				OutputDebugStringA(out);
#endif
				char *cstr = strstr(buffer, "DX de");
				if (cstr != NULL)
				{
					processDX(buffer);
					bzero(buffer, MAXBUF);
					pos = 0;
				}
			}
		}
	}
#if defined(_M_X64) || defined(_M_IX86)
	closesocket(sock);
	WSACleanup();
#else
	close(s);
#endif
}

void SDRunoPlugin_DXCluster::processDX(const char *s)
{
	// process DX line (TAB delimited) 80 columns
	// examples:
	// 0.........1.........2.........3.........4.........5.........6.........7.........
	// 01234567890123456789012345678901234567890123456789012345678901234567890123456789
	// DX de RW7F:		21190.0  R3UG         TNX                            0849Z
	// DX de SV3GLL:	24915.0  UT8IA        FT8 - 03dB from KN29 1414Hz    0849Z KM17
	// DX de DJ9YE:     50313.0  OH6OKSA      JO43HV<ES>KP32EQ FT8 mni tnx Q 0849Z JO43
	// DX de DL4CH:  10489710.0  DF7DQ        qo 100                         0853Z
	if (s == NULL)
	{
#ifdef DEBUG
		OutputDebugStringA("SDRuno processDX null string");
#endif
		return;
	}
#ifdef DEBUG
	char out[MAXBUF];
	sprintf(out, "SDRuno pr %s", s);
	OutputDebugStringA(out);
#endif
	std::string lineToProcess = s;
	int dxdepos = lineToProcess.find("DX de");

	if (dxdepos == std::string::npos)
	{
#ifdef DEBUG
		OutputDebugStringA("SDRuno DX de not found");
#endif
		return;
	}

	std::string sfreqkHz = lineToProcess.substr(dxdepos + 14, 10);
	std::string scsign =   lineToProcess.substr(dxdepos + 26, 10);
	std::string stimeZ =   lineToProcess.substr(dxdepos + 70,  4);

	if (sfreqkHz.empty() || scsign.empty() || stimeZ.empty())
	{
#ifdef DEBUG
		OutputDebugStringA("SDRuno process line error");
#endif
		return;
	}

	// remove whitespace
	sfreqkHz.erase(std::remove_if(sfreqkHz.begin(), sfreqkHz.end(), ::isspace), sfreqkHz.end());
	scsign.erase(std::remove_if(scsign.begin(), scsign.end(), ::isspace), scsign.end());

	double dfreqkHz;
	long long llfreqHz;
	try
	{
		dfreqkHz = stod(sfreqkHz);
		llfreqHz = (long long)(dfreqkHz * 1000);
	}
	catch (...)
	{
		return;
	}

	if (head == NULL)
	{
		add_DXEntry(scsign, stimeZ, llfreqHz);
	}
	else
	{
		prev = NULL;
		current = head;

		while (current != NULL)
		{
			// remove duplicate callsign entries or
			// entries older than timer (in mins)
			if (scsign.compare(current->callsign) == 0 || olderThanTimer(current->timeUTC)) 
			{
				if (prev == NULL)
				{
					head = current->next;
				}
				else
				{
					prev->next = current->next;
				}
			}
			prev = current;
			if (current->next != NULL)
			{
				current = current->next;
			}
			else
			{
				tail = current;
				break;
			}
		}

		add_DXEntry(scsign, stimeZ, llfreqHz);
	}
}

void SDRunoPlugin_DXCluster::add_DXEntry(std::string callsign, std::string t, long long freq)
{
	DXEntry *tmp = new DXEntry;
	tmp->callsign = callsign;
	tmp->freq = freq;
	tmp->timeUTC = t;
	tmp->next = NULL;

	if (head == NULL)
	{
		head = tmp;
		tail = tmp;
	}
	else
	{
		tail->next = tmp;
		tail = tail->next;
	}
}

bool SDRunoPlugin_DXCluster::olderThanTimer(std::string timeUTC)
{
	std::string shour = timeUTC.substr(0, 2);
	std::string smin = timeUTC.substr(2, 2);
	int ihour = stoi(shour);
	int imin = stoi(smin);
	int totalMins = (ihour * 60) + imin;

	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	time_t tt = std::chrono::system_clock::to_time_t(now);
	tm utc_tm = *gmtime(&tt);
	int totalMinsUTC = (utc_tm.tm_hour * 60) + utc_tm.tm_min;

	// TODO: cope for 24 -> 0 hour case

	if (totalMinsUTC - totalMins > timeMinutes)
	{
		return true;
	}

	return false;
}

void SDRunoPlugin_DXCluster::UpdateSampleRate()
{
	sampleRate = (int)m_controller.GetSampleRate(0);
}