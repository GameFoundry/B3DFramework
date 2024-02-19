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

RTTITypeBase* ComponentDelta::GetRttiStatic()
{
	return ComponentDeltaRTTI::Instance();
}

RTTITypeBase* ComponentDelta::GetRtti() const
{
	return GetRttiStatic();
}

RTTITypeBase* SceneObjectDelta::GetRttiStatic()
{
	return SceneObjectDeltaRTTI::Instance();
}

RTTITypeBase* SceneObjectDelta::GetRtti() const
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

	SPtr<SceneObjectHierarchyDelta> output = B3DMakeShared<SceneObjectHierarchyDelta>();
	output->mRoot = GenerateDelta(original, modified, flags);

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

void SceneObjectHierarchyDelta::ApplyDiff(const SPtr<SceneObjectDelta>& delta, const HSceneObject& original, SerializationContext* context)
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

SPtr<SceneObjectDelta> SceneObjectHierarchyDelta::GenerateDelta(const HSceneObject& original, const HSceneObject& modified, SceneObjectHierarchyDeltaFlags flags)
{
	SPtr<SceneObjectDelta> output;

	if(original->GetName() != modified->GetName())
	{
		if(output == nullptr)
			output = B3DMakeShared<SceneObjectDelta>();

		output->Name = modified->GetName();
		output->SoFlags |= (u32)SceneObjectDiffFlags::Name;
	}

	const Transform& prefabTfrm = original->GetLocalTransform();
	const Transform& instanceTfrm = modified->GetLocalTransform();
	if(prefabTfrm.GetPosition() != instanceTfrm.GetPosition())
	{
		if(output == nullptr)
			output = B3DMakeShared<SceneObjectDelta>();

		output->Position = instanceTfrm.GetPosition();
		output->SoFlags |= (u32)SceneObjectDiffFlags::Position;
	}

	if(prefabTfrm.GetRotation() != instanceTfrm.GetRotation())
	{
		if(output == nullptr)
			output = B3DMakeShared<SceneObjectDelta>();

		output->Rotation = instanceTfrm.GetRotation();
		output->SoFlags |= (u32)SceneObjectDiffFlags::Rotation;
	}

	if(prefabTfrm.GetScale() != instanceTfrm.GetScale())
	{
		if(output == nullptr)
			output = B3DMakeShared<SceneObjectDelta>();

		output->Scale = instanceTfrm.GetScale();
		output->SoFlags |= (u32)SceneObjectDiffFlags::Scale;
	}

	if(original->GetActive() != modified->GetActive())
	{
		if(output == nullptr)
			output = B3DMakeShared<SceneObjectDelta>();

		output->IsActive = modified->GetActive();
		output->SoFlags |= (u32)SceneObjectDiffFlags::Active;
	}

	u32 prefabChildCount = original->GetChildCount();
	u32 instanceChildCount = modified->GetChildCount();

	// Find modified and removed children
	for(u32 i = 0; i < prefabChildCount; i++)
	{
		HSceneObject prefabChild = original->GetChild(i);

		SPtr<SceneObjectDelta> childDiff;
		bool foundMatching = false;
		for(u32 j = 0; j < instanceChildCount; j++)
		{
			HSceneObject instanceChild = modified->GetChild(j);

			if(prefabChild->GetId() == instanceChild->GetPrefabObjectId())
			{
				if(!instanceChild->IsPrefabInstanceRoot())
					childDiff = GenerateDelta(prefabChild, instanceChild, flags);

				foundMatching = true;
				break;
			}
		}

		if(foundMatching)
		{
			if(childDiff != nullptr)
			{
				if(output == nullptr)
					output = B3DMakeShared<SceneObjectDelta>();

				output->ChildDeltas.push_back(childDiff);
			}
		}
		else
		{
			if(output == nullptr)
				output = B3DMakeShared<SceneObjectDelta>();

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
				output = B3DMakeShared<SceneObjectDelta>();

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
					output = B3DMakeShared<SceneObjectDelta>();

				output->ComponentDeltas.push_back(childDiff);
			}
		}
		else
		{
			if(output == nullptr)
				output = B3DMakeShared<SceneObjectDelta>();

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
				output = B3DMakeShared<SceneObjectDelta>();

			output->AddedComponents.push_back(obj);
		}
	}

	if(output != nullptr)
		output->Id = modified->GetPrefabObjectId();

	return output;
}

RTTITypeBase* SceneObjectHierarchyDelta::GetRttiStatic()
{
	return SceneObjectHierarchyDeltaRTTI::Instance();
}

RTTITypeBase* SceneObjectHierarchyDelta::GetRtti() const
{
	return GetRttiStatic();
}
