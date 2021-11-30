#include <sstream>

#include "ADSBpluginForm.h"
#include "ADSBpluginSettingsDialog.h"
#include "ADSBpluginUi.h"
#include "resource.h"
#include <ShlObj.h>

#define VERSION "V1.2"

ADSBpluginForm::ADSBpluginForm(ADSBpluginUi& parent, IUnoPluginController& controller) :
	nana::form(nana::API::make_center(formWidth, formHeight), nana::appearance(false, true, false, false, true, false, false)),
	m_parent(parent),
	m_controller(controller),
	m_clearRequired(false),
	m_updateRequired(false)
{
	LoadSettings();
	LoadPosition();
	Setup();
}

ADSBpluginForm::~ADSBpluginForm()
{
	//
}

void ADSBpluginForm::SetHttpPorts(const std::string& ports)
{
	m_parent.http_ports = ports;
	m_controller.SetConfigurationKey("ADSB.HTTP", m_parent.http_ports);
}

void ADSBpluginForm::SetRawPorts(const std::string& ports)
{
	m_parent.raw_ports = ports;
	m_controller.SetConfigurationKey("ADSB.RAW", m_parent.raw_ports);
}

void ADSBpluginForm::SetSbsPorts(const std::string& ports)
{
	m_parent.sbs_ports = ports;
	m_controller.SetConfigurationKey("ADSB.SBS", m_parent.sbs_ports);
}

void ADSBpluginForm::SetBeastPorts(const std::string& ports)
{
	m_parent.beast_ports = ports;
	m_controller.SetConfigurationKey("ADSB.BEAST", m_parent.beast_ports);
}

std::string ADSBpluginForm::GetHttpPorts()
{
	return m_parent.http_ports;
}

std::string ADSBpluginForm::GetRawPorts()
{
	return m_parent.raw_ports;
}

std::string ADSBpluginForm::GetSbsPorts()
{
	return m_parent.sbs_ports;
}

std::string ADSBpluginForm::GetBeastPorts()
{
	return m_parent.beast_ports;
}

int ADSBpluginForm::GetSettingsX()
{
	return SettingsX;
}

int ADSBpluginForm::GetSettingsY()
{
	return SettingsY;
}

void ADSBpluginForm::SetSettingsX(int x)
{
	SettingsX = x;
}

void ADSBpluginForm::SetSettingsY(int y)
{
	SettingsY = y;
}

bool ADSBpluginForm::GetOverSample()
{
	return m_parent.GetOverSample();
}

void ADSBpluginForm::SetOverSample(bool en)
{
	m_parent.SetOversample(en);
	int tmp = (GetOverSample()) ? 1 : 0;
	m_controller.SetConfigurationKey("ADSB.Oversample", std::to_string(tmp));
}

void ADSBpluginForm::LoadSettings()
{
	std::string tmp;
	m_controller.GetConfigurationKey("ADSB.HTTP", tmp);
	if (tmp.empty())
	{
		m_parent.http_ports = "8080";
	}
	else
	{
		m_parent.http_ports = tmp;
	}

	m_controller.GetConfigurationKey("ADSB.RAW", tmp);
	if (tmp.empty())
	{
		m_parent.raw_ports = "30002";
	}
	else
	{
		m_parent.raw_ports = tmp;
	}

	m_controller.GetConfigurationKey("ADSB.SBS", tmp);
	if (tmp.empty())
	{
		m_parent.sbs_ports = "30003";
	}
	else
	{
		m_parent.sbs_ports = tmp;
	}

	m_controller.GetConfigurationKey("ADSB.BEAST", tmp);
	if (tmp.empty())
	{
		m_parent.beast_ports = "30005";
	}
	else
	{
		m_parent.beast_ports = tmp;
	}

	SetOverSample(GetOversampleSetting());
}

void ADSBpluginForm::SavePosition()
{
	m_controller.SetConfigurationKey("ADSB.X", std::to_string(posX));
	m_controller.SetConfigurationKey("ADSB.Y", std::to_string(posY));
	m_controller.SetConfigurationKey("ADSB.SettingsX", std::to_string(SettingsX));
	m_controller.SetConfigurationKey("ADSB.SettingsY", std::to_string(SettingsY));
}

