//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPlayInEditor.h"
#include "Utility/BsTime.h"
#include "Scene/BsSceneManager.h"
#include "Scene/BsSceneObject.h"
#include "BsApplication.h"
#include "Physics/BsPhysics.h"
#include "Audio/BsAudio.h"
#include "Animation/BsAnimationManager.h"
#include "Scene/BsGameObjectCollection.h"

using namespace bs;
PlayInEditor::PlayInEditor()
	: mState(PlayInEditorState::Stopped), mNextState(PlayInEditorState::Stopped), mFrameStepActive(false), mScheduledStateChange(false)
{
	if(!GetApplication().IsEditor())
		mState = PlayInEditorState::Playing;
	else
	{
		SetSystemsPauseState(true);
		GetTime().ResetSimulationTime();
		GetSceneManager().SetComponentState(ComponentState::Stopped);
	}
}

void PlayInEditor::SetState(PlayInEditorState state)
{
	if(!GetApplication().IsEditor())
		return;

	// Delay state change to next frame as this method could be called in middle of object update, in which case
	// part of the objects before this call would receive different state than other objects.
	mScheduledStateChange = true;
	mNextState = state;
}

void PlayInEditor::SetStateImmediate(PlayInEditorState state)
{
	if(mState == state)
		return;

	PlayInEditorState oldState = mState;
	mState = state;

	switch(state)
	{
	case PlayInEditorState::Stopped:
		{
			mFrameStepActive = false;

			SetSystemsPauseState(true);
			GetTime().ResetSimulationTime();

			GetSceneManager().SetComponentState(ComponentState::Stopped);
			mSavedScene->Initialize();
			GetSceneManager().SetRootNodeInternal(mSavedScene, mSavedSceneResourceId);

			mSavedScene = nullptr;
			OnStopped();
		}
		break;
	case PlayInEditorState::Playing:
		{
			if(oldState == PlayInEditorState::Stopped)
				SaveSceneInMemory();

			GetSceneManager().SetComponentState(ComponentState::Running);
			SetSystemsPauseState(false);
			GetAnimationManager().SetPaused(false);

			if(oldState == PlayInEditorState::Stopped)
				OnPlay();
			else
				OnUnpaused();
		}
		break;
	case PlayInEditorState::Paused:
		{
			mFrameStepActive = false;
			SetSystemsPauseState(true);
			GetAnimationManager().SetPaused(true);

			if(oldState == PlayInEditorState::Stopped)
				SaveSceneInMemory();

			GetSceneManager().SetComponentState(ComponentState::Paused);

			if(oldState == PlayInEditorState::Stopped)
				OnPlay();

			OnPaused();
		}
		break;
	default:
		break;
	}
}

void PlayInEditor::FrameStep()
{
	if(!GetApplication().IsEditor())
		return;

	switch(mState)
	{
	case PlayInEditorState::Stopped:
	case PlayInEditorState::Paused:
		SetState(PlayInEditorState::Playing);
		break;
	default:
		break;
	}

	mFrameStepActive = true;
}

void PlayInEditor::Update()
{
	if(mScheduledStateChange)
	{
		SetStateImmediate(mNextState);
		mScheduledStateChange = false;
	}

	if(mFrameStepActive)
	{
		SetState(PlayInEditorState::Paused);
		mFrameStepActive = false;
	}
}

void PlayInEditor::SaveSceneInMemory()
{
	mSavedSceneGameObjectCollection = GameObjectCollection::Create();
	mSavedScene = SceneManager::Instance().GetMainScene()->GetRoot()->Clone(mSavedSceneGameObjectCollection, true);
	mSavedSceneResourceId = SceneManager::Instance().GetMainScene()->GetAssociatedResourceId();

	// Remove objects with "dont save" flag
	Stack<HSceneObject> todo;
	todo.push(mSavedScene);

	while(!todo.empty())
	{
		HSceneObject current = todo.top();
		todo.pop();

		if(current->HasFlag(SceneObjectFlag::DontSave))
			current->Destroy();
		else
		{
			u32 numChildren = current->GetChildCount();
			for(u32 i = 0; i < numChildren; i++)
				todo.push(current->GetChild(i));
		}
	}
}

void PlayInEditor::SetSystemsPauseState(bool paused)
{
	GetTime().SetSimulationTimePaused(paused);
	GetPhysics().SetPaused(paused);
	GetAudio().SetPaused(paused);
}
