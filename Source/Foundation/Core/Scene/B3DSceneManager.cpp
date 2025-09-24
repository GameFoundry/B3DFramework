//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/B3DSceneManager.h"

#include "B3DGameObjectCollection.h"
#include "Scene/B3DSceneObject.h"
#include "Components/B3DCamera.h"
#include "RenderAPI/B3DViewport.h"
#include "RenderAPI/B3DRenderTarget.h"
#include "Scene/B3DSceneInstance.h"

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
