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

	/// <summary>Wraps Decal as a Component.</summary>
	[ShowInInspector]
	public partial class Decal : Component
	{
		private Decal(bool __dummy0) { }
		protected Decal() { }

		/// <summary>Determines the material to use when rendering the decal.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public RRef<Material> Material
		{
			get { return Internal_getMaterial(mCachedPtr); }
			set { Internal_setMaterial(mCachedPtr, value); }
		}

		/// <summary>Size of the decal in world space units.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector2 Size
		{
			get
			{
				Vector2 temp;
				Internal_getSize(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setSize(mCachedPtr, ref value); }
		}

		/// <summary>Determines the maximum distance (from its origin) at which the decal is displayed.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float MaxDistance
		{
			get { return Internal_getMaxDistance(mCachedPtr); }
			set { Internal_setMaxDistance(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the layer that controls whether a system is considered visible in a specific camera. Layer must match  
		/// camera layer bitfield in order for the camera to render the decal.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public ulong Layer
		{
			get { return Internal_getLayer(mCachedPtr); }
			set { Internal_setLayer(mCachedPtr, value); }
		}

		/// <summary>
		/// Bitfield that allows you to mask on which objects will the decal be projected onto. Only objects with the  matching 
		/// layers will be projected onto. Note that decal layer mask only supports 32-bits and objects with layers in bits >= 32 
		/// will always be projected onto.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public uint LayerMask
		{
			get { return Internal_getLayerMask(mCachedPtr); }
			set { Internal_setLayerMask(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setMaterial(IntPtr thisPtr, RRef<Material> material);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Material> Internal_getMaterial(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setSize(IntPtr thisPtr, ref Vector2 size);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getSize(IntPtr thisPtr, out Vector2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setMaxDistance(IntPtr thisPtr, float distance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getMaxDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setLayer(IntPtr thisPtr, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ulong Internal_getLayer(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setLayerMask(IntPtr thisPtr, uint mask);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern uint Internal_getLayerMask(IntPtr thisPtr);
	}

	/** @} */
}
