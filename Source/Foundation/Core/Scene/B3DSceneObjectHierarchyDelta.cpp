//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/B3DSceneObjectHierarchyDelta.h"

#include "B3DGameObjectCollection.h"
#include "RTTI/B3DSceneObjectHierarchyDeltaRTTI.h"
#include "Scene/B3DSceneObject.h"
#include "Serialization/B3DBinarySerializer.h"
#include "Serialization/B3DBinaryDelta.h"
#include "Scene/B3DSceneManager.h"
#include "Utility/B3DUtility.h"
#include "B3DPrefab.h"
#include "B3DPrefabUtility.h"

using namespace b3d;

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
	Flags.Set(GameObjectDeltaFlag::ComponentDelta);
}

SceneObjectHierarchyDeltaObject::SceneObjectHierarchyDeltaObject(const HSceneObject& sceneObject, const SPtr<SerializedObject>& data)
{
	Id = sceneObject.GetId();
	ParentId = sceneObject->GetParent().IsValid() ? sceneObject->GetParent().GetId() : UUID::kEmpty;
	PrefabObjectId = sceneObject->GetPrefabObjectId();
	PrefabResourceId = sceneObject->GetPrefabResourceId();
	Data = data;
	Flags.Set(GameObjectDeltaFlag::SceneObjectDelta);
}

RTTIType* SceneObjectHierarchyDeltaObject::GetRttiStatic()
{
	return SceneObjectHierarchyDeltaObjectRTTI::Instance();
}

RTTIType* SceneObjectHierarchyDeltaObject::GetRtti() const
{
	return GetRttiStatic();
}

SPtr<SceneObjectHierarchyDelta> SceneObjectHierarchyDelta::Create(const HSceneObject& original, const HSceneObject& modified, SceneObjectHierarchyDeltaFlags flags)
{
	if(original->GetPrefabResourceId() != modified->GetPrefabResourceId())
	{
		B3D_LOG(Warning, Prefab, "Cannot create a delta between objects not linked to the same prefab.");
		return nullptr;
	}

	const bool isPrefabDelta = flags.IsSet(SceneObjectHierarchyDeltaFlag::PrefabDelta);

	RTTIOperationEngineContext rttiOperationContext;
	if(isPrefabDelta && modified.IsValid())
	{
		UnorderedMap<UUID, PrefabLinkInformation> instanceIdToPrefabLink = PrefabUtility::GetInstanceToPrefabLinkInformationMap(modified, true);
		for(const auto& pair : instanceIdToPrefabLink)
			rttiOperationContext.GameObjectIdRemapping[pair.first] = pair.second.PrefabObjectId;
	}

	SPtr<SceneObjectHierarchyDelta> output = B3DMakeShared<SceneObjectHierarchyDelta>();
	GenerateHierarchyDelta(original, modified, rttiOperationContext, flags, output);

	return output;
}

