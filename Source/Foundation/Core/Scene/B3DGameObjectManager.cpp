//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/B3DGameObjectManager.h"
#include "B3DGameObjectCollection.h"

using namespace b3d;

GameObjectManager::~GameObjectManager()
{
	DestroyQueuedObjects();
}

void GameObjectManager::DestroyQueuedObjects()
{
	for(const auto& entry : mGameObjectCollections)
	{
		const SPtr<GameObjectCollection>& collection = entry.second.lock();
		if(!B3D_ENSURE(collection != nullptr))
			continue;

		collection->DestroyQueuedObjects();
	}
}

void GameObjectManager::RegisterGameObjectCollection(const SPtr<GameObjectCollection>& collection)
{
	if(!B3D_ENSURE(collection != nullptr))
		return;

	const UUID& id = collection->GetId();

	auto result = mGameObjectCollections.insert(std::make_pair(id, collection));
	B3D_ENSURE(result.second);
}

void GameObjectManager::UnregisterGameObjectCollection(const GameObjectCollection& collection)
{
	const UUID& id = collection.GetId();
	mGameObjectCollections.erase(id);
}
