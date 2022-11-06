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

	/// <summary>Wraps Decal as a Component.</summary>
	[ShowInInspector]
	public partial class Decal : Component
	{
		private Decal(bool __dummy0) { }
		protected Decal() { }

		[ShowInInspector]
		[NativeWrapper]
		public RRef<Material> Material
		{
			get { return Internal_GetMaterial(mCachedPtr); }
			set { Internal_SetMaterial(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public Vector2 Size
		{
			get
			{
				Vector2 temp;
				Internal_GetSize(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetSize(mCachedPtr, ref value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float MaxDistance
		{
			get { return Internal_GetMaxDistance(mCachedPtr); }
			set { Internal_SetMaxDistance(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public ulong Layer
		{
			get { return Internal_GetLayer(mCachedPtr); }
			set { Internal_SetLayer(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public int LayerMask
		{
			get { return Internal_GetLayerMask(mCachedPtr); }
			set { Internal_SetLayerMask(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMaterial(IntPtr thisPtr, RRef<Material> material);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Material> Internal_GetMaterial(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSize(IntPtr thisPtr, ref Vector2 size);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSize(IntPtr thisPtr, out Vector2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMaxDistance(IntPtr thisPtr, float distance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetMaxDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetLayer(IntPtr thisPtr, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ulong Internal_GetLayer(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetLayerMask(IntPtr thisPtr, int mask);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetLayerMask(IntPtr thisPtr);
	}

	/** @} */
}