void SceneObjectHierarchyDelta::Apply(const HSceneObject& original, SceneObjectHierarchyDeltaFlags flags)
{
	const SPtr<GameObjectCollection> gameObjectCollection = original->GetOwnerCollection().lock();
	if(!B3D_ENSURE(gameObjectCollection != nullptr))
		return;

	const bool isPrefabDelta = flags.IsSet(SceneObjectHierarchyDeltaFlag::PrefabDelta);

	RTTIOperationEngineContext rttiOperationContext;
	rttiOperationContext.IsGameObjectDeserializationActive = true;
	rttiOperationContext.PreserveGameObjectIds = !isPrefabDelta;
	rttiOperationContext.GameObjectCollection = gameObjectCollection;
	// TODO - Should pass a flag to the RTTI system here, so individual RTTIType implementations can gracefully handle delta apply (rather than thinking it is deserialization)

	rttiOperationContext.GameObjectCollection->BeginHandleResolve();

	FrameAllocatorScope frameScope;

	FrameUnorderedSet<UUID> addedSceneObjectsMap;
	for(const auto& entry : AddedSceneObjects)
	{
		auto result = addedSceneObjectsMap.insert(entry);
		B3D_ENSURE(result.second); // This failing means there's a logic error when generating the delta, resulting in duplicate IDs
	}

	FrameUnorderedSet<UUID> addedComponentsMap;
	for(const auto& entry : AddedComponents)
	{
		auto result = addedComponentsMap.insert(entry);
		B3D_ENSURE(result.second); // This failing means there's a logic error when generating the delta, resulting in duplicate IDs
	}

	// Construct newly added scene objects, but don't instantiate them until later
	UnorderedMap<UUID, HSceneObject> createdSceneObjects;
	for(const auto& sceneObjectId : AddedSceneObjects)
	{
		if(!B3D_ENSURE(!gameObjectCollection->GetObject(sceneObjectId).IsValid())) // Object with same ID already exists
			continue;

		auto found = Objects.find(sceneObjectId);
		if(!B3D_ENSURE(found != Objects.end() && found->second != nullptr))
			continue;

		const SPtr<SceneObjectHierarchyDeltaObject> deltaObject = found->second;
		if(!B3D_ENSURE(deltaObject->Data != nullptr))
			continue;

		const SPtr<SceneObject> newSceneObject = B3DRTTICast<SceneObject>(deltaObject->Data->Decode(rttiOperationContext));
		if(!B3D_ENSURE(newSceneObject != nullptr))
			continue;

		newSceneObject->SetPrefabObjectId(deltaObject->PrefabObjectId);
		newSceneObject->SetPrefabResourceId(deltaObject->PrefabResourceId);

		auto result = createdSceneObjects.insert(std::make_pair(sceneObjectId, newSceneObject->GetHandle()));
		B3D_ENSURE(result.second);
	}

	// Looks up scene objects based on modified object IDs stored in the delta
	auto fnFindSceneObject = [&createdSceneObjects, &gameObjectCollection](const UUID& id)
	{
		if(auto found = createdSceneObjects.find(id); found != createdSceneObjects.end())
			return found->second;

		return B3DStaticGameObjectCast<SceneObject>(gameObjectCollection->GetObject(id));
	};

	// Construct newly added components
	for(const auto& componentId : AddedComponents)
	{
		if(!B3D_ENSURE(!gameObjectCollection->GetObject(componentId).IsValid())) // Object with same ID already exists
			continue;

		auto found = Objects.find(componentId);
		if(!B3D_ENSURE(found != Objects.end() && found->second != nullptr))
			continue;

		const SPtr<SceneObjectHierarchyDeltaObject> deltaObject = found->second;
		if(!B3D_ENSURE(deltaObject->Data != nullptr))
			continue;

		HSceneObject parentSceneObject = fnFindSceneObject(deltaObject->ParentId);
		if(!B3D_ENSURE(parentSceneObject.IsValid()))
			continue;

		const SPtr<Component> newComponent = B3DRTTICast<Component>(deltaObject->Data->Decode(rttiOperationContext));
		if(!B3D_ENSURE(newComponent != nullptr))
			continue;

		newComponent->SetPrefabObjectId(deltaObject->PrefabObjectId);

		parentSceneObject->InternalAddComponent(newComponent, false);
	}

	// Once all the objects are created, apply all changes
	for(const auto& entry : Objects)
	{
		const SPtr<SceneObjectHierarchyDeltaObject> deltaObject = entry.second;
		if(!B3D_ENSURE(deltaObject != nullptr))
			continue;

		const UUID& gameObjectId = entry.first;
		if(deltaObject->Flags.IsSet(GameObjectDeltaFlag::SceneObjectDelta))
		{
			HSceneObject sceneObject = fnFindSceneObject(gameObjectId);
			if(!B3D_ENSURE(sceneObject.IsValid()))
				return;

			const bool isNewObject = addedSceneObjectsMap.find(gameObjectId) != addedSceneObjectsMap.end();
			if(deltaObject->Flags.IsSet(GameObjectDeltaFlag::ParentChanged) || isNewObject)
			{
				HSceneObject parentSceneObject = fnFindSceneObject(deltaObject->ParentId);
				if(B3D_ENSURE(parentSceneObject.IsValid()))
					sceneObject->SetParent(parentSceneObject, false);
			}

			if(!isNewObject && deltaObject->Data != nullptr)
			{
				IDeltaHandler& deltaHandler = sceneObject->GetRtti()->GetDeltaHandler();
				deltaHandler.ApplyDelta(sceneObject.GetShared(), deltaObject->Data, rttiOperationContext);
			}
		}
		else if(deltaObject->Flags.IsSet(GameObjectDeltaFlag::ComponentDelta))
		{
			const bool isNewObject = addedComponentsMap.find(gameObjectId) != addedComponentsMap.end();
			if(isNewObject)
				continue;

			HComponent component = B3DStaticGameObjectCast<Component>(gameObjectCollection->GetObject(gameObjectId));
			if(!B3D_ENSURE(component.IsValid()))
				return;

			if(deltaObject->Data != nullptr)
			{
				IDeltaHandler& deltaHandler = component->GetRtti()->GetDeltaHandler();
				deltaHandler.ApplyDelta(component.GetShared(), deltaObject->Data, rttiOperationContext);
			}
		}
		else
		{
			B3D_ENSURE(false);
		}
	}

	// Initialize if the object we're applying the delta to is also initialized
	if(original->HasGameObjectFlag(GameObjectTransientFlag::Initialized))
	{
		for(const auto& entry : createdSceneObjects)
		{
			if(!entry.second->HasGameObjectFlag(GameObjectTransientFlag::Initialized))
				entry.second->Initialize();
		}
	}

	UnorderedMap<UUID, UUID> prefabToInstanceMap = PrefabUtility::GetPrefabToInstanceIdMap(original, true);

	// Remove components and scene objects
	for(const auto& componentId : RemovedComponents)
	{
		UUID componentInstanceId = componentId;

		if(isPrefabDelta)
		{
			// For destroyed objects we record the ID of the original object (which will be the prefab object ID of the object we're applying the delta to)
			if(auto found = prefabToInstanceMap.find(componentId); found != prefabToInstanceMap.end())
				componentInstanceId = found->second;
		}

		HComponent component = B3DStaticGameObjectCast<Component>(gameObjectCollection->GetObject(componentInstanceId));
		if(!B3D_ENSURE(component.IsValid()))
			continue;

		component->Destroy(true);
	}

	FrameUnorderedSet<UUID> destroyedObjects;
	for(const auto& sceneObjectId : RemovedSceneObjects)
	{
		UUID sceneObjectInstanceId = sceneObjectId;

		if(isPrefabDelta)
		{
			// For destroyed objects we record the ID of the original object (which will be the prefab object ID of the object we're applying the delta to)
			if(auto found = prefabToInstanceMap.find(sceneObjectId); found != prefabToInstanceMap.end())
				sceneObjectInstanceId = found->second;
		}

		HSceneObject sceneObject = B3DStaticGameObjectCast<SceneObject>(gameObjectCollection->GetObject(sceneObjectInstanceId));
		if(!B3D_ENSURE(sceneObject.IsValid()))
			continue;

		sceneObject->Destroy(true);
	}

	rttiOperationContext.GameObjectCollection->EndHandleResolve();
}

