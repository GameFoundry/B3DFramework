//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Utility/BsModule.h"
#include "Input/BsInputConfiguration.h"

namespace bs
{
	/** @addtogroup Input-Engine
	 *  @{
	 */

	/**
	 * Handles virtual input that allows you to receive virtual input events that hide the actual physical input, allowing
	 * you to easily change the input keys while being transparent to the external code.
	 */
	class B3D_EXPORT VirtualInput : public Module<VirtualInput>
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

		/**	Creates a new empty input configuration. */
		static SPtr<InputConfiguration> CreateConfiguration();

		/**	Sets an input configuration that determines how physical keys map to virtual buttons. */
		void SetConfiguration(const SPtr<InputConfiguration>& input);

		/**	Retrieves the active input configuration that determines how physical keys map to virtual buttons. */
		SPtr<InputConfiguration> GetConfiguration() const { return mInputConfiguration; }

		/**
		 * Check is the virtual button just getting pressed. This state is only active for one frame.
		 *
		 * @param[in]	button		Virtual button identifier.
		 * @param[in]	deviceIdx	Optional device index in case multiple input devices are available.
		 */
		bool IsButtonDown(const VirtualButton& button, u32 deviceIdx = 0) const;

		/**
		 * Check is the virtual button just getting released. This state is only active for one frame.
		 *
		 * @param[in]	button		Virtual button identifier.
		 * @param[in]	deviceIdx	Optional device index in case multiple input devices are available.
		 */
		bool IsButtonUp(const VirtualButton& button, u32 deviceIdx = 0) const;

		/**
		 * Check is the virtual button is being held. This state is active as long as the button is being held down,
		 * possibly for multiple frames.
		 *
		 * @param[in]	button		Virtual button identifier.
		 * @param[in]	deviceIdx	Optional device index in case multiple input devices are available.
		 */
		bool IsButtonHeld(const VirtualButton& button, u32 deviceIdx = 0) const;

		/**
		 * Returns normalized value for the specified input axis. Returned value will usually be in [-1.0, 1.0] range, but
		 * can be outside the range for devices with unbound axes (for example mouse).
		 *
		 * @param[in]	axis		Virtual axis identifier.
		 * @param[in]	deviceIdx	Optional device index in case multiple input devices are available.
		 */
		float GetAxisValue(const VirtualAxis& axis, u32 deviceIdx = 0) const;

		/**	Triggered when a virtual button is pressed. */
		Event<void(const VirtualButton&, u32 deviceIdx)> OnButtonDown;

		/**	Triggered when a virtual button is released. */
		Event<void(const VirtualButton&, u32 deviceIdx)> OnButtonUp;

		/**	Triggered every frame when a virtual button is being held down. */
		Event<void(const VirtualButton&, u32 deviceIdx)> OnButtonHeld;

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

		SPtr<InputConfiguration> mInputConfiguration;
		Vector<DeviceData> mDevices;
		Queue<VirtualButtonEvent> mEvents;
		u32 mActiveModifiers = (u32)ButtonModifier::None;

		// Transient
		Vector<VirtualButton> tempButtons;
		Vector<VIRTUAL_BUTTON_DESC> tempBtnDescs;
	};

	/** Provides easier access to VirtualInput. */
	B3D_EXPORT VirtualInput& GetVirtualInput();

	/** @} */
} // namespace bs
