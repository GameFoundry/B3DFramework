//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DSceneUtility.h"
#include "B3DGameObjectCollection.h"
#include "B3DSceneManager.h"
#include "Scene/B3DSceneObject.h"
#include "Resources/B3DResources.h"

using namespace b3d;

UnorderedMap<UUID, TShared<GameObjectInstanceData>> SceneUtility::RecordSceneObjectHierarchyInstanceData(const HSceneObject& root)
{
	UnorderedMap<UUID, TShared<GameObjectInstanceData>> outInstanceData;
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

void SceneUtility::RestoreSceneObjectHierarchyInstanceData(const HSceneObject& root, const UnorderedMap<UUID, TShared<GameObjectInstanceData>>& instanceData)
{
	TShared<GameObjectCollection> gameObjectCollection = root->GetOwnerCollection().lock();
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
