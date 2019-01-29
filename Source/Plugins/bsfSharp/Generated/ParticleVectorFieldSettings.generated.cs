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
	/** @addtogroup Particles
	 *  @{
	 */

	/// <summary>Settings used for controlling a vector field in a GPU simulated particle system.</summary>
	[ShowInInspector]
	public partial class ParticleVectorFieldSettings : ScriptObject
	{
		private ParticleVectorFieldSettings(bool __dummy0) { }
		protected ParticleVectorFieldSettings() { }

		/// <summary>Vector field resource used for influencing the particles.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public RRef<VectorField> VectorField
		{
			get { return Internal_getvectorField(mCachedPtr); }
			set { Internal_setvectorField(mCachedPtr, value); }
		}

		/// <summary>Intensity of the forces and velocities applied by the vector field.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Intensity
		{
			get { return Internal_getintensity(mCachedPtr); }
			set { Internal_setintensity(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines how closely does the particle velocity follow the vectors in the field. If set to 1 particles will be 
		/// snapped to the exact velocity of the value in the field, and if set to 0 the field will not influence particle 
		/// velocities directly.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Tightness
		{
			get { return Internal_gettightness(mCachedPtr); }
			set { Internal_settightness(mCachedPtr, value); }
		}

		/// <summary>
		/// Scale to apply to the vector field bounds. This is multiplied with the bounds of the vector field resource.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3 Scale
		{
			get
			{
				Vector3 temp;
				Internal_getscale(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setscale(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Amount of to move the vector field by relative to the parent particle system. This is added to the bounds provided in 
		/// the vector field resource.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3 Offset
		{
			get
			{
				Vector3 temp;
				Internal_getoffset(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setoffset(mCachedPtr, ref value); }
		}

		/// <summary>Initial rotation of the vector field.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Quaternion Rotation
		{
			get
			{
				Quaternion temp;
				Internal_getrotation(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setrotation(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Determines the amount to rotate the vector field every second, in degrees, around XYZ axis respectively.  Evaluated 
		/// over the particle system lifetime.
		/// </summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public Vector3Distribution RotationRate
		{
			get { return Internal_getrotationRate(mCachedPtr); }
			set { Internal_setrotationRate(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the field influence particles outside of the field bounds. If true the field will be tiled 
		/// infinitely in the X direction.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool TilingX
		{
			get { return Internal_gettilingX(mCachedPtr); }
			set { Internal_settilingX(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the field influence particles outside of the field bounds. If true the field will be tiled 
		/// infinitely in the Y direction.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool TilingY
		{
			get { return Internal_gettilingY(mCachedPtr); }
			set { Internal_settilingY(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the field influence particles outside of the field bounds. If true the field will be tiled 
		/// infinitely in the Z direction.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool TilingZ
		{
			get { return Internal_gettilingZ(mCachedPtr); }
			set { Internal_settilingZ(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<VectorField> Internal_getvectorField(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setvectorField(IntPtr thisPtr, RRef<VectorField> value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getintensity(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setintensity(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_gettightness(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_settightness(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getscale(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setscale(IntPtr thisPtr, ref Vector3 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getoffset(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setoffset(IntPtr thisPtr, ref Vector3 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getrotation(IntPtr thisPtr, out Quaternion __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setrotation(IntPtr thisPtr, ref Quaternion value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector3Distribution Internal_getrotationRate(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setrotationRate(IntPtr thisPtr, Vector3Distribution value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_gettilingX(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_settilingX(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_gettilingY(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_settilingY(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_gettilingZ(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_settilingZ(IntPtr thisPtr, bool value);
	}

	/** @} */
}
