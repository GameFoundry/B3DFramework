//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
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
			Internal_Create(this);
		}

		/// <summary>Pixel format to import as.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public PixelFormat Format
		{
			get { return Internal_GetFormat(mCachedPtr); }
			set { Internal_SetFormat(mCachedPtr, value); }
		}

		/// <summary>Enables or disables mipmap generation for the texture.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool GenerateMips
		{
			get { return Internal_GetGenerateMips(mCachedPtr); }
			set { Internal_SetGenerateMips(mCachedPtr, value); }
		}

		/// <summary>
		/// Maximum mip level to generate when generating mipmaps. If 0 then maximum amount of mip levels will be generated.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int MaxMip
		{
			get { return Internal_GetMaxMip(mCachedPtr); }
			set { Internal_SetMaxMip(mCachedPtr, value); }
		}

		/// <summary>Determines whether the texture data is also stored in main memory, available for fast CPU access.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool CpuCached
		{
			get { return Internal_GetCpuCached(mCachedPtr); }
			set { Internal_SetCpuCached(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines whether the texture data should be treated as if its in sRGB (gamma) space. Such texture will be converted 
		/// by hardware to linear space before use on the GPU.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool SRgb
		{
			get { return Internal_GetSRgb(mCachedPtr); }
			set { Internal_SetSRgb(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the texture be imported as a cubemap. See CubemapSourceType to choose how will the source texture 
		/// be converted to a cubemap.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Cubemap
		{
			get { return Internal_GetCubemap(mCachedPtr); }
			set { Internal_SetCubemap(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines how should the source texture be interpreted when generating a cubemap. Only relevant when <see 
		/// cref="Cubemap"/> is set to true.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public CubemapSourceType CubemapSourceType
		{
			get { return Internal_GetCubemapSourceType(mCachedPtr); }
			set { Internal_SetCubemapSourceType(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PixelFormat Internal_GetFormat(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFormat(IntPtr thisPtr, PixelFormat value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetGenerateMips(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetGenerateMips(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetMaxMip(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMaxMip(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetCpuCached(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCpuCached(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetSRgb(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSRgb(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetCubemap(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCubemap(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern CubemapSourceType Internal_GetCubemapSourceType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCubemapSourceType(IntPtr thisPtr, CubemapSourceType value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(TextureImportOptions managedInstance);
	}

	/** @} */
#endif
}
