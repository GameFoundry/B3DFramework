//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Script/B3DIScriptExportable.h"
#include "Image/B3DPixelVolume.h"
#include "Resources/B3DGpuResourceData.h"
#include "Reflection/B3DIReflectable.h"

namespace b3d
{
	/** @addtogroup Image
	 *  @{
	 */

	/** Pixel formats usable by images, textures and render surfaces. */
	enum B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) PixelFormat
	{
		/** Unknown pixel format. */
		PF_UNKNOWN B3D_SCRIPT_EXPORT(Exclude(true)) = 0,
		/** 8-bit 1-channel pixel format, unsigned normalized. */
		PF_R8 B3D_SCRIPT_EXPORT(ExportName(R8)) = 1,
		/** 8-bit 2-channel pixel format, unsigned normalized. */
		PF_RG8 B3D_SCRIPT_EXPORT(ExportName(RG8)) = 2,
		/** 8-bit 3-channel pixel format, unsigned normalized. */
		PF_RGB8 B3D_SCRIPT_EXPORT(ExportName(RGB8)) = 3,
		/** 8-bit 3-channel pixel format, unsigned normalized. */
		PF_BGR8 B3D_SCRIPT_EXPORT(ExportName(BGR8)) = 4,
		/** 8-bit 4-channel pixel format, unsigned normalized. */
		PF_BGRA8 B3D_SCRIPT_EXPORT(ExportName(BGRA8)) = 7,
		/** 8-bit 4-channel pixel format, unsigned normalized. */
		PF_RGBA8 B3D_SCRIPT_EXPORT(ExportName(RGBA8)) = 8,
		/** DXT1/BC1 format containing opaque RGB or 1-bit alpha RGB. 4 bits per pixel. */
		PF_BC1 B3D_SCRIPT_EXPORT(ExportName(BC1)) = 13,
		/** DXT3/BC2 format containing RGB with premultiplied alpha. 4 bits per pixel. */
		PF_BC1a B3D_SCRIPT_EXPORT(Exclude(true)) = 14,
		/** DXT3/BC2 format containing RGB with explicit alpha. 8 bits per pixel. */
		PF_BC2 B3D_SCRIPT_EXPORT(ExportName(BC2)) = 15,
		/** DXT5/BC2 format containing RGB with explicit alpha. 8 bits per pixel. Better alpha gradients than BC2. */
		PF_BC3 B3D_SCRIPT_EXPORT(ExportName(BC3)) = 16,
		/** One channel compressed format. 4 bits per pixel. */
		PF_BC4 B3D_SCRIPT_EXPORT(ExportName(BC4)) = 17,
		/** Two channel compressed format. 8 bits per pixel. */
		PF_BC5 B3D_SCRIPT_EXPORT(ExportName(BC5)) = 18,
		/** Format storing RGB in half (16-bit) floating point format usable for HDR. 8 bits per pixel. */
		PF_BC6H B3D_SCRIPT_EXPORT(ExportName(BC6H)) = 19,
		/**
		 * Format storing RGB with optional alpha channel. Similar to BC1/BC2/BC3 formats but with higher quality and
		 * higher decompress overhead. 8 bits per pixel.
		 */
		PF_BC7 B3D_SCRIPT_EXPORT(ExportName(BC7)) = 20,
		/** 16-bit 1-channel pixel format, signed float. */
		PF_R16F B3D_SCRIPT_EXPORT(ExportName(R16F)) = 21,
		/** 16-bit 2-channel pixel format, signed float. */
		PF_RG16F B3D_SCRIPT_EXPORT(ExportName(RG16F)) = 22,
		/** 16-bit 4-channel pixel format, signed float. */
		PF_RGBA16F B3D_SCRIPT_EXPORT(ExportName(RGBA16F)) = 24,
		/** 32-bit 1-channel pixel format, signed float. */
		PF_R32F B3D_SCRIPT_EXPORT(ExportName(R32F)) = 25,
		/** 32-bit 2-channel pixel format, signed float. */
		PF_RG32F B3D_SCRIPT_EXPORT(ExportName(RG32F)) = 26,
		/** 32-bit 3-channel pixel format, signed float. */
		PF_RGB32F B3D_SCRIPT_EXPORT(ExportName(RGB32F)) = 27,
		/** 32-bit 4-channel pixel format, signed float. */
		PF_RGBA32F B3D_SCRIPT_EXPORT(ExportName(RGBA32F)) = 28,
		/** Depth stencil format, 32bit depth, 8bit stencil + 24 unused. Depth stored as signed float. */
		PF_D32_S8X24 B3D_SCRIPT_EXPORT(ExportName(D32_S8X24)) = 29,
		/** Depth stencil fomrat, 24bit depth + 8bit stencil. Depth stored as unsigned normalized. */
		PF_D24S8 B3D_SCRIPT_EXPORT(ExportName(D24S8)) = 30,
		/** Depth format, 32bits. Signed float. */
		PF_D32 B3D_SCRIPT_EXPORT(ExportName(D32)) = 31,
		/** Depth format, 16bits. Unsigned normalized. */
		PF_D16 B3D_SCRIPT_EXPORT(ExportName(D16)) = 32,
		/** Packed unsigned float format, 11 bits for red, 11 bits for green, 10 bits for blue. */
		PF_RG11B10F B3D_SCRIPT_EXPORT(Exclude(true)) = 33,
		/**
		 * Packed unsigned normalized format, 10 bits for red, 10 bits for green, 10 bits for blue, and two bits for alpha.
		 */
		PF_RGB10A2 B3D_SCRIPT_EXPORT(Exclude(true)) = 34,
		/** 8-bit 1-channel pixel format, signed integer. */
		PF_R8I B3D_SCRIPT_EXPORT(ExportName(R8I)) = 35,
		/** 8-bit 2-channel pixel format, signed integer. */
		PF_RG8I B3D_SCRIPT_EXPORT(ExportName(RG8I)) = 36,
		/** 8-bit 4-channel pixel format, signed integer. */
		PF_RGBA8I B3D_SCRIPT_EXPORT(ExportName(RGBA8I)) = 37,
		/** 8-bit 1-channel pixel format, unsigned integer. */
		PF_R8U B3D_SCRIPT_EXPORT(ExportName(R8U)) = 38,
		/** 8-bit 2-channel pixel format, unsigned integer. */
		PF_RG8U B3D_SCRIPT_EXPORT(ExportName(RG8U)) = 39,
		/** 8-bit 4-channel pixel format, unsigned integer. */
		PF_RGBA8U B3D_SCRIPT_EXPORT(ExportName(RGBA8U)) = 40,
		/** 8-bit 1-channel pixel format, signed normalized. */
		PF_R8S B3D_SCRIPT_EXPORT(ExportName(R8S)) = 41,
		/** 8-bit 2-channel pixel format, signed normalized. */
		PF_RG8S B3D_SCRIPT_EXPORT(ExportName(RG8S)) = 42,
		/** 8-bit 4-channel pixel format, signed normalized. */
		PF_RGBA8S B3D_SCRIPT_EXPORT(ExportName(RGBA8S)) = 43,
		/** 16-bit 1-channel pixel format, signed integer. */
		PF_R16I B3D_SCRIPT_EXPORT(ExportName(R16I)) = 44,
		/** 16-bit 2-channel pixel format, signed integer. */
		PF_RG16I B3D_SCRIPT_EXPORT(ExportName(RG16I)) = 45,
		/** 16-bit 4-channel pixel format, signed integer. */
		PF_RGBA16I B3D_SCRIPT_EXPORT(ExportName(RGBA16I)) = 46,
		/** 16-bit 1-channel pixel format, unsigned integer. */
		PF_R16U B3D_SCRIPT_EXPORT(ExportName(R16U)) = 47,
		/** 16-bit 2-channel pixel format, unsigned integer. */
		PF_RG16U B3D_SCRIPT_EXPORT(ExportName(RG16U)) = 48,
		/** 16-bit 4-channel pixel format, unsigned integer. */
		PF_RGBA16U B3D_SCRIPT_EXPORT(ExportName(RGBA16U)) = 49,
		/** 32-bit 1-channel pixel format, signed integer. */
		PF_R32I B3D_SCRIPT_EXPORT(ExportName(R32I)) = 50,
		/** 32-bit 2-channel pixel format, signed integer. */
		PF_RG32I B3D_SCRIPT_EXPORT(ExportName(RG32I)) = 51,
		/** 32-bit 3-channel pixel format, signed integer. */
		PF_RGB32I B3D_SCRIPT_EXPORT(ExportName(RGB32I)) = 52,
		/** 32-bit 4-channel pixel format, signed integer. */
		PF_RGBA32I B3D_SCRIPT_EXPORT(ExportName(RGBA32I)) = 53,
		/** 32-bit 1-channel pixel format, unsigned integer. */
		PF_R32U B3D_SCRIPT_EXPORT(ExportName(R32U)) = 54,
		/** 32-bit 2-channel pixel format, unsigned integer. */
		PF_RG32U B3D_SCRIPT_EXPORT(ExportName(RG32U)) = 55,
		/** 32-bit 3-channel pixel format, unsigned integer. */
		PF_RGB32U B3D_SCRIPT_EXPORT(ExportName(RGB32U)) = 56,
		/** 32-bit 4-channel pixel format, unsigned integer. */
		PF_RGBA32U B3D_SCRIPT_EXPORT(ExportName(RGBA32U)) = 57,
		/** 16-bit 1-channel pixel format, signed normalized. */
		PF_R16S B3D_SCRIPT_EXPORT(ExportName(R16S)) = 58,
		/** 16-bit 2-channel pixel format, signed normalized. */
		PF_RG16S B3D_SCRIPT_EXPORT(ExportName(RG16S)) = 59,
		/** 16-bit 4-channel pixel format, signed normalized. */
		PF_RGBA16S B3D_SCRIPT_EXPORT(ExportName(RGBA16S)) = 60,
		/** 16-bit 1-channel pixel format, unsigned normalized. */
		PF_R16 B3D_SCRIPT_EXPORT(ExportName(R16)) = 61,
		/** 16-bit 2-channel pixel format, unsigned normalized. */
		PF_RG16 B3D_SCRIPT_EXPORT(ExportName(RG16)) = 62,
		/** 16-bit 3-channel pixel format, unsigned normalized. */
		PF_RGB16 B3D_SCRIPT_EXPORT(ExportName(RGB16)) = 63,
		/** 16-bit 4-channel pixel format, unsigned normalized. */
		PF_RGBA16 B3D_SCRIPT_EXPORT(ExportName(RGBA16)) = 64,
		/** Number of pixel formats currently defined. */
		PF_COUNT B3D_SCRIPT_EXPORT(Exclude(true))
	};

	/**	Flags defining some properties of pixel formats. */
	enum PixelFormatFlags
	{
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
		PCT_COUNT /**< Number of pixel types */
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
	class B3D_EXPORT B3D_SCRIPT_EXPORT() PixelData : public GpuResourceData, public IScriptExportable
	{
	public:
		PixelData() = default;

		/**
		 * Constructs a new object with an internal buffer capable of holding "extents" volume of pixels, where each pixel
		 * is of the specified pixel format. Extent offsets are also stored, but are not used internally.
		 */
		PixelData(const PixelVolume& extents, PixelFormat pixelFormat);

		/**
		 * Constructs a new object with an internal buffer capable of holding volume of pixels described by	provided width,
		 * height and depth, where each pixel is of the specified pixel format.
		 */
		PixelData(u32 width, u32 height, u32 depth, PixelFormat pixelFormat);

		PixelData(const PixelData& copy);
		PixelData& operator=(const PixelData& rhs);

		/**
		 * Returns the number of bytes that offsets one row from another. This can be exact number of bytes required
		 * to hold "width" pixel, but doesn't have to be as some buffers require padding.
		 */
		B3D_SCRIPT_EXPORT(ExportName(RawRowPitch), Property(Getter))
		u32 GetRowPitch() const { return mRowPitch; }

		/**
		 * Returns the number of bytes that offsets one depth slice from another. This can be exact number of bytes
		 * required to hold "width * height" pixels, but doesn't have to be as some buffers require padding.
		 */
		B3D_SCRIPT_EXPORT(ExportName(RawSlicePitch), Property(Getter))
		u32 GetSlicePitch() const { return mSlicePitch; }

		/**
		 * Sets the pitch (in bytes) that determines offset between rows of the pixel buffer. Call this before allocating
		 * the buffer.
		 */
		void SetRowPitch(u32 rowPitch) { mRowPitch = rowPitch; }

		/**
		 * Sets the pitch (in bytes) that determines offset between depth slices of the pixel buffer. Call this before
		 * allocating the buffer.
		 */
		void SetSlicePitch(u32 slicePitch) { mSlicePitch = slicePitch; }

		/**
		 * Returns the number of extra bytes in a row (non-zero only if rows are not consecutive (row pitch is larger
		 * than the number of bytes required to hold "width" pixels)).
		 */
		u32 GetRowSkip() const;

		/**
		 * Returns the number of extra bytes in a depth slice (non-zero only if slices aren't consecutive (slice pitch is
		 * larger than the number of bytes required to hold "width * height").
		 */
		u32 GetSliceSkip() const;

		/** Returns the pixel format used by the internal buffer for storing the pixels. */
		B3D_SCRIPT_EXPORT(ExportName(Format), Property(Getter))
		PixelFormat GetFormat() const { return mFormat; }

		/**	Returns width of the buffer in pixels. */
		u32 GetWidth() const { return mExtents.GetWidth(); }

		/**	Returns height of the buffer in pixels. */
		u32 GetHeight() const { return mExtents.GetHeight(); }

		/**	Returns depth of the buffer in pixels. */
		u32 GetDepth() const { return mExtents.GetDepth(); }

		/**
		 * Returns left-most start of the pixel volume. This value is not used internally in any way. It is just passed
		 * through from the constructor.
		 */
		u32 GetLeft() const { return mExtents.Left; }

		/**
		 * Returns right-most end of the pixel volume. This value is not used internally in any way. It is just passed
		 * through from the constructor.
		 */
		u32 GetRight() const { return mExtents.Right; }

		/**
		 * Returns top-most start of the pixel volume. This value is not used internally in any way. It is just passed
		 * through from the constructor.
		 */
		u32 GetTop() const { return mExtents.Top; }

		/**
		 * Returns bottom-most end of the pixel volume. This value is not used internally in any way. It is just passed
		 * through from the constructor.
		 */
		u32 GetBottom() const { return mExtents.Bottom; }

		/**
		 * Returns front-most start of the pixel volume. This value is not used internally in any way. It is just passed
		 * through from the constructor.
		 */
		u32 GetFront() const { return mExtents.Front; }

		/**
		 * Returns back-most end of the pixel volume. This value is not used internally in any way. It is just passed
		 * through from the constructor.
		 */
		u32 GetBack() const { return mExtents.Back; }

		/** Returns extents of the pixel volume this object is capable of holding. */
		B3D_SCRIPT_EXPORT(ExportName(Extents), Property(Getter))
		PixelVolume GetExtents() const { return mExtents; }

		/**
		 * Return whether this buffer is laid out consecutive in memory (meaning the pitches are equal to the dimensions).
		 */
		B3D_SCRIPT_EXPORT(ExportName(RawIsConsecutive), Property(Getter))
		bool IsConsecutive() const
		{
			return mSlicePitch * GetDepth() == GetConsecutiveSize();
		}

		/** Return the size (in bytes) this image would take if it was laid out consecutive in memory. */
		u32 GetConsecutiveSize() const;

		/**	Return the size (in bytes) of the buffer this image requires. */
		B3D_SCRIPT_EXPORT(ExportName(RawSize), Property(Getter))
		u32 GetSize() const;

		/**
		 * Returns pixel data containing a sub-volume of this object. Returned data will not have its own buffer, but will
		 * instead point to this one. It is up to the caller to ensure this object outlives any sub-volume objects.
		 */
		PixelData GetSubVolume(const PixelVolume& volume) const;

		/**
		 * Samples a color at the specified coordinates using a specific filter.
		 *
		 * @param coords	Coordinates to sample the color at. They start at top left corner (0, 0), and are in range
		 *					[0, 1].
		 * @param filter	Filtering mode to use when sampling the color.
		 * @return			Sampled color.
		 */
		Color SampleColorAt(const Vector2& coords, TextureFilter filter = TF_BILINEAR) const;

		/**	Returns pixel color at the specified coordinates. */
		Color GetColorAt(u32 x, u32 y, u32 z = 0) const;

		/**	Sets the pixel color at the specified coordinates. */
		void SetColorAt(const Color& color, u32 x, u32 y, u32 z = 0);

		/**
		 * Converts all the internal data into an array of colors. Array is mapped as such:
		 * arrayIdx = x + y * width + z * width * height.
		 */
		Vector<Color> GetColors() const;

		/**
		 * Initializes the internal buffer with the provided set of colors. The array should be of width * height * depth
		 * size and mapped as such: arrayIdx = x + y * width + z * width * height.
		 */
		void SetColors(const Vector<Color>& colors);

		/**
		 * Initializes the internal buffer with the provided set of colors. The array should be of
		 * width * height * depth size and mapped as such: arrayIdx = x + y * width + z * width * height.
		 */
		void SetColors(Color* colors, u32 elementCount);

		/** Initializes all the pixels with a single color. */
		void SetColors(const Color& color);

		/**
		 * Interprets pixel data as depth information as retrieved from the GPU's depth buffer. Converts the device specific
		 * depth value to range [0, 1] and returns it.
		 */
		float GetDepthAt(u32 x, u32 y, u32 z = 0) const;

		/**
		 * Converts all the internal data into an array of floats as if each individual pixel is retrieved with
		 * getDepthAt(). Array is mapped as such: arrayIdx = x + y * width + z * width * height.
		 */
		Vector<float> GetDepths() const;

		/**
		 * Constructs a new object with an internal buffer capable of holding "extents" volume of pixels, where each pixel
		 * is of the specified pixel format. Extent offsets are also stored, but are not used internally.
		 */
		static SPtr<PixelData> Create(const PixelVolume& extents, PixelFormat pixelFormat);

		/**
		 * Constructs a new object with an internal buffer capable of holding volume of pixels described by provided width,
		 * height and depth, where each pixel is of the specified pixel format.
		 */
		static SPtr<PixelData> Create(u32 width, u32 height, u32 depth, PixelFormat pixelFormat);

	private:
		/**
		 * Initializes the internal buffer with the provided set of colors. The array should be of width * height * depth
		 * size and mapped as such: arrayIdx = x + y * width + z * width * height.
		 *
		 * @note	A generic method that is reused in other more specific setColors() calls.
		 */
		template <class T>
		void SetColorsInternal(const T& colors, u32 elementCount);

		/**	Returns the needed size of the internal buffer, in bytes. */
		u32 GetInternalBufferSize() const override;

	private:
		PixelVolume mExtents = PixelVolume(0, 0, 0, 0);
		PixelFormat mFormat = PF_UNKNOWN;
		u32 mRowPitch = 0;
		u32 mSlicePitch = 0;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class PixelDataRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
