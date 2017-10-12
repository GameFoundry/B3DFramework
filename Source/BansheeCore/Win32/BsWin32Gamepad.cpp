//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Input/BsGamepad.h"
#include "Input/BsInput.h"
#include "Win32/BsWin32Input.h"
#include "Error/BsException.h"

#undef DIJOFS_BUTTON
#define DIJOFS_BUTTON(n)  (FIELD_OFFSET(DIJOYSTATE2, rgbButtons) + (n))

namespace bs
{
	/** Contains state of a POV (DPad). */
	struct POVState
	{
		ButtonCode code;
		bool pressed;
	};

	/** Contains private data for the Win32 Gamepad implementation. */
	struct Gamepad::Pimpl
	{
		IDirectInput8* directInput;
		IDirectInputDevice8* gamepad;
		GamepadInfo info;
		DWORD coopSettings;
		HWND hWnd;

		POVState povState[4];
		INT32 axisState[6]; // Only for XInput
		bool buttonState[16]; // Only for XInput
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

		if (FAILED(m->directInput->CreateDevice(m->info.guidInstance, &m->gamepad, nullptr)))
			BS_EXCEPT(InternalErrorException, "DirectInput gamepad init: Failed to create device.");

		if (FAILED(m->gamepad->SetDataFormat(&c_dfDIJoystick2)))
			BS_EXCEPT(InternalErrorException, "DirectInput gamepad init: Failed to set format.");

		if (FAILED(m->gamepad->SetCooperativeLevel(hWnd, m->coopSettings)))
			BS_EXCEPT(InternalErrorException, "DirectInput gamepad init: Failed to set coop level.");

