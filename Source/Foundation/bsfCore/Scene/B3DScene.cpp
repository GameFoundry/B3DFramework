//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsScene.h"
#include "Private/RTTI/BsSceneRTTI.h"
#include "Resources/BsResources.h"
#include "Scene/BsSceneObject.h"
#include "Scene/BsPrefabUtility.h"
#include "BsCoreApplication.h"
#include "BsGameObjectCollection.h"
#include "Scene/BsSceneInstance.h"

using namespace b3d;

Scene::Scene()
	: Resource(false), mGameObjectCollection(GameObjectCollection::Create())
{
}

Scene::~Scene()
{
	if(mRoot != nullptr)
		mRoot->Destroy(true);
}

HScene Scene::Create(const HSceneObject& root)
{
	SPtr<Scene> newScene = CreateEmpty();
	newScene->mUUID = UUIDGenerator::GenerateRandom(); // TODO - This should be done automatically on resource creation

	newScene->ReplaceInternalHierarchy(root);
	newScene->Initialize();

	return B3DStaticResourceCast<Scene>(GetResources().CreateResourceHandle(newScene, newScene->mUUID));
}

SPtr<Scene> Scene::CreateEmpty()
{
	SPtr<Scene> newScene = B3DMakeSharedFromExisting<Scene>(new(B3DAllocate<Scene>()) Scene());
	newScene->SetShared(newScene);

	return newScene;
}

void Scene::ReplaceInternalHierarchy(const HSceneObject& sceneObject)
{
	const SPtr<GameObjectCollection> newGameObjectCollection = GameObjectCollection::Create();
	HSceneObject newRoot = sceneObject->Clone(newGameObjectCollection, true);
	newRoot->mParent = nullptr;

	// Remove objects that should not be saved
	FrameScope frameScope;
	FrameVector<HSceneObject> sceneObjectsToDestroy;
	newRoot->IterateHierarchy([&sceneObjectsToDestroy](const HSceneObject& sceneObject) {
		if(sceneObject->HasFlag(SceneObjectFlag::DontSave) || sceneObject->HasFlag(SceneObjectFlag::RuntimePersistent))
		{
			sceneObjectsToDestroy.push_back(sceneObject);
			return false;
		}

		return true;
	}, nullptr);

	for(const auto& entry : sceneObjectsToDestroy)
		entry->Destroy();

	if(mRoot.IsValid())
		mRoot->Destroy(true);

	mRoot = newRoot;
	mGameObjectCollection = newGameObjectCollection;

	// TODO - Might need to record nested prefab instance deltas here
}

SPtr<SceneInstance> Scene::Instantiate() const
{
	SPtr<SceneInstance> sceneInstance;
	Instantiate(sceneInstance);

	return sceneInstance;
}

HSceneObject Scene::Instantiate(SPtr<SceneInstance>& inOutSceneInstance) const
{
	if(mRoot == nullptr)
		return HSceneObject();

	SPtr<GameObjectCollection> gameObjectCollection;
	if(inOutSceneInstance != nullptr)
		gameObjectCollection = inOutSceneInstance->GetGameObjectCollection();
	else
		gameObjectCollection = GameObjectCollection::Create();

	HSceneObject clone;
	if(mRoot == nullptr)
		clone = HSceneObject();
	else 
		clone = mRoot->Clone(gameObjectCollection);

	if(inOutSceneInstance != nullptr)
	{
		inOutSceneInstance->SetRoot(clone);
		inOutSceneInstance->SetAssociatedResourceId(GetId());
	}
	else
		inOutSceneInstance = SceneInstance::Create("SceneInstance", clone, GetId());

	clone->Initialize();
	return clone;
}

RTTIType* Scene::GetRttiStatic()
{
	return SceneRTTI::Instance();
}

RTTIType* Scene::GetRtti() const
{
	return Scene::GetRttiStatic();
}
