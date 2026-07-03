//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/B3DInput.h"
#include "Private/Win32/B3DWin32Input.h"

using namespace b3d;

namespace
{
	constexpr DWORD kMouseOffsetX = static_cast<DWORD>(offsetof(DIMOUSESTATE2, lX));
	constexpr DWORD kMouseOffsetY = static_cast<DWORD>(offsetof(DIMOUSESTATE2, lY));
	constexpr DWORD kMouseOffsetZ = static_cast<DWORD>(offsetof(DIMOUSESTATE2, lZ));
	constexpr DWORD kMouseOffsetButton0 = static_cast<DWORD>(offsetof(DIMOUSESTATE2, rgbButtons[0]));
	constexpr DWORD kMouseOffsetButton1 = static_cast<DWORD>(offsetof(DIMOUSESTATE2, rgbButtons[1]));
	constexpr DWORD kMouseOffsetButton2 = static_cast<DWORD>(offsetof(DIMOUSESTATE2, rgbButtons[2]));
	constexpr DWORD kMouseOffsetButton3 = static_cast<DWORD>(offsetof(DIMOUSESTATE2, rgbButtons[3]));
	constexpr DWORD kMouseOffsetButton4 = static_cast<DWORD>(offsetof(DIMOUSESTATE2, rgbButtons[4]));
	constexpr DWORD kMouseOffsetButton5 = static_cast<DWORD>(offsetof(DIMOUSESTATE2, rgbButtons[5]));
	constexpr DWORD kMouseOffsetButton6 = static_cast<DWORD>(offsetof(DIMOUSESTATE2, rgbButtons[6]));
	constexpr DWORD kMouseOffsetButton7 = static_cast<DWORD>(offsetof(DIMOUSESTATE2, rgbButtons[7]));
}

/** Notifies the input handler that a mouse press or release occurred. Triggers an event in the input handler. */
void DoMouseClick(Input& owner, ButtonCode mouseButton, const DIDEVICEOBJECTDATA& data)
{
	if(data.dwData & 0x80)
		owner.NotifyButtonPressed(0, mouseButton, data.dwTimeStamp);
	else
		owner.NotifyButtonReleased(0, mouseButton, data.dwTimeStamp);
}

Win32Mouse::Win32Mouse(Input& owner, IDirectInput8* directInput, DWORD coopSettings, u64 windowHandle)
	: mOwner(owner), mDirectInput(directInput), mCoopSettings(coopSettings)
{
	// Don't initialize DirectInput in headless mode (window handle == 0)
	if(windowHandle != 0)
		InitializeDirectInput((HWND)windowHandle);
}

Win32Mouse::~Win32Mouse()
{
	ReleaseDirectInput();
}

void Win32Mouse::InitializeDirectInput(HWND hWnd)
{
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = DI_BUFFER_SIZE_MOUSE;

	HRESULT result = mDirectInput->CreateDevice(GUID_SysMouse, &mMouse, nullptr);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput mouse init: Failed to create device. Error code: {0}.", (u64)result);
		return;
	}

	result = mMouse->SetDataFormat(&c_dfDIMouse2);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput mouse init: Failed to set format. Error code: {0}.", (u64)result);
		return;
	}

	result = mMouse->SetCooperativeLevel(hWnd, mCoopSettings);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput mouse init: Failed to set coop level. Error code: {0}.", (u64)result);
		return;
	}

	result = mMouse->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput mouse init: Failed to set property. Error code: {0}.", (u64)result);
		return;
	}

	result = mMouse->Acquire();
	if(FAILED(result) && result != DIERR_OTHERAPPHASPRIO)
	{
		B3D_LOG(Error, LogInput, "DirectInput mouse init: Failed to acquire device. Error code: {0}.", (u64)result);
		return;
	}

	mHWnd = hWnd;
}

void Win32Mouse::ReleaseDirectInput()
{
	if(mMouse)
	{
		mMouse->Unacquire();
		mMouse->Release();
		mMouse = nullptr;
	}
}

void Win32Mouse::Capture()
{
	if(mMouse == nullptr)
		return;

	DIDEVICEOBJECTDATA diBuff[DI_BUFFER_SIZE_MOUSE];
	DWORD numEntries = DI_BUFFER_SIZE_MOUSE;

	HRESULT hr = mMouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &numEntries, 0);
	if(hr != DI_OK)
	{
		hr = mMouse->Acquire();
		while(hr == DIERR_INPUTLOST)
			hr = mMouse->Acquire();

		hr = mMouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &numEntries, 0);

		if(FAILED(hr))
			return;
	}

	i32 relativeX, relativeY, relativeZ;
	relativeX = relativeY = relativeZ = 0;

	bool axesMoved = false;
	for(u32 entryIndex = 0; entryIndex < numEntries; ++entryIndex)
	{
		switch(diBuff[entryIndex].dwOfs)
		{
		case kMouseOffsetButton0:
			DoMouseClick(mOwner, ButtonCode::MouseLeft, diBuff[entryIndex]);
			break;
		case kMouseOffsetButton1:
			DoMouseClick(mOwner, ButtonCode::MouseRight, diBuff[entryIndex]);
			break;
		case kMouseOffsetButton2:
			DoMouseClick(mOwner, ButtonCode::MouseMiddle, diBuff[entryIndex]);
			break;
		case kMouseOffsetButton3:
			DoMouseClick(mOwner, ButtonCode::MouseButton4, diBuff[entryIndex]);
			break;
		case kMouseOffsetButton4:
			DoMouseClick(mOwner, ButtonCode::MouseButton5, diBuff[entryIndex]);
			break;
		case kMouseOffsetButton5:
			DoMouseClick(mOwner, ButtonCode::MouseButton6, diBuff[entryIndex]);
			break;
		case kMouseOffsetButton6:
			DoMouseClick(mOwner, ButtonCode::MouseButton7, diBuff[entryIndex]);
			break;
		case kMouseOffsetButton7:
			DoMouseClick(mOwner, ButtonCode::MouseButton8, diBuff[entryIndex]);
			break;
		case kMouseOffsetX:
			relativeX += diBuff[entryIndex].dwData;
			axesMoved = true;
			break;
		case kMouseOffsetY:
			relativeY += diBuff[entryIndex].dwData;
			axesMoved = true;
			break;
		case kMouseOffsetZ:
			relativeZ += diBuff[entryIndex].dwData;
			axesMoved = true;
			break;
		default: break;
		}
	}

	if(axesMoved)
		mOwner.NotifyMouseMoved(relativeX, relativeY, relativeZ);
}

void Win32Mouse::ChangeCaptureContext(u64 windowHandle)
{
	HWND newWindowHandle = (HWND)windowHandle;

	if(mHWnd != newWindowHandle)
	{
		ReleaseDirectInput();

		// Don't initialize DirectInput for invalid handles (headless mode or lost focus)
		if(windowHandle != 0)
			InitializeDirectInput(newWindowHandle);
		else
			mHWnd = newWindowHandle;
	}
}
