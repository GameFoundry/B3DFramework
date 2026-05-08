//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/B3DInput.h"
#include "Private/Win32/B3DWin32Input.h"

#include "B3DApplication.h"
#include "Input/B3DMouse.h"
#include "Input/B3DKeyboard.h"
#include "Input/B3DGamepad.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"

using namespace b3d;

BOOL CALLBACK DIEnumDevCallbackInternal(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
	InputPrivateData* data = (InputPrivateData*)(pvRef);

	if(GET_DIDEVICE_TYPE(lpddi->dwDevType) == DI8DEVTYPE_JOYSTICK ||
	   GET_DIDEVICE_TYPE(lpddi->dwDevType) == DI8DEVTYPE_GAMEPAD ||
	   GET_DIDEVICE_TYPE(lpddi->dwDevType) == DI8DEVTYPE_1STPERSON ||
	   GET_DIDEVICE_TYPE(lpddi->dwDevType) == DI8DEVTYPE_DRIVING ||
	   GET_DIDEVICE_TYPE(lpddi->dwDevType) == DI8DEVTYPE_FLIGHT)
	{
		GamepadInfo gamepadInfo;
		gamepadInfo.Name = lpddi->tszInstanceName;
		gamepadInfo.GuidInstance = lpddi->guidInstance;
		gamepadInfo.GuidProduct = lpddi->guidProduct;
		gamepadInfo.Id = (u32)data->GamepadInfos.size();
		gamepadInfo.IsXInput = false;
		gamepadInfo.XInputDev = 0;

		data->GamepadInfos.push_back(gamepadInfo);
	}

	return DIENUM_CONTINUE;
}

void CheckXInputDevices(Vector<GamepadInfo>& infos)
{
	if(infos.size() == 0)
		return;

	HRESULT hr = CoInitialize(nullptr);
	bool cleanupCOM = SUCCEEDED(hr);

	BSTR classNameSpace = SysAllocString(L"\\\\.\\root\\cimv2");
	BSTR className = SysAllocString(L"Win32_PNPEntity");
	BSTR deviceID = SysAllocString(L"DeviceID");

	IWbemServices* IWbemServices = nullptr;
	IEnumWbemClassObject* enumDevices = nullptr;
	IWbemClassObject* devices[20] = { 0 };

	// Create WMI
	IWbemLocator* IWbemLocator = nullptr;
	hr = CoCreateInstance(__uuidof(WbemLocator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWbemLocator), (LPVOID*)&IWbemLocator);
	if(FAILED(hr) || IWbemLocator == nullptr)
		goto cleanup;

	if(classNameSpace == nullptr)
		goto cleanup;

	if(className == nullptr)
		goto cleanup;

	if(deviceID == nullptr)
		goto cleanup;

	// Connect to WMI
	hr = IWbemLocator->ConnectServer(classNameSpace, nullptr, nullptr, 0L, 0L, nullptr, nullptr, &IWbemServices);
	if(FAILED(hr) || IWbemServices == nullptr)
		goto cleanup;

	// Switch security level to IMPERSONATE
	CoSetProxyBlanket(IWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);

	hr = IWbemServices->CreateInstanceEnum(className, 0, nullptr, &enumDevices);
	if(FAILED(hr) || enumDevices == nullptr)
		goto cleanup;

	// Loop over all devices
	for(;;)
	{
		DWORD numDevices = 0;
		hr = enumDevices->Next(5000, 20, devices, &numDevices);
		if(FAILED(hr))
			goto cleanup;

		if(numDevices == 0)
			break;

		for(DWORD i = 0; i < numDevices; i++)
		{
			// For each device, get its device ID
			VARIANT var;
			hr = devices[i]->Get(deviceID, 0L, &var, nullptr, nullptr);
			if(SUCCEEDED(hr) && var.vt == VT_BSTR && var.bstrVal != nullptr)
			{
				// Check if the device ID contains "IG_".  If it does, then it's an XInput device
				if(wcsstr(var.bstrVal, L"IG_"))
				{
					// If it does, then get the VID/PID from var.bstrVal
					DWORD dwPid = 0, dwVid = 0;
					WCHAR* strVid = wcsstr(var.bstrVal, L"VID_");
					if(strVid && swscanf_s(strVid, L"VID_%4X", &dwVid) != 1)
						dwVid = 0;

					WCHAR* strPid = wcsstr(var.bstrVal, L"PID_");
					if(strPid && swscanf_s(strPid, L"PID_%4X", &dwPid) != 1)
						dwPid = 0;

					// Compare the VID/PID to the DInput device
					DWORD dwVidPid = MAKELONG(dwVid, dwPid);
					for(auto entry : infos)
					{
						if(dwVidPid == entry.GuidProduct.Data1)
						{
							entry.IsXInput = true;
							entry.XInputDev = (int)entry.Id; // Note: These might not match and I might need to get the XInput id differently
						}
					}
				}
			}

			devices[i]->Release();
			devices[i] = nullptr;
		}
	}

cleanup:
	if(classNameSpace)
		SysFreeString(classNameSpace);

	if(deviceID)
		SysFreeString(deviceID);

	if(className)
		SysFreeString(className);

	for(DWORD i = 0; i < 20; i++)
	{
		if(devices[i])
			devices[i]->Release();
	}

	enumDevices->Release();
	IWbemLocator->Release();
	IWbemServices->Release();

	if(cleanupCOM)
		CoUninitialize();
}

