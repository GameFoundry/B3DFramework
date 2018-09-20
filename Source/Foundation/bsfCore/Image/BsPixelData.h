//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Image/BsPixelVolume.h"
#include "Resources/BsGpuResourceData.h"
#include "Reflection/BsIReflectable.h"

namespace bs
{
	/** @addtogroup Resources
	 *  @{
	 */

	/** Pixel formats usable by images, textures and render surfaces. */
	enum BS_SCRIPT_EXPORT(m:Rendering) PixelFormat
	{
		/** Unknown pixel format. */
		PF_UNKNOWN				BS_SCRIPT_EXPORT(ex:true) = 0,
		/** 8-bit 1-channel pixel format, unsigned normalized. */
		PF_R8					BS_SCRIPT_EXPORT(n:R8) = 1,
		/** 8-bit 2-channel pixel format, unsigned normalized. */
		PF_RG8					BS_SCRIPT_EXPORT(n:RG8) = 2,
		/** 8-bit 3-channel pixel format, unsigned normalized. */
		PF_RGB8					BS_SCRIPT_EXPORT(n:RGB8) = 3,
		/** 8-bit 3-channel pixel format, unsigned normalized. */
		PF_BGR8					BS_SCRIPT_EXPORT(n:BGR8) = 4,
		/** 8-bit 4-channel pixel format, unsigned normalized. */
		PF_BGRA8				BS_SCRIPT_EXPORT(n:BGRA8) = 7,
		/** 8-bit 4-channel pixel format, unsigned normalized. */
		PF_RGBA8				BS_SCRIPT_EXPORT(n:RGBA8) = 8,
		/** DXT1/BC1 format containing opaque RGB or 1-bit alpha RGB. 4 bits per pixel. */
		PF_BC1					BS_SCRIPT_EXPORT(n:BC1) = 13,
		/** DXT3/BC2 format containing RGB with premultiplied alpha. 4 bits per pixel. */
		PF_BC1a					BS_SCRIPT_EXPORT(ex:true) = 14,
		/** DXT3/BC2 format containing RGB with explicit alpha. 8 bits per pixel. */
		PF_BC2					BS_SCRIPT_EXPORT(n:BC2) = 15,
		/** DXT5/BC2 format containing RGB with explicit alpha. 8 bits per pixel. Better alpha gradients than BC2. */
		PF_BC3					BS_SCRIPT_EXPORT(n:BC3) = 16,
		/** One channel compressed format. 4 bits per pixel. */
		PF_BC4					BS_SCRIPT_EXPORT(n:BC4) = 17,
		/** Two channel compressed format. 8 bits per pixel. */
		PF_BC5					BS_SCRIPT_EXPORT(n:BC5) = 18,
		/** Format storing RGB in half (16-bit) floating point format usable for HDR. 8 bits per pixel. */
		PF_BC6H					BS_SCRIPT_EXPORT(n:BC6H) = 19,
		/** 
		 * Format storing RGB with optional alpha channel. Similar to BC1/BC2/BC3 formats but with higher quality and 
		 * higher decompress overhead. 8 bits per pixel. 
		 */
		PF_BC7					BS_SCRIPT_EXPORT(n:BC7) = 20,
		/** 16-bit 1-channel pixel format, signed float. */
		PF_R16F					BS_SCRIPT_EXPORT(n:R16F) = 21,
		/** 16-bit 2-channel pixel format, signed float. */
		PF_RG16F				BS_SCRIPT_EXPORT(n:RG16F) = 22,
		/** 16-bit 4-channel pixel format, signed float. */
		PF_RGBA16F				BS_SCRIPT_EXPORT(n:RGBA16F) = 24,
		/** 32-bit 1-channel pixel format, signed float. */
		PF_R32F					BS_SCRIPT_EXPORT(n:R32F) = 25,
		/** 32-bit 2-channel pixel format, signed float. */
		PF_RG32F				BS_SCRIPT_EXPORT(n:RG32F) = 26,
		/** 32-bit 3-channel pixel format, signed float. */
		PF_RGB32F				BS_SCRIPT_EXPORT(n:RGB32F) = 27,
		/** 32-bit 4-channel pixel format, signed float. */
		PF_RGBA32F				BS_SCRIPT_EXPORT(n:RGBA32F) = 28,
		/** Depth stencil format, 32bit depth, 8bit stencil + 24 unused. Depth stored as signed float. */
		PF_D32_S8X24			BS_SCRIPT_EXPORT(n:D32_S8X24) = 29,
		/** Depth stencil fomrat, 24bit depth + 8bit stencil. Depth stored as unsigned normalized. */
		PF_D24S8				BS_SCRIPT_EXPORT(n:D24S8) = 30,
		/** Depth format, 32bits. Signed float. */
		PF_D32					BS_SCRIPT_EXPORT(n:D32) = 31,
		/** Depth format, 16bits. Unsigned normalized. */
		PF_D16					BS_SCRIPT_EXPORT(n:D16) = 32,
		/** Packed unsigned float format, 11 bits for red, 11 bits for green, 10 bits for blue. */
		PF_RG11B10F				BS_SCRIPT_EXPORT(ex:true) = 33,
		/** 
		 * Packed unsigned normalized format, 10 bits for red, 10 bits for green, 10 bits for blue, and two bits for alpha.
		 */
		PF_RGB10A2				BS_SCRIPT_EXPORT(ex:true) = 34,
		/** 8-bit 1-channel pixel format, signed integer. */
		PF_R8I					BS_SCRIPT_EXPORT(n:R8I) = 35,
		/** 8-bit 2-channel pixel format, signed integer. */
		PF_RG8I					BS_SCRIPT_EXPORT(n:RG8I) = 36,
		/** 8-bit 4-channel pixel format, signed integer. */
		PF_RGBA8I				BS_SCRIPT_EXPORT(n:RGBA8I) = 37,
		/** 8-bit 1-channel pixel format, unsigned integer. */
		PF_R8U					BS_SCRIPT_EXPORT(n:R8U) = 38,
		/** 8-bit 2-channel pixel format, unsigned integer. */
		PF_RG8U					BS_SCRIPT_EXPORT(n:RG8U) = 39,
		/** 8-bit 4-channel pixel format, unsigned integer. */
		PF_RGBA8U				BS_SCRIPT_EXPORT(n:RGBA8U) = 40,
		/** 8-bit 1-channel pixel format, signed normalized. */
		PF_R8S					BS_SCRIPT_EXPORT(n:R8S) = 41,
		/** 8-bit 2-channel pixel format, signed normalized. */
		PF_RG8S					BS_SCRIPT_EXPORT(n:RG8S) = 42,
		/** 8-bit 4-channel pixel format, signed normalized. */
		PF_RGBA8S				BS_SCRIPT_EXPORT(n:RGBA8S) = 43,
		/** 16-bit 1-channel pixel format, signed integer. */
		PF_R16I					BS_SCRIPT_EXPORT(n:R16I) = 44,
		/** 16-bit 2-channel pixel format, signed integer. */
		PF_RG16I				BS_SCRIPT_EXPORT(n:RG16I) = 45,
		/** 16-bit 4-channel pixel format, signed integer. */
		PF_RGBA16I				BS_SCRIPT_EXPORT(n:RGBA16I) = 46,
		/** 16-bit 1-channel pixel format, unsigned integer. */
		PF_R16U					BS_SCRIPT_EXPORT(n:R16U) = 47,
		/** 16-bit 2-channel pixel format, unsigned integer. */
		PF_RG16U				BS_SCRIPT_EXPORT(n:RG16U) = 48,
		/** 16-bit 4-channel pixel format, unsigned integer. */
		PF_RGBA16U				BS_SCRIPT_EXPORT(n:RGBA16U) = 49,
		/** 32-bit 1-channel pixel format, signed integer. */
		PF_R32I					BS_SCRIPT_EXPORT(n:R32I) = 50,
		/** 32-bit 2-channel pixel format, signed integer. */
		PF_RG32I				BS_SCRIPT_EXPORT(n:RG32I) = 51,
		/** 32-bit 3-channel pixel format, signed integer. */
		PF_RGB32I				BS_SCRIPT_EXPORT(n:RGB32I) = 52,
		/** 32-bit 4-channel pixel format, signed integer. */
		PF_RGBA32I				BS_SCRIPT_EXPORT(n:RGBA32I) = 53,
		/** 32-bit 1-channel pixel format, unsigned integer. */
		PF_R32U					BS_SCRIPT_EXPORT(n:R32U) = 54,
		/** 32-bit 2-channel pixel format, unsigned integer. */
		PF_RG32U				BS_SCRIPT_EXPORT(n:RG32U) = 55,
		/** 32-bit 3-channel pixel format, unsigned integer. */
		PF_RGB32U				BS_SCRIPT_EXPORT(n:RGB32U) = 56,
		/** 32-bit 4-channel pixel format, unsigned integer. */
		PF_RGBA32U				BS_SCRIPT_EXPORT(n:RGBA32U) = 57,
		/** 16-bit 1-channel pixel format, signed normalized. */
		PF_R16S					BS_SCRIPT_EXPORT(n:R16S) = 58,
		/** 16-bit 2-channel pixel format, signed normalized. */
		PF_RG16S				BS_SCRIPT_EXPORT(n:RG16S) = 59,
		/** 16-bit 4-channel pixel format, signed normalized. */
		PF_RGBA16S				BS_SCRIPT_EXPORT(n:RGBA16S) = 60,
		/** 16-bit 1-channel pixel format, unsigned normalized. */
		PF_R16					BS_SCRIPT_EXPORT(n:R16) = 61,
		/** 16-bit 2-channel pixel format, unsigned normalized. */
		PF_RG16					BS_SCRIPT_EXPORT(n:RG16) = 62,
		/** 16-bit 3-channel pixel format, unsigned normalized. */
		PF_RGB16				BS_SCRIPT_EXPORT(n:RGB16) = 63,
		/** 16-bit 4-channel pixel format, unsigned normalized. */
		PF_RGBA16				BS_SCRIPT_EXPORT(n:RGBA16) = 64,
		/** Number of pixel formats currently defined. */
		PF_COUNT				BS_SCRIPT_EXPORT(ex:true)
	};

