#include <windows.h>
#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <filesystem>
#include <set>
#include <setupapi.h>      // SetupDi* like APIs
#include <devguid.h>       // GUID_DEVCLASS_MONITOR
#pragma comment(lib, "setupapi.lib")

void PrintUsage(const std::wstring& exeName) {
    std::wcout << L"\nUsage:\n";
    std::wcout << L"  " << exeName << L" [<display>] -w <width> -h <height> [-r <refresh>]\n";
    std::wcout << L"  " << exeName << L" [<display>] --width <width> --height <height> [--refresh <refresh>]\n";
    std::wcout << L"  " << exeName << L" [<display>] /x:<width> /y:<height> [/r:<refresh>]\n";
    std::wcout << L"  " << exeName << L" --list\n";
    std::wcout << L"  " << exeName << L" --modes <display>\n";
    std::wcout << L"  " << exeName << L" --test <display> <width> <height> [<refresh>]\n";
    std::wcout << L"  " << exeName << L" --help\n\n";
    std::wcout << L"Example:\n";
    std::wcout << L"  " << exeName << L" 2 -w 2560 -h 1440 -r 165\n";
    std::wcout << L"  " << exeName << L" \\\\.\\DISPLAY2 -w 3840 -h 2160 -r 160\n";
    std::wcout << L"  " << exeName << L" /x:2560 /y:1440\n\n";
    std::wcout << L"If no display is specified, the setting will be applied to the display under your cursor.\n\n";
}

// return the "\.\DISPLAYx" device name for the monitor currently under the cursor
std::wstring GetDisplayFromCursor()
{
    POINT pt; GetCursorPos(&pt);
    HMONITOR hMon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    MONITORINFOEXW mi{ sizeof(mi) };
    if (GetMonitorInfoW(hMon, &mi))
        return mi.szDevice;            // already in "\.\DISPLAY#" format
    return L"\\\\.\\DISPLAY1";    // safe fallback
}

std::wstring NormalizeDisplayName(const std::wstring& input) {
    if (input.find(L"DISPLAY") == std::wstring::npos) {
        return L"\\\\.\\DISPLAY" + input;
    }
    return input;
}

