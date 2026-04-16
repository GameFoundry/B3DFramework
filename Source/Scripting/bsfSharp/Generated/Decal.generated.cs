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

	/// <summary>Wraps Decal as a Component.</summary>
	[ShowInInspector]
	public partial class Decal : Component
	{
		private Decal(bool __dummy0) { }
		protected Decal() { }

		/// <summary>Width and height of the decal.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public TVector2<float> Size
		{
			get
			{
				TVector2<float> temp;
				Internal_GetSize(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetSize(mCachedPtr, ref value); }
		}

		/// <summary>Determines the material to use when rendering the decal.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public RRef<Material> Material
		{
			get { return Internal_GetMaterial(mCachedPtr); }
			set { Internal_SetMaterial(mCachedPtr, value); }
		}

		/// <summary>Determines the maximum distance (from its origin) at which the decal is displayed.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float MaxDistance
		{
			get { return Internal_GetMaxDistance(mCachedPtr); }
			set { Internal_SetMaxDistance(mCachedPtr, value); }
		}

		/// <summary>
		/// Bitfield that allows you to mask on which objects will the decal be projected onto. Only objects with the matching 
		/// layers will be projected onto. Note that decal layer mask only supports 32-bits and objects with layers in bits &gt;= 
		/// 32 will always be projected onto.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int LayerMask
		{
			get { return Internal_GetLayerMask(mCachedPtr); }
			set { Internal_SetLayerMask(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the layer that controls whether a system is considered visible in a specific camera. Layer must match 
		/// camera layer bitfield in order for the camera to render the decal.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public ulong Layer
		{
			get { return Internal_GetLayer(mCachedPtr); }
			set { Internal_SetLayer(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSize(IntPtr thisPtr, ref TVector2<float> size);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMaterial(IntPtr thisPtr, RRef<Material> material);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMaxDistance(IntPtr thisPtr, float distance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetLayerMask(IntPtr thisPtr, int mask);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetLayer(IntPtr thisPtr, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSize(IntPtr thisPtr, out TVector2<float> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Material> Internal_GetMaterial(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetMaxDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetLayerMask(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ulong Internal_GetLayer(IntPtr thisPtr);
	}

	/** @} */
}