void SceneObjectHierarchyDelta::GenerateHierarchyDelta(const HSceneObject& original, const HSceneObject& modified, RTTIOperationContext& context, SceneObjectHierarchyDeltaFlags flags, SPtr<SceneObjectHierarchyDelta>& outDelta)
{
	const bool isPrefabDelta = flags.IsSet(SceneObjectHierarchyDeltaFlag::PrefabDelta);

	const bool isOriginalValid = original.IsValid();
	const bool isModifiedValid = modified.IsValid();

	if(!isOriginalValid && !isModifiedValid)
		return;

	const SPtr<GameObjectCollection> modifiedGameObjectCollection = isModifiedValid ? modified->GetOwnerCollection().lock() : nullptr;
	const UnorderedMap<UUID, UUID> modifiedPrefabToInstanceIdMap = isPrefabDelta ? PrefabUtility::GetPrefabToInstanceIdMap(modified, true) : UnorderedMap<UUID, UUID>();

	auto fnFindModifiedAndRemovedChildren =
		[isOriginalValid, modifiedGameObjectCollection, &modifiedPrefabToInstanceIdMap, context, flags, outDelta](const HSceneObject& original, bool ignoreParent, auto& fnFindModifiedAndRemovedChildren) mutable -> void {
		const u32 originalChildCount = isOriginalValid ? original->GetChildCount() : 0;
		for(u32 originalChildIndex = 0; originalChildIndex < originalChildCount; ++originalChildIndex)
		{
			const HSceneObject& originalChild = original->GetChild(originalChildIndex);

			if(originalChild->HasFlag(SceneObjectFlag::DontSave) || originalChild->HasFlag(SceneObjectFlag::RuntimePersistent))
				continue;

			// Process children first
			fnFindModifiedAndRemovedChildren(originalChild, false, fnFindModifiedAndRemovedChildren);

			UUID modifiedCompareId = originalChild.GetId();
			if(auto found = modifiedPrefabToInstanceIdMap.find(originalChild.GetId()); found != modifiedPrefabToInstanceIdMap.end())
				modifiedCompareId = found->second;

			HSceneObject modifiedChild = B3DStaticGameObjectCast<SceneObject>(modifiedGameObjectCollection->GetObject(modifiedCompareId));
			GenerateSceneObjectDelta(originalChild, modifiedChild, context, flags, ignoreParent, outDelta);
		}
	};

	const SPtr<GameObjectCollection> originalGameObjectCollection = isOriginalValid ? original->GetOwnerCollection().lock() : nullptr;

	auto fnFindAddedChildren =
		[isModifiedValid, originalGameObjectCollection, isPrefabDelta, context, flags, outDelta](const HSceneObject& modified, auto& fnFindAddedChildren) mutable -> void {
		const u32 modifiedChildCount = isModifiedValid ? modified->GetChildCount() : 0;
		for(u32 modifiedChildIndex = 0; modifiedChildIndex < modifiedChildCount; ++modifiedChildIndex)
		{
			const HSceneObject& modifiedChild = modified->GetChild(modifiedChildIndex);

			if(modifiedChild->HasFlag(SceneObjectFlag::DontSave) || modifiedChild->HasFlag(SceneObjectFlag::RuntimePersistent))
				continue;

			// Process children first
			fnFindAddedChildren(modifiedChild, fnFindAddedChildren);

			UUID originalCompareId = GetPrefabOrInstanceId(modifiedChild, isPrefabDelta);
			HSceneObject originalChild = B3DStaticGameObjectCast<SceneObject>(originalGameObjectCollection->GetObject(originalCompareId));
			if(!originalChild.IsValid())
				GenerateSceneObjectDelta(HSceneObject(), modifiedChild, context, flags, false, outDelta);
		}
	};

	fnFindModifiedAndRemovedChildren(original, true, fnFindModifiedAndRemovedChildren);
	fnFindAddedChildren(modified, fnFindAddedChildren);
}

