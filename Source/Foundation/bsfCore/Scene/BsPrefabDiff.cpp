//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsPrefabDiff.h"
#include "Private/RTTI/BsPrefabDiffRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Serialization/BsBinarySerializer.h"
#include "Serialization/BsBinaryDiff.h"
#include "Scene/BsSceneManager.h"
#include "Utility/BsUtility.h"

namespace bs
{
	RTTITypeBase* PrefabComponentDiff::GetRttiStatic()
	{
		return PrefabComponentDiffRTTI::Instance();
	}

	RTTITypeBase* PrefabComponentDiff::GetRtti() const
	{
		return PrefabComponentDiff::GetRttiStatic();
	}

	RTTITypeBase* PrefabObjectDiff::GetRttiStatic()
	{
		return PrefabObjectDiffRTTI::Instance();
	}

	RTTITypeBase* PrefabObjectDiff::GetRtti() const
	{
		return PrefabObjectDiff::GetRttiStatic();
	}

	SPtr<PrefabDiff> PrefabDiff::Create(const HSceneObject& prefab, const HSceneObject& instance)
	{
		if (prefab->mPrefabLinkUUID != instance->mPrefabLinkUUID)
			return nullptr;

		// Note: If this method is called multiple times in a row then renaming all objects every time is redundant, it
		// would be more efficient to do it once outside of this method. I'm keeping it this way for simplicity for now.

		// Rename instance objects so they share the same IDs as the prefab objects (if they link IDs match). This allows
		// game object handle diff to work properly, because otherwise handles that point to same objects would be
		// marked as different because the instance IDs of the two objects don't match (since one is in prefab and one
		// in instance).
		Vector<RenamedGameObject> renamedObjects;
		RenameInstanceIds(prefab, instance, renamedObjects);

		SPtr<PrefabDiff> output = bs_shared_ptr_new<PrefabDiff>();
		output->mRoot = GenerateDiff(prefab, instance);

		RestoreInstanceIds(renamedObjects);

		return output;
	}

	void PrefabDiff::Apply(const HSceneObject& object)
	{
		if (mRoot == nullptr)
			return;

		CoreSerializationContext serzContext;
		serzContext.goState = bs_shared_ptr_new<GameObjectDeserializationState>(GODM_UseNewIds | GODM_RestoreExternal);
		serzContext.goDeserializationActive = true;

		ApplyDiff(mRoot, object, &serzContext);

		serzContext.goState->Resolve();
	}

	void PrefabDiff::ApplyDiff(const SPtr<PrefabObjectDiff>& diff, const HSceneObject& object,
		SerializationContext* context)
	{
		if ((diff->soFlags & (UINT32)SceneObjectDiffFlags::Name) != 0)
			object->SetName(diff->name);

		if ((diff->soFlags & (UINT32)SceneObjectDiffFlags::Position) != 0)
			object->SetPosition(diff->position);

		if ((diff->soFlags & (UINT32)SceneObjectDiffFlags::Rotation) != 0)
			object->SetRotation(diff->rotation);

		if ((diff->soFlags & (UINT32)SceneObjectDiffFlags::Scale) != 0)
			object->SetScale(diff->scale);

		if ((diff->soFlags & (UINT32)SceneObjectDiffFlags::Active) != 0)
			object->SetActive(diff->isActive);

		// Note: It is important to remove objects and components first, before adding them.
		//		 Some systems rely on the fact that applyDiff added components/objects are
		//       always at the end.
		const Vector<HComponent>& components = object->GetComponents();
		for (auto& removedId : diff->removedComponents)
		{
			for (auto component : components)
			{
				if (removedId == component->GetLinkId())
				{
					component->Destroy(true);
					break;
				}
			}
		}

		for (auto& removedId : diff->removedChildren)
		{
			UINT32 childCount = object->GetNumChildren();
			for (UINT32 i = 0; i < childCount; i++)
			{
				HSceneObject child = object->GetChild(i);
				if (removedId == child->GetLinkId())
				{
					child->Destroy(true);
					break;
				}
			}
		}

		for (auto& addedComponentData : diff->addedComponents)
		{
			SPtr<Component> component = std::static_pointer_cast<Component>(addedComponentData->Decode(context));

			object->AddAndInitializeComponent(component);
		}

		for (auto& addedChildData : diff->addedChildren)
		{
			SPtr<SceneObject> sceneObject = std::static_pointer_cast<SceneObject>(addedChildData->Decode(context));
			sceneObject->SetParent(object);

			if(object->IsInstantiated())
				sceneObject->InstantiateInternal();
		}

		for (auto& componentDiff : diff->componentDiffs)
		{
			for (auto& component : components)
			{
				if (componentDiff->id == (INT32)component->GetLinkId())
				{
					IDiff& diffHandler = component->GetRtti()->GetDiffHandler();
					diffHandler.ApplyDiff(component.GetInternalPtr(), componentDiff->data, context);
					break;
				}
			}
		}

		for (auto& childDiff : diff->childDiffs)
		{
			UINT32 childCount = object->GetNumChildren();
			for (UINT32 i = 0; i < childCount; i++)
			{
				HSceneObject child = object->GetChild(i);
				if (childDiff->id == child->GetLinkId())
				{
					ApplyDiff(childDiff, child, context);
					break;
				}
			}
		}
	}

