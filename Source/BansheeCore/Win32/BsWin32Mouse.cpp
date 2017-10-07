//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Input/BsMouse.h"
#include "Input/BsRawInputHandler.h"
#include "Win32/BsWin32Input.h"
#include "Error/BsException.h"

namespace bs
{
	/** Contains private data for the Win32 Mouse implementation. */
	struct Mouse::Pimpl
	{
		IDirectInput8* directInput;
		IDirectInputDevice8* mouse;
		DWORD coopSettings;
		HWND hWnd;
	};

	/** 
	 * Initializes DirectInput mouse device for a window with the specified handle. Only input from that window will be 
	 * reported. 
	 */
	void initializeDirectInput(Mouse::Pimpl* m, HWND hWnd)
	{
		DIPROPDWORD dipdw;
		dipdw.diph.dwSize = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj = 0;
		dipdw.diph.dwHow = DIPH_DEVICE;
		dipdw.dwData = DI_BUFFER_SIZE_MOUSE;

		if (FAILED(m->directInput->CreateDevice(GUID_SysMouse, &m->mouse, nullptr)))
			BS_EXCEPT(InternalErrorException, "DirectInput mouse init: Failed to create device.");

		if (FAILED(m->mouse->SetDataFormat(&c_dfDIMouse2)))
			BS_EXCEPT(InternalErrorException, "DirectInput mouse init: Failed to set format.");

		if (FAILED(m->mouse->SetCooperativeLevel(hWnd, m->coopSettings)))
			BS_EXCEPT(InternalErrorException, "DirectInput mouse init: Failed to set coop level.");

		if (FAILED(m->mouse->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
			BS_EXCEPT(InternalErrorException, "DirectInput mouse init: Failed to set property.");

		HRESULT hr = m->mouse->Acquire();
		if (FAILED(hr) && hr != DIERR_OTHERAPPHASPRIO)
			BS_EXCEPT(InternalErrorException, "DirectInput mouse init: Failed to acquire device.");

		m->hWnd = hWnd;
	}

	/** Releases DirectInput resources for the provided device */
	void releaseDirectInput(Mouse::Pimpl* m)
	{
		if(m->mouse)
		{
			m->mouse->Unacquire();
			m->mouse->Release();
			m->mouse = nullptr;
		}
	}

	/** Notifies the input handler that a mouse press or release occurred. Triggers an event in the input handler. */
	void doMouseClick(RawInputHandler* owner, ButtonCode mouseButton, const DIDEVICEOBJECTDATA& data)
	{
		if (data.dwData & 0x80)
			owner->_notifyButtonPressed(0, mouseButton, data.dwTimeStamp);
		else
			owner->_notifyButtonReleased(0, mouseButton, data.dwTimeStamp);
	}

	Mouse::Mouse(const String& name, RawInputHandler* owner)
		: mName(name), mOwner(owner)
	{
		InputPrivateData* pvtData = owner->_getPrivateData();

		m = bs_new<Pimpl>();
		m->directInput = pvtData->directInput;
		m->coopSettings = pvtData->mouseSettings;
		m->mouse = nullptr;

		initializeDirectInput(m, (HWND)owner->_getWindowHandle());
	}

	Mouse::~Mouse()
	{
		releaseDirectInput(m);

		bs_delete(m);
	}

	void Mouse::capture()
	{
		DIDEVICEOBJECTDATA diBuff[DI_BUFFER_SIZE_MOUSE];
		DWORD numEntries = DI_BUFFER_SIZE_MOUSE;

		HRESULT hr = m->mouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &numEntries, 0);
		if (hr != DI_OK)
		{
			hr = m->mouse->Acquire();
			while (hr == DIERR_INPUTLOST)
				hr = m->mouse->Acquire();

			hr = m->mouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &numEntries, 0);

			if (FAILED(hr))
				return;
		}

		INT32 relX, relY, relZ;
		relX = relY = relZ = 0;

		bool axesMoved = false;
		for (UINT32 i = 0; i < numEntries; ++i)
		{
			switch (diBuff[i].dwOfs)
			{
			case DIMOFS_BUTTON0:
				doMouseClick(mOwner, BC_MOUSE_LEFT, diBuff[i]);
				break;
			case DIMOFS_BUTTON1:
				doMouseClick(mOwner, BC_MOUSE_RIGHT, diBuff[i]);
				break;
			case DIMOFS_BUTTON2:
				doMouseClick(mOwner, BC_MOUSE_MIDDLE, diBuff[i]);
				break;
			case DIMOFS_BUTTON3:
				doMouseClick(mOwner, BC_MOUSE_BTN4, diBuff[i]);
				break;
			case DIMOFS_BUTTON4:
				doMouseClick(mOwner, BC_MOUSE_BTN5, diBuff[i]);
				break;
			case DIMOFS_BUTTON5:
				doMouseClick(mOwner, BC_MOUSE_BTN6, diBuff[i]);
				break;
			case DIMOFS_BUTTON6:
				doMouseClick(mOwner, BC_MOUSE_BTN7, diBuff[i]);
				break;
			case DIMOFS_BUTTON7:
				doMouseClick(mOwner, BC_MOUSE_BTN8, diBuff[i]);
				break;
			case DIMOFS_X:
				relX += diBuff[i].dwData;
				axesMoved = true;
				break;
			case DIMOFS_Y:
				relY += diBuff[i].dwData;
				axesMoved = true;
				break;
			case DIMOFS_Z:
				relZ += diBuff[i].dwData;
				axesMoved = true;
				break;
			default: break;
			}
		}

		if (axesMoved)
			mOwner->_notifyMouseMoved(relX, relY, relZ);

	}

	void Mouse::changeCaptureContext(UINT64 windowHandle)
	{
		HWND newhWnd = (HWND)windowHandle;

		if(m->hWnd != newhWnd)
		{
			releaseDirectInput(m);
			initializeDirectInput(m, newhWnd);
		}
	}
}