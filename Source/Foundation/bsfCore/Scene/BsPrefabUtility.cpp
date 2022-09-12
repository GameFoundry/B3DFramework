//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsPrefabUtility.h"
#include "Scene/BsPrefabDiff.h"
#include "Scene/BsPrefab.h"
#include "Scene/BsSceneObject.h"
#include "Resources/BsResources.h"

namespace bs
{
	namespace impl
	{
		/**	Contains saved Component instance data. */
		struct ComponentProxy
		{
			GameObjectInstanceDataPtr instanceData;
			UINT32 linkId;
			UUID uuid;
		};

		/** Contains saved SceneObject instance data, as well as saved instance data for all its children and components. */
		struct SceneObjectProxy
		{
			GameObjectInstanceDataPtr instanceData;
			UINT32 linkId;
			UUID uuid;

			Vector<ComponentProxy> components;
			Vector<SceneObjectProxy> children;
		};

		/** Contains linked game-object instance data and UUID. */
		struct LinkedInstanceData
		{
			GameObjectInstanceDataPtr instanceData;
			UUID uuid;
		};

		/**
	     * Traverses the object hierarchy, finds all child objects and components and records their instance data, as well
		 * as their original place in the hierarchy. Instance data essentially holds the object's "identity" and by
		 * restoring it we ensure any handles pointing to the object earlier will still point to the new version.
		 *
		 * @param[in]	so					Object to traverse and record.
		 * @param[out]	output				Contains the output hierarchy of instance data.
		 * @param[out]	linkedInstanceData	A map of link IDs to instance data. Objects without link IDs will not be
		 *									included here.
		 *
		 * @note	Does not recurse into child prefab instances.
		 */
		void recordInstanceData(const HSceneObject& so, SceneObjectProxy& output,
			UnorderedMap<UINT32, LinkedInstanceData>& linkedInstanceData)
		{
			struct StackData
			{
				HSceneObject so;
				SceneObjectProxy* proxy;
			};

			Stack<StackData> todo;
			todo.push({so, &output});

			output.instanceData = so->_getInstanceData();
			output.uuid = so->getUUID();
			output.linkId = (UINT32)-1;

			while (!todo.empty())
			{
				StackData curData = todo.top();
				todo.pop();

				const Vector<HComponent>& components = curData.so->getComponents();
				for (auto& component : components)
				{
					curData.proxy->components.push_back(ComponentProxy());

					ComponentProxy& componentProxy = curData.proxy->components.back();
					componentProxy.instanceData = component->_getInstanceData();
					componentProxy.uuid = component->getUUID();
					componentProxy.linkId = component->getLinkId();

					linkedInstanceData[componentProxy.linkId] = { componentProxy.instanceData, componentProxy.uuid };
				}

				UINT32 numChildren = curData.so->getNumChildren();
				curData.proxy->children.resize(numChildren);

				for (UINT32 i = 0; i < numChildren; i++)
				{
					HSceneObject child = curData.so->getChild(i);

					SceneObjectProxy& childProxy = curData.proxy->children[i];

					childProxy.instanceData = child->_getInstanceData();
					childProxy.uuid = child->getUUID();
					childProxy.linkId = child->getLinkId();

					linkedInstanceData[childProxy.linkId] = { childProxy.instanceData, childProxy.uuid };

					if (child->_getPrefabLinkUUID().empty())
						todo.push({ child, &curData.proxy->children[i] });
				}
			}
		}

		/**
		 * Restores instance data in the provided hierarchy, using link ids to determine what data maps to which objects.
		 *
		 * @param[in]	so					Object to traverse and restore the instance data.
		 * @param[in]	proxy				Hierarchy containing instance data for all objects and components, returned by
		 *									recordInstanceData() method.				
		 * @param[in]	linkedInstanceData	A map of link IDs to instance data, returned by recordInstanceData() method.
		 *
		 * @note	Does not recurse into child prefab instances.
		 */
		void restoreLinkedInstanceData(const HSceneObject& so, SceneObjectProxy& proxy,
			UnorderedMap<UINT32, LinkedInstanceData>& linkedInstanceData)
		{
			Stack<HSceneObject> todo;
			todo.push(so);

			while (!todo.empty())
			{
				HSceneObject current = todo.top();
				todo.pop();

				Vector<HComponent>& components = current->_getComponents();
				for (auto& component : components)
				{
					if (component->getLinkId() != (UINT32)-1)
					{
						auto iterFind = linkedInstanceData.find(component->getLinkId());
						if (iterFind != linkedInstanceData.end())
						{
							component->_setInstanceData(iterFind->second.instanceData);
							component->_setUUID(iterFind->second.uuid);
							component._setHandleData(component.getInternalPtr());
						}
					}
				}

				UINT32 numChildren = current->getNumChildren();
				for (UINT32 i = 0; i < numChildren; i++)
				{
					HSceneObject child = current->getChild(i);

					if (child->getLinkId() != (UINT32)-1)
					{
						auto iterFind = linkedInstanceData.find(child->getLinkId());
						if (iterFind != linkedInstanceData.end())
						{
							child->_setInstanceData(iterFind->second.instanceData);
							child->_setUUID(iterFind->second.uuid);
						}
					}

					if (child->_getPrefabLinkUUID().empty())
						todo.push(child);
				}
			}
		}

