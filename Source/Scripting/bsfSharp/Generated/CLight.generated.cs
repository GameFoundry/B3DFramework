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
	public partial class Light : Component
	{
		private Light(bool __dummy0) { }
		protected Light() { }

		[ShowInInspector]
		[NativeWrapper]
		public LightType Type
		{
			get { return Internal_GetType(mCachedPtr); }
			set { Internal_SetType(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public Color Color
		{
			get
			{
				Color temp;
				Internal_GetColor(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetColor(mCachedPtr, ref value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float Intensity
		{
			get { return Internal_GetIntensity(mCachedPtr); }
			set { Internal_SetIntensity(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public bool UseAutoAttenuation
		{
			get { return Internal_GetUseAutoAttenuation(mCachedPtr); }
			set { Internal_SetUseAutoAttenuation(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float AttenuationRadius
		{
			get { return Internal_GetAttenuationRadius(mCachedPtr); }
			set { Internal_SetAttenuationRadius(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float SourceRadius
		{
			get { return Internal_GetSourceRadius(mCachedPtr); }
			set { Internal_SetSourceRadius(mCachedPtr, value); }
		}

		[ShowInInspector]
		[Range(1f, 180f, true)]
		[NativeWrapper]
		public Degree SpotAngle
		{
			get
			{
				Degree temp;
				Internal_GetSpotAngle(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetSpotAngle(mCachedPtr, ref value); }
		}

		[ShowInInspector]
		[Range(1f, 180f, true)]
		[NativeWrapper]
		public Degree SpotAngleFalloff
		{
			get
			{
				Degree temp;
				Internal_GetSpotFalloffAngle(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetSpotFalloffAngle(mCachedPtr, ref value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public bool CastsShadow
		{
			get { return Internal_GetCastsShadow(mCachedPtr); }
			set { Internal_SetCastsShadow(mCachedPtr, value); }
		}

		[ShowInInspector]
		[Range(-1f, 1f, true)]
		[NativeWrapper]
		public float ShadowBias
		{
			get { return Internal_GetShadowBias(mCachedPtr); }
			set { Internal_SetShadowBias(mCachedPtr, value); }
		}

		[NativeWrapper]
		public Sphere Bounds
		{
			get
			{
				Sphere temp;
				Internal_GetBounds(mCachedPtr, out temp);
				return temp;
			}
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetType(IntPtr thisPtr, LightType type);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern LightType Internal_GetType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetColor(IntPtr thisPtr, ref Color color);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetColor(IntPtr thisPtr, out Color __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetIntensity(IntPtr thisPtr, float intensity);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetIntensity(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetUseAutoAttenuation(IntPtr thisPtr, bool enabled);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetUseAutoAttenuation(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetAttenuationRadius(IntPtr thisPtr, float radius);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetAttenuationRadius(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSourceRadius(IntPtr thisPtr, float radius);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetSourceRadius(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSpotAngle(IntPtr thisPtr, ref Degree spotAngle);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSpotAngle(IntPtr thisPtr, out Degree __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSpotFalloffAngle(IntPtr thisPtr, ref Degree spotAngle);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSpotFalloffAngle(IntPtr thisPtr, out Degree __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCastsShadow(IntPtr thisPtr, bool castsShadow);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetCastsShadow(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetShadowBias(IntPtr thisPtr, float bias);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetShadowBias(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetBounds(IntPtr thisPtr, out Sphere __output);
	}

	/** @} */
}
