//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
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
			get { return Internal_GetVectorField(mCachedPtr); }
			set { Internal_SetVectorField(mCachedPtr, value); }
		}

		/// <summary>Intensity of the forces and velocities applied by the vector field.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Intensity
		{
			get { return Internal_GetIntensity(mCachedPtr); }
			set { Internal_SetIntensity(mCachedPtr, value); }
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
			get { return Internal_GetTightness(mCachedPtr); }
			set { Internal_SetTightness(mCachedPtr, value); }
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
				Internal_GetScale(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetScale(mCachedPtr, ref value); }
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
				Internal_GetOffset(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetOffset(mCachedPtr, ref value); }
		}

		/// <summary>Initial rotation of the vector field.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Quaternion Rotation
		{
			get
			{
				Quaternion temp;
				Internal_GetRotation(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetRotation(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Determines the amount to rotate the vector field every second, in degrees, around XYZ axis respectively. Evaluated 
		/// over the particle system lifetime.
		/// </summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public Vector3Distribution RotationRate
		{
			get { return Internal_GetRotationRate(mCachedPtr); }
			set { Internal_SetRotationRate(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the field influence particles outside of the field bounds. If true the field will be tiled 
		/// infinitely in the X direction.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool TilingX
		{
			get { return Internal_GetTilingX(mCachedPtr); }
			set { Internal_SetTilingX(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the field influence particles outside of the field bounds. If true the field will be tiled 
		/// infinitely in the Y direction.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool TilingY
		{
			get { return Internal_GetTilingY(mCachedPtr); }
			set { Internal_SetTilingY(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the field influence particles outside of the field bounds. If true the field will be tiled 
		/// infinitely in the Z direction.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool TilingZ
		{
			get { return Internal_GetTilingZ(mCachedPtr); }
			set { Internal_SetTilingZ(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<VectorField> Internal_GetVectorField(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetVectorField(IntPtr thisPtr, RRef<VectorField> value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetIntensity(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetIntensity(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetTightness(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTightness(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetScale(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetScale(IntPtr thisPtr, ref Vector3 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetOffset(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetOffset(IntPtr thisPtr, ref Vector3 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetRotation(IntPtr thisPtr, out Quaternion __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetRotation(IntPtr thisPtr, ref Quaternion value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector3Distribution Internal_GetRotationRate(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetRotationRate(IntPtr thisPtr, Vector3Distribution value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetTilingX(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTilingX(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetTilingY(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTilingY(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetTilingZ(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTilingZ(IntPtr thisPtr, bool value);
	}

	/** @} */
}
