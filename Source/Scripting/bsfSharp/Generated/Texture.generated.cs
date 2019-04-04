//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
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

		private Texture(PixelFormat format, int width, int height, int depth, TextureType texType, TextureUsage usage, int numSamples, bool hasMipmaps, bool gammaCorrection)
		{
			Internal_create(this, format, width, height, depth, texType, usage, numSamples, hasMipmaps, gammaCorrection);
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public RRef<Texture> Ref
		{
			get { return Internal_GetRef(mCachedPtr); }
		}

		/// <summary>Returns the pixel format for the texture surface.</summary>
		[NativeWrapper]
		public PixelFormat PixelFormat
		{
			get { return Internal_getPixelFormat(mCachedPtr); }
		}

		/// <summary>Returns a value that signals the engine in what way is the texture expected to be used.</summary>
		[NativeWrapper]
		public TextureUsage Usage
		{
			get { return Internal_getUsage(mCachedPtr); }
		}

		/// <summary>Gets the type of texture.</summary>
		[NativeWrapper]
		public TextureType Type
		{
			get { return Internal_getType(mCachedPtr); }
		}

		/// <summary>Returns the width of the texture.</summary>
		[NativeWrapper]
		public int Width
		{
			get { return Internal_getWidth(mCachedPtr); }
		}

		/// <summary>Returns the height of the texture.</summary>
		[NativeWrapper]
		public int Height
		{
			get { return Internal_getHeight(mCachedPtr); }
		}

		/// <summary>Returns the depth of the texture (only applicable for 3D textures).</summary>
		[NativeWrapper]
		public int Depth
		{
			get { return Internal_getDepth(mCachedPtr); }
		}

		/// <summary>
		/// Determines does the texture contain gamma corrected data. If true then the GPU will automatically convert the  pixels 
		/// to linear space before reading from the texture, and convert them to gamma space when writing to the texture.
		/// </summary>
		[NativeWrapper]
		public bool GammaSpace
		{
			get { return Internal_getGammaCorrection(mCachedPtr); }
		}

		/// <summary>Gets the number of samples used for multisampling (0 or 1 if multisampling is not used).</summary>
		[NativeWrapper]
		public int SampleCount
		{
			get { return Internal_getSampleCount(mCachedPtr); }
		}

		/// <summary>
		/// Gets the number of mipmaps to be used for this texture. This number excludes the top level map (which is always 
		/// assumed to be present).
		/// </summary>
		[NativeWrapper]
		public int MipMapCount
		{
			get { return Internal_getMipmapCount(mCachedPtr); }
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public static implicit operator RRef<Texture>(Texture x)
		{ return Internal_GetRef(x.mCachedPtr); }

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
			return Internal_getPixels(mCachedPtr, face, mipLevel);
		}

		/// <summary>
		/// Reads texture pixels directly from the GPU. This is similar to GetPixels but the texture doesn&apos;t need to be 
		/// created with TextureUsage.CPUCached, and the data will contain any updates performed by the GPU. This method can be 
		/// potentially slow as it introduces a CPU-GPU synchronization point. Additionally this method is asynchronous which 
		/// means the data is not available immediately.
		/// </summary>
		/// <param name="mipLevel">Mip level to retrieve pixels for. Top level (0) is the highest quality.</param>
		/// <param name="face">
		/// Face to read the pixels from. Cubemap textures have six faces whose face indices are as specified in the CubeFace 
		/// enum. Array textures can have an arbitrary number of faces (if it&apos;s a cubemap array it has to be a multiple of 
		/// 6).
		/// </param>
		/// <returns>AsyncOp object that will contain a PixelData object when the operation completes.</returns>
		public AsyncOp GetGPUPixels(int face = 0, int mipLevel = 0)
		{
			return Internal_getGPUPixels(mCachedPtr, face, mipLevel);
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
			Internal_setPixels(mCachedPtr, data, face, mipLevel);
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
			Internal_setPixelsArray(mCachedPtr, colors, face, mipLevel);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Texture> Internal_GetRef(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(Texture managedInstance, PixelFormat format, int width, int height, int depth, TextureType texType, TextureUsage usage, int numSamples, bool hasMipmaps, bool gammaCorrection);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PixelFormat Internal_getPixelFormat(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern TextureUsage Internal_getUsage(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern TextureType Internal_getType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getWidth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getHeight(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getDepth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getGammaCorrection(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getSampleCount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getMipmapCount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PixelData Internal_getPixels(IntPtr thisPtr, int face, int mipLevel);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AsyncOp Internal_getGPUPixels(IntPtr thisPtr, int face, int mipLevel);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setPixels(IntPtr thisPtr, PixelData data, int face, int mipLevel);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setPixelsArray(IntPtr thisPtr, Color[] colors, int face, int mipLevel);
	}

	/** @} */
}
