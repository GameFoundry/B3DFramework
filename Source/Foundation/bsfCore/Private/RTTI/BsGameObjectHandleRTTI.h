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
		UINT64& getInstanceId(GameObjectHandleBase* obj)
		{
			static UINT64 invalidId = 0;

			if (obj->mData->mPtr != nullptr)
				return obj->mData->mPtr->mInstanceId;

			return invalidId;
		}

		void setInstanceId(GameObjectHandleBase* obj, UINT64& value) { mOriginalInstanceId = value; }

	public:
		GameObjectHandleRTTI()
		{
			addPlainField("mInstanceID", 0, &GameObjectHandleRTTI::getInstanceId, &GameObjectHandleRTTI::setInstanceId);
		}

		void onDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			if(context == nullptr || !rtti_is_of_type<CoreSerializationContext>(context))
				return;

			auto coreContext = static_cast<CoreSerializationContext*>(context);
			if(coreContext->goState)
			{
				GameObjectHandleBase* gameObjectHandle = static_cast<GameObjectHandleBase*>(obj);
				coreContext->goState->registerUnresolvedHandle(mOriginalInstanceId, *gameObjectHandle);
			}
		}

		const String& getRTTIName() override
		{
			static String name = "GameObjectHandleBase";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_GameObjectHandleBase;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			SPtr<GameObjectHandleBase> obj = bs_shared_ptr<GameObjectHandleBase>(new (bs_alloc<GameObjectHandleBase>()) GameObjectHandleBase());

			return obj;
		}

	private:
		UINT64 mOriginalInstanceId;
	};

	/** @} */
	/** @endcond */
}
