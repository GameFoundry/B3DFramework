//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Input/B3DInputBackend.h"
#include "Input/B3DInputFwd.h"
#include "Utility/B3DEvent.h"

#include <atomic>

namespace b3d
{
	/** @addtogroup Platform-Internal
	 *  @{
	 */

	/** Information about an analog axis that's part of a gamepad. */
	struct AxisInfo
	{
		i32 AxisIdx;
		i32 Min;
		i32 Max;
	};

	/** Information about a gamepad, corresponding to a single /dev/input/eventX device. */
	struct GamepadInfo
	{
		u32 Id;
		u32 EventHandlerIdx;
		String Name;

		UnorderedMap<i32, ButtonCode> ButtonMap;
		UnorderedMap<i32, AxisInfo> AxisMap;
	};

	/** Data about relative pointer / scroll wheel movement, accumulated by the platform message pump. */
	struct LinuxMouseMotionEvent
	{
		double DeltaX; /**< Relative pointer movement in X direction. */
		double DeltaY; /**< Relative pointer movement in Y direction. */
		double DeltaZ; /**< Relative vertical scroll amount. */
	};

	/** Data about a single button press or release, queued by the platform message pump. */
	struct LinuxButtonEvent
	{
		u64 Timestamp;
		ButtonCode Button;
		bool Pressed;
	};

	/** Contains state of the DPad, tracked so the directional hat events can be reported as a single 8-way POV button. */
	struct POVState
	{
		ButtonCode Code;
		bool Pressed;
	};

	/** Represents a single hardware gamepad, backed by an evdev input event device. */
	class LinuxGamepad
	{
	public:
		LinuxGamepad(const GamepadInfo& gamepadInfo, Input& owner);
		~LinuxGamepad();

		/** Returns the name of the device. */
		String GetName() const { return mInfo.Name; }

		/** Returns the information the device was created from. */
		const GamepadInfo& GetInfo() const { return mInfo; }

		/** Captures the input since the last call and triggers the events on the parent Input. */
		void Capture();

		/** Changes the capture context. Should be called when focus is moved to a new window. */
		void ChangeCaptureContext(u64 windowHandle);

	private:
		Input& mOwner;
		GamepadInfo mInfo;
		i32 mFileHandle = -1;
		bool mHasInputFocus = true;

		i32 mPovX = 0; /**< Last reported horizontal hat direction, in [-1, 1]. */
		i32 mPovY = 0; /**< Last reported vertical hat direction, in [-1, 1]. */
		POVState mPovState;
	};

	/** Represents a single hardware keyboard. Reports button events queued by the platform message pump. */
	class LinuxKeyboard
	{
	public:
		LinuxKeyboard(Input& owner);

		/** Captures the input since the last call and triggers the events on the parent Input. */
		void Capture();

		/** Changes the capture context. Should be called when focus is moved to a new window. */
		void ChangeCaptureContext(u64 windowHandle);

	private:
		Input& mOwner;
		bool mHasInputFocus = true;
	};

	/** Represents a single hardware mouse. Reports relative motion accumulated by the platform message pump. */
	class LinuxMouse
	{
	public:
		LinuxMouse(Input& owner);

		/** Captures the input since the last call and triggers the events on the parent Input. */
		void Capture();

		/** Changes the capture context. Should be called when focus is moved to a new window. */
		void ChangeCaptureContext(u64 windowHandle);

	private:
		Input& mOwner;
		bool mHasInputFocus = true;
	};

	/** Linux implementation of the input backend, based on evdev for gamepads and X11 events for keyboard/mouse. */
	class LinuxInputBackend final : public IInputBackend
	{
	public:
		LinuxInputBackend(Input& owner);
		~LinuxInputBackend();

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

		LinuxKeyboard* mKeyboard = nullptr;
		LinuxMouse* mMouse = nullptr;
		Vector<LinuxGamepad*> mGamepads;

		std::atomic<bool> mDevicesDirty{false};
		HEvent mDevicesChangedConn;
	};

	/** @} */
} // namespace b3d
