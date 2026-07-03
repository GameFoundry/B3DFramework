//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/B3DInput.h"
#include "Private/Win32/B3DWin32Input.h"

#undef DIJOFS_BUTTON
#define DIJOFS_BUTTON(n) (FIELD_OFFSET(DIJOYSTATE2, rgbButtons) + (n))

using namespace b3d;

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

Win32Gamepad::Win32Gamepad(const GamepadInfo& gamepadInfo, Input& owner, IDirectInput8* directInput, DWORD coopSettings,
	u64 windowHandle)
	: mOwner(owner), mInfo(gamepadInfo), mDirectInput(directInput), mCoopSettings(coopSettings)
	, mHWnd((HWND)windowHandle)
{
	B3DZeroOut(mPovState);
	B3DZeroOut(mAxisState);
	B3DZeroOut(mButtonState);

	// Don't initialize DirectInput for XInput devices or in headless mode (window handle == 0)
	if(!mInfo.IsXInput && mHWnd != nullptr)
		InitializeDirectInput(mHWnd);
}

Win32Gamepad::~Win32Gamepad()
{
	ReleaseDirectInput();
}

void Win32Gamepad::InitializeDirectInput(HWND hWnd)
{
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = DI_BUFFER_SIZE_GAMEPAD;

	HRESULT result = mDirectInput->CreateDevice(mInfo.GuidInstance, &mGamepad, nullptr);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput gamepad init: Failed to create device. Error code: {0}.", (u64)result);
		return;
	}

	result = mGamepad->SetDataFormat(&c_dfDIJoystick2);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput gamepad init: Failed to set format. Error code: {0}.", (u64)result);
		return;
	}

	result = mGamepad->SetCooperativeLevel(hWnd, mCoopSettings);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput gamepad init: Failed to set coop level. Error code: {0}.", (u64)result);
		return;
	}

	result = mGamepad->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput gamepad init: Failed to set property. Error code: {0}.", (u64)result);
		return;
	}

	result = mGamepad->Acquire();
	if(FAILED(result) && result != DIERR_OTHERAPPHASPRIO)
	{
		B3D_LOG(Error, LogInput, "DirectInput gamepad init: Failed to acquire device. Error code: {0}.", (u64)result);
		return;
	}

	mHWnd = hWnd;
}

void Win32Gamepad::ReleaseDirectInput()
{
	if(mGamepad)
	{
		mGamepad->Unacquire();
		mGamepad->Release();
		mGamepad = nullptr;
	}
}

void Win32Gamepad::HandlePov(int pov, DIDEVICEOBJECTDATA& di)
{
	if(LOWORD(di.dwData) == 0xFFFF)
	{
		// Centered, release any buttons
		if(mPovState[pov].Pressed)
		{
			mOwner.NotifyButtonReleased(mInfo.Id, mPovState[pov].Code, di.dwTimeStamp);
			mPovState[pov].Pressed = false;
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
			if(mPovState[pov].Pressed)
			{
				// If its a different button, release the old one and press the new one
				if(mPovState[pov].Code != newPOVState.Code)
				{
					mOwner.NotifyButtonReleased(mInfo.Id, mPovState[pov].Code, di.dwTimeStamp);
					mOwner.NotifyButtonPressed(mInfo.Id, newPOVState.Code, di.dwTimeStamp);

					mPovState[pov].Code = newPOVState.Code;
				}
			}
			else
			{
				mOwner.NotifyButtonPressed(mInfo.Id, newPOVState.Code, di.dwTimeStamp);
				mPovState[pov].Code = newPOVState.Code;
				mPovState[pov].Pressed = true;
			}
		}
	}
}

