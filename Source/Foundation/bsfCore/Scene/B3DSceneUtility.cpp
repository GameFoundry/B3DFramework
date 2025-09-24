//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsSceneUtility.h"
#include "BsGameObjectCollection.h"
#include "BsSceneManager.h"
#include "Scene/BsSceneObject.h"
#include "Resources/BsResources.h"

using namespace b3d;

UnorderedMap<UUID, SPtr<GameObjectInstanceData>> SceneUtility::RecordSceneObjectHierarchyInstanceData(const HSceneObject& root)
{
	UnorderedMap<UUID, SPtr<GameObjectInstanceData>> outInstanceData;
	root->IterateHierarchy(
		[&outInstanceData](const HSceneObject& sceneObject)
		{
			outInstanceData[sceneObject->GetId()] = sceneObject->GetInstanceData();
			return true;
		},
		[&outInstanceData](const HComponent& component)
		{
			outInstanceData[component->GetId()] = component->GetInstanceData();
		});

	return outInstanceData;
}

void SceneUtility::RestoreSceneObjectHierarchyInstanceData(const HSceneObject& root, const UnorderedMap<UUID, SPtr<GameObjectInstanceData>>& instanceData)
{
	SPtr<GameObjectCollection> gameObjectCollection = root->GetOwnerCollection().lock();
	if(!B3D_ENSURE(gameObjectCollection))
		return;

	root->IterateHierarchy(
		[&instanceData, &gameObjectCollection](const HSceneObject& sceneObject)
		{
			const UUID& objectId = sceneObject->GetId();
			if(auto found = instanceData.find(objectId); found != instanceData.end())
			{
				HSceneObject sceneObjectMutableHandle = sceneObject;
				gameObjectCollection->ReplaceGameObjectInstance(sceneObjectMutableHandle, found->second);
			}

			return true;
		},
		[&instanceData, &gameObjectCollection](const HComponent& component)
		{
			const UUID& objectId = component->GetId();
			if(auto found = instanceData.find(objectId); found != instanceData.end())
			{
				HComponent componentMutableHandle = component;
				gameObjectCollection->ReplaceGameObjectInstance(componentMutableHandle, found->second);
			}
		});
}