void ADSBpluginForm::LoadPosition()
{
	std::string tmp;
	m_controller.GetConfigurationKey("ADSB.SettingsX", tmp); 
	if (tmp.empty())
	{
		SettingsX = -1;
	}
	else
	{
		SettingsX = stoi(tmp);
	}

	m_controller.GetConfigurationKey("ADSB.SettingsY", tmp);
	if (tmp.empty())
	{
		SettingsY = -1;
	}
	else
	{
		SettingsY = stoi(tmp);
	}
}

void ADSBpluginForm::Run()
{
	show();
	nana::exec();
}

void ADSBpluginForm::GetStoredValues(int &x, int &y)
{
	int localX, localY;
	std::string tmp;

	m_controller.GetConfigurationKey("ADSB.X", tmp);
	if (tmp.empty())
	{
		localX = -1;
	}
	else
	{
		localX = stoi(tmp);
	}

	m_controller.GetConfigurationKey("ADSB.Y", tmp);
	if (tmp.empty())
	{
		localY = -1;
	}
	else
	{
		localY = stoi(tmp);
	}

	x = localX;
	y = localY;
}

void ADSBpluginForm::Setup()
{
	int storedX, storedY;
	GetStoredValues(storedX, storedY);
	auto s = nana::screen();
	auto wa = s.from_window(nana::window(this)).workarea();
	if (storedX == -1)
	{
		storedX = (wa.width / 2);
	}
	posX = storedX;
	if (storedY == -1)
	{
		storedY = (wa.height / 2);
	}
	posY = storedY;
	this->move(posX, posY);

	nana::paint::image img_border;
	nana::paint::image img_inner;
	HMODULE hModule = NULL;
	HRSRC rc_border = NULL;
	HRSRC rc_inner = NULL;
	HRSRC rc_close = NULL;
	HRSRC rc_close_over = NULL;
	HRSRC rc_min = NULL;
	HRSRC rc_min_over = NULL;
	HRSRC rc_bar = NULL;
	HRSRC rc_sett = NULL;
	HRSRC rc_sett_over = NULL;
	HRSRC rc_help = NULL;
	HRSRC rc_help_over = NULL;

	HBITMAP bm_border = NULL;
	HBITMAP bm_inner = NULL;
	HBITMAP bm_close = NULL;
	HBITMAP bm_close_over = NULL;
	HBITMAP bm_min = NULL;
	HBITMAP bm_min_over = NULL;
	HBITMAP bm_bar = NULL;
	HBITMAP bm_sett = NULL;
	HBITMAP bm_sett_over = NULL;
	HBITMAP bm_help = NULL;
	HBITMAP bm_help_over = NULL;

	BITMAPINFO bmInfo_border = { 0 };
	BITMAPINFO bmInfo_inner = { 0 };
	BITMAPINFO bmInfo_close = { 0 };
	BITMAPINFO bmInfo_close_over = { 0 };
	BITMAPINFO bmInfo_min = { 0 };
	BITMAPINFO bmInfo_min_over = { 0 };
	BITMAPINFO bmInfo_bar = { 0 };
	BITMAPINFO bmInfo_sett = { 0 };
	BITMAPINFO bmInfo_sett_over = { 0 };
	BITMAPINFO bmInfo_help = { 0 };
	BITMAPINFO bmInfo_help_over = { 0 };

	BITMAPFILEHEADER borderHeader = { 0 };
	BITMAPFILEHEADER innerHeader = { 0 };
	BITMAPFILEHEADER closeHeader = { 0 };
	BITMAPFILEHEADER closeoverHeader = { 0 };
	BITMAPFILEHEADER minHeader = { 0 };
	BITMAPFILEHEADER minoverHeader = { 0 };
	BITMAPFILEHEADER barHeader = { 0 };
	BITMAPFILEHEADER settHeader = { 0 };
	BITMAPFILEHEADER settoverHeader = { 0 };
	BITMAPFILEHEADER helpHeader = { 0 };
	BITMAPFILEHEADER helpoverHeader = { 0 };

	HDC hdc = NULL;

	BYTE* borderPixels = NULL;
	BYTE* innerPixels = NULL;
	BYTE* closePixels = NULL;
	BYTE* closeoverPixels = NULL;
	BYTE* minPixels = NULL;
	BYTE* minoverPixels = NULL;
	BYTE* barPixels = NULL;
	BYTE* barfocusedPixels = NULL;
	BYTE* settPixels = NULL;
	BYTE* settoverPixels = NULL;
	BYTE* helpPixels = NULL;
	BYTE* helpoverPixels = NULL;

	const unsigned int rawDataOffset = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);

	hModule = GetModuleHandle(L"SDRunoPlugin_ADSB");
	hdc = GetDC(NULL);

	rc_border = FindResource(hModule, MAKEINTRESOURCE(IDB_BG_BORDER), RT_BITMAP);
	rc_inner = FindResource(hModule, MAKEINTRESOURCE(IDB_BACKGROUND), RT_BITMAP);
	rc_close = FindResource(hModule, MAKEINTRESOURCE(IDB_CLOSE), RT_BITMAP);
	rc_close_over = FindResource(hModule, MAKEINTRESOURCE(IDB_CLOSE_DOWN), RT_BITMAP);
	rc_min = FindResource(hModule, MAKEINTRESOURCE(IDB_MIN), RT_BITMAP);
	rc_min_over = FindResource(hModule, MAKEINTRESOURCE(IDB_MIN_DOWN), RT_BITMAP);
	rc_bar = FindResource(hModule, MAKEINTRESOURCE(IDB_HEADER), RT_BITMAP);
	rc_sett = FindResource(hModule, MAKEINTRESOURCE(IDB_SETT), RT_BITMAP);
	rc_sett_over = FindResource(hModule, MAKEINTRESOURCE(IDB_SETT_DOWN), RT_BITMAP);
	rc_help = FindResource(hModule, MAKEINTRESOURCE(IDB_HELP), RT_BITMAP);
	rc_help_over = FindResource(hModule, MAKEINTRESOURCE(IDB_HELP_DOWN), RT_BITMAP);

	bm_border = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_BG_BORDER), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_inner = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_BACKGROUND), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_close = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_CLOSE), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_close_over = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_CLOSE_DOWN), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_min = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_MIN), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_min_over = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_MIN_DOWN), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_bar = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_HEADER), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_sett = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_SETT), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_sett_over = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_SETT_DOWN), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_help = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_HELP), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_help_over = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_HELP_DOWN), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);

	bmInfo_border.bmiHeader.biSize = sizeof(bmInfo_border.bmiHeader);
	bmInfo_inner.bmiHeader.biSize = sizeof(bmInfo_inner.bmiHeader);
	bmInfo_close.bmiHeader.biSize = sizeof(bmInfo_close.bmiHeader);
	bmInfo_close_over.bmiHeader.biSize = sizeof(bmInfo_close_over.bmiHeader);
	bmInfo_min.bmiHeader.biSize = sizeof(bmInfo_min.bmiHeader);
	bmInfo_min_over.bmiHeader.biSize = sizeof(bmInfo_min_over.bmiHeader);
	bmInfo_bar.bmiHeader.biSize = sizeof(bmInfo_bar.bmiHeader);
	bmInfo_sett.bmiHeader.biSize = sizeof(bmInfo_sett.bmiHeader);
	bmInfo_sett_over.bmiHeader.biSize = sizeof(bmInfo_sett_over.bmiHeader);
	bmInfo_help.bmiHeader.biSize = sizeof(bmInfo_help.bmiHeader);
	bmInfo_help_over.bmiHeader.biSize = sizeof(bmInfo_help_over.bmiHeader);

	GetDIBits(hdc, bm_border, 0, 0, NULL, &bmInfo_border, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_inner, 0, 0, NULL, &bmInfo_inner, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_close, 0, 0, NULL, &bmInfo_close, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_close_over, 0, 0, NULL, &bmInfo_close_over, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_min, 0, 0, NULL, &bmInfo_min, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_min_over, 0, 0, NULL, &bmInfo_min_over, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_bar, 0, 0, NULL, &bmInfo_bar, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_sett, 0, 0, NULL, &bmInfo_sett, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_sett_over, 0, 0, NULL, &bmInfo_sett_over, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_help, 0, 0, NULL, &bmInfo_help, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_help_over, 0, 0, NULL, &bmInfo_help_over, DIB_RGB_COLORS);

	bmInfo_border.bmiHeader.biCompression = BI_RGB;
	bmInfo_inner.bmiHeader.biCompression = BI_RGB;
	bmInfo_close.bmiHeader.biCompression = BI_RGB;
	bmInfo_close_over.bmiHeader.biCompression = BI_RGB;
	bmInfo_min.bmiHeader.biCompression = BI_RGB;
	bmInfo_min_over.bmiHeader.biCompression = BI_RGB;
	bmInfo_bar.bmiHeader.biCompression = BI_RGB;
	bmInfo_sett.bmiHeader.biCompression = BI_RGB;
	bmInfo_sett_over.bmiHeader.biCompression = BI_RGB;
	bmInfo_help.bmiHeader.biCompression = BI_RGB;
	bmInfo_help_over.bmiHeader.biCompression = BI_RGB;

	borderHeader.bfOffBits = rawDataOffset;
	borderHeader.bfSize = bmInfo_border.bmiHeader.biSizeImage;
	borderHeader.bfType = 0x4D42;

	innerHeader.bfOffBits = rawDataOffset;
	innerHeader.bfSize = bmInfo_inner.bmiHeader.biSizeImage;
	innerHeader.bfType = 0x4D42;

	closeHeader.bfOffBits = rawDataOffset;
	closeHeader.bfSize = bmInfo_close.bmiHeader.biSizeImage;
	closeHeader.bfType = 0x4D42;

	closeoverHeader.bfOffBits = rawDataOffset;
	closeoverHeader.bfSize = bmInfo_close_over.bmiHeader.biSizeImage;
	closeoverHeader.bfType = 0x4D42;

	minHeader.bfOffBits = rawDataOffset;
	minHeader.bfSize = bmInfo_min.bmiHeader.biSizeImage;
	minHeader.bfType = 0x4D42;

	minoverHeader.bfOffBits = rawDataOffset;
	minoverHeader.bfSize = bmInfo_min_over.bmiHeader.biSizeImage;
	minoverHeader.bfType = 0x4D42;

	barHeader.bfOffBits = rawDataOffset;
	barHeader.bfSize = bmInfo_bar.bmiHeader.biSizeImage;
	barHeader.bfType = 0x4D42;

	settHeader.bfOffBits = rawDataOffset;
	settHeader.bfSize = bmInfo_sett.bmiHeader.biSizeImage;
	settHeader.bfType = 0x4D42;

	settoverHeader.bfOffBits = rawDataOffset;
	settoverHeader.bfSize = bmInfo_sett_over.bmiHeader.biSizeImage;
	settoverHeader.bfType = 0x4D42;

	helpHeader.bfOffBits = rawDataOffset;
	helpHeader.bfSize = bmInfo_help.bmiHeader.biSizeImage;
	helpHeader.bfType = 0x4D42;

	helpoverHeader.bfOffBits = rawDataOffset;
	helpoverHeader.bfSize = bmInfo_help_over.bmiHeader.biSizeImage;
	helpoverHeader.bfType = 0x4D42;

	borderPixels = new BYTE[bmInfo_border.bmiHeader.biSizeImage + rawDataOffset];
	innerPixels = new BYTE[bmInfo_inner.bmiHeader.biSizeImage + rawDataOffset];
	closePixels = new BYTE[bmInfo_close.bmiHeader.biSizeImage + rawDataOffset];
	closeoverPixels = new BYTE[bmInfo_close_over.bmiHeader.biSizeImage + rawDataOffset];
	minPixels = new BYTE[bmInfo_min.bmiHeader.biSizeImage + rawDataOffset];
	minoverPixels = new BYTE[bmInfo_min_over.bmiHeader.biSizeImage + rawDataOffset];
	barPixels = new BYTE[bmInfo_bar.bmiHeader.biSizeImage + rawDataOffset];
	settPixels = new BYTE[bmInfo_sett.bmiHeader.biSizeImage + rawDataOffset];
	settoverPixels = new BYTE[bmInfo_sett_over.bmiHeader.biSizeImage + rawDataOffset];
	helpPixels = new BYTE[bmInfo_help.bmiHeader.biSizeImage + rawDataOffset];
	helpoverPixels = new BYTE[bmInfo_help_over.bmiHeader.biSizeImage + rawDataOffset];

	*(BITMAPFILEHEADER*)borderPixels = borderHeader;
	*(BITMAPINFO*)(borderPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_border;

	*(BITMAPFILEHEADER*)innerPixels = innerHeader;
	*(BITMAPINFO*)(innerPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_inner;

	*(BITMAPFILEHEADER*)closePixels = closeHeader;
	*(BITMAPINFO*)(closePixels + sizeof(BITMAPFILEHEADER)) = bmInfo_close;

	*(BITMAPFILEHEADER*)closeoverPixels = closeoverHeader;
	*(BITMAPINFO*)(closeoverPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_close_over;

	*(BITMAPFILEHEADER*)minPixels = minHeader;
	*(BITMAPINFO*)(minPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_min;

	*(BITMAPFILEHEADER*)minoverPixels = minoverHeader;
	*(BITMAPINFO*)(minoverPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_min_over;

	*(BITMAPFILEHEADER*)barPixels = barHeader;
	*(BITMAPINFO*)(barPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_bar;

	*(BITMAPFILEHEADER*)settPixels = settHeader;
	*(BITMAPINFO*)(settPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_sett;

	*(BITMAPFILEHEADER*)settoverPixels = settoverHeader;
	*(BITMAPINFO*)(settoverPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_sett_over;

	*(BITMAPFILEHEADER*)helpPixels = helpHeader;
	*(BITMAPINFO*)(helpPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_help;

	*(BITMAPFILEHEADER*)helpoverPixels = helpoverHeader;
	*(BITMAPINFO*)(helpoverPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_help_over;

	GetDIBits(hdc, bm_border, 0, bmInfo_border.bmiHeader.biHeight, (LPVOID)(borderPixels + rawDataOffset), &bmInfo_border, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_inner, 0, bmInfo_inner.bmiHeader.biHeight, (LPVOID)(innerPixels + rawDataOffset), &bmInfo_inner, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_close, 0, bmInfo_close.bmiHeader.biHeight, (LPVOID)(closePixels + rawDataOffset), &bmInfo_close, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_close_over, 0, bmInfo_close_over.bmiHeader.biHeight, (LPVOID)(closeoverPixels + rawDataOffset), &bmInfo_close_over, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_min, 0, bmInfo_min.bmiHeader.biHeight, (LPVOID)(minPixels + rawDataOffset), &bmInfo_min, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_min_over, 0, bmInfo_min_over.bmiHeader.biHeight, (LPVOID)(minoverPixels + rawDataOffset), &bmInfo_min_over, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_bar, 0, bmInfo_bar.bmiHeader.biHeight, (LPVOID)(barPixels + rawDataOffset), &bmInfo_bar, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_sett, 0, bmInfo_sett.bmiHeader.biHeight, (LPVOID)(settPixels + rawDataOffset), &bmInfo_sett, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_sett_over, 0, bmInfo_sett_over.bmiHeader.biHeight, (LPVOID)(settoverPixels + rawDataOffset), &bmInfo_sett_over, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_help, 0, bmInfo_help.bmiHeader.biHeight, (LPVOID)(helpPixels + rawDataOffset), &bmInfo_help, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_help_over, 0, bmInfo_help_over.bmiHeader.biHeight, (LPVOID)(helpoverPixels + rawDataOffset), &bmInfo_help_over, DIB_RGB_COLORS);

	img_border.open(borderPixels, bmInfo_border.bmiHeader.biSizeImage);
	img_inner.open(innerPixels, bmInfo_inner.bmiHeader.biSizeImage);

	//Load button state bitmaps into the images
	img_close_normal.open(closePixels, bmInfo_close.bmiHeader.biSizeImage);
	img_close_down.open(closeoverPixels, bmInfo_close_over.bmiHeader.biSizeImage);
	img_min_normal.open(minPixels, bmInfo_min.bmiHeader.biSizeImage);
	img_min_down.open(minoverPixels, bmInfo_min_over.bmiHeader.biSizeImage);
	img_header.open(barPixels, bmInfo_bar.bmiHeader.biSizeImage);
	img_sett_normal.open(settPixels, bmInfo_sett.bmiHeader.biSizeImage);
	img_sett_down.open(settoverPixels, bmInfo_sett_over.bmiHeader.biSizeImage);
	img_help_normal.open(helpPixels, bmInfo_help.bmiHeader.biSizeImage);
	img_help_down.open(helpoverPixels, bmInfo_help_over.bmiHeader.biSizeImage);

	ReleaseDC(NULL, hdc);

	bg_border.load(img_border, nana::rectangle(0, 0, 590, 340));
	bg_border.stretchable(0, 0, 0, 0);
	bg_border.transparent(true);
	bg_inner.load(img_inner, nana::rectangle(0, 0, 582, 299));
	bg_inner.stretchable(sideBorderWidth, 0, sideBorderWidth, bottomBarHeight);
	bg_inner.transparent(false);

	caption("SDRuno ADSB Plugin");

	events().destroy([&] { m_parent.FormClosed(); });

	events().move([&](const nana::arg_move& mov) {
		posX = mov.x;
		posY = mov.y;
	});

	//Initialize header bar
	header_bar.size(nana::size(122, 20));
	header_bar.load(img_header, nana::rectangle(0, 0, 122, 20));
	header_bar.stretchable(0, 0, 0, 0);
	header_bar.move(nana::point((formWidth / 2) - 61, 5));
	header_bar.transparent(true);

	//Initial header text 
	title_bar_label.size(nana::size(65, 12));
	title_bar_label.move(nana::point((formWidth / 2) - 5, 9));
	title_bar_label.format(true);
	title_bar_label.caption("< bold size = 6 color = 0x000000 font = \"Verdana\">ADSB PLUGIN</>");
	title_bar_label.text_align(nana::align::center, nana::align_v::center);
	title_bar_label.fgcolor(nana::color_rgb(0x000000));
	title_bar_label.transparent(true);

	//Iniitialize drag_label
	form_drag_label.move(nana::point(0, 0));
	form_drag_label.transparent(true);

	//Initialize dragger and set target to form, and trigger to drag_label 
	form_dragger.target(*this);
	form_dragger.trigger(form_drag_label);

	//Initialize "Minimize button"
	min_button.load(img_min_normal, nana::rectangle(0, 0, 20, 15));
	min_button.bgcolor(nana::color_rgb(0x000000));
	min_button.move(nana::point(formWidth - 51, 9));
	min_button.transparent(true);
	min_button.events().mouse_down([&] { min_button.load(img_min_down, nana::rectangle(0, 0, 20, 15)); });
	min_button.events().mouse_up([&] { min_button.load(img_min_normal, nana::rectangle(0, 0, 20, 15));
	nana::API::zoom_window(this->handle(), false); });
	min_button.events().mouse_leave([&] { min_button.load(img_min_normal, nana::rectangle(0, 0, 20, 15)); });

	//Initialize "Close button"
	close_button.load(img_close_normal, nana::rectangle(0, 0, 20, 15));
	close_button.bgcolor(nana::color_rgb(0x000000));
	close_button.move(nana::point(formWidth - 26, 9));
	close_button.transparent(true);
	close_button.events().mouse_down([&] { close_button.load(img_close_down, nana::rectangle(0, 0, 20, 15)); });
	close_button.events().mouse_up([&] { close_button.load(img_close_normal, nana::rectangle(0, 0, 20, 15)); close(); });
	close_button.events().mouse_leave([&] { close_button.load(img_close_normal, nana::rectangle(0, 0, 20, 15)); });

	sett_button.load(img_sett_normal, nana::rectangle(0, 0, 40, 15));
	sett_button.bgcolor(nana::color_rgb(0x000000));
	sett_button.move(nana::point(10, 9));
	sett_button.events().mouse_down([&] { sett_button.load(img_sett_down, nana::rectangle(0, 0, 40, 15)); });
	sett_button.events().mouse_up([&] { sett_button.load(img_sett_normal, nana::rectangle(0, 0, 40, 15)); SettingsButton_Click(); });
	sett_button.events().mouse_leave([&] { sett_button.load(img_sett_normal, nana::rectangle(0, 0, 40, 15)); });
	sett_button.tooltip("Show settings window");
	sett_button.transparent(true);

	help_button.load(img_help_normal, nana::rectangle(0, 0, 40, 15));
	help_button.bgcolor(nana::color_rgb(0x000000));
	help_button.move(nana::point(55, 9));
	help_button.events().mouse_down([&] { help_button.load(img_help_down, nana::rectangle(0, 0, 40, 15)); });
	help_button.events().mouse_up([&] { help_button.load(img_help_normal, nana::rectangle(0, 0, 40, 15)); HelpButton_Click(); });
	help_button.events().mouse_leave([&] { help_button.load(img_help_normal, nana::rectangle(0, 0, 40, 15)); });
	help_button.tooltip("Open Help Document");
	help_button.transparent(true);

	versionLbl.fgcolor(nana::colors::white);
	versionLbl.caption(VERSION);
	versionLbl.transparent(true);
	
	// Main form UI starts here

	m_startButton.caption("Start");
	m_startButton.enabled(m_controller.IsStreamingEnabled(0));
	m_startButton.edge_effects(false);
	m_startButton.events().click([&] { m_parent.StartButtonClicked(); });

	m_aircraftLb.append_header("Aircraft", formWidth - 55);
	m_aircraftLb.show_header(false);
	m_aircraftLb.bgcolor(nana::color_rgb(0x163240));
	m_aircraftLb.fgcolor(nana::colors::white);
	m_aircraftLb.enable_single(true, false);
	m_aircraftLb.auto_draw(true);
	m_aircraftLb.typeface(nana::paint::font("Courier New", 9, nana::detail::font_style(0, false, false, false)));

	m_updateTimer.interval(std::chrono::milliseconds(500));
	m_updateTimer.elapse([&] {
			TimerTick();
		});
	m_updateTimer.start();
}

void ADSBpluginForm::SettingsButton_Click()
{
	//Create a new settings dialog object
	ADSBpluginSettingsDialog settingsDialog{ *this/*, m_controller*/ };

	//disable this form so settings dialog retains top level focus
	this->enabled(false);

	//Attach a handler to the settings dialog close event
	settingsDialog.events().unload([&] { SettingsDialog_Closed(); });

	//Show the setttings dialog
	settingsDialog.Run();
}

void ADSBpluginForm::HelpButton_Click()
{
	char selfdir[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, selfdir, MAX_PATH);
	std::string csPath(selfdir);
	
	std::size_t index = csPath.find_last_of('\\');
	std::string str = csPath.substr(0, index);
	str.append("\\Plugins\\ADSBPluginHelp.pdf");

	ShellExecuteA(NULL, "open", str.c_str(), NULL, NULL, SW_SHOW);
}

void ADSBpluginForm::SettingsDialog_Closed()
{
	//DO NOT REMOVE THE FLLOWING CODE it is required for the proper operation of the settings dialog form
	//ADSBpluginSettingsDialog m_settings{ *this, m_controller };

	this->enabled(true);
	this->focus();

	//TODO: Extra code goes here to be preformed when settings dialog form closes
}

void ADSBpluginForm::SetStartButtonState(bool state)
{
	m_startButton.enabled(state);
}

bool ADSBpluginForm::GetStartButtonState()
{
	return m_startButton.enabled();
}

void ADSBpluginForm::SetStartButtonCaption(std::string text)
{
	m_startButton.caption(text);
}

bool ADSBpluginForm::GetOversampleSetting()
{
	bool setting;
	std::string tmp;

	m_controller.GetConfigurationKey("ADSB.Oversample", tmp);
	if (tmp.empty())
	{
		setting = true;
	}
	else
	{
		int val = stoi(tmp);
		if (val == 1)
		{
			setting = true;
		}
		else
		{
			setting = false;
		}
	}
	return setting;
}

void ADSBpluginForm::ClearDisplay()
{
	std::lock_guard<std::mutex> l(m_updateLock);

	m_updateItems.clear();
	m_clearRequired = true;
	m_updateRequired = false;
}

void ADSBpluginForm::PutDisplayString(char* str)
{
	if (strlen(str) > 5)
	{
		std::lock_guard<std::mutex> l(m_updateLock);
		m_updateItems.push_back(std::string(str));
		m_updateRequired = true;
	}
}

void ADSBpluginForm::TimerTick()
{
	std::lock_guard<std::mutex> l(m_updateLock);

	if (m_clearRequired)
	{
		m_aircraftLb.clear();
		m_clearRequired = false;
	}

	if (m_updateRequired)
	{
		auto cat = m_aircraftLb.at(0);
		while (!m_updateItems.empty())
		{
			auto front = m_updateItems.front();
			m_updateItems.pop_front();
			cat.push_back(front);
		}
		m_updateRequired = false;
	}
}
