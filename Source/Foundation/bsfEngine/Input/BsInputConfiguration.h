//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Input/BsInputFwd.h"

namespace bs
{
	/** @addtogroup Input-Engine
	 *  @{
	 */

	/**
	 * Describes a virtual button. Virtual buttons allow you to map custom actions without needing to know about what
	 * physical buttons trigger those actions.
	 */
	struct BS_EXPORT VIRTUAL_BUTTON_DESC
	{
		VIRTUAL_BUTTON_DESC() = default;

		/**
		 * Constructs a virtual button descriptor.
		 *
		 * @param[in]	buttonCode	Physical button the virtual button is triggered by.
		 * @param[in]	modifiers	Modifiers required to be pressed with the physical button to trigger the virtual button.
		 * @param[in]	repeatable	If true, the virtual button events will be sent continually while the physical button
		 *							is being held.
		 */
		VIRTUAL_BUTTON_DESC(ButtonCode buttonCode, ButtonModifier modifiers = ButtonModifier::None, bool repeatable = false);

		ButtonCode ButtonCode = BC_0;
		ButtonModifier Modifiers = ButtonModifier::None;
		bool Repeatable = false;
	};

	/**
	 * Describes a virtual axis. Virtual axes allow you to map custom axes without needing to know the actual physical
	 * device handling those axes.
	 */
	struct BS_EXPORT VIRTUAL_AXIS_DESC
	{
		VIRTUAL_AXIS_DESC() = default;

		/**
		 * Constructs a new virtual axis descriptor.
		 *
		 * @param[in]	type		@copydoc VIRTUAL_AXIS_DESC::type
		 */
		VIRTUAL_AXIS_DESC(u32 type);

		/** Type of physical axis to map to. See InputAxis type for common types, but you are not limited to those values. */
		u32 Type = (u32)InputAxis::MouseX;

		/** Value below which to ignore axis value and consider it 0. */
		float DeadZone = 0.0001f;

		/** Higher sensitivity means the axis will more easily reach its maximum values. */
		float Sensitivity = 1.0f;

		/** Should the axis be inverted. */
		bool Invert = false;

		/**
		 * If enabled, axis values will be normalized to [-1, 1] range. Most axes already come in normalized form and this
		 * value will not affect such axes. Some axes, like mouse movement are not normalized by default and will instead
		 * report relative movement. By enabling this you will normalize such axes to [-1, 1] range.
		 */
		bool Normalize = false;
	};

	/**
	 * Identifier for a virtual button.
	 *
	 * Primary purpose of this class is to avoid expensive string compare, and instead use a unique button identifier for
	 * compare. Generally you want to create one of these using the button name, and then store it for later use.
	 *
	 * @note
	 * This class is not thread safe and should only be used on the sim thread.
	 *
	 * @see		VIRTUAL_BUTTON_DESC
	 */
	class BS_EXPORT VirtualButton
	{
	public:
		VirtualButton() = default;
		VirtualButton(const String& name);

		bool operator==(const VirtualButton& rhs) const
		{
			return (ButtonIdentifier == rhs.ButtonIdentifier);
		}

		u32 ButtonIdentifier = 0;

	private:
		/** Returns a static map of all virtual button identifiers and their buttons. */
		static Map<String, u32>& GetUniqueButtonIds();

		static u32 NextButtonId;
	};

	/**
	 * Identifier for a virtual axis.
	 *
	 * Primary purpose of this class is to avoid expensive string compare (axis names), and instead use a unique axis
	 * identifier for compare. Generally you want to create one of these using the axis name, and then store it for later
	 * use.
	 *
	 * @note
	 * This class is not thread safe and should only be used on the sim thread.
	 *
	 * @see		VIRTUAL_AXIS_DESC
	 */
	class BS_EXPORT VirtualAxis
	{
	public:
		VirtualAxis() = default;
		VirtualAxis(const String& name);

		u32 AxisIdentifier = 0;

		bool operator==(const VirtualAxis& rhs) const
		{
			return (AxisIdentifier == rhs.AxisIdentifier);
		}

	private:
		static Map<String, u32> UniqueAxisIds;
		static u32 NextAxisId;
	};

	/**	Contains virtual <-> physical key mappings. */
	class BS_EXPORT InputConfiguration
	{
		static const int kMaxNumDevicesPerType = 8;
		static const int kMaxNumDevices = (u32)InputDevice::Count * kMaxNumDevicesPerType;

		/**	Internal virtual button data container. */
		struct VirtualButtonData
		{
			String Name;
			VirtualButton Button;
			VIRTUAL_BUTTON_DESC Desc;
		};

		/**	Internal virtual axis data container. */
		struct VirtualAxisData
		{
			String Name;
			VirtualAxis Axis;
			VIRTUAL_AXIS_DESC Desc;
		};

		/**	Internal container for holding axis data for all devices. */
		struct DeviceAxisData
		{
			VirtualAxisData Axes[(u32)InputAxis::Count];
		};

	public:
		InputConfiguration() = default;

		/**
		 * Registers a new virtual button.
		 *
		 * @param[in]	name		Unique name used to access the virtual button.
		 * @param[in]	buttonCode	Physical button the virtual button is triggered by.
		 * @param[in]	modifiers	Modifiers required to be pressed with the physical button to trigger the virtual button.
		 * @param[in]	repeatable	If true, the virtual button events will be sent continually while the physical button
		 *							is being held.
		 */
		void RegisterButton(const String& name, ButtonCode buttonCode, ButtonModifier modifiers = ButtonModifier::None, bool repeatable = false);

		/**	Unregisters a virtual button with the specified name. Events will no longer be generated for that button. */
		void UnregisterButton(const String& name);

		/**
		 * Registers a new virtual axis.
		 *
		 * @param[in]	name	Unique name used to access the axis.
		 * @param[in]	desc	Descriptor structure containing virtual axis creation parameters.
		 */
		void RegisterAxis(const String& name, const VIRTUAL_AXIS_DESC& desc);

		/**
		 * Unregisters a virtual axis with the specified name. You will no longer be able to retrieve valid values for that
		 * axis.
		 */
		void UnregisterAxis(const String& name);

		/**
		 * Sets repeat interval for held virtual buttons. Buttons will be continously triggered in interval increments as
		 * long as they button is being held.
		 */
		void SetRepeatInterval(u64 milliseconds) { mRepeatInterval = milliseconds; }

		/**	Gets the currently set repeat interval for held virtual buttons. */
		u64 GetRepeatInterval() const { return mRepeatInterval; }

		/** @name Internal
		 *  @{
		 */

		/**
		 * Returns data about virtual buttons that are triggered by the specified physical button code and modifier flags.
		 */
		bool GetButtonsInternal(ButtonCode code, u32 modifiers, Vector<VirtualButton>& btns, Vector<VIRTUAL_BUTTON_DESC>& btnDescs) const;

		/**	Retrieves virtual axis descriptor for the provided axis. */
		bool GetAxisInternal(const VirtualAxis& axis, VIRTUAL_AXIS_DESC& axisDesc) const;

		/** @} */

	private:
		Vector<VirtualButtonData> mButtons[BC_Count];
		Vector<VirtualAxisData> mAxes;

		u64 mRepeatInterval = 300;
	};

	/** @} */
} // namespace bs
