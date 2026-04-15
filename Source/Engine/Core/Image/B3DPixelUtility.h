//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Image/B3DPixelData.h"

namespace b3d
{
	// Undefine conflicting defines from other libs
#undef None

	/** @addtogroup Image
	 *  @{
	 */

	/** Flags that describe how a texture is used. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) TextureUsageFlag
	{
		/**
		 * Ensures the texture is placed into memory on the GPU device. This allows the GPU to access
		 * the texture quickly, but makes updating the texture slower. Required for GPU-writeable
		 * textures (render target, depth stencil or unordered access).
		 */
		StoreOnGPU B3D_SCRIPT_EXPORT(ExportName(Default)) = 1 << 0,

		/**
		 * Places the texture into CPU memory accessible to the GPU. This means the texture is faster
		 * to update from the CPU, but it's slower to access by the GPU. Not allowed for GPU-writeable
		 * textures (render target, depth stencil or unordered access).
		 */
		StoreOnCPUWithGPUAccess B3D_SCRIPT_EXPORT(ExportName(Dynamic)) = 1 << 1,

		/** Texture that can be rendered to by the GPU. Must be combined with StoreOnGPU flag. */
		RenderTarget B3D_SCRIPT_EXPORT(ExportName(Render)) = 1 << 9,

		/** Texture used as a depth/stencil buffer by the GPU. Must be combined with StoreOnGPU flag. */
		DepthStencil B3D_SCRIPT_EXPORT(ExportName(DepthStencil)) = 1 << 10,

		/**
		 * Ensures that the GPU can perform unordered write operations on the texture. Generally used
		 * for textures in compute operations. Must be combined with StoreOnGPU flag.
		 */
		AllowUnorderedAccessOnTheGPU B3D_SCRIPT_EXPORT(ExportName(LoadStore)) = 1 << 11,

		/**
		 * All texture data will also be cached in CPU memory for fast read access from the CPU. Only relevant for main
		 * thread textures, ignored for render thread textures.
		 */
		CPUCached B3D_SCRIPT_EXPORT(ExportName(CPUCached)) = 1 << 12,

		/** Allows retrieving views of the texture using a different format than specified on creation. */
		MutableFormat B3D_SCRIPT_EXPORT(ExportName(MutableFormat)) = 1 << 14,

