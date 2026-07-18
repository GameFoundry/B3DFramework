//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/B3DPixelData.h"
#include "Image/B3DPixelUtility.h"
#include "RTTI/B3DPixelDataRTTI.h"
#include "Image/B3DColor.h"
#include "Math/B3DVector2.h"
#include "Math/B3DMath.h"
#include "Debug/B3DDebug.h"

using namespace b3d;

PixelData::PixelData(const PixelVolume& extents, PixelFormat pixelFormat)
	: mExtents(extents), mFormat(pixelFormat)
{
	PixelUtility::GetPitch(extents.GetWidth(), extents.GetHeight(), extents.GetDepth(), pixelFormat, mRowPitch, mSlicePitch);
}

PixelData::PixelData(u32 width, u32 height, u32 depth, PixelFormat pixelFormat)
	: mExtents(0, 0, 0, width, height, depth), mFormat(pixelFormat)
{
	PixelUtility::GetPitch(width, height, depth, pixelFormat, mRowPitch, mSlicePitch);
}

PixelData::PixelData(const PixelData& copy)
	: GpuResourceData(copy)
{
	mFormat = copy.mFormat;
	mRowPitch = copy.mRowPitch;
	mSlicePitch = copy.mSlicePitch;
	mExtents = copy.mExtents;
}

PixelData& PixelData::operator=(const PixelData& rhs)
{
	GpuResourceData::operator=(rhs);

	mFormat = rhs.mFormat;
	mRowPitch = rhs.mRowPitch;
	mSlicePitch = rhs.mSlicePitch;
	mExtents = rhs.mExtents;

	return *this;
}

u32 PixelData::GetRowSkip() const
{
	u32 optimalRowPitch, optimalSlicePitch;
	PixelUtility::GetPitch(GetWidth(), GetHeight(), GetDepth(), mFormat, optimalRowPitch, optimalSlicePitch);

	return mRowPitch - optimalRowPitch;
}

u32 PixelData::GetSliceSkip() const
{
	u32 rowCount = GetHeight();
	if(PixelUtility::IsCompressed(mFormat))
	{
		const Vector2I blockDimensions = PixelUtility::GetBlockDimensions(mFormat);
		rowCount = Math::DivideAndRoundUp(rowCount, (u32)blockDimensions.Y);
	}

	return mSlicePitch - rowCount * mRowPitch;
}

u32 PixelData::GetConsecutiveSize() const
{
	return PixelUtility::GetMemorySize(GetWidth(), GetHeight(), GetDepth(), mFormat);
}

u32 PixelData::GetSize() const
{
	return mSlicePitch * GetDepth();
}

PixelData PixelData::GetSubVolume(const PixelVolume& volume) const
{
	if(PixelUtility::IsCompressed(mFormat))
	{
		if(volume.Left == GetLeft() && volume.Top == GetTop() && volume.Front == GetFront() &&
		   volume.Right == GetRight() && volume.Bottom == GetBottom() && volume.Back == GetBack())
		{
			// Entire buffer is being queried
			return *this;
		}

		if(!B3D_ENSURE_LOG(false, "Cannot return subvolume of compressed PixelBuffer"))
			return *this;
	}

	if(!B3D_ENSURE_LOG(mExtents.Contains(volume), "Bounds out of range"))
		return *this;

	const size_t elemSize = PixelUtility::GetElementByteCount(mFormat);
	PixelData rval(volume.GetWidth(), volume.GetHeight(), volume.GetDepth(), mFormat);

	rval.SetExternalBuffer(((u8*)GetData()) + ((volume.Left - GetLeft()) * elemSize) + ((volume.Top - GetTop()) * mRowPitch) + ((volume.Front - GetFront()) * mSlicePitch));

	rval.mFormat = mFormat;
	PixelUtility::GetPitch(volume.GetWidth(), volume.GetHeight(), volume.GetDepth(), mFormat, rval.mRowPitch, rval.mSlicePitch);

	return rval;
}

Color PixelData::SampleColorAt(const Vector2& coords, TextureFilter filter) const
{
	Vector2 pixelCoords = coords * Vector2((float)mExtents.GetWidth(), (float)mExtents.GetHeight());

	i32 maxExtentX = std::max(0, (i32)mExtents.GetWidth() - 1);
	i32 maxExtentY = std::max(0, (i32)mExtents.GetHeight() - 1);

	if(filter == TF_BILINEAR)
	{
		pixelCoords -= Vector2(0.5f, 0.5f);

		u32 x = (u32)Math::Clamp(Math::FloorToInt(pixelCoords.X), 0, maxExtentX);
		u32 y = (u32)Math::Clamp(Math::FloorToInt(pixelCoords.Y), 0, maxExtentY);

		float fracX = pixelCoords.X - x;
		float fracY = pixelCoords.Y - y;

		x = Math::Clamp(x, 0U, (u32)maxExtentX);
		y = Math::Clamp(y, 0U, (u32)maxExtentY);

		i32 x1 = Math::Clamp(x + 1, 0U, (u32)maxExtentX);
		i32 y1 = Math::Clamp(y + 1, 0U, (u32)maxExtentY);

		Color color = Color::kZero;
		color += (1.0f - fracX) * (1.0f - fracY) * GetColorAt(x, y);
		color += fracX * (1.0f - fracY) * GetColorAt(x1, y);
		color += (1.0f - fracX) * fracY * GetColorAt(x, y1);
		color += fracX * fracY * GetColorAt(x1, y1);

		return color;
	}
	else
	{
		u32 x = (u32)Math::Clamp(Math::FloorToInt(pixelCoords.X), 0, maxExtentX);
		u32 y = (u32)Math::Clamp(Math::FloorToInt(pixelCoords.Y), 0, maxExtentY);

		return GetColorAt(x, y);
	}
}

Color PixelData::GetColorAt(u32 x, u32 y, u32 z) const
{
	Color cv;

	u32 pixelSize = PixelUtility::GetElementByteCount(mFormat);
	u32 pixelOffset = z * mSlicePitch + y * mRowPitch + x * pixelSize;
	PixelUtility::UnpackColor(&cv, mFormat, (unsigned char*)GetData() + pixelOffset);

	return cv;
}

void PixelData::SetColorAt(const Color& color, u32 x, u32 y, u32 z)
{
	u32 pixelSize = PixelUtility::GetElementByteCount(mFormat);
	u32 pixelOffset = z * mSlicePitch + y * mRowPitch + x * pixelSize;
	PixelUtility::PackColor(color, mFormat, (unsigned char*)GetData() + pixelOffset);
}

Vector<Color> PixelData::GetColors() const
{
	u32 depth = mExtents.GetDepth();
	u32 height = mExtents.GetHeight();
	u32 width = mExtents.GetWidth();

	u32 pixelSize = PixelUtility::GetElementByteCount(mFormat);
	u8* data = GetData();

	Vector<Color> colors(width * height * depth);
	for(u32 z = 0; z < depth; z++)
	{
		u32 zArrayIdx = z * width * height;
		u32 zDataIdx = z * mSlicePitch;

		for(u32 y = 0; y < height; y++)
		{
			u32 yArrayIdx = y * width;
			u32 yDataIdx = y * mRowPitch;

			for(u32 x = 0; x < width; x++)
			{
				u32 arrayIdx = x + yArrayIdx + zArrayIdx;
				u32 dataIdx = x * pixelSize + yDataIdx + zDataIdx;

				u8* dest = data + dataIdx;
				PixelUtility::UnpackColor(&colors[arrayIdx], mFormat, dest);
			}
		}
	}

	return colors;
}

template <class T>
void PixelData::SetColorsInternal(const T& colors, u32 elementCount)
{
	u32 depth = mExtents.GetDepth();
	u32 height = mExtents.GetHeight();
	u32 width = mExtents.GetWidth();

	u32 totalElementCount = width * height * depth;
	if(elementCount != totalElementCount)
	{
		B3D_LOG(Error, LogPixelUtility, "Unable to set colors, invalid array size.");
		return;
	}

	u32 pixelSize = PixelUtility::GetElementByteCount(mFormat);
	u8* data = GetData();

	for(u32 z = 0; z < depth; z++)
	{
		u32 zArrayIdx = z * width * height;
		u32 zDataIdx = z * mSlicePitch;

		for(u32 y = 0; y < height; y++)
		{
			u32 yArrayIdx = y * width;
			u32 yDataIdx = y * mRowPitch;

			for(u32 x = 0; x < width; x++)
			{
				u32 arrayIdx = x + yArrayIdx + zArrayIdx;
				u32 dataIdx = x * pixelSize + yDataIdx + zDataIdx;

				u8* dest = data + dataIdx;
				PixelUtility::PackColor(colors[arrayIdx], mFormat, dest);
			}
		}
	}
}

