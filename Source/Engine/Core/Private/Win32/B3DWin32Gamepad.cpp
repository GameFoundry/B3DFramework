//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/B3DGamepad.h"
#include "Input/B3DInput.h"
#include "Private/Win32/B3DWin32Input.h"
#include "Error/B3DException.h"

#undef DIJOFS_BUTTON
#define DIJOFS_BUTTON(n) (FIELD_OFFSET(DIJOYSTATE2, rgbButtons) + (n))

using namespace b3d;

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
void InitializeDirectInput(Gamepad::Pimpl* m, HWND hWnd)
{
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = DI_BUFFER_SIZE_GAMEPAD;

	HRESULT result = m->DirectInput->CreateDevice(m->Info.GuidInstance, &m->Gamepad, nullptr);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput gamepad init: Failed to create device. Error code: {0}.", (u64)result);
		return;
	}

	result = m->Gamepad->SetDataFormat(&c_dfDIJoystick2);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput gamepad init: Failed to set format. Error code: {0}.", (u64)result);
		return;
	}

	result = m->Gamepad->SetCooperativeLevel(hWnd, m->CoopSettings);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput gamepad init: Failed to set coop level. Error code: {0}.", (u64)result);
		return;
	}

	result = m->Gamepad->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput gamepad init: Failed to set property. Error code: {0}.", (u64)result);
		return;
	}

	result = m->Gamepad->Acquire();
	if(FAILED(result) && result != DIERR_OTHERAPPHASPRIO)
	{
		B3D_LOG(Error, LogInput, "DirectInput gamepad init: Failed to acquire device. Error code: {0}.", (u64)result);
		return;
	}

	m->HWnd = hWnd;
}

/** Releases DirectInput resources for the provided device */
void ReleaseDirectInput(Gamepad::Pimpl* m)
{
	if(m->Gamepad)
	{
		m->Gamepad->Unacquire();
		m->Gamepad->Release();
		m->Gamepad = nullptr;
	}
}

