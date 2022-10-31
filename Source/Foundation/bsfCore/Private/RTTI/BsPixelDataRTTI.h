//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Image/BsPixelData.h"
#include "FileSystem/BsDataStream.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT PixelDataRTTI : public RTTIType<PixelData, GpuResourceData, PixelDataRTTI>
	{
		static u32 VERSION;

		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(left, mExtents.Left, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(top, mExtents.Top, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(right, mExtents.Right, 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(bottom, mExtents.Bottom, 3)
			BS_RTTI_MEMBER_PLAIN_NAMED(front, mExtents.Front, 4)
			BS_RTTI_MEMBER_PLAIN_NAMED(back, mExtents.Back, 5)
			BS_RTTI_MEMBER_PLAIN(mRowPitch, 6)
			BS_RTTI_MEMBER_PLAIN(mSlicePitch, 7)
			BS_RTTI_MEMBER_PLAIN(mFormat, 8)
		BS_END_RTTI_MEMBERS

		u32& GetVersion(PixelData* obj) { return VERSION; }

		void SetVersion(PixelData* obj, u32& val) { mVersion = val; }

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
			AddPlainField("version", 10, &PixelDataRTTI::GetVersion, &PixelDataRTTI::SetVersion);
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context)
		{
			PixelData* pixelData = static_cast<PixelData*>(obj);

			// Convert row & slice pitch from pixels to bytes, in case pixel data was stored with an older version
			if(mVersion == 0)
			{
				u32 pixelSize = PixelUtil::GetNumElemBytes(pixelData->GetFormat());
				pixelData->mRowPitch *= pixelSize;
				pixelData->mSlicePitch *= pixelSize;
			}
		}

		const String& GetRttiName()
		{
			static String name = "PixelData";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_PixelData;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			SPtr<PixelData> newPixelData = B3DMakeShared<PixelData>();

			return newPixelData;
		}

	private:
		u32 mVersion = 0;
	};

	u32 PixelDataRTTI::VERSION = 1;

	/** @} */
	/** @endcond */
} // namespace bs
