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
	enum class B3D_SCRIPT_EXPORT(API(Editor), DocumentationGroup(Editor - General)) PlayInEditorState
	{
		Stopped,
		Playing,
		Paused
	};

	/** Handles functionality specific to running the game in editor. */
	class B3D_SCRIPT_INTEROP_EXPORT B3D_SCRIPT_EXPORT(API(Editor), DocumentationGroup(Editor - General)) PlayInEditor : public Module<PlayInEditor>
	{
	public:
		PlayInEditor();

		/**	Returns the current play state of the game. */
		B3D_SCRIPT_EXPORT(InteropOnly(true))

		PlayInEditorState GetState() const { return mState; }

		/**
		 * Updates the play state of the game, making the game stop or start running. Note the actual state change
		 * will be delayed until the next update() call. Use the onPlay/onStopped/onPaused/onUnpaused event to get notified
		 * when the change actually happens.
		 */
		B3D_SCRIPT_EXPORT(InteropOnly(true))
		void SetState(PlayInEditorState state);

		/**	Runs the game for a single frame and then pauses it. */
		B3D_SCRIPT_EXPORT()
		void FrameStep();

		/** Triggered right after the play mode is entered. */
		B3D_SCRIPT_EXPORT()
		Event<void()> OnPlay;

		/** Triggered right after the play mode is exited. */
		B3D_SCRIPT_EXPORT(InteropOnly(true))
		Event<void()> OnStopped;

		/** Triggered right after the user pauses play mode. */
		B3D_SCRIPT_EXPORT()
		Event<void()> OnPaused;

		/** Triggered right after the user unpauses play mode. */
		B3D_SCRIPT_EXPORT()
		Event<void()> OnUnpaused;

		/** @name Internal
		 *  @{
		 */

		/**
		 * Called once per frame.
		 *
		 * @note	Internal method.
		 */
		void Update();

		/** @} */

	private:
		/**
		 * Updates the play state of the game, making the game stop or start running. Unlike setState() this will trigger
		 * the state change right away.
		 */
		void SetStateImmediate(PlayInEditorState state);

		/**	Saves the current state of the scene in memory. */
		void SaveSceneInMemory();

		/** Pauses or unpauses all pausable engine systems. */
		void SetSystemsPauseState(bool paused);

		PlayInEditorState mState;
		PlayInEditorState mNextState;
		bool mFrameStepActive;
		bool mScheduledStateChange;

		SPtr<GameObjectCollection> mSavedSceneGameObjectCollection;
		HSceneObject mSavedScene;
		UUID mSavedSceneResourceId;
	};

	/** @} */
} // namespace bs