	/**	Flags defining some properties of pixel formats. */
	enum PixelFormatFlags {
		/** This format has an alpha channel. */
		PFF_HASALPHA = 0x1,
		/**
		 * This format is compressed. This invalidates the values in elemBytes, elemBits and the bit counts as these might
		 * not be fixed in a compressed format.
		 */
		PFF_COMPRESSED = 0x2,
		/** This is a floating point format. */
		PFF_FLOAT = 0x4,
		/** This is a depth format (for depth textures). */
		PFF_DEPTH = 0x8,
		/** This format stores data internally as integers. */
		PFF_INTEGER = 0x10,
		/** Format contains signed data. Absence of this flag implies unsigned data. */
		PFF_SIGNED = 0x20,
		/** Format contains normalized data. This will be [0, 1] for unsigned, and [-1,1] for signed formats. */
		PFF_NORMALIZED = 0x40
	};

	/**	Types used for individual components of a pixel. */
	enum PixelComponentType
	{
		PCT_BYTE = 0, /**< 8-bit integer per component */
		PCT_SHORT = 1, /**< 16-bit integer per component. */
		PCT_INT = 2, /**< 32-bit integer per component. */
		PCT_FLOAT16 = 3, /**< 16 bit float per component */
		PCT_FLOAT32 = 4, /**< 32 bit float per component */
		PCT_PACKED_R11G11B10 = 5, /**< 11 bits for first two components, 10 for third component. */
		PCT_PACKED_R10G10B10A2 = 6, /**< 10 bits for first three components, 2 bits for last component */
		PCT_COUNT    /**< Number of pixel types */
	};

