//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsSceneObjectHierarchyDelta.h"
#include "Private/RTTI/BsSceneObjectHierarchyDeltaRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Serialization/BsBinarySerializer.h"
#include "Serialization/BsBinaryDiff.h"
#include "Scene/BsSceneManager.h"
#include "Utility/BsUtility.h"
#include "BsPrefab.h"

using namespace bs;

/**
 * Returns either the prefab or instance id of the object, depending on @p returnPrefabId parameter.
 * If object has no valid prefab object id, returns instance id in either case.
 */
static UUID GetPrefabOrInstanceId(const GameObjectHandleBase& object, bool returnPrefabId)
{
	if(returnPrefabId && !object->GetPrefabObjectId().Empty())
		return object->GetPrefabObjectId();

	return object.GetId();
}

SceneObjectHierarchyDeltaObject::SceneObjectHierarchyDeltaObject(const HComponent& component, const SPtr<SerializedObject>& data)
{
	Id = component.GetId();
	ParentId = component->SceneObject().GetId();
	PrefabObjectId = component->GetPrefabObjectId();
	Data = data;
}

SceneObjectHierarchyDeltaObject::SceneObjectHierarchyDeltaObject(const HSceneObject& sceneObject, const SPtr<SerializedObject>& data)
{
	Id = sceneObject.GetId();
	ParentId = sceneObject->GetParent().IsValid() ? sceneObject->GetParent().GetId() : UUID::kEmpty;
	PrefabObjectId = sceneObject->GetPrefabObjectId();
	PrefabResourceId = sceneObject->GetPrefabResourceId();
	Data = data;
}


RTTITypeBase* SceneObjectHierarchyDeltaObject::GetRttiStatic()
{
	return SceneObjectHierarchyDeltaObjectRTTI::Instance();
}

RTTITypeBase* SceneObjectHierarchyDeltaObject::GetRtti() const
{
	return GetRttiStatic();
}

SPtr<SceneObjectHierarchyDelta> SceneObjectHierarchyDelta::Create(const HSceneObject& original, const HSceneObject& modified)
{
	if(original->GetPrefabResourceId() != modified->GetPrefabResourceId())
	{
		B3D_LOG(Warning, Prefab, "Cannot create a delta between objects not linked to the same prefab.");
		return nullptr;
	}

	SPtr<SceneObjectHierarchyDelta> output = B3DMakeShared<SceneObjectHierarchyDelta>();
	output->mRoot = GenerateDelta(original, modified);

	return output;
}

void SceneObjectHierarchyDelta::Apply(const HSceneObject& original)
{
	if(mRoot == nullptr)
		return;

	CoreSerializationContext serzContext;
	serzContext.GoState = B3DMakeShared<GameObjectDeserializationState>(GODM_UseNewIds | GODM_RestoreExternal);
	serzContext.GoDeserializationActive = true;

	ApplyDiff(mRoot, original, &serzContext);

	serzContext.GoState->Resolve();
}

void SceneObjectHierarchyDelta::ApplyDiff(const SPtr<SceneObjectHierarchyDeltaObject>& delta, const HSceneObject& original, SerializationContext* context)
{
	if((delta->SoFlags & (u32)SceneObjectDiffFlags::Name) != 0)
		original->SetName(delta->Name);

	if((delta->SoFlags & (u32)SceneObjectDiffFlags::Position) != 0)
		original->SetPosition(delta->Position);

	if((delta->SoFlags & (u32)SceneObjectDiffFlags::Rotation) != 0)
		original->SetRotation(delta->Rotation);

	if((delta->SoFlags & (u32)SceneObjectDiffFlags::Scale) != 0)
		original->SetScale(delta->Scale);

	if((delta->SoFlags & (u32)SceneObjectDiffFlags::Active) != 0)
		original->SetActive(delta->IsActive);

	// Note: It is important to remove objects and components first, before adding them.
	//		 Some systems rely on the fact that applyDiff added components/objects are
	//       always at the end.
	const Vector<HComponent>& components = original->GetComponents();
	for(auto& removedId : delta->RemovedComponents)
	{
		for(auto component : components)
		{
			if(removedId == component->GetPrefabObjectId())
			{
				component->Destroy(true);
				break;
			}
		}
	}

	for(auto& removedId : delta->RemovedChildren)
	{
		u32 childCount = original->GetChildCount();
		for(u32 i = 0; i < childCount; i++)
		{
			HSceneObject child = original->GetChild(i);
			if(removedId == child->GetPrefabObjectId())
			{
				child->Destroy(true);
				break;
			}
		}
	}

	for(auto& addedComponentData : delta->AddedComponents)
	{
		SPtr<Component> component = std::static_pointer_cast<Component>(addedComponentData->Decode(context));

		original->AddAndInitializeComponent(component);
	}

	for(auto& addedChildData : delta->AddedChildren)
	{
		SPtr<SceneObject> sceneObject = std::static_pointer_cast<SceneObject>(addedChildData->Decode(context));
		sceneObject->SetParent(original);

		if(original->IsInstantiated())
			sceneObject->InstantiateInternal();
	}

	for(auto& componentDiff : delta->ComponentDeltas)
	{
		for(auto& component : components)
		{
			if(componentDiff->Id == component->GetPrefabObjectId())
			{
				IDiff& diffHandler = component->GetRtti()->GetDiffHandler();
				diffHandler.ApplyDiff(component.GetShared(), componentDiff->Data, context);
				break;
			}
		}
	}

	for(auto& childDiff : delta->ChildDeltas)
	{
		u32 childCount = original->GetChildCount();
		for(u32 i = 0; i < childCount; i++)
		{
			HSceneObject child = original->GetChild(i);
			if(childDiff->Id == child->GetPrefabObjectId())
			{
				ApplyDiff(childDiff, child, context);
				break;
			}
		}
	}
}