		/** Default setting suitable for majority of textures. */
		Default = StoreOnGPU
	};

	using TextureUsageFlags = Flags<TextureUsageFlag>;
	B3D_FLAGS_OPERATORS(TextureUsageFlag);

	/**	Types of texture compression quality. */
	enum class B3D_SCRIPT_EXPORT() CompressionQuality
	{
		Fastest,
		Normal,
		Production,
		Highest
	};

	/**	Mode of the alpha channel in a texture. */
	enum class B3D_SCRIPT_EXPORT() AlphaMode
	{
		None, /*< Texture has no alpha values. */
		Transparency, /*< Alpha is in the separate transparency channel. */
		Premultiplied /*< Alpha values have been pre-multiplied with the color values. */
	};

	/**	Wrap mode to use when generating mip maps. */
	enum class B3D_SCRIPT_EXPORT() MipMapWrapMode
	{
		Mirror,
		Repeat,
		Clamp
	};

	/**	Filter to use when generating mip maps. */
	enum class B3D_SCRIPT_EXPORT() MipMapFilter
	{
		Box,
		Triangle,
		Kaiser
	};

	/** Determines on which axes to mirror an image. */
	enum class MirrorModeBits
	{
		X = 1 << 0,
		Y = 1 << 1,
		Z = 1 << 2
	};

	typedef Flags<MirrorModeBits> MirrorMode;
	B3D_FLAGS_OPERATORS(MirrorModeBits);

	/**	Options used to control texture compression. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true)) CompressionOptions
	{
		PixelFormat Format = PF_BC1; /*< Format to compress to. Must be a format containing compressed data. */
		AlphaMode AlphaMode = AlphaMode::None; /*< Controls how to (and if) to compress the alpha channel. */
		bool IsNormalMap = false; /*< Determines does the input data represent a normal map. */
		bool IsSrgb = false; /*< Determines has the input data been gamma corrected. */
		CompressionQuality Quality = CompressionQuality::Normal; /*< Compressed image quality. Better compression might take longer to execute but will generate better results. */
	};

	/**	Options used to control texture mip map generation. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true)) MipMapGenOptions
	{
		MipMapFilter Filter = MipMapFilter::Box; /*< Filter to use when downsamping input data. */
		MipMapWrapMode WrapMode = MipMapWrapMode::Mirror; /*< Determines how to downsample pixels on borders. */
		bool IsNormalMap = false; /*< Determines does the input data represent a normal map. */
		bool NormalizeMipmaps = false; /*< Should the downsampled values be re-normalized. Only relevant for mip-maps representing normal maps. */
		bool IsSrgb = false; /*< Determines has the input data been gamma corrected. */
	};

	/**	Filtering types to use when scaling images. */
	enum class B3D_SCRIPT_EXPORT() ScaleFilter
	{
		Nearest, /*< No filtering is performed and nearest existing value is used. */
		Linear /*< Box filter is applied, averaging nearby pixels. */
	};

	/**	Supported image file formats for export. */
	enum class B3D_SCRIPT_EXPORT() ImageFormat
	{
		PNG, /*< Portable Network Graphics (lossless, compressed). */
		JPG, /*< JPEG (lossy, quality 90). */
		BMP, /*< Bitmap (uncompressed). */
		TGA  /*< Truevision TGA (run-length encoding). */
	};

	/**	Utility methods for converting and managing pixel data and formats. */
	class B3D_EXPORT PixelUtility
	{
	public:
		/**	Returns the size of a single pixel of the provided pixel format, in bytes. */
		B3D_SCRIPT_EXPORT()
		static u32 GetElementByteCount(PixelFormat format);

		/**
		 * Returns the size of a single compressed block, in bytes. Returns pixel size if the format is not block
		 * compressed.
		 */
		B3D_SCRIPT_EXPORT()
		static u32 GetBlockSize(PixelFormat format);

		/**
		 * Returns the dimensions of a single compressed block, in number of pixels. Returns 1x1 for non-block-compressed
		 * formats.
		 */
		B3D_SCRIPT_EXPORT()
		static Vector2I GetBlockDimensions(PixelFormat format);

		/**	Returns the size of a single pixel of the provided pixel format, in bits. */
		B3D_SCRIPT_EXPORT()
		static u32 GetElementBitCount(PixelFormat format);

		/**	Returns the size of the memory region required to hold pixels of the provided size ana format. */
		B3D_SCRIPT_EXPORT()
		static u32 GetMemorySize(u32 width, u32 height, u32 depth, PixelFormat format);

		/**	Calculates the size of a mip level of a texture with the provided size. */
		static void GetSizeForMipLevel(u32 width, u32 height, u32 depth, u32 mipLevel, u32& mipWidth, u32& mipHeight, u32& mipDepth);

		/**
		 * Calculates row and depth pitch for a texture surface of the specified size and format. For most formats row
		 * pitch will equal the number of bytes required for storing "width" pixels, and slice pitch will equal the
		 * number of bytes required for storing "width*height" pixels. But some texture formats (especially compressed
		 * ones) might require extra padding. Input width/height/depth values are in pixels, while output pitch values
		 * are in bytes.
		 */
		static void GetPitch(u32 width, u32 height, u32 depth, PixelFormat format, u32& rowPitch, u32& depthPitch);

		/**
		 * Returns property flags for this pixel format.
		 *
		 * @see		PixelFormatFlags
		 */
		static u32 GetFlags(PixelFormat format);

		/**	Checks if the provided pixel format has an alpha channel. */
		B3D_SCRIPT_EXPORT()
		static bool HasAlpha(PixelFormat format);

		/**	Checks is the provided pixel format a floating point format. */
		B3D_SCRIPT_EXPORT()
		static bool IsFloatingPoint(PixelFormat format);

		/**	Checks is the provided pixel format compressed. */
		B3D_SCRIPT_EXPORT()
		static bool IsCompressed(PixelFormat format);

		/**	Checks is the provided pixel format a depth/stencil buffer format. */
		B3D_SCRIPT_EXPORT()
		static bool IsDepth(PixelFormat format);

		/** Checks does the provided format store data in normalized range. */
		B3D_SCRIPT_EXPORT()
		static bool IsNormalized(PixelFormat format);

		/**
		 * Checks is the provided format valid for the texture type and usage.
		 *
		 * @param format		Format to check. If format is not valid the method will update this with the closest
		 *						relevant format.
		 * @param textureType	Type of the texture the format will be used for.
		 * @param usage			A set of TextureUsage flags that define how will a texture be used.
		 * @return				True if the format is valid, false if not.
		 *
		 * @note	This method checks only for obvious format mismatches:
		 *			- Using depth format for anything but a depth-stencil buffer
		 *			- Using anything but a depth format for a depth-stencil-buffer
		 *			- Using compressed format for anything but normal textures
		 *			- Using compressed format for 1D textures
		 *
		 *			Caller should still check for platform-specific unsupported formats.
		 */
		static bool CheckFormat(PixelFormat& format, TextureType textureType, TextureUsageFlags usage);

		/**
		 * Checks are the provided dimensions valid for the specified pixel format. Some formats (like BC) require
		 * width/height to be multiples of four and some formats dont allow depth larger than 1.
		 */
		static bool IsValidExtent(u32 width, u32 height, u32 depth, PixelFormat format);

		/**
		 * Returns the number of bits per each element in the provided pixel format. This will return all zero for
		 * compressed and depth/stencil formats.
		 */
		static void GetBitDepths(PixelFormat format, int (&rgba)[4]);

		/**
		 * Returns bit masks that determine in what bit range is each channel stored.
		 *
		 * @note
		 * For example if your color is stored in an u32 and you want to extract the red channel you should AND the color
		 * u32 with the bit-mask for the red channel and then right shift it by the red channel bit shift amount.
		 */
		static void GetBitMasks(PixelFormat format, u32 (&rgba)[4]);

		/**
		 * Returns number of bits you need to shift a pixel element in order to move it to the start of the data type.
		 *
		 * @note
		 * For example if your color is stored in an u32 and you want to extract the red channel you should AND the color
		 * u32 with the bit-mask for the red channel and then right shift it by the red channel bit shift amount.
		 */
		static void GetBitShifts(PixelFormat format, u8 (&rgba)[4]);

		/**	Returns the name of the pixel format. */
		B3D_SCRIPT_EXPORT()
		static String GetFormatName(PixelFormat format);

		/**
		 * Returns true if the pixel data in the format can be directly accessed and read. This is generally not true
		 * for compressed formats.
		 */
		static bool IsAccessible(PixelFormat format);

		/**	Returns the type of an individual pixel element in the provided format. */
		static PixelComponentType GetElementType(PixelFormat format);

		/**	Returns the number of pixel elements in the provided format. */
		B3D_SCRIPT_EXPORT()
		static u32 GetElementCount(PixelFormat format);

		/**
		 * Returns the maximum number of mip maps that can be generated until we reach the minimum size possible. This
		 * does not count the base level.
		 */
		B3D_SCRIPT_EXPORT()
		static u32 GetMipmapCount(u32 width, u32 height, u32 depth, PixelFormat format);

		/**	Writes the color to the provided memory location. */
		static void PackColor(const Color& color, PixelFormat format, void* dest);

		/**
		 * Writes the color to the provided memory location. If the destination	format is floating point, the byte values
		 * will be converted into [0.0, 1.0] range.
		 */
		static void PackColor(u8 r, u8 g, u8 b, u8 a, PixelFormat format, void* dest);

		/**
		 * Writes the color to the provided memory location. If the destination format in non-floating point, the float
		 * values will be assumed to be in [0.0, 1.0] which	will be converted to integer range. ([0, 255] in the case of bytes)
		 */
		static void PackColor(float r, float g, float b, float a, const PixelFormat format, void* dest);

		/** Reads the color from the provided memory location and stores it into the provided color object. */
		static void UnpackColor(Color* color, PixelFormat format, const void* src);

		/**
		 * Reads the color from the provided memory location and stores it into the provided color elements, as bytes
		 * clamped to [0, 255] range.
		 */
		static void UnpackColor(u8* r, u8* g, u8* b, u8* a, PixelFormat format, const void* src);

		/**
		 * Reads the color from the provided memory location and stores it into the provided color elements. If the format
		 * is not natively floating point a conversion is done in such a way that returned values range [0.0, 1.0].
		 */
		static void UnpackColor(float* r, float* g, float* b, float* a, PixelFormat format, const void* src);

		/** Writes a depth value to the provided memory location. Depth should be in range [0, 1]. */
		static void PackDepth(float depth, const PixelFormat format, void* dest);

		/** Reads the depth from the provided memory location. Value ranges in [0, 1]. */
		static float UnpackDepth(PixelFormat format, void* src);

		/**
		 * Converts pixels from one format to another. Provided pixel data objects must have previously allocated buffers
		 * of adequate size and their sizes must match.
		 */
		static void BulkPixelConversion(const PixelData& source, PixelData& destination);

		/** Flips the order of components in each individual pixel. For example RGBA -> ABGR. */
		static void FlipComponentOrder(PixelData& data);

		/** Converts provided pixels from one format to another.  */
		B3D_SCRIPT_EXPORT();
		static SPtr<PixelData> ConvertFormat(const SPtr<PixelData>& source, PixelFormat format);

		/** Compresses the provided data using the specified compression options.  */
		static void Compress(const PixelData& source, PixelData& destination, const CompressionOptions& options);

		/**
		 * Compresses the provided data using the specified compression options. Caller must ensure that specified format
		 * is a compressed format.
		 */
		B3D_SCRIPT_EXPORT();
		static SPtr<PixelData> Compress(const SPtr<PixelData>& source, const CompressionOptions& options);

		/**
		 * Generates mip-maps from the provided source data using the specified compression options. Returned list includes
		 * the base level.
		 *
		 * @return	A list of calculated mip-map data. First entry is the largest mip and other follow in order from
		 *			largest to smallest.
		 */
		B3D_SCRIPT_EXPORT()
		static Vector<SPtr<PixelData>> GenerateMipmaps(const SPtr<PixelData>& source, const MipMapGenOptions& options);

		/**
		 * Scales pixel data in the source buffer and stores the scaled data in the destination buffer. Provided pixel data
		 * objects must have previously allocated buffers of adequate size. You may also provided a filtering method to use
		 * when scaling.
		 */
		static void Scale(const PixelData& source, PixelData& destination, ScaleFilter filter = ScaleFilter::Linear);

		/** Scales pixel data in the source buffer according to the provided size and filtering method. */
		B3D_SCRIPT_EXPORT()
		static SPtr<PixelData> Scale(const SPtr<PixelData>& source, const Size3UI& size, ScaleFilter filter = ScaleFilter::Linear);

		/**
		 * Mirrors the contents of the provided object along the X, Y and/or Z axes. */
		static void Mirror(PixelData& pixelData, MirrorMode mode);

		/**
		 * Copies the contents of the @p src buffer into the @p dst buffer. The size of the copied contents is determined
		 * by the size of the @p dst buffer. First pixel copied from @p src is determined by offset provided in
		 * @p offsetX, @p offsetY and @p offsetZ parameters.
		 */
		static void Copy(const PixelData& src, PixelData& dst, u32 offsetX = 0, u32 offsetY = 0, u32 offsetZ = 0);

		/** Converts pixel data in linear space to one in sRGB space. Only converts the RGB components. */
		B3D_SCRIPT_EXPORT()
		static SPtr<PixelData> LinearToSrgb(const SPtr<PixelData>& input);

		/** Converts pixel data in sRGB space to one in linear space. Only converts the RGB components. */
		B3D_SCRIPT_EXPORT()
		static SPtr<PixelData> SRGBToLinear(const SPtr<PixelData>& input);

		/**
		 * Saves pixel data to an image file.
		 *
		 * @param pixelData		Pixel data to save. Must be non-null with valid dimensions.
		 * @param outputPath	Output file path. Extension will be set based on format.
		 * @param format		Image format to save as (PNG, JPG, BMP, TGA).
		 * @param ignoreAlpha	If true, alpha channel will be set to fully opaque (255) in the output.
		 * @return				True if save succeeded, false on error (with log message).
		 *
		 * @note				Only supports 8-bit normalized formats: R8, RGB8, BGR8, RGBA8, BGRA8.
		 *						Compressed, float, depth, and integer formats will return false.
		 */
		B3D_SCRIPT_EXPORT()
		static bool SaveImage(const SPtr<PixelData>& pixelData, const Path& outputPath, ImageFormat format, bool ignoreAlpha = false);
	};

	/** @} */
} // namespace b3d