	/** Determines how are texture pixels filtered during sampling. */
	enum TextureFilter
	{
		/** Pixel nearest to the sampled location is chosen. */
		TF_NEAREST,
		/** Four pixels nearest to the sampled location are interpolated to yield the sampled color. */
		TF_BILINEAR
	};

	/** A list of cubemap faces. */
	enum CubemapFace
	{
		CF_PositiveX,
		CF_NegativeX,
		CF_PositiveY,
		CF_NegativeY,
		CF_PositiveZ,
		CF_NegativeZ
	};

	/**
	 * A buffer describing a volume (3D), image (2D) or line (1D) of pixels in memory. Pixels are stored as a succession 
	 * of "depth" slices, each containing "height" rows of "width" pixels.
	 *
	 * @note
	 * If using the constructor instead of create() you must call GpuResourceData::allocateInternalBuffer or set the buffer
	 * in some other way before reading/writing from this object, as by the default there is no buffer allocated.
	 *
	 * @see		GpuResourceData
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT() PixelData : public GpuResourceData
	{
	public:
		PixelData();

		/**
		 * Constructs a new object with an internal buffer capable of holding "extents" volume of pixels, where each pixel 
		 * is of the specified pixel format. Extent offsets are also stored, but are not used internally.
		 */
		PixelData(const PixelVolume& extents, PixelFormat pixelFormat);