		/**
		 * Restores instance data in the provided hierarchy, but only for objects without a link id. Since the objects do
		 * not have a link ID we rely on their sequential order to find out which instance data belongs to which object.
		 *
		 * @param[in]	so		Object to traverse and restore the instance data.
		 * @param[in]	proxy	Hierarchy containing instance data for all objects and components, returned by
		 *						recordInstanceData() method.
		 *
		 * @note	Does not recurse into child prefab instances.
		 */
		void RestoreUnlinkedInstanceData(const HSceneObject& so, SceneObjectProxy& proxy)
		{
			struct StackEntry
			{
				HSceneObject so;
				SceneObjectProxy* proxy;
			};

			Stack<StackEntry> todo;
			todo.push(StackEntry());

			StackEntry& topEntry = todo.top();
			topEntry.so = so;
			topEntry.proxy = &proxy;

			while (!todo.empty())
			{
				StackEntry current = todo.top();
				todo.pop();

				if (current.proxy->linkId == (UINT32)-1)
				{
					current.so->_setInstanceData(current.proxy->instanceData);
					current.so->_setUUID(current.proxy->uuid);
				}

				Vector<HComponent>& components = current.so->_getComponents();
				UINT32 componentProxyIdx = 0;
				UINT32 numComponentProxies = (UINT32)current.proxy->components.size();
				for (auto& component : components)
				{
					if (component->getLinkId() == (UINT32)-1)
					{
						bool foundInstanceData = false;
						(void)foundInstanceData;
						for (; componentProxyIdx < numComponentProxies; componentProxyIdx++)
						{
							if (current.proxy->components[componentProxyIdx].linkId != (UINT32)-1)
								continue;

							component->_setInstanceData(current.proxy->components[componentProxyIdx].instanceData);
							component->_setUUID(current.proxy->components[componentProxyIdx].uuid);
							component._setHandleData(component.getInternalPtr());

							foundInstanceData = true;
							break;
						}

						assert(foundInstanceData);
					}
				}

				UINT32 numChildren = current.so->getNumChildren();
				UINT32 childProxyIdx = 0;
				UINT32 numChildProxies = (UINT32)current.proxy->children.size();
				for (UINT32 i = 0; i < numChildren; i++)
				{
					HSceneObject child = current.so->getChild(i);

					if (child->getLinkId() == (UINT32)-1)
					{
						bool foundInstanceData = false;
						(void)foundInstanceData;
						for (; childProxyIdx < numChildProxies; childProxyIdx++)
						{
							if (current.proxy->children[childProxyIdx].linkId != (UINT32)-1)
								continue;

							assert(current.proxy->children[childProxyIdx].linkId == (UINT32)-1);
							child->_setInstanceData(current.proxy->children[childProxyIdx].instanceData);
							child->_setUUID(current.proxy->children[childProxyIdx].uuid);

							if (child->_getPrefabLinkUUID().empty())
							{
								todo.push(StackEntry());

								StackEntry& newEntry = todo.top();
								newEntry.so = child;
								newEntry.proxy = &current.proxy->children[childProxyIdx];
							}

							foundInstanceData = true;
							break;
						}

						assert(foundInstanceData);
					}
					else
					{
						if (!child->_getPrefabLinkUUID().empty())
							continue;

						for (UINT32 j = 0; j < numChildProxies; j++)
						{
							if (child->getLinkId() == current.proxy->children[j].linkId)
							{
								todo.push(StackEntry());

								StackEntry& newEntry = todo.top();
								newEntry.so = child;
								newEntry.proxy = &current.proxy->children[j];
								break;
							}
						}
					}
				}
			}
		}
	}

