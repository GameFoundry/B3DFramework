//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>
	/// Render target specialization that allows you to render into one or multiple textures. Such textures can then be used 
	/// in other operations as GPU program input.
	/// </summary>
	[ShowInInspector]
	public partial class RenderTexture : RenderTarget
	{
		private RenderTexture(bool __dummy0) { }
		protected RenderTexture() { }

		/// <summary>Creates a new 2D render texture.</summary>
		/// <param name="format">Pixel format of the texture. Format must be a valid uncompressed color format.</param>
		/// <param name="width">Width of the texture in pixels.</param>
		/// <param name="height">Height of the texture in pixels.</param>
		/// <param name="numSamples">Number of samples contained per pixel.</param>
		/// <param name="gammaCorrection">Determines should the pixels written on the texture be gamma corrected.</param>
		/// <param name="createDepth">Should the render texture also contain a depth/stencil buffer.</param>
		/// <param name="depthStencilFormat">
		/// Format of the depth/stencil buffer, if <paramref name="createDepth"/> is enabled. Format must be a valid 
		/// depth/stencil format.
		/// </param>
		public RenderTexture(PixelFormat format, int width, int height, int numSamples = 1, bool gammaCorrection = false, bool createDepth = false, PixelFormat depthStencilFormat = PixelFormat.D32)
		{
			Internal_create(this, format, width, height, numSamples, gammaCorrection, createDepth, depthStencilFormat);
		}

		/// <summary>Creates a new 2D render texture using an existing color texture, and no depth-stencil texture.</summary>
		/// <param name="colorSurface">Color texture to render color data to.</param>
		public RenderTexture(Texture colorSurface)
		{
			Internal_create0(this, colorSurface);
		}

		/// <summary>Creates a new 2D render texture using existing textures as render destinations.</summary>
		/// <param name="colorSurface">Color texture to render color data to.</param>
		/// <param name="depthStencilSurface">Optional depth/stencil texture to render depth/stencil data to.</param>
		public RenderTexture(Texture colorSurface, Texture depthStencilSurface)
		{
			Internal_create1(this, colorSurface, depthStencilSurface);
		}

		/// <summary>Creates a new 2D render texture using one or multiple color textures and no depth-stencil texture.</summary>
		/// <param name="colorSurface">Color texture(s) to render color data to.</param>
		public RenderTexture(Texture[] colorSurface)
		{
			Internal_create2(this, colorSurface);
		}

		/// <summary>Creates a new 2D render texture using one or multiple color textures and a depth/stencil texture.</summary>
		/// <param name="colorSurface">Color texture(s) to render color data to.</param>
		/// <param name="depthStencilSurface">Optional depth/stencil texture to render depth/stencil data to.</param>
		public RenderTexture(Texture[] colorSurface, Texture depthStencilSurface)
		{
			Internal_create3(this, colorSurface, depthStencilSurface);
		}

		/// <summary>Returns the primary color surface that contains rendered color data.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Texture ColorSurface
		{
			get { return Internal_getColorSurface(mCachedPtr); }
		}

		/// <summary>Returns all the color surfaces.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Texture[] ColorSurfaces
		{
			get { return Internal_getColorSurfaces(mCachedPtr); }
		}

		/// <summary>Returns the depth/stencil surface that contains rendered depth and stencil data.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Texture DepthStencilSurface
		{
			get { return Internal_getDepthStencilSurface(mCachedPtr); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(RenderTexture managedInstance, PixelFormat format, int width, int height, int numSamples, bool gammaCorrection, bool createDepth, PixelFormat depthStencilFormat);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create0(RenderTexture managedInstance, Texture colorSurface);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create1(RenderTexture managedInstance, Texture colorSurface, Texture depthStencilSurface);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create2(RenderTexture managedInstance, Texture[] colorSurface);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create3(RenderTexture managedInstance, Texture[] colorSurface, Texture depthStencilSurface);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Texture Internal_getColorSurface(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Texture[] Internal_getColorSurfaces(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Texture Internal_getDepthStencilSurface(IntPtr thisPtr);
	}

	/** @} */
}
