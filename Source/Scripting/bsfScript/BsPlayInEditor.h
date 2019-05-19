//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Utility/BsModule.h"

namespace bs
{
	/** @addtogroup bsfScript
	 *  @{
	 */

	/**	States the game in editor can be in. */
	enum class BS_SCRIPT_EXPORT(api:bed,m:Editor-General) PlayInEditorState
	{
		Stopped,
		Playing,
		Paused
	};

	/** Handles functionality specific to running the game in editor. */
	class BS_SCR_BE_EXPORT BS_SCRIPT_EXPORT(api:bed,m:Editor-General) PlayInEditor : public Module<PlayInEditor>
	{
	public:
		PlayInEditor();

		/**	Returns the current play state of the game. */
		BS_SCRIPT_EXPORT(in:true)
		PlayInEditorState getState() const { return mState; }

		/**	
		 * Updates the play state of the game, making the game stop or start running. Note the actual state change
		 * will be delayed until the next update() call. Use the onPlay/onStopped/onPaused/onUnpaused event to get notified 
		 * when the change actually happens.
		 */
		BS_SCRIPT_EXPORT(in:true)
		void setState(PlayInEditorState state);

		/**
		 * Gets the number of seconds that have elapsed since the game was started. This time does not include time passed
		 * while the game is paused.
		 */
		float getPausableTime() const { return mPausableTime; }

		/**	Runs the game for a single frame and then pauses it. */
		BS_SCRIPT_EXPORT()
		void frameStep();

		/** Triggered right after the play mode is entered. */
		BS_SCRIPT_EXPORT()
		Event<void()> onPlay;

		/** Triggered right after the play mode is exited. */
		BS_SCRIPT_EXPORT(in:true)
		Event<void()> onStopped;

		/** Triggered right after the user pauses play mode. */
		BS_SCRIPT_EXPORT()
		Event<void()> onPaused;

		/** Triggered right after the user unpauses play mode. */
		BS_SCRIPT_EXPORT()
		Event<void()> onUnpaused;

		/** @name Internal
		 *  @{
		 */

		/**
		 * Called once per frame.
		 *
		 * @note	Internal method.
		 */
		void update();

		/** @} */

	private:
		/**
		 * Updates the play state of the game, making the game stop or start running. Unlike setState() this will trigger
		 * the state change right away.
		 */
		void setStateImmediate(PlayInEditorState state);

		/**	Saves the current state of the scene in memory. */
		void saveSceneInMemory();

		/** Pauses or unpauses all pausable engine systems. */
		void setSystemsPauseState(bool paused);

		PlayInEditorState mState;
		PlayInEditorState mNextState;
		bool mFrameStepActive;
		bool mScheduledStateChange;

		float mPausableTime;
		HSceneObject mSavedScene;
	};

	/** @} */
}