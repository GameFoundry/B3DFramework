//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsSceneObjectHierarchyDelta.h"

#include "BsGameObjectCollection.h"
#include "Private/RTTI/BsSceneObjectHierarchyDeltaRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Serialization/BsBinarySerializer.h"
#include "Serialization/BsBinaryDelta.h"
#include "Scene/BsSceneManager.h"
#include "Utility/BsUtility.h"
#include "BsPrefab.h"
#include "BsPrefabUtility.h"

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

SPtr<SceneObjectHierarchyDelta> SceneObjectHierarchyDelta::Create(const HSceneObject& original, const HSceneObject& modified, SceneObjectHierarchyDeltaFlags flags)
{
	if(original->GetPrefabResourceId() != modified->GetPrefabResourceId())
	{
		B3D_LOG(Warning, Prefab, "Cannot create a delta between objects not linked to the same prefab.");
		return nullptr;
	}

	const bool isPrefabDelta = flags.IsSet(SceneObjectHierarchyDeltaFlag::PrefabDelta);

	CoreSerializationContext serializationContext;
	if(isPrefabDelta && modified.IsValid())
	{
		UnorderedMap<UUID, PrefabLinkInformation> instanceIdToPrefabLink = PrefabUtility::GetInstanceToPrefabLinkInformationMap(modified, true);
		for(const auto& pair : instanceIdToPrefabLink)
			serializationContext.GameObjectIdRemapping[pair.first] = pair.second.PrefabObjectId;
	}

	SPtr<SceneObjectHierarchyDelta> output = B3DMakeShared<SceneObjectHierarchyDelta>();
	GenerateHierarchyDelta(original, modified, &serializationContext, flags, output);

	return output;
}

void SceneObjectHierarchyDelta::Apply(const HSceneObject& original, SceneObjectHierarchyDeltaFlags flags)
{
	const SPtr<GameObjectCollection> gameObjectCollection = original->GetOwnerCollection().lock();
	if(!B3D_ENSURE(gameObjectCollection != nullptr))
		return;

	const bool isPrefabDelta = flags.IsSet(SceneObjectHierarchyDeltaFlag::PrefabDelta);

	CoreSerializationContext serializationContext;
	serializationContext.IsGameObjectDeserializationActive = true;
	serializationContext.PreserveGameObjectIds = !isPrefabDelta;
	serializationContext.GameObjectCollection = gameObjectCollection;
	// TODO - Should pass a flag to the RTTI system here, so individual RTTIType implementations can gracefully handle delta apply (rather than thinking it is deserialization)

	serializationContext.GameObjectCollection->BeginHandleResolve();

	FrameScope frameScope;

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
	Vector<HSceneObject> createdSceneObjects;
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

		const SPtr<SceneObject> newSceneObject = B3DRTTICast<SceneObject>(deltaObject->Data->Decode(&serializationContext));
		if(!B3D_ENSURE(newSceneObject != nullptr))
			continue;

		newSceneObject->SetPrefabObjectId(deltaObject->PrefabObjectId);
		newSceneObject->SetPrefabResourceId(deltaObject->PrefabResourceId);

		createdSceneObjects.push_back(newSceneObject->GetHandle());
	}

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

		HSceneObject parentSceneObject = B3DStaticGameObjectCast<SceneObject>(gameObjectCollection->GetObject(deltaObject->ParentId));
		if(!B3D_ENSURE(parentSceneObject.IsValid()))
			continue;

		const SPtr<Component> newComponent = B3DRTTICast<Component>(deltaObject->Data->Decode(&serializationContext));
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

		if(!B3D_ENSURE(deltaObject->Data != nullptr)) // TODO - If there is only a parent change, this might be null
			continue;

		const UUID& gameObjectId = entry.first;
		const bool isSceneObject = deltaObject->Data->GetRootTypeId() == SceneObject::GetRttiStatic()->GetRttiId();
		if(isSceneObject)
		{
			HSceneObject sceneObject = B3DStaticGameObjectCast<SceneObject>(gameObjectCollection->GetObject(gameObjectId));
			if(!B3D_ENSURE(sceneObject.IsValid()))
				return;

			if(deltaObject->Flags.IsSet(GameObjectDeltaFlag::ParentChanged))
			{
				HSceneObject parentSceneObject = B3DStaticGameObjectCast<SceneObject>(gameObjectCollection->GetObject(deltaObject->ParentId));
				if(B3D_ENSURE(parentSceneObject.IsValid()))
					sceneObject->SetParent(parentSceneObject, false);
			}

			const bool isNewObject = addedSceneObjectsMap.find(gameObjectId) != addedSceneObjectsMap.end();
			if(!isNewObject && deltaObject->Data != nullptr)
			{
				IDeltaHandler& deltaHandler = sceneObject->GetRtti()->GetDeltaHandler();
				deltaHandler.ApplyDelta(sceneObject.GetShared(), deltaObject->Data, &serializationContext);
			}
		}
		else // Component
		{
			HComponent component = B3DStaticGameObjectCast<Component>(gameObjectCollection->GetObject(gameObjectId));
			if(!B3D_ENSURE(component.IsValid()))
				return;

			const bool isNewObject = addedComponentsMap.find(gameObjectId) != addedComponentsMap.end();
			if(!isNewObject && deltaObject->Data != nullptr)
			{
				IDeltaHandler& deltaHandler = component->GetRtti()->GetDeltaHandler();
				deltaHandler.ApplyDelta(component.GetShared(), deltaObject->Data, &serializationContext);
			}
		}
	}

	// Instantiate if the object we're applying the delta to is also instantiated
	if(original->IsInstantiated())
	{
		for(const auto& entry : createdSceneObjects)
		{
			if(!entry->IsInstantiated())
				entry->InstantiateInternal();
		}
	}

	// Remove components and scene objects
	for(const auto& componentId : RemovedComponents)
	{
		HComponent component = B3DStaticGameObjectCast<Component>(gameObjectCollection->GetObject(componentId));
		if(!B3D_ENSURE(component.IsValid()))
			continue;

		component->Destroy(true);
	}

	FrameUnorderedSet<UUID> destroyedObjects;
	for(const auto& sceneObjectId : RemovedSceneObjects)
	{
		HSceneObject sceneObject = B3DStaticGameObjectCast<SceneObject>(gameObjectCollection->GetObject(sceneObjectId));
		if(!B3D_ENSURE(sceneObject.IsValid()))
			continue;

		sceneObject->Destroy(true);
	}

	serializationContext.GameObjectCollection->EndHandleResolve();
}

