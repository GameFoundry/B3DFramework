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
		static UINT32 VERSION;

		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(left, mExtents.left, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(top, mExtents.top, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(right, mExtents.right, 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(bottom, mExtents.bottom, 3)
			BS_RTTI_MEMBER_PLAIN_NAMED(front, mExtents.front, 4)
			BS_RTTI_MEMBER_PLAIN_NAMED(back, mExtents.back, 5)
			BS_RTTI_MEMBER_PLAIN(mRowPitch, 6)
			BS_RTTI_MEMBER_PLAIN(mSlicePitch, 7)
			BS_RTTI_MEMBER_PLAIN(mFormat, 8)
		BS_END_RTTI_MEMBERS

		UINT32& GetVersion(PixelData* obj) { return VERSION; }
		void SetVersion(PixelData* obj, UINT32& val) { mVersion = val; }

		SPtr<DataStream> GetData(PixelData* obj, UINT32& size)
		{
			size = obj->GetConsecutiveSize();

			return bs_shared_ptr_new<MemoryDataStream>(obj->GetData(), size);
		}

		void SetData(PixelData* obj, const SPtr<DataStream>& value, UINT32 size)
		{
			obj->AllocateInternalBuffer(size);
			value->Read(obj->getData(), size);
		}
		
	public:
		PixelDataRTTI()
		{
			addDataBlockField("data", 9, &PixelDataRTTI::getData, &PixelDataRTTI::setData);
			addPlainField("version", 10, &PixelDataRTTI::getVersion, &PixelDataRTTI::setVersion);
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			PixelData* pixelData = static_cast<PixelData*>(obj);

			// Convert row & slice pitch from pixels to bytes, in case pixel data was stored with an older version
			if(mVersion == 0)
			{
				UINT32 pixelSize = PixelUtil::getNumElemBytes(pixelData->GetFormat());
				pixelData->mRowPitch *= pixelSize;
				pixelData->mSlicePitch *= pixelSize;
			}
		}

		const String& GetRTTIName() override
		{
			static String name = "PixelData";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_PixelData;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			SPtr<PixelData> newPixelData = bs_shared_ptr_new<PixelData>();

			return newPixelData;
		}

	private:
		UINT32 mVersion = 0;
	};

	UINT32 PixelDataRTTI::VERSION = 1;

	/** @} */
	/** @endcond */
}
