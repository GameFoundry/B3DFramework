//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup Rendering
     *  @{
     */

    /// <summary>
    /// Base class for all textures. Contains a set of pixels of certain dimensions that can be used for rendering
    /// or read/written directly.
    /// </summary>
    public class Texture : Resource
    {
        /// <summary>
        /// Returns the pixel format for the texture surface.
        /// </summary>
        public PixelFormat PixelFormat
        {
            get
            {
                PixelFormat value;
                Internal_GetPixelFormat(mCachedPtr, out value);
                return value;
            }
        }

        /// <summary>
        /// Returns a value that signals the engine in what way is the texture expected to be used.
        /// </summary>
        public TextureUsage Usage
        {
            get
            {
                TextureUsage value;
                Internal_GetUsage(mCachedPtr, out value);
                return value;
            }
        }

        /// <summary>
        /// Width of the texture in pixels.
        /// </summary>
        public int Width
        {
            get
            {
                int value;
                Internal_GetWidth(mCachedPtr, out value);
                return value;
            }
        }

        /// <summary>
        /// Height of the texture in pixels.
        /// </summary>
        public int Height
        {
            get
            {
                int value;
                Internal_GetHeight(mCachedPtr, out value);
                return value;
            }
        }

        /// <summary>
        /// Determines does the texture contain gamma corrected data. If true then the GPU will automatically convert
        /// the pixels to linear space before reading from the texture, and convert them to gamma space when writing
        /// to the texture.
        /// </summary>
        public bool GammaCorrection
        {
            get
            {
                bool value;
                Internal_GetGammaCorrection(mCachedPtr, out value);
                return value;
            }
        }

        /// <summary>
        /// Number of samples per pixel. Zero or one mean no multisampling will be used.
        /// </summary>
        public int SampleCount
        {
            get
            {
                int value;
                Internal_GetSampleCount(mCachedPtr, out value);
                return value;
            }
        }

        /// <summary>
        /// Returns how many mipmap levels does the texture contain.
        /// </summary>
        public int MipmapCount
        {
            get
            {
                int value;
                Internal_GetMipmapCount(mCachedPtr, out value);
                return value;
            }
        }

        /// <summary>
        /// Constructor for the internal use by the runtime.
        /// </summary>
        private Texture()
        { }

        /// <summary>
        /// Creates a new blank 2D texture.
        /// </summary>
        /// <param name="width">Width of the texture in pixels.</param>
        /// <param name="height">Height of the texture in pixels.</param>
        /// <param name="format">Format of the pixels.</param>
        /// <param name="usage">Describes planned texture use.</param>
        /// <param name="numSamples">If higher than 1, texture containing multiple samples per pixel is created.</param>
        /// <param name="hasMipmaps">Should the texture allocate memory for the entire mip-map chain or only the top level.
        ///                          </param>
        /// <param name="gammaCorrection">If true the texture data is assumed to have be gamma corrected and will be
        ///                               converted back to linear space when sampled on GPU, and converted to gamma space
        ///                               before being written by the GPU.</param>
        public static Texture Create2D(int width, int height, PixelFormat format = PixelFormat.R8G8B8A8, 
            TextureUsage usage = TextureUsage.Default, int numSamples = 1, bool hasMipmaps = false, 
            bool gammaCorrection = false)
        {
            Texture texture = new Texture();
            Internal_CreateInstance(texture, format, width, height, 1, TextureType.Texture2D, usage, numSamples,
                hasMipmaps, gammaCorrection);

            return texture;
        }

        /// <summary>
        /// Creates a new blank 3D texture.
        /// </summary>
        /// <param name="width">Width of the texture in pixels.</param>
        /// <param name="height">Height of the texture in pixels.</param>
        /// <param name="depth">Depth of the texture in pixels.</param>
        /// <param name="format">Format of the pixels.</param>
        /// <param name="usage">Describes planned texture use.</param>
        /// <param name="hasMipmaps">Should the texture allocate memory for the entire mip-map chain or only the top level.
        ///                          </param>
        public static Texture Create3D(int width, int height, int depth, PixelFormat format = PixelFormat.R8G8B8A8,
            TextureUsage usage = TextureUsage.Default, bool hasMipmaps = false)
        {
            Texture texture = new Texture();
            Internal_CreateInstance(texture, format, width, height, depth, TextureType.Texture3D, usage, 1,
                hasMipmaps, false);

            return texture;
        }

        /// <summary>
        /// Creates a new blank cubemap texture.
        /// </summary>
        /// <param name="size">Width & height of a single cubemap face in pixels.</param>
        /// <param name="format">Format of the pixels.</param>
        /// <param name="usage">Describes planned texture use.</param>
        /// <param name="hasMipmaps">Should the texture allocate memory for the entire mip-map chain or only the top level.
        ///                          </param>
        /// <param name="gammaCorrection">If true the texture data is assumed to have be gamma corrected and will be
        ///                               converted back to linear space when sampled on GPU, and converted to gamma space
        ///                               before being written by the GPU.</param>
        public static Texture CreateCube(int size, PixelFormat format = PixelFormat.R8G8B8A8,
            TextureUsage usage = TextureUsage.Default, bool hasMipmaps = false, bool gammaCorrection = false)
        {
            Texture texture = new Texture();
            Internal_CreateInstance(texture, format, size, size, 1, TextureType.TextureCube, usage, 1,
                hasMipmaps, gammaCorrection);

            return texture;
        }

        /// <summary>
        /// Returns pixels for the specified mip level & face. Pixels will be read from system memory, which means the
        /// texture has to be created with <see cref="TextureUsage.CPUCached"/>. If the texture was updated from the GPU the
        /// pixels retrieved from this method will not reflect that, and you should use <see cref="GetGPUPixels"/> instead.
        /// </summary>
        /// <param name="mipLevel">Mip level to retrieve pixels for. Top level (0) is the highest quality.</param>
        /// <param name="face">Face to read the pixels from. Cubemap textures have six faces whose face indices are as
        ///                    specified in the <see cref="CubeFace"/> enum. Array textures can have an arbitrary number
        ///                    of faces (if it's a cubemap array it has to be a multiple of 6).</param>
        /// <returns>A set of pixels for the specified mip level.</returns>
        public PixelData GetPixels(int mipLevel = 0, int face = 0)
        {
            return Internal_GetPixels(mCachedPtr, face, mipLevel);
        }

        /// <summary>
        /// Sets pixels for the specified mip level and face.
        /// </summary>
        /// <param name="data">Pixels to assign to the specified mip level. Pixel data must match the mip level size
        ///                    and texture pixel format.</param>
        /// <param name="mipLevel">Mip level to set pixels for. Top level (0) is the highest quality.</param>
        /// <param name="face">Face to set pixels for. Cubemap textures have six faces whose face indices are as
        ///                    specified in the <see cref="CubeFace"/> enum. Array textures can have an arbitrary number
        ///                    of faces (if it's a cubemap array it has to be a multiple of 6).</param>
        public void SetPixels(PixelData data, int mipLevel = 0, int face = 0)
        {
            Internal_SetPixels(mCachedPtr, data, face, mipLevel);
        }

        /// <summary>
        /// Sets pixels for the specified mip level and face.
        /// </summary>
        /// <param name="data">Pixels to assign to the specified mip level. Size of the array must match the mip level
        ///                    dimensions. Data is expected to be laid out row by row. Pixels will be automatically
        ///                    converted to the valid pixel format.</param>
        /// <param name="mipLevel">Mip level to set pixels for. Top level (0) is the highest quality.</param>
        /// <param name="face">Face to set pixels for. Cubemap textures have six faces whose face indices are as
        ///                    specified in the <see cref="CubeFace"/> enum. Array textures can have an arbitrary number
        ///                    of faces (if it's a cubemap array it has to be a multiple of 6).</param>
        public void SetPixels(Color[] data, int mipLevel = 0, int face = 0)
        {
            Internal_SetPixelsArray(mCachedPtr, data, face, mipLevel);
        }

        /// <summary>
        /// Reads texture pixels directly from the GPU. This is similar to <see cref="GetPixels"/> but the texture doesn't
        /// need to be created with <see cref="TextureUsage.CPUCached"/>, and the data will contain any updates performed by
        /// the GPU. This method can be potentially slow as it introduces a CPU-GPU synchronization point. Additionally
        /// this method is asynchronous which means the data is not available immediately.
        /// </summary>
        /// <param name="mipLevel">Mip level to retrieve pixels for. Top level (0) is the highest quality.</param>
        /// <param name="face">Face to read the pixels from. Cubemap textures have six faces whose face indices are as
        ///                    specified in the <see cref="CubeFace"/> enum. Array textures can have an arbitrary number
        ///                    of faces (if it's a cubemap array it has to be a multiple of 6).</param>
        /// <returns><see cref="AsyncOp"/> object that will contain <see cref="PixelData"/> object when the operation 
        ///          completes.</returns>
        public AsyncOp GetGPUPixels(int mipLevel = 0, int face = 0)
        {
            return Internal_GetGPUPixels(mCachedPtr, face, mipLevel);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetPixelFormat(IntPtr thisPtr, out PixelFormat value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetUsage(IntPtr thisPtr, out TextureUsage value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetWidth(IntPtr thisPtr, out int value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetHeight(IntPtr thisPtr, out int value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetGammaCorrection(IntPtr thisPtr, out bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetSampleCount(IntPtr thisPtr, out int value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetMipmapCount(IntPtr thisPtr, out int value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(Texture instance, PixelFormat format, int width,
            int height, int depth, TextureType texType, TextureUsage usage, int numSamples, bool hasMipmaps, 
            bool gammaCorrection);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern PixelData Internal_GetPixels(IntPtr thisPtr, int face, int mipLevel);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern AsyncOp Internal_GetGPUPixels(IntPtr thisPtr, int face, int mipLevel);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetPixels(IntPtr thisPtr, PixelData data, int face, int mipLevel);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetPixelsArray(IntPtr thisPtr, Color[] data, int face, int mipLevel);
    }

    /// <summary>
    /// Flags that describe how is a texture used.
    /// </summary>
    public enum TextureUsage // Note: Must match C++ enum TextureUsage
    {
        /// <summary>
        /// A regular texture that is not often or ever updated from the CPU.
        /// </summary>
        Default = 0x1,

        /// <summary>
        /// A regular texture that is often updated by the CPU.
        /// </summary>
        Dynamic = 0x2,

        /// <summary>
        /// Texture that can be rendered to by the GPU.
        /// </summary>
        Render = 0x200,

        /// <summary>
        /// Texture used as a depth/stencil buffer by the GPU.
        /// </summary>
        DepthStencil = 0x400,

        /// <summary>
        /// Texture that allows load/store operations from the GPU program.
        /// </summary>
        LoadStore = 0x800,

        /// <summary>
        /// All mesh data will also be cached in CPU memory, making it available for fast read access from the CPU.
        /// </summary>
        CPUCached = 0x1000,

        /// <summary>
        /// Allows the CPU to directly read the texture data buffers from the GPU.
        /// </summary>
        CPUReadable = 0x2000,
    }

    /// <summary>
    /// Indices for the faces of a cube texture.
    /// </summary>
    public enum CubeFace
    {
        PositiveX = 0,
        NegativeX = 1,
        PositiveY = 2,
        NegativeY = 3,
        PositiveZ = 4,
        NegativeZ = 5,
    }

    /// <summary>
    /// Supported texture types.
    /// </summary>
    internal enum TextureType // Note: Must match C++ enum TextureType
    {
        Texture2D = 2,
        Texture3D = 3,
        TextureCube = 4
    }

    /** @} */
}
