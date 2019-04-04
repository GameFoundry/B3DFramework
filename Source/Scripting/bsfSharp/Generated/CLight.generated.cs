//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>Illuminates a portion of the scene covered by the light.</summary>
	[ShowInInspector]
	public partial class Light : Component
	{
		private Light(bool __dummy0) { }
		protected Light() { }

		/// <summary>Determines the type of the light.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public LightType Type
		{
			get { return Internal_getType(mCachedPtr); }
			set { Internal_setType(mCachedPtr, value); }
		}

		/// <summary>Determines does this light cast shadows when rendered.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool CastsShadow
		{
			get { return Internal_getCastsShadow(mCachedPtr); }
			set { Internal_setCastsShadow(mCachedPtr, value); }
		}

		/// <summary>
		/// Shadow bias determines offset at which the shadows are rendered from the shadow caster. Bias value of 0 means the 
		/// shadow will be renderered exactly at the casters position. If your geometry has thin areas this will produce an 
		/// artifact called shadow acne, in which case you can increase the shadow bias value to eliminate it.  Note that 
		/// increasing the shadow bias will on the other hand make the shadow be offset from the caster and may make the caster 
		/// appear as if floating (Peter Panning artifact). Neither is perfect, so it is preferable to ensure all your geometry 
		/// has thickness and keep the bias at zero, or even at negative values.
		///
		/// Default value is 0.5. Should be in rough range [-1, 1].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float ShadowBias
		{
			get { return Internal_getShadowBias(mCachedPtr); }
			set { Internal_setShadowBias(mCachedPtr, value); }
		}

		/// <summary>Determines the color emitted by the light.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Color Color
		{
			get
			{
				Color temp;
				Internal_getColor(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setColor(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Range at which the light contribution fades out to zero. Use setUseAutoAttenuation to provide a radius automatically 
		/// dependant on light intensity. The radius will cut-off light contribution and therefore manually set very small radius 
		/// can end up being very physically incorrect.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float AttenuationRadius
		{
			get { return Internal_getAttenuationRadius(mCachedPtr); }
			set { Internal_setAttenuationRadius(mCachedPtr, value); }
		}

		/// <summary>
		/// Radius of the light source. If non-zero then this light represents an area light, otherwise it is a punctual light. 
		/// Area lights have different attenuation then punctual lights, and their appearance in specular reflections is 
		/// realistic. Shape of the area light depends on light type: - For directional light the shape is a disc projected on 
		/// the hemisphere on the sky. This parameter represents angular radius (in degrees) of the disk and should be very small 
		/// (think of how much space the Sun takes on the sky - roughly 0.25 degree radius). - For radial light the shape is a 
		/// sphere and the source radius is the radius of the sphere. - For spot lights the shape is a disc oriented in the 
		/// direction of the spot light and the source radius is the radius of the disc.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float SourceRadius
		{
			get { return Internal_getSourceRadius(mCachedPtr); }
			set { Internal_setSourceRadius(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the power of the light source. This will be luminous flux for radial &amp; spot lights,  luminance for 
		/// directional lights with no area, and illuminance for directional lights with area (non-zero source radius).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Intensity
		{
			get { return Internal_getIntensity(mCachedPtr); }
			set { Internal_setIntensity(mCachedPtr, value); }
		}

		/// <summary>Determines the total angle covered by a spot light.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Degree SpotAngle
		{
			get
			{
				Degree temp;
				Internal_getSpotAngle(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setSpotAngle(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Determines the falloff angle covered by a spot light. Falloff angle determines at what point does light intensity  
		/// starts quadratically falling off as the angle approaches the total spot angle.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Degree SpotAngleFalloff
		{
			get
			{
				Degree temp;
				Internal_getSpotFalloffAngle(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setSpotFalloffAngle(mCachedPtr, ref value); }
		}

		/// <summary>
		/// If enabled the attenuation radius will automatically be controlled in order to provide reasonable light radius,  
		/// depending on its intensity.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool UseAutoAttenuation
		{
			get { return Internal_getUseAutoAttenuation(mCachedPtr); }
			set { Internal_setUseAutoAttenuation(mCachedPtr, value); }
		}

		/// <summary>Returns world space bounds that completely encompass the light&apos;s area of influence.</summary>
		[NativeWrapper]
		public Sphere Bounds
		{
			get
			{
				Sphere temp;
				Internal_getBounds(mCachedPtr, out temp);
				return temp;
			}
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setType(IntPtr thisPtr, LightType type);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern LightType Internal_getType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setCastsShadow(IntPtr thisPtr, bool castsShadow);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getCastsShadow(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setShadowBias(IntPtr thisPtr, float bias);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getShadowBias(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setColor(IntPtr thisPtr, ref Color color);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getColor(IntPtr thisPtr, out Color __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setAttenuationRadius(IntPtr thisPtr, float radius);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getAttenuationRadius(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setSourceRadius(IntPtr thisPtr, float radius);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getSourceRadius(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setIntensity(IntPtr thisPtr, float intensity);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getIntensity(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setSpotAngle(IntPtr thisPtr, ref Degree spotAngle);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getSpotAngle(IntPtr thisPtr, out Degree __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setSpotFalloffAngle(IntPtr thisPtr, ref Degree spotAngle);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getSpotFalloffAngle(IntPtr thisPtr, out Degree __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setUseAutoAttenuation(IntPtr thisPtr, bool enabled);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getUseAutoAttenuation(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getBounds(IntPtr thisPtr, out Sphere __output);
	}

	/** @} */
}