/** Handles a DirectInput POV event. */
void HandlePov(Input* owner, Gamepad::Pimpl* m, int pov, DIDEVICEOBJECTDATA& di)
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
		B3DZeroOut(newPOVState);

		switch(di.dwData)
		{
		case 0:
			newPOVState.Code = ButtonCode::GamepadDPadUp;
			newPOVState.Pressed = true;
			break;
		case 4500:
			newPOVState.Code = ButtonCode::GamepadDPadUpRight;
			newPOVState.Pressed = true;
			break;
		case 9000:
			newPOVState.Code = ButtonCode::GamepadDPatRight;
			newPOVState.Pressed = true;
			break;
		case 13500:
			newPOVState.Code = ButtonCode::GamepadDPadDownRight;
			newPOVState.Pressed = true;
			break;
		case 18000:
			newPOVState.Code = ButtonCode::GamepadDPadDown;
			newPOVState.Pressed = true;
			break;
		case 22500:
			newPOVState.Code = ButtonCode::GamepadDPadDownLeft;
			newPOVState.Pressed = true;
			break;
		case 27000:
			newPOVState.Code = ButtonCode::GamepadDPadLeft;
			newPOVState.Pressed = true;
			break;
		case 31500:
			newPOVState.Code = ButtonCode::GamepadDPadUpLeft;
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
ButtonCode GamepadButtonToButtonCode(i32 code)
{
	switch(code)
	{
	case 0:
		return ButtonCode::GamepadDPadUp;
	case 1:
		return ButtonCode::GamepadDPadDown;
	case 2:
		return ButtonCode::GamepadDPadLeft;
	case 3:
		return ButtonCode::GamepadDPatRight;
	case 4:
		return ButtonCode::GamepadStart;
	case 5:
		return ButtonCode::GamepadBack;
	case 6:
		return ButtonCode::GamepadLeftStick;
	case 7:
		return ButtonCode::GamepadRightStick;
	case 8:
		return ButtonCode::GamepadLeftBumper;
	case 9:
		return ButtonCode::GamepadRightBumper;
	case 10:
		return ButtonCode::GamepadButton1;
	case 11:
		return ButtonCode::GamepadLeftStick;
	case 12:
		return ButtonCode::GamepadA;
	case 13:
		return ButtonCode::GamepadB;
	case 14:
		return ButtonCode::GamepadX;
	case 15:
		return ButtonCode::GamepadY;
	}

	return (ButtonCode)(static_cast<unsigned>(ButtonCode::GamepadButton1) + (code - 15));
}

Gamepad::Gamepad(const String& name, const GamepadInfo& gamepadInfo, Input* owner)
	: mName(name), mOwner(owner)
{
	InputPrivateData* pvtData = owner->GetPrivateDataInternal();

	m = B3DNew<Pimpl>();
	m->DirectInput = pvtData->DirectInput;
	m->CoopSettings = pvtData->MouseSettings;
	m->Info = gamepadInfo;
	m->Gamepad = nullptr;
	m->HWnd = (HWND)owner->GetWindowHandle();
	B3DZeroOut(m->PovState);
	B3DZeroOut(m->AxisState);
	B3DZeroOut(m->ButtonState);

	// Don't initialize DirectInput for XInput devices or in headless mode (window handle == 0)
	if(!m->Info.IsXInput && m->HWnd != nullptr)
		InitializeDirectInput(m, m->HWnd);
}

Gamepad::~Gamepad()
{
	ReleaseDirectInput(m);

	B3DDelete(m);
}

void Gamepad::Capture()
{
	// Skip capture in headless mode or when focus is lost
	if(m->HWnd == nullptr || m->Gamepad == nullptr)
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
		B3DZeroOut(axisState);

		// Note: Order of axes must match InputAxis enum
		// Left stick
		axisState[0].Value = (int)inputState.Gamepad.sThumbLX;
		axisState[1].Value = -(int)inputState.Gamepad.sThumbLY;

		// Right stick
		axisState[2].Value = (int)inputState.Gamepad.sThumbRX;
		axisState[3].Value = -(int)inputState.Gamepad.sThumbRY;

		// Left trigger
		axisState[4].Value = std::min((int)inputState.Gamepad.bLeftTrigger * 129, kMaxAxis);

		// Right trigger
		axisState[5].Value = std::min((int)inputState.Gamepad.bRightTrigger * 129, kMaxAxis);

		for(u32 axisIndex = 0; axisIndex < 6; axisIndex++)
		{
			axisState[axisIndex].Moved = axisState[axisIndex].Value != m->AxisState[axisIndex];
			m->AxisState[axisIndex] = axisState[axisIndex].Value;
		}

		// DPAD (POV)
		ButtonCode dpadButton = ButtonCode::Unassigned;
		if((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0)
			dpadButton = ButtonCode::GamepadDPadUp;
		else if((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0)
			dpadButton = ButtonCode::GamepadDPadDown;
		if((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0)
			dpadButton = ButtonCode::GamepadDPadLeft;
		else if((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0)
			dpadButton = ButtonCode::GamepadDPatRight;

		if(dpadButton != ButtonCode::Unassigned) // Pressed
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
		for(u32 buttonIndex = 0; buttonIndex < 16; buttonIndex++)
		{
			bool buttonState = (inputState.Gamepad.wButtons & (1 << buttonIndex)) != 0;

			if(buttonState != m->ButtonState[buttonIndex])
			{
				if(buttonState)
					mOwner->NotifyButtonPressedInternal(m->Info.Id, GamepadButtonToButtonCode(buttonIndex), GetTickCount64());
				else
					mOwner->NotifyButtonReleasedInternal(m->Info.Id, GamepadButtonToButtonCode(buttonIndex), GetTickCount64());

				m->ButtonState[buttonIndex] = buttonState;
			}
		}

		for(int axisIndex = 0; axisIndex < 6; ++axisIndex)
		{
			if(!axisState[axisIndex].Moved)
				continue;

			mOwner->NotifyAxisMovedInternal(m->Info.Id, axisIndex + (int)InputAxis::MouseZ, axisState[axisIndex].Value);
		}
	}
	else // DirectInput
	{
		DIDEVICEOBJECTDATA diBuff[DI_BUFFER_SIZE_GAMEPAD];
		DWORD entryCount = DI_BUFFER_SIZE_GAMEPAD;

		HRESULT hr = m->Gamepad->Poll();
		if(hr == DI_OK)
			hr = m->Gamepad->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &entryCount, 0);

		if(hr != DI_OK)
		{
			hr = m->Gamepad->Acquire();
			while(hr == DIERR_INPUTLOST)
				hr = m->Gamepad->Acquire();

			m->Gamepad->Poll();
			hr = m->Gamepad->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &entryCount, 0);

			if(FAILED(hr))
				return;
		}

		struct AxisState
		{
			bool Moved;
			i32 Value;
		};

		AxisState axisState[24];
		B3DZeroOut(axisState);

		// Note: Not reporting slider or POV events
		for(u32 entryIndex = 0; entryIndex < entryCount; ++entryIndex)
		{
			switch(diBuff[entryIndex].dwOfs)
			{
			case DIJOFS_POV(0):
				HandlePov(mOwner, m, 0, diBuff[entryIndex]);
				break;
			case DIJOFS_POV(1):
				HandlePov(mOwner, m, 1, diBuff[entryIndex]);
				break;
			case DIJOFS_POV(2):
				HandlePov(mOwner, m, 2, diBuff[entryIndex]);
				break;
			case DIJOFS_POV(3):
				HandlePov(mOwner, m, 3, diBuff[entryIndex]);
				break;
			default:
				// Button event
				if(diBuff[entryIndex].dwOfs >= DIJOFS_BUTTON(0) && diBuff[entryIndex].dwOfs < DIJOFS_BUTTON(128))
				{
					int button = diBuff[entryIndex].dwOfs - DIJOFS_BUTTON(0);

					if((diBuff[entryIndex].dwData & 0x80) != 0)
						mOwner->NotifyButtonPressedInternal(m->Info.Id, GamepadButtonToButtonCode(button), diBuff[entryIndex].dwTimeStamp);
					else
						mOwner->NotifyButtonReleasedInternal(m->Info.Id, GamepadButtonToButtonCode(button), diBuff[entryIndex].dwTimeStamp);
				}
				else if((short)(diBuff[entryIndex].uAppData >> 16) == 0x1313) // Axis event
				{
					int axis = (int)(0x0000FFFF & diBuff[entryIndex].uAppData);
					if(axis < 24)
					{
						axisState[axis].Moved = true;
						axisState[axis].Value = diBuff[entryIndex].dwData;
					}
				}
			}
		}

		if(entryCount > 0)
		{
			for(int axisIndex = 0; axisIndex < 24; ++axisIndex)
			{
				if(!axisState[axisIndex].Moved)
					continue;

				mOwner->NotifyAxisMovedInternal(m->Info.Id, axisIndex + (int)InputAxis::MouseZ, axisState[axisIndex].Value);
			}
		}
	}
}

void Gamepad::ChangeCaptureContext(u64 windowHandle)
{
	HWND newWindowHandle = (HWND)windowHandle;

	if(m->HWnd != newWindowHandle)
	{
		ReleaseDirectInput(m);

		// Don't initialize DirectInput for invalid handles (headless mode or lost focus)
		if(!m->Info.IsXInput && windowHandle != 0)
			InitializeDirectInput(m, newWindowHandle);
		else
			m->HWnd = newWindowHandle;
	}
}