		if (FAILED(m->gamepad->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
			BS_EXCEPT(InternalErrorException, "DirectInput gamepad init: Failed to set property.");

		HRESULT hr = m->gamepad->Acquire();
		if (FAILED(hr) && hr != DIERR_OTHERAPPHASPRIO)
			BS_EXCEPT(InternalErrorException, "DirectInput gamepad init: Failed to acquire device.");

		m->hWnd = hWnd;
	}

	/** Releases DirectInput resources for the provided device */
	void releaseDirectInput(Gamepad::Pimpl* m)
	{
		if(m->gamepad)
		{
			m->gamepad->Unacquire();
			m->gamepad->Release();
			m->gamepad = nullptr;
		}
	}

	/** Handles a DirectInput POV event. */
	void handlePOV(Input* owner, Gamepad::Pimpl* m, int pov, DIDEVICEOBJECTDATA& di)
	{
		if (LOWORD(di.dwData) == 0xFFFF)
		{
			// Centered, release any buttons
			if (m->povState[pov].pressed)
			{
				owner->_notifyButtonReleased(m->info.id, m->povState[pov].code, di.dwTimeStamp);
				m->povState[pov].pressed = false;
			}
		}
		else
		{
			POVState newPOVState;
			bs_zero_out(newPOVState);

			switch (di.dwData)
			{
			case 0: 
				newPOVState.code = BC_GAMEPAD_DPAD_UP;
				newPOVState.pressed = true;
				break;
			case 4500:
				newPOVState.code = BC_GAMEPAD_DPAD_UPRIGHT;
				newPOVState.pressed = true;
				break;
			case 9000:
				newPOVState.code = BC_GAMEPAD_DPAD_RIGHT;
				newPOVState.pressed = true;
				break;
			case 13500:
				newPOVState.code = BC_GAMEPAD_DPAD_DOWNRIGHT;
				newPOVState.pressed = true;
				break;
			case 18000:
				newPOVState.code = BC_GAMEPAD_DPAD_DOWN;
				newPOVState.pressed = true;
				break;
			case 22500:
				newPOVState.code = BC_GAMEPAD_DPAD_DOWNLEFT;
				newPOVState.pressed = true;
				break;
			case 27000:
				newPOVState.code = BC_GAMEPAD_DPAD_LEFT;
				newPOVState.pressed = true;
				break;
			case 31500:
				newPOVState.code = BC_GAMEPAD_DPAD_UPLEFT;
				newPOVState.pressed = true;
				break;
			}

			// Button was pressed
			if (newPOVState.pressed)
			{
				// Another button was previously pressed
				if (m->povState[pov].pressed)
				{
					// If its a different button, release the old one and press the new one
					if (m->povState[pov].code != newPOVState.code)
					{
						owner->_notifyButtonReleased(m->info.id, m->povState[pov].code, di.dwTimeStamp);
						owner->_notifyButtonPressed(m->info.id, newPOVState.code, di.dwTimeStamp);

						m->povState[pov].code = newPOVState.code;
					}
				}
				else
				{
					owner->_notifyButtonPressed(m->info.id, newPOVState.code, di.dwTimeStamp);
					m->povState[pov].code = newPOVState.code;
					m->povState[pov].pressed = true;
				}
			}
		}
	}

	/** Converts a DirectInput or XInput button code to Banshee ButtonCode. */
	ButtonCode gamepadButtonToButtonCode(INT32 code)
	{
		switch (code)
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
		InputPrivateData* pvtData = owner->_getPrivateData();

		m = bs_new<Pimpl>();
		m->directInput = pvtData->directInput;
		m->coopSettings = pvtData->mouseSettings;
		m->info = gamepadInfo;
		m->gamepad = nullptr;
		m->hWnd = (HWND)owner->_getWindowHandle();
		bs_zero_out(m->povState);
		bs_zero_out(m->axisState);
		bs_zero_out(m->buttonState);

		if(!m->info.isXInput)
			initializeDirectInput(m, m->hWnd);
	}

	Gamepad::~Gamepad()
	{
		releaseDirectInput(m);

		bs_delete(m);
	}

	void Gamepad::capture()
	{
		if (m->hWnd == (HWND)-1 || m->gamepad == nullptr)
			return;

		if(m->info.isXInput)
		{
			XINPUT_STATE inputState;
			if (XInputGetState((DWORD)m->info.xInputDev, &inputState) != ERROR_SUCCESS)
				memset(&inputState, 0, sizeof(inputState));

			// Sticks and triggers
			struct AxisState
			{
				bool moved;
				INT32 value;
			};

			AxisState axisState[6];
			bs_zero_out(axisState);

			// Note: Order of axes must match InputAxis enum
			// Left stick
			axisState[0].value = (int)inputState.Gamepad.sThumbLX;
			axisState[1].value = -(int)inputState.Gamepad.sThumbLY;

			// Right stick 
			axisState[2].value = (int)inputState.Gamepad.sThumbRX;
			axisState[3].value = -(int)inputState.Gamepad.sThumbRY;

			// Left trigger
			axisState[4].value = std::min((int)inputState.Gamepad.bLeftTrigger * 129, MAX_AXIS);

			// Right trigger
			axisState[5].value = std::min((int)inputState.Gamepad.bRightTrigger * 129, MAX_AXIS);

			for (UINT32 i = 0; i < 6; i++)
			{
				axisState[i].moved = axisState[i].value != m->axisState[i];
				m->axisState[i] = axisState[i].value;
			}

			// DPAD (POV)
			ButtonCode dpadButton = BC_UNASSIGNED;
			if ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0)
				dpadButton = BC_GAMEPAD_DPAD_UP;
			else if ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0)
				dpadButton = BC_GAMEPAD_DPAD_DOWN;
			if ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0 )
				dpadButton = BC_GAMEPAD_DPAD_LEFT;
			else if ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0)
				dpadButton = BC_GAMEPAD_DPAD_RIGHT;