	void PrefabUtility::RevertToPrefab(const HSceneObject& so)
	{
		UUID prefabLinkUUID = so->getPrefabLink();
		HPrefab prefabLink = static_resource_cast<Prefab>(gResources().loadFromUUID(prefabLinkUUID, false, ResourceLoadFlag::None));

		if (!prefabLink.isLoaded(false))
			return;

		// Save IDs, destroy original, create new, restore IDs
		impl::SceneObjectProxy soProxy;
		UnorderedMap<UINT32, impl::LinkedInstanceData> linkedInstanceData;
		impl::recordInstanceData(so, soProxy, linkedInstanceData);

		HSceneObject parent = so->getParent();

		// This will destroy the object but keep it in the parent's child list
		HSceneObject currentSO = so;
		so->destroyInternal(currentSO, true);

		HSceneObject newInstance = prefabLink->instantiate();

		// Remove default parent, and replace with original one
		newInstance->mParent->removeChild(newInstance);
		newInstance->mParent = parent;

		impl::restoreLinkedInstanceData(newInstance, soProxy, linkedInstanceData);
	}

	void PrefabUtility::UpdateFromPrefab(const HSceneObject& so)
	{
		HSceneObject topLevelObject = so;

		while (topLevelObject != nullptr)
		{
			if (!topLevelObject->mPrefabLinkUUID.empty())
				break;

			if (topLevelObject->mParent != nullptr)
				topLevelObject = topLevelObject->mParent;
			else
				topLevelObject = nullptr;
		}

		if (topLevelObject == nullptr)
			topLevelObject = so;

		Stack<HSceneObject> todo;
		todo.push(topLevelObject);

		// Find any prefab instances
		Vector<HSceneObject> prefabInstanceRoots;

		while (!todo.empty())
		{
			HSceneObject current = todo.top();
			todo.pop();

			if (!current->mPrefabLinkUUID.empty())
				prefabInstanceRoots.push_back(current);

			UINT32 childCount = current->getNumChildren();
			for (UINT32 i = 0; i < childCount; i++)
			{
				HSceneObject child = current->getChild(i);
				todo.push(child);
			}
		}

		// Stores data about the new prefab instance and its original parent and link id
		// (as those aren't stored in the prefab diff)
		struct RestoredPrefabInstance
		{
			HSceneObject newInstance;
			HSceneObject originalParent;
			SPtr<PrefabDiff> diff;
			UINT32 originalLinkId;
		};

		Vector<RestoredPrefabInstance> newPrefabInstanceData;

		// For each prefab instance load its reference prefab from the disk and check if it changed. If it has changed
		// instantiate the prefab and destroy the current instance. Then apply instance specific changes stored in a
		// prefab diff, if any, as well as restore the original parent and link id (link id of the root prefab instance
		// belongs to the parent prefab if any). Finally fix any handles pointing to the old objects so that they now point
		// to the newly instantiated objects. To the outside world it should be transparent that we just destroyed and then
		// re-created from scratch the entire hierarchy.

		// Need to do this bottom up to ensure I don't destroy the parents before children
		for (auto iter = prefabInstanceRoots.rbegin(); iter != prefabInstanceRoots.rend(); ++iter)
		{
			HSceneObject current = *iter;
			HPrefab prefabLink = static_resource_cast<Prefab>(gResources().loadFromUUID(current->mPrefabLinkUUID, false, ResourceLoadFlag::None));

			if (prefabLink.isLoaded(false) && prefabLink->getHash() != current->mPrefabHash)
			{
				// Save IDs, destroy original, create new, restore IDs
				impl::SceneObjectProxy soProxy;
				UnorderedMap<UINT32, impl::LinkedInstanceData> linkedInstanceData;
				impl::recordInstanceData(current, soProxy, linkedInstanceData);

				HSceneObject parent = current->getParent();
				SPtr<PrefabDiff> prefabDiff = current->mPrefabDiff;

				current->destroy(true);
				HSceneObject newInstance = prefabLink->_clone();

				// When restoring instance IDs it is important to make all the new handles point to the old GameObjectInstanceData.
				// This is because old handles will have different GameObjectHandleData and we have no easy way of accessing it to
				// change to which GameObjectInstanceData it points. But the GameObjectManager ensures that all handles deserialized
				// at once (i.e. during the ::_clone() call above) will share GameObjectHandleData so we can simply replace
				// to what they point to, affecting all of the handles to that object. (In another words, we can modify the
				// new handles at this point, but old ones must keep referencing what they already were.)
				impl::restoreLinkedInstanceData(newInstance, soProxy, linkedInstanceData);
				impl::restoreUnlinkedInstanceData(newInstance, soProxy);

				newPrefabInstanceData.push_back({ newInstance, parent, prefabDiff, newInstance->getLinkId() });
			}
		}

		// Once everything is cloned, apply diffs, restore old parents & link IDs for root.
		for (auto& entry : newPrefabInstanceData)
		{
			// Diffs must be applied after everything is instantiated and instance data restored since it may contain
			// game object handles within or external to its prefab instance.
			if (entry.diff != nullptr)
				entry.diff->apply(entry.newInstance);

			entry.newInstance->mPrefabDiff = entry.diff;

			entry.newInstance->setParent(entry.originalParent, false);
			entry.newInstance->mLinkId = entry.originalLinkId;
		}

		// Finally, instantiate everything if the top scene object is live (instantiated)
		if (topLevelObject->isInstantiated())
		{
			for (auto& entry : newPrefabInstanceData)
				entry.newInstance->_instantiate(true);
		}

		gResources().unloadAllUnused();
	}

