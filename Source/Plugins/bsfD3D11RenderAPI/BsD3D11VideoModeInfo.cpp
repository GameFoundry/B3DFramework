//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11VideoModeInfo.h"
#include "Error/BsException.h"
#include "String/BsUnicode.h"

namespace bs { namespace ct
{
	D3D11VideoModeInfo::D3D11VideoModeInfo(IDXGIAdapter* dxgiAdapter)
	{
		UINT32 outputIdx = 0;
		IDXGIOutput* output = nullptr;
		while (dxgiAdapter->EnumOutputs(outputIdx, &output) != DXGI_ERROR_NOT_FOUND)
		{
			mOutputs.push_back(bs_new<D3D11VideoOutputInfo>(output, outputIdx));
			outputIdx++;
		}
	}

	D3D11VideoOutputInfo::D3D11VideoOutputInfo(IDXGIOutput* output, UINT32 outputIdx)
		:mDXGIOutput(output)
	{
		DXGI_OUTPUT_DESC outputDesc;
		output->GetDesc(&outputDesc);
		mName = UTF8::fromWide(WString(outputDesc.DeviceName));

		UINT32 numModes = 0;

		HRESULT hr = output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, nullptr);
		if (FAILED(hr))
		{
			SAFE_RELEASE(output);
			BS_EXCEPT(InternalErrorException, "Error while enumerating adapter output video modes.");
		}

		DXGI_MODE_DESC* modeDesc = bs_newN<DXGI_MODE_DESC>(numModes);

		hr = output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, modeDesc);
		if (FAILED(hr))
		{
			bs_deleteN(modeDesc, numModes);

			SAFE_RELEASE(output);
			BS_EXCEPT(InternalErrorException, "Error while enumerating adapter output video modes.");
		}

		for (UINT32 i = 0; i < numModes; i++)
		{
			DXGI_MODE_DESC displayMode = modeDesc[i];

			bool foundVideoMode = false;
			for (auto videoMode : mVideoModes)
			{
				D3D11VideoMode* d3d11videoMode = static_cast<D3D11VideoMode*>(videoMode);

				if (d3d11videoMode->mWidth == displayMode.Width && d3d11videoMode->mHeight == displayMode.Height &&
					d3d11videoMode->mRefreshRateNumerator == displayMode.RefreshRate.Numerator &&
					d3d11videoMode->mRefreshRateDenominator == displayMode.RefreshRate.Denominator)
				{
					foundVideoMode = true;
					break;
				}
			}

			if (!foundVideoMode)
			{
				float refreshRate = displayMode.RefreshRate.Numerator / (float)displayMode.RefreshRate.Denominator;
				D3D11VideoMode* videoMode = bs_new<D3D11VideoMode>(displayMode.Width, displayMode.Height, refreshRate,
					outputIdx, displayMode.RefreshRate.Numerator, displayMode.RefreshRate.Denominator, displayMode);

				mVideoModes.push_back(videoMode);
			}
		}

		bs_deleteN(modeDesc, numModes);

		// Get desktop display mode
		HMONITOR hMonitor = outputDesc.Monitor;
		MONITORINFOEX monitorInfo;
		monitorInfo.cbSize = sizeof(MONITORINFOEX);
		GetMonitorInfo(hMonitor, &monitorInfo);

		DEVMODE devMode;
		devMode.dmSize = sizeof(DEVMODE);
		devMode.dmDriverExtra = 0;
		EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &devMode);

		DXGI_MODE_DESC currentMode;
		currentMode.Width = devMode.dmPelsWidth;
		currentMode.Height = devMode.dmPelsHeight;
		bool useDefaultRefreshRate = 1 == devMode.dmDisplayFrequency || 0 == devMode.dmDisplayFrequency;
		currentMode.RefreshRate.Numerator = useDefaultRefreshRate ? 0 : devMode.dmDisplayFrequency;
		currentMode.RefreshRate.Denominator = useDefaultRefreshRate ? 0 : 1;
		currentMode.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		currentMode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		currentMode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		DXGI_MODE_DESC nearestMode;
		ZeroMemory(&nearestMode, sizeof(nearestMode));

		output->FindClosestMatchingMode(&currentMode, &nearestMode, nullptr);

		float refreshRate = nearestMode.RefreshRate.Numerator / (float)nearestMode.RefreshRate.Denominator;
		mDesktopVideoMode = bs_new<D3D11VideoMode>(nearestMode.Width, nearestMode.Height, refreshRate, 
			outputIdx, nearestMode.RefreshRate.Numerator, nearestMode.RefreshRate.Denominator, nearestMode);
	}

	D3D11VideoOutputInfo::~D3D11VideoOutputInfo()
	{
		SAFE_RELEASE(mDXGIOutput);
	}

	D3D11VideoMode::D3D11VideoMode(UINT32 width, UINT32 height, float refreshRate, UINT32 outputIdx, 
		UINT32 refreshRateNumerator, UINT32 refreshRateDenominator, DXGI_MODE_DESC mode)
		:VideoMode(width, height, refreshRate, outputIdx), mRefreshRateNumerator(refreshRateNumerator),
		mRefreshRateDenominator(refreshRateDenominator), mD3D11Mode(mode)
	{ }
}}