bool SceneObjectHierarchyDelta::GenerateSceneObjectDelta(const HSceneObject& original, const HSceneObject& modified, RTTIOperationContext& context, SceneObjectHierarchyDeltaFlags flags, bool ignoreParent, SPtr<SceneObjectHierarchyDelta>& outDelta)
{
	const bool isPrefabDelta = flags.IsSet(SceneObjectHierarchyDeltaFlag::PrefabDelta);
	const bool isOriginalValid = original.IsValid();
	const bool isModifiedValid = modified.IsValid();

	if(!isOriginalValid && !isModifiedValid)
		return false;

	if(isOriginalValid && isModifiedValid)
	{
		if(!B3D_ENSURE(original.GetId() == GetPrefabOrInstanceId(modified, isPrefabDelta)))
			return false;
	}

	auto fnEnsureOutputObjectIsValid = [&outDelta]
	{
		if(!outDelta)
			outDelta = B3DMakeShared<SceneObjectHierarchyDelta>();
	};

	if(!isModifiedValid)
	{
		fnEnsureOutputObjectIsValid();
		outDelta->RemovedSceneObjects.push_back(original.GetId());

		return true;
	}

	if(!isOriginalValid)
	{
		SPtr<SerializedObject> serializedModified = SerializedObject::Create(*modified, SerializedObjectEncodeFlag::IsDeltaCopy);
		SPtr<SceneObjectHierarchyDeltaObject> sceneDeltaObject = B3DMakeShared<SceneObjectHierarchyDeltaObject>(modified, serializedModified);
		fnEnsureOutputObjectIsValid();

		const auto result = outDelta->Objects.insert(std::make_pair(sceneDeltaObject->Id, sceneDeltaObject));
		if(result.second)
			outDelta->AddedSceneObjects.push_back(sceneDeltaObject->Id);
	}
	else
	{
		// Convert to serialized object first, because non-native delta handler only supports this
		const SPtr<SerializedObject> serializedOriginal = SerializedObject::Create(*original, SerializedObjectEncodeFlag::IsDeltaCopy);
		const SPtr<SerializedObject> serializedModified = SerializedObject::Create(*modified, SerializedObjectEncodeFlag::IsDeltaCopy);

		IDeltaHandler& deltaHandler = original->GetRtti()->GetDeltaHandler();
		SPtr<SerializedObject> delta = deltaHandler.GenerateDelta(serializedOriginal, serializedModified, context);

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
		const UUID& modifiedParentId = modifiedParent.IsValid() ? GetPrefabOrInstanceId(modifiedParent, isPrefabDelta) : UUID::kEmpty;
		if(!ignoreParent && originalParentId != modifiedParentId)
		{
			if(sceneDeltaObject == nullptr)
			{
				sceneDeltaObject = B3DMakeShared<SceneObjectHierarchyDeltaObject>(modified, nullptr);
				fnEnsureOutputObjectIsValid();

				const auto result = outDelta->Objects.insert(std::make_pair(sceneDeltaObject->Id, sceneDeltaObject));
				B3D_ASSERT(result.second);
			}

			sceneDeltaObject->Flags.Set(GameObjectDeltaFlag::ParentChanged);
		}
	}

	GenerateComponentDelta(original, modified, context, flags, outDelta);
	return true;
}