	void PrefabUtility::GeneratePrefabIds(const HSceneObject& sceneObject)
	{
		UINT32 startingId = 0;

		Stack<HSceneObject> todo;
		todo.push(sceneObject);

		while (!todo.empty())
		{
			HSceneObject currentSO = todo.top();
			todo.pop();

			for (auto& component : currentSO->mComponents)
			{
				if (component->getLinkId() != (UINT32)-1)
					startingId = std::max(component->mLinkId + 1, startingId);
			}

			UINT32 numChildren = (UINT32)currentSO->getNumChildren();
			for (UINT32 i = 0; i < numChildren; i++)
			{
				HSceneObject child = currentSO->getChild(i);

				if (!child->hasFlag(SOF_DontSave))
				{
					if (child->getLinkId() != (UINT32)-1)
						startingId = std::max(child->mLinkId + 1, startingId);

					if (child->mPrefabLinkUUID.empty())
						todo.push(currentSO->getChild(i));
				}
			}
		}

		UINT32 currentId = startingId;
		todo.push(sceneObject);

		while (!todo.empty())
		{
			HSceneObject currentSO = todo.top();
			todo.pop();

			for (auto& component : currentSO->mComponents)
			{
				if (component->getLinkId() == (UINT32)-1)
					component->mLinkId = currentId++;
			}

			UINT32 numChildren = (UINT32)currentSO->getNumChildren();
			for (UINT32 i = 0; i < numChildren; i++)
			{
				HSceneObject child = currentSO->getChild(i);

				if (!child->hasFlag(SOF_DontSave))
				{
					if (child->getLinkId() == (UINT32)-1)
						child->mLinkId = currentId++;

					if(child->mPrefabLinkUUID.empty())
						todo.push(currentSO->getChild(i));
				}
			}
		}

		if (currentId < startingId)
		{
			BS_EXCEPT(InternalErrorException, "Prefab ran out of IDs to assign. " \
				"Consider increasing the size of the prefab ID data type.");
		}
	}

	void PrefabUtility::ClearPrefabIds(const HSceneObject& sceneObject, bool recursive, bool clearRoot)
	{
		Stack<HSceneObject> todo;
		todo.push(sceneObject);

		if (clearRoot)
			sceneObject->mLinkId = (UINT32)-1;

		while (!todo.empty())
		{
			HSceneObject currentSO = todo.top();
			todo.pop();

			for (auto& component : currentSO->mComponents)
				component->mLinkId = (UINT32)-1;

			if (recursive)
			{
				UINT32 numChildren = (UINT32)currentSO->getNumChildren();
				for (UINT32 i = 0; i < numChildren; i++)
				{
					HSceneObject child = currentSO->getChild(i);
					child->mLinkId = (UINT32)-1;

					if (child->mPrefabLinkUUID.empty())
						todo.push(child);
				}
			}
		}
	}

	void PrefabUtility::RecordPrefabDiff(const HSceneObject& sceneObject)
	{
		HSceneObject topLevelObject = sceneObject;

		while (topLevelObject != nullptr)
		{
			if (!topLevelObject->mPrefabLinkUUID.empty())
				break;

			if (topLevelObject->mParent != nullptr)
				topLevelObject = topLevelObject->mParent;
			else
				topLevelObject = nullptr;
		}

		if (topLevelObject == nullptr)
			topLevelObject = sceneObject;

		Stack<HSceneObject> todo;
		todo.push(topLevelObject);

		while (!todo.empty())
		{
			HSceneObject current = todo.top();
			todo.pop();

			if (!current->mPrefabLinkUUID.empty())
			{
				current->mPrefabDiff = nullptr;

				HPrefab prefabLink = static_resource_cast<Prefab>(gResources().loadFromUUID(current->mPrefabLinkUUID, false, ResourceLoadFlag::None));
				if (prefabLink.isLoaded(false))
					current->mPrefabDiff = PrefabDiff::create(prefabLink->_getRoot(), current->getHandle());
			}

			UINT32 childCount = current->getNumChildren();
			for (UINT32 i = 0; i < childCount; i++)
			{
				HSceneObject child = current->getChild(i);
				todo.push(child);
			}
		}

		gResources().unloadAllUnused();
	}
}
