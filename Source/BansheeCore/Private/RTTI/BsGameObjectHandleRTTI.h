//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Scene/BsGameObjectHandle.h"
#include "Scene/BsGameObjectManager.h"

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

		void setInstanceId(GameObjectHandleBase* obj, UINT64& value) { obj->mRTTIData = value; } 

	public:
		GameObjectHandleRTTI()
		{
			addPlainField("mInstanceID", 0, &GameObjectHandleRTTI::getInstanceId, &GameObjectHandleRTTI::setInstanceId);
		}

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			GameObjectHandleBase* gameObjectHandle = static_cast<GameObjectHandleBase*>(obj);

			UINT64 originalInstanceId = any_cast<UINT64>(gameObjectHandle->mRTTIData);
			GameObjectManager::instance().registerUnresolvedHandle(originalInstanceId, *gameObjectHandle);
			gameObjectHandle->mRTTIData = nullptr;
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
	};

	/** @} */
	/** @endcond */
}