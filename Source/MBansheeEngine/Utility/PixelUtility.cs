//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace BansheeEngine
{
    /** @addtogroup Utility
     *  @{
     */

    /// <summary>
    /// Utility methods for converting and managing pixel data and formats.
    /// </summary>
    public static class PixelUtility
    {
        /// <summary>
        /// Returns the size of the memory region required to hold pixels of the provided size ana format.
        /// </summary>
        /// <param name="width">Number of pixels in each row.</param>
        /// <param name="height">Number of pixels in each column.</param>
        /// <param name="depth">Number of 2D slices.</param>
        /// <param name="format">Format of individual pixels.</param>
        /// <returns>Size of the memory region in bytes.</returns>
        public static int GetMemorySize(int width, int height, int depth, PixelFormat format)
        {
            int value;
            Internal_GetMemorySize(width, height, depth, format, out value);
            return value;
        }

        /// <summary>
        /// Checks if the provided pixel format has an alpha channel.
        /// </summary>
        /// <param name="format">Format to check.</param>
        /// <returns>True if the format contains an alpha channel.</returns>
        public static bool HasAlpha(PixelFormat format)
        {
            bool value;
            Internal_HasAlpha(format, out value);
            return value;
        }

        /// <summary>
        /// Checks is the provided pixel format a floating point format.
        /// </summary>
        /// <param name="format">Format to check.</param>
        /// <returns>True if the format contains floating point values.</returns>
        public static bool IsFloatingPoint(PixelFormat format)
        {
            bool value;
            Internal_IsFloatingPoint(format, out value);
            return value;
        }

        /// <summary>
        /// Checks is the provided pixel format contains compressed data.
        /// </summary>
        /// <param name="format">Format to check.</param>
        /// <returns>True if the format contains compressed data.</returns>
        public static bool IsCompressed(PixelFormat format)
        {
            bool value;
            Internal_IsCompressed(format, out value);
            return value;
        }

        /// <summary>
        /// Checks is the provided pixel format a depth/stencil buffer format.
        /// </summary>
        /// <param name="format">Format to check.</param>
        /// <returns>True if the format is a depth/stencil buffer format.</returns>
        public static bool IsDepth(PixelFormat format)
        {
            bool value;
            Internal_IsDepth(format, out value);
            return value;
        }

        /// <summary>
        /// Returns the maximum number of mip maps that can be generated until we reachthe minimum size possible. This does 
        /// not count the base level.
        /// </summary>
        /// <param name="width">Number of pixels in each row.</param>
        /// <param name="height">Number of pixels in each column.</param>
        /// <param name="depth">Number of 2D slices.</param>
        /// <param name="format">Format of individual pixels.</param>
        /// <returns>Possible number of mip-maps not counting the base level.</returns>
        public static int GetMaxMipmaps(int width, int height, int depth, PixelFormat format)
        {
            int value;
            Internal_GetMaxMipmaps(width, height, depth, format, out value);
            return value;
        }

        /// <summary>
        /// Converts a set of pixels from one format to another.
        /// </summary>
        /// <param name="source">Pixels to convert.</param>
        /// <param name="newFormat">New pixel format.</param>
        /// <returns>New pixel data object containing the converted pixels.</returns>
        public static PixelData ConvertFormat(PixelData source, PixelFormat newFormat)
        {
            return Internal_ConvertFormat(source, newFormat);
        }

        /// <summary>
        /// Compresses the provided pixels using the specified compression options.
        /// </summary>
        /// <param name="source">Pixels to compress.</param>
        /// <param name="options">Options to control the compression. Make sure the format contained within is a
        ///                       compressed format.</param>
        /// <returns>New pixel data object containing the compressed pixels.</returns>
        public static PixelData Compress(PixelData source, CompressionOptions options)
        {
            return Internal_Compress(source, ref options);
        }

        /// <summary>
        /// Generates mip-maps from the provided source data using the specified compression options. Returned list includes 
        /// the base level.
        /// </summary>
        /// <param name="source">Pixels to generate mip-maps for.</param>
        /// <param name="options">Options controlling mip-map generation.</param>
        /// <returns>A list of calculated mip-map data. First entry is the largest mip and other follow in order from 
        ///          largest to smallest.</returns>
		public static PixelData[] GenerateMipmaps(PixelData source, MipMapGenOptions options)
        {
            return Internal_GenerateMipmaps(source, ref options);
        }

        /// <summary>
        /// Scales pixel data in the source buffer and stores the scaled data in the destination buffer.
        /// </summary>
        /// <param name="source">Source pixels to scale.</param>
        /// <param name="newSize">New dimensions to scale to.</param>
        /// <param name="filter">Filter to use when scaling.</param>
        /// <returns>New pixel data object containing the scaled pixels.</returns>
        public static PixelData Scale(PixelData source, PixelVolume newSize, ScaleFilter filter = ScaleFilter.Linear)
        {
            return Internal_Scale(source, ref newSize, filter);
        }

        /// <summary>
        /// Applies gamma correction to the pixels in the provided buffer.
        /// </summary>
        /// <param name="source">Source pixels to gamma correct.</param>
        /// <param name="gamma">Gamma value to apply.</param>
        public static void ApplyGamma(PixelData source, float gamma)
        {
            Internal_ApplyGamma(source, gamma);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetMemorySize(int width, int height, int depth, PixelFormat format, out int value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_HasAlpha(PixelFormat format, out bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_IsFloatingPoint(PixelFormat format, out bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_IsCompressed(PixelFormat format, out bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_IsDepth(PixelFormat format, out bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetMaxMipmaps(int width, int height, int depth, PixelFormat format, out int value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern PixelData Internal_ConvertFormat(PixelData source, PixelFormat newFormat);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern PixelData Internal_Compress(PixelData source, ref CompressionOptions options);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern PixelData[] Internal_GenerateMipmaps(PixelData source, ref MipMapGenOptions options);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern PixelData Internal_Scale(PixelData source, ref PixelVolume newSize, ScaleFilter filter);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_ApplyGamma(PixelData source, float gamma);
    }

    /// <summary>
    /// Filtering types to use when scaling images.
    /// </summary>
    public enum ScaleFilter // Note: Must match the C++ enum PixelUtil::Filter
    {
        /// <summary>
        /// No filtering is performed and nearest existing value is used.
        /// </summary>
        Nearest,
        /// <summary>
        /// Box filter is applied, averaging nearby pixels.
        /// </summary>
        Linear
    };

    /// <summary>
    /// Types of texture compression quality.
    /// </summary>
    public enum CompressionQuality // Note: Must match the C++ enum CompressionQuality
	{
		Fastest,
		Normal,
		Production,
		Highest
	};

    /// <summary>
    /// Mode of the alpha channel in a texture.
    /// </summary>
    public enum AlphaMode // Note: Must match the C++ enum AlphaMode
	{
        /// <summary>
        /// Texture has no alpha values.
        /// </summary>
		None,
        /// <summary>
        /// Alpha is in the separate transparency channel.
        /// </summary>
		Transparency,
        /// <summary>
        /// Alpha values have been pre-multiplied with the color values.
        /// </summary>
		Premultiplied
	};

    /// <summary>
    /// Wrap mode to use when generating mip maps.
    /// </summary>
    public enum MipMapWrapMode // Note: Must match the C++ enum MipMapWrapMode
	{
		Mirror,
		Repeat,
		Clamp
	};

    /// <summary>
    /// Filter to use when generating mip maps.
    /// </summary>
    public enum MipMapFilter // Note: Must match the C++ enum MipMapFilter
	{
		Box,
		Triangle,
		Kaiser
	};

    /// <summary>
    /// Options used to control texture compression.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct CompressionOptions // Note: Must match the C++ struct CompressionOptions
	{
        /// <summary>
        /// Format to compress to. Must be a format containing compressed data.
        /// </summary>
		public PixelFormat format;

        /// <summary>
        /// Controls how to (and if) to compress the alpha channel.
        /// </summary>
	    public AlphaMode alphaMode;

        /// <summary>
        /// Determines does the input data represent a normal map.
        /// </summary>
		public bool isNormalMap;

        /// <summary>
        /// Determines has the input data been gamma corrected.
        /// </summary>
		public bool isSRGB;

        /// <summary>
        /// Compressed image quality. Better compression might take longer to execute but will generate better results.
        /// </summary>
		public CompressionQuality quality;
	};

    /// <summary>
    /// Options used to control texture mip map generation.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct MipMapGenOptions // Note: Must match the C++ struct MipMapGenOptions
	{
        /// <summary>
        /// Filter to use when downsamping input data.
        /// </summary>
		public MipMapFilter filter;

        /// <summary>
        /// Determines how to downsample pixels on borders.
        /// </summary>
		public MipMapWrapMode wrapMode;

        /// <summary>
        /// Determines does the input data represent a normal map.
        /// </summary>
		public bool isNormalMap;

        /// <summary>
        /// Should the downsampled values be re-normalized. Only relevant for mip-maps representing normal maps.
        /// </summary>
		public bool normalizeMipmaps;

        /// <summary>
        /// Determines has the input data been gamma corrected.
        /// </summary>
        bool isSRGB; 
    };

    /** @} */
}