void SceneObjectHierarchyDelta::GenerateComponentDelta(const HSceneObject& original, const HSceneObject& modified, RTTIOperationContext& context, SceneObjectHierarchyDeltaFlags flags, SPtr<SceneObjectHierarchyDelta>& outDelta)
{
	if(!B3D_ENSURE(modified.IsValid()))
		return;

	auto fnEnsureOutputObjectIsValid = [&outDelta]
	{
		if(!outDelta)
			outDelta = B3DMakeShared<SceneObjectHierarchyDelta>();
	};

	const bool isPrefabDelta = flags.IsSet(SceneObjectHierarchyDeltaFlag::PrefabDelta);
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
				// Convert to serialized object first, because non-native delta handler only supports this
				const SPtr<SerializedObject> serializedOriginal = SerializedObject::Create(*originalComponent, SerializedObjectEncodeFlag::IsDeltaCopy);
				const SPtr<SerializedObject> serializedModified = SerializedObject::Create(*modifiedComponent, SerializedObjectEncodeFlag::IsDeltaCopy);

				IDeltaHandler& deltaHandler = originalComponent->GetRtti()->GetDeltaHandler();
				SPtr<SerializedObject> delta = deltaHandler.GenerateDelta(serializedOriginal, serializedModified, context);

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
		const UUID& modifiedComponentId = GetPrefabOrInstanceId(modifiedComponent, isPrefabDelta);

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
			SPtr<SerializedObject> serializedModified = SerializedObject::Create(*modifiedComponent, SerializedObjectEncodeFlag::IsDeltaCopy);
			SPtr<SceneObjectHierarchyDeltaObject> sceneDeltaObject = B3DMakeShared<SceneObjectHierarchyDeltaObject>(modifiedComponent, serializedModified);
			fnEnsureOutputObjectIsValid();

			const auto result = outDelta->Objects.insert(std::make_pair(sceneDeltaObject->Id, sceneDeltaObject));
			if(result.second)
				outDelta->AddedComponents.push_back(sceneDeltaObject->Id);
		}
	}
}

RTTIType* SceneObjectHierarchyDelta::GetRttiStatic()
{
	return SceneObjectHierarchyDeltaRTTI::Instance();
}

RTTIType* SceneObjectHierarchyDelta::GetRtti() const
{
	return GetRttiStatic();
}
