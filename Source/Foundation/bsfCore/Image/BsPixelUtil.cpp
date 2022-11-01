//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/BsPixelUtil.h"
#include "Utility/BsBitwise.h"
#include "Image/BsColor.h"
#include "Math/BsMath.h"
#include "Error/BsException.h"
#include "Image/BsTexture.h"
#include <nvtt.h>

using namespace bs;

/**
 * Performs pixel data resampling using the point filter (nearest neighbor). Does not perform format conversions.
 *
 * @tparam elementSize	Size of a single pixel in bytes.
 */
template <u32 elementSize>
struct NearestResampler
{
	static void Scale(const PixelData& source, const PixelData& dest)
	{
		u8* sourceData = source.GetData();
		u8* destPtr = dest.GetData();

		// Get steps for traversing source data in 16/48 fixed point format
		u64 stepX = ((u64)source.GetWidth() << 48) / dest.GetWidth();
		u64 stepY = ((u64)source.GetHeight() << 48) / dest.GetHeight();
		u64 stepZ = ((u64)source.GetDepth() << 48) / dest.GetDepth();

		u64 curZ = (stepZ >> 1) - 1; // Offset half a pixel to start at pixel center
		for(u32 z = dest.GetFront(); z < dest.GetBack(); z++, curZ += stepZ)
		{
			u32 offsetZ = (u32)(curZ >> 48) * source.GetSlicePitch();

			u64 curY = (stepY >> 1) - 1; // Offset half a pixel to start at pixel center
			for(u32 y = dest.GetTop(); y < dest.GetBottom(); y++, curY += stepY)
			{
				u32 offsetY = (u32)(curY >> 48) * source.GetRowPitch();

				u64 curX = (stepX >> 1) - 1; // Offset half a pixel to start at pixel center
				for(u32 x = dest.GetLeft(); x < dest.GetRight(); x++, curX += stepX)
				{
					u32 offsetX = (u32)(curX >> 48);
					u32 offsetBytes = elementSize * offsetX + offsetY + offsetZ;

					u8* curSourcePtr = sourceData + offsetBytes;

					memcpy(destPtr, curSourcePtr, elementSize);
					destPtr += elementSize;
				}

				destPtr += dest.GetRowSkip();
			}

			destPtr += dest.GetSliceSkip();
		}
	}
};

/** Performs pixel data resampling using the box filter (linear). Performs format conversions. */
struct LinearResampler
{
	static void Scale(const PixelData& source, const PixelData& dest)
	{
		u32 sourceElemSize = PixelUtil::GetNumElemBytes(source.GetFormat());
		u32 destElemSize = PixelUtil::GetNumElemBytes(dest.GetFormat());

		u8* sourceData = source.GetData();
		u8* destPtr = dest.GetData();

		// Get steps for traversing source data in 16/48 fixed point precision format
		u64 stepX = ((u64)source.GetWidth() << 48) / dest.GetWidth();
		u64 stepY = ((u64)source.GetHeight() << 48) / dest.GetHeight();
		u64 stepZ = ((u64)source.GetDepth() << 48) / dest.GetDepth();

		// Contains 16/16 fixed point precision format. Most significant
		// 16 bits will contain the coordinate in the source image, and the
		// least significant 16 bits will contain the fractional part of the coordinate
		// that will be used for determining the blend amount.
		u32 temp = 0;

		u64 curZ = (stepZ >> 1) - 1; // Offset half a pixel to start at pixel center
		for(u32 z = dest.GetFront(); z < dest.GetBack(); z++, curZ += stepZ)
		{
			temp = u32(curZ >> 32);
			temp = (temp > 0x8000) ? temp - 0x8000 : 0;
			u32 sampleCoordZ1 = temp >> 16;
			u32 sampleCoordZ2 = std::min(sampleCoordZ1 + 1, (u32)source.GetDepth() - 1);
			float sampleWeightZ = (temp & 0xFFFF) / 65536.0f;

			u64 curY = (stepY >> 1) - 1; // Offset half a pixel to start at pixel center
			for(u32 y = dest.GetTop(); y < dest.GetBottom(); y++, curY += stepY)
			{
				temp = (u32)(curY >> 32);
				temp = (temp > 0x8000) ? temp - 0x8000 : 0;
				u32 sampleCoordY1 = temp >> 16;
				u32 sampleCoordY2 = std::min(sampleCoordY1 + 1, (u32)source.GetHeight() - 1);
				float sampleWeightY = (temp & 0xFFFF) / 65536.0f;

				u64 curX = (stepX >> 1) - 1; // Offset half a pixel to start at pixel center
				for(u32 x = dest.GetLeft(); x < dest.GetRight(); x++, curX += stepX)
				{
					temp = (u32)(curX >> 32);
					temp = (temp > 0x8000) ? temp - 0x8000 : 0;
					u32 sampleCoordX1 = temp >> 16;
					u32 sampleCoordX2 = std::min(sampleCoordX1 + 1, (u32)source.GetWidth() - 1);
					float sampleWeightX = (temp & 0xFFFF) / 65536.0f;

					Color x1y1z1, x2y1z1, x1y2z1, x2y2z1;
					Color x1y1z2, x2y1z2, x1y2z2, x2y2z2;

#define GETSOURCEDATA(x, y, z) sourceData + sourceElemSize*(x) + (y)*source.GetRowPitch() + (z)*source.GetSlicePitch()

					PixelUtil::UnpackColor(&x1y1z1, source.GetFormat(), GETSOURCEDATA(sampleCoordX1, sampleCoordY1, sampleCoordZ1));
					PixelUtil::UnpackColor(&x2y1z1, source.GetFormat(), GETSOURCEDATA(sampleCoordX2, sampleCoordY1, sampleCoordZ1));
					PixelUtil::UnpackColor(&x1y2z1, source.GetFormat(), GETSOURCEDATA(sampleCoordX1, sampleCoordY2, sampleCoordZ1));
					PixelUtil::UnpackColor(&x2y2z1, source.GetFormat(), GETSOURCEDATA(sampleCoordX2, sampleCoordY2, sampleCoordZ1));
					PixelUtil::UnpackColor(&x1y1z2, source.GetFormat(), GETSOURCEDATA(sampleCoordX1, sampleCoordY1, sampleCoordZ2));
					PixelUtil::UnpackColor(&x2y1z2, source.GetFormat(), GETSOURCEDATA(sampleCoordX2, sampleCoordY1, sampleCoordZ2));
					PixelUtil::UnpackColor(&x1y2z2, source.GetFormat(), GETSOURCEDATA(sampleCoordX1, sampleCoordY2, sampleCoordZ2));
					PixelUtil::UnpackColor(&x2y2z2, source.GetFormat(), GETSOURCEDATA(sampleCoordX2, sampleCoordY2, sampleCoordZ2));
#undef GETSOURCEDATA

					Color accum =
						x1y1z1 * ((1.0f - sampleWeightX) * (1.0f - sampleWeightY) * (1.0f - sampleWeightZ)) +
						x2y1z1 * (sampleWeightX * (1.0f - sampleWeightY) * (1.0f - sampleWeightZ)) +
						x1y2z1 * ((1.0f - sampleWeightX) * sampleWeightY * (1.0f - sampleWeightZ)) +
						x2y2z1 * (sampleWeightX * sampleWeightY * (1.0f - sampleWeightZ)) +
						x1y1z2 * ((1.0f - sampleWeightX) * (1.0f - sampleWeightY) * sampleWeightZ) +
						x2y1z2 * (sampleWeightX * (1.0f - sampleWeightY) * sampleWeightZ) +
						x1y2z2 * ((1.0f - sampleWeightX) * sampleWeightY * sampleWeightZ) +
						x2y2z2 * (sampleWeightX * sampleWeightY * sampleWeightZ);

					PixelUtil::PackColor(accum, dest.GetFormat(), destPtr);

					destPtr += destElemSize;
				}

				destPtr += dest.GetRowSkip();
			}

			destPtr += dest.GetSliceSkip();
		}
	}
};

/**
 * Performs pixel data resampling using the box filter (linear). Only handles float RGB or RGBA pixel data (32 bits per
 * channel).
 */
