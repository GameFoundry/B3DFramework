//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/B3DInput.h"
#include "Private/Win32/B3DWin32Input.h"

#include "B3DApplication.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "Platform/B3DPlatform.h"

using namespace b3d;

BOOL CALLBACK DIEnumDevCallbackInternal(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
	Vector<GamepadInfo>* infos = (Vector<GamepadInfo>*)pvRef;

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
		gamepadInfo.Id = (u32)infos->size();
		gamepadInfo.IsXInput = false;
		gamepadInfo.XInputDev = 0;

		infos->push_back(gamepadInfo);
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
					for(auto& entry : infos)
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

Win32InputBackend::Win32InputBackend(Input& owner)
	: mOwner(owner)
{
	mWindowHandle = owner.GetWindowHandle();

	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();

	const bool isHeadless = gpuDevice == nullptr || gpuDevice->GetCapabilities().DeviceName == "Null" || mWindowHandle == 0;
	if(isHeadless)
		return;

	if(IsWindow((HWND)mWindowHandle) == 0)
		B3D_LOG(Fatal, LogPlatform, "Input backend failed to initialize. Invalid HWND provided.");

	HINSTANCE hInst = GetModuleHandle(0);

	HRESULT hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&mDirectInput, nullptr);
	if(FAILED(hr))
		B3D_LOG(Fatal, LogPlatform, "Unable to initialize DirectInput.");

	mKbSettings = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
	mMouseSettings = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;

	// Note: Assuming there is always exactly 1 keyboard and 1 mouse
	mKeyboard = B3DNew<Win32Keyboard>(owner, mDirectInput, mKbSettings, mWindowHandle);
	mMouse = B3DNew<Win32Mouse>(owner, mDirectInput, mMouseSettings, mWindowHandle);

	Vector<GamepadInfo> gamepadInfos;
	EnumerateGamepads(gamepadInfos);

	for(auto& gamepadInfo : gamepadInfos)
	{
		mGamepads.push_back(B3DNew<Win32Gamepad>(gamepadInfo, owner, mDirectInput, mMouseSettings, mWindowHandle));
		mOwner.NotifyGamepadAdded(gamepadInfo.Id, gamepadInfo.Name);
	}

	// Detect gamepad hot-plug. The event may trigger from the message loop thread, so only flag the change here and
	// process it on the next Update().
	mDevicesChangedConn = Platform::OnInputDevicesChanged.Connect([this]() { mDevicesDirty.store(true); });
}

Win32InputBackend::~Win32InputBackend()
{
	mDevicesChangedConn.Disconnect();

	if(mMouse != nullptr)
		B3DDelete(mMouse);

	if(mKeyboard != nullptr)
		B3DDelete(mKeyboard);

	for(auto& gamepad : mGamepads)
		B3DDelete(gamepad);

	if(mDirectInput != nullptr)
		mDirectInput->Release();
}

void Win32InputBackend::Update()
{
	if(mDevicesDirty.exchange(false))
		RebuildGamepads();

	if(mMouse != nullptr)
		mMouse->Capture();

	if(mKeyboard != nullptr)
		mKeyboard->Capture();

	for(auto& gamepad : mGamepads)
		gamepad->Capture();
}

u32 Win32InputBackend::GetDeviceCount(InputDevice device) const
{
	switch(device)
	{
	case InputDevice::Keyboard: return mKeyboard != nullptr ? 1 : 0;
	case InputDevice::Mouse: return mMouse != nullptr ? 1 : 0;
	case InputDevice::Gamepad: return (u32)mGamepads.size();
	default:
	case InputDevice::Count: return 0;
	}
}

String Win32InputBackend::GetDeviceName(InputDevice type, u32 deviceIndex) const
{
	switch(type)
	{
	case InputDevice::Keyboard:
		if(mKeyboard != nullptr && deviceIndex == 0)
			return "Keyboard";

		return StringUtility::kBlank;
	case InputDevice::Mouse:
		if(mMouse != nullptr && deviceIndex == 0)
			return "Mouse";

		return StringUtility::kBlank;
	case InputDevice::Gamepad:
		if(deviceIndex < (u32)mGamepads.size())
			return mGamepads[deviceIndex]->GetName();

		return StringUtility::kBlank;
	default:
		return StringUtility::kBlank;
	}
}

void Win32InputBackend::ChangeCaptureContext(u64 windowHandle)
{
	mWindowHandle = windowHandle;

	if(mKeyboard != nullptr)
		mKeyboard->ChangeCaptureContext(windowHandle);

	if(mMouse != nullptr)
		mMouse->ChangeCaptureContext(windowHandle);

	for(auto& gamepad : mGamepads)
		gamepad->ChangeCaptureContext(windowHandle);
}

void Win32InputBackend::EnumerateGamepads(Vector<GamepadInfo>& outGamepadInfos)
{
	// Enumerate all attached DirectInput devices
	mDirectInput->EnumDevices(NULL, DIEnumDevCallbackInternal, &outGamepadInfos, DIEDFL_ATTACHEDONLY);

	for(u32 i = 0; i < 4; ++i)
	{
		XINPUT_STATE state;
		if(XInputGetState(i, &state) != ERROR_DEVICE_NOT_CONNECTED)
		{
			CheckXInputDevices(outGamepadInfos);
			break;
		}
	}
}

void Win32InputBackend::RebuildGamepads()
{
	Vector<GamepadInfo> attachedInfos;
	EnumerateGamepads(attachedInfos);

	// Destroy gamepads that are no longer attached, matching devices by their unique instance GUID
	for(auto it = mGamepads.begin(); it != mGamepads.end();)
	{
		Win32Gamepad* gamepad = *it;

		bool isAttached = false;
		for(auto& attachedInfo : attachedInfos)
			isAttached |= IsEqualGUID(attachedInfo.GuidInstance, gamepad->GetInfo().GuidInstance) != 0;

		if(isAttached)
		{
			++it;
			continue;
		}

		mOwner.NotifyGamepadRemoved(gamepad->GetInfo().Id, gamepad->GetName());
		B3DDelete(gamepad);
		it = mGamepads.erase(it);
	}

	// Create newly attached gamepads
	for(auto& attachedInfo : attachedInfos)
	{
		bool exists = false;
		for(auto& gamepad : mGamepads)
			exists |= IsEqualGUID(attachedInfo.GuidInstance, gamepad->GetInfo().GuidInstance) != 0;

		if(exists)
			continue;

		// Assign the lowest free id, so device ids (and with them the per-device state slots in Input) get reused
		// instead of growing unbounded across plug/unplug cycles
		u32 id = 0;
		while(true)
		{
			bool isUsed = false;
			for(auto& gamepad : mGamepads)
				isUsed |= gamepad->GetInfo().Id == id;

			if(!isUsed)
				break;

			id++;
		}

		attachedInfo.Id = id;

		mGamepads.push_back(B3DNew<Win32Gamepad>(attachedInfo, mOwner, mDirectInput, mMouseSettings, mWindowHandle));
		mOwner.NotifyGamepadAdded(attachedInfo.Id, attachedInfo.Name);
	}
}

namespace b3d
{
IInputBackend* CreateInputBackend(Input& owner)
{
	return B3DNew<Win32InputBackend>(owner);
}
} // namespace b3d
