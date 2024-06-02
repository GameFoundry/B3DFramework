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

	class B3D_CORE_EXPORT StrongResourceHandleRTTI : public TRTTIType<StrongResourceHandle, IReflectable, StrongResourceHandleRTTI>
	{
		UUID mId;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_GENERATED_MEMBER(mId, 0)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(StrongResourceHandle& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				mId = object.mData != nullptr ? object.mData->Id : UUID::kEmpty;
			}
		}

		void OnOperationEnded(StrongResourceHandle& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				if(object.mData != nullptr)
				{
					object.mData->Id = mId;

					if(!object.mData->Id.Empty())
					{
						HResource loadedResource = GetResources().GetOrCreateResourceHandle(object.mData->Id);

						object.DecrementReferenceCount();
						object.mData = loadedResource.mData;
						object.IncrementReferenceCount();
					}
				}
			}
		}

		const String& GetRttiName()
		{
			static String name = "StrongResourceHandle";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_StrongResourceHandle;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			SPtr<StrongResourceHandle> obj = B3DMakeSharedFromExisting<StrongResourceHandle>(new(B3DAllocate<StrongResourceHandle>()) StrongResourceHandle());
			obj->mData = B3DMakeShared<ResourceHandleData>();
			obj->mData->ReferenceCount.fetch_add(1, std::memory_order_relaxed);

			return obj;
		}
	};

	class B3D_CORE_EXPORT WeakResourceHandleRTTI : public TRTTIType<WeakResourceHandle2, IReflectable, WeakResourceHandleRTTI>
	{
		UUID mId;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_GENERATED_MEMBER(mId, 0)
		B3D_RTTI_END_MEMBERS
	public:
		void OnOperationStarted(WeakResourceHandle2& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				mId = object.mData != nullptr ? object.mData->Id : UUID::kEmpty;
			}
		}

		void OnOperationEnded(WeakResourceHandle2& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				if(object.mData != nullptr)
				{
					object.mData->Id = mId;

					if(!object.mData->Id.Empty())
					{
						HResource loadedResource = GetResources().GetOrCreateResourceHandle(object.mData->Id);
						object.mData = loadedResource.mData;
					}
				}
			}
		}

		const String& GetRttiName() override
		{
			static String name = "WeakResourceHandle";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_WeakResourceHandle;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			SPtr<WeakResourceHandle2> obj = B3DMakeSharedFromExisting<WeakResourceHandle2>(new(B3DAllocate<WeakResourceHandle2>()) WeakResourceHandle2());
			obj->mData = B3DMakeShared<ResourceHandleData>();

			return obj;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
