//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/B3DMouse.h"
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

/** Contains private data for the Win32 Mouse implementation. */
struct Mouse::Pimpl
{
	IDirectInput8* DirectInput;
	IDirectInputDevice8* Mouse;
	DWORD CoopSettings;
	HWND HWnd;
};

/**
 * Initializes DirectInput mouse device for a window with the specified handle. Only input from that window will be
 * reported.
 */
void InitializeDirectInput(Mouse::Pimpl* m, HWND hWnd)
{
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = DI_BUFFER_SIZE_MOUSE;

	HRESULT result = m->DirectInput->CreateDevice(GUID_SysMouse, &m->Mouse, nullptr);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput mouse init: Failed to create device. Error code: {0}.", (u64)result);
		return;
	}

	result = m->Mouse->SetDataFormat(&c_dfDIMouse2);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput mouse init: Failed to set format. Error code: {0}.", (u64)result);
		return;
	}

	result = m->Mouse->SetCooperativeLevel(hWnd, m->CoopSettings);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput mouse init: Failed to set coop level. Error code: {0}.", (u64)result);
		return;
	}

	result = m->Mouse->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput mouse init: Failed to set property. Error code: {0}.", (u64)result);
		return;
	}

	result = m->Mouse->Acquire();
	if(FAILED(result) && result != DIERR_OTHERAPPHASPRIO)
	{
		B3D_LOG(Error, LogInput, "DirectInput mouse init: Failed to acquire device. Error code: {0}.", (u64)result);
		return;
	}

	m->HWnd = hWnd;
}

/** Releases DirectInput resources for the provided device */
void ReleaseDirectInput(Mouse::Pimpl* m)
{
	if(m->Mouse)
	{
		m->Mouse->Unacquire();
		m->Mouse->Release();
		m->Mouse = nullptr;
	}
}

/** Notifies the input handler that a mouse press or release occurred. Triggers an event in the input handler. */
void DoMouseClick(Input* owner, ButtonCode mouseButton, const DIDEVICEOBJECTDATA& data)
{
	if(data.dwData & 0x80)
		owner->NotifyButtonPressedInternal(0, mouseButton, data.dwTimeStamp);
	else
		owner->NotifyButtonReleasedInternal(0, mouseButton, data.dwTimeStamp);
}

Mouse::Mouse(const String& name, Input* owner)
	: mName(name), mOwner(owner)
{
	InputPrivateData* pvtData = owner->GetPrivateDataInternal();

	m = B3DNew<Pimpl>();
	m->DirectInput = pvtData->DirectInput;
	m->CoopSettings = pvtData->MouseSettings;
	m->Mouse = nullptr;
	m->HWnd = nullptr;

	// Don't initialize DirectInput in headless mode (window handle == 0)
	const u64 windowHandle = owner->GetWindowHandle();
	if(windowHandle != 0)
		InitializeDirectInput(m, (HWND)windowHandle);
}

Mouse::~Mouse()
{
	ReleaseDirectInput(m);

	B3DDelete(m);
}

void Mouse::Capture()
{
	if(m->Mouse == nullptr)
		return;

	DIDEVICEOBJECTDATA diBuff[DI_BUFFER_SIZE_MOUSE];
	DWORD numEntries = DI_BUFFER_SIZE_MOUSE;

	HRESULT hr = m->Mouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &numEntries, 0);
	if(hr != DI_OK)
	{
		hr = m->Mouse->Acquire();
		while(hr == DIERR_INPUTLOST)
			hr = m->Mouse->Acquire();

		hr = m->Mouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &numEntries, 0);

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
		mOwner->NotifyMouseMovedInternal(relativeX, relativeY, relativeZ);
}

void Mouse::ChangeCaptureContext(u64 windowHandle)
{
	HWND newWindowHandle = (HWND)windowHandle;

	if(m->HWnd != newWindowHandle)
	{
		ReleaseDirectInput(m);

		// Don't initialize DirectInput for invalid handles (headless mode or lost focus)
		if(windowHandle != 0)
			InitializeDirectInput(m, newWindowHandle);
		else
			m->HWnd = newWindowHandle;
	}
}
