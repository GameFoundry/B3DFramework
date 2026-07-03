//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Input/B3DInputBackend.h"
#include "Input/B3DInputFwd.h"
#include "Utility/B3DEvent.h"

#define WIN32_LEAN_AND_MEAN
#ifndef DIRECTINPUT_VERSION
#	define DIRECTINPUT_VERSION 0x0800
#endif

#include <windows.h>
#include <dinput.h>
#include <Xinput.h>

#include <wbemidl.h>
#include <oleauto.h>

#include <atomic>

namespace b3d
{
	/** @addtogroup Platform-Internal
	 *  @{
	 */

	/** Information about a gamepad from DirectInput. */
	struct GamepadInfo
	{
		u32 Id;
		GUID GuidInstance;
		GUID GuidProduct;
		String Name;

		bool IsXInput;
		int XInputDev;
	};

	/** Contains state of a POV (DPad). */
	struct POVState
	{
		ButtonCode Code;
		bool Pressed;
	};

	/** Represents a single hardware gamepad. Uses XInput for XInput-compatible devices, and DirectInput for the rest. */
	class Win32Gamepad
	{
	public:
		Win32Gamepad(const GamepadInfo& gamepadInfo, Input& owner, IDirectInput8* directInput, DWORD coopSettings,
			u64 windowHandle);
		~Win32Gamepad();

		/** Returns the name of the device. */
		String GetName() const { return mInfo.Name; }

		/** Returns the information the device was created from. */
		const GamepadInfo& GetInfo() const { return mInfo; }

		/** Captures the input since the last call and triggers the events on the parent Input. */
		void Capture();

		/** Changes the capture context. Should be called when focus is moved to a new window. */
		void ChangeCaptureContext(u64 windowHandle);

	private:
		/**
		 * Initializes the DirectInput device for a window with the specified handle. Only input from that window will be
		 * reported.
		 */
		void InitializeDirectInput(HWND hWnd);

		/** Releases the DirectInput device. */
		void ReleaseDirectInput();

		/** Handles a DirectInput POV event. */
		void HandlePov(int pov, DIDEVICEOBJECTDATA& di);

		Input& mOwner;
		GamepadInfo mInfo;
		IDirectInput8* mDirectInput = nullptr;
		IDirectInputDevice8* mGamepad = nullptr;
		DWORD mCoopSettings = 0;
		HWND mHWnd = nullptr;

		POVState mPovState[4];
		i32 mAxisState[6]; // Only for XInput
		bool mButtonState[16]; // Only for XInput
	};

	/** Represents a single hardware mouse, based on DirectInput. */
	class Win32Mouse
	{
	public:
		Win32Mouse(Input& owner, IDirectInput8* directInput, DWORD coopSettings, u64 windowHandle);
		~Win32Mouse();

		/** Captures the input since the last call and triggers the events on the parent Input. */
		void Capture();

		/** Changes the capture context. Should be called when focus is moved to a new window. */
		void ChangeCaptureContext(u64 windowHandle);

	private:
		/**
		 * Initializes the DirectInput device for a window with the specified handle. Only input from that window will be
		 * reported.
		 */
		void InitializeDirectInput(HWND hWnd);

		/** Releases the DirectInput device. */
		void ReleaseDirectInput();

		Input& mOwner;
		IDirectInput8* mDirectInput = nullptr;
		IDirectInputDevice8* mMouse = nullptr;
		DWORD mCoopSettings = 0;
		HWND mHWnd = nullptr;
	};

	/** Represents a single hardware keyboard, based on DirectInput. */
	class Win32Keyboard
	{
	public:
		Win32Keyboard(Input& owner, IDirectInput8* directInput, DWORD coopSettings, u64 windowHandle);
		~Win32Keyboard();

		/** Captures the input since the last call and triggers the events on the parent Input. */
		void Capture();

		/** Changes the capture context. Should be called when focus is moved to a new window. */
		void ChangeCaptureContext(u64 windowHandle);

	private:
		/**
		 * Initializes the DirectInput device for a window with the specified handle. Only input from that window will be
		 * reported.
		 */
		void InitializeDirectInput(HWND hWnd);

		/** Releases the DirectInput device. */
		void ReleaseDirectInput();

		Input& mOwner;
		IDirectInput8* mDirectInput = nullptr;
		IDirectInputDevice8* mKeyboard = nullptr;
		DWORD mCoopSettings = 0;
		HWND mHWnd = nullptr;

		u8 mKeyBuffer[256];
	};

	/** Win32 implementation of the input backend, based on DirectInput and XInput. */
	class Win32InputBackend final : public IInputBackend
	{
	public:
		Win32InputBackend(Input& owner);
		~Win32InputBackend();

		void Update() override;
		u32 GetDeviceCount(InputDevice type) const override;
		String GetDeviceName(InputDevice type, u32 deviceIndex) const override;
		void ChangeCaptureContext(u64 windowHandle) override;

	private:
		/** Enumerates the currently attached gamepads. Assigned ids match the enumeration order. */
		void EnumerateGamepads(Vector<GamepadInfo>& outGamepadInfos);

		/**
		 * Compares the attached gamepads with the currently created gamepad devices after a hardware configuration
		 * change, then creates/destroys gamepad devices to match. Devices that remain attached are unaffected.
		 */
		void RebuildGamepads();

		Input& mOwner;
		IDirectInput8* mDirectInput = nullptr;
		DWORD mKbSettings = 0;
		DWORD mMouseSettings = 0;
		u64 mWindowHandle = 0;

		Win32Mouse* mMouse = nullptr;
		Win32Keyboard* mKeyboard = nullptr;
		Vector<Win32Gamepad*> mGamepads;

		std::atomic<bool> mDevicesDirty{false};
		HEvent mDevicesChangedConn;
	};

// Max number of elements to collect from buffered DirectInput
#define DI_BUFFER_SIZE_KEYBOARD 17
#define DI_BUFFER_SIZE_MOUSE 128
#define DI_BUFFER_SIZE_GAMEPAD 129

	/** @} */
} // namespace b3d
