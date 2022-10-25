//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/BsGamepad.h"
#include "Input/BsInput.h"
#include "Private/Win32/BsWin32Input.h"
#include "Error/BsException.h"

#undef DIJOFS_BUTTON
#define DIJOFS_BUTTON(n) (FIELD_OFFSET(DIJOYSTATE2, rgbButtons) + (n))

using namespace bs;

/** Contains state of a POV (DPad). */
struct POVState
{
	ButtonCode Code;
	bool Pressed;
};

/** Contains private data for the Win32 Gamepad implementation. */
struct Gamepad::Pimpl
{
	IDirectInput8* DirectInput;
	IDirectInputDevice8* Gamepad;
	GamepadInfo Info;
	DWORD CoopSettings;
	HWND HWnd;

	POVState PovState[4];
	i32 AxisState[6]; // Only for XInput
	bool ButtonState[16]; // Only for XInput
};

/**
 * Initializes DirectInput gamepad device for a window with the specified handle. Only input from that window will be
 * reported.
 */
void initializeDirectInput(Gamepad::Pimpl* m, HWND hWnd)
{
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = DI_BUFFER_SIZE_GAMEPAD;

	if(FAILED(m->DirectInput->CreateDevice(m->Info.GuidInstance, &m->Gamepad, nullptr)))
		BS_EXCEPT(InternalErrorException, "DirectInput gamepad init: Failed to create device.");

	if(FAILED(m->Gamepad->SetDataFormat(&c_dfDIJoystick2)))
		BS_EXCEPT(InternalErrorException, "DirectInput gamepad init: Failed to set format.");

	if(FAILED(m->Gamepad->SetCooperativeLevel(hWnd, m->CoopSettings)))
		BS_EXCEPT(InternalErrorException, "DirectInput gamepad init: Failed to set coop level.");

	if(FAILED(m->Gamepad->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
		BS_EXCEPT(InternalErrorException, "DirectInput gamepad init: Failed to set property.");

	HRESULT hr = m->Gamepad->Acquire();
	if(FAILED(hr) && hr != DIERR_OTHERAPPHASPRIO)
		BS_EXCEPT(InternalErrorException, "DirectInput gamepad init: Failed to acquire device.");

	m->HWnd = hWnd;
}

/** Releases DirectInput resources for the provided device */
void releaseDirectInput(Gamepad::Pimpl* m)
{
	if(m->Gamepad)
	{
		m->Gamepad->Unacquire();
		m->Gamepad->Release();
		m->Gamepad = nullptr;
	}
}

/** Handles a DirectInput POV event. */
void handlePOV(Input* owner, Gamepad::Pimpl* m, int pov, DIDEVICEOBJECTDATA& di)
{
	if(LOWORD(di.dwData) == 0xFFFF)
	{
		// Centered, release any buttons
		if(m->PovState[pov].Pressed)
		{
			owner->NotifyButtonReleasedInternal(m->Info.Id, m->PovState[pov].Code, di.dwTimeStamp);
			m->PovState[pov].Pressed = false;
		}
	}
	else
	{
		POVState newPOVState;
		bs_zero_out(newPOVState);

		switch(di.dwData)
		{
		case 0:
			newPOVState.Code = BC_GAMEPAD_DPAD_UP;
			newPOVState.Pressed = true;
			break;
		case 4500:
			newPOVState.Code = BC_GAMEPAD_DPAD_UPRIGHT;
			newPOVState.Pressed = true;
			break;
		case 9000:
			newPOVState.Code = BC_GAMEPAD_DPAD_RIGHT;
			newPOVState.Pressed = true;
			break;
		case 13500:
			newPOVState.Code = BC_GAMEPAD_DPAD_DOWNRIGHT;
			newPOVState.Pressed = true;
			break;
		case 18000:
			newPOVState.Code = BC_GAMEPAD_DPAD_DOWN;
			newPOVState.Pressed = true;
			break;
		case 22500:
			newPOVState.Code = BC_GAMEPAD_DPAD_DOWNLEFT;
			newPOVState.Pressed = true;
			break;
		case 27000:
			newPOVState.Code = BC_GAMEPAD_DPAD_LEFT;
			newPOVState.Pressed = true;
			break;
		case 31500:
			newPOVState.Code = BC_GAMEPAD_DPAD_UPLEFT;
			newPOVState.Pressed = true;
			break;
		}

		// Button was pressed
		if(newPOVState.Pressed)
		{
			// Another button was previously pressed
			if(m->PovState[pov].Pressed)
			{
				// If its a different button, release the old one and press the new one
				if(m->PovState[pov].Code != newPOVState.Code)
				{
					owner->NotifyButtonReleasedInternal(m->Info.Id, m->PovState[pov].Code, di.dwTimeStamp);
					owner->NotifyButtonPressedInternal(m->Info.Id, newPOVState.Code, di.dwTimeStamp);

					m->PovState[pov].Code = newPOVState.Code;
				}
			}
			else
			{
				owner->NotifyButtonPressedInternal(m->Info.Id, newPOVState.Code, di.dwTimeStamp);
				m->PovState[pov].Code = newPOVState.Code;
				m->PovState[pov].Pressed = true;
			}
		}
	}
}

/** Converts a DirectInput or XInput button code to BSF ButtonCode. */
ButtonCode gamepadButtonToButtonCode(i32 code)
{
	switch(code)
	{
	case 0:
		return BC_GAMEPAD_DPAD_UP;
	case 1:
		return BC_GAMEPAD_DPAD_DOWN;
	case 2:
		return BC_GAMEPAD_DPAD_LEFT;
	case 3:
		return BC_GAMEPAD_DPAD_RIGHT;
	case 4:
		return BC_GAMEPAD_START;
	case 5:
		return BC_GAMEPAD_BACK;
	case 6:
		return BC_GAMEPAD_LS;
	case 7:
		return BC_GAMEPAD_RS;
	case 8:
		return BC_GAMEPAD_LB;
	case 9:
		return BC_GAMEPAD_RB;
	case 10:
		return BC_GAMEPAD_BTN1;
	case 11:
		return BC_GAMEPAD_LS;
	case 12:
		return BC_GAMEPAD_A;
	case 13:
		return BC_GAMEPAD_B;
	case 14:
		return BC_GAMEPAD_X;
	case 15:
		return BC_GAMEPAD_Y;
	}

	return (ButtonCode)(BC_GAMEPAD_BTN1 + (code - 15));
}

Gamepad::Gamepad(const String& name, const GamepadInfo& gamepadInfo, Input* owner)
	: mName(name), mOwner(owner)
{
	InputPrivateData* pvtData = owner->GetPrivateDataInternal();

	m = bs_new<Pimpl>();
	m->DirectInput = pvtData->DirectInput;
	m->CoopSettings = pvtData->MouseSettings;
	m->Info = gamepadInfo;
	m->Gamepad = nullptr;
	m->HWnd = (HWND)owner->GetWindowHandleInternal();
	bs_zero_out(m->PovState);
	bs_zero_out(m->AxisState);
	bs_zero_out(m->ButtonState);

	if(!m->Info.IsXInput)
		initializeDirectInput(m, m->HWnd);
}

Gamepad::~Gamepad()
{
	releaseDirectInput(m);

	bs_delete(m);
}

void Gamepad::Capture()
{
	if(m->HWnd == (HWND)-1 || m->Gamepad == nullptr)
		return;

	if(m->Info.IsXInput)
	{
		XINPUT_STATE inputState;
		if(XInputGetState((DWORD)m->Info.XInputDev, &inputState) != ERROR_SUCCESS)
			memset(&inputState, 0, sizeof(inputState));

		// Sticks and triggers
		struct AxisState
		{
			bool Moved;
			i32 Value;
		};

		AxisState axisState[6];
		bs_zero_out(axisState);

		// Note: Order of axes must match InputAxis enum
		// Left stick
		axisState[0].Value = (int)inputState.Gamepad.sThumbLX;
		axisState[1].Value = -(int)inputState.Gamepad.sThumbLY;

		// Right stick
		axisState[2].Value = (int)inputState.Gamepad.sThumbRX;
		axisState[3].Value = -(int)inputState.Gamepad.sThumbRY;

		// Left trigger
		axisState[4].Value = std::min((int)inputState.Gamepad.bLeftTrigger * 129, MAX_AXIS);

		// Right trigger
		axisState[5].Value = std::min((int)inputState.Gamepad.bRightTrigger * 129, MAX_AXIS);

		for(u32 i = 0; i < 6; i++)
		{
			axisState[i].Moved = axisState[i].Value != m->AxisState[i];
			m->AxisState[i] = axisState[i].Value;
		}

		// DPAD (POV)
		ButtonCode dpadButton = BC_UNASSIGNED;
		if((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0)
			dpadButton = BC_GAMEPAD_DPAD_UP;
		else if((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0)
			dpadButton = BC_GAMEPAD_DPAD_DOWN;
		if((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0)
			dpadButton = BC_GAMEPAD_DPAD_LEFT;
		else if((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0)
			dpadButton = BC_GAMEPAD_DPAD_RIGHT;

		if(dpadButton != BC_UNASSIGNED) // Pressed
		{
			// Another button was previously pressed
			if(m->PovState[0].Pressed)
			{
				// If its a different button, release the old one and press the new one
				if(m->PovState[0].Code != dpadButton)
				{
					mOwner->NotifyButtonReleasedInternal(m->Info.Id, m->PovState[0].Code, GetTickCount64());
					mOwner->NotifyButtonPressedInternal(m->Info.Id, dpadButton, GetTickCount64());

					m->PovState[0].Code = dpadButton;
				}
			}
			else
			{
				mOwner->NotifyButtonPressedInternal(m->Info.Id, dpadButton, GetTickCount64());
				m->PovState[0].Code = dpadButton;
				m->PovState[0].Pressed = true;
			}
		}
		else
		{
			if(m->PovState[0].Pressed)
			{
				mOwner->NotifyButtonReleasedInternal(m->Info.Id, m->PovState[0].Code, GetTickCount64());
				m->PovState[0].Pressed = false;
			}
		}

		// Buttons
		for(u32 i = 0; i < 16; i++)
		{
			bool buttonState = (inputState.Gamepad.wButtons & (1 << i)) != 0;

			if(buttonState != m->ButtonState[i])
			{
				if(buttonState)
					mOwner->NotifyButtonPressedInternal(m->Info.Id, gamepadButtonToButtonCode(i), GetTickCount64());
				else
					mOwner->NotifyButtonReleasedInternal(m->Info.Id, gamepadButtonToButtonCode(i), GetTickCount64());

				m->ButtonState[i] = buttonState;
			}
		}

		for(int i = 0; i < 6; ++i)
		{
			if(!axisState[i].Moved)
				continue;

			mOwner->NotifyAxisMovedInternal(m->Info.Id, i + (int)InputAxis::MouseZ, axisState[i].Value);
		}
	}
	else // DirectInput
	{
		DIDEVICEOBJECTDATA diBuff[DI_BUFFER_SIZE_GAMEPAD];
		DWORD numEntries = DI_BUFFER_SIZE_GAMEPAD;

		HRESULT hr = m->Gamepad->Poll();
		if(hr == DI_OK)
			hr = m->Gamepad->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &numEntries, 0);

		if(hr != DI_OK)
		{
			hr = m->Gamepad->Acquire();
			while(hr == DIERR_INPUTLOST)
				hr = m->Gamepad->Acquire();

			m->Gamepad->Poll();
			hr = m->Gamepad->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &numEntries, 0);

			if(FAILED(hr))
				return;
		}

		struct AxisState
		{
			bool Moved;
			i32 Value;
		};

		AxisState axisState[24];
		bs_zero_out(axisState);

		// Note: Not reporting slider or POV events
		for(u32 i = 0; i < numEntries; ++i)
		{
			switch(diBuff[i].dwOfs)
			{
			case DIJOFS_POV(0):
				handlePOV(mOwner, m, 0, diBuff[i]);
				break;
			case DIJOFS_POV(1):
				handlePOV(mOwner, m, 1, diBuff[i]);
				break;
			case DIJOFS_POV(2):
				handlePOV(mOwner, m, 2, diBuff[i]);
				break;
			case DIJOFS_POV(3):
				handlePOV(mOwner, m, 3, diBuff[i]);
				break;
			default:
				// Button event
				if(diBuff[i].dwOfs >= DIJOFS_BUTTON(0) && diBuff[i].dwOfs < DIJOFS_BUTTON(128))
				{
					int button = diBuff[i].dwOfs - DIJOFS_BUTTON(0);

					if((diBuff[i].dwData & 0x80) != 0)
						mOwner->NotifyButtonPressedInternal(m->Info.Id, gamepadButtonToButtonCode(button), diBuff[i].dwTimeStamp);
					else
						mOwner->NotifyButtonReleasedInternal(m->Info.Id, gamepadButtonToButtonCode(button), diBuff[i].dwTimeStamp);
				}
				else if((short)(diBuff[i].uAppData >> 16) == 0x1313) // Axis event
				{
					int axis = (int)(0x0000FFFF & diBuff[i].uAppData);
					if(axis < 24)
					{
						axisState[axis].Moved = true;
						axisState[axis].Value = diBuff[i].dwData;
					}
				}
			}
		}

		if(numEntries > 0)
		{
			for(int i = 0; i < 24; ++i)
			{
				if(!axisState[i].Moved)
					continue;

				mOwner->NotifyAxisMovedInternal(m->Info.Id, i + (int)InputAxis::MouseZ, axisState[i].Value);
			}
		}
	}
}

void Gamepad::ChangeCaptureContext(u64 windowHandle)
{
	HWND newhWnd = (HWND)windowHandle;

	if(m->HWnd != newhWnd)
	{
		releaseDirectInput(m);

		if(!m->Info.IsXInput && windowHandle != (u64)-1)
			initializeDirectInput(m, newhWnd);
		else
			m->HWnd = newhWnd;
	}
}
