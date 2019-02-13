//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Win32/BsWin32VideoModeInfo.h"
#include "Math/BsMath.h"

namespace bs { namespace ct
{
	BOOL CALLBACK monitorEnumCallback(HMONITOR hMonitor, HDC hdc, LPRECT rect, LPARAM lParam)
	{
		Vector<HMONITOR>* outputInfos = (Vector<HMONITOR>*)lParam;
		outputInfos->push_back(hMonitor);

		return TRUE;
	};

	Win32VideoModeInfo::Win32VideoModeInfo()
	{
		Vector<HMONITOR> handles;
		EnumDisplayMonitors(0, nullptr, &monitorEnumCallback, (LPARAM)&handles);

		// Sort so that primary is the first output
		for (auto iter = handles.begin(); iter != handles.end(); ++iter)
		{
			MONITORINFOEX monitorInfo;

			memset(&monitorInfo, 0, sizeof(MONITORINFOEX));
			monitorInfo.cbSize = sizeof(MONITORINFOEX);
			GetMonitorInfo(*iter, &monitorInfo);

			if ((monitorInfo.dwFlags & MONITORINFOF_PRIMARY) != 0)
			{
				if (iter != handles.begin())
				{
					HMONITOR temp = handles[0];
					handles[0] = *iter;
					*iter = temp;
				}

				break;
			}
		}

		UINT32 idx = 0;
		for (auto& handle : handles)
		{
			mOutputs.push_back(bs_new<Win32VideoOutputInfo>(handle, idx++));
		}
	}

	Win32VideoOutputInfo::Win32VideoOutputInfo(HMONITOR monitorHandle, UINT32 outputIdx)
		:mMonitorHandle(monitorHandle)
	{
		MONITORINFOEX monitorInfo;

		memset(&monitorInfo, 0, sizeof(MONITORINFOEX));
		monitorInfo.cbSize = sizeof(MONITORINFOEX);
		GetMonitorInfo(mMonitorHandle, &monitorInfo);

		mName = monitorInfo.szDevice;

		DEVMODE devMode;
		devMode.dmSize = sizeof(DEVMODE);
		devMode.dmDriverExtra = 0;

		UINT32 i = 0;
		while (EnumDisplaySettings(monitorInfo.szDevice, i++, &devMode))
		{
			bool foundVideoMode = false;
			for (auto videoMode : mVideoModes)
			{
				Win32VideoMode* win32VideoMode = static_cast<Win32VideoMode*>(videoMode);

				UINT32 intRefresh = Math::roundToInt(win32VideoMode->refreshRate);
				if (win32VideoMode->width == devMode.dmPelsWidth && win32VideoMode->height == devMode.dmPelsHeight
					&& intRefresh == devMode.dmDisplayFrequency)
				{
					foundVideoMode = true;
					break;
				}
			}

			if (!foundVideoMode)
			{
				Win32VideoMode* videoMode = bs_new<Win32VideoMode>(devMode.dmPelsWidth, devMode.dmPelsHeight,
					(float)devMode.dmDisplayFrequency, outputIdx);
				videoMode->isCustom = false;

				mVideoModes.push_back(videoMode);
			}
		}

		// Get desktop display mode
		EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &devMode);

		Win32VideoMode* desktopVideoMode = bs_new<Win32VideoMode>(devMode.dmPelsWidth, devMode.dmPelsHeight,
			(float)devMode.dmDisplayFrequency, outputIdx);
		desktopVideoMode->isCustom = false;

		mDesktopVideoMode = desktopVideoMode;
	}

	Win32VideoMode::Win32VideoMode(UINT32 width, UINT32 height, float refreshRate, UINT32 outputIdx)
		:VideoMode(width, height, refreshRate, outputIdx)
	{ }
}}