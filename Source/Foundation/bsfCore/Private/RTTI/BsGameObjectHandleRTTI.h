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
		UUID& GetId(GameObjectHandleBase* object)
		{
			if(!mRemappedId.Empty())
				return mRemappedId;

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
			//AddPlainField("mInstanceID", 0, &GameObjectHandleRTTI::GetInstanceId, &GameObjectHandleRTTI::SetInstanceId);
			AddPlainField("mId", 1, &GameObjectHandleRTTI::GetId, &GameObjectHandleRTTI::SetId);
		}

		void OnDeserializationEnded(IReflectable* object, SerializationContext* context)
		{
			CoreSerializationContext* const serializationContext = B3DRTTICast<CoreSerializationContext>(context);
			if(serializationContext == nullptr)
				return;

			GameObjectHandleBase* gameObjectHandle = static_cast<GameObjectHandleBase*>(object);
			if(serializationContext->GameObjectCollection != nullptr)
				serializationContext->GameObjectCollection->RegisterUnresolvedHandle(*gameObjectHandle);
		}

		void OnSerializationStarted(IReflectable* object, SerializationContext* context) override
		{
			CoreSerializationContext* const serializationContext = B3DRTTICast<CoreSerializationContext>(context);
			if(serializationContext == nullptr)
				return;

			GameObjectHandleBase* gameObjectHandle = static_cast<GameObjectHandleBase*>(object);
			if(auto found = serializationContext->GameObjectIdRemapping.find(gameObjectHandle->GetId()); found != serializationContext->GameObjectIdRemapping.end())
				mRemappedId = found->second;
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

		UUID mRemappedId;
	};

	/** @} */
	/** @endcond */
} // namespace bs
