//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "RTTI/BsUUIDRTTI.h"
#include "Scene/BsGameObjectCollection.h"
#include "Scene/BsGameObjectHandle.h"
#include "Scene/BsGameObjectManager.h"
#include "Utility/BsUtility.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT GameObjectHandleRTTI : public RTTIType<GameObjectHandleBase, IReflectable, GameObjectHandleRTTI>
	{
	private:
		u64& GetInstanceId(GameObjectHandleBase* object)
		{
			static u64 invalidId = 0;

			if(object->mSharedHandleData->InstanceData != nullptr)
				return object->mSharedHandleData->InstanceData->MInstanceId;

			return invalidId;
		}

		void SetInstanceId(GameObjectHandleBase* obj, u64& value) { mOriginalInstanceId = value; }

		UUID& GetId(GameObjectHandleBase* object)
		{
			if(object->mSharedHandleData == nullptr)
				return const_cast<UUID&>(UUID::kEmpty);

			return object->mSharedHandleData->Id;
		}

		void SetId(GameObjectHandleBase* object, UUID& value)
		{
			if(object->mSharedHandleData == nullptr)
				return;

			object->mSharedHandleData->Id = value;
		}

	public:
		GameObjectHandleRTTI()
		{
			AddPlainField("mInstanceID", 0, &GameObjectHandleRTTI::GetInstanceId, &GameObjectHandleRTTI::SetInstanceId);
			AddPlainField("mId", 1, &GameObjectHandleRTTI::GetId, &GameObjectHandleRTTI::SetId);
		}

		void OnDeserializationEnded(IReflectable* object, SerializationContext* context)
		{
			CoreSerializationContext* const serializationContext = B3DRTTICast<CoreSerializationContext>(context);
			if(serializationContext == nullptr)
				return;

			GameObjectHandleBase* gameObjectHandle = static_cast<GameObjectHandleBase*>(object);
			if(serializationContext->GoState)
				serializationContext->GoState->RegisterUnresolvedHandle(mOriginalInstanceId, *gameObjectHandle);

			if(serializationContext->GameObjectCollection != nullptr)
				serializationContext->GameObjectCollection->RegisterUnresolvedHandle(*gameObjectHandle);
		}

		const String& GetRttiName()
		{
			static String name = "GameObjectHandleBase";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_GameObjectHandleBase;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeSharedFromExisting<GameObjectHandleBase>(new(B3DAllocate<GameObjectHandleBase>()) GameObjectHandleBase());
		}

	private:
		u64 mOriginalInstanceId;
	};

	/** @} */
	/** @endcond */
} // namespace bs
