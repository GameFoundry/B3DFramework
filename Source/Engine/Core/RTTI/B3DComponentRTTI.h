//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Scene/B3DComponent.h"
#include "RTTI/B3DGameObjectRTTI.h"
#include "Scene/B3DGameObjectCollection.h"
#include "Utility/B3DUtility.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT ComponentRTTI : public TRTTIType<Component, GameObject, ComponentRTTI>
	{
	public:
		void OnOperationStarted(Component& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(!operationType.IsSet(RTTIOperationType::WriteBit))
				return;

			if(object.mRTTIData.Empty())
				return;

			auto* const serializationContext = context.As<RTTIOperationEngineContext>();
			if(serializationContext == nullptr)
				return;

			if(serializationContext->GameObjectCollection != nullptr && serializationContext->CurrentSceneObjectEntity != ecs::kNullEntity)
			{
				object.mECSRegistry = &serializationContext->GameObjectCollection->GetECSRegistry();
				object.mECSEntity = serializationContext->CurrentSceneObjectEntity;
			}
		}

		void OnOperationEnded(Component& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(!operationType.IsSet(RTTIOperationType::WriteBit))
				return;

			// It's possible we're just accessing the game object fields, in which case the process below is not needed
			// (it's only required for new components).
			if(object.mRTTIData.Empty())
				return;

			auto* const serializationContext = context.As<RTTIOperationEngineContext>();
			B3D_ASSERT(serializationContext != nullptr);

			if(object.mId.Empty() || !serializationContext->PreserveGameObjectIds)
			{
				const UUID oldId = object.GetId();
				object.mId = UUIDGenerator::GenerateRandom();

				if(!oldId.Empty())
				{
					if(B3D_ENSURE(serializationContext->GameObjectCollection != nullptr))
						serializationContext->GameObjectCollection->RegisterUnresolvedHandleIdRemapping(oldId, object.GetId());
				}
			}

			GODeserializationData& deserializationData = AnyCastRef<GODeserializationData>(object.mRTTIData);

			// This shouldn't be null during normal deserialization but could be during some other operations, like applying
			// a binary diff.
			if(deserializationData.Ptr != nullptr)
			{
				// Register the newly created SO with the GameObjectManager and provide it with the original ID so that
				// deserialized handles pointing to this object can be resolved.
				TShared<Component> componentShared = std::static_pointer_cast<Component>(deserializationData.Ptr);

				if(B3D_ENSURE(serializationContext->GameObjectCollection != nullptr))
					GameObjectHandle handle = serializationContext->GameObjectCollection->RegisterNewObject(componentShared);
			}

			object.mRTTIData = nullptr;
		}

		const String& GetRttiName() override
		{
			static String name = "Component";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Component;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			B3D_ASSERT(false && "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