	SPtr<PrefabObjectDiff> PrefabDiff::GenerateDiff(const HSceneObject& prefab, const HSceneObject& instance)
	{
		SPtr<PrefabObjectDiff> output;

		if (prefab->GetName() != instance->GetName())
		{
			if (output == nullptr)
				output = bs_shared_ptr_new<PrefabObjectDiff>();

			output->name = instance->GetName();
			output->soFlags |= (UINT32)SceneObjectDiffFlags::Name;
		}

		const Transform& prefabTfrm = prefab->GetLocalTransform();
		const Transform& instanceTfrm = instance->GetLocalTransform();
		if (prefabTfrm.GetPosition() != instanceTfrm.GetPosition())
		{
			if (output == nullptr)
				output = bs_shared_ptr_new<PrefabObjectDiff>();

			output->position = instanceTfrm.GetPosition();
			output->soFlags |= (UINT32)SceneObjectDiffFlags::Position;
		}

		if (prefabTfrm.GetRotation() != instanceTfrm.GetRotation())
		{
			if (output == nullptr)
				output = bs_shared_ptr_new<PrefabObjectDiff>();

			output->rotation = instanceTfrm.GetRotation();
			output->soFlags |= (UINT32)SceneObjectDiffFlags::Rotation;
		}

		if (prefabTfrm.GetScale() != instanceTfrm.GetScale())
		{
			if (output == nullptr)
				output = bs_shared_ptr_new<PrefabObjectDiff>();

			output->scale = instanceTfrm.GetScale();
			output->soFlags |= (UINT32)SceneObjectDiffFlags::Scale;
		}

		if (prefab->GetActive() != instance->GetActive())
		{
			if (output == nullptr)
				output = bs_shared_ptr_new<PrefabObjectDiff>();

			output->isActive = instance->GetActive();
			output->soFlags |= (UINT32)SceneObjectDiffFlags::Active;
		}

		UINT32 prefabChildCount = prefab->GetNumChildren();
		UINT32 instanceChildCount = instance->GetNumChildren();

		// Find modified and removed children
		for (UINT32 i = 0; i < prefabChildCount; i++)
		{
			HSceneObject prefabChild = prefab->GetChild(i);

			SPtr<PrefabObjectDiff> childDiff;
			bool foundMatching = false;
			for (UINT32 j = 0; j < instanceChildCount; j++)
			{
				HSceneObject instanceChild = instance->GetChild(j);

				if (prefabChild->GetLinkId() == instanceChild->GetLinkId())
				{
					if (instanceChild->mPrefabLinkUUID.Empty())
						childDiff = GenerateDiff(prefabChild, instanceChild);

					foundMatching = true;
					break;
				}
			}

			if (foundMatching)
			{
				if (childDiff != nullptr)
				{
					if (output == nullptr)
						output = bs_shared_ptr_new<PrefabObjectDiff>();

					output->childDiffs.push_back(childDiff);
				}
			}
			else
			{
				if (output == nullptr)
					output = bs_shared_ptr_new<PrefabObjectDiff>();

				output->removedChildren.push_back(prefabChild->GetLinkId());
			}	
		}

		// Find added children
		for (UINT32 i = 0; i < instanceChildCount; i++)
		{
			HSceneObject instanceChild = instance->GetChild(i);

			if (instanceChild->HasFlag(SOF_DontSave))
				continue;

			bool foundMatching = false;
			if (instanceChild->GetLinkId() != (UINT32)-1)
			{
				for (UINT32 j = 0; j < prefabChildCount; j++)
				{
					HSceneObject prefabChild = prefab->GetChild(j);

					if (prefabChild->GetLinkId() == instanceChild->GetLinkId())
					{
						foundMatching = true;
						break;
					}
				}
			}

			if (!foundMatching)
			{
				SPtr<SerializedObject> obj = SerializedObject::Create(*instanceChild);

				if (output == nullptr)
					output = bs_shared_ptr_new<PrefabObjectDiff>();

				output->addedChildren.push_back(obj);
			}
		}

		const Vector<HComponent>& prefabComponents = prefab->GetComponents();
		const Vector<HComponent>& instanceComponents = instance->GetComponents();

		UINT32 prefabComponentCount = (UINT32)prefabComponents.size();
		UINT32 instanceComponentCount = (UINT32)instanceComponents.size();

		// Find modified and removed components
		for (UINT32 i = 0; i < prefabComponentCount; i++)
		{
			HComponent prefabComponent = prefabComponents[i];

			SPtr<PrefabComponentDiff> childDiff;
			bool foundMatching = false;
			for (UINT32 j = 0; j < instanceComponentCount; j++)
			{
				HComponent instanceComponent = instanceComponents[j];

				if (prefabComponent->GetLinkId() == instanceComponent->GetLinkId())
				{
					SPtr<SerializedObject> encodedPrefab = SerializedObject::Create(*prefabComponent);
					SPtr<SerializedObject> encodedInstance = SerializedObject::Create(*instanceComponent);

					IDiff& diffHandler = prefabComponent->GetRtti()->GetDiffHandler();
					SPtr<SerializedObject> diff = diffHandler.GenerateDiff(encodedPrefab, encodedInstance);

					if (diff != nullptr)
					{
						childDiff = bs_shared_ptr_new<PrefabComponentDiff>();
						childDiff->id = prefabComponent->GetLinkId();
						childDiff->data = diff;
					}

					foundMatching = true;
					break;
				}
			}

			if (foundMatching)
			{
				if (childDiff != nullptr)
				{
					if (output == nullptr)
						output = bs_shared_ptr_new<PrefabObjectDiff>();

					output->componentDiffs.push_back(childDiff);
				}
			}
			else
			{
				if (output == nullptr)
					output = bs_shared_ptr_new<PrefabObjectDiff>();

				output->removedComponents.push_back(prefabComponent->GetLinkId());
			}
		}

		// Find added components
		for (UINT32 i = 0; i < instanceComponentCount; i++)
		{
			HComponent instanceComponent = instanceComponents[i];

			bool foundMatching = false;
			if (instanceComponent->GetLinkId() != (UINT32)-1)
			{
				for (UINT32 j = 0; j < prefabComponentCount; j++)
				{
					HComponent prefabComponent = prefabComponents[j];

					if (prefabComponent->GetLinkId() == instanceComponent->GetLinkId())
					{
						foundMatching = true;
						break;
					}
				}
			}

			if (!foundMatching)
			{
				SPtr<SerializedObject> obj = SerializedObject::Create(*instanceComponent);

				if (output == nullptr)
					output = bs_shared_ptr_new<PrefabObjectDiff>();

				output->addedComponents.push_back(obj);
			}
		}

		if (output != nullptr)
			output->id = instance->GetLinkId();

		return output;
	}

