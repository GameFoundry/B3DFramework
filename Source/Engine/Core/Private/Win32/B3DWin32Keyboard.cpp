//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/B3DKeyboard.h"
#include "Input/B3DInput.h"
#include "Private/Win32/B3DWin32Input.h"
#include "Error/B3DException.h"

using namespace b3d;

/** Contains private data for the Win32 Keyboard implementation. */
struct Keyboard::Pimpl
{
	IDirectInput8* DirectInput;
	IDirectInputDevice8* Keyboard;
	DWORD CoopSettings;
	HWND HWnd;

	u8 KeyBuffer[256];
};

/**
 * Initializes DirectInput keyboard device for a window with the specified handle. Only input from that window will be
 * reported.
 */
void InitializeDirectInput(Keyboard::Pimpl* m, HWND hWnd)
{
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = DI_BUFFER_SIZE_KEYBOARD;

	HRESULT result = m->DirectInput->CreateDevice(GUID_SysKeyboard, &m->Keyboard, nullptr);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput keyboard init: Failed to create device. Error code: {0}.", (u64)result);
		return;
	}

	result = m->Keyboard->SetDataFormat(&c_dfDIKeyboard);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput keyboard init: Failed to set format. Error code: {0}.", (u64)result);
		return;
	}

	result = m->Keyboard->SetCooperativeLevel(hWnd, m->CoopSettings);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput keyboard init: Failed to set coop level. Error code: {0}.", (u64)result);
		return;
	}

	result = m->Keyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput keyboard init: Failed to set property. Error code: {0}.", (u64)result);
		return;
	}

	result = m->Keyboard->Acquire();
	if(FAILED(result) && result != DIERR_OTHERAPPHASPRIO)
	{
		B3D_LOG(Error, LogInput, "DirectInput keyboard init: Failed to acquire device. Error code: {0}.", (u64)result);
		return;
	}

	m->HWnd = hWnd;
}

/** Releases DirectInput resources for the provided device */
void ReleaseDirectInput(Keyboard::Pimpl* m)
{
	if(m->Keyboard)
	{
		m->Keyboard->Unacquire();
		m->Keyboard->Release();
		m->Keyboard = nullptr;
	}
}

Keyboard::Keyboard(const String& name, Input* owner)
	: mName(name), mOwner(owner)
{
	InputPrivateData* pvtData = owner->GetPrivateDataInternal();

	m = B3DNew<Pimpl>();
	m->DirectInput = pvtData->DirectInput;
	m->CoopSettings = pvtData->KbSettings;
	m->Keyboard = nullptr;
	B3DZeroOut(m->KeyBuffer);

	InitializeDirectInput(m, (HWND)owner->GetWindowHandle());
}

Keyboard::~Keyboard()
{
	ReleaseDirectInput(m);

	B3DDelete(m);
}

void Keyboard::Capture()
{
	if(m->Keyboard == nullptr)
		return;

	DIDEVICEOBJECTDATA diBuff[DI_BUFFER_SIZE_KEYBOARD];
	DWORD entryCount = DI_BUFFER_SIZE_KEYBOARD;

	// Note: Only one keyboard per app due to this static (which is fine)
	static bool verifyAfterAltTab = false;

	HRESULT hr = m->Keyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &entryCount, 0);
	if(hr != DI_OK)
	{
		hr = m->Keyboard->Acquire();
		if(hr == E_ACCESSDENIED)
			verifyAfterAltTab = true;

		while(hr == DIERR_INPUTLOST)
			hr = m->Keyboard->Acquire();

		return;
	}

	if(FAILED(hr))
	{
		B3D_LOG(Error, Platform, "Failed to read keyboard input. Internal error. ");
		return;
	}

	for(u32 entryIndex = 0; entryIndex < entryCount; ++entryIndex)
	{
		ButtonCode buttonCode = (ButtonCode)diBuff[entryIndex].dwOfs;

		m->KeyBuffer[(u32)buttonCode] = (u8)(diBuff[entryIndex].dwData);

		if(diBuff[entryIndex].dwData & 0x80)
			mOwner->NotifyButtonPressedInternal(0, buttonCode, diBuff[entryIndex].dwTimeStamp);
		else
			mOwner->NotifyButtonReleasedInternal(0, buttonCode, diBuff[entryIndex].dwTimeStamp);
	}

	// If a lost device/access denied was detected, recover
	if(verifyAfterAltTab)
	{
		// Store old buffer
		u8 keyBufferCopy[256];
		memcpy(keyBufferCopy, m->KeyBuffer, 256);

		// Read immediate state
		hr = m->Keyboard->GetDeviceState(sizeof(m->KeyBuffer), &m->KeyBuffer);

		if(hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
		{
			hr = m->Keyboard->Acquire();
			if(hr != DIERR_OTHERAPPHASPRIO)
				m->Keyboard->GetDeviceState(sizeof(m->KeyBuffer), &m->KeyBuffer);
		}

		for(u32 keyIndex = 0; keyIndex < 256; keyIndex++)
		{
			if(keyBufferCopy[keyIndex] != m->KeyBuffer[keyIndex])
			{
				if(m->KeyBuffer[keyIndex])
					mOwner->NotifyButtonPressedInternal(0, (ButtonCode)keyIndex, GetTickCount64());
				else
					mOwner->NotifyButtonReleasedInternal(0, (ButtonCode)keyIndex, GetTickCount64());
			}
		}

		verifyAfterAltTab = false;
	}
}

void Keyboard::ChangeCaptureContext(u64 windowHandle)
{
	HWND newWindowHandle = (HWND)windowHandle;

	if(m->HWnd != newWindowHandle)
	{
		ReleaseDirectInput(m);

		if(windowHandle != (u64)-1)
			InitializeDirectInput(m, newWindowHandle);
		else
			m->HWnd = (HWND)-1;
	}
}
