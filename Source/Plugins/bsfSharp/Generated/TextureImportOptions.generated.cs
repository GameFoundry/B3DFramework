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
#if !IS_B3D
	/** @addtogroup Importer
	 *  @{
	 */

	/// <summary>Contains import options you may use to control how is a texture imported.</summary>
	[ShowInInspector]
	public partial class TextureImportOptions : ImportOptions
	{
		private TextureImportOptions(bool __dummy0) { }

		/// <summary>Creates a new import options object that allows you to customize how are textures imported.</summary>
		public TextureImportOptions()
		{
			Internal_create(this);
		}

		/// <summary>Pixel format to import as.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public PixelFormat Format
		{
			get { return Internal_getformat(mCachedPtr); }
			set { Internal_setformat(mCachedPtr, value); }
		}

		/// <summary>Enables or disables mipmap generation for the texture.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool GenerateMips
		{
			get { return Internal_getgenerateMips(mCachedPtr); }
			set { Internal_setgenerateMips(mCachedPtr, value); }
		}

		/// <summary>
		/// Maximum mip level to generate when generating mipmaps. If 0 then maximum amount of mip levels will be generated.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int MaxMip
		{
			get { return Internal_getmaxMip(mCachedPtr); }
			set { Internal_setmaxMip(mCachedPtr, value); }
		}

		/// <summary>Determines whether the texture data is also stored in main memory, available for fast CPU access.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool CpuCached
		{
			get { return Internal_getcpuCached(mCachedPtr); }
			set { Internal_setcpuCached(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines whether the texture data should be treated as if its in sRGB (gamma) space. Such texture will be  
		/// converted by hardware to linear space before use on the GPU.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool SRGB
		{
			get { return Internal_getsRGB(mCachedPtr); }
			set { Internal_setsRGB(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the texture be imported as a cubemap. See setCubemapSource to choose how will the source texture be 
		/// converted to a cubemap.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Cubemap
		{
			get { return Internal_getcubemap(mCachedPtr); }
			set { Internal_setcubemap(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines how should the source texture be interpreted when generating a cubemap. Only relevant when <see 
		/// cref="cubemap"/> is set to true.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public CubemapSourceType CubemapSourceType
		{
			get { return Internal_getcubemapSourceType(mCachedPtr); }
			set { Internal_setcubemapSourceType(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PixelFormat Internal_getformat(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setformat(IntPtr thisPtr, PixelFormat value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getgenerateMips(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setgenerateMips(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getmaxMip(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setmaxMip(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getcpuCached(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setcpuCached(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getsRGB(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setsRGB(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getcubemap(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setcubemap(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern CubemapSourceType Internal_getcubemapSourceType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setcubemapSourceType(IntPtr thisPtr, CubemapSourceType value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(TextureImportOptions managedInstance);
	}

	/** @} */
#endif
}
