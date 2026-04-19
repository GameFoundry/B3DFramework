//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Image/B3DPixelData.h"
#include "FileSystem/B3DDataStream.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT PixelDataRTTI : public TRTTIType<PixelData, GpuResourceData, PixelDataRTTI>
	{
		static constexpr u32 kVersion = 1;

		u32 mVersion = kVersion;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(left, mExtents.Left, 0)
			B3D_RTTI_MEMBER_NAMED(top, mExtents.Top, 1)
			B3D_RTTI_MEMBER_NAMED(right, mExtents.Right, 2)
			B3D_RTTI_MEMBER_NAMED(bottom, mExtents.Bottom, 3)
			B3D_RTTI_MEMBER_NAMED(front, mExtents.Front, 4)
			B3D_RTTI_MEMBER_NAMED(back, mExtents.Back, 5)
			B3D_RTTI_MEMBER(mRowPitch, 6)
			B3D_RTTI_MEMBER(mSlicePitch, 7)
			B3D_RTTI_MEMBER(mFormat, 8)
			B3D_RTTI_GENERATED_MEMBER(mVersion, 10)
		B3D_RTTI_END_MEMBERS

		SPtr<DataStream> GetData(PixelData* obj, u32& size)
		{
			size = obj->GetConsecutiveSize();

			return B3DMakeShared<MemoryDataStream>(obj->GetData(), size);
		}

		void SetData(PixelData* obj, const SPtr<DataStream>& value, u32 size)
		{
			obj->AllocateInternalBuffer(size);
			value->Read(obj->GetData(), size);
		}

	public:
		PixelDataRTTI()
		{
			AddDataBlockField("data", 9, &PixelDataRTTI::GetData, &PixelDataRTTI::SetData);
		}

		void OnOperationEnded(PixelData& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				// Convert row & slice pitch from pixels to bytes, in case pixel data was stored with an older version
				if(mVersion == 0)
				{
					u32 pixelSize = PixelUtility::GetElementByteCount(object.GetFormat());
					object.mRowPitch *= pixelSize;
					object.mSlicePitch *= pixelSize;
				}
			}
		}

		const String& GetRttiName()
		{
			static String name = "PixelData";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_PixelData;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<PixelData>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
