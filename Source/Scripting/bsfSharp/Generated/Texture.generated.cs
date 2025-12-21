//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>
	/// Abstract class representing a texture. Specific render systems have their own Texture implementations. Internally 
	/// represented as one or more surfaces with pixels in a certain number of dimensions, backed by a hardware buffer.
	/// </summary>
	[ShowInInspector]
	public partial class Texture : Resource
	{
		private Texture(bool __dummy0) { }
		protected Texture() { }

		private Texture(PixelFormat format, int width, int height, int depth, TextureType texType, int usage, int numSamples, bool hasMipmaps, bool gammaCorrection)
		{
			Internal_Create(this, format, width, height, depth, texType, usage, numSamples, hasMipmaps, gammaCorrection);
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public RRef<Texture> Ref
		{
			get { return Internal_GetRef(mCachedPtr); }
		}

		[NativeWrapper]
		public PixelFormat PixelFormat
		{
			get { return Internal_GetPixelFormat(mCachedPtr); }
		}

		[NativeWrapper]
		public int Usage
		{
			get { return Internal_GetUsage(mCachedPtr); }
		}

		[NativeWrapper]
		public TextureType Type
		{
			get { return Internal_GetType(mCachedPtr); }
		}

		[NativeWrapper]
		public int Width
		{
			get { return Internal_GetWidth(mCachedPtr); }
		}

		[NativeWrapper]
		public int Height
		{
			get { return Internal_GetHeight(mCachedPtr); }
		}

		[NativeWrapper]
		public int Depth
		{
			get { return Internal_GetDepth(mCachedPtr); }
		}

		[NativeWrapper]
		public bool GammaSpace
		{
			get { return Internal_GetGammaCorrection(mCachedPtr); }
		}

		[NativeWrapper]
		public int SampleCount
		{
			get { return Internal_GetSampleCount(mCachedPtr); }
		}

		[NativeWrapper]
		public int MipMapCount
		{
			get { return Internal_GetMipmapCount(mCachedPtr); }
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public static implicit operator RRef<Texture>(Texture x)
		{
			if(x != null)
				return Internal_GetRef(x.mCachedPtr);
			else
				return null;
		}

		/// <summary>Reads internal texture data into a newly allocated buffer.</summary>
		/// <param name="face">Texture face to read from.</param>
		/// <param name="mipLevel">Mipmap level to read from.</param>
		/// <returns>Async operation object that will contain the buffer with the data once the operation completes.</returns>
		public AsyncOp<PixelData> GetGPUPixels(int face = 0, int mipLevel = 0)
		{
			return Internal_ReadData(mCachedPtr, face, mipLevel);
		}

		/// <summary>
		/// Returns pixels for the specified mip level &amp; face. Pixels will be read from system memory, which means the 
		/// texture has to be created with TextureUsage.CPUCached. If the texture was updated from the GPU the pixels retrieved 
		/// from this method will not reflect that, and you should use GetGPUPixels instead.
		/// </summary>
		/// <param name="mipLevel">Mip level to retrieve pixels for. Top level (0) is the highest quality.</param>
		/// <param name="face">
		/// Face to read the pixels from. Cubemap textures have six faces whose face indices are as specified in the CubeFace 
		/// enum. Array textures can have an arbitrary number of faces (if it&apos;s a cubemap array it has to be a multiple of 
		/// 6).
		/// </param>
		/// <returns>A set of pixels for the specified mip level.</returns>
		public PixelData GetPixels(int face = 0, int mipLevel = 0)
		{
			return Internal_GetPixels(mCachedPtr, face, mipLevel);
		}

		/// <summary>Sets pixels for the specified mip level and face.</summary>
		/// <param name="data">
		/// Pixels to assign to the specified mip level. Pixel data must match the mip level size and texture pixel format.
		/// </param>
		/// <param name="mipLevel">Mip level to set pixels for. Top level (0) is the highest quality.</param>
		/// <param name="face">
		/// Face to write the pixels to. Cubemap textures have six faces whose face indices are as specified in the CubeFace 
		/// enum. Array textures can have an arbitrary number of faces (if it&apos;s a cubemap array it has to be a multiple of 
		/// 6).
		/// </param>
		public void SetPixels(PixelData data, int face = 0, int mipLevel = 0)
		{
			Internal_SetPixels(mCachedPtr, data, face, mipLevel);
		}

		/// <summary>Sets pixels for the specified mip level and face.</summary>
		/// <param name="colors">
		/// Pixels to assign to the specified mip level. Size of the array must match the mip level dimensions. Data is expected 
		/// to be laid out row by row. Pixels will be automatically converted to the valid pixel format.
		/// </param>
		/// <param name="mipLevel">Mip level to set pixels for. Top level (0) is the highest quality.</param>
		/// <param name="face">
		/// Face to write the pixels to. Cubemap textures have six faces whose face indices are as specified in the CubeFace 
		/// enum. Array textures can have an arbitrary number of faces (if it&apos;s a cubemap array it has to be a multiple of 
		/// 6).
		/// </param>
		public void SetPixels(Color[] colors, int face = 0, int mipLevel = 0)
		{
			Internal_SetPixelsArray(mCachedPtr, colors, face, mipLevel);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Texture> Internal_GetRef(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AsyncOp<PixelData> Internal_ReadData(IntPtr thisPtr, int face, int mipLevel);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(Texture managedInstance, PixelFormat format, int width, int height, int depth, TextureType texType, int usage, int numSamples, bool hasMipmaps, bool gammaCorrection);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PixelFormat Internal_GetPixelFormat(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetUsage(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern TextureType Internal_GetType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetWidth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetHeight(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetDepth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetGammaCorrection(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetSampleCount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetMipmapCount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PixelData Internal_GetPixels(IntPtr thisPtr, int face, int mipLevel);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetPixels(IntPtr thisPtr, PixelData data, int face, int mipLevel);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetPixelsArray(IntPtr thisPtr, Color[] colors, int face, int mipLevel);
	}

	/** @} */
}
