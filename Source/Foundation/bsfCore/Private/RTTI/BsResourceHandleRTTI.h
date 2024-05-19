//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "RTTI/BsUUIDRTTI.h"
#include "Resources/BsResourceHandle.h"
#include "Resources/BsResources.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT ResourceHandleRTTI : public RTTIType<TResourceHandleBase<false>, IReflectable, ResourceHandleRTTI>
	{
	private:
		UUID& GetUuid(TResourceHandleBase<false>* obj) { return obj->mData != nullptr ? obj->mData->MUuid : UUID::kEmpty; }

		void SetUuid(TResourceHandleBase<false>* obj, UUID& uuid) { obj->mData->MUuid = uuid; }

	public:
		ResourceHandleRTTI()
		{
			AddPlainField("mUUID", 0, &ResourceHandleRTTI::GetUuid, &ResourceHandleRTTI::SetUuid);
		}

		void OnDeserializationEnded(IReflectable* obj, RTTIOperationContext* context)
		{
			TResourceHandleBase<false>* resourceHandle = static_cast<TResourceHandleBase<false>*>(obj);

			if(resourceHandle->mData && !resourceHandle->mData->MUuid.Empty())
			{
				HResource loadedResource = GetResources().GetResourceHandleInternal(resourceHandle->mData->MUuid);

				resourceHandle->ReleaseRef();
				resourceHandle->mData = loadedResource.mData;
				resourceHandle->AddRef();
			}
		}

		const String& GetRttiName()
		{
			static String name = "ResourceHandleBase";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ResourceHandle;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			SPtr<TResourceHandleBase<false>> obj = B3DMakeSharedFromExisting<TResourceHandleBase<false>>(new(B3DAllocate<TResourceHandleBase<false>>()) TResourceHandleBase<false>());
			obj->mData = B3DMakeShared<ResourceHandleData>();
			obj->mData->MRefCount.fetch_add(1, std::memory_order_relaxed);

			return obj;
		}
	};

	class B3D_CORE_EXPORT WeakResourceHandleRTTI : public RTTIType<TResourceHandleBase<true>, IReflectable, WeakResourceHandleRTTI>
	{
	private:
		UUID& GetUuid(TResourceHandleBase<true>* obj) { return obj->mData != nullptr ? obj->mData->MUuid : UUID::kEmpty; }

		void SetUuid(TResourceHandleBase<true>* obj, UUID& uuid) { obj->mData->MUuid = uuid; }

	public:
		WeakResourceHandleRTTI()
		{
			AddPlainField("mUUID", 0, &WeakResourceHandleRTTI::GetUuid, &WeakResourceHandleRTTI::SetUuid);
		}

		void OnDeserializationEnded(IReflectable* obj, RTTIOperationContext* context) override
		{
			TResourceHandleBase<true>* resourceHandle = static_cast<TResourceHandleBase<true>*>(obj);

			if(resourceHandle->mData && !resourceHandle->mData->MUuid.Empty())
			{
				HResource loadedResource = GetResources().GetResourceHandleInternal(resourceHandle->mData->MUuid);
				resourceHandle->mData = loadedResource.mData;
			}
		}

		const String& GetRttiName() override
		{
			static String name = "WeakResourceHandleBase";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_WeakResourceHandle;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			SPtr<TResourceHandleBase<true>> obj = B3DMakeSharedFromExisting<TResourceHandleBase<true>>(new(B3DAllocate<TResourceHandleBase<true>>()) TResourceHandleBase<true>());
			obj->mData = B3DMakeShared<ResourceHandleData>();

			return obj;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
