//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>Allows you to specify an environment map to use for sampling radiance of the sky.</summary>
	[ShowInInspector]
	public partial class Skybox : Component
	{
		private Skybox(bool __dummy0) { }
		protected Skybox() { }

		/// <summary>
		/// Determines an environment map to use for sampling skybox radiance. Must be a cube-map texture, and should ideally 
		/// contain HDR data.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public RRef<Texture> Texture
		{
			get { return Internal_GetTexture(mCachedPtr); }
			set { Internal_SetTexture(mCachedPtr, value); }
		}

		/// <summary>
		/// Brightness multiplier that will be applied to skybox values before they&apos;re being used. Allows you to make the 
		/// skybox more or less bright. Equal to one by default.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Brightness
		{
			get { return Internal_GetBrightness(mCachedPtr); }
			set { Internal_SetBrightness(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTexture(IntPtr thisPtr, RRef<Texture> texture);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetBrightness(IntPtr thisPtr, float brightness);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetBrightness(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Texture> Internal_GetTexture(IntPtr thisPtr);
	}

	/** @} */
}