	void PrefabDiff::RenameInstanceIds(const HSceneObject& prefab, const HSceneObject& instance, Vector<RenamedGameObject>& output)
	{
		UnorderedMap<UUID, UnorderedMap<UINT32, UINT64>> linkToInstanceId;

		struct StackEntry
		{
			HSceneObject so;
			UUID uuid;
		};

		// When renaming it is important to rename the prefab and not the instance, since the diff will otherwise
		// contain prefab's IDs, but will be used for the instance.

		Stack<StackEntry> todo;
		todo.push({ instance, UUID::EMPTY });

		while (!todo.empty())
		{
			StackEntry current = todo.top();
			todo.pop();

			UUID childParentUUID;
			if (current.so->mPrefabLinkUUID.Empty())
				childParentUUID = current.uuid;
			else
				childParentUUID = current.so->mPrefabLinkUUID;

			UnorderedMap<UINT32, UINT64>& idMap = linkToInstanceId[childParentUUID];

			const Vector<HComponent>& components = current.so->GetComponents();
			for (auto& component : components)
			{
				if (component->GetLinkId() != (UINT32)-1)
					idMap[component->GetLinkId()] = component->GetInstanceId();
			}

			UINT32 numChildren = current.so->GetNumChildren();
			for (UINT32 i = 0; i < numChildren; i++)
			{
				HSceneObject child = current.so->GetChild(i);

				if (child->GetLinkId() != (UINT32)-1)
					idMap[child->GetLinkId()] = child->GetInstanceId();

				todo.push({ child, childParentUUID });
			}
		}

		// Root has link ID from its parent so we handle it separately
		{
			output.push_back(RenamedGameObject());
			RenamedGameObject& renamedGO = output.back();
			renamedGO.instanceData = instance->mInstanceData;
			renamedGO.originalId = instance->GetInstanceId();

			prefab->mInstanceData->mInstanceId = instance->GetInstanceId();
		}

		todo.push({ prefab, UUID::EMPTY });
		while (!todo.empty())
		{
			StackEntry current = todo.top();
			todo.pop();

			UUID childParentUUID;
			if (current.so->mPrefabLinkUUID.Empty())
				childParentUUID = current.uuid;
			else
				childParentUUID = current.so->mPrefabLinkUUID;

			auto iterFind = linkToInstanceId.find(childParentUUID);
			if (iterFind != linkToInstanceId.end())
			{
				UnorderedMap<UINT32, UINT64>& idMap = iterFind->second;

				const Vector<HComponent>& components = current.so->GetComponents();
				for (auto& component : components)
				{
					auto iterFind2 = idMap.find(component->GetLinkId());
					if (iterFind2 != idMap.end())
					{
						output.push_back(RenamedGameObject());
						RenamedGameObject& renamedGO = output.back();
						renamedGO.instanceData = component->mInstanceData;
						renamedGO.originalId = component->GetInstanceId();

						component->mInstanceData->mInstanceId = iterFind2->second;
					}
				}
			}

			UINT32 numChildren = current.so->GetNumChildren();
			for (UINT32 i = 0; i < numChildren; i++)
			{
				HSceneObject child = current.so->GetChild(i);

				if (iterFind != linkToInstanceId.end())
				{
					if (child->GetLinkId() != (UINT32)-1)
					{
						UnorderedMap<UINT32, UINT64>& idMap = iterFind->second;

						auto iterFind2 = idMap.find(child->GetLinkId());
						if (iterFind2 != idMap.end())
						{
							output.push_back(RenamedGameObject());
							RenamedGameObject& renamedGO = output.back();
							renamedGO.instanceData = child->mInstanceData;
							renamedGO.originalId = child->GetInstanceId();

							child->mInstanceData->mInstanceId = iterFind2->second;
						}
					}
				}

				todo.push({ child, childParentUUID });
			}
		}
	}

	void PrefabDiff::RestoreInstanceIds(const Vector<RenamedGameObject>& renamedObjects)
	{
		for (auto& renamedGO : renamedObjects)
			renamedGO.instanceData->mInstanceId = renamedGO.originalId;
	}

	RTTITypeBase* PrefabDiff::GetRttiStatic()
	{
		return PrefabDiffRTTI::Instance();
	}

	RTTITypeBase* PrefabDiff::GetRtti() const
	{
		return PrefabDiff::GetRttiStatic();
	}
}
