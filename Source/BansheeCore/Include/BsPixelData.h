//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsPixelVolume.h"
#include "BsGpuResourceData.h"
#include "BsIReflectable.h"

namespace bs
{
	/** @addtogroup Resources
	 *  @{
	 */

	/** Pixel formats usable by images, textures and render surfaces. */
    enum PixelFormat
    {
        /** Unknown pixel format. */
        PF_UNKNOWN = 0,
        /** 8-bit pixel format, all bits red. */
        PF_R8 = 1,
		/** 2 byte pixel format, 1 byte red, 1 byte green. */
		PF_R8G8 = 2,
        /** 24-bit pixel format, 8 bits for red, green and blue. */
        PF_R8G8B8 = 3,
        /** 24-bit pixel format, 8 bits for blue, green and red. */
        PF_B8G8R8 = 4,
        /** 32-bit pixel format, 8 bits for alpha, red, green and blue. */
        PF_A8R8G8B8 = 5,
        /** 32-bit pixel format, 8 bits for blue, green, red and alpha. */
        PF_A8B8G8R8 = 6,
        /** 32-bit pixel format, 8 bits for blue, green, red and alpha. */
        PF_B8G8R8A8 = 7,
		/** 32-bit pixel format, 8 bits for red, green, blue and alpha. */
		PF_R8G8B8A8 = 8,
        /** 
		 * 32-bit pixel format, 8 bits for red, 8 bits for green, 8 bits for blue. Like PF_A8R8G8B8, but alpha will get 
		 * discarded.
		 */
        PF_X8R8G8B8 = 9,
        /** 
		 * 32-bit pixel format, 8 bits for blue, 8 bits for green, 8 bits for red. Like PF_A8B8G8R8, but alpha will get 
		 * discarded.
		 */
        PF_X8B8G8R8 = 10,
		/** 
		 * 32-bit pixel format, 8 bits for red, 8 bits for green, 8 bits for blue. Like PF_R8G8B8A8, but alpha will get 
		 * discarded. 
		 */
		PF_R8G8B8X8 = 11,
		/** 32-bit pixel format, 8 bits for blue, 8 bits for green, 8 bits for red. */
		/** Like PF_B8G8R8A8, but alpha will get discarded. */
		PF_B8G8R8X8 = 12,
		/** 24-bit pixel format, 8 bits for red, green and blue. */
		PF_BYTE_RGB = PF_R8G8B8,
		/** 24-bit pixel format, 8 bits for blue, green and red. */
		PF_BYTE_BGR = PF_B8G8R8,
		/** 32-bit pixel format, 8 bits for blue, green, red and alpha. */
		PF_BYTE_BGRA = PF_B8G8R8A8,
		/** 32-bit pixel format, 8 bits for red, green, blue and alpha. */
		PF_BYTE_RGBA = PF_R8G8B8A8,      
        /** DXT1/BC1 format containing opaque RGB or 1-bit alpha RGB. 4 bits per pixel. */
        PF_BC1 = 13,
		/** DXT3/BC2 format containing RGB with premultiplied alpha. 4 bits per pixel. */
		PF_BC1a = 14,
        /** DXT3/BC2 format containing RGB with explicit alpha. 8 bits per pixel. */
        PF_BC2 = 15,
        /** DXT5/BC2 format containing RGB with explicit alpha. 8 bits per pixel. Better alpha gradients than BC2. */
        PF_BC3 = 16,
		/** One channel compressed format. 4 bits per pixel. */
		PF_BC4 = 17,
		/** Two channel compressed format. 8 bits per pixel. */
		PF_BC5 = 18,
		/** Format storing RGB in half (16-bit) floating point format usable for HDR. 8 bits per pixel. */
		PF_BC6H = 19,
		/** 
		 * Format storing RGB with optional alpha channel. Similar to BC1/BC2/BC3 formats but with higher quality and 
		 * higher decompress overhead. 8 bits per pixel. 
		 */
		PF_BC7 = 20,
		/** 16-bit pixel format, 16 bits (float) for red. */
        PF_FLOAT16_R = 21,
		/** 32-bit, 2-channel s10e5 floating point pixel format, 16-bit red, 16-bit green. */
		PF_FLOAT16_RG = 22,
        /** 48-bit pixel format, 16 bits (float) for red, 16 bits (float) for green, 16 bits (float) for blue. */
        PF_FLOAT16_RGB = 23,
        /** 
		 * 64-bit pixel format, 16 bits (float) for red, 16 bits (float) for green, 16 bits (float) for blue, 16 bits 
		 * (float) for alpha. 
		 */
        PF_FLOAT16_RGBA = 24,
		/** 32-bit pixel format, 32 bits (float) for red. */
        PF_FLOAT32_R = 25,
		/** 64-bit, 2-channel floating point pixel format, 32-bit red, 32-bit green. */
		PF_FLOAT32_RG = 26,
        /** 96-bit pixel format, 32 bits (float) for red, 32 bits (float) for green, 32 bits (float) for blue. */
        PF_FLOAT32_RGB = 27,
        /** 
		 * 128-bit pixel format, 32 bits (float) for red, 32 bits (float) for green, 32 bits (float) for blue, 32 bits 
		 * (float) for alpha. 
		 */
        PF_FLOAT32_RGBA = 28,
		/** Depth stencil format, 32bit depth, 8bit stencil + 24 unused. */
		PF_D32_S8X24 = 29,
		/** Depth stencil fomrat, 24bit depth + 8bit stencil. */
		PF_D24S8 = 30,
		/** Depth format, 32bits. */
		PF_D32 = 31,
		/** Depth format, 16bits. */
		PF_D16 = 32,
		/** 
		 * 32-bit float format, 11 bits (float) for red, 11 bits (float) for green, 10 bits (float) for blue. Framebuffer 
		 * only format, not for CPU use. 
		 */
		PF_FLOAT_R11G11B10 = 33,
		/** 
		 * 32-bit unsigned normalized format, 10 bits (float) for red, 10 bits (float) for green, 10 bits (float) for blue, 
		 * and two bits for alpha. Framebuffer only format, not for CPU use.
		 */
		PF_UNORM_R10G10B10A2 = 34,
		/** Number of pixel formats currently defined. */
        PF_COUNT = 35
    };
	typedef Vector<PixelFormat> PixelFormatList;