void SceneObjectHierarchyDelta::GenerateHierarchyDelta(const HSceneObject& original, const HSceneObject& modified, SerializationContext* context, SceneObjectHierarchyDeltaFlags flags, SPtr<SceneObjectHierarchyDelta>& outDelta)
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

			if(originalChild->HasFlag(SOF_DontSave))
				continue;

			// Process children first
			fnFindModifiedAndRemovedChildren(originalChild, false, fnFindModifiedAndRemovedChildren);

			UUID modifiedCompareId = originalChild.GetId();
			if(auto found = modifiedPrefabToInstanceIdMap.find(originalChild.GetId()); found != modifiedPrefabToInstanceIdMap.end())
				modifiedCompareId = found->second;

			HSceneObject modifiedChild = B3DStaticGameObjectCast<SceneObject>(modifiedGameObjectCollection->GetObject(modifiedCompareId));
			if(!modifiedChild.IsValid())
				outDelta->RemovedSceneObjects.push_back(originalChild.GetId());
			else
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

			if(modifiedChild->HasFlag(SOF_DontSave))
				continue;

			// Process children first
			fnFindAddedChildren(modifiedChild, fnFindAddedChildren);

			UUID originalCompareId = GetPrefabOrInstanceId(modifiedChild, isPrefabDelta);
			HSceneObject originalChild = B3DStaticGameObjectCast<SceneObject>(originalGameObjectCollection->GetObject(originalCompareId));
			if(!originalChild.IsValid())
			{
				GenerateSceneObjectDelta(HSceneObject(), modifiedChild, context, flags, false, outDelta);

				outDelta->AddedSceneObjects.push_back(modifiedChild.GetId());
			}
		}
	};

	fnFindModifiedAndRemovedChildren(original, true, fnFindModifiedAndRemovedChildren);
	fnFindAddedChildren(modified, fnFindAddedChildren);
}

bool SceneObjectHierarchyDelta::GenerateSceneObjectDelta(const HSceneObject& original, const HSceneObject& modified, SerializationContext* context, SceneObjectHierarchyDeltaFlags flags, bool ignoreParent, SPtr<SceneObjectHierarchyDelta>& outDelta)
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
		outDelta->RemovedSceneObjects.push_back(modified.GetId());

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

void SceneObjectHierarchyDelta::GenerateComponentDelta(const HSceneObject& original, const HSceneObject& modified, SerializationContext* context, SceneObjectHierarchyDeltaFlags flags, SPtr<SceneObjectHierarchyDelta>& outDelta)
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
			SPtr<SerializedObject> serializedModified = SerializedObject::Create(*modifiedComponent, SerializedObjectEncodeFlag::IsDeltaCopy);
			SPtr<SceneObjectHierarchyDeltaObject> sceneDeltaObject = B3DMakeShared<SceneObjectHierarchyDeltaObject>(modifiedComponent, serializedModified);
			fnEnsureOutputObjectIsValid();

			const auto result = outDelta->Objects.insert(std::make_pair(sceneDeltaObject->Id, sceneDeltaObject));
			if(result.second)
				outDelta->AddedComponents.push_back(sceneDeltaObject->Id);
		}
	}
}

RTTITypeBase* SceneObjectHierarchyDelta::GetRttiStatic()
{
	return SceneObjectHierarchyDeltaRTTI::Instance();
}

RTTITypeBase* SceneObjectHierarchyDelta::GetRtti() const
{
	return GetRttiStatic();
}
