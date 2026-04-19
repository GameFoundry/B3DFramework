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
	/// Specifies a location at which a pre-computed texture containing scene radiance will be generated. This texture will 
	/// then be used by the renderer to provide specular reflections.
	/// </summary>
	[ShowInInspector]
	public partial class ReflectionProbe : Component
	{
		private ReflectionProbe(bool __dummy0) { }
		protected ReflectionProbe() { }

		/// <summary>Determines the type of the probe.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ReflectionProbeType Type
		{
			get { return Internal_GetType(mCachedPtr); }
			set { Internal_SetType(mCachedPtr, value); }
		}

		/// <summary>Determines the radius of a sphere reflection probe.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Radius
		{
			get { return Internal_GetRadius(mCachedPtr); }
			set { Internal_SetRadius(mCachedPtr, value); }
		}

		/// <summary>Determines the extents of a box reflection probe. Determines range of influence.</summary>
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

		/// <summary>
		/// Allows you assign a custom texture to use as a reflection map. This will disable automatic generation of reflections. 
		/// To re-enable auto-generation call this with a null parameter.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public RRef<Texture> CustomTexture
		{
			get { return Internal_GetCustomTexture(mCachedPtr); }
			set { Internal_SetCustomTexture(mCachedPtr, value); }
		}

		/// <summary>Returns the radius of a sphere reflection probe, scaled by scene object transform.</summary>
		[NativeWrapper]
		public float WorldRadius
		{
			get { return Internal_GetWorldRadius(mCachedPtr); }
		}

		/// <summary>Returns the extents of a box reflection probe, scaled by scene object transform.</summary>
		[NativeWrapper]
		public Vector3 WorldExtents
		{
			get
			{
				Vector3 temp;
				Internal_GetWorldExtents(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>
		/// Captures the scene at the current location and generates a filtered reflection cubemap. No action is taken if a 
		/// custom texture is set.
		/// </summary>
		public void Capture()
		{
			Internal_Capture(mCachedPtr);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetType(IntPtr thisPtr, ReflectionProbeType type);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetRadius(IntPtr thisPtr, float radius);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetExtents(IntPtr thisPtr, ref Vector3 extents);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCustomTexture(IntPtr thisPtr, RRef<Texture> texture);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Texture> Internal_GetCustomTexture(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetWorldRadius(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetWorldExtents(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Capture(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ReflectionProbeType Internal_GetType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetRadius(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetExtents(IntPtr thisPtr, out Vector3 __output);
	}

	/** @} */
}
