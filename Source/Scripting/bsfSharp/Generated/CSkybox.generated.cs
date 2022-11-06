//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Rendering
	 *  @{
	 */

	[ShowInInspector]
	public partial class Skybox : Component
	{
		private Skybox(bool __dummy0) { }
		protected Skybox() { }

		[ShowInInspector]
		[NativeWrapper]
		public RRef<Texture> Texture
		{
			get { return Internal_GetTexture(mCachedPtr); }
			set { Internal_SetTexture(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float Brightness
		{
			get { return Internal_GetBrightness(mCachedPtr); }
			set { Internal_SetBrightness(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Texture> Internal_GetTexture(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTexture(IntPtr thisPtr, RRef<Texture> texture);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetBrightness(IntPtr thisPtr, float brightness);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetBrightness(IntPtr thisPtr);
	}

	/** @} */
}