std::wstring QueryModelName(const std::wstring& deviceInstanceId) {
    std::wstring result;
    std::wstring hardwareId;
    size_t firstSlash = deviceInstanceId.find(L'\\');
    size_t secondSlash = deviceInstanceId.find(L'\\', firstSlash + 1);
    if (firstSlash != std::wstring::npos && secondSlash != std::wstring::npos) {
        std::wstring buf = deviceInstanceId.substr(firstSlash + 1, secondSlash - firstSlash - 1); // e.g., DEL40A1
        hardwareId = L"MONITOR\\";
        hardwareId += buf;
    }
    else {
        return L"";
    }

	// 1) get information of monitor class devices only that are present
    HDEVINFO hDevInfo = SetupDiGetClassDevs(
		&GUID_DEVCLASS_MONITOR,   // Monitor Class GUID
        nullptr,                  // Enumerator(unused)
        nullptr,                  // HWND(nullptr because of console app)
		DIGCF_PRESENT             // only currently present devices
    );

    if (hDevInfo == INVALID_HANDLE_VALUE) {
        return L"";
    }

    SP_DEVINFO_DATA devInfo{};
    devInfo.cbSize = sizeof(devInfo);

	// 2) extracting the HardwareID of each device while iterating through the set
    for (DWORD index = 0; SetupDiEnumDeviceInfo(hDevInfo, index, &devInfo); ++index)
    {
        WCHAR buf[256];
        if (SetupDiGetDeviceRegistryPropertyW(
            hDevInfo,
            &devInfo,
            SPDRP_HARDWAREID,
            nullptr,
            reinterpret_cast<PBYTE>(buf),
            sizeof(buf),
            nullptr))
        {
			WCHAR friendlyName[256];
			if (buf == hardwareId) {
				// 3) extract FriendlyName
				if (SetupDiGetDeviceRegistryPropertyW(
					hDevInfo,
					&devInfo,
					SPDRP_FRIENDLYNAME,
					nullptr,
					reinterpret_cast<PBYTE>(friendlyName),
					sizeof(friendlyName),
					nullptr))
				{
					result = friendlyName;
				} else {
                    result = hardwareId;
				}
				break; // 4) loop break
			}
        }
        
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
    return result;
}

void ListDisplays() {
    DISPLAY_DEVICEW dd = { sizeof(dd) };
    DISPLAY_DEVICEW monitor = { sizeof(monitor) };
    int i = 0;

    std::wcout << L"\nAvailable Displays:\n";
    std::wcout << L"\n";

    while (EnumDisplayDevicesW(nullptr, i, &dd, 0)) {
        if (dd.StateFlags & DISPLAY_DEVICE_ACTIVE) {
            std::wstring deviceName = dd.DeviceName;
            size_t pos = deviceName.find(L"DISPLAY");
            std::wstring displayNum = (pos != std::wstring::npos) ? deviceName.substr(pos + 7) : L"?";
            std::wcout << L"[DISPLAY" << displayNum << L"]\n";
            std::wcout << L"  Device Name  : " << dd.DeviceName << L"\n";
            std::wcout << L"  Location     : " << dd.DeviceString << L"\n";

            if (EnumDisplayDevicesW(dd.DeviceName, 0, &monitor, 0)) {
                std::wstring instanceId = monitor.DeviceID;
                std::wstring modelName = QueryModelName(instanceId);
                std::wcout << L"  Monitor Name : " << modelName;
                if (modelName.empty()) {
                    std::wcout << monitor.DeviceString;
                }
                std::wcout << L"\n";
            }

            std::wcout << L"\n";
        }
        ++i;
    }
}

void ListModes(const std::wstring& inputDisplay) {
    std::wstring device = NormalizeDisplayName(inputDisplay);

    // current display mode
    DEVMODE current = {};
    current.dmSize = sizeof(DEVMODE);
    if (!EnumDisplaySettingsW(device.c_str(), ENUM_CURRENT_SETTINGS, &current)) {
        std::wcerr << L"[!] Failed to get current display mode.\n";
    }

    std::map<std::pair<int, int>, std::set<int>> resMap;
    std::pair<int, int> currentRes = { current.dmPelsWidth, current.dmPelsHeight };
    int currentHz = current.dmDisplayFrequency;

    DEVMODE mode = {};
    mode.dmSize = sizeof(DEVMODE);
    int iMode = 0;

    while (EnumDisplaySettingsW(device.c_str(), iMode++, &mode)) {
        auto key = std::make_pair(mode.dmPelsWidth, mode.dmPelsHeight);
        resMap[key].insert(mode.dmDisplayFrequency);
    }

    std::wcout << L"\nAvailable Modes for " << device << L":\n";
    for (const auto& [res, hzs] : resMap) {
        std::wcout << L"  " << res.first << L"x" << res.second << L" @";
        for (int hz : hzs) {
            std::wcout << L" " << hz << L"Hz";
            if (res == currentRes && hz == currentHz) {
                std::wcout << L"*";
            }
        }
        std::wcout << L"\n";
    }
}

void TestDisplayResolution(const std::wstring& deviceName, int width, int height, int refresh)
{
    DEVMODE mode = {};
    mode.dmSize = sizeof(DEVMODE);
    mode.dmPelsWidth = width;
    mode.dmPelsHeight = height;
    if (refresh > 0) {
        mode.dmDisplayFrequency = refresh;
        mode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
    }
    else {
        mode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
    }

    LONG result = ChangeDisplaySettingsExW(deviceName.c_str(), &mode, nullptr, CDS_TEST, nullptr);
    if (result == DISP_CHANGE_SUCCESSFUL) {
        std::wcout << L"[+] " << deviceName << L" " << width << L"x" << height;
        if (refresh > 0) std::wcout << L" @" << refresh << L"Hz";
        std::wcout << L" is supported.";
    }
    else {
        std::wcerr << L"[!] NOT supported on " << deviceName << L" (Error code: " << result << L")";
    }
}

bool SetDisplayResolution(const std::wstring& deviceName, int width, int height, int refreshRate) {
    DEVMODE devMode = {};
    devMode.dmSize = sizeof(DEVMODE);
    devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

    if (refreshRate > 0) {
        devMode.dmDisplayFrequency = refreshRate;
        devMode.dmFields |= DM_DISPLAYFREQUENCY;
    }

    devMode.dmPelsWidth = width;
    devMode.dmPelsHeight = height;

    LONG result = ChangeDisplaySettingsExW(deviceName.c_str(), &devMode, nullptr, CDS_UPDATEREGISTRY | CDS_GLOBAL, nullptr);
    if (result == DISP_CHANGE_SUCCESSFUL) {
        std::wcout << L"[+] Resolution changed successfully.\n";
        return true;
    }
    else {
        std::wcerr << L"[!] Failed to change resolution. Error code: " << result << L"\n";
        return false;
    }
}

int wmain(int argc, wchar_t* argv[]) {
    std::wstring exeName = std::filesystem::path(argv[0]).filename().wstring();

    if (argc < 2) {
        PrintUsage(exeName);
        return 1;
    }

    std::wstring firstArg = argv[1];

    if (firstArg == L"--help" || firstArg == L"/?") {
        PrintUsage(exeName);
        return 0;
    }

    if (firstArg == L"--list") {
        ListDisplays();
        return 0;
    }

    if (firstArg == L"--test" && argc >= 5) {
        std::wstring display = NormalizeDisplayName(argv[2]);
        int w = _wtoi(argv[3]);
        int h = _wtoi(argv[4]);
        int r = 0;
        if (argc >= 6) r = _wtoi(argv[5]);
        TestDisplayResolution(display, w, h, r);
        return 0;
    }

    if (firstArg == L"--modes" && argc >= 3) {
        ListModes(argv[2]);
        return 0;
    }


    std::wstring displayName;
    int optStart;
    if (!firstArg.empty() && (firstArg[0] == L'-' || firstArg[0] == L'/')) {
        // No display specified — use monitor under cursor
        displayName = GetDisplayFromCursor();
        optStart = 1;            // options start at argv[1]
    }
    else {
        displayName = NormalizeDisplayName(firstArg);
        optStart = 2;            // options start at argv[2]
    }

    int width = 0, height = 0, refresh = 0;

    std::map<std::wstring, std::function<void(const wchar_t*)>> optionHandlers = {
    {L"-w",        [&](const wchar_t* val) { width = _wtoi(val); }},
    {L"--width",   [&](const wchar_t* val) { width = _wtoi(val); }},
    {L"/x",        [&](const wchar_t* val) { width = _wtoi(val); }},
    {L"-h",        [&](const wchar_t* val) { height = _wtoi(val); }},
    {L"--height",  [&](const wchar_t* val) { height = _wtoi(val); }},
    {L"/y",        [&](const wchar_t* val) { height = _wtoi(val); }},
    {L"-r",        [&](const wchar_t* val) { refresh = _wtoi(val); }},
    {L"--refresh", [&](const wchar_t* val) { refresh = _wtoi(val); }},
    {L"/r",        [&](const wchar_t* val) { refresh = _wtoi(val); }},
    };

    for (int i = optStart; i < argc; ++i) {
        std::wstring arg = argv[i];
        // Handle QRes style /x:1920 formatted tokens
        if (arg.rfind(L"/x:", 0) == 0) { width = _wtoi(arg.c_str() + 3); continue; }
        if (arg.rfind(L"/y:", 0) == 0) { height = _wtoi(arg.c_str() + 3); continue; }
        if (arg.rfind(L"/r:", 0) == 0) { refresh = _wtoi(arg.c_str() + 3); continue; }

        if (optionHandlers.count(arg) && i + 1 < argc) {
            optionHandlers[arg](argv[++i]);
        }
    }

    for (int i = 2; i < argc - 1; ++i) {
        std::wstring arg = argv[i];
        if (optionHandlers.count(arg)) {
            optionHandlers[arg](argv[++i]);
        }
    }

    if (width == 0 || height == 0) {
        std::wcerr << L"[!] Width and height must be specified.\n";
        PrintUsage(exeName);
        return 1;
    }

    if (!SetDisplayResolution(displayName, width, height, refresh))
        return 1;

    return 0;
}
