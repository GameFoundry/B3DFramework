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
    /// A two dimensional render texture. Allows rendering to be performed into a 2D texture which can then be read, as
    /// opposed to rendering directly into the frame buffer.
    /// </summary>
    public class RenderTexture2D : RenderTexture
    {
        /// <summary>
        /// Creates a new 2D render texture.
        /// </summary>
        /// <param name="format">Pixel format of the texture. Format must be a valid uncompressed color format.</param>
        /// <param name="width">Width of the texture in pixels.</param>
        /// <param name="height">Height of the texture in pixels.</param>
        /// <param name="numSamples">Number of samples contained per pixel.</param>
        /// <param name="gammaCorrection">Determines should the pixels written on the texture be gamma corrected.</param>
        /// <param name="createDepth">Should the render texture also contain a depth/stencil buffer.</param>
        /// <param name="depthStencilFormat">Format of the depth/stencil buffer, if <paramref name="createDepth"/> is
        ///                                  enabled. Format must be a valid depth/stencil format.</param>
        public RenderTexture2D(PixelFormat format, int width, int height, int numSamples = 1, 
            bool gammaCorrection = false, bool createDepth = false, PixelFormat depthStencilFormat = PixelFormat.D32)
        {
            Internal_CreateDetailed(this, format, width, height, numSamples, gammaCorrection, createDepth, depthStencilFormat);
        }

        /// <summary>
        /// Creates a new 2D render texture using existing textures as render destinations.
        /// </summary>
        /// <param name="colorSurface">Color texture to render color data to.</param>
        /// <param name="depthStencilSurface">Optional depth/stencil texture to render depth/stencil data to.</param>
        public RenderTexture2D(Texture colorSurface, Texture depthStencilSurface = null)
        {
            IntPtr[] colorSurfaceInstances = new IntPtr[1];
            colorSurfaceInstances[0] = colorSurface.GetCachedPtr();

            IntPtr depthStencilInstance = IntPtr.Zero;
            if (depthStencilSurface != null)
                depthStencilInstance = depthStencilSurface.GetCachedPtr();

            Internal_Create(this, colorSurfaceInstances, depthStencilInstance);
        }

        /// <summary>
        /// Creates a new 2D render texture using one or multiple color textures and a depth/stencil texture.
        /// </summary>
        /// <param name="colorSurfaces">Color texture(s) to render color data to. </param>
        /// <param name="depthStencilSurface">>Optional depth/stencil texture to render depth/stencil data to.</param>
        public RenderTexture2D(Texture[] colorSurfaces, Texture depthStencilSurface = null)
        {
            IntPtr[] colorSurfaceInstances = new IntPtr[colorSurfaces.Length];

            for(int i = 0; i < colorSurfaces.Length; i++)
                colorSurfaceInstances[i] = colorSurfaces[i] != null ? colorSurfaces[i].GetCachedPtr() : IntPtr.Zero;

            IntPtr depthStencilInstance = IntPtr.Zero;
            if (depthStencilSurface != null)
                depthStencilInstance = depthStencilSurface.GetCachedPtr();

            Internal_Create(this, colorSurfaceInstances, depthStencilInstance);
        }

        /// <summary>
        /// Returns the primary color surface that contains rendered color data.
        /// </summary>
        public Texture colorSurface
        {
            get
            {
                Texture[] surfaces;
                Internal_GetColorSurfaces(mCachedPtr, out surfaces);
                return surfaces[0];
            }
        }

        /// <summary>
        /// Returns all of the color surfaces.
        /// </summary>
        public Texture[] colorSurfaces
        {
            get
            {
                Texture[] surfaces;
                Internal_GetColorSurfaces(mCachedPtr, out surfaces);
                return surfaces;
            }
        }

        /// <summary>
        /// Returns the depth/stencil surface that contains rendered depth and stencil data.
        /// </summary>
        public Texture depthStencilSurface
        {
            get
            {
                Texture surface;
                Internal_GetDepthStencilSurface(mCachedPtr, out surface);
                return surface;
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateDetailed(RenderTexture2D instance, PixelFormat format, 
            int width, int height, int numSamples, bool gammaCorrection, bool createDepth, PixelFormat depthStencilFormat);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Create(RenderTexture2D instance, IntPtr[] colorSurfaces, IntPtr depthStencilSurface);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetColorSurfaces(IntPtr thisPtr, out Texture[] surfaces);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetDepthStencilSurface(IntPtr thisPtr, out Texture surface);
    }

    /** @} */
}
