//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPlayInEditor.h"
#include "BsScriptGameObjectManager.h"
#include "Utility/BsTime.h"
#include "Scene/BsSceneManager.h"
#include "Scene/BsSceneObject.h"
#include "BsApplication.h"
#include "Physics/BsPhysics.h"
#include "Audio/BsAudio.h"
#include "Animation/BsAnimationManager.h"

namespace bs
{
	PlayInEditor::PlayInEditor()
		:mState(PlayInEditorState::Stopped), mNextState(PlayInEditorState::Stopped), 
		mFrameStepActive(false), mScheduledStateChange(false), mPausableTime(0.0f)
	{
		if (!gApplication().isEditor())
			mState = PlayInEditorState::Playing;
		else
		{
			setSystemsPauseState(true);
			gSceneManager().setComponentState(ComponentState::Stopped);
		}
	}

	void PlayInEditor::setState(PlayInEditorState state)
	{
		if (!gApplication().isEditor())
			return;

		// Delay state change to next frame as this method could be called in middle of object update, in which case
		// part of the objects before this call would receive different state than other objects.
		mScheduledStateChange = true;
		mNextState = state;
	}

	void PlayInEditor::setStateImmediate(PlayInEditorState state)
	{
		if (mState == state)
			return;

		PlayInEditorState oldState = mState;
		mState = state;

		switch (state)
		{
		case PlayInEditorState::Stopped:
		{
			mFrameStepActive = false;
			mPausableTime = 0.0f;

			setSystemsPauseState(true);

			gSceneManager().setComponentState(ComponentState::Stopped);
			mSavedScene->_instantiate();
			gSceneManager()._setRootNode(mSavedScene);

			mSavedScene = nullptr;
			onStopped();
		}
			break;
		case PlayInEditorState::Playing:
		{
			if (oldState == PlayInEditorState::Stopped)
				saveSceneInMemory();

			gSceneManager().setComponentState(ComponentState::Running);
			setSystemsPauseState(false);
			gAnimation().setPaused(false);

			if (oldState == PlayInEditorState::Stopped)
				onPlay();
			else
				onUnpaused();
		}
			break;
		case PlayInEditorState::Paused:
		{
			mFrameStepActive = false;
			setSystemsPauseState(true);
			gAnimation().setPaused(true);

			if (oldState == PlayInEditorState::Stopped)
				saveSceneInMemory();

			gSceneManager().setComponentState(ComponentState::Paused);

			if (oldState == PlayInEditorState::Stopped)
				onPlay();

			onPaused();
		}
			break;
		default:
			break;
		}		
	}

	void PlayInEditor::frameStep()
	{
		if (!gApplication().isEditor())
			return;

		switch (mState)
		{
		case PlayInEditorState::Stopped:
		case PlayInEditorState::Paused:
			setState(PlayInEditorState::Playing);
			break;
		default:
			break;
		}

		mFrameStepActive = true;
	}

	void PlayInEditor::update()
	{
		if (mState == PlayInEditorState::Playing)
			mPausableTime += gTime().getFrameDelta();

		if (mScheduledStateChange)
		{
			setStateImmediate(mNextState);
			mScheduledStateChange = false;
		}

		if (mFrameStepActive)
		{
			setState(PlayInEditorState::Paused);
			mFrameStepActive = false;
		}
	}

	void PlayInEditor::saveSceneInMemory()
	{
		mSavedScene = SceneManager::instance().getMainScene()->getRoot()->clone(false, true);

		// Remove objects with "dont save" flag
		Stack<HSceneObject> todo;
		todo.push(mSavedScene);

		while (!todo.empty())
		{
			HSceneObject current = todo.top();
			todo.pop();

			if (current->hasFlag(SOF_DontSave))
				current->destroy();
			else
			{
				UINT32 numChildren = current->getNumChildren();
				for (UINT32 i = 0; i < numChildren; i++)
					todo.push(current->getChild(i));
			}
		}
	}

	void PlayInEditor::setSystemsPauseState(bool paused)
	{
		gPhysics().setPaused(paused);
		gAudio().setPaused(paused);
	}
}