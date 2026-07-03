//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Input/B3DInputFwd.h"

namespace b3d
{
	/** @addtogroup Input-Internal
	 *  @{
	 */

	/**
	 * Interface implemented by each platform, providing Input with access to raw input devices (keyboard, mouse,
	 * gamepads). The backend owns device enumeration and lifetime, and reports captured input through the owner
	 * Input's Notify* methods.
	 */
	class IInputBackend
	{
	public:
		virtual ~IInputBackend() = default;

		/**
		 * Called once per frame from the main thread. Detects device changes (hardware hot-plug, user login/logout) and
		 * captures input from all attached devices, reporting events through the owner Input.
		 */
		virtual void Update() = 0;

		/** Returns the number of attached devices of the specified type. */
		virtual u32 GetDeviceCount(InputDevice type) const = 0;

		/** Returns the name of a specific device. Returns an empty string if the device doesn't exist. */
		virtual String GetDeviceName(InputDevice type, u32 deviceIndex) const = 0;

		/**
		 * Changes the window input is captured for. Called when input focus moves to a new window, or with zero when
		 * focus is lost entirely (or the application is running headless).
		 */
		virtual void ChangeCaptureContext(u64 windowHandle) = 0;

		/** Minimum allowed value as reported by the axis movement events. */
		static constexpr i32 kMinAxis = -32768;

		/** Maximum allowed value as reported by the axis movement events. */
		static constexpr i32 kMaxAxis = 32767;
	};

	/** Creates the input backend implementation for the current platform. The caller owns the returned object. */
	IInputBackend* CreateInputBackend(Input& owner);

	/** @} */
} // namespace b3d
