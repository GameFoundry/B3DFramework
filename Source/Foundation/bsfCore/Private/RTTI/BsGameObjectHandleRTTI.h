//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Scene/BsGameObjectHandle.h"
#include "Scene/BsGameObjectManager.h"
#include "Utility/BsUtility.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT GameObjectHandleRTTI : public RTTIType<GameObjectHandleBase, IReflectable, GameObjectHandleRTTI>
	{
	private:
		u64& GetInstanceId(GameObjectHandleBase* obj)
		{
			static u64 invalidId = 0;

			if(obj->mData->MPtr != nullptr)
				return obj->mData->MPtr->MInstanceId;

			return invalidId;
		}

		void SetInstanceId(GameObjectHandleBase* obj, u64& value) { mOriginalInstanceId = value; }

	public:
		GameObjectHandleRTTI()
		{
			AddPlainField("mInstanceID", 0, &GameObjectHandleRTTI::GetInstanceId, &GameObjectHandleRTTI::SetInstanceId);
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context)
		{
			if(context == nullptr || !rtti_is_of_type<CoreSerializationContext>(context))
				return;

			auto coreContext = static_cast<CoreSerializationContext*>(context);
			if(coreContext->GoState)
			{
				GameObjectHandleBase* gameObjectHandle = static_cast<GameObjectHandleBase*>(obj);
				coreContext->GoState->RegisterUnresolvedHandle(mOriginalInstanceId, *gameObjectHandle);
			}
		}

		const String& GetRttiName()
		{
			static String name = "GameObjectHandleBase";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_GameObjectHandleBase;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			SPtr<GameObjectHandleBase> obj = B3DMakeSharedFromExisting<GameObjectHandleBase>(new(B3DAllocate<GameObjectHandleBase>()) GameObjectHandleBase());

			return obj;
		}

	private:
		u64 mOriginalInstanceId;
	};

	/** @} */
	/** @endcond */
} // namespace bs
