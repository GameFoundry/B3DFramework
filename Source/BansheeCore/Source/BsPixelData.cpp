//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPixelData.h"
#include "BsPixelUtil.h"
#include "BsPixelDataRTTI.h"
#include "BsColor.h"
#include "BsDebug.h"

namespace BansheeEngine
{
	PixelData::PixelData(const PixelData& copy)
		:GpuResourceData(copy)
	{
		mFormat = copy.mFormat;
		mRowPitch = copy.mRowPitch;
		mSlicePitch = copy.mSlicePitch;
		mExtents = copy.mExtents;
	}

	PixelData& PixelData::operator=(const PixelData& rhs)
	{
		GpuResourceData::operator= (rhs);

		mFormat = rhs.mFormat;
		mRowPitch = rhs.mRowPitch;
		mSlicePitch = rhs.mSlicePitch;
		mExtents = rhs.mExtents;

		return *this;
	}

	UINT32 PixelData::getConsecutiveSize() const
	{
		return PixelUtil::getMemorySize(getWidth(), getHeight(), getDepth(), mFormat);
	}

	UINT32 PixelData::getSize() const
	{
		return PixelUtil::getMemorySize(mRowPitch, mSlicePitch / mRowPitch, getDepth(), getFormat());
	}

	PixelData PixelData::getSubVolume(const PixelVolume &def) const
	{
		if (PixelUtil::isCompressed(mFormat))
		{
			if (def.left == getLeft() && def.top == getTop() && def.front == getFront() &&
				def.right == getRight() && def.bottom == getBottom() && def.back == getBack())
			{
				// Entire buffer is being queried
				return *this;
			}

			BS_EXCEPT(InvalidParametersException, "Cannot return subvolume of compressed PixelBuffer");
		}

		if (!mExtents.contains(def))
		{
			BS_EXCEPT(InvalidParametersException, "Bounds out of range");
		}

		const size_t elemSize = PixelUtil::getNumElemBytes(mFormat);
		PixelData rval(def.getWidth(), def.getHeight(), def.getDepth(), mFormat);

		rval.setExternalBuffer(((UINT8*)getData()) + ((def.left - getLeft())*elemSize)
			+ ((def.top - getTop())*mRowPitch*elemSize)
			+ ((def.front - getFront())*mSlicePitch*elemSize));

		rval.mRowPitch = mRowPitch;
		rval.mSlicePitch = mSlicePitch;
		rval.mFormat = mFormat;

		return rval;
	}

	Color PixelData::getColorAt(UINT32 x, UINT32 y, UINT32 z) const
	{
		Color cv;

		UINT32 pixelSize = PixelUtil::getNumElemBytes(mFormat);
		UINT32 pixelOffset = pixelSize * (z * mSlicePitch + y * mRowPitch + x);
		PixelUtil::unpackColor(&cv, mFormat, (unsigned char *)getData() + pixelOffset);

		return cv;
	}

	void PixelData::setColorAt(const Color& color, UINT32 x, UINT32 y, UINT32 z)
	{
		UINT32 pixelSize = PixelUtil::getNumElemBytes(mFormat);
		UINT32 pixelOffset = pixelSize * (z * mSlicePitch + y * mRowPitch + x);
		PixelUtil::packColor(color, mFormat, (unsigned char *)getData() + pixelOffset);
	}

	Vector<Color> PixelData::getColors() const
	{
		UINT32 depth = mExtents.getDepth();
		UINT32 height = mExtents.getHeight();
		UINT32 width = mExtents.getWidth();

		UINT32 pixelSize = PixelUtil::getNumElemBytes(mFormat);
		UINT8* data = getData();

		Vector<Color> colors(width * height * depth);
		for (UINT32 z = 0; z < depth; z++)
		{
			UINT32 zArrayIdx = z * width * height;
			UINT32 zDataIdx = z * mSlicePitch * pixelSize;

			for (UINT32 y = 0; y < height; y++)
			{
				UINT32 yArrayIdx = y * width;
				UINT32 yDataIdx = y * mRowPitch * pixelSize;

				for (UINT32 x = 0; x < width; x++)
				{
					UINT32 arrayIdx = x + yArrayIdx + zArrayIdx;
					UINT32 dataIdx = x * pixelSize + yDataIdx + zDataIdx;

					UINT8* dest = data + dataIdx;
					PixelUtil::unpackColor(&colors[arrayIdx], mFormat, dest);
				}
			}
		}

		return colors;
	}

	template<class T>
	void PixelData::setColorsInternal(const T& colors, UINT32 numElements)
	{
		UINT32 depth = mExtents.getDepth();
		UINT32 height = mExtents.getHeight();
		UINT32 width = mExtents.getWidth();

		UINT32 totalNumElements = width * height * depth;
		if (numElements != totalNumElements)
		{
			LOGERR("Unable to set colors, invalid array size.");
			return;
		}

		UINT32 pixelSize = PixelUtil::getNumElemBytes(mFormat);
		UINT8* data = getData();

		for (UINT32 z = 0; z < depth; z++)
		{
			UINT32 zArrayIdx = z * width * height;
			UINT32 zDataIdx = z * mSlicePitch * pixelSize;

			for (UINT32 y = 0; y < height; y++)
			{
				UINT32 yArrayIdx = y * width;
				UINT32 yDataIdx = y * mRowPitch * pixelSize;

				for (UINT32 x = 0; x < width; x++)
				{
					UINT32 arrayIdx = x + yArrayIdx + zArrayIdx;
					UINT32 dataIdx = x * pixelSize + yDataIdx + zDataIdx;

					UINT8* dest = data + dataIdx;
					PixelUtil::packColor(colors[arrayIdx], mFormat, dest);
				}
			}
		}
	}

	template BS_CORE_EXPORT void PixelData::setColorsInternal(Color* const &, UINT32);
	template BS_CORE_EXPORT void PixelData::setColorsInternal(const Vector<Color>&, UINT32);

	void PixelData::setColors(const Vector<Color>& colors)
	{
		setColorsInternal(colors, (UINT32)colors.size());
	}

	void PixelData::setColors(Color* colors, UINT32 numElements)
	{
		setColorsInternal(colors, numElements);
	}

	float PixelData::getDepthAt(UINT32 x, UINT32 y, UINT32 z) const
	{
		UINT32 pixelSize = PixelUtil::getNumElemBytes(mFormat);
		UINT32 pixelOffset = pixelSize * (z * mSlicePitch + y * mRowPitch + x);
		return PixelUtil::unpackDepth(mFormat, (unsigned char *)getData() + pixelOffset);;
	}

	void PixelData::setDepthAt(float depth, UINT32 x, UINT32 y, UINT32 z)
	{
		UINT32 pixelSize = PixelUtil::getNumElemBytes(mFormat);
		UINT32 pixelOffset = pixelSize * (z * mSlicePitch + y * mRowPitch + x);
		PixelUtil::packDepth(depth, mFormat, (unsigned char *)getData() + pixelOffset);
	}

	Vector<float> PixelData::getDepths() const
	{
		UINT32 depth = mExtents.getDepth();
		UINT32 height = mExtents.getHeight();
		UINT32 width = mExtents.getWidth();

		UINT32 pixelSize = PixelUtil::getNumElemBytes(mFormat);
		UINT8* data = getData();

		Vector<float> depths(width * height * depth);
		for (UINT32 z = 0; z < depth; z++)
		{
			UINT32 zArrayIdx = z * width * height;
			UINT32 zDataIdx = z * mSlicePitch * pixelSize;

			for (UINT32 y = 0; y < height; y++)
			{
				UINT32 yArrayIdx = y * width;
				UINT32 yDataIdx = y * mRowPitch * pixelSize;

				for (UINT32 x = 0; x < width; x++)
				{
					UINT32 arrayIdx = x + yArrayIdx + zArrayIdx;
					UINT32 dataIdx = x * pixelSize + yDataIdx + zDataIdx;

					UINT8* dest = data + dataIdx;
					depths[arrayIdx] = PixelUtil::unpackDepth(mFormat, dest);
				}
			}
		}

		return depths;
	}

	SPtr<PixelData> PixelData::create(const PixelVolume &extents, PixelFormat pixelFormat)
	{
		SPtr<PixelData> pixelData = bs_shared_ptr_new<PixelData>(extents, pixelFormat);
		pixelData->allocateInternalBuffer();

		return pixelData;
	}

	SPtr<PixelData> PixelData::create(UINT32 width, UINT32 height, UINT32 depth, PixelFormat pixelFormat)
	{
		SPtr<PixelData> pixelData = bs_shared_ptr_new<PixelData>(width, height, depth, pixelFormat);
		pixelData->allocateInternalBuffer();

		return pixelData;
	}

	UINT32 PixelData::getInternalBufferSize() const
	{
		return getSize();
	}

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/

	RTTITypeBase* PixelData::getRTTIStatic()
	{
		return PixelDataRTTI::instance();
	}

	RTTITypeBase* PixelData::getRTTI() const
	{
		return PixelData::getRTTIStatic();
	}
}
