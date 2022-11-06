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
	public partial class ReflectionProbe : Component
	{
		private ReflectionProbe(bool __dummy0) { }
		protected ReflectionProbe() { }

		[ShowInInspector]
		[NativeWrapper]
		public ReflectionProbeType Type
		{
			get { return Internal_GetType(mCachedPtr); }
			set { Internal_SetType(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float Radius
		{
			get { return Internal_GetRadius(mCachedPtr); }
			set { Internal_SetRadius(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public Vector3 Extents
		{
			get
			{
				Vector3 temp;
				Internal_GetExtents(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetExtents(mCachedPtr, ref value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public RRef<Texture> CustomTexture
		{
			get { return Internal_GetCustomTexture(mCachedPtr); }
			set { Internal_SetCustomTexture(mCachedPtr, value); }
		}

		public void Capture()
		{
			Internal_Capture(mCachedPtr);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ReflectionProbeType Internal_GetType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetType(IntPtr thisPtr, ReflectionProbeType type);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetRadius(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetRadius(IntPtr thisPtr, float radius);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetExtents(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetExtents(IntPtr thisPtr, ref Vector3 extents);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Texture> Internal_GetCustomTexture(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCustomTexture(IntPtr thisPtr, RRef<Texture> texture);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Capture(IntPtr thisPtr);
	}

	/** @} */
}
