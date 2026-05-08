//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Input/B3DInputConfiguration.h"

namespace b3d
{
	/** @addtogroup Input
	 *  @{
	 */

	/**
	 * Handles virtual input that allows you to receive virtual input events that hide the actual physical input, allowing
	 * you to easily change the input keys while being transparent to the external code.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Input)) VirtualInput : public Module<VirtualInput>
	{
		/**	Possible states of virtual buttons. */
		enum class ButtonState
		{
			Off,
			On,
			ToggledOn,
			ToggledOff
		};

		/**	Data container for a single virtual button. */
		struct ButtonData
		{
			VirtualButton Button;
			ButtonState State;
			u64 Timestamp;
			u64 UpdateFrameIdx;
			bool AllowRepeat;
		};

		/**	Contains button data for a specific input device. */
		struct DeviceData
		{
			Map<u32, ButtonData> CachedStates;
			TArray<u32> HeldButtons;
		};

		/**	Data container for a virtual button event. */
		struct VirtualButtonEvent
		{
			VirtualButton Button;
			ButtonState State;
			u32 DeviceIdx;
		};

	public:
		VirtualInput();

		/**	Input configuration that determines how physical keys map to virtual buttons. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Configuration))
		void SetConfiguration(const TShared<InputConfiguration>& input);

		/** @copydoc SetConfiguration */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Configuration))
		TShared<InputConfiguration> GetConfiguration() const { return mInputConfiguration; }

		/** Creates a new virtual button associated with the name, or returns an existing button if it exists. */
		B3D_SCRIPT_EXPORT()
		static VirtualButton GetOrCreateVirtualButton(const String& name);

		/** Creates a new virtual axis associated with the name, or returns an existing axis if it exists. */
		B3D_SCRIPT_EXPORT()
		static VirtualAxis GetOrCreateVirtualAxis(const String& name);

		/**
		 * Check is the virtual button just getting pressed. This state is only active for one frame.
		 *
		 * @param	button			Virtual button identifier.
		 * @param	deviceIndex		Optional device index in case multiple input devices are available.
		 */
		B3D_SCRIPT_EXPORT()
		bool IsButtonDown(const VirtualButton& button, u32 deviceIndex = 0) const;

		/**
		 * Check is the virtual button just getting released. This state is only active for one frame.
		 *
		 * @param	button			Virtual button identifier.
		 * @param	deviceIndex		Optional device index in case multiple input devices are available.
		 */
		B3D_SCRIPT_EXPORT()
		bool IsButtonUp(const VirtualButton& button, u32 deviceIndex = 0) const;

		/**
		 * Check is the virtual button is being held. This state is active as long as the button is being held down,
		 * possibly for multiple frames.
		 *
		 * @param	button			Virtual button identifier.
		 * @param	deviceIndex		Optional device index in case multiple input devices are available.
		 */
		B3D_SCRIPT_EXPORT()
		bool IsButtonHeld(const VirtualButton& button, u32 deviceIndex = 0) const;

		/**
		 * Returns normalized value for the specified input axis. Returned value will usually be in [-1.0, 1.0] range, but
		 * can be outside the range for devices with unbound axes (for example mouse).
		 *
		 * @param	axis			Virtual axis identifier.
		 * @param	deviceIndex		Optional device index in case multiple input devices are available.
		 */
		B3D_SCRIPT_EXPORT()
		float GetAxisValue(const VirtualAxis& axis, u32 deviceIndex = 0) const;

		/**	Triggered when a virtual button is pressed. */
		B3D_SCRIPT_EXPORT()
		Event<void(const VirtualButton&, u32 deviceIndex)> OnButtonDown;

		/**	Triggered when a virtual button is released. */
		B3D_SCRIPT_EXPORT()
		Event<void(const VirtualButton&, u32 deviceIndex)> OnButtonUp;

		/**	Triggered every frame when a virtual button is being held down. */
		B3D_SCRIPT_EXPORT()
		Event<void(const VirtualButton&, u32 deviceIndex)> OnButtonHeld;

		/** @name Internal
		 *  @{
		 */

		/** Called once every frame. Triggers button callbacks. */
		void UpdateInternal();

		/** @} */
	private:
		friend class VirtualButton;

		/** Performs all logic related to a button press. */
		void ButtonDown(const ButtonEvent& event);

		/** Performs all logic related to a button release. */
		void ButtonUp(const ButtonEvent& event);

		TShared<InputConfiguration> mInputConfiguration;
		Vector<DeviceData> mDevices;
		Queue<VirtualButtonEvent> mEvents;
		u32 mActiveModifiers = (u32)ButtonModifier::None;

		// Transient
		Vector<VirtualButton> tempButtons;
		Vector<VirtualButtonInformation> tempBtnDescs;

		static u32 sNextVirtualButtonId;
		static u32 sNextVirtualAxisId;
	};

	/** Provides easier access to VirtualInput. */
	B3D_EXPORT VirtualInput& GetVirtualInput();

	/** @} */
} // namespace b3d
