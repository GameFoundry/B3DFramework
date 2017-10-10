//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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
		VIRTUAL_BUTTON_DESC();

		/**
		 * Constructs a virtual button descriptor.
		 *
		 * @param[in]	buttonCode	Physical button the virtual button is triggered by.
		 * @param[in]	modifiers	Modifiers required to be pressed with the physical button to trigger the virtual button.
		 * @param[in]	repeatable	If true, the virtual button events will be sent continually while the physical button 
		 *							is being held.
		 */
		VIRTUAL_BUTTON_DESC(ButtonCode buttonCode, ButtonModifier modifiers = ButtonModifier::None, bool repeatable = false);

		ButtonCode buttonCode;
		ButtonModifier modifiers;
		bool repeatable;
	};

	/**
	 * Describes a virtual axis. Virtual axes allow you to map custom axes without needing to know the actual physical
	 * device handling those axes.
	 */
	struct BS_EXPORT VIRTUAL_AXIS_DESC
	{
		VIRTUAL_AXIS_DESC() {}

		/**
		 * Constructs a new virtual axis descriptor.
		 *
		 * @param[in]	type		@copydoc VIRTUAL_AXIS_DESC::type
		 */
		VIRTUAL_AXIS_DESC(UINT32 type);

		/** Type of physical axis to map to. See InputAxis type for common types, but you are not limited to those values. */
		UINT32 type = (UINT32)InputAxis::MouseX;

		/** Value below which to ignore axis value and consider it 0. */
		float deadZone = 0.0001f;

		/** Higher sensitivity means the axis will more easily reach its maximum values. */
		float sensitivity = 1.0f;

		/** Should the axis be inverted. */
		bool invert = false;

		/**
		 * If enabled, axis values will be normalized to [-1, 1] range. Most axes already come in normalized form and this
		 * value will not affect such axes. Some axes, like mouse movement are not normalized by default and will instead
		 * report relative movement. By enabling this you will normalize such axes to [-1, 1] range.
		 */
		bool normalize = false;
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
		VirtualButton();
		VirtualButton(const String& name);

		bool operator== (const VirtualButton& rhs) const
		{
			return (buttonIdentifier == rhs.buttonIdentifier);
		}

		UINT32 buttonIdentifier;
	private:
		/** Returns a static map of all virtual button identifiers and their buttons. */
		static Map<String, UINT32>& getUniqueButtonIds();

		static UINT32 NextButtonId;
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
		VirtualAxis();
		VirtualAxis(const String& name);

		UINT32 axisIdentifier;

		bool operator== (const VirtualAxis& rhs) const
		{
			return (axisIdentifier == rhs.axisIdentifier);
		}

	private:
		static Map<String, UINT32> UniqueAxisIds;
		static UINT32 NextAxisId;
	};

	/**	Contains virtual <-> physical key mappings. */
	class BS_EXPORT InputConfiguration
	{
		static const int MAX_NUM_DEVICES_PER_TYPE = 8;
		static const int MAX_NUM_DEVICES = (UINT32)InputDevice::Count * MAX_NUM_DEVICES_PER_TYPE;

		/**	Internal virtual button data container. */
		struct VirtualButtonData
		{
			String name;
			VirtualButton button;
			VIRTUAL_BUTTON_DESC desc;
		};

		/**	Internal virtual axis data container. */
		struct VirtualAxisData
		{
			String name;
			VirtualAxis axis;
			VIRTUAL_AXIS_DESC desc;
		};

		/**	Internal container for holding axis data for all devices. */
		struct DeviceAxisData
		{
			VirtualAxisData axes[(UINT32)InputAxis::Count];
		};

	public:
		InputConfiguration();

		/**
		 * Registers a new virtual button.
		 *
		 * @param[in]	name		Unique name used to access the virtual button.
		 * @param[in]	buttonCode	Physical button the virtual button is triggered by.
		 * @param[in]	modifiers	Modifiers required to be pressed with the physical button to trigger the virtual button.
		 * @param[in]	repeatable	If true, the virtual button events will be sent continually while the physical button 
		 *							is being held.
		 */
		void registerButton(const String& name, ButtonCode buttonCode, ButtonModifier modifiers = ButtonModifier::None, bool repeatable = false);

		/**	Unregisters a virtual button with the specified name. Events will no longer be generated for that button. */
		void unregisterButton(const String& name);

		/**
		 * Registers a new virtual axis.
		 *
		 * @param[in]	name	Unique name used to access the axis.
		 * @param[in]	desc	Descriptor structure containing virtual axis creation parameters.
		 */
		void registerAxis(const String& name, const VIRTUAL_AXIS_DESC& desc);

		/**
		 * Unregisters a virtual axis with the specified name. You will no longer be able to retrieve valid values for that
		 * axis.
		 */
		void unregisterAxis(const String& name);

		/**
		 * Sets repeat interval for held virtual buttons. Buttons will be continously triggered in interval increments as
		 * long as they button is being held.
		 */
		void setRepeatInterval(UINT64 milliseconds) { mRepeatInterval = milliseconds; }

		/**	Gets the currently set repeat interval for held virtual buttons. */
		UINT64 getRepeatInterval() const { return mRepeatInterval; }

		/** @name Internal
		 *  @{
		 */

		/** 
		 * Returns data about virtual buttons that are triggered by the specified physical button code and modifier flags. 
		 */
		bool _getButtons(ButtonCode code, UINT32 modifiers, Vector<VirtualButton>& btns, Vector<VIRTUAL_BUTTON_DESC>& btnDescs) const;

		/**	Retrieves virtual axis descriptor for the provided axis. */
		bool _getAxis(const VirtualAxis& axis, VIRTUAL_AXIS_DESC& axisDesc) const;

		/** @} */

	private:
		Vector<VirtualButtonData> mButtons[BC_Count];
		Vector<VirtualAxisData> mAxes;

		UINT64 mRepeatInterval;
	};

	/** @} */
}