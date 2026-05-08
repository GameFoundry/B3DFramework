//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Scene/B3DPrefabUtility.h"
#include "Utility/B3DModule.h"

namespace b3d
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
	class B3D_SCRIPT_INTEROP_EXPORT B3D_SCRIPT_EXPORT(API(Editor), DocumentationGroup(Editor - General)) PlayInEditor : public IScriptExportable
	{
	public:
		PlayInEditor(const TShared<SceneInstance>& sceneInstance);

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

		/** Returns the scene that will be played when entering PIE. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Scene))
		TShared<SceneInstance> GetScene() const { return mAssociatedScene; }

		/**	Runs the game for a single frame and then pauses it. */
		B3D_SCRIPT_EXPORT()
		void FrameStep();

		/** Creates a new play in editor object associated with the provided scene instance. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(PlayInEditor))
		static TShared<PlayInEditor> Create(const TShared<SceneInstance>& sceneInstance) { return B3DMakeShared<PlayInEditor>(sceneInstance); }

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

		/** Called once per frame. */
		B3D_SCRIPT_EXPORT()
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

		TShared<SceneInstance> mAssociatedScene;

		PlayInEditorState mState;
		PlayInEditorState mNextState;
		bool mFrameStepActive;
		bool mScheduledStateChange;

		TShared<GameObjectCollection> mSavedSceneGameObjectCollection;
		HSceneObject mSavedScene;
		UnorderedMap<UUID, TShared<GameObjectInstanceData>> mSavedSceneInstanceData;
		UUID mSavedSceneResourceId;
	};

	/** @} */
} // namespace b3d
