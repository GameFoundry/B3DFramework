//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIECSField.h"
#include "Scene/B3DSceneObject.h"
#include "Scene/B3DSceneObjectFragments.h"
#include "Scene/B3DGameObjectHandle.h"
#include "Scene/B3DGameObjectManager.h"
#include "Scene/B3DComponent.h"
#include "RTTI/B3DGameObjectRTTI.h"
#include "RTTI/B3DSceneObjectFragmentsRTTI.h"
#include "Scene/B3DGameObjectCollection.h"
#include "Scene/B3DSceneObjectHierarchyDelta.h"
#include "Utility/B3DUtility.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT SceneObjectRTTI : public TRTTIType<SceneObject, GameObject, SceneObjectRTTI>
	{
		Vector<SPtr<SceneObject>> mChildren;
		Vector<SPtr<Component>> mComponents;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_GENERATED_MEMBER_CONTAINER(mChildren, 0)
			B3D_RTTI_GENERATED_MEMBER_CONTAINER(mComponents, 1)
			B3D_RTTI_MEMBER(mFlags, 3)
			B3D_RTTI_MEMBER(mPrefabDelta, 4)
			B3D_RTTI_MEMBER(mPrefabVersion, 5)
			B3D_RTTI_MEMBER_ECS(WorldTransform, 11)
			B3D_RTTI_MEMBER_ECS(LocalTransform, 12)
			B3D_RTTI_MEMBER_INFO(mPrefabResourceId, 13, RTTIFieldInfo(RTTIFieldFlag::SkipInDeltaCompare | RTTIFieldFlag::SkipInDeltaCopy))
			B3D_RTTI_MEMBER_ECS(MobilityTags, 14)
			B3D_RTTI_MEMBER_ECS(HierarchyDepth, 15)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(SceneObject& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				mChildren.clear();
				mChildren.reserve(object.mChildren.size());
				for(const auto& entry : object.mChildren)
					mChildren.push_back(entry.GetShared());

				mComponents.clear();
				mComponents.reserve(object.mComponents.size());
				for(const auto& entry : object.mComponents)
					mComponents.push_back(entry.GetShared());
			}

			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				// If this is the root scene object we're deserializing, activate game object deserialization so the system
				// can resolve deserialized handles to the newly created objects

				// It's possible we're just accessing the game object fields, in which case the process below is not needed
				// (it's only required for new scene objects).
				if(object.mRTTIData.Empty())
					return;

				if(auto* serializationContext = context.As<RTTIOperationEngineContext>())
				{
					if(!serializationContext->IsGameObjectDeserializationActive)
					{
						mIsDeserializationParent = true;
						serializationContext->IsGameObjectDeserializationActive = true;

						if(serializationContext->GameObjectCollection != nullptr)
							serializationContext->GameObjectCollection->BeginHandleResolve();
					}

					// Create ECS entity early so that ECS fragment fields can write to it during deserialization
					if(object.mECSEntity == ecs::kNullEntity && serializationContext->GameObjectCollection != nullptr)
					{
						ecs::Registry& registry = serializationContext->GameObjectCollection->GetECSRegistry();
						object.CreateECSEntity(&registry);
					}
				}
			}
		}

		void OnOperationEnded(SceneObject& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				// It's possible we're just accessing the game object fields, in which case the process below is not needed
				// (it's only required for new scene objects).
				if(object.mRTTIData.Empty())
					return;

				auto* serializationContext = context.As<RTTIOperationEngineContext>();
				GODeserializationData& goDeserializationData = AnyCastRef<GODeserializationData>(object.mRTTIData);

				// Register the newly created SO with the GameObjectManager and provide it with the original ID so that
				// deserialized handles pointing to this object can be resolved.
				SPtr<SceneObject> sceneObjectShared = std::static_pointer_cast<SceneObject>(goDeserializationData.Ptr);

				if(object.mId.Empty() || !serializationContext->PreserveGameObjectIds)
				{
					const UUID oldId = object.mId;
					object.mId = UUIDGenerator::GenerateRandom();

					if(!oldId.Empty())
					{
						if(serializationContext->GameObjectCollection != nullptr)
							serializationContext->GameObjectCollection->RegisterUnresolvedHandleIdRemapping(oldId, object.mId);
					}
				}

				HSceneObject sceneObjectHandle = SceneObject::CreateInternal(serializationContext->GameObjectCollection, sceneObjectShared);

				// We stored all components and children in a temporary structure because they rely on the SceneObject being
				// initialized with the GameObjectManager. Now that it is, we add them.
				for(auto& component : mComponents)
					object.InternalAddComponent(component, false);

				for(auto& child : mChildren)
				{
					if(child != nullptr)
						child->SetParentInternal(object.GetHandle(), false);
				}

				// If this is the deserialization parent, end deserialization (which resolves all game object handles, if we
				// provided valid IDs), and instantiate (i.e. activate) the deserialized hierarchy.
				if(mIsDeserializationParent)
				{
					if(serializationContext->GameObjectCollection != nullptr)
						serializationContext->GameObjectCollection->EndHandleResolve();

					serializationContext->IsGameObjectDeserializationActive = false;

					bool parentActive = true;
					if(object.GetParent() != nullptr)
						parentActive = object.GetParent()->GetActive();

					object.SetActiveHierarchy(parentActive, false);

					if(serializationContext->InitializeNewGameObjects)
						object.Initialize();
				}

				object.mRTTIData = nullptr;
			}
		}

		const String& GetRttiName() override
		{
			static String name = "SceneObject";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SceneObject;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			SPtr<SceneObject> sceneObject = SPtr<SceneObject>(new(B3DAllocate<SceneObject>()) SceneObject("", 0), &B3DDelete<SceneObject>, StdAlloc<SceneObject>());
			sceneObject->mRTTIData = sceneObject;

			return sceneObject;
		}

	private:
		bool mIsDeserializationParent = false;
	};

	/** @} */
	/** @endcond */
} // namespace b3d