template B3D_EXPORT void PixelData::SetColorsInternal(Color* const&, u32);
template B3D_EXPORT void PixelData::SetColorsInternal(const Vector<Color>&, u32);

void PixelData::SetColors(const Vector<Color>& colors)
{
	SetColorsInternal(colors, (u32)colors.size());
}

void PixelData::SetColors(Color* colors, u32 elementCount)
{
	SetColorsInternal(colors, elementCount);
}

void PixelData::SetColors(const Color& color)
{
	u32 depth = mExtents.GetDepth();
	u32 height = mExtents.GetHeight();
	u32 width = mExtents.GetWidth();

	u32 pixelSize = PixelUtility::GetElementByteCount(mFormat);
	u32 packedColor[4];
	B3D_ASSERT(pixelSize <= sizeof(packedColor));

	PixelUtility::PackColor(color, mFormat, packedColor);

	u8* data = GetData();
	for(u32 z = 0; z < depth; z++)
	{
		u32 zDataIdx = z * mSlicePitch;

		for(u32 y = 0; y < height; y++)
		{
			u32 yDataIdx = y * mRowPitch;

			for(u32 x = 0; x < width; x++)
			{
				u32 dataIdx = x * pixelSize + yDataIdx + zDataIdx;

				u8* dest = data + dataIdx;
				memcpy(dest, packedColor, pixelSize);
			}
		}
	}
}

float PixelData::GetDepthAt(u32 x, u32 y, u32 z) const
{
	u32 pixelSize = PixelUtility::GetElementByteCount(mFormat);
	u32 pixelOffset = z * mSlicePitch + y * mRowPitch + x * pixelSize;
	return PixelUtility::UnpackDepth(mFormat, (unsigned char*)GetData() + pixelOffset);
	;
}

Vector<float> PixelData::GetDepths() const
{
	u32 depth = mExtents.GetDepth();
	u32 height = mExtents.GetHeight();
	u32 width = mExtents.GetWidth();

	u32 pixelSize = PixelUtility::GetElementByteCount(mFormat);
	u8* data = GetData();

	Vector<float> depths(width * height * depth);
	for(u32 z = 0; z < depth; z++)
	{
		u32 zArrayIdx = z * width * height;
		u32 zDataIdx = z * mSlicePitch;

		for(u32 y = 0; y < height; y++)
		{
			u32 yArrayIdx = y * width;
			u32 yDataIdx = y * mRowPitch;

			for(u32 x = 0; x < width; x++)
			{
				u32 arrayIdx = x + yArrayIdx + zArrayIdx;
				u32 dataIdx = x * pixelSize + yDataIdx + zDataIdx;

				u8* dest = data + dataIdx;
				depths[arrayIdx] = PixelUtility::UnpackDepth(mFormat, dest);
			}
		}
	}

	return depths;
}

TShared<PixelData> PixelData::Create(const PixelVolume& extents, PixelFormat pixelFormat)
{
	TShared<PixelData> pixelData = B3DMakeShared<PixelData>(extents, pixelFormat);
	pixelData->AllocateInternalBuffer();

	return pixelData;
}

TShared<PixelData> PixelData::Create(u32 width, u32 height, u32 depth, PixelFormat pixelFormat)
{
	TShared<PixelData> pixelData = B3DMakeShared<PixelData>(width, height, depth, pixelFormat);
	pixelData->AllocateInternalBuffer();

	return pixelData;
}

u32 PixelData::GetInternalBufferSize() const
{
	return GetSize();
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/

RTTIType* PixelData::GetRttiStatic()
{
	return PixelDataRTTI::Instance();
}

RTTIType* PixelData::GetRtti() const
{
	return PixelData::GetRttiStatic();
}
