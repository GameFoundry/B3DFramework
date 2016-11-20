//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"

namespace bs
{
	/** @addtogroup Handles-Internal
	 *  @{
	 */

	/** Controls all instantiated HandleSlider%s. */
	class BS_ED_EXPORT HandleSliderManager
	{
		/** Contains active/hover sliders for each camera. */
		struct StatePerCamera
		{
			HandleSlider* activeSlider = nullptr;
			HandleSlider* hoverSlider = nullptr;
		};

	public:
		/**
		 * Updates all underlying sliders, changing their state and dragging them depending on their state and pointer 
		 * movement.
		 *
		 * @param[in]	camera		Camera through which we're interacting with sliders.
		 * @param[in]	inputPos	Position of the pointer.
		 * @param[in]	inputDelta	Movement of the pointer since last frame.
		 */
		void update(const SPtr<Camera>& camera, const Vector2I& inputPos, const Vector2I& inputDelta);

		/**
		 * Attempts to select (activate) a slider at the specified position.
		 *
		 * @param[in]	camera		Camera through which we're interacting with sliders.
		 * @param[in]	inputPos	Position of the pointer.
		 */
		void trySelect(const SPtr<Camera>& camera, const Vector2I& inputPos);

		/** Clears the active slider (deactivates it) for the specified camera. */
		void clearSelection(const SPtr<Camera>& camera);

		/** Checks is any slider active for the specified camera. */
		bool isSliderActive(const SPtr<Camera>& camera) const;

		/** Registers a new instantiated slider. */
		void _registerSlider(HandleSlider* slider);

		/**	Unregisters a previously instantiated slider. */
		void _unregisterSlider(HandleSlider* slider);

	private:

		/**
		 * Attempts to find slider at the specified position.
		 *
		 * @param[in]	camera		Camera through which we're interacting with sliders.
		 * @param[in]	inputPos	Position of the pointer.
		 * @return					Slider if we're intersecting with one, or null otherwise.
		 */
		HandleSlider* findUnderCursor(const SPtr<Camera>& camera, const Vector2I& inputPos) const;

		UnorderedMap<UINT64, StatePerCamera> mStates;
		UnorderedSet<HandleSlider*> mSliders;
	};

	/** @} */
}