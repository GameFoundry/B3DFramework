//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Image/BsPixelData.h"
#include "Image/BsPixelUtil.h"
#include "Private/RTTI/BsPixelDataRTTI.h"
#include "Image/BsColor.h"
#include "Math/BsVector2.h"
#include "Math/BsMath.h"
#include "Debug/BsDebug.h"

namespace bs
{
	PixelData::PixelData()
		:mExtents(0, 0, 0, 0), mFormat(PF_UNKNOWN), mRowPitch(0), mSlicePitch(0)
	{ }

	PixelData::PixelData(const PixelVolume& extents, PixelFormat pixelFormat)
		:mExtents(extents), mFormat(pixelFormat)
	{
		PixelUtil::getPitch(extents.getWidth(), extents.getHeight(), extents.getDepth(), pixelFormat, mRowPitch, 
			mSlicePitch);
	}

	PixelData::PixelData(UINT32 width, UINT32 height, UINT32 depth, PixelFormat pixelFormat)
		: mExtents(0, 0, 0, width, height, depth), mFormat(pixelFormat)
	{
		PixelUtil::getPitch(width, height, depth, pixelFormat, mRowPitch, mSlicePitch);
	}

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

	PixelData PixelData::getSubVolume(const PixelVolume& volume) const
	{
		if (PixelUtil::isCompressed(mFormat))
		{
			if (volume.left == getLeft() && volume.top == getTop() && volume.front == getFront() &&
				volume.right == getRight() && volume.bottom == getBottom() && volume.back == getBack())
			{
				// Entire buffer is being queried
				return *this;
			}

			BS_EXCEPT(InvalidParametersException, "Cannot return subvolume of compressed PixelBuffer");
		}

		if (!mExtents.contains(volume))
		{
			BS_EXCEPT(InvalidParametersException, "Bounds out of range");
		}

		const size_t elemSize = PixelUtil::getNumElemBytes(mFormat);
		PixelData rval(volume.getWidth(), volume.getHeight(), volume.getDepth(), mFormat);

		rval.setExternalBuffer(((UINT8*)getData()) + ((volume.left - getLeft())*elemSize)
			+ ((volume.top - getTop())*mRowPitch*elemSize)
			+ ((volume.front - getFront())*mSlicePitch*elemSize));

		rval.mFormat = mFormat;
		PixelUtil::getPitch(volume.getWidth(), volume.getHeight(), volume.getDepth(), mFormat, rval.mRowPitch,
							rval.mSlicePitch);

		return rval;
	}

	Color PixelData::sampleColorAt(const Vector2& coords, TextureFilter filter) const
	{
		Vector2 pixelCoords = coords * Vector2((float)mExtents.getWidth(), (float)mExtents.getHeight());

		INT32 maxExtentX = std::max(0, (INT32)mExtents.getWidth() - 1);
		INT32 maxExtentY = std::max(0, (INT32)mExtents.getHeight() - 1);

		if(filter == TF_BILINEAR)
		{
			pixelCoords -= Vector2(0.5f, 0.5f);

			UINT32 x = (UINT32)Math::clamp(Math::floorToInt(pixelCoords.x), 0, maxExtentX);
			UINT32 y = (UINT32)Math::clamp(Math::floorToInt(pixelCoords.y), 0, maxExtentY);

			float fracX = pixelCoords.x - x;
			float fracY = pixelCoords.y - y;

			x = Math::clamp(x, 0U, (UINT32)maxExtentX);
			y = Math::clamp(y, 0U, (UINT32)maxExtentY);

			INT32 x1 = Math::clamp(x + 1, 0U, (UINT32)maxExtentX);
			INT32 y1 = Math::clamp(y + 1, 0U, (UINT32)maxExtentY);

			Color color = Color::ZERO;
			color += (1.0f - fracX) * (1.0f - fracY) * getColorAt(x, y);
			color += fracX * (1.0f - fracY) * getColorAt(x1, y);
			color += (1.0f - fracX) * fracY * getColorAt(x, y1);
			color += fracX * fracY * getColorAt(x1, y1);

			return color;
		}
		else
		{
			UINT32 x = (UINT32)Math::clamp(Math::floorToInt(pixelCoords.x), 0, maxExtentX);
			UINT32 y = (UINT32)Math::clamp(Math::floorToInt(pixelCoords.y), 0, maxExtentY);

			return getColorAt(x, y);
		}
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

	void PixelData::setColors(const Color& color)
	{
		UINT32 depth = mExtents.getDepth();
		UINT32 height = mExtents.getHeight();
		UINT32 width = mExtents.getWidth();

		UINT32 pixelSize = PixelUtil::getNumElemBytes(mFormat);
		UINT32 packedColor[4];
		assert(pixelSize <= sizeof(packedColor));

		PixelUtil::packColor(color, mFormat, packedColor);

		UINT8* data = getData();
		for (UINT32 z = 0; z < depth; z++)
		{
			UINT32 zDataIdx = z * mSlicePitch * pixelSize;

			for (UINT32 y = 0; y < height; y++)
			{
				UINT32 yDataIdx = y * mRowPitch * pixelSize;

				for (UINT32 x = 0; x < width; x++)
				{
					UINT32 dataIdx = x * pixelSize + yDataIdx + zDataIdx;

					UINT8* dest = data + dataIdx;
					memcpy(dest, packedColor, pixelSize);
				}
			}
		}
	}

	float PixelData::getDepthAt(UINT32 x, UINT32 y, UINT32 z) const
	{
		UINT32 pixelSize = PixelUtil::getNumElemBytes(mFormat);
		UINT32 pixelOffset = pixelSize * (z * mSlicePitch + y * mRowPitch + x);
		return PixelUtil::unpackDepth(mFormat, (unsigned char *)getData() + pixelOffset);;
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