struct LinearResampler_Float32
{
	static void Scale(const PixelData& source, const PixelData& dest)
	{
		u32 sourcePixelSize = PixelUtil::GetNumElemBytes(source.GetFormat());
		u32 destPixelSize = PixelUtil::GetNumElemBytes(dest.GetFormat());

		u32 numSourceChannels = sourcePixelSize / sizeof(float);
		u32 numDestChannels = destPixelSize / sizeof(float);

		float* sourceData = (float*)source.GetData();
		float* destPtr = (float*)dest.GetData();

		// Get steps for traversing source data in 16/48 fixed point precision format
		u64 stepX = ((u64)source.GetWidth() << 48) / dest.GetWidth();
		u64 stepY = ((u64)source.GetHeight() << 48) / dest.GetHeight();
		u64 stepZ = ((u64)source.GetDepth() << 48) / dest.GetDepth();

		u32 sourceRowPitch = source.GetRowPitch() / sourcePixelSize;
		u32 sourceSlicePitch = source.GetSlicePitch() / sourcePixelSize;

		// Contains 16/16 fixed point precision format. Most significant
		// 16 bits will contain the coordinate in the source image, and the
		// least significant 16 bits will contain the fractional part of the coordinate
		// that will be used for determining the blend amount.
		u32 temp = 0;

		u64 curZ = (stepZ >> 1) - 1; // Offset half a pixel to start at pixel center
		for(u32 z = dest.GetFront(); z < dest.GetBack(); z++, curZ += stepZ)
		{
			temp = (u32)(curZ >> 32);
			temp = (temp > 0x8000) ? temp - 0x8000 : 0;
			u32 sampleCoordZ1 = temp >> 16;
			u32 sampleCoordZ2 = std::min(sampleCoordZ1 + 1, (u32)source.GetDepth() - 1);
			float sampleWeightZ = (temp & 0xFFFF) / 65536.0f;

			u64 curY = (stepY >> 1) - 1; // Offset half a pixel to start at pixel center
			for(u32 y = dest.GetTop(); y < dest.GetBottom(); y++, curY += stepY)
			{
				temp = (u32)(curY >> 32);
				temp = (temp > 0x8000) ? temp - 0x8000 : 0;
				u32 sampleCoordY1 = temp >> 16;
				u32 sampleCoordY2 = std::min(sampleCoordY1 + 1, (u32)source.GetHeight() - 1);
				float sampleWeightY = (temp & 0xFFFF) / 65536.0f;

				u64 curX = (stepX >> 1) - 1; // Offset half a pixel to start at pixel center
				for(u32 x = dest.GetLeft(); x < dest.GetRight(); x++, curX += stepX)
				{
					temp = (u32)(curX >> 32);
					temp = (temp > 0x8000) ? temp - 0x8000 : 0;
					u32 sampleCoordX1 = temp >> 16;
					u32 sampleCoordX2 = std::min(sampleCoordX1 + 1, (u32)source.GetWidth() - 1);
					float sampleWeightX = (temp & 0xFFFF) / 65536.0f;

					// process R,G,B,A simultaneously for cache coherence?
					float accum[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

#define ACCUM3(x, y, z, factor)                                                           \
	{                                                                                     \
		float f = factor;                                                                 \
		u32 offset = (x + y * sourceRowPitch + z * sourceSlicePitch) * numSourceChannels; \
		accum[0] += sourceData[offset + 0] * f;                                           \
		accum[1] += sourceData[offset + 1] * f;                                           \
		accum[2] += sourceData[offset + 2] * f;                                           \
	}

#define ACCUM4(x, y, z, factor)                                                           \
	{                                                                                     \
		float f = factor;                                                                 \
		u32 offset = (x + y * sourceRowPitch + z * sourceSlicePitch) * numSourceChannels; \
		accum[0] += sourceData[offset + 0] * f;                                           \
		accum[1] += sourceData[offset + 1] * f;                                           \
		accum[2] += sourceData[offset + 2] * f;                                           \
		accum[3] += sourceData[offset + 3] * f;                                           \
	}

					if(numSourceChannels == 3 || numDestChannels == 3)
					{
						// RGB
						ACCUM3(sampleCoordX1, sampleCoordY1, sampleCoordZ1, (1.0f - sampleWeightX) * (1.0f - sampleWeightY) * (1.0f - sampleWeightZ));
						ACCUM3(sampleCoordX2, sampleCoordY1, sampleCoordZ1, sampleWeightX * (1.0f - sampleWeightY) * (1.0f - sampleWeightZ));
						ACCUM3(sampleCoordX1, sampleCoordY2, sampleCoordZ1, (1.0f - sampleWeightX) * sampleWeightY * (1.0f - sampleWeightZ));
						ACCUM3(sampleCoordX2, sampleCoordY2, sampleCoordZ1, sampleWeightX * sampleWeightY * (1.0f - sampleWeightZ));
						ACCUM3(sampleCoordX1, sampleCoordY1, sampleCoordZ2, (1.0f - sampleWeightX) * (1.0f - sampleWeightY) * sampleWeightZ);
						ACCUM3(sampleCoordX2, sampleCoordY1, sampleCoordZ2, sampleWeightX * (1.0f - sampleWeightY) * sampleWeightZ);
						ACCUM3(sampleCoordX1, sampleCoordY2, sampleCoordZ2, (1.0f - sampleWeightX) * sampleWeightY * sampleWeightZ);
						ACCUM3(sampleCoordX2, sampleCoordY2, sampleCoordZ2, sampleWeightX * sampleWeightY * sampleWeightZ);
						accum[3] = 1.0f;
					}
					else
					{
						// RGBA
						ACCUM4(sampleCoordX1, sampleCoordY1, sampleCoordZ1, (1.0f - sampleWeightX) * (1.0f - sampleWeightY) * (1.0f - sampleWeightZ));
						ACCUM4(sampleCoordX2, sampleCoordY1, sampleCoordZ1, sampleWeightX * (1.0f - sampleWeightY) * (1.0f - sampleWeightZ));
						ACCUM4(sampleCoordX1, sampleCoordY2, sampleCoordZ1, (1.0f - sampleWeightX) * sampleWeightY * (1.0f - sampleWeightZ));
						ACCUM4(sampleCoordX2, sampleCoordY2, sampleCoordZ1, sampleWeightX * sampleWeightY * (1.0f - sampleWeightZ));
						ACCUM4(sampleCoordX1, sampleCoordY1, sampleCoordZ2, (1.0f - sampleWeightX) * (1.0f - sampleWeightY) * sampleWeightZ);
						ACCUM4(sampleCoordX2, sampleCoordY1, sampleCoordZ2, sampleWeightX * (1.0f - sampleWeightY) * sampleWeightZ);
						ACCUM4(sampleCoordX1, sampleCoordY2, sampleCoordZ2, (1.0f - sampleWeightX) * sampleWeightY * sampleWeightZ);
						ACCUM4(sampleCoordX2, sampleCoordY2, sampleCoordZ2, sampleWeightX * sampleWeightY * sampleWeightZ);
					}

					memcpy(destPtr, accum, sizeof(float) * numDestChannels);

#undef ACCUM3
#undef ACCUM4

					destPtr += numDestChannels;
				}

				destPtr += dest.GetRowSkip() / sizeof(float);
			}

			destPtr += dest.GetSliceSkip() / sizeof(float);
		}
	}
};

// byte linear resampler, does not do any format conversions.
// only handles pixel formats that use 1 byte per color channel.
// 2D only; punts 3D pixelboxes to default LinearResampler (slow).
// templated on bytes-per-pixel to allow compiler optimizations, such
// as unrolling loops and replacing multiplies with bitshifts

/**
 * Performs pixel data resampling using the box filter (linear). Only handles pixel formats with one byte per channel.
 * Does not perform format conversion.
 *
 * @tparam	channels	Number of channels in the pixel format.
 */
template <u32 channels>
struct LinearResampler_Byte
{
	static void Scale(const PixelData& source, const PixelData& dest)
	{
		// Only optimized for 2D
		if(source.GetDepth() > 1 || dest.GetDepth() > 1)
		{
			LinearResampler::Scale(source, dest);
			return;
		}

		u8* sourceData = (u8*)source.GetData();
		u8* destPtr = (u8*)dest.GetData();

		// Get steps for traversing source data in 16/48 fixed point precision format
		u64 stepX = ((u64)source.GetWidth() << 48) / dest.GetWidth();
		u64 stepY = ((u64)source.GetHeight() << 48) / dest.GetHeight();

		// Contains 16/16 fixed point precision format. Most significant
		// 16 bits will contain the coordinate in the source image, and the
		// least significant 16 bits will contain the fractional part of the coordinate
		// that will be used for determining the blend amount.
		u32 temp;

		u64 curY = (stepY >> 1) - 1; // Offset half a pixel to start at pixel center
		for(u32 y = dest.GetTop(); y < dest.GetBottom(); y++, curY += stepY)
		{
			temp = (u32)(curY >> 36);
			temp = (temp > 0x800) ? temp - 0x800 : 0;
			u32 sampleWeightY = temp & 0xFFF;
			u32 sampleCoordY1 = temp >> 12;
			u32 sampleCoordY2 = std::min(sampleCoordY1 + 1, (u32)source.GetBottom() - source.GetTop() - 1);

			u32 sampleY1Offset = sampleCoordY1 * source.GetRowPitch();
			u32 sampleY2Offset = sampleCoordY2 * source.GetRowPitch();

			u64 curX = (stepX >> 1) - 1; // Offset half a pixel to start at pixel center
			for(u32 x = dest.GetLeft(); x < dest.GetRight(); x++, curX += stepX)
			{
				temp = (u32)(curX >> 36);
				temp = (temp > 0x800) ? temp - 0x800 : 0;
				u32 sampleWeightX = temp & 0xFFF;
				u32 sampleCoordX1 = temp >> 12;
				u32 sampleCoordX2 = std::min(sampleCoordX1 + 1, (u32)source.GetRight() - source.GetLeft() - 1);

				u32 sxfsyf = sampleWeightX * sampleWeightY;
				for(u32 k = 0; k < channels; k++)
				{
					u32 accum =
						sourceData[sampleCoordX1 * channels + sampleY1Offset + k] * (0x1000000 - (sampleWeightX << 12) - (sampleWeightY << 12) + sxfsyf) +
						sourceData[sampleCoordX2 * channels + sampleY1Offset + k] * ((sampleWeightX << 12) - sxfsyf) +
						sourceData[sampleCoordX1 * channels + sampleY2Offset + k] * ((sampleWeightY << 12) - sxfsyf) +
						sourceData[sampleCoordX2 * channels + sampleY2Offset + k] * sxfsyf;

					// Round up to byte size
					*destPtr = (u8)((accum + 0x800000) >> 24);
					destPtr++;
				}
			}
			destPtr += dest.GetRowSkip();
		}
	}
};

/**	Data describing a pixel format. */
struct PixelFormatDescription
{
	const char* Name; /**< Name of the format. */
	u8 ElemBytes; /**< Number of bytes one element (color value) uses. */
	u32 Flags; /**< PixelFormatFlags set by the pixel format. */
	PixelComponentType ComponentType; /**< Data type of a single element of the format. */
	u8 ComponentCount; /**< Number of elements in the format. */

	u8 Rbits, Gbits, Bbits, Abits; /**< Number of bits per element in the format. */