		/**
		 * Constructs a new object with an internal buffer capable of holding volume of pixels described by	provided width, 
		 * height and depth, where each pixel is of the specified pixel format.
		 */
		PixelData(UINT32 width, UINT32 height, UINT32 depth, PixelFormat pixelFormat);

		PixelData(const PixelData& copy);
		PixelData& operator=(const PixelData& rhs);

		/**
		 * Returns the number of pixels that offsets one row from another. This can be "width", but doesn't have to be as
		 * some buffers require padding.
		 */
		BS_SCRIPT_EXPORT(n:RawRowPitch,pr:getter)
		UINT32 getRowPitch() const { return mRowPitch; }

		/**
		 * Returns the number of pixels that offsets one depth slice from another. This can be "width * height", but 
		 * doesn't have to be as some buffers require padding.
		 */
		BS_SCRIPT_EXPORT(n:RawSlicePitch,pr:getter)
		UINT32 getSlicePitch() const { return mSlicePitch; }

		/**
		 * Sets the pitch (in pixels) that determines offset between rows of the pixel buffer. Call this before allocating
		 * the buffer.
		 */
		void setRowPitch(UINT32 rowPitch) { mRowPitch = rowPitch; }

		/**
		 * Sets the pitch (in pixels) that determines offset between depth slices of the pixel buffer. Call this before 
		 * allocating the buffer.
		 */
		void setSlicePitch(UINT32 slicePitch) { mSlicePitch = slicePitch; }

		/**
		 * Returns the number of extra pixels in a row (non-zero only if rows are not consecutive (row pitch is larger 
		 * than width)).
		 */
		UINT32 getRowSkip() const { return mRowPitch - getWidth(); }

		/**
		 * Returns the number of extra pixels in a depth slice (non-zero only if slices aren't consecutive (slice pitch is 
		 * larger than width*height).
		 */
		UINT32 getSliceSkip() const { return mSlicePitch - (getHeight() * mRowPitch); }

		/** Returns the pixel format used by the internal buffer for storing the pixels. */
		BS_SCRIPT_EXPORT(n:Format,pr:getter)
		PixelFormat getFormat() const { return mFormat; }

		/**	Returns width of the buffer in pixels. */
		UINT32 getWidth() const { return mExtents.getWidth(); }

		/**	Returns height of the buffer in pixels. */
		UINT32 getHeight() const { return mExtents.getHeight(); }

		/**	Returns depth of the buffer in pixels. */
		UINT32 getDepth() const { return mExtents.getDepth(); }

		/**
		 * Returns left-most start of the pixel volume. This value is not used internally in any way. It is just passed 
		 * through from the constructor.
		 */
		UINT32 getLeft() const { return mExtents.left; }

		/**
		 * Returns right-most end of the pixel volume. This value is not used internally in any way. It is just passed 
		 * through from the constructor.
		 */
		UINT32 getRight() const { return mExtents.right; }

		/**
		 * Returns top-most start of the pixel volume. This value is not used internally in any way. It is just passed 
		 * through from the constructor.
		 */
		UINT32 getTop() const { return mExtents.top; }

		/**
		 * Returns bottom-most end of the pixel volume. This value is not used internally in any way. It is just passed 
		 * through from the constructor.
		 */
		UINT32 getBottom() const { return mExtents.bottom; }

		/**
		 * Returns front-most start of the pixel volume. This value is not used internally in any way. It is just passed 
		 * through from the constructor.
		 */
		UINT32 getFront() const { return mExtents.front; }

		/**
		 * Returns back-most end of the pixel volume. This value is not used internally in any way. It is just passed 
		 * through from the constructor.
		 */
		UINT32 getBack() const { return mExtents.back; }

