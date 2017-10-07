//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Input/BsKeyboard.h"
#include "Input/BsInput.h"
#include "Win32/BsWin32Input.h"
#include "Error/BsException.h"

namespace bs
{
	/** Contains private data for the Win32 Keyboard implementation. */
	struct Keyboard::Pimpl
	{
		IDirectInput8* directInput;
		IDirectInputDevice8* keyboard;
		DWORD coopSettings;
		HWND hWnd;

		UINT8 keyBuffer[256];
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

		if (FAILED(m->directInput->CreateDevice(GUID_SysKeyboard, &m->keyboard, nullptr)))
			BS_EXCEPT(InternalErrorException, "DirectInput keyboard init: Failed to create device.");

		if (FAILED(m->keyboard->SetDataFormat(&c_dfDIKeyboard)))
			BS_EXCEPT(InternalErrorException, "DirectInput keyboard init: Failed to set format.");

		if (FAILED(m->keyboard->SetCooperativeLevel(hWnd, m->coopSettings)))
			BS_EXCEPT(InternalErrorException, "DirectInput keyboard init: Failed to set coop level.");

		if (FAILED(m->keyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
			BS_EXCEPT(InternalErrorException, "DirectInput keyboard init: Failed to set property.");

		HRESULT hr = m->keyboard->Acquire();
		if (FAILED(hr) && hr != DIERR_OTHERAPPHASPRIO)
			BS_EXCEPT(InternalErrorException, "DirectInput keyboard init: Failed to acquire device.");

		m->hWnd = hWnd;
	}

	/** Releases DirectInput resources for the provided device */
	void releaseDirectInput(Keyboard::Pimpl* m)
	{
		if(m->keyboard)
		{
			m->keyboard->Unacquire();
			m->keyboard->Release();
			m->keyboard = nullptr;
		}
	}

	Keyboard::Keyboard(const String& name, Input* owner)
		: mName(name), mOwner(owner)
	{
		InputPrivateData* pvtData = owner->_getPrivateData();

		m = bs_new<Pimpl>();
		m->directInput = pvtData->directInput;
		m->coopSettings = pvtData->kbSettings;
		m->keyboard = nullptr;
		bs_zero_out(m->keyBuffer);

		initializeDirectInput(m, (HWND)owner->_getWindowHandle());
	}

	Keyboard::~Keyboard()
	{
		releaseDirectInput(m);

		bs_delete(m);
	}

	void Keyboard::capture()
	{
		DIDEVICEOBJECTDATA diBuff[DI_BUFFER_SIZE_KEYBOARD];
		DWORD numEntries = DI_BUFFER_SIZE_KEYBOARD;

		// Note: Only one keyboard per app due to this static (which is fine)
		static bool verifyAfterAltTab = false;

		HRESULT hr = m->keyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &numEntries, 0);
		if (hr != DI_OK)
		{
			hr = m->keyboard->Acquire();
			if (hr == E_ACCESSDENIED)
				verifyAfterAltTab = true;

			while (hr == DIERR_INPUTLOST)
				hr = m->keyboard->Acquire();

			return;
		}

		if (FAILED(hr))
		{
			LOGERR("Failed to read keyboard input. Internal error. ");
			return;
		}

		for (UINT32 i = 0; i < numEntries; ++i)
		{
			ButtonCode buttonCode = (ButtonCode)diBuff[i].dwOfs;

			m->keyBuffer[buttonCode] = (UINT8)(diBuff[i].dwData);

			if (diBuff[i].dwData & 0x80)
				mOwner->_notifyButtonPressed(0, buttonCode, diBuff[i].dwTimeStamp);
			else
				mOwner->_notifyButtonReleased(0, buttonCode, diBuff[i].dwTimeStamp);
		}

		// If a lost device/access denied was detected, recover
		if (verifyAfterAltTab)
		{
			// Store old buffer
			UINT8 keyBufferCopy[256];
			memcpy(keyBufferCopy, m->keyBuffer, 256);

			// Read immediate state
			hr = m->keyboard->GetDeviceState(sizeof(m->keyBuffer), &m->keyBuffer);

			if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
			{
				hr = m->keyboard->Acquire();
				if (hr != DIERR_OTHERAPPHASPRIO)
					m->keyboard->GetDeviceState(sizeof(m->keyBuffer), &m->keyBuffer);
			}

			for (UINT32 i = 0; i < 256; i++)
			{
				if (keyBufferCopy[i] != m->keyBuffer[i])
				{
					if (m->keyBuffer[i])
						mOwner->_notifyButtonPressed(0, (ButtonCode)i, GetTickCount64());
					else
						mOwner->_notifyButtonReleased(0, (ButtonCode)i, GetTickCount64());
				}
			}

			verifyAfterAltTab = false;
		}
	}

	void Keyboard::changeCaptureContext(UINT64 windowHandle)
	{
		HWND newhWnd = (HWND)windowHandle;

		if(m->hWnd != newhWnd)
		{
			releaseDirectInput(m);
			initializeDirectInput(m, newhWnd);
		}
	}
}