void Input::InitRawInput()
{
	mPlatformData = B3DNew<InputPrivateData>();

	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();

	const bool isHeadless = gpuDevice == nullptr || gpuDevice->GetCapabilities().DeviceName == "Null" || mWindowHandle == 0;
	if(isHeadless)
		return;

	if(IsWindow((HWND)mWindowHandle) == 0)
		B3D_LOG(Fatal, LogPlatform, "RawInputManager failed to initialized. Invalid HWND provided.");

	HINSTANCE hInst = GetModuleHandle(0);

	HRESULT hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&mPlatformData->DirectInput, nullptr);
	if(FAILED(hr))
		B3D_LOG(Fatal, LogPlatform, "Unable to initialize DirectInput.");

	mPlatformData->KbSettings = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
	mPlatformData->MouseSettings = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;

	// Enumerate all attached devices
	// Note: Only enumerating gamepads, assuming there is 1 keyboard and 1 mouse
	mPlatformData->DirectInput->EnumDevices(NULL, DIEnumDevCallbackInternal, mPlatformData, DIEDFL_ATTACHEDONLY);

	for(u32 i = 0; i < 4; ++i)
	{
		XINPUT_STATE state;
		if(XInputGetState(i, &state) != ERROR_DEVICE_NOT_CONNECTED)
		{
			CheckXInputDevices(mPlatformData->GamepadInfos);
			break;
		}
	}

	if(GetDeviceCount(InputDevice::Keyboard) > 0)
		mKeyboard = B3DNew<Keyboard>("Keyboard", this);

	if(GetDeviceCount(InputDevice::Mouse) > 0)
		mMouse = B3DNew<Mouse>("Mouse", this);

	u32 numGamepads = GetDeviceCount(InputDevice::Gamepad);
	for(u32 i = 0; i < numGamepads; i++)
		mGamepads.push_back(B3DNew<Gamepad>(mPlatformData->GamepadInfos[i].Name, mPlatformData->GamepadInfos[i], this));
}

void Input::CleanUpRawInput()
{
	if(mMouse != nullptr)
		B3DDelete(mMouse);

	if(mKeyboard != nullptr)
		B3DDelete(mKeyboard);

	for(auto& gamepad : mGamepads)
		B3DDelete(gamepad);

	if(mPlatformData->DirectInput != nullptr)
		mPlatformData->DirectInput->Release();

	B3DDelete(mPlatformData);
}

u32 Input::GetDeviceCount(InputDevice device) const
{
	switch(device)
	{
	case InputDevice::Keyboard: return 1;
	case InputDevice::Mouse: return 1;
	case InputDevice::Gamepad: return (u32)mPlatformData->GamepadInfos.size();
	default:
	case InputDevice::Count: return 0;
	}
}
