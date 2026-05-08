//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DPlayInEditor.h"
#include "Utility/B3DTime.h"
#include "Scene/B3DSceneObject.h"
#include "B3DApplication.h"
#include "Physics/B3DPhysics.h"
#include "Audio/B3DAudio.h"
#include "Animation/B3DAnimationScene.h"
#include "Scene/B3DGameObjectCollection.h"
#include "Scene/B3DSceneInstance.h"
#include "Scene/B3DSceneUtility.h"

using namespace b3d;
PlayInEditor::PlayInEditor(const TShared<SceneInstance>& scene)
	: mAssociatedScene(scene), mState(PlayInEditorState::Stopped), mNextState(PlayInEditorState::Stopped), mFrameStepActive(false), mScheduledStateChange(false)
{
	if(!GetApplication().IsEditor())
		mState = PlayInEditorState::Playing;
	else
	{
		SetSystemsPauseState(true);
		mAssociatedScene->GetTime().Reset();
		mAssociatedScene->SetComponentState(ComponentState::Stopped);
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

			mAssociatedScene->GetTime().Reset();
			mAssociatedScene->SetComponentState(ComponentState::Stopped);
			mAssociatedScene->SetRoot(mSavedScene);
			mAssociatedScene->SetAssociatedResourceId(mSavedSceneResourceId);

			// Restore instance data as cloning the hierarchy created new game object handles, and we wish to ensure that anything holding the old handles still remains valid.
			SceneUtility::RestoreSceneObjectHierarchyInstanceData(mAssociatedScene->GetRoot(), mSavedSceneInstanceData);
			mSavedSceneInstanceData = {};

			mSavedScene->Initialize();

			mSavedScene = nullptr;
			OnStopped();
		}
		break;
	case PlayInEditorState::Playing:
		{
			if(oldState == PlayInEditorState::Stopped)
				SaveSceneInMemory();

			mAssociatedScene->SetComponentState(ComponentState::Running);
			SetSystemsPauseState(false);

			mAssociatedScene->GetAnimationScene()->SetPaused(false);

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

			mAssociatedScene->GetAnimationScene()->SetPaused(true);

			if(oldState == PlayInEditorState::Stopped)
				SaveSceneInMemory();

			mAssociatedScene->SetComponentState(ComponentState::Paused);

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
	mSavedScene = mAssociatedScene->GetRoot()->Clone(mSavedSceneGameObjectCollection, true);
	mSavedSceneInstanceData = SceneUtility::RecordSceneObjectHierarchyInstanceData(mAssociatedScene->GetRoot());
	mSavedSceneResourceId = mAssociatedScene->GetAssociatedResourceId();

	// Remove objects with "dont save" flag
	Stack<HSceneObject> todo;
	todo.push(mSavedScene);

	while(!todo.empty())
	{
		HSceneObject current = todo.top();
		todo.pop();

		if(current->HasFlag(SceneObjectFlag::DontSave) || current->HasFlag(SceneObjectFlag::RuntimePersistent))
			current->Destroy();
		else
		{
			const u32 childCount = current->GetChildCount();
			for(u32 i = 0; i < childCount; i++)
				todo.push(current->GetChild(i));
		}
	}

	mSavedSceneGameObjectCollection->DestroyQueuedObjects();
}

void PlayInEditor::SetSystemsPauseState(bool paused)
{
	mAssociatedScene->GetTime().SetPaused(paused);
	mAssociatedScene->GetPhysicsScene()->SetPaused(paused);
	GetAudio().SetPaused(paused);
}
