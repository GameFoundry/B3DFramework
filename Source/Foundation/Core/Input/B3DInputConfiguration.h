//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Input/B3DInputFwd.h"

namespace b3d
{
	/** @addtogroup Input
	 *  @{
	 */

	/**
	 * Describes a virtual button. Virtual buttons allow you to map custom actions without needing to know about what
	 * physical buttons trigger those actions.
	 */
	struct B3D_EXPORT VirtualButtonInformation
	{
		VirtualButtonInformation() = default;

		/**
		 * Constructs a virtual button descriptor.
		 *
		 * @param	buttonCode	Physical button the virtual button is triggered by.
		 * @param	modifiers	Modifiers required to be pressed with the physical button to trigger the virtual button.
		 * @param	repeatable	If true, the virtual button events will be sent continually while the physical button
		 *						is being held.
		 */
		VirtualButtonInformation(ButtonCode buttonCode, ButtonModifier modifiers = ButtonModifier::None, bool repeatable = false);

		ButtonCode ButtonCode = ButtonCode::Key0;
		ButtonModifier Modifiers = ButtonModifier::None;
		bool Repeatable = false;
	};

	/** Information used for initializing a virtual button. */
	struct B3D_EXPORT VirtualButtonCreateInformation : VirtualButtonInformation { };

	/**
	 * Describes a virtual axis. Virtual axes allow you to map custom axes without needing to know the actual physical
	 * device handling those axes.
	 */
	struct B3D_EXPORT VirtualAxisInformation
	{
		VirtualAxisInformation() = default;

		/**
		 * Constructs a new virtual axis descriptor.
		 *
		 * @param	type	@copydoc VirtualAxisInformation::Type
		 */
		VirtualAxisInformation(u32 type);

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

	/** Information used for initializing a virtual axis. */
	struct B3D_EXPORT B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Input)) VirtualAxisCreateInformation : VirtualAxisInformation
	{
		using VirtualAxisInformation::VirtualAxisInformation;
	};

	/**
	 * Identifier for a virtual button.
	 *
	 * Primary purpose of this class is to avoid expensive string compare, and instead use a unique button identifier for
	 * compare. Generally you want to create one of these using the button name, and then store it for later use.
	 *
	 * @note
	 * This class is not thread safe and should only be used on the main thread.
	 *
	 * @see		VirtualButtonInformation
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Input)) VirtualButton
	{
	public:
		VirtualButton() = default;

		B3D_SCRIPT_EXPORT(Exclude(true))
		VirtualButton(u32 buttonIdentifier)
			: ButtonIdentifier(buttonIdentifier)
		{ }

		bool operator==(const VirtualButton& rhs) const
		{
			return (ButtonIdentifier == rhs.ButtonIdentifier);
		}

		u32 ButtonIdentifier = 0;
	};

	/**
	 * Identifier for a virtual axis.
	 *
	 * Primary purpose of this class is to avoid expensive string compare (axis names), and instead use a unique axis
	 * identifier for compare. Generally you want to create one of these using the axis name, and then store it for later
	 * use.
	 *
	 * @note
	 * This class is not thread safe and should only be used on the main thread.
	 *
	 * @see		VirtualAxisCreateInformation
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Input)) VirtualAxis
	{
	public:
		VirtualAxis() = default;

		B3D_SCRIPT_EXPORT(Exclude(true))
		VirtualAxis(u32 axisIdentifier)
			: AxisIdentifier(axisIdentifier)
		{ }

		u32 AxisIdentifier = 0;

		bool operator==(const VirtualAxis& rhs) const
		{
			return (AxisIdentifier == rhs.AxisIdentifier);
		}
	};

	/**	Contains virtual <-> physical key mappings. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Input)) InputConfiguration : public IScriptExportable
	{
		static const int kMaxDeviceCountPerType = 8;
		static const int kMaxNumDevices = (u32)InputDevice::Count * kMaxDeviceCountPerType;

		/**	Internal virtual button data container. */
		struct VirtualButtonData
		{
			String Name;
			VirtualButton Button;
			VirtualButtonInformation Desc;
		};

		/**	Internal virtual axis data container. */
		struct VirtualAxisData
		{
			String Name;
			VirtualAxis Axis;
			VirtualAxisInformation Desc;
		};

		/**	Internal container for holding axis data for all devices. */
		struct DeviceAxisData
		{
			VirtualAxisData Axes[(u32)InputAxis::Count];
		};

	public:
		B3D_SCRIPT_EXPORT()
		InputConfiguration() = default;

		/**
		 * Registers a new virtual button.
		 *
		 * @param	name			Unique name used to access the virtual button.
		 * @param	buttonCode		Physical button the virtual button is triggered by.
		 * @param	modifiers		Modifiers required to be pressed with the physical button to trigger the virtual button.
		 * @param	repeatable		If true, the virtual button events will be sent continually while the physical button
		 *							is being held.
		 */
		B3D_SCRIPT_EXPORT()
		void RegisterButton(const String& name, ButtonCode buttonCode, ButtonModifier modifiers = ButtonModifier::None, bool repeatable = false);

		/**	Unregisters a virtual button with the specified name. Events will no longer be generated for that button. */
		B3D_SCRIPT_EXPORT()
		void UnregisterButton(const String& name);

		/**
		 * Registers a new virtual axis.
		 *
		 * @param	name				Unique name used to access the axis.
		 * @param	createInformation	Descriptor structure containing virtual axis creation parameters.
		 */
		B3D_SCRIPT_EXPORT()
		void RegisterAxis(const String& name, const VirtualAxisCreateInformation& createInformation);

		/**
		 * Unregisters a virtual axis with the specified name. You will no longer be able to retrieve valid values for that
		 * axis.
		 */
		B3D_SCRIPT_EXPORT()
		void UnregisterAxis(const String& name);

		/**
		 * Repeat interval for held virtual buttons. Buttons will be continously triggered in interval increments as
		 * long as they button is being held.
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(RepeatInterval))
		void SetRepeatInterval(u64 milliseconds) { mRepeatInterval = milliseconds; }

		/** @copydoc SetRepeatInterval */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(RepeatInterval))
		u64 GetRepeatInterval() const { return mRepeatInterval; }

		/** @name Internal
		 *  @{
		 */

		/**
		 * Returns data about virtual buttons that are triggered by the specified physical button code and modifier flags.
		 */
		bool GetButtonsInternal(ButtonCode code, u32 modifiers, Vector<VirtualButton>& buttons, Vector<VirtualButtonInformation>& buttonDescriptions) const;

		/**	Retrieves virtual axis descriptor for the provided axis. */
		bool GetAxisInternal(const VirtualAxis& axis, VirtualAxisInformation& axisDescription) const;

		/** @} */

	private:
		Vector<VirtualButtonData> mButtons[static_cast<size_t>(ButtonCode::TotalKeyCount)];
		Vector<VirtualAxisData> mAxes;

		u64 mRepeatInterval = 300;
	};

	/** @} */
} // namespace b3d