	/**	Flags defining some properties of pixel formats. */
    enum PixelFormatFlags {
        /** This format has an alpha channel. */
        PFF_HASALPHA = 0x00000001,      
        /**
		 * This format is compressed. This invalidates the values in elemBytes, elemBits and the bit counts as these might
		 * not be fixed in a compressed format.
		 */
        PFF_COMPRESSED = 0x00000002,
        /** This is a floating point format. */
        PFF_FLOAT = 0x00000004,         
        /** This is a depth format (for depth textures). */
        PFF_DEPTH = 0x00000008,
        /** 
		 * Format is in native endian. Generally true for the 16, 24 and 32 bits formats which can be represented as 
		 * machine integers.
		 */
        PFF_NATIVEENDIAN = 0x00000010
    };
    
	/**	Types used for individual components of a pixel. */
    enum PixelComponentType
    {
        PCT_BYTE = 0,    /**< Byte per component */
        PCT_SHORT = 1,   /**< Short per component */
        PCT_FLOAT16 = 2, /**< 16 bit float per component */
        PCT_FLOAT32 = 3, /**< 32 bit float per component */
		PCT_PACKED_R11G11B10 = 4, /**< 11 bits for first two components, 10 for third component. */
		PCT_PACKED_R10G10B10A2 = 5, /**< 10 bits for first three components, 2 bits for last component */
        PCT_COUNT = 4    /**< Number of pixel types */
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
    class BS_CORE_EXPORT PixelData : public GpuResourceData
	{
    public:
    	PixelData() {}
		~PixelData() {}

		/**
		 * Constructs a new object with an internal buffer capable of holding "extents" volume of pixels, where each pixel 
		 * is of the specified pixel format. Extent offsets are also stored, but are not used internally.
		 */
		PixelData(const PixelVolume& extents, PixelFormat pixelFormat)
			:mExtents(extents), mFormat(pixelFormat)
		{
			setConsecutive();
		}

		/**
		 * Constructs a new object with an internal buffer capable of holding volume of pixels described by	provided width, 
		 * height and depth, where each pixel is of the specified pixel format.
		 */
    	PixelData(UINT32 width, UINT32 height, UINT32 depth, PixelFormat pixelFormat)
			: mExtents(0, 0, 0, width, height, depth), mFormat(pixelFormat)
    	{
    		setConsecutive();
    	}

		PixelData(const PixelData& copy);
		PixelData& operator=(const PixelData& rhs);

		/**
		 * Returns the number of pixels that offsets one row from another. This can be "width", but doesn't have to be as
		 * some buffers require padding.
		 */
		UINT32 getRowPitch() const { return mRowPitch; }

		/**
		 * Returns the number of pixels that offsets one depth slice from another. This can be "width * height", but 
		 * doesn't have to be as some buffers require padding.
		 */
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
		PixelVolume getExtents() const { return mExtents; }

		/** 
		 * Return whether this buffer is laid out consecutive in memory (meaning the pitches are equal to the dimensions). 
		 */
        bool isConsecutive() const 
		{ 
			return mRowPitch == getWidth() && mSlicePitch == getWidth()*getHeight(); 
		}

		/** Return the size (in bytes) this image would take if it was laid out consecutive in memory. */
      	UINT32 getConsecutiveSize() const;

		/**	Return the size (in bytes) of the buffer this image requires. */
      	UINT32 getSize() const;

		/**
		 * Returns pixel data containing a sub-volume of this object. Returned data will not have its own buffer, but will
		 * instead point to this one. It is up to the caller to ensure this object outlives any sub-volume objects.
		 */
      	PixelData getSubVolume(const PixelVolume &def) const;
        
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

		/** 
		 * Decodes data stored in a depth texture at the specified pixel coordinates, and outputs a floating point depth
		 * value in range [0, 1]. 
		 */
		float getDepthAt(UINT32 x, UINT32 y, UINT32 z = 0) const;

		/** Sets a depth value in range [0, 1] at the specified pixel coordinates. */
		void setDepthAt(float depth, UINT32 x, UINT32 y, UINT32 z = 0);

		/**
		 * Converts all the internal data into an array of float. Array is mapped as such:
		 * arrayIdx = x + y * width + z * width * height.
		 */
		Vector<float> getDepths() const;

		/**
		 * Constructs a new object with an internal buffer capable of holding "extents" volume of pixels, where each pixel
		 * is of the specified pixel format. Extent offsets are also stored, but are not used internally.
		 */
		static SPtr<PixelData> create(const PixelVolume &extents, PixelFormat pixelFormat);

		/**
		 * Constructs a new object with an internal buffer capable of holding volume of pixels described by provided width,
		 * height and depth, where each pixel is of the specified pixel format.
		 */
		static SPtr<PixelData> create(UINT32 width, UINT32 height, UINT32 depth, PixelFormat pixelFormat);

	private:
		/**
		 * Set the rowPitch and slicePitch so that the buffer is laid out consecutive in memory. Does not actually modify
		 * the buffer itself.
		 */
		void setConsecutive()
		{
			mRowPitch = getWidth();
			mSlicePitch = getWidth()*getHeight();
		}

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
