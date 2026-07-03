//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/B3DInput.h"
#include "Private/Win32/B3DWin32Input.h"

using namespace b3d;

Win32Keyboard::Win32Keyboard(Input& owner, IDirectInput8* directInput, DWORD coopSettings, u64 windowHandle)
	: mOwner(owner), mDirectInput(directInput), mCoopSettings(coopSettings)
{
	B3DZeroOut(mKeyBuffer);

	// Don't initialize DirectInput in headless mode (window handle == 0)
	if(windowHandle != 0)
		InitializeDirectInput((HWND)windowHandle);
}

Win32Keyboard::~Win32Keyboard()
{
	ReleaseDirectInput();
}

void Win32Keyboard::InitializeDirectInput(HWND hWnd)
{
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = DI_BUFFER_SIZE_KEYBOARD;

	HRESULT result = mDirectInput->CreateDevice(GUID_SysKeyboard, &mKeyboard, nullptr);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput keyboard init: Failed to create device. Error code: {0}.", (u64)result);
		return;
	}

	result = mKeyboard->SetDataFormat(&c_dfDIKeyboard);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput keyboard init: Failed to set format. Error code: {0}.", (u64)result);
		return;
	}

	result = mKeyboard->SetCooperativeLevel(hWnd, mCoopSettings);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput keyboard init: Failed to set coop level. Error code: {0}.", (u64)result);
		return;
	}

	result = mKeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
	if(FAILED(result))
	{
		B3D_LOG(Error, LogInput, "DirectInput keyboard init: Failed to set property. Error code: {0}.", (u64)result);
		return;
	}

	result = mKeyboard->Acquire();
	if(FAILED(result) && result != DIERR_OTHERAPPHASPRIO)
	{
		B3D_LOG(Error, LogInput, "DirectInput keyboard init: Failed to acquire device. Error code: {0}.", (u64)result);
		return;
	}

	mHWnd = hWnd;
}

void Win32Keyboard::ReleaseDirectInput()
{
	if(mKeyboard)
	{
		mKeyboard->Unacquire();
		mKeyboard->Release();
		mKeyboard = nullptr;
	}
}

void Win32Keyboard::Capture()
{
	if(mKeyboard == nullptr)
		return;

	DIDEVICEOBJECTDATA diBuff[DI_BUFFER_SIZE_KEYBOARD];
	DWORD entryCount = DI_BUFFER_SIZE_KEYBOARD;

	// Note: Only one keyboard per app due to this static (which is fine)
	static bool verifyAfterAltTab = false;

	HRESULT hr = mKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &entryCount, 0);
	if(hr != DI_OK)
	{
		hr = mKeyboard->Acquire();
		if(hr == E_ACCESSDENIED)
			verifyAfterAltTab = true;

		while(hr == DIERR_INPUTLOST)
			hr = mKeyboard->Acquire();

		return;
	}

	if(FAILED(hr))
	{
		B3D_LOG(Error, LogPlatform, "Failed to read keyboard input. Internal error. ");
		return;
	}

	for(u32 entryIndex = 0; entryIndex < entryCount; ++entryIndex)
	{
		ButtonCode buttonCode = (ButtonCode)diBuff[entryIndex].dwOfs;

		mKeyBuffer[(u32)buttonCode] = (u8)(diBuff[entryIndex].dwData);

		if(diBuff[entryIndex].dwData & 0x80)
			mOwner.NotifyButtonPressed(0, buttonCode, diBuff[entryIndex].dwTimeStamp);
		else
			mOwner.NotifyButtonReleased(0, buttonCode, diBuff[entryIndex].dwTimeStamp);
	}

	// If a lost device/access denied was detected, recover
	if(verifyAfterAltTab)
	{
		// Store old buffer
		u8 keyBufferCopy[256];
		memcpy(keyBufferCopy, mKeyBuffer, 256);

		// Read immediate state
		hr = mKeyboard->GetDeviceState(sizeof(mKeyBuffer), &mKeyBuffer);

		if(hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
		{
			hr = mKeyboard->Acquire();
			if(hr != DIERR_OTHERAPPHASPRIO)
				mKeyboard->GetDeviceState(sizeof(mKeyBuffer), &mKeyBuffer);
		}

		for(u32 keyIndex = 0; keyIndex < 256; keyIndex++)
		{
			if(keyBufferCopy[keyIndex] != mKeyBuffer[keyIndex])
			{
				if(mKeyBuffer[keyIndex])
					mOwner.NotifyButtonPressed(0, (ButtonCode)keyIndex, GetTickCount64());
				else
					mOwner.NotifyButtonReleased(0, (ButtonCode)keyIndex, GetTickCount64());
			}
		}

		verifyAfterAltTab = false;
	}
}

void Win32Keyboard::ChangeCaptureContext(u64 windowHandle)
{
	HWND newWindowHandle = (HWND)windowHandle;

	if(mHWnd != newWindowHandle)
	{
		ReleaseDirectInput();

		// Don't initialize DirectInput for invalid handles (headless mode or lost focus)
		if(windowHandle != 0)
			InitializeDirectInput(newWindowHandle);
		else
			mHWnd = newWindowHandle;
	}
}