void Win32Gamepad::Capture()
{
	// Skip capture in headless mode or when focus is lost
	if(mHWnd == nullptr || (!mInfo.IsXInput && mGamepad == nullptr))
		return;

	if(mInfo.IsXInput)
	{
		XINPUT_STATE inputState;
		if(XInputGetState((DWORD)mInfo.XInputDev, &inputState) != ERROR_SUCCESS)
			memset(&inputState, 0, sizeof(inputState));

		// Sticks and triggers
		struct AxisState
		{
			bool Moved;
			i32 Value;
		};

		AxisState axisState[6];
		B3DZeroOut(axisState);

		// Note: Order of axes must match the InputAxis enum, starting at LeftStickX
		// Left stick
		axisState[0].Value = (int)inputState.Gamepad.sThumbLX;
		axisState[1].Value = -(int)inputState.Gamepad.sThumbLY;

		// Right stick
		axisState[2].Value = (int)inputState.Gamepad.sThumbRX;
		axisState[3].Value = -(int)inputState.Gamepad.sThumbRY;

		// Left trigger
		axisState[4].Value = std::min((int)inputState.Gamepad.bLeftTrigger * 129, IInputBackend::kMaxAxis);

		// Right trigger
		axisState[5].Value = std::min((int)inputState.Gamepad.bRightTrigger * 129, IInputBackend::kMaxAxis);

		for(u32 axisIndex = 0; axisIndex < 6; axisIndex++)
		{
			axisState[axisIndex].Moved = axisState[axisIndex].Value != mAxisState[axisIndex];
			mAxisState[axisIndex] = axisState[axisIndex].Value;
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
			if(mPovState[0].Pressed)
			{
				// If its a different button, release the old one and press the new one
				if(mPovState[0].Code != dpadButton)
				{
					mOwner.NotifyButtonReleased(mInfo.Id, mPovState[0].Code, GetTickCount64());
					mOwner.NotifyButtonPressed(mInfo.Id, dpadButton, GetTickCount64());

					mPovState[0].Code = dpadButton;
				}
			}
			else
			{
				mOwner.NotifyButtonPressed(mInfo.Id, dpadButton, GetTickCount64());
				mPovState[0].Code = dpadButton;
				mPovState[0].Pressed = true;
			}
		}
		else
		{
			if(mPovState[0].Pressed)
			{
				mOwner.NotifyButtonReleased(mInfo.Id, mPovState[0].Code, GetTickCount64());
				mPovState[0].Pressed = false;
			}
		}

		// Buttons
		for(u32 buttonIndex = 0; buttonIndex < 16; buttonIndex++)
		{
			bool buttonState = (inputState.Gamepad.wButtons & (1 << buttonIndex)) != 0;

			if(buttonState != mButtonState[buttonIndex])
			{
				if(buttonState)
					mOwner.NotifyButtonPressed(mInfo.Id, GamepadButtonToButtonCode(buttonIndex), GetTickCount64());
				else
					mOwner.NotifyButtonReleased(mInfo.Id, GamepadButtonToButtonCode(buttonIndex), GetTickCount64());

				mButtonState[buttonIndex] = buttonState;
			}
		}

		for(u32 axisIndex = 0; axisIndex < 6; ++axisIndex)
		{
			if(!axisState[axisIndex].Moved)
				continue;

			mOwner.NotifyAxisMoved(mInfo.Id, axisIndex + (u32)InputAxis::LeftStickX, axisState[axisIndex].Value);
		}
	}
	else // DirectInput
	{
		DIDEVICEOBJECTDATA diBuff[DI_BUFFER_SIZE_GAMEPAD];
		DWORD entryCount = DI_BUFFER_SIZE_GAMEPAD;

		HRESULT hr = mGamepad->Poll();
		if(hr == DI_OK)
			hr = mGamepad->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &entryCount, 0);

		if(hr != DI_OK)
		{
			hr = mGamepad->Acquire();
			while(hr == DIERR_INPUTLOST)
				hr = mGamepad->Acquire();

			mGamepad->Poll();
			hr = mGamepad->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &entryCount, 0);

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
				HandlePov(0, diBuff[entryIndex]);
				break;
			case DIJOFS_POV(1):
				HandlePov(1, diBuff[entryIndex]);
				break;
			case DIJOFS_POV(2):
				HandlePov(2, diBuff[entryIndex]);
				break;
			case DIJOFS_POV(3):
				HandlePov(3, diBuff[entryIndex]);
				break;
			default:
				// Button event
				if(diBuff[entryIndex].dwOfs >= DIJOFS_BUTTON(0) && diBuff[entryIndex].dwOfs < DIJOFS_BUTTON(128))
				{
					int button = diBuff[entryIndex].dwOfs - DIJOFS_BUTTON(0);

					if((diBuff[entryIndex].dwData & 0x80) != 0)
						mOwner.NotifyButtonPressed(mInfo.Id, GamepadButtonToButtonCode(button), diBuff[entryIndex].dwTimeStamp);
					else
						mOwner.NotifyButtonReleased(mInfo.Id, GamepadButtonToButtonCode(button), diBuff[entryIndex].dwTimeStamp);
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
			for(u32 axisIndex = 0; axisIndex < 24; ++axisIndex)
			{
				if(!axisState[axisIndex].Moved)
					continue;

				mOwner.NotifyAxisMoved(mInfo.Id, axisIndex + (u32)InputAxis::LeftStickX, axisState[axisIndex].Value);
			}
		}
	}
}

void Win32Gamepad::ChangeCaptureContext(u64 windowHandle)
{
	HWND newWindowHandle = (HWND)windowHandle;

	if(mHWnd != newWindowHandle)
	{
		ReleaseDirectInput();

		// Don't initialize DirectInput for invalid handles (headless mode or lost focus)
		if(!mInfo.IsXInput && windowHandle != 0)
			InitializeDirectInput(newWindowHandle);
		else
			mHWnd = newWindowHandle;
	}
}
