//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/BsKeyboard.h"
#include "Input/BsInput.h"
#include "Private/Win32/BsWin32Input.h"
#include "Error/BsException.h"

using namespace bs;

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
void initializeDirectInput(Keyboard::Pimpl* m, HWND hWnd)
{
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = DI_BUFFER_SIZE_KEYBOARD;

	if(FAILED(m->DirectInput->CreateDevice(GUID_SysKeyboard, &m->Keyboard, nullptr)))
		BS_EXCEPT(InternalErrorException, "DirectInput keyboard init: Failed to create device.");

	if(FAILED(m->Keyboard->SetDataFormat(&c_dfDIKeyboard)))
		BS_EXCEPT(InternalErrorException, "DirectInput keyboard init: Failed to set format.");

	if(FAILED(m->Keyboard->SetCooperativeLevel(hWnd, m->CoopSettings)))
		BS_EXCEPT(InternalErrorException, "DirectInput keyboard init: Failed to set coop level.");

	if(FAILED(m->Keyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
		BS_EXCEPT(InternalErrorException, "DirectInput keyboard init: Failed to set property.");

	HRESULT hr = m->Keyboard->Acquire();
	if(FAILED(hr) && hr != DIERR_OTHERAPPHASPRIO)
		BS_EXCEPT(InternalErrorException, "DirectInput keyboard init: Failed to acquire device.");

	m->HWnd = hWnd;
}

/** Releases DirectInput resources for the provided device */
void releaseDirectInput(Keyboard::Pimpl* m)
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

	m = bs_new<Pimpl>();
	m->DirectInput = pvtData->DirectInput;
	m->CoopSettings = pvtData->KbSettings;
	m->Keyboard = nullptr;
	bs_zero_out(m->KeyBuffer);

	initializeDirectInput(m, (HWND)owner->GetWindowHandleInternal());
}

Keyboard::~Keyboard()
{
	releaseDirectInput(m);

	bs_delete(m);
}

void Keyboard::Capture()
{
	if(m->Keyboard == nullptr)
		return;

	DIDEVICEOBJECTDATA diBuff[DI_BUFFER_SIZE_KEYBOARD];
	DWORD numEntries = DI_BUFFER_SIZE_KEYBOARD;

	// Note: Only one keyboard per app due to this static (which is fine)
	static bool verifyAfterAltTab = false;

	HRESULT hr = m->Keyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &numEntries, 0);
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
		BS_LOG(Error, Platform, "Failed to read keyboard input. Internal error. ");
		return;
	}

	for(u32 i = 0; i < numEntries; ++i)
	{
		ButtonCode buttonCode = (ButtonCode)diBuff[i].dwOfs;

		m->KeyBuffer[buttonCode] = (u8)(diBuff[i].dwData);

		if(diBuff[i].dwData & 0x80)
			mOwner->NotifyButtonPressedInternal(0, buttonCode, diBuff[i].dwTimeStamp);
		else
			mOwner->NotifyButtonReleasedInternal(0, buttonCode, diBuff[i].dwTimeStamp);
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

		for(u32 i = 0; i < 256; i++)
		{
			if(keyBufferCopy[i] != m->KeyBuffer[i])
			{
				if(m->KeyBuffer[i])
					mOwner->NotifyButtonPressedInternal(0, (ButtonCode)i, GetTickCount64());
				else
					mOwner->NotifyButtonReleasedInternal(0, (ButtonCode)i, GetTickCount64());
			}
		}

		verifyAfterAltTab = false;
	}
}

void Keyboard::ChangeCaptureContext(u64 windowHandle)
{
	HWND newhWnd = (HWND)windowHandle;

	if(m->HWnd != newhWnd)
	{
		releaseDirectInput(m);

		if(windowHandle != (u64)-1)
			initializeDirectInput(m, newhWnd);
		else
			m->HWnd = (HWND)-1;
	}
}