	u32 Rmask, Gmask, Bmask, Amask; /**< Masks used by packers/unpackers. */
	u8 Rshift, Gshift, Bshift, Ashift; /**< Shifts used by packers/unpackers. */
};

/**	A list of all available pixel formats. */
PixelFormatDescription _pixelFormats[PF_COUNT] = {
	{
		"PF_UNKNOWN",
		/* Bytes per element */
		0,
		/* Flags */
		0,
		/* Component type and count */
		PCT_BYTE,
		0,
		/* rbits, gbits, bbits, abits */
		0,
		0,
		0,
		0,
		/* Masks and shifts */
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_R8",
		/* Bytes per element */
		1,
		/* Flags */
		PFF_INTEGER | PFF_NORMALIZED,
		/* Component type and count */
		PCT_BYTE,
		1,
		/* rbits, gbits, bbits, abits */
		8,
		0,
		0,
		0,
		/* Masks and shifts */
		0x000000FF,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RG8",
		/* Bytes per element */
		2,
		/* Flags */
		PFF_INTEGER | PFF_NORMALIZED,
		/* Component type and count */
		PCT_BYTE,
		2,
		/* rbits, gbits, bbits, abits */
		8,
		8,
		0,
		0,
		/* Masks and shifts */
		0x000000FF,
		0x0000FF00,
		0,
		0,
		0,
		8,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RGB8",
		/* Bytes per element */
		4, // 4th byte is unused
		/* Flags */
		PFF_INTEGER | PFF_NORMALIZED,
		/* Component type and count */
		PCT_BYTE,
		3,
		/* rbits, gbits, bbits, abits */
		8,
		8,
		8,
		0,
		/* Masks and shifts */
		0x000000FF,
		0x0000FF00,
		0x00FF0000,
		0,
		0,
		8,
		16,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_BGR8",
		/* Bytes per element */
		4, // 4th byte is unused
		/* Flags */
		PFF_INTEGER | PFF_NORMALIZED,
		/* Component type and count */
		PCT_BYTE,
		3,
		/* rbits, gbits, bbits, abits */
		8,
		8,
		8,
		0,
		/* Masks and shifts */
		0x00FF0000,
		0x0000FF00,
		0x000000FF,
		0,
		16,
		8,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{}, // Deleted format
	//-----------------------------------------------------------------------
	{}, // Deleted format
	//-----------------------------------------------------------------------
	{
		"PF_BGRA8",
		/* Bytes per element */
		4,
		/* Flags */
		PFF_HASALPHA | PFF_INTEGER | PFF_NORMALIZED,
		/* Component type and count */
		PCT_BYTE,
		4,
		/* rbits, gbits, bbits, abits */
		8,
		8,
		8,
		8,
		/* Masks and shifts */
		0x00FF0000,
		0x0000FF00,
		0x000000FF,
		0xFF000000,
		16,
		8,
		0,
		24,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RGBA8",
		/* Bytes per element */
		4,
		/* Flags */
		PFF_HASALPHA | PFF_INTEGER | PFF_NORMALIZED,
		/* Component type and count */
		PCT_BYTE,
		4,
		/* rbits, gbits, bbits, abits */
		8,
		8,
		8,
		8,
		/* Masks and shifts */
		0x000000FF,
		0x0000FF00,
		0x00FF0000,
		0xFF000000,
		0,
		8,
		16,
		24,
	},
	//-----------------------------------------------------------------------
	{}, // Deleted format
	//-----------------------------------------------------------------------
	{}, // Deleted format
	//-----------------------------------------------------------------------
	{}, // Deleted format
	//-----------------------------------------------------------------------
	{}, // Deleted format
	//-----------------------------------------------------------------------
	{
		"PF_BC1",
		/* Bytes per element */
		0,
		/* Flags */
		PFF_COMPRESSED | PFF_HASALPHA,
		/* Component type and count */
		PCT_BYTE,
		3, // No alpha
		/* rbits, gbits, bbits, abits */
		0,
		0,
		0,
		0,
		/* Masks and shifts */
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_BC1a",
		/* Bytes per element */
		0,
		/* Flags */
		PFF_COMPRESSED,
		/* Component type and count */
		PCT_BYTE,
		3,
		/* rbits, gbits, bbits, abits */
		0,
		0,
		0,
		0,
		/* Masks and shifts */
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_BC2",
		/* Bytes per element */
		0,
		/* Flags */
		PFF_COMPRESSED | PFF_HASALPHA,
		/* Component type and count */
		PCT_BYTE,
		4,
		/* rbits, gbits, bbits, abits */
		0,
		0,
		0,
		0,
		/* Masks and shifts */
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_BC3",
		/* Bytes per element */
		0,
		/* Flags */
		PFF_COMPRESSED | PFF_HASALPHA,
		/* Component type and count */
		PCT_BYTE,
		4,
		/* rbits, gbits, bbits, abits */
		0,
		0,
		0,
		0,
		/* Masks and shifts */
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_BC4",
		/* Bytes per element */
		0,
		/* Flags */
		PFF_COMPRESSED,
		/* Component type and count */
		PCT_BYTE,
		1,
		/* rbits, gbits, bbits, abits */
		0,
		0,
		0,
		0,
		/* Masks and shifts */
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_BC5",
		/* Bytes per element */
		0,
		/* Flags */
		PFF_COMPRESSED,
		/* Component type and count */
		PCT_BYTE,
		2,
		/* rbits, gbits, bbits, abits */
		0,
		0,
		0,
		0,
		/* Masks and shifts */
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_BC6H",
		/* Bytes per element */
		0,
		/* Flags */
		PFF_COMPRESSED,
		/* Component type and count */
		PCT_FLOAT16,
		3,
		/* rbits, gbits, bbits, abits */
		0,
		0,
		0,
		0,
		/* Masks and shifts */
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_BC7",
		/* Bytes per element */
		0,
		/* Flags */
		PFF_COMPRESSED | PFF_HASALPHA,
		/* Component type and count */
		PCT_BYTE,
		4,
		/* rbits, gbits, bbits, abits */
		0,
		0,
		0,
		0,
		/* Masks and shifts */
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_R16F",
		/* Bytes per element */
		2,
		/* Flags */
		PFF_FLOAT,
		/* Component type and count */
		PCT_FLOAT16,
		1,
		/* rbits, gbits, bbits, abits */
		16,
		0,
		0,
		0,
		/* Masks and shifts */
		0x0000FFFF,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RG16F",
		/* Bytes per element */
		4,
		/* Flags */
		PFF_FLOAT,
		/* Component type and count */
		PCT_FLOAT16,
		2,
		/* rbits, gbits, bbits, abits */
		16,
		16,
		0,
		0,
		/* Masks and shifts */
		0x0000FFFF,
		0xFFFF0000,
		0,
		0,
		0,
		16,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{}, //  Deleted format
	//-----------------------------------------------------------------------
	{
		"PF_RGBA16F",
		/* Bytes per element */
		8,
		/* Flags */
		PFF_FLOAT | PFF_HASALPHA,
		/* Component type and count */
		PCT_FLOAT16,
		4,
		/* rbits, gbits, bbits, abits */
		16,
		16,
		16,
		16,
		/* Masks and shifts */
		0x0000FFFF,
		0xFFFF0000,
		0x0000FFFF,
		0xFFFF0000,
		0,
		16,
		0,
		16,
	},
	//-----------------------------------------------------------------------
	{
		"PF_R32F",
		/* Bytes per element */
		4,
		/* Flags */
		PFF_FLOAT,
		/* Component type and count */
		PCT_FLOAT32,
		1,
		/* rbits, gbits, bbits, abits */
		32,
		0,
		0,
		0,
		/* Masks and shifts */
		0xFFFFFFFF,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RG32F",
		/* Bytes per element */
		8,
		/* Flags */
		PFF_FLOAT,
		/* Component type and count */
		PCT_FLOAT32,
		2,
		/* rbits, gbits, bbits, abits */
		32,
		32,
		0,
		0,
		/* Masks and shifts */
		0xFFFFFFFF,
		0xFFFFFFFF,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RGB32F",
		/* Bytes per element */
		12,
		/* Flags */
		PFF_FLOAT,
		/* Component type and count */
		PCT_FLOAT32,
		3,
		/* rbits, gbits, bbits, abits */
		32,
		32,
		32,
		0,
		/* Masks and shifts */
		0xFFFFFFFF,
		0xFFFFFFFF,
		0xFFFFFFFF,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RGBA32F",
		/* Bytes per element */
		16,
		/* Flags */
		PFF_FLOAT | PFF_HASALPHA,
		/* Component type and count */
		PCT_FLOAT32,
		4,
		/* rbits, gbits, bbits, abits */
		32,
		32,
		32,
		32,
		/* Masks and shifts */
		0xFFFFFFFF,
		0xFFFFFFFF,
		0xFFFFFFFF,
		0xFFFFFFFF,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_D32_S8X24",
		/* Bytes per element */
		8,
		/* Flags */
		PFF_DEPTH | PFF_NORMALIZED,
		/* Component type and count */
		PCT_FLOAT32,
		2,
		/* rbits, gbits, bbits, abits */
		32,
		8,
		0,
		0,
		/* Masks and shifts */
		0xFFFFFFFF,
		0x000000FF,
		0x00000000,
		0x00000000,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_D24_S8",
		/* Bytes per element */
		4,
		/* Flags */
		PFF_DEPTH | PFF_INTEGER | PFF_NORMALIZED,
		/* Component type and count */
		PCT_INT,
		2,
		/* rbits, gbits, bbits, abits */
		24,
		8,
		0,
		0,
		/* Masks and shifts */
		0x00FFFFFF,
		0x0FF0000,
		0x00000000,
		0x00000000,
		0,
		24,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_D32",
		/* Bytes per element */
		4,
		/* Flags */
		PFF_DEPTH | PFF_FLOAT,
		/* Component type and count */
		PCT_FLOAT32,
		1,
		/* rbits, gbits, bbits, abits */
		32,
		0,
		0,
		0,
		/* Masks and shifts */
		0xFFFFFFFF,
		0x00000000,
		0x00000000,
		0x00000000,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_D16",
		/* Bytes per element */
		2,
		/* Flags */
		PFF_DEPTH | PFF_INTEGER | PFF_NORMALIZED,
		/* Component type and count */
		PCT_SHORT,
		1,
		/* rbits, gbits, bbits, abits */
		16,
		0,
		0,
		0,
		/* Masks and shifts */
		0x0000FFFF,
		0x00000000,
		0x00000000,
		0x00000000,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RG11B10F",
		/* Bytes per element */
		4,
		/* Flags */
		PFF_FLOAT,
		/* Component type and count */
		PCT_PACKED_R11G11B10,
		1,
		/* rbits, gbits, bbits, abits */
		11,
		11,
		10,
		0,
		/* Masks and shifts */
		0x000007FF,
		0x003FF800,
		0xFFC00000,
		0,
		0,
		11,
		22,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RGB10A2",
		/* Bytes per element */
		4,
		/* Flags */
		PFF_INTEGER | PFF_NORMALIZED | PFF_HASALPHA,
		/* Component type and count */
		PCT_PACKED_R10G10B10A2,
		1,
		/* rbits, gbits, bbits, abits */
		10,
		10,
		10,
		2,
		/* Masks and shifts */
		0x000003FF,
		0x000FFC00,
		0x3FF00000,
		0xC0000000,
		0,
		10,
		20,
		30,
	},
	//-----------------------------------------------------------------------
	{
		"PF_R8I",
		/* Bytes per element */
		1,
		/* Flags */
		PFF_INTEGER | PFF_SIGNED,
		/* Component type and count */
		PCT_BYTE,
		1,
		/* rbits, gbits, bbits, abits */
		8,
		0,
		0,
		0,
		/* Masks and shifts */
		0x000000FF,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RG8I",
		/* Bytes per element */
		2,
		/* Flags */
		PFF_INTEGER | PFF_SIGNED,
		/* Component type and count */
		PCT_BYTE,
		2,
		/* rbits, gbits, bbits, abits */
		8,
		8,
		0,
		0,
		/* Masks and shifts */
		0x000000FF,
		0x0000FF00,
		0,
		0,
		0,
		8,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RGBA8I",
		/* Bytes per element */
		4,
		/* Flags */
		PFF_INTEGER | PFF_SIGNED | PFF_HASALPHA,
		/* Component type and count */
		PCT_BYTE,
		4,
		/* rbits, gbits, bbits, abits */
		8,
		8,
		8,
		8,
		/* Masks and shifts */
		0x000000FF,
		0x0000FF00,
		0x00FF0000,
		0xFF000000,
		0,
		8,
		16,
		24,
	},
	//-----------------------------------------------------------------------
	{
		"PF_R8U",
		/* Bytes per element */
		1,
		/* Flags */
		PFF_INTEGER,
		/* Component type and count */
		PCT_BYTE,
		1,
		/* rbits, gbits, bbits, abits */
		8,
		0,
		0,
		0,
		/* Masks and shifts */
		0x000000FF,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RG8U",
		/* Bytes per element */
		2,
		/* Flags */
		PFF_INTEGER,
		/* Component type and count */
		PCT_BYTE,
		2,
		/* rbits, gbits, bbits, abits */
		8,
		8,
		0,
		0,
		/* Masks and shifts */
		0x000000FF,
		0x0000FF00,
		0,
		0,
		0,
		8,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RGBA8U",
		/* Bytes per element */
		4,
		/* Flags */
		PFF_INTEGER | PFF_HASALPHA,
		/* Component type and count */
		PCT_BYTE,
		4,
		/* rbits, gbits, bbits, abits */
		8,
		8,
		8,
		8,
		/* Masks and shifts */
		0x000000FF,
		0x0000FF00,
		0x00FF0000,
		0xFF000000,
		0,
		8,
		16,
		24,
	},
	//-----------------------------------------------------------------------
	{
		"PF_R8S",
		/* Bytes per element */
		1,
		/* Flags */
		PFF_INTEGER | PFF_NORMALIZED | PFF_SIGNED,
		/* Component type and count */
		PCT_BYTE,
		1,
		/* rbits, gbits, bbits, abits */
		8,
		0,
		0,
		0,
		/* Masks and shifts */
		0x000000FF,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RG8S",
		/* Bytes per element */
		2,
		/* Flags */
		PFF_INTEGER | PFF_NORMALIZED | PFF_SIGNED,
		/* Component type and count */
		PCT_BYTE,
		2,
		/* rbits, gbits, bbits, abits */
		8,
		8,
		0,
		0,
		/* Masks and shifts */
		0x000000FF,
		0x0000FF00,
		0,
		0,
		0,
		8,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RGBA8S",
		/* Bytes per element */
		4,
		/* Flags */
		PFF_INTEGER | PFF_NORMALIZED | PFF_SIGNED | PFF_HASALPHA,
		/* Component type and count */
		PCT_BYTE,
		4,
		/* rbits, gbits, bbits, abits */
		8,
		8,
		8,
		8,
		/* Masks and shifts */
		0x000000FF,
		0x0000FF00,
		0x00FF0000,
		0xFF000000,
		0,
		8,
		16,
		24,
	},
	//-----------------------------------------------------------------------
	{
		"PF_R16I",
		/* Bytes per element */
		2,
		/* Flags */
		PFF_INTEGER | PFF_SIGNED,
		/* Component type and count */
		PCT_SHORT,
		1,
		/* rbits, gbits, bbits, abits */
		16,
		0,
		0,
		0,
		/* Masks and shifts */
		0x0000FFFF,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RG16I",
		/* Bytes per element */
		4,
		/* Flags */
		PFF_INTEGER | PFF_SIGNED,
		/* Component type and count */
		PCT_SHORT,
		2,
		/* rbits, gbits, bbits, abits */
		16,
		16,
		0,
		0,
		/* Masks and shifts */
		0x0000FFFF,
		0xFFFF0000,
		0,
		0,
		0,
		16,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RGBA16I",
		/* Bytes per element */
		8,
		/* Flags */
		PFF_INTEGER | PFF_SIGNED | PFF_HASALPHA,
		/* Component type and count */
		PCT_SHORT,
		4,
		/* rbits, gbits, bbits, abits */
		16,
		16,
		16,
		16,
		/* Masks and shifts */
		0x0000FFFF,
		0xFFFF0000,
		0x0000FFFF,
		0xFFFF0000,
		0,
		16,
		0,
		16,
	},
	//-----------------------------------------------------------------------
	{
		"PF_R16U",
		/* Bytes per element */
		2,
		/* Flags */
		PFF_INTEGER,
		/* Component type and count */
		PCT_SHORT,
		1,
		/* rbits, gbits, bbits, abits */
		16,
		0,
		0,
		0,
		/* Masks and shifts */
		0x0000FFFF,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RG16U",
		/* Bytes per element */
		4,
		/* Flags */
		PFF_INTEGER,
		/* Component type and count */
		PCT_SHORT,
		2,
		/* rbits, gbits, bbits, abits */
		16,
		16,
		0,
		0,
		/* Masks and shifts */
		0x0000FFFF,
		0xFFFF0000,
		0,
		0,
		0,
		16,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RGBA16U",
		/* Bytes per element */
		8,
		/* Flags */
		PFF_INTEGER | PFF_HASALPHA,
		/* Component type and count */
		PCT_SHORT,
		4,
		/* rbits, gbits, bbits, abits */
		16,
		16,
		16,
		16,
		/* Masks and shifts */
		0x0000FFFF,
		0xFFFF0000,
		0x0000FFFF,
		0xFFFF0000,
		0,
		16,
		0,
		16,
	},
	//-----------------------------------------------------------------------
	{
		"PF_R32I",
		/* Bytes per element */
		4,
		/* Flags */
		PFF_INTEGER,
		/* Component type and count */
		PCT_INT,
		1,
		/* rbits, gbits, bbits, abits */
		32,
		0,
		0,
		0,
		/* Masks and shifts */
		0xFFFFFFFF,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RG32I",
		/* Bytes per element */
		8,
		/* Flags */
		PFF_INTEGER | PFF_SIGNED,
		/* Component type and count */
		PCT_INT,
		2,
		/* rbits, gbits, bbits, abits */
		32,
		32,
		0,
		0,
		/* Masks and shifts */
		0xFFFFFFFF,
		0xFFFFFFFF,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RGB32I",
		/* Bytes per element */
		12,
		/* Flags */
		PFF_INTEGER | PFF_SIGNED,
		/* Component type and count */
		PCT_INT,
		3,
		/* rbits, gbits, bbits, abits */
		32,
		32,
		32,
		0,
		/* Masks and shifts */
		0xFFFFFFFF,
		0xFFFFFFFF,
		0xFFFFFFFF,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{ "PF_RGBA32I",
	  /* Bytes per element */
	  16,
	  /* Flags */
	  PFF_INTEGER | PFF_SIGNED | PFF_HASALPHA,
	  /* Component type and count */
	  PCT_INT, 4,
	  /* rbits, gbits, bbits, abits */
	  32, 32, 32, 32,
	  /* Masks and shifts */
	  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	  0, 0, 0, 0 },
	//-----------------------------------------------------------------------
	{
		"PF_R32U",
		/* Bytes per element */
		4,
		/* Flags */
		PFF_INTEGER,
		/* Component type and count */
		PCT_INT,
		1,
		/* rbits, gbits, bbits, abits */
		32,
		0,
		0,
		0,
		/* Masks and shifts */
		0xFFFFFFFF,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RG32U",
		/* Bytes per element */
		8,
		/* Flags */
		PFF_INTEGER,
		/* Component type and count */
		PCT_INT,
		2,
		/* rbits, gbits, bbits, abits */
		32,
		32,
		0,
		0,
		/* Masks and shifts */
		0xFFFFFFFF,
		0xFFFFFFFF,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RGB32U",
		/* Bytes per element */
		12,
		/* Flags */
		PFF_INTEGER,
		/* Component type and count */
		PCT_INT,
		3,
		/* rbits, gbits, bbits, abits */
		32,
		32,
		32,
		0,
		/* Masks and shifts */
		0xFFFFFFFF,
		0xFFFFFFFF,
		0xFFFFFFFF,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{ "PF_RGBA32U",
	  /* Bytes per element */
	  16,
	  /* Flags */
	  PFF_INTEGER | PFF_HASALPHA,
	  /* Component type and count */
	  PCT_INT, 4,
	  /* rbits, gbits, bbits, abits */
	  32, 32, 32, 32,
	  /* Masks and shifts */
	  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	  0, 0, 0, 0 },
	//-----------------------------------------------------------------------
	{
		"PF_R16S",
		/* Bytes per element */
		2,
		/* Flags */
		PFF_INTEGER | PFF_NORMALIZED | PFF_SIGNED,
		/* Component type and count */
		PCT_SHORT,
		1,
		/* rbits, gbits, bbits, abits */
		16,
		0,
		0,
		0,
		/* Masks and shifts */
		0x0000FFFF,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RG16S",
		/* Bytes per element */
		4,
		/* Flags */
		PFF_INTEGER | PFF_NORMALIZED | PFF_SIGNED,
		/* Component type and count */
		PCT_SHORT,
		2,
		/* rbits, gbits, bbits, abits */
		16,
		16,
		0,
		0,
		/* Masks and shifts */
		0x0000FFFF,
		0xFFFF0000,
		0,
		0,
		0,
		16,
		0,
		0,
	},
	//-----------------------------------------------------------------------
	{
		"PF_RGBA16S",
		/* Bytes per element */
		8,
		/* Flags */
		PFF_INTEGER | PFF_NORMALIZED | PFF_SIGNED | PFF_HASALPHA,
		/* Component type and count */
		PCT_SHORT,
		4,
		/* rbits, gbits, bbits, abits */
		16,
		16,
		16,
		16,
		/* Masks and shifts */
		0x0000FFFF,
		0xFFFF0000,
		0x0000FFFF,
		0xFFFF0000,
		0,
		16,
		0,
		16,
	},
	//-----------------------------------------------------------------------
	{ "PF_R16",
	  /* Bytes per element */
	  2,
	  /* Flags */
	  PFF_INTEGER | PFF_NORMALIZED,
	  /* Component type and count */
	  PCT_SHORT, 1,
	  /* rbits, gbits, bbits, abits */
	  16, 0, 0, 0,
	  /* Masks and shifts */
	  0x0000FFFF, 0, 0, 0,
	  0, 0, 0, 0 },
	//-----------------------------------------------------------------------
	{ "PF_RG16",
	  /* Bytes per element */
	  4,
	  /* Flags */
	  PFF_INTEGER | PFF_NORMALIZED,
	  /* Component type and count */
	  PCT_SHORT, 2,
	  /* rbits, gbits, bbits, abits */
	  16, 16, 0, 0,
	  /* Masks and shifts */
	  0x0000FFFF, 0xFFFF0000, 0, 0,
	  0, 16, 0, 0 },
	//-----------------------------------------------------------------------
	{ "PF_RGB16",
	  /* Bytes per element */
	  6,
	  /* Flags */
	  PFF_INTEGER | PFF_NORMALIZED,
	  /* Component type and count */
	  PCT_SHORT, 3,
	  /* rbits, gbits, bbits, abits */
	  16, 16, 16, 0,
	  /* Masks and shifts */
	  0x0000FFFF, 0xFFFF0000, 0x0000FFFF, 0,
	  0, 16, 0, 0 },
	//-----------------------------------------------------------------------
	{ "PF_RGBA16",
	  /* Bytes per element */
	  8,
	  /* Flags */
	  PFF_INTEGER | PFF_NORMALIZED | PFF_HASALPHA,
	  /* Component type and count */
	  PCT_SHORT, 4,
	  /* rbits, gbits, bbits, abits */
	  16, 16, 16, 16,
	  /* Masks and shifts */
	  0x0000FFFF, 0xFFFF0000, 0x0000FFFF, 0xFFFF0000,
	  0, 16, 0, 16 },
};

static inline const PixelFormatDescription& getDescriptionFor(const PixelFormat fmt)
{
	const int ord = (int)fmt;
	assert(ord >= 0 && ord < PF_COUNT);

	return _pixelFormats[ord];
}

/**	Handles compression output from NVTT library for a single image. */
struct NVTTCompressOutputHandler : public nvtt::OutputHandler
{
	NVTTCompressOutputHandler(u8* buffer, u32 sizeBytes)
		: Buffer(buffer), BufferWritePos(buffer), BufferEnd(buffer + sizeBytes)
	{}

	void beginImage(int size, int width, int height, int depth, int face, int miplevel) override
	{}

	bool writeData(const void* data, int size) override
	{
		assert((BufferWritePos + size) <= BufferEnd);
		memcpy(BufferWritePos, data, size);
		BufferWritePos += size;

		return true;
	}

	void endImage() override
	{}

	u8* Buffer;
	u8* BufferWritePos;
	u8* BufferEnd;
};

/**	Handles output from NVTT library for a mip-map chain. */
struct NVTTMipmapOutputHandler : public nvtt::OutputHandler
{
	NVTTMipmapOutputHandler(const Vector<SPtr<PixelData>>& buffers)
		: Buffers(buffers), BufferWritePos(nullptr), BufferEnd(nullptr)
	{}

	void beginImage(int size, int width, int height, int depth, int face, int miplevel) override
	{
		assert(miplevel >= 0 && miplevel < (int)Buffers.size());
		assert((u32)size == Buffers[miplevel]->GetConsecutiveSize());

		ActiveBuffer = Buffers[miplevel];

		BufferWritePos = ActiveBuffer->GetData();
		BufferEnd = BufferWritePos + ActiveBuffer->GetConsecutiveSize();
	}

	bool writeData(const void* data, int size) override
	{
		assert((BufferWritePos + size) <= BufferEnd);
		memcpy(BufferWritePos, data, size);
		BufferWritePos += size;

		return true;
	}

	void endImage() override
	{}

	Vector<SPtr<PixelData>> Buffers;
	SPtr<PixelData> ActiveBuffer;

	u8* BufferWritePos;
	u8* BufferEnd;
};

nvtt::Format toNVTTFormat(PixelFormat format)
{
	switch(format)
	{
	case PF_BC1:
		return nvtt::Format_BC1;
	case PF_BC1a:
		return nvtt::Format_BC1a;
	case PF_BC2:
		return nvtt::Format_BC2;
	case PF_BC3:
		return nvtt::Format_BC3;
	case PF_BC4:
		return nvtt::Format_BC4;
	case PF_BC5:
		return nvtt::Format_BC5;
	case PF_BC6H:
		return nvtt::Format_BC6;
	case PF_BC7:
		return nvtt::Format_BC7;
	default: // Unsupported format
		return nvtt::Format_BC3;
	}
}

nvtt::Quality toNVTTQuality(CompressionQuality quality)
{
	switch(quality)
	{
	case CompressionQuality::Fastest:
		return nvtt::Quality_Fastest;
	case CompressionQuality::Highest:
		return nvtt::Quality_Highest;
	case CompressionQuality::Normal:
		return nvtt::Quality_Normal;
	case CompressionQuality::Production:
		return nvtt::Quality_Normal;
	}

	// Unknown quality level
	return nvtt::Quality_Normal;
}

nvtt::AlphaMode toNVTTAlphaMode(AlphaMode alphaMode)
{
	switch(alphaMode)
	{
	case AlphaMode::None:
		return nvtt::AlphaMode_None;
	case AlphaMode::Premultiplied:
		return nvtt::AlphaMode_Premultiplied;
	case AlphaMode::Transparency:
		return nvtt::AlphaMode_Transparency;
	}

	// Unknown alpha mode
	return nvtt::AlphaMode_None;
}

nvtt::WrapMode toNVTTWrapMode(MipMapWrapMode wrapMode)
{
	switch(wrapMode)
	{
	case MipMapWrapMode::Clamp:
		return nvtt::WrapMode_Clamp;
	case MipMapWrapMode::Mirror:
		return nvtt::WrapMode_Mirror;
	case MipMapWrapMode::Repeat:
		return nvtt::WrapMode_Repeat;
	}

	// Unknown alpha mode
	return nvtt::WrapMode_Mirror;
}

u32 PixelUtil::GetNumElemBytes(PixelFormat format)
{
	return getDescriptionFor(format).ElemBytes;
}

u32 PixelUtil::GetBlockSize(PixelFormat format)
{
	switch(format)
	{
	case PF_BC1:
	case PF_BC1a:
	case PF_BC4:
		return 8;
	case PF_BC2:
	case PF_BC3:
	case PF_BC5:
	case PF_BC6H:
	case PF_BC7:
		return 16;
	default:
		return GetNumElemBytes(format);
	}
}

Vector2I PixelUtil::GetBlockDimensions(PixelFormat format)
{
	switch(format)
	{
	case PF_BC1:
	case PF_BC1a:
	case PF_BC4:
	case PF_BC2:
	case PF_BC3:
	case PF_BC5:
	case PF_BC6H:
	case PF_BC7:
		return Vector2I(4, 4);
	default:
		return Vector2I(1, 1);
	}
}

u32 PixelUtil::GetMemorySize(u32 width, u32 height, u32 depth, PixelFormat format)
{
	if(IsCompressed(format))
	{
		switch(format)
		{
		// BC formats work by dividing the image into 4x4 blocks
		case PF_BC1:
		case PF_BC1a:
		case PF_BC4:
		case PF_BC2:
		case PF_BC3:
		case PF_BC5:
		case PF_BC6H:
		case PF_BC7:
			width = Math::DivideAndRoundUp(width, 4U);
			height = Math::DivideAndRoundUp(height, 4U);
			break;
		default:
			break;
		}
	}

	return width * height * depth * GetBlockSize(format);
}

void PixelUtil::GetPitch(u32 width, u32 height, u32 depth, PixelFormat format, u32& rowPitch, u32& depthPitch)
{
	u32 blockSize = GetBlockSize(format);

	if(IsCompressed(format))
	{
		switch(format)
		{
			// BC formats work by dividing the image into 4x4 blocks
		case PF_BC1:
		case PF_BC1a:
		case PF_BC4:
		case PF_BC2:
		case PF_BC3:
		case PF_BC5:
		case PF_BC6H:
		case PF_BC7:
			width = Math::DivideAndRoundUp(width, 4U);
			height = Math::DivideAndRoundUp(height, 4U);
			break;
		default:
			break;
		}
	}

	rowPitch = width * blockSize;
	depthPitch = width * height * blockSize;
}

void PixelUtil::GetSizeForMipLevel(u32 width, u32 height, u32 depth, u32 mipLevel, u32& mipWidth, u32& mipHeight, u32& mipDepth)
{
	mipWidth = width;
	mipHeight = height;
	mipDepth = depth;

	for(u32 i = 0; i < mipLevel; i++)
	{
		if(mipWidth != 1) mipWidth /= 2;
		if(mipHeight != 1) mipHeight /= 2;
		if(mipDepth != 1) mipDepth /= 2;
	}
}

u32 PixelUtil::GetNumElemBits(PixelFormat format)
{
	return getDescriptionFor(format).ElemBytes * 8;
}

u32 PixelUtil::GetFlags(PixelFormat format)
{
	return getDescriptionFor(format).Flags;
}

bool PixelUtil::HasAlpha(PixelFormat format)
{
	return (PixelUtil::GetFlags(format) & PFF_HASALPHA) > 0;
}

bool PixelUtil::IsFloatingPoint(PixelFormat format)
{
	return (PixelUtil::GetFlags(format) & PFF_FLOAT) > 0;
}

bool PixelUtil::IsCompressed(PixelFormat format)
{
	return (PixelUtil::GetFlags(format) & PFF_COMPRESSED) > 0;
}

bool PixelUtil::IsNormalized(PixelFormat format)
{
	return (PixelUtil::GetFlags(format) & PFF_NORMALIZED) > 0;
}

bool PixelUtil::IsDepth(PixelFormat format)
{
	return (PixelUtil::GetFlags(format) & PFF_DEPTH) > 0;
}

bool PixelUtil::CheckFormat(PixelFormat& format, TextureType texType, int usage)
{
	// First check just the usage since it's the most limiting factor

	//// Depth-stencil only supports depth formats
	if((usage & TU_DEPTHSTENCIL) != 0)
	{
		if(IsDepth(format))
			return true;

		format = PF_D32_S8X24;
		return false;
	}

	//// Render targets support everything but compressed & depth-stencil formats
	if((usage & TU_RENDERTARGET) != 0)
	{
		if(!IsDepth(format) && !IsCompressed(format))
			return true;

		format = PF_RGBA8;
		return false;
	}

	//// Load-store textures support everything but compressed & depth-stencil formats
	if((usage & TU_LOADSTORE) != 0)
	{
		if(!IsDepth(format) && !IsCompressed(format))
			return true;

		format = PF_RGBA8;
		return false;
	}

	//// Sampled texture support depends on texture type
	switch(texType)
	{
	case TEX_TYPE_1D:
		{
			// 1D textures support anything but depth & compressed formats
			if(!IsDepth(format) && !IsCompressed(format))
				return true;

			format = PF_RGBA8;
			return false;
		}
	case TEX_TYPE_3D:
		{
			// 3D textures support anything but depth & compressed formats
			if(!IsDepth(format))
				return true;

			format = PF_RGBA8;
			return false;
		}
	default: // 2D & cube
		{
			// 2D/cube textures support anything but depth formats
			if(!IsDepth(format))
				return true;

			format = PF_RGBA8;
			return false;
		}
	}
}

bool PixelUtil::IsValidExtent(u32 width, u32 height, u32 depth, PixelFormat format)
{
	if(IsCompressed(format))
	{
		switch(format)
		{
		case PF_BC1:
		case PF_BC2:
		case PF_BC1a:
		case PF_BC3:
		case PF_BC4:
		case PF_BC5:
		case PF_BC6H:
		case PF_BC7:
			return ((width & 3) == 0 && (height & 3) == 0 && depth == 1);
		default:
			return true;
		}
	}
	else
	{
		return true;
	}
}

void PixelUtil::GetBitDepths(PixelFormat format, int (&rgba)[4])
{
	const PixelFormatDescription& des = getDescriptionFor(format);
	rgba[0] = des.Rbits;
	rgba[1] = des.Gbits;
	rgba[2] = des.Bbits;
	rgba[3] = des.Abits;
}

void PixelUtil::GetBitMasks(PixelFormat format, u32 (&rgba)[4])
{
	const PixelFormatDescription& des = getDescriptionFor(format);
	rgba[0] = des.Rmask;
	rgba[1] = des.Gmask;
	rgba[2] = des.Bmask;
	rgba[3] = des.Amask;
}

void PixelUtil::GetBitShifts(PixelFormat format, u8 (&rgba)[4])
{
	const PixelFormatDescription& des = getDescriptionFor(format);
	rgba[0] = des.Rshift;
	rgba[1] = des.Gshift;
	rgba[2] = des.Bshift;
	rgba[3] = des.Ashift;
}

String PixelUtil::GetFormatName(PixelFormat srcformat)
{
	return getDescriptionFor(srcformat).Name;
}

bool PixelUtil::IsAccessible(PixelFormat srcformat)
{
	if(srcformat == PF_UNKNOWN)
		return false;

	u32 flags = GetFlags(srcformat);
	return !((flags & PFF_COMPRESSED) || (flags & PFF_DEPTH));
}

PixelComponentType PixelUtil::GetElementType(PixelFormat format)
{
	const PixelFormatDescription& des = getDescriptionFor(format);
	return des.ComponentType;
}

u32 PixelUtil::GetNumElements(PixelFormat format)
{
	const PixelFormatDescription& des = getDescriptionFor(format);
	return des.ComponentCount;
}

u32 PixelUtil::GetMaxMipmaps(u32 width, u32 height, u32 depth, PixelFormat format)
{
	u32 count = 0;
	if((width > 0) && (height > 0))
	{
		while(!(width == 1 && height == 1 && depth == 1))
		{
			if(width > 1) width = width / 2;
			if(height > 1) height = height / 2;
			if(depth > 1) depth = depth / 2;

			count++;
		}
	}

	return count;
}

void PixelUtil::PackColor(const Color& color, PixelFormat format, void* dest)
{
	PackColor(color.R, color.G, color.B, color.A, format, dest);
}

void PixelUtil::PackColor(u8 r, u8 g, u8 b, u8 a, PixelFormat format, void* dest)
{
	const PixelFormatDescription& des = getDescriptionFor(format);

	if(des.Flags & PFF_INTEGER)
	{
		// Shortcut for integer formats packing
		u32 value = ((Bitwise::FixedToFixed(r, 8, des.Rbits) << des.Rshift) & des.Rmask) |
			((Bitwise::FixedToFixed(g, 8, des.Gbits) << des.Gshift) & des.Gmask) |
			((Bitwise::FixedToFixed(b, 8, des.Bbits) << des.Bshift) & des.Bmask) |
			((Bitwise::FixedToFixed(a, 8, des.Abits) << des.Ashift) & des.Amask);

		// And write to memory
		Bitwise::IntWrite(dest, des.ElemBytes, value);
	}
	else
	{
		// Convert to float
		PackColor((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)a / 255.0f, format, dest);
	}
}

void PixelUtil::PackColor(float r, float g, float b, float a, const PixelFormat format, void* dest)
{
	// Special cases
	if(format == PF_RG11B10F)
	{
		u32 value;
		value = Bitwise::FloatToFloat11(r);
		value |= Bitwise::FloatToFloat11(g) << 11;
		value |= Bitwise::FloatToFloat10(b) << 22;

		((u32*)dest)[0] = value;
		return;
	}

	if(format == PF_RGB10A2)
	{
		BS_LOG(Error, PixelUtility, "packColor() not implemented for format \"{0}\".", GetFormatName(PF_RGB10A2));
		return;
	}

	// All other formats handled in a generic way
	const PixelFormatDescription& des = getDescriptionFor(format);
	assert(des.ComponentCount <= 4);

	float inputs[] = { r, g, b, a };
	u8 bits[] = { des.Rbits, des.Gbits, des.Bbits, des.Abits };
	u32 masks[] = { des.Rmask, des.Gmask, des.Bmask, des.Amask };
	u8 shifts[] = { des.Rshift, des.Gshift, des.Bshift, des.Ashift };

	memset(dest, 0, des.ElemBytes);

	u32 curBit = 0;
	u32 prevDword = 0;
	u32 dwordValue = 0;
	for(u32 i = 0; i < des.ComponentCount; i++)
	{
		u32 curDword = curBit / 32;

		// New dword reached, write current one and advance
		if(curDword > prevDword)
		{
			u32* curDst = ((u32*)dest) + prevDword;
			Bitwise::IntWrite(curDst, 4, dwordValue);

			dwordValue = 0;
			prevDword = curDword;
		}

		if(des.Flags & PFF_INTEGER)
		{
			if(des.Flags & PFF_NORMALIZED)
			{
				if(des.Flags & PFF_SIGNED)
					dwordValue |= (Bitwise::SnormToUint(inputs[i], bits[i]) << shifts[i]) & masks[i];
				else
					dwordValue |= (Bitwise::UnormToUint(inputs[i], bits[i]) << shifts[i]) & masks[i];
			}
			else
			{
				// Note: Casting integer to float. A better option would be to have a separate unpackColor that has
				// integer output parameters.
				dwordValue |= (((u32)inputs[i]) << shifts[i]) & masks[i];
			}
		}
		else if(des.Flags & PFF_FLOAT)
		{
			// Note: Not handling unsigned floats

			if(des.ComponentType == PCT_FLOAT16)
				dwordValue |= (Bitwise::FloatToHalf(inputs[i]) << shifts[i]) & masks[i];
			else
				dwordValue |= *(u32*)&inputs[i];
		}
		else
		{
			BS_LOG(Error, PixelUtility, "packColor() not implemented for format \"{0}\".", GetFormatName(format));
			return;
		}

		curBit += bits[i];
	}

	// Write last dword
	u32 numBytes = std::min((prevDword + 1) * 4, (u32)des.ElemBytes) - (prevDword * 4);
	u32* curDst = ((u32*)dest) + prevDword;
	Bitwise::IntWrite(curDst, numBytes, dwordValue);
}

void PixelUtil::UnpackColor(Color* color, PixelFormat format, const void* src)
{
	UnpackColor(&color->R, &color->G, &color->B, &color->A, format, src);
}

void PixelUtil::UnpackColor(u8* r, u8* g, u8* b, u8* a, PixelFormat format, const void* src)
{
	const PixelFormatDescription& des = getDescriptionFor(format);

	if(des.Flags & PFF_INTEGER)
	{
		// Shortcut for integer formats unpacking
		const u32 value = Bitwise::IntRead(src, des.ElemBytes);

		*r = (u8)Bitwise::FixedToFixed((value & des.Rmask) >> des.Rshift, des.Rbits, 8);
		*g = (u8)Bitwise::FixedToFixed((value & des.Gmask) >> des.Gshift, des.Gbits, 8);
		*b = (u8)Bitwise::FixedToFixed((value & des.Bmask) >> des.Bshift, des.Bbits, 8);

		if(des.Flags & PFF_HASALPHA)
		{
			*a = (u8)Bitwise::FixedToFixed((value & des.Amask) >> des.Ashift, des.Abits, 8);
		}
		else
		{
			*a = 255; // No alpha, default a component to full
		}
	}
	else
	{
		// Do the operation with the more generic floating point
		float rr, gg, bb, aa;
		UnpackColor(&rr, &gg, &bb, &aa, format, src);

		*r = (u8)Bitwise::UnormToUint(rr, 8);
		*g = (u8)Bitwise::UnormToUint(gg, 8);
		*b = (u8)Bitwise::UnormToUint(bb, 8);
		*a = (u8)Bitwise::UnormToUint(aa, 8);
	}
}

void PixelUtil::UnpackColor(float* r, float* g, float* b, float* a, PixelFormat format, const void* src)
{
	// Special cases
	if(format == PF_RG11B10F)
	{
		u32 value = ((u32*)src)[0];
		*r = Bitwise::Float11ToFloat(value);
		*g = Bitwise::Float11ToFloat(value >> 11);
		*b = Bitwise::Float10ToFloat(value >> 22);

		return;
	}

	if(format == PF_RGB10A2)
	{
		BS_LOG(Error, PixelUtility, "unpackColor() not implemented for format \"{0}\".", GetFormatName(PF_RGB10A2));
		return;
	}

	// All other formats handled in a generic way
	const PixelFormatDescription& des = getDescriptionFor(format);
	assert(des.ComponentCount <= 4);

	float* outputs[] = { r, g, b, a };
	u8 bits[] = { des.Rbits, des.Gbits, des.Bbits, des.Abits };
	u32 masks[] = { des.Rmask, des.Gmask, des.Bmask, des.Amask };
	u8 shifts[] = { des.Rshift, des.Gshift, des.Bshift, des.Ashift };

	u32 curBit = 0;
	for(u32 i = 0; i < des.ComponentCount; i++)
	{
		u32 curDword = curBit / 32;
		u32 numBytes = std::min((curDword + 1) * 4, (u32)des.ElemBytes) - (curDword * 4);

		u32* curSrc = ((u32*)src) + curDword;
		u32 value = Bitwise::IntRead(curSrc, numBytes);
		if(des.Flags & PFF_INTEGER)
		{
			if(des.Flags & PFF_NORMALIZED)
			{
				if(des.Flags & PFF_SIGNED)
					*outputs[i] = Bitwise::UintToSnorm((value & masks[i]) >> shifts[i], bits[i]);
				else
					*outputs[i] = Bitwise::UintToUnorm((value & masks[i]) >> shifts[i], bits[i]);
			}
			else
			{
				// Note: Casting integer to float. A better option would be to have a separate unpackColor that has
				// integer output parameters.
				*outputs[i] = (float)((value & masks[i]) >> shifts[i]);
			}
		}
		else if(des.Flags & PFF_FLOAT)
		{
			// Note: Not handling unsigned floats

			if(des.ComponentType == PCT_FLOAT16)
				*outputs[i] = Bitwise::HalfToFloat((u16)((value & masks[i]) >> shifts[i]));
			else
				*outputs[i] = *(float*)&value;
		}
		else
		{
			BS_LOG(Error, PixelUtility, "unpackColor() not implemented for format \"{0}\".", GetFormatName(format));
			return;
		}

		curBit += bits[i];
	}

	// Fill empty components
	for(u32 i = des.ComponentCount; i < 3; i++)
		*outputs[i] = 0.0f;

	if(des.ComponentCount < 4)
		*outputs[3] = 1.0f;
}

void PixelUtil::PackDepth(float depth, const PixelFormat format, void* dest)
{
	if(!IsDepth(format))
	{
		BS_LOG(Error, PixelUtility, "Cannot convert depth to {0}: it is not a depth format", GetFormatName(format));
		return;
	}

	BS_LOG(Error, PixelUtility, "Method is not implemented");
	// TODO implement depth packing
}

float PixelUtil::UnpackDepth(PixelFormat format, void* src)
{
	if(!IsDepth(format))
	{
		BS_LOG(Error, PixelUtility, "Cannot unpack from {0}: it is not a depth format", GetFormatName(format));
		return 0;
	}

	u32* color = (u32*)src;
	u32 masked = 0;
	switch(format)
	{
	case PF_D24S8:
		return static_cast<float>(*color & 0x00FFFFFF) / (float)16777216;
		break;
	case PF_D16:
		return static_cast<float>(*color & 0xFFFF) / (float)65536;
		break;
	case PF_D32:
		masked = *color & 0xFFFFFFFF;
		return *((float*)&masked);
		break;
	case PF_D32_S8X24:
		masked = *color & 0xFFFFFFFF;
		return *((float*)&masked);
		break;
	default:
		BS_LOG(Error, PixelUtility, "Cannot unpack from {0}", GetFormatName(format));
		return 0;
		break;
	}
}

void PixelUtil::BulkPixelConversion(const PixelData& src, PixelData& dst)
{
	if(src.GetWidth() != dst.GetWidth() || src.GetHeight() != dst.GetHeight() || src.GetDepth() != dst.GetDepth())
	{
		BS_LOG(Error, PixelUtility, "Cannot convert pixels between buffers of different sizes.");
		return;
	}

	// The easy case
	if(src.GetFormat() == dst.GetFormat())
	{
		// Everything consecutive?
		if(src.IsConsecutive() && dst.IsConsecutive())
		{
			memcpy(dst.GetData(), src.GetData(), src.GetConsecutiveSize());
			return;
		}

		PixelFormat format = src.GetFormat();
		u32 pixelSize = GetNumElemBytes(format);

		Vector2I blockDim = GetBlockDimensions(format);
		if(IsCompressed(format))
		{
			u32 blockSize = GetBlockSize(format);
			pixelSize = blockSize / blockDim.X;

			if(src.GetLeft() % blockDim.X != 0 || src.GetTop() % blockDim.Y != 0)
			{
				BS_LOG(Error, PixelUtility, "Source offset must be a multiple of block size for compressed formats.");
			}

			if(dst.GetLeft() % blockDim.X != 0 || dst.GetTop() % blockDim.Y != 0)
			{
				BS_LOG(Error, PixelUtility, "Destination offset must be a multiple of block size for compressed formats.");
			}
		}

		u8* srcPtr = static_cast<u8*>(src.GetData()) + src.GetLeft() * pixelSize + src.GetTop() * src.GetRowPitch() + src.GetFront() * src.GetSlicePitch();
		u8* dstPtr = static_cast<u8*>(dst.GetData()) + dst.GetLeft() * pixelSize + dst.GetTop() * dst.GetRowPitch() + dst.GetFront() * dst.GetSlicePitch();

		// Get pitches+skips in bytes
		const u32 srcRowPitchBytes = src.GetRowPitch();
		const u32 srcSliceSkipBytes = src.GetSliceSkip();

		const u32 dstRowPitchBytes = dst.GetRowPitch();
		const u32 dstSliceSkipBytes = dst.GetSliceSkip();

		// Otherwise, copy per row
		const u32 rowSize = src.GetWidth() * pixelSize;
		for(u32 z = src.GetFront(); z < src.GetBack(); z++)
		{
			for(u32 y = src.GetTop(); y < src.GetBottom(); y += blockDim.Y)
			{
				memcpy(dstPtr, srcPtr, rowSize);

				srcPtr += srcRowPitchBytes;
				dstPtr += dstRowPitchBytes;
			}

			srcPtr += srcSliceSkipBytes;
			dstPtr += dstSliceSkipBytes;
		}

		return;
	}

	// Check for compressed formats, we don't support decompression
	if(IsCompressed(src.GetFormat()))
	{
		if(src.GetFormat() != dst.GetFormat())
		{
			BS_LOG(Error, PixelUtility, "Cannot convert from a compressed format to another format.");
			return;
		}
	}

	// Check for compression
	if(IsCompressed(dst.GetFormat()))
	{
		if(src.GetFormat() != dst.GetFormat())
		{
			CompressionOptions co;
			co.Format = dst.GetFormat();
			Compress(src, dst, co);

			return;
		}
	}

	u32 srcPixelSize = GetNumElemBytes(src.GetFormat());
	u32 dstPixelSize = GetNumElemBytes(dst.GetFormat());
	u8* srcptr = static_cast<u8*>(src.GetData()) + src.GetLeft() * srcPixelSize + src.GetTop() * src.GetRowPitch() + src.GetFront() * src.GetSlicePitch();
	u8* dstptr = static_cast<u8*>(dst.GetData()) + dst.GetLeft() * dstPixelSize + dst.GetTop() * dst.GetRowPitch() + dst.GetFront() * dst.GetSlicePitch();

	// Get pitches+skips in bytes
	u32 srcRowSkipBytes = src.GetRowSkip();
	u32 srcSliceSkipBytes = src.GetSliceSkip();
	u32 dstRowSkipBytes = dst.GetRowSkip();
	u32 dstSliceSkipBytes = dst.GetSliceSkip();

	// The brute force fallback
	float r, g, b, a;
	for(u32 z = src.GetFront(); z < src.GetBack(); z++)
	{
		for(u32 y = src.GetTop(); y < src.GetBottom(); y++)
		{
			for(u32 x = src.GetLeft(); x < src.GetRight(); x++)
			{
				UnpackColor(&r, &g, &b, &a, src.GetFormat(), srcptr);
				PackColor(r, g, b, a, dst.GetFormat(), dstptr);

				srcptr += srcPixelSize;
				dstptr += dstPixelSize;
			}

			srcptr += srcRowSkipBytes;
			dstptr += dstRowSkipBytes;
		}

		srcptr += srcSliceSkipBytes;
		dstptr += dstSliceSkipBytes;
	}
}

void PixelUtil::FlipComponentOrder(PixelData& data)
{
	if(IsCompressed(data.GetFormat()))
	{
		BS_LOG(Error, PixelUtility, "flipComponentOrder() not supported on compressed images.");
		return;
	}

	const PixelFormatDescription& pfd = getDescriptionFor(data.GetFormat());
	if(pfd.ElemBytes > 4)
	{
		BS_LOG(Error, PixelUtility, "flipComponentOrder() only supported on 4 byte or smaller pixel formats.");
		return;
	}

	if(pfd.ComponentCount <= 1) // Nothing to flip
		return;

	bool bitCountMismatch = false;
	if(pfd.Rbits != pfd.Gbits)
		bitCountMismatch = true;

	if(pfd.ComponentCount > 2 && pfd.Rbits != pfd.Bbits)
		bitCountMismatch = true;

	if(pfd.ComponentCount > 3 && pfd.Rbits != pfd.Abits)
		bitCountMismatch = true;

	if(bitCountMismatch)
	{
		BS_LOG(Error, PixelUtility, "flipComponentOrder() not supported for formats that don't have the same number "
									"of bytes for all components.");
		return;
	}

	struct CompData
	{
		u32 Mask;
		u8 Shift;
	};

	std::array<CompData, 4> compData = { { { pfd.Rmask, pfd.Rshift },
										   { pfd.Gmask, pfd.Gshift },
										   { pfd.Bmask, pfd.Bshift },
										   { pfd.Amask, pfd.Ashift } } };

	// Ensure unused components are at the end, after sort
	if(pfd.ComponentCount < 4)
		compData[3].Shift = 0xFF;

	if(pfd.ComponentCount < 3)
		compData[2].Shift = 0xFF;

	std::sort(compData.begin(), compData.end(), [&](const CompData& lhs, const CompData& rhs)
			  { return lhs.Shift < rhs.Shift; });

	u8* dataPtr = data.GetData();

	u32 pixelSize = pfd.ElemBytes;
	u32 rowSkipBytes = data.GetRowSkip();
	u32 sliceSkipBytes = data.GetSliceSkip();

	for(u32 z = 0; z < data.GetDepth(); z++)
	{
		for(u32 y = 0; y < data.GetHeight(); y++)
		{
			for(u32 x = 0; x < data.GetWidth(); x++)
			{
				if(pfd.ComponentCount == 2)
				{
					u64 pixelData = 0;
					memcpy(&pixelData, dataPtr, pixelSize);

					u64 output = 0;
					output |= (pixelData & compData[1].Mask) >> compData[1].Shift;
					output |= (pixelData & compData[0].Mask) << compData[1].Shift;

					memcpy(dataPtr, &output, pixelSize);
				}
				else if(pfd.ComponentCount == 3)
				{
					u64 pixelData = 0;
					memcpy(&pixelData, dataPtr, pixelSize);

					u64 output = 0;
					output |= (pixelData & compData[2].Mask) >> compData[2].Shift;
					output |= (pixelData & compData[0].Mask) << compData[2].Shift;

					memcpy(dataPtr, &output, pixelSize);
				}
				else if(pfd.ComponentCount == 4)
				{
					u64 pixelData = 0;
					memcpy(&pixelData, dataPtr, pixelSize);

					u64 output = 0;
					output |= (pixelData & compData[3].Mask) >> compData[3].Shift;
					output |= (pixelData & compData[0].Mask) << compData[3].Shift;

					output |= (pixelData & compData[2].Mask) >> (compData[2].Shift - compData[1].Shift);
					output |= (pixelData & compData[1].Mask) << (compData[2].Shift - compData[1].Shift);

					memcpy(dataPtr, &output, pixelSize);
				}

				dataPtr += pixelSize;
			}

			dataPtr += rowSkipBytes;
		}

		dataPtr += sliceSkipBytes;
	}
}

void PixelUtil::Scale(const PixelData& src, PixelData& scaled, Filter filter)
{
	assert(PixelUtil::IsAccessible(src.GetFormat()));
	assert(PixelUtil::IsAccessible(scaled.GetFormat()));

	PixelData temp;
	switch(filter)
	{
	default:
	case FILTER_NEAREST:
		if(src.GetFormat() == scaled.GetFormat())
		{
			// No intermediate buffer needed
			temp = scaled;
		}
		else
		{
			// Allocate temporary buffer of destination size in source format
			temp = PixelData(scaled.GetWidth(), scaled.GetHeight(), scaled.GetDepth(), src.GetFormat());
			temp.AllocateInternalBuffer();
		}

		// No conversion
		switch(PixelUtil::GetNumElemBytes(src.GetFormat()))
		{
		case 1: NearestResampler<1>::Scale(src, temp); break;
		case 2: NearestResampler<2>::Scale(src, temp); break;
		case 3: NearestResampler<3>::Scale(src, temp); break;
		case 4: NearestResampler<4>::Scale(src, temp); break;
		case 6: NearestResampler<6>::Scale(src, temp); break;
		case 8: NearestResampler<8>::Scale(src, temp); break;
		case 12: NearestResampler<12>::Scale(src, temp); break;
		case 16: NearestResampler<16>::Scale(src, temp); break;
		default:
			// Never reached
			assert(false);
		}

		if(temp.GetData() != scaled.GetData())
		{
			// Blit temp buffer
			PixelUtil::BulkPixelConversion(temp, scaled);

			temp.FreeInternalBuffer();
		}

		break;

	case FILTER_LINEAR:
		switch(src.GetFormat())
		{
		case PF_RG8:
		case PF_RGB8:
		case PF_BGR8:
		case PF_RGBA8:
		case PF_BGRA8:
			if(src.GetFormat() == scaled.GetFormat())
			{
				// No intermediate buffer needed
				temp = scaled;
			}
			else
			{
				// Allocate temp buffer of destination size in source format
				temp = PixelData(scaled.GetWidth(), scaled.GetHeight(), scaled.GetDepth(), src.GetFormat());
				temp.AllocateInternalBuffer();
			}

			// No conversion
			switch(PixelUtil::GetNumElemBytes(src.GetFormat()))
			{
			case 1: LinearResampler_Byte<1>::Scale(src, temp); break;
			case 2: LinearResampler_Byte<2>::Scale(src, temp); break;
			case 3: LinearResampler_Byte<3>::Scale(src, temp); break;
			case 4: LinearResampler_Byte<4>::Scale(src, temp); break;
			default:
				// Never reached
				assert(false);
			}

			if(temp.GetData() != scaled.GetData())
			{
				// Blit temp buffer
				PixelUtil::BulkPixelConversion(temp, scaled);
				temp.FreeInternalBuffer();
			}

			break;
		case PF_RGB32F:
		case PF_RGBA32F:
			if(scaled.GetFormat() == PF_RGB32F || scaled.GetFormat() == PF_RGBA32F)
			{
				// float32 to float32, avoid unpack/repack overhead
				LinearResampler_Float32::Scale(src, scaled);
				break;
			}
			// Else, fall through
		default:
			// Fallback case, slow but works
			LinearResampler::Scale(src, scaled);
		}
		break;
	}
}

void PixelUtil::Copy(const PixelData& src, PixelData& dst, u32 offsetX, u32 offsetY, u32 offsetZ)
{
	if(src.GetFormat() != dst.GetFormat())
	{
		BS_LOG(Error, PixelUtility, "Source format is different from destination format for copy(). This operation "
									"cannot be used for a format conversion. Aborting copy.");
		return;
	}

	u32 right = offsetX + dst.GetWidth();
	u32 bottom = offsetY + dst.GetHeight();
	u32 back = offsetZ + dst.GetDepth();

	if(right > src.GetWidth() || bottom > src.GetHeight() || back > src.GetDepth())
	{
		BS_LOG(Error, PixelUtility, "Provided offset or destination size is too large and is referencing pixels that "
									"are out of bounds on the source texture. Aborting copy().");
		return;
	}

	u8* srcPtr = (u8*)src.GetData() + offsetZ * src.GetSlicePitch();
	u8* dstPtr = (u8*)dst.GetData();

	u32 elemSize = GetNumElemBytes(dst.GetFormat());
	u32 rowSize = dst.GetWidth() * elemSize;

	for(u32 z = 0; z < dst.GetDepth(); z++)
	{
		u8* srcRowPtr = srcPtr + offsetY * src.GetRowPitch();
		u8* dstRowPtr = dstPtr;

		for(u32 y = 0; y < dst.GetHeight(); y++)
		{
			memcpy(dstRowPtr, srcRowPtr + offsetX * elemSize, rowSize);

			srcRowPtr += src.GetRowPitch();
			dstRowPtr += dst.GetRowPitch();
		}

		srcPtr += src.GetSlicePitch();
		dstPtr += dst.GetSlicePitch();
	}
}

void PixelUtil::Mirror(PixelData& pixelData, MirrorMode mode)
{
	u32 width = pixelData.GetWidth();
	u32 height = pixelData.GetHeight();
	u32 depth = pixelData.GetDepth();

	u32 elemSize = GetNumElemBytes(pixelData.GetFormat());

	if(mode.IsSet(MirrorModeBits::Z))
	{
		u32 sliceSize = width * height * elemSize;
		u8* sliceTemp = B3DStackAllocate<u8>(sliceSize);

		u8* dataPtr = pixelData.GetData();
		u32 halfDepth = depth / 2;
		for(u32 z = 0; z < halfDepth; z++)
		{
			u32 srcZ = z * sliceSize;
			u32 dstZ = (depth - z - 1) * sliceSize;

			memcpy(sliceTemp, &dataPtr[dstZ], sliceSize);
			memcpy(&dataPtr[dstZ], &dataPtr[srcZ], sliceSize);
			memcpy(&dataPtr[srcZ], sliceTemp, sliceSize);
		}

		// Note: If flipping Y or X as well I could do it here without an extra set of memcpys

		B3DStackFree(sliceTemp);
	}

	if(mode.IsSet(MirrorModeBits::Y))
	{
		u32 rowSize = width * elemSize;
		u8* rowTemp = B3DStackAllocate<u8>(rowSize);

		u8* slicePtr = pixelData.GetData();
		for(u32 z = 0; z < depth; z++)
		{
			u32 halfHeight = height / 2;
			for(u32 y = 0; y < halfHeight; y++)
			{
				u32 srcY = y * rowSize;
				u32 dstY = (height - y - 1) * rowSize;

				memcpy(rowTemp, &slicePtr[dstY], rowSize);
				memcpy(&slicePtr[dstY], &slicePtr[srcY], rowSize);
				memcpy(&slicePtr[srcY], rowTemp, rowSize);
			}

			// Note: If flipping X as well I could do it here without an extra set of memcpys

			slicePtr += pixelData.GetSlicePitch();
		}

		B3DStackFree(rowTemp);
	}

	if(mode.IsSet(MirrorModeBits::X))
	{
		u8* elemTemp = B3DStackAllocate<u8>(elemSize);

		u8* slicePtr = pixelData.GetData();
		for(u32 z = 0; z < depth; z++)
		{
			u8* rowPtr = slicePtr;
			for(u32 y = 0; y < height; y++)
			{
				u32 halfWidth = width / 2;
				for(u32 x = 0; x < halfWidth; x++)
				{
					u32 srcX = x * elemSize;
					u32 dstX = (width - x - 1) * elemSize;

					memcpy(elemTemp, &rowPtr[dstX], elemSize);
					memcpy(&rowPtr[dstX], &rowPtr[srcX], elemSize);
					memcpy(&rowPtr[srcX], elemTemp, elemSize);
				}

				rowPtr += pixelData.GetRowPitch();
			}

			slicePtr += pixelData.GetSlicePitch();
		}

		B3DStackFree(elemTemp);
	}
}

void PixelUtil::LinearToSrgb(PixelData& pixelData)
{
	u32 depth = pixelData.GetDepth();
	u32 height = pixelData.GetHeight();
	u32 width = pixelData.GetWidth();

	u32 pixelSize = PixelUtil::GetNumElemBytes(pixelData.GetFormat());
	u8* data = pixelData.GetData();

	for(u32 z = 0; z < depth; z++)
	{
		u32 zDataIdx = z * pixelData.GetSlicePitch();

		for(u32 y = 0; y < height; y++)
		{
			u32 yDataIdx = y * pixelData.GetRowPitch();

			for(u32 x = 0; x < width; x++)
			{
				u32 dataIdx = x * pixelSize + yDataIdx + zDataIdx;
				u8* dest = data + dataIdx;

				Color color;

				PixelUtil::UnpackColor(&color, pixelData.GetFormat(), dest);
				color = color.GetGamma();
				PixelUtil::PackColor(color, pixelData.GetFormat(), dest);
			}
		}
	}
}

void PixelUtil::SRGBToLinear(PixelData& pixelData)
{
	u32 depth = pixelData.GetDepth();
	u32 height = pixelData.GetHeight();
	u32 width = pixelData.GetWidth();

	u32 pixelSize = PixelUtil::GetNumElemBytes(pixelData.GetFormat());
	u8* data = pixelData.GetData();

	for(u32 z = 0; z < depth; z++)
	{
		u32 zDataIdx = z * pixelData.GetSlicePitch();

		for(u32 y = 0; y < height; y++)
		{
			u32 yDataIdx = y * pixelData.GetRowPitch();

			for(u32 x = 0; x < width; x++)
			{
				u32 dataIdx = x * pixelSize + yDataIdx + zDataIdx;
				u8* dest = data + dataIdx;

				Color color;

				PixelUtil::UnpackColor(&color, pixelData.GetFormat(), dest);
				color = color.GetLinear();
				PixelUtil::PackColor(color, pixelData.GetFormat(), dest);
			}
		}
	}
}

void PixelUtil::Compress(const PixelData& src, PixelData& dst, const CompressionOptions& options)
{
	if(!IsCompressed(options.Format))
	{
		BS_LOG(Error, PixelUtility, "Compression failed. Destination format is not a valid compressed format.");
		return;
	}

	if(src.GetDepth() != 1)
	{
		BS_LOG(Error, PixelUtility, "Compression failed. 3D texture compression not supported.");
		return;
	}

	if(IsCompressed(src.GetFormat()))
	{
		BS_LOG(Error, PixelUtility, "Compression failed. Source data cannot be compressed.");
		return;
	}

	PixelFormat interimFormat = options.Format == PF_BC6H ? PF_RGBA32F : PF_BGRA8;

	PixelData interimData(src.GetWidth(), src.GetHeight(), 1, interimFormat);
	interimData.AllocateInternalBuffer();
	BulkPixelConversion(src, interimData);

	nvtt::InputOptions io;
	io.setTextureLayout(nvtt::TextureType_2D, src.GetWidth(), src.GetHeight());
	io.setMipmapGeneration(false);
	io.setAlphaMode(toNVTTAlphaMode(options.AlphaMode));
	io.setNormalMap(options.IsNormalMap);

	if(interimFormat == PF_RGBA32F)
		io.setFormat(nvtt::InputFormat_RGBA_32F);
	else
		io.setFormat(nvtt::InputFormat_BGRA_8UB);

	if(options.IsSrgb)
		io.setGamma(2.2f, 2.2f);
	else
		io.setGamma(1.0f, 1.0f);

	io.setMipmapData(interimData.GetData(), src.GetWidth(), src.GetHeight());

	nvtt::CompressionOptions co;
	co.setFormat(toNVTTFormat(options.Format));
	co.setQuality(toNVTTQuality(options.Quality));

	NVTTCompressOutputHandler outputHandler(dst.GetData(), dst.GetConsecutiveSize());

	nvtt::OutputOptions oo;
	oo.setOutputHeader(false);
	oo.setOutputHandler(&outputHandler);

	nvtt::Compressor compressor;
	if(!compressor.process(io, co, oo))
	{
		BS_LOG(Error, PixelUtility, "Compression failed. Internal error.");
		return;
	}
}

Vector<SPtr<PixelData>> PixelUtil::GenMipmaps(const PixelData& src, const MipMapGenOptions& options)
{
	Vector<SPtr<PixelData>> outputMipBuffers;

	if(src.GetDepth() != 1)
	{
		BS_LOG(Error, PixelUtility, "Mipmap generation failed. 3D texture formats not supported.");
		return outputMipBuffers;
	}

	if(IsCompressed(src.GetFormat()))
	{
		BS_LOG(Error, PixelUtility, "Mipmap generation failed. Source data cannot be compressed.");
		return outputMipBuffers;
	}

	if(!Bitwise::IsPow2(src.GetWidth()) || !Bitwise::IsPow2(src.GetHeight()))
	{
		BS_LOG(Error, PixelUtility, "Mipmap generation failed. Texture width & height must be powers of 2.");
		return outputMipBuffers;
	}

	PixelFormat interimFormat = IsFloatingPoint(src.GetFormat()) ? PF_RGBA32F : PF_BGRA8;

	PixelData interimData(src.GetWidth(), src.GetHeight(), 1, interimFormat);
	interimData.AllocateInternalBuffer();
	BulkPixelConversion(src, interimData);

	if(interimFormat != PF_RGBA32F)
		FlipComponentOrder(interimData);

	nvtt::InputOptions io;
	io.setTextureLayout(nvtt::TextureType_2D, src.GetWidth(), src.GetHeight());
	io.setMipmapGeneration(true);
	io.setNormalMap(options.IsNormalMap);
	io.setNormalizeMipmaps(options.NormalizeMipmaps);
	io.setWrapMode(toNVTTWrapMode(options.WrapMode));

	if(interimFormat == PF_RGBA32F)
		io.setFormat(nvtt::InputFormat_RGBA_32F);
	else
		io.setFormat(nvtt::InputFormat_BGRA_8UB);

	if(options.IsSrgb)
		io.setGamma(2.2f, 2.2f);
	else
		io.setGamma(1.0f, 1.0f);

	io.setMipmapData(interimData.GetData(), src.GetWidth(), src.GetHeight());

	nvtt::CompressionOptions co;
	co.setFormat(nvtt::Format_RGBA);

	if(interimFormat == PF_RGBA32F)
	{
		co.setPixelType(nvtt::PixelType_Float);
		co.setPixelFormat(32, 32, 32, 32);
	}
	else
	{
		co.setPixelType(nvtt::PixelType_UnsignedNorm);
		co.setPixelFormat(32, 0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF);
	}

	u32 numMips = GetMaxMipmaps(src.GetWidth(), src.GetHeight(), 1, src.GetFormat());

	Vector<SPtr<PixelData>> rgbaMipBuffers;

	// Note: This can be done more effectively without creating so many temp buffers
	// and working with the original formats directly, but it would complicate the code
	// too much at the moment.
	u32 curWidth = src.GetWidth();
	u32 curHeight = src.GetHeight();
	for(u32 i = 0; i < numMips; i++)
	{
		rgbaMipBuffers.push_back(B3DMakeShared<PixelData>(curWidth, curHeight, 1, interimFormat));
		rgbaMipBuffers.back()->AllocateInternalBuffer();

		if(curWidth > 1)
			curWidth = curWidth / 2;

		if(curHeight > 1)
			curHeight = curHeight / 2;
	}

	rgbaMipBuffers.push_back(B3DMakeShared<PixelData>(curWidth, curHeight, 1, interimFormat));
	rgbaMipBuffers.back()->AllocateInternalBuffer();

	NVTTMipmapOutputHandler outputHandler(rgbaMipBuffers);

	nvtt::OutputOptions oo;
	oo.setOutputHeader(false);
	oo.setOutputHandler(&outputHandler);

	nvtt::Compressor compressor;
	if(!compressor.process(io, co, oo))
	{
		BS_LOG(Error, PixelUtility, "Mipmap generation failed. Internal error.");
		return outputMipBuffers;
	}

	interimData.FreeInternalBuffer();

	for(u32 i = 0; i < (u32)rgbaMipBuffers.size(); i++)
	{
		SPtr<PixelData> argbBuffer = rgbaMipBuffers[i];
		SPtr<PixelData> outputBuffer = B3DMakeShared<PixelData>(argbBuffer->GetWidth(), argbBuffer->GetHeight(), 1, src.GetFormat());
		outputBuffer->AllocateInternalBuffer();

		BulkPixelConversion(*argbBuffer, *outputBuffer);
		argbBuffer->FreeInternalBuffer();

		outputMipBuffers.push_back(outputBuffer);
	}

	return outputMipBuffers;
}
