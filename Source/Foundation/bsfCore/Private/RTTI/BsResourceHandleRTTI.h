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

	class B3D_CORE_EXPORT ResourceHandleRTTI : public TRTTIType<TResourceHandleBase<false>, IReflectable, ResourceHandleRTTI>
	{
		UUID mId;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_GENERATED_MEMBER(mId, 0)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(TResourceHandleBase<false>& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				mId = object.mData != nullptr ? object.mData->MUuid : UUID::kEmpty;
			}
		}

		void OnOperationEnded(TResourceHandleBase<false>& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				if(object.mData != nullptr)
				{
					object.mData->MUuid = mId;

					if(!object.mData->MUuid.Empty())
					{
						HResource loadedResource = GetResources().GetOrCreateResourceHandle(object.mData->MUuid);

						object.ReleaseRef();
						object.mData = loadedResource.mData;
						object.AddRef();
					}
				}
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

	class B3D_CORE_EXPORT WeakResourceHandleRTTI : public TRTTIType<TResourceHandleBase<true>, IReflectable, WeakResourceHandleRTTI>
	{
		UUID mId;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_GENERATED_MEMBER(mId, 0)
		B3D_RTTI_END_MEMBERS
	public:
		void OnOperationStarted(TResourceHandleBase<true>& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				mId = object.mData != nullptr ? object.mData->MUuid : UUID::kEmpty;
			}
		}

		void OnOperationEnded(TResourceHandleBase<true>& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				if(object.mData != nullptr)
				{
					object.mData->MUuid = mId;

					if(!object.mData->MUuid.Empty())
					{
						HResource loadedResource = GetResources().GetOrCreateResourceHandle(object.mData->MUuid);
						object.mData = loadedResource.mData;
					}
				}
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