SPtr<SceneObjectHierarchyDeltaObject> SceneObjectHierarchyDelta::GenerateDelta(const HSceneObject& original, const HSceneObject& modified)
{
	const bool isPrefabCompare = true; // TODO - Toggle this via a flag

	const bool isOriginalValid = original.IsValid();
	const bool isModifiedValid = modified.IsValid();

	if(!isOriginalValid && !isModifiedValid)
		return nullptr;

	CoreSerializationContext serializationContext;
	if(isPrefabCompare)
	{
		// TODO - Set up
		// serializationContext.GameObjectIdRemappingTable
	}

	SPtr<SceneObjectHierarchyDeltaObject> output;


	u32 prefabChildCount = original->GetChildCount();
	u32 instanceChildCount = modified->GetChildCount();

	// Find modified and removed children
	for(u32 i = 0; i < prefabChildCount; i++)
	{
		HSceneObject prefabChild = original->GetChild(i);

		SPtr<SceneObjectHierarchyDeltaObject> childDiff;
		bool foundMatching = false;
		for(u32 j = 0; j < instanceChildCount; j++)
		{
			HSceneObject instanceChild = modified->GetChild(j);

			if(prefabChild->GetId() == instanceChild->GetPrefabObjectId())
			{
				if(!instanceChild->IsPrefabInstanceRoot())
					childDiff = GenerateDelta(prefabChild, instanceChild);

				foundMatching = true;
				break;
			}
		}

		if(foundMatching)
		{
			if(childDiff != nullptr)
			{
				if(output == nullptr)
					output = B3DMakeShared<SceneObjectHierarchyDeltaObject>();

				output->ChildDeltas.push_back(childDiff);
			}
		}
		else
		{
			if(output == nullptr)
				output = B3DMakeShared<SceneObjectHierarchyDeltaObject>();

			output->RemovedChildren.push_back(prefabChild->GetId());
		}
	}

	// Find added children
	for(u32 i = 0; i < instanceChildCount; i++)
	{
		HSceneObject instanceChild = modified->GetChild(i);

		if(instanceChild->HasFlag(SOF_DontSave))
			continue;

		bool foundMatching = false;
		if(instanceChild->IsPrefabInstance())
		{
			for(u32 j = 0; j < prefabChildCount; j++)
			{
				HSceneObject prefabChild = original->GetChild(j);

				if(prefabChild->GetId() == instanceChild->GetPrefabObjectId())
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
				output = B3DMakeShared<SceneObjectHierarchyDeltaObject>();

			output->AddedChildren.push_back(obj);
		}
	}

	const Vector<HComponent>& prefabComponents = original->GetComponents();
	const Vector<HComponent>& instanceComponents = modified->GetComponents();

	u32 prefabComponentCount = (u32)prefabComponents.size();
	u32 instanceComponentCount = (u32)instanceComponents.size();

	// Find modified and removed components
	for(u32 i = 0; i < prefabComponentCount; i++)
	{
		HComponent prefabComponent = prefabComponents[i];

		SPtr<ComponentDelta> childDiff;
		bool foundMatching = false;
		for(u32 j = 0; j < instanceComponentCount; j++)
		{
			HComponent instanceComponent = instanceComponents[j];

			if(prefabComponent->GetId() == instanceComponent->GetPrefabObjectId())
			{
				SPtr<SerializedObject> encodedPrefab = SerializedObject::Create(*prefabComponent);
				SPtr<SerializedObject> encodedInstance = SerializedObject::Create(*instanceComponent);

				IDiff& diffHandler = prefabComponent->GetRtti()->GetDiffHandler();
				SPtr<SerializedObject> diff = diffHandler.GenerateDiff(encodedPrefab, encodedInstance);

				if(diff != nullptr)
				{
					childDiff = B3DMakeShared<ComponentDelta>();
					childDiff->Id = prefabComponent->GetId();
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
					output = B3DMakeShared<SceneObjectHierarchyDeltaObject>();

				output->ComponentDeltas.push_back(childDiff);
			}
		}
		else
		{
			if(output == nullptr)
				output = B3DMakeShared<SceneObjectHierarchyDeltaObject>();

			output->RemovedComponents.push_back(prefabComponent->GetId());
		}
	}

	// Find added components
	for(u32 i = 0; i < instanceComponentCount; i++)
	{
		HComponent instanceComponent = instanceComponents[i];

		bool foundMatching = false;
		if(instanceComponent->IsPrefabInstance())
		{
			for(u32 j = 0; j < prefabComponentCount; j++)
			{
				HComponent prefabComponent = prefabComponents[j];

				if(prefabComponent->GetId() == instanceComponent->GetPrefabObjectId())
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
				output = B3DMakeShared<SceneObjectHierarchyDeltaObject>();

			output->AddedComponents.push_back(obj);
		}
	}

	if(output != nullptr)
		output->Id = modified->GetPrefabObjectId();

	return output;
}

void SceneObjectHierarchyDelta::GenerateComponentDelta(const HSceneObject& original, const HSceneObject& modified, SPtr<SceneObjectHierarchyDelta>& outDelta)
{
	if(!B3D_ENSURE(modified.IsValid()))
		return;

	auto fnEnsureOutputObjectIsValid = [&outDelta]
	{
		if(!outDelta)
			outDelta = B3DMakeShared<SceneObjectHierarchyDelta>();
	};

	const Vector<HComponent>& originalComponents = original.IsValid() ? original->GetComponents() : Vector<HComponent>();
	const Vector<HComponent>& modifiedComponents = modified->GetComponents();

	const u32 originalComponentCount = (u32)originalComponents.size();
	const u32 modifiedComponentCount = (u32)modifiedComponents.size();

	// Find modified and removed components
	for(u32 originalComponentIndex = 0; originalComponentIndex < originalComponentCount; originalComponentIndex++)
	{
		const HComponent& originalComponent = originalComponents[originalComponentIndex];

		bool foundMatch = false;
		for(u32 headComponentIndex = 0; headComponentIndex < modifiedComponentCount; headComponentIndex++)
		{
			const HComponent& modifiedComponent = modifiedComponents[headComponentIndex];
			const UUID& modifiedComponentId = GetPrefabOrInstanceId(modifiedComponent, isPrefabDelta);

			if(originalComponent.GetId() == modifiedComponentId)
			{
				const SPtr<SerializedObject> serializedOriginal = SerializedObject::Create(*originalComponent);
				const SPtr<SerializedObject> serializedModified = SerializedObject::Create(*modifiedComponent);

				IDiff& diffHandler = originalComponent->GetRtti()->GetDiffHandler();
				SPtr<SerializedObject> delta = diffHandler.GenerateDiff(serializedOriginal, serializedModified, context);

				if(delta != nullptr)
				{
					SPtr<SceneObjectHierarchyDeltaObject> sceneDeltaObject = B3DMakeShared<SceneObjectHierarchyDeltaObject>(modifiedComponent, delta);
					fnEnsureOutputObjectIsValid();

					const auto result = outDelta->Objects.insert(std::make_pair(sceneDeltaObject->Id, sceneDeltaObject));
					B3D_ASSERT(result.second);
				}

				foundMatch = true;
				break;
			}
		}

		if(!foundMatch)
		{
			fnEnsureOutputObjectIsValid();
			outDelta->RemovedComponents.insert(originalComponent.GetId());
		}
	}

	// Find added components
	for(u32 modifiedComponentIndex = 0; modifiedComponentIndex < modifiedComponentCount; modifiedComponentIndex++)
	{
		const HComponent& modifiedComponent = modifiedComponents[modifiedComponentIndex];
		const UUID& modifiedComponentId = GetPrefabOrInstanceId(modified, isPrefabDelta);

		bool foundMatch = false;
		for(u32 originalComponentIndex = 0; originalComponentIndex < originalComponentCount; originalComponentIndex++)
		{
			const HComponent& originalComponent = originalComponents[originalComponentIndex];

			if(originalComponent.GetId() == modifiedComponentId)
			{
				foundMatch = true;
				break;
			}
		}

		if(!foundMatch)
		{
			SPtr<SerializedObject> serializedModified = SerializedObject::Create(*modifiedComponent);
			SPtr<SceneObjectHierarchyDeltaObject> sceneDeltaObject = B3DMakeShared<SceneObjectHierarchyDeltaObject>(modifiedComponent, serializedModified);
			fnEnsureOutputObjectIsValid();

			const auto result = outDelta->Objects.insert(std::make_pair(sceneDeltaObject->Id, sceneDeltaObject));
			if(result.second)
				outDelta->AddedComponents.insert(sceneDeltaObject->Id);
		}
	}
}

void SceneObjectHierarchyDelta::GenerateSceneObjectDelta(const HSceneObject& original, const HSceneObject& modified, SPtr<SceneObjectHierarchyDelta>& outDelta)
{
	const bool isOriginalValid = original.IsValid();
	const bool isModifiedValid = modified.IsValid();

	if(!isOriginalValid && !isModifiedValid)
		return;

	if(isOriginalValid && isModifiedValid)
	{
		if(!B3D_ENSURE(original.GetId() == GetPrefabOrInstanceId(modified, isPrefabDelta)))
			return;
	}

	auto fnEnsureOutputObjectIsValid = [&outDelta]
	{
		if(!outDelta)
			outDelta = B3DMakeShared<SceneObjectHierarchyDelta>();
	};

	if(!isModifiedValid)
	{
		fnEnsureOutputObjectIsValid();
		outDelta->RemovedSceneObjects.insert(modified.GetId());

		return;
	}

	if(!isOriginalValid)
	{
		SPtr<SerializedObject> serializedModified = SerializedObject::Create(*modified);
		SPtr<SceneObjectHierarchyDeltaObject> sceneDeltaObject = B3DMakeShared<SceneObjectHierarchyDeltaObject>(modified, serializedModified);
		fnEnsureOutputObjectIsValid();

		const auto result = outDelta->Objects.insert(std::make_pair(sceneDeltaObject->Id, sceneDeltaObject));
		if(result.second)
			outDelta->AddedSceneObjects.insert(sceneDeltaObject->Id);
	}
	else
	{
		const SPtr<SerializedObject> serializedOriginal = SerializedObject::Create(*original);
		const SPtr<SerializedObject> serializedModified = SerializedObject::Create(*modified);

		IDiff& diffHandler = original->GetRtti()->GetDiffHandler();
		SPtr<SerializedObject> delta = diffHandler.GenerateDiff(serializedOriginal, serializedModified, context);

		SPtr<SceneObjectHierarchyDeltaObject> sceneDeltaObject;
		if(delta != nullptr)
		{
			sceneDeltaObject = B3DMakeShared<SceneObjectHierarchyDeltaObject>(modified, delta);
			fnEnsureOutputObjectIsValid();

			const auto result = outDelta->Objects.insert(std::make_pair(sceneDeltaObject->Id, sceneDeltaObject));
			B3D_ASSERT(result.second);
		}

		const HSceneObject& originalParent = original->GetParent();
		const HSceneObject& modifiedParent = modified->GetParent();

		const UUID& originalParentId = originalParent.IsValid() ? originalParent.GetId() : UUID::kEmpty;
		const UUID& modifiedParentId = modifiedParent.IsValid() ? GetPrefabOrInstanceId(modifiedParent.GetId(), isPrefabDelta) : UUID::kEmpty;
		if(originalParentId != modifiedParentId)
		{
			if(sceneDeltaObject == nullptr)
			{
				sceneDeltaObject = B3DMakeShared<SceneObjectHierarchyDeltaObject>(modified, nullptr);
				fnEnsureOutputObjectIsValid();
				
				const auto result = outDelta->Objects.insert(std::make_pair(sceneDeltaObject->Id, sceneDeltaObject));
				B3D_ASSERT(result.second);
			}

			sceneDeltaObject->Flags.Set(ParentModified);
		}
	}

	// TODO - Missing serialization context flags to avoid comparing the parent/child/component fields in the general purpose serialization
}


RTTITypeBase* SceneObjectHierarchyDelta::GetRttiStatic()
{
	return SceneObjectHierarchyDeltaRTTI::Instance();
}

RTTITypeBase* SceneObjectHierarchyDelta::GetRtti() const
{
	return GetRttiStatic();
}
