//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "RTTI/B3DUUIDRTTI.h"
#include "Resources/B3DResourceHandle.h"
#include "Resources/B3DResources.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT StrongResourceHandleRTTI : public TRTTIType<StrongResourceHandle, IReflectable, StrongResourceHandleRTTI>
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
				if(!mId.Empty())
				{
					B3D_ENSURE(object.mData == nullptr); // Has to be empty, otherwise we'll leak the handle data

					HResource loadedResource = GetResources().GetOrCreateResourceHandle(mId);
					object.mData = loadedResource.mData;
					object.IncrementReferenceCount();
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
			return B3DMakeSharedFromExisting<StrongResourceHandle>(new(B3DAllocate<StrongResourceHandle>()) StrongResourceHandle());
		}
	};

	class B3D_EXPORT WeakResourceHandleRTTI : public TRTTIType<WeakResourceHandle, IReflectable, WeakResourceHandleRTTI>
	{
		UUID mId;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_GENERATED_MEMBER(mId, 0)
		B3D_RTTI_END_MEMBERS
	public:
		void OnOperationStarted(WeakResourceHandle& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				mId = object.mData != nullptr ? object.mData->Id : UUID::kEmpty;
			}
		}

		void OnOperationEnded(WeakResourceHandle& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				if(!mId.Empty())
				{
					B3D_ENSURE(object.mData == nullptr); // Has to be empty, otherwise we'll leak the handle data

					HResource loadedResource = GetResources().GetOrCreateResourceHandle(mId);
					object.mData = loadedResource.mData;
					object.IncrementReferenceCount();
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
			return B3DMakeSharedFromExisting<WeakResourceHandle>(new(B3DAllocate<WeakResourceHandle>()) WeakResourceHandle());
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
