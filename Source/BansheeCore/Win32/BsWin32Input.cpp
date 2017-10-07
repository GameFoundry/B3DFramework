//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Input/BsRawInputHandler.h"
#include "Error/BsException.h"
#include "Win32/BsWin32Input.h"
#include "Input/BsMouse.h"
#include "Input/BsKeyboard.h"
#include "Input/BsGamepad.h"

namespace bs
{
	BOOL CALLBACK _DIEnumDevCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
	{
		InputPrivateData* data = (InputPrivateData*)(pvRef);

		if (GET_DIDEVICE_TYPE(lpddi->dwDevType) == DI8DEVTYPE_JOYSTICK ||
			GET_DIDEVICE_TYPE(lpddi->dwDevType) == DI8DEVTYPE_GAMEPAD ||
			GET_DIDEVICE_TYPE(lpddi->dwDevType) == DI8DEVTYPE_1STPERSON ||
			GET_DIDEVICE_TYPE(lpddi->dwDevType) == DI8DEVTYPE_DRIVING ||
			GET_DIDEVICE_TYPE(lpddi->dwDevType) == DI8DEVTYPE_FLIGHT)
		{
			GamepadInfo gamepadInfo;
			gamepadInfo.name = lpddi->tszInstanceName;
			gamepadInfo.guidInstance = lpddi->guidInstance;
			gamepadInfo.guidProduct = lpddi->guidProduct;
			gamepadInfo.id = (UINT32)data->gamepadInfos.size();
			gamepadInfo.isXInput = false;
			gamepadInfo.xInputDev = 0;

			data->gamepadInfos.push_back(gamepadInfo);
		}

		return DIENUM_CONTINUE;
	}
	
	void CheckXInputDevices(Vector<GamepadInfo>& infos)
	{
		if (infos.size() == 0)
			return;

		HRESULT hr = CoInitialize(nullptr);
		bool cleanupCOM = SUCCEEDED(hr);

		// Create WMI
		IWbemLocator* IWbemLocator = nullptr;
		hr = CoCreateInstance(__uuidof(WbemLocator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWbemLocator), (LPVOID*)&IWbemLocator);
		if (FAILED(hr) || IWbemLocator == nullptr)
			goto cleanup;

		BSTR classNameSpace = SysAllocString(L"\\\\.\\root\\cimv2");
		if (classNameSpace == nullptr)
			goto cleanup;

		BSTR className = SysAllocString(L"Win32_PNPEntity");
		if (className == nullptr)
			goto cleanup;

		BSTR deviceID = SysAllocString(L"DeviceID");
		if (deviceID == nullptr)
			goto cleanup;

		// Connect to WMI
		IWbemServices* IWbemServices = nullptr;
		hr = IWbemLocator->ConnectServer(classNameSpace, nullptr, nullptr, 0L, 0L, nullptr, nullptr, &IWbemServices);
		if (FAILED(hr) || IWbemServices == nullptr)
			goto cleanup;

		// Switch security level to IMPERSONATE
		CoSetProxyBlanket(IWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);

		IEnumWbemClassObject* enumDevices = nullptr;
		hr = IWbemServices->CreateInstanceEnum(className, 0, nullptr, &enumDevices);
		if (FAILED(hr) || enumDevices == nullptr)
			goto cleanup;

		// Loop over all devices
		IWbemClassObject* devices[20] = { 0 };
		for (;; )
		{
			DWORD numDevices = 0;
			hr = enumDevices->Next(5000, 20, devices, &numDevices);
			if (FAILED(hr))
				goto cleanup;

			if (numDevices == 0)
				break;

			for (DWORD i = 0; i < numDevices; i++)
			{
				// For each device, get its device ID
				VARIANT var;
				hr = devices[i]->Get(deviceID, 0L, &var, nullptr, nullptr);
				if (SUCCEEDED(hr) && var.vt == VT_BSTR && var.bstrVal != nullptr)
				{
					// Check if the device ID contains "IG_".  If it does, then it's an XInput device
					if (wcsstr(var.bstrVal, L"IG_"))
					{
						// If it does, then get the VID/PID from var.bstrVal
						DWORD dwPid = 0, dwVid = 0;
						WCHAR* strVid = wcsstr(var.bstrVal, L"VID_");
						if (strVid && swscanf_s(strVid, L"VID_%4X", &dwVid) != 1)
							dwVid = 0;

						WCHAR* strPid = wcsstr(var.bstrVal, L"PID_");
						if (strPid && swscanf_s(strPid, L"PID_%4X", &dwPid) != 1)
							dwPid = 0;

						// Compare the VID/PID to the DInput device
						DWORD dwVidPid = MAKELONG(dwVid, dwPid);
						for (auto entry : infos)
						{
							if (dwVidPid == entry.guidProduct.Data1)
							{
								entry.isXInput = true;
								entry.xInputDev = (int)entry.id; // Note: These might not match and I might need to get the XInput id differently
							}
						}
					}
				}

				devices[i]->Release();
			}
		}

	cleanup:
		if (classNameSpace)
			SysFreeString(classNameSpace);

		if (deviceID)
			SysFreeString(deviceID);

		if (className)
			SysFreeString(className);

		for (DWORD i = 0; i < 20; i++)
			devices[i]->Release();

		enumDevices->Release();
		IWbemLocator->Release();
		IWbemServices->Release();

		if (cleanupCOM)
			CoUninitialize();
	}

	void RawInputHandler::initialize()
	{
		mPlatformData = bs_new<InputPrivateData>();
		
		if (IsWindow((HWND)mWindowHandle) == 0)
			BS_EXCEPT(InvalidStateException, "RawInputManager failed to initialized. Invalid HWND provided.")

		HINSTANCE hInst = GetModuleHandle(0);

		HRESULT hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&mPlatformData->directInput, nullptr);
		if (FAILED(hr))
			BS_EXCEPT(InternalErrorException, "Unable to initialize DirectInput.");

		mPlatformData->kbSettings = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
		mPlatformData->mouseSettings = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
		
		// Enumerate all attached devices
		// Note: Only enumerating gamepads, assuming there is 1 keyboard and 1 mouse
		mPlatformData->directInput->EnumDevices(NULL, _DIEnumDevCallback, mPlatformData, DIEDFL_ATTACHEDONLY);

		for (UINT32 i = 0; i < 4; ++i)
		{
			XINPUT_STATE state;
			if (XInputGetState(i, &state) != ERROR_DEVICE_NOT_CONNECTED)
			{
				CheckXInputDevices(mPlatformData->gamepadInfos);
				break;
			}
		}

		if (getDeviceCount(InputDevice::Keyboard) > 0)
			mKeyboard = bs_new<Keyboard>("Keyboard", this);

		if (getDeviceCount(InputDevice::Mouse) > 0)
			mMouse = bs_new<Mouse>("Mouse", this);

		UINT32 numGamepads = getDeviceCount(InputDevice::Gamepad);
		for (UINT32 i = 0; i < numGamepads; i++)
			mGamepads.push_back(bs_new<Gamepad>(mPlatformData->gamepadInfos[i].name, mPlatformData->gamepadInfos[i], this));
	}

	void RawInputHandler::cleanUp()
	{
		mPlatformData->directInput->Release();

		bs_delete(mPlatformData);
	}

	UINT32 RawInputHandler::getDeviceCount(InputDevice device) const
	{
		switch(device)
		{
		case InputDevice::Keyboard: return 1;
		case InputDevice::Mouse: return 1;
		case InputDevice::Gamepad: return (UINT32)mPlatformData->gamepadInfos.size();
		default:
		case InputDevice::Count: return 0;
		}
	}
}
