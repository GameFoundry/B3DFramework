//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsModule.h"
#include "BsInputConfiguration.h"

namespace bs
{
	/** @addtogroup Input-Engine
	 *  @{
	 */

	/**
	 * Handles virtual input that allows you to receive virtual input events that hide the actual physical input, allowing
	 * you to easily change the input keys while being transparent to the external code.
	 */
	class BS_EXPORT VirtualInput : public Module<VirtualInput>
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
			VirtualButton button;
			ButtonState state;
			UINT64 timestamp;
			UINT64 updateFrameIdx;
			bool allowRepeat;
		};

		/**	Contains button data for a specific input device. */
		struct DeviceData
		{
			Map<UINT32, ButtonData> cachedStates;
		};

		/**	Data container for a virtual button event. */
		struct VirtualButtonEvent
		{
			VirtualButton button;
			ButtonState state;
			UINT32 deviceIdx;
		};

	public:
		VirtualInput();

		/**	Creates a new empty input configuration. */
		static SPtr<InputConfiguration> createConfiguration();

		/**	Sets an input configuration that determines how physical keys map to virtual buttons. */
		void setConfiguration(const SPtr<InputConfiguration>& input);

		/**	Retrieves the active input configuration that determines how physical keys map to virtual buttons. */
		SPtr<InputConfiguration> getConfiguration() const { return mInputConfiguration; }

		/**
		 * Check is the virtual button just getting pressed. This state is only active for one frame.
		 *
		 * @param[in]	button		Virtual button identifier.
		 * @param[in]	deviceIdx	Optional device index in case multiple input devices are available.
		 */
		bool isButtonDown(const VirtualButton& button, UINT32 deviceIdx = 0) const;

		/**
		 * Check is the virtual button just getting released. This state is only active for one frame.
		 *
		 * @param[in]	button		Virtual button identifier.
		 * @param[in]	deviceIdx	Optional device index in case multiple input devices are available.
		 */
		bool isButtonUp(const VirtualButton& button, UINT32 deviceIdx = 0) const;

		/**
		 * Check is the virtual button is being held. This state is active as long as the button is being held down, 
		 * possibly for multiple frames.
		 *
		 * @param[in]	button		Virtual button identifier.
		 * @param[in]	deviceIdx	Optional device index in case multiple input devices are available.
		 */
		bool isButtonHeld(const VirtualButton& button, UINT32 deviceIdx = 0) const;

		/**
		 * Returns normalized value for the specified input axis. Returned value will usually be in [-1.0, 1.0] range, but
		 * can be outside the range for devices with unbound axes (for example mouse).
		 *
		 * @param[in]	axis		Virtual axis identifier.
		 * @param[in]	deviceIdx	Optional device index in case multiple input devices are available.
		 */
		float getAxisValue(const VirtualAxis& axis, UINT32 deviceIdx = 0) const;

		/**	Triggered when a virtual button is pressed. */
		Event<void(const VirtualButton&, UINT32 deviceIdx)> onButtonDown;

		/**	Triggered when a virtual button is released. */
		Event<void(const VirtualButton&, UINT32 deviceIdx)> onButtonUp;

		/**	Triggered every frame when a virtual button is being held down. */
		Event<void(const VirtualButton&, UINT32 deviceIdx)> onButtonHeld;

		/** @name Internal
		 *  @{
		 */

		/** Called once every frame. Triggers button callbacks. */
		void _update();

		/** @} */
	private:
		friend class VirtualButton;

		/** Performs all logic related to a button press. */
		void buttonDown(const ButtonEvent& event);

		/** Performs all logic related to a button release. */
		void buttonUp(const ButtonEvent& event);

		SPtr<InputConfiguration> mInputConfiguration;
		Vector<DeviceData> mDevices;
		Queue<VirtualButtonEvent> mEvents;
		UINT32 mActiveModifiers;

		// Transient
		Vector<VirtualButton> tempButtons;
		Vector<VIRTUAL_BUTTON_DESC> tempBtnDescs;
	};

	/** Provides easier access to VirtualInput. */
	BS_EXPORT VirtualInput& gVirtualInput();

	/** @} */
}