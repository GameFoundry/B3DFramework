//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsPrefabDiff.h"
#include "Private/RTTI/BsPrefabDiffRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Serialization/BsBinarySerializer.h"
#include "Serialization/BsBinaryDiff.h"
#include "Scene/BsSceneManager.h"
#include "Utility/BsUtility.h"

using namespace bs;

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
	if(prefab->mPrefabLinkUUID != instance->mPrefabLinkUUID)
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
	if(mRoot == nullptr)
		return;

	CoreSerializationContext serzContext;
	serzContext.GoState = bs_shared_ptr_new<GameObjectDeserializationState>(GODM_UseNewIds | GODM_RestoreExternal);
	serzContext.GoDeserializationActive = true;

	ApplyDiff(mRoot, object, &serzContext);

	serzContext.GoState->Resolve();
}

void PrefabDiff::ApplyDiff(const SPtr<PrefabObjectDiff>& diff, const HSceneObject& object, SerializationContext* context)
{
	if((diff->SoFlags & (u32)SceneObjectDiffFlags::Name) != 0)
		object->SetName(diff->Name);

	if((diff->SoFlags & (u32)SceneObjectDiffFlags::Position) != 0)
		object->SetPosition(diff->Position);

	if((diff->SoFlags & (u32)SceneObjectDiffFlags::Rotation) != 0)
		object->SetRotation(diff->Rotation);

	if((diff->SoFlags & (u32)SceneObjectDiffFlags::Scale) != 0)
		object->SetScale(diff->Scale);

	if((diff->SoFlags & (u32)SceneObjectDiffFlags::Active) != 0)
		object->SetActive(diff->IsActive);

	// Note: It is important to remove objects and components first, before adding them.
	//		 Some systems rely on the fact that applyDiff added components/objects are
	//       always at the end.
	const Vector<HComponent>& components = object->GetComponents();
	for(auto& removedId : diff->RemovedComponents)
	{
		for(auto component : components)
		{
			if(removedId == component->GetLinkId())
			{
				component->Destroy(true);
				break;
			}
		}
	}

	for(auto& removedId : diff->RemovedChildren)
	{
		u32 childCount = object->GetNumChildren();
		for(u32 i = 0; i < childCount; i++)
		{
			HSceneObject child = object->GetChild(i);
			if(removedId == child->GetLinkId())
			{
				child->Destroy(true);
				break;
			}
		}
	}

	for(auto& addedComponentData : diff->AddedComponents)
	{
		SPtr<Component> component = std::static_pointer_cast<Component>(addedComponentData->Decode(context));

		object->AddAndInitializeComponent(component);
	}

	for(auto& addedChildData : diff->AddedChildren)
	{
		SPtr<SceneObject> sceneObject = std::static_pointer_cast<SceneObject>(addedChildData->Decode(context));
		sceneObject->SetParent(object);

		if(object->IsInstantiated())
			sceneObject->InstantiateInternal();
	}

	for(auto& componentDiff : diff->ComponentDiffs)
	{
		for(auto& component : components)
		{
			if(componentDiff->Id == (i32)component->GetLinkId())
			{
				IDiff& diffHandler = component->GetRtti()->GetDiffHandler();
				diffHandler.ApplyDiff(component.GetInternalPtr(), componentDiff->Data, context);
				break;
			}
		}
	}

	for(auto& childDiff : diff->ChildDiffs)
	{
		u32 childCount = object->GetNumChildren();
		for(u32 i = 0; i < childCount; i++)
		{
			HSceneObject child = object->GetChild(i);
			if(childDiff->Id == child->GetLinkId())
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

	if(prefab->GetName() != instance->GetName())
	{
		if(output == nullptr)
			output = bs_shared_ptr_new<PrefabObjectDiff>();

		output->Name = instance->GetName();
		output->SoFlags |= (u32)SceneObjectDiffFlags::Name;
	}

	const Transform& prefabTfrm = prefab->GetLocalTransform();
	const Transform& instanceTfrm = instance->GetLocalTransform();
	if(prefabTfrm.GetPosition() != instanceTfrm.GetPosition())
	{
		if(output == nullptr)
			output = bs_shared_ptr_new<PrefabObjectDiff>();

		output->Position = instanceTfrm.GetPosition();
		output->SoFlags |= (u32)SceneObjectDiffFlags::Position;
	}

	if(prefabTfrm.GetRotation() != instanceTfrm.GetRotation())
	{
		if(output == nullptr)
			output = bs_shared_ptr_new<PrefabObjectDiff>();

		output->Rotation = instanceTfrm.GetRotation();
		output->SoFlags |= (u32)SceneObjectDiffFlags::Rotation;
	}

	if(prefabTfrm.GetScale() != instanceTfrm.GetScale())
	{
		if(output == nullptr)
			output = bs_shared_ptr_new<PrefabObjectDiff>();

		output->Scale = instanceTfrm.GetScale();
		output->SoFlags |= (u32)SceneObjectDiffFlags::Scale;
	}

	if(prefab->GetActive() != instance->GetActive())
	{
		if(output == nullptr)
			output = bs_shared_ptr_new<PrefabObjectDiff>();

		output->IsActive = instance->GetActive();
		output->SoFlags |= (u32)SceneObjectDiffFlags::Active;
	}

	u32 prefabChildCount = prefab->GetNumChildren();
	u32 instanceChildCount = instance->GetNumChildren();

	// Find modified and removed children
	for(u32 i = 0; i < prefabChildCount; i++)
	{
		HSceneObject prefabChild = prefab->GetChild(i);

		SPtr<PrefabObjectDiff> childDiff;
		bool foundMatching = false;
		for(u32 j = 0; j < instanceChildCount; j++)
		{
			HSceneObject instanceChild = instance->GetChild(j);

			if(prefabChild->GetLinkId() == instanceChild->GetLinkId())
			{
				if(instanceChild->mPrefabLinkUUID.Empty())
					childDiff = GenerateDiff(prefabChild, instanceChild);

				foundMatching = true;
				break;
			}
		}

		if(foundMatching)
		{
			if(childDiff != nullptr)
			{
				if(output == nullptr)
					output = bs_shared_ptr_new<PrefabObjectDiff>();

				output->ChildDiffs.push_back(childDiff);
			}
		}
		else
		{
			if(output == nullptr)
				output = bs_shared_ptr_new<PrefabObjectDiff>();

			output->RemovedChildren.push_back(prefabChild->GetLinkId());
		}
	}

	// Find added children
	for(u32 i = 0; i < instanceChildCount; i++)
	{
		HSceneObject instanceChild = instance->GetChild(i);

		if(instanceChild->HasFlag(SOF_DontSave))
			continue;

		bool foundMatching = false;
		if(instanceChild->GetLinkId() != (u32)-1)
		{
			for(u32 j = 0; j < prefabChildCount; j++)
			{
				HSceneObject prefabChild = prefab->GetChild(j);

				if(prefabChild->GetLinkId() == instanceChild->GetLinkId())
				{
					foundMatching = true;
					break;
				}
			}
		}

		if(!foundMatching)
		{
			SPtr<SerializedObject> obj = SerializedObject::Create(*instanceChild);

			if(output == nullptr)
				output = bs_shared_ptr_new<PrefabObjectDiff>();

			output->AddedChildren.push_back(obj);
		}
	}

	const Vector<HComponent>& prefabComponents = prefab->GetComponents();
	const Vector<HComponent>& instanceComponents = instance->GetComponents();

	u32 prefabComponentCount = (u32)prefabComponents.size();
	u32 instanceComponentCount = (u32)instanceComponents.size();

	// Find modified and removed components
	for(u32 i = 0; i < prefabComponentCount; i++)
	{
		HComponent prefabComponent = prefabComponents[i];

		SPtr<PrefabComponentDiff> childDiff;
		bool foundMatching = false;
		for(u32 j = 0; j < instanceComponentCount; j++)
		{
			HComponent instanceComponent = instanceComponents[j];

			if(prefabComponent->GetLinkId() == instanceComponent->GetLinkId())
			{
				SPtr<SerializedObject> encodedPrefab = SerializedObject::Create(*prefabComponent);
				SPtr<SerializedObject> encodedInstance = SerializedObject::Create(*instanceComponent);

				IDiff& diffHandler = prefabComponent->GetRtti()->GetDiffHandler();
				SPtr<SerializedObject> diff = diffHandler.GenerateDiff(encodedPrefab, encodedInstance);

				if(diff != nullptr)
				{
					childDiff = bs_shared_ptr_new<PrefabComponentDiff>();
					childDiff->Id = prefabComponent->GetLinkId();
					childDiff->Data = diff;
				}

				foundMatching = true;
				break;
			}
		}

		if(foundMatching)
		{
			if(childDiff != nullptr)
			{
				if(output == nullptr)
					output = bs_shared_ptr_new<PrefabObjectDiff>();

				output->ComponentDiffs.push_back(childDiff);
			}
		}
		else
		{
			if(output == nullptr)
				output = bs_shared_ptr_new<PrefabObjectDiff>();

			output->RemovedComponents.push_back(prefabComponent->GetLinkId());
		}
	}

	// Find added components
	for(u32 i = 0; i < instanceComponentCount; i++)
	{
		HComponent instanceComponent = instanceComponents[i];

		bool foundMatching = false;
		if(instanceComponent->GetLinkId() != (u32)-1)
		{
			for(u32 j = 0; j < prefabComponentCount; j++)
			{
				HComponent prefabComponent = prefabComponents[j];

				if(prefabComponent->GetLinkId() == instanceComponent->GetLinkId())
				{
					foundMatching = true;
					break;
				}
			}
		}

		if(!foundMatching)
		{
			SPtr<SerializedObject> obj = SerializedObject::Create(*instanceComponent);

			if(output == nullptr)
				output = bs_shared_ptr_new<PrefabObjectDiff>();

			output->AddedComponents.push_back(obj);
		}
	}

	if(output != nullptr)
		output->Id = instance->GetLinkId();

	return output;
}

void PrefabDiff::RenameInstanceIds(const HSceneObject& prefab, const HSceneObject& instance, Vector<RenamedGameObject>& output)
{
	UnorderedMap<UUID, UnorderedMap<u32, u64>> linkToInstanceId;

	struct StackEntry
	{
		HSceneObject So;
		UUID Uuid;
	};

	// When renaming it is important to rename the prefab and not the instance, since the diff will otherwise
	// contain prefab's IDs, but will be used for the instance.

	Stack<StackEntry> todo;
	todo.push({ instance, UUID::EMPTY });

	while(!todo.empty())
	{
		StackEntry current = todo.top();
		todo.pop();

		UUID childParentUUID;
		if(current.So->mPrefabLinkUUID.Empty())
			childParentUUID = current.Uuid;
		else
			childParentUUID = current.So->mPrefabLinkUUID;

		UnorderedMap<u32, u64>& idMap = linkToInstanceId[childParentUUID];

		const Vector<HComponent>& components = current.So->GetComponents();
		for(auto& component : components)
		{
			if(component->GetLinkId() != (u32)-1)
				idMap[component->GetLinkId()] = component->GetInstanceId();
		}

		u32 numChildren = current.So->GetNumChildren();
		for(u32 i = 0; i < numChildren; i++)
		{
			HSceneObject child = current.So->GetChild(i);

			if(child->GetLinkId() != (u32)-1)
				idMap[child->GetLinkId()] = child->GetInstanceId();

			todo.push({ child, childParentUUID });
		}
	}

	// Root has link ID from its parent so we handle it separately
	{
		output.push_back(RenamedGameObject());
		RenamedGameObject& renamedGO = output.back();
		renamedGO.InstanceData = instance->mInstanceData;
		renamedGO.OriginalId = instance->GetInstanceId();

		prefab->mInstanceData->MInstanceId = instance->GetInstanceId();
	}

	todo.push({ prefab, UUID::EMPTY });
	while(!todo.empty())
	{
		StackEntry current = todo.top();
		todo.pop();

		UUID childParentUUID;
		if(current.So->mPrefabLinkUUID.Empty())
			childParentUUID = current.Uuid;
		else
			childParentUUID = current.So->mPrefabLinkUUID;

		auto iterFind = linkToInstanceId.find(childParentUUID);
		if(iterFind != linkToInstanceId.end())
		{
			UnorderedMap<u32, u64>& idMap = iterFind->second;

			const Vector<HComponent>& components = current.So->GetComponents();
			for(auto& component : components)
			{
				auto iterFind2 = idMap.find(component->GetLinkId());
				if(iterFind2 != idMap.end())
				{
					output.push_back(RenamedGameObject());
					RenamedGameObject& renamedGO = output.back();
					renamedGO.InstanceData = component->mInstanceData;
					renamedGO.OriginalId = component->GetInstanceId();

					component->mInstanceData->MInstanceId = iterFind2->second;
				}
			}
		}

		u32 numChildren = current.So->GetNumChildren();
		for(u32 i = 0; i < numChildren; i++)
		{
			HSceneObject child = current.So->GetChild(i);

			if(iterFind != linkToInstanceId.end())
			{
				if(child->GetLinkId() != (u32)-1)
				{
					UnorderedMap<u32, u64>& idMap = iterFind->second;

					auto iterFind2 = idMap.find(child->GetLinkId());
					if(iterFind2 != idMap.end())
					{
						output.push_back(RenamedGameObject());
						RenamedGameObject& renamedGO = output.back();
						renamedGO.InstanceData = child->mInstanceData;
						renamedGO.OriginalId = child->GetInstanceId();

						child->mInstanceData->MInstanceId = iterFind2->second;
					}
				}
			}

			todo.push({ child, childParentUUID });
		}
	}
}

void PrefabDiff::RestoreInstanceIds(const Vector<RenamedGameObject>& renamedObjects)
{
	for(auto& renamedGO : renamedObjects)
		renamedGO.InstanceData->MInstanceId = renamedGO.OriginalId;
}

RTTITypeBase* PrefabDiff::GetRttiStatic()
{
	return PrefabDiffRTTI::Instance();
}

RTTITypeBase* PrefabDiff::GetRtti() const
{
	return PrefabDiff::GetRttiStatic();
}