		/** Returns extents of the pixel volume this object is capable of holding. */
		BS_SCRIPT_EXPORT(n:Extents,pr:getter)
		PixelVolume getExtents() const { return mExtents; }

		/** 
		 * Return whether this buffer is laid out consecutive in memory (meaning the pitches are equal to the dimensions). 
		 */
		BS_SCRIPT_EXPORT(n:RawIsConsecutive,pr:getter)
		bool isConsecutive() const
		{ 
			return mRowPitch == getWidth() && mSlicePitch == getWidth()*getHeight(); 
		}

		/** Return the size (in bytes) this image would take if it was laid out consecutive in memory. */
		UINT32 getConsecutiveSize() const;

		/**	Return the size (in bytes) of the buffer this image requires. */
		BS_SCRIPT_EXPORT(n:RawSize,pr:getter)
		UINT32 getSize() const;

		/**
		 * Returns pixel data containing a sub-volume of this object. Returned data will not have its own buffer, but will
		 * instead point to this one. It is up to the caller to ensure this object outlives any sub-volume objects.
		 */
		PixelData getSubVolume(const PixelVolume& volume) const;

		/** 
		 * Samples a color at the specified coordinates using a specific filter.
		 * 
		 * @param[in]	coords	Coordinates to sample the color at. They start at top left corner (0, 0), and are in range
		 *						[0, 1].
		 * @param[in]	filter	Filtering mode to use when sampling the color.
		 * @return				Sampled color.
		 */
		Color sampleColorAt(const Vector2& coords, TextureFilter filter = TF_BILINEAR) const;

		/**	Returns pixel color at the specified coordinates. */
		Color getColorAt(UINT32 x, UINT32 y, UINT32 z = 0) const;

		/**	Sets the pixel color at the specified coordinates. */
		void setColorAt(const Color& color, UINT32 x, UINT32 y, UINT32 z = 0);

		/**
		 * Converts all the internal data into an array of colors. Array is mapped as such: 
		 * arrayIdx = x + y * width + z * width * height.
		 */
		Vector<Color> getColors() const;

		/**
		 * Initializes the internal buffer with the provided set of colors. The array should be of width * height * depth 
		 * size and mapped as such: arrayIdx = x + y * width + z * width * height.
		 */
		void setColors(const Vector<Color>& colors);

		/**
		 * Initializes the internal buffer with the provided set of colors. The array should be of 
		 * width * height * depth size and mapped as such: arrayIdx = x + y * width + z * width * height.
		 */
		void setColors(Color* colors, UINT32 numElements);

		/** Initializes all the pixels with a single color. */
		void setColors(const Color& color);

		/** 
		 * Interprets pixel data as depth information as retrieved from the GPU's depth buffer. Converts the device specific
		 * depth value to range [0, 1] and returns it.
		 */
		float getDepthAt(UINT32 x, UINT32 y, UINT32 z = 0) const;

		/**
		 * Converts all the internal data into an array of floats as if each individual pixel is retrieved with 
		 * getDepthAt(). Array is mapped as such: arrayIdx = x + y * width + z * width * height.
		 */
		Vector<float> getDepths() const;

		/**
		 * Constructs a new object with an internal buffer capable of holding "extents" volume of pixels, where each pixel
		 * is of the specified pixel format. Extent offsets are also stored, but are not used internally.
		 */
		static SPtr<PixelData> create(const PixelVolume& extents, PixelFormat pixelFormat);

		/**
		 * Constructs a new object with an internal buffer capable of holding volume of pixels described by provided width,
		 * height and depth, where each pixel is of the specified pixel format.
		 */
		static SPtr<PixelData> create(UINT32 width, UINT32 height, UINT32 depth, PixelFormat pixelFormat);

	private:
		/**
		 * Initializes the internal buffer with the provided set of colors. The array should be of width * height * depth 
		 * size and mapped as such: arrayIdx = x + y * width + z * width * height.
		 *
		 * @note	A generic method that is reused in other more specific setColors() calls.
		 */
		template<class T>
		void setColorsInternal(const T& colors, UINT32 numElements);

		/**	Returns the needed size of the internal buffer, in bytes. */
		UINT32 getInternalBufferSize() const override;

	private:
		PixelVolume mExtents;
		PixelFormat mFormat;
		UINT32 mRowPitch;
		UINT32 mSlicePitch;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class PixelDataRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}