			if(dpadButton != BC_UNASSIGNED) // Pressed
			{
				// Another button was previously pressed
				if (m->povState[0].pressed)
				{
					// If its a different button, release the old one and press the new one
					if (m->povState[0].code != dpadButton)
					{
						mOwner->_notifyButtonReleased(m->info.id, m->povState[0].code, GetTickCount64());
						mOwner->_notifyButtonPressed(m->info.id, dpadButton, GetTickCount64());

						m->povState[0].code = dpadButton;
					}
				}
				else
				{
					mOwner->_notifyButtonPressed(m->info.id, dpadButton, GetTickCount64());
					m->povState[0].code = dpadButton;
					m->povState[0].pressed = true;
				}
			}
			else
			{
				if (m->povState[0].pressed)
				{
					mOwner->_notifyButtonReleased(m->info.id, m->povState[0].code, GetTickCount64());
					m->povState[0].pressed = false;
				}
			}

			// Buttons
			for (UINT32 i = 0; i < 16; i++)
			{
				bool buttonState = (inputState.Gamepad.wButtons & (1 << i)) != 0;

				if(buttonState != m->buttonState[i])
				{
					if (buttonState)
						mOwner->_notifyButtonPressed(m->info.id, gamepadButtonToButtonCode(i), GetTickCount64());
					else
						mOwner->_notifyButtonReleased(m->info.id, gamepadButtonToButtonCode(i), GetTickCount64());

					m->buttonState[i] = buttonState;
				}
			}

			for (int i = 0; i < 6; ++i)
			{
				if (!axisState[i].moved)
					continue;

				mOwner->_notifyAxisMoved(m->info.id, i + (int)InputAxis::MouseZ, axisState[i].value);
			}
		}
		else // DirectInput
		{
			DIDEVICEOBJECTDATA diBuff[DI_BUFFER_SIZE_GAMEPAD];
			DWORD numEntries = DI_BUFFER_SIZE_GAMEPAD;

			HRESULT hr = m->gamepad->Poll();
			if (hr == DI_OK)
				hr = m->gamepad->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &numEntries, 0);

			if (hr != DI_OK)
			{
				hr = m->gamepad->Acquire();
				while (hr == DIERR_INPUTLOST)
					hr = m->gamepad->Acquire();

				m->gamepad->Poll();
				hr = m->gamepad->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &numEntries, 0);

				if (FAILED(hr))
					return;
			}

			struct AxisState
			{
				bool moved;
				INT32 value;
			};

			AxisState axisState[24];
			bs_zero_out(axisState);

			// Note: Not reporting slider or POV events
			for (UINT32 i = 0; i < numEntries; ++i)
			{
				switch (diBuff[i].dwOfs)
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
					if (diBuff[i].dwOfs >= DIJOFS_BUTTON(0) && diBuff[i].dwOfs < DIJOFS_BUTTON(128))
					{
						int button = diBuff[i].dwOfs - DIJOFS_BUTTON(0);

						if ((diBuff[i].dwData & 0x80) != 0)
							mOwner->_notifyButtonPressed(m->info.id, gamepadButtonToButtonCode(button), diBuff[i].dwTimeStamp);
						else
							mOwner->_notifyButtonReleased(m->info.id, gamepadButtonToButtonCode(button), diBuff[i].dwTimeStamp);

					}
					else if ((short)(diBuff[i].uAppData >> 16) == 0x1313) // Axis event
					{
						int axis = (int)(0x0000FFFF & diBuff[i].uAppData);
						if (axis < 24)
						{
							axisState[axis].moved = true;
							axisState[axis].value = diBuff[i].dwData;
						}
					}
				}
			}

			if (numEntries > 0)
			{
				for (int i = 0; i < 24; ++i)
				{
					if (!axisState[i].moved)
						continue;

					mOwner->_notifyAxisMoved(m->info.id, i + (int)InputAxis::MouseZ, axisState[i].value);
				}
			}
		}
	}

	void Gamepad::changeCaptureContext(UINT64 windowHandle)
	{
		HWND newhWnd = (HWND)windowHandle;

		if(m->hWnd != newhWnd)
		{
			releaseDirectInput(m);

			if (!m->info.isXInput && windowHandle != (UINT64)-1)
				initializeDirectInput(m, newhWnd);
			else
				m->hWnd = newhWnd;
		}
	}
}
