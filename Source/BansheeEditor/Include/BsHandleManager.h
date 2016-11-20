//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsModule.h"
#include "BsDegree.h"

namespace bs
{
	/** @addtogroup Handles-Internal
	 *  @{
	 */

	/** The central place for interacting with and drawing handles. */
	class BS_ED_EXPORT HandleManager : public Module<HandleManager>
	{
	public:
		HandleManager();
		virtual ~HandleManager();

		/** 
		 * Triggers handle pre-input callbacks. Must be called before all updateInput() calls and followed by endInput(). 
		 * This should be called once per frame.
		 */
		void beginInput();

		/**
		 * To be called every frame. Updates interactable handle sliders based on provided input. Make sure to call 
		 * beginInput() before this method, followed by endInput() when done.
		 *
		 * @param[in]	camera		Camera that the input positions are relative to.
		 * @param[in]	inputPos	Position of the pointer, relative to the provided camera viewport.
		 * @param[in]	inputDelta	Determines pointer movement since last call to this method.
		 */
		void updateInput(const SPtr<Camera>& camera, const Vector2I& inputPos, const Vector2I& inputDelta);

		/** 
		 * Triggers handle post-input callbacks. Must be called after all updateInput() calls and after beginInput().
		 * This should be called once per frame.
		 */
		void endInput();

		/**
		 * To be called every frame. Queues handles for drawing.
		 *
		 * @param[in]	camera		Camera to draw the handles to.
		 */
		void draw(const SPtr<Camera>& camera);

		/**
		 * Select a handle slider at the specified location, if there is any under the pointer. Makes the selected slider 
		 * active and draggable.
		 *
		 * @param[in]	camera		Camera that the input positions are relative to, and destination to draw the handles to.
		 * @param[in]	inputPos	Position of the pointer, relative to the provided camera viewport.
		 */
		void trySelect(const SPtr<Camera>& camera, const Vector2I& inputPos);

		/**	Clears the currently selected/active handle slider for the specified camera. */
		void clearSelection(const SPtr<Camera>& camera);

		/**	Is any handle slider selected/active on the specified camera. */
		bool isHandleActive(const SPtr<Camera>& camera) const;

		/** Returns the manager that can be used for interacting with handle sliders. */
		HandleSliderManager& getSliderManager() const { return *mSliderManager; }

		/** Returns the manager that can be used for drawing handle geometry. */
		HandleDrawManager& getDrawManager() const { return *mDrawManager; }

		/**	
		 * Returns the uniform size for a handle rendered in @p camera, at the world position @p handlePos. The handles 
		 * will be scaled so that they appear the same size regardless of distance from camera.
		 */
		float getHandleSize(const SPtr<Camera>& camera, const Vector3& handlePos) const;

		/** Sets the default handle size. This controls the uniform scale returned from getHandleSize() method. */
		void setDefaultHandleSize(float value) { mDefaultHandleSize = value; }

		/** Sets editor settings that will be used for controlling various handle behaviour. */
		void setSettings(const SPtr<EditorSettings>& settings);

	protected:
		/** Updates the internal properties from editor settings. */
		void updateFromEditorSettings();

		/** Called during handle update. Allows handle sliders to be created or destroyed before any input is handled. */
		virtual void triggerPreInput() = 0;

		/**
		 * Called during handle update after handle input is processed. Allows implementation to respond to delta values 
		 * calculated in sliders due to input.
		 */
		virtual void triggerPostInput() = 0;

		/** Called during handle update. Allows implementation to queue handle draw commands. */
		virtual void queueDrawCommands() = 0;

		HandleSliderManager* mSliderManager;
		HandleDrawManager* mDrawManager;

		float mDefaultHandleSize = 20.0f;
		bool mInputStarted;

		SPtr<EditorSettings> mSettings;
		UINT32 mSettingsHash;
		UINT64 mLastDrawFrameIdx;
	};

	/** @} */
}