//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsSceneManager.h"

#include "BsGameObjectCollection.h"
#include "Scene/BsSceneObject.h"
#include "Renderer/BsRenderable.h"
#include "Renderer/BsCamera.h"
#include "RenderAPI/BsViewport.h"
#include "RenderAPI/BsRenderTarget.h"
#include "Scene/BsSceneInstance.h"

using namespace b3d;

SceneManager::~SceneManager()
{
	mMainScene->mPhysicsScene = nullptr;

	if(mMainScene->mRoot != nullptr && !mMainScene->mRoot.IsDestroyed())
		mMainScene->mRoot->Destroy(true);

	// Clear strong references to SceneInstance before destructor exits, as their destructors may call into the SceneManager, so we must sure
	// data remains valid
	mMainScene = nullptr;
}

void SceneManager::OnStartUp()
{
	SetMainScene(nullptr); // Forces creation of an empty main scene
}

void SceneManager::SetMainScene(const SPtr<SceneInstance>& scene)
{
	if(scene == nullptr)
	{
		mMainScene = SceneInstance::Create("Main");
		mMainScene->mRoot->SetScene(mMainScene);
	}
	else
		mMainScene = scene;
}

void SceneManager::UpdateLinkedSceneActorTransforms()
{
	for(auto& entry : mSceneInstances)
	{
		const SPtr<SceneInstance>& scene = entry.second.lock();
		scene->UpdateLinkedSceneActorTransforms();
	}
}

void SceneManager::SetMainCameraRenderTarget(const SPtr<RenderTarget>& renderTarget)
{
	for(auto& entry : mSceneInstances)
	{
		const SPtr<SceneInstance>& scene = entry.second.lock();
		scene->SetMainCameraRenderTarget(renderTarget);
	}
}

void SceneManager::NotifySceneInstanceCreated(const SPtr<SceneInstance>& sceneInstance)
{
	if(!B3D_ENSURE(sceneInstance != nullptr))
		return;

	mSceneInstances[sceneInstance.get()] = sceneInstance;
}

void SceneManager::NotifySceneInstanceDestroyed(SceneInstance* sceneInstance)
{
	auto found = mSceneInstances.find(sceneInstance);
	if(B3D_ENSURE(found != mSceneInstances.end()))
		mSceneInstances.erase(found);
}

namespace b3d
{
SceneManager& GetSceneManager()
{
	return SceneManager::Instance();
}
} // namespace b3d
