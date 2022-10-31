//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/BsMouse.h"
#include "Input/BsInput.h"
#include "Private/Win32/BsWin32Input.h"
#include "Error/BsException.h"

using namespace bs;

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
void initializeDirectInput(Mouse::Pimpl* m, HWND hWnd)
{
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = DI_BUFFER_SIZE_MOUSE;

	if(FAILED(m->DirectInput->CreateDevice(GUID_SysMouse, &m->Mouse, nullptr)))
		BS_EXCEPT(InternalErrorException, "DirectInput mouse init: Failed to create device.");

	if(FAILED(m->Mouse->SetDataFormat(&c_dfDIMouse2)))
		BS_EXCEPT(InternalErrorException, "DirectInput mouse init: Failed to set format.");

	if(FAILED(m->Mouse->SetCooperativeLevel(hWnd, m->CoopSettings)))
		BS_EXCEPT(InternalErrorException, "DirectInput mouse init: Failed to set coop level.");

	if(FAILED(m->Mouse->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
		BS_EXCEPT(InternalErrorException, "DirectInput mouse init: Failed to set property.");

	HRESULT hr = m->Mouse->Acquire();
	if(FAILED(hr) && hr != DIERR_OTHERAPPHASPRIO)
		BS_EXCEPT(InternalErrorException, "DirectInput mouse init: Failed to acquire device.");

	m->HWnd = hWnd;
}

/** Releases DirectInput resources for the provided device */
void releaseDirectInput(Mouse::Pimpl* m)
{
	if(m->Mouse)
	{
		m->Mouse->Unacquire();
		m->Mouse->Release();
		m->Mouse = nullptr;
	}
}

/** Notifies the input handler that a mouse press or release occurred. Triggers an event in the input handler. */
void doMouseClick(Input* owner, ButtonCode mouseButton, const DIDEVICEOBJECTDATA& data)
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

	initializeDirectInput(m, (HWND)owner->GetWindowHandleInternal());
}

Mouse::~Mouse()
{
	releaseDirectInput(m);

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

	i32 relX, relY, relZ;
	relX = relY = relZ = 0;

	bool axesMoved = false;
	for(u32 i = 0; i < numEntries; ++i)
	{
		switch(diBuff[i].dwOfs)
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

	if(axesMoved)
		mOwner->NotifyMouseMovedInternal(relX, relY, relZ);
}

void Mouse::ChangeCaptureContext(u64 windowHandle)
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
