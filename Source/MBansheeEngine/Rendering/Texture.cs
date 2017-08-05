//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup Rendering
     *  @{
     */

    public partial class Texture
    {
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
        public static Texture Create2D(int width, int height, PixelFormat format = PixelFormat.RGBA8, 
            TextureUsage usage = TextureUsage.Default, int numSamples = 1, bool hasMipmaps = false, 
            bool gammaCorrection = false)
        {
            Texture texture = new Texture(true);
            Internal_create(texture, format, width, height, 1, TextureType.Texture2D, usage, numSamples,
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
        public static Texture Create3D(int width, int height, int depth, PixelFormat format = PixelFormat.RGBA8,
            TextureUsage usage = TextureUsage.Default, bool hasMipmaps = false)
        {
            Texture texture = new Texture(true);
            Internal_create(texture, format, width, height, depth, TextureType.Texture3D, usage, 1,
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
        public static Texture CreateCube(int size, PixelFormat format = PixelFormat.RGBA8,
            TextureUsage usage = TextureUsage.Default, bool hasMipmaps = false, bool gammaCorrection = false)
        {
            Texture texture = new Texture(true);
            Internal_create(texture, format, size, size, 1, TextureType.TextureCube, usage, 1,
                hasMipmaps, gammaCorrection);

            return texture;
        }
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

    /** @} */
}
