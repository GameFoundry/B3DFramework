//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Image/BsPixelData.h"
#include "Reflection/BsRTTIType.h"
#include "FileSystem/BsDataStream.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT PixelDataRTTI : public RTTIType<PixelData, GpuResourceData, PixelDataRTTI>
	{
		UINT32& getLeft(PixelData* obj) { return obj->mExtents.left; }
		void setLeft(PixelData* obj, UINT32& val) { obj->mExtents.left = val; }

		UINT32& getTop(PixelData* obj) { return obj->mExtents.top; }
		void setTop(PixelData* obj, UINT32& val) { obj->mExtents.top = val; }

		UINT32& getRight(PixelData* obj) { return obj->mExtents.right; }
		void setRight(PixelData* obj, UINT32& val) { obj->mExtents.right = val; }

		UINT32& getBottom(PixelData* obj) { return obj->mExtents.bottom; }
		void setBottom(PixelData* obj, UINT32& val) { obj->mExtents.bottom = val; }

		UINT32& getFront(PixelData* obj) { return obj->mExtents.front; }
		void setFront(PixelData* obj, UINT32& val) { obj->mExtents.front = val; }

		UINT32& getBack(PixelData* obj) { return obj->mExtents.back; }
		void setBack(PixelData* obj, UINT32& val) { obj->mExtents.back = val; }

		UINT32& getRowPitch(PixelData* obj) { return obj->mRowPitch; }
		void setRowPitch(PixelData* obj, UINT32& val) { obj->mRowPitch = val; }

		UINT32& getSlicePitch(PixelData* obj) { return obj->mSlicePitch; }
		void setSlicePitch(PixelData* obj, UINT32& val) { obj->mSlicePitch = val; }

		PixelFormat& getFormat(PixelData* obj) { return obj->mFormat; }
		void setFormat(PixelData* obj, PixelFormat& val) { obj->mFormat = val; }

		SPtr<DataStream> getData(PixelData* obj, UINT32& size)
		{
			size = obj->getConsecutiveSize();

			return bs_shared_ptr_new<MemoryDataStream>(obj->getData(), size, false);
		}

		void setData(PixelData* obj, const SPtr<DataStream>& value, UINT32 size)
		{
			obj->allocateInternalBuffer(size);
			value->read(obj->getData(), size);
		}
		
	public:
		PixelDataRTTI()
		{
			addPlainField("left", 0, &PixelDataRTTI::getLeft, &PixelDataRTTI::setLeft);
			addPlainField("top", 1, &PixelDataRTTI::getTop, &PixelDataRTTI::setTop);
			addPlainField("right", 2, &PixelDataRTTI::getRight, &PixelDataRTTI::setRight);
			addPlainField("bottom", 3, &PixelDataRTTI::getBottom, &PixelDataRTTI::setBottom);
			addPlainField("front", 4, &PixelDataRTTI::getFront, &PixelDataRTTI::setFront);
			addPlainField("back", 5, &PixelDataRTTI::getBack, &PixelDataRTTI::setBack);
			addPlainField("rowPitch", 6, &PixelDataRTTI::getRowPitch, &PixelDataRTTI::setRowPitch);
			addPlainField("slicePitch", 7, &PixelDataRTTI::getSlicePitch, &PixelDataRTTI::setSlicePitch);
			addPlainField("format", 8, &PixelDataRTTI::getFormat, &PixelDataRTTI::setFormat);
			addDataBlockField("data", 9, &PixelDataRTTI::getData, &PixelDataRTTI::setData, 0);
		}

		virtual const String& getRTTIName() override
		{
			static String name = "PixelData";
			return name;
		}

		virtual UINT32 getRTTIId() override
		{
			return TID_PixelData;
		}

		virtual SPtr<IReflectable> newRTTIObject() override
		{
			SPtr<PixelData> newPixelData = bs_shared_ptr_new<PixelData>();

			return newPixelData;
		}
	};

	/** @} */
	/** @endcond */
}