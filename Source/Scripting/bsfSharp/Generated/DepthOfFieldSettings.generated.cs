//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Settings that control the depth-of-field effect.</summary>
	[ShowInInspector]
	public partial class DepthOfFieldSettings : ScriptObject
	{
		private DepthOfFieldSettings(bool __dummy0) { }

		public DepthOfFieldSettings()
		{
			Internal_DepthOfFieldSettings(this);
		}

		/// <summary>Texture to use for the bokeh shape. Only relevant when using Bokeh depth of field.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public RRef<Texture> BokehShape
		{
			get { return Internal_getbokehShape(mCachedPtr); }
			set { Internal_setbokehShape(mCachedPtr, value); }
		}

		/// <summary>Enables or disables the depth of field effect.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Enabled
		{
			get { return Internal_getenabled(mCachedPtr); }
			set { Internal_setenabled(mCachedPtr, value); }
		}

		/// <summary>Type of depth of field effect to use.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public DepthOfFieldType Type
		{
			get { return Internal_gettype(mCachedPtr); }
			set { Internal_settype(mCachedPtr, value); }
		}

		/// <summary>
		/// Distance from the camera at which the focal plane is located in. Objects at this distance will be fully in focus. In 
		/// world units (meters).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FocalDistance
		{
			get { return Internal_getfocalDistance(mCachedPtr); }
			set { Internal_setfocalDistance(mCachedPtr, value); }
		}

		/// <summary>
		/// Range within which the objects remain fully in focus. This range is applied relative to the focal distance. This 
		/// parameter should usually be non-zero when using the Gaussian depth of field effect. When using other types of 
		/// depth-of-field you can set this to zero for a more physically-based effect, or keep it non-zero for more artistic 
		/// control. In world units (meters).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FocalRange
		{
			get { return Internal_getfocalRange(mCachedPtr); }
			set { Internal_setfocalRange(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the size of the range within which objects transition from focused to fully unfocused, at the near  plane. 
		/// Only relevant for Gaussian and Bokeh depth of field. In world units (meters).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float NearTransitionRange
		{
			get { return Internal_getnearTransitionRange(mCachedPtr); }
			set { Internal_setnearTransitionRange(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the size of the range within which objects transition from focused to fully unfocused, at the far  plane. 
		/// Only relevant for Gaussian and Bokeh depth of field. In world units (meters).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FarTransitionRange
		{
			get { return Internal_getfarTransitionRange(mCachedPtr); }
			set { Internal_setfarTransitionRange(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the amount of blur to apply to fully unfocused objects that are closer to camera than the in-focus zone. 
		/// Set to zero to disable near-field blur. Only relevant for Gaussian depth of field.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float NearBlurAmount
		{
			get { return Internal_getnearBlurAmount(mCachedPtr); }
			set { Internal_setnearBlurAmount(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the amount of blur to apply to fully unfocused objects that are farther away from camera than the in-focus 
		/// zone. Set to zero to disable far-field blur. Only relevant for Gaussian depth of field.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FarBlurAmount
		{
			get { return Internal_getfarBlurAmount(mCachedPtr); }
			set { Internal_setfarBlurAmount(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the maximum size of the blur kernel, in percent of view size. Larger values cost more performance.  Only 
		/// relevant when using Bokeh depth of field.
		/// </summary>
		[ShowInInspector]
		[Range(0f, 1f, false)]
		[NativeWrapper]
		public float MaxBokehSize
		{
			get { return Internal_getmaxBokehSize(mCachedPtr); }
			set { Internal_setmaxBokehSize(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the maximum color difference between surrounding pixels allowed (as a sum of all channels) before higher 
		/// fidelity sampling is triggered. Increasing this value can improve performance as less higher fidelity samples will be 
		/// required, but may decrease quality of the effect. Only relevant when using Bokeh depth of field.
		/// </summary>
		[ShowInInspector]
		[Range(0f, 10f, false)]
		[NativeWrapper]
		public float AdaptiveColorThreshold
		{
			get { return Internal_getadaptiveColorThreshold(mCachedPtr); }
			set { Internal_setadaptiveColorThreshold(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the minimum circle of confusion size before higher fidelity sampling is triggered. Small values trigger 
		/// high fidelity sampling because they can otherwise produce aliasing, and they are small enough so they don&apos;t cost 
		/// much. Increasing this value can improve performance as less higher fidelity samples will be required,  but may 
		/// decrease quality of the effect. Only relevant when using Bokeh depth of field.
		/// </summary>
		[ShowInInspector]
		[Range(0f, 1f, false)]
		[NativeWrapper]
		public float AdaptiveRadiusThreshold
		{
			get { return Internal_getadaptiveRadiusThreshold(mCachedPtr); }
			set { Internal_setadaptiveRadiusThreshold(mCachedPtr, value); }
		}

		/// <summary>Camera aperture size in mm. Only relevant when using Bokeh depth of field.</summary>
		[ShowInInspector]
		[Range(1f, 200f, false)]
		[NativeWrapper]
		public float ApertureSize
		{
			get { return Internal_getapertureSize(mCachedPtr); }
			set { Internal_setapertureSize(mCachedPtr, value); }
		}

		/// <summary>Focal length of the camera&apos;s lens (e.g. 75mm). Only relevant when using Bokeh depth of field.</summary>
		[ShowInInspector]
		[Range(1f, 500f, false)]
		[NativeWrapper]
		public float FocalLength
		{
			get { return Internal_getfocalLength(mCachedPtr); }
			set { Internal_setfocalLength(mCachedPtr, value); }
		}

		/// <summary>
		/// Camera sensor width and height, in mm. Used for controlling the size of the circle of confusion. Only relevant  when 
		/// using Bokeh depth of field.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector2 SensorSize
		{
			get
			{
				Vector2 temp;
				Internal_getsensorSize(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setsensorSize(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Set to true if Bokeh flare should use soft depth testing to ensure they don&apos;t render on top of foreground 
		/// geometry. This can help reduce background bleeding into the foreground, which can be especially noticeable if the 
		/// background is much brighter than the foreground. Use <see cref="occlusionDepthRange"/> to tweak the effect.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool BokehOcclusion
		{
			get { return Internal_getbokehOcclusion(mCachedPtr); }
			set { Internal_setbokehOcclusion(mCachedPtr, value); }
		}

		/// <summary>
		/// Range in world units over which range to fade out Bokeh flare influence. Influence of the flare will be faded out as 
		/// the depth difference between the flare&apos;s origin pixel and the destination pixel grows larger. Only relevant if 
		/// <see cref="bokehOcclusion"/> is turned on.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float OcclusionDepthRange
		{
			get { return Internal_getocclusionDepthRange(mCachedPtr); }
			set { Internal_setocclusionDepthRange(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_DepthOfFieldSettings(DepthOfFieldSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Texture> Internal_getbokehShape(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setbokehShape(IntPtr thisPtr, RRef<Texture> value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getenabled(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setenabled(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern DepthOfFieldType Internal_gettype(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_settype(IntPtr thisPtr, DepthOfFieldType value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getfocalDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfocalDistance(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getfocalRange(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfocalRange(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getnearTransitionRange(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setnearTransitionRange(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getfarTransitionRange(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfarTransitionRange(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getnearBlurAmount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setnearBlurAmount(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getfarBlurAmount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfarBlurAmount(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getmaxBokehSize(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setmaxBokehSize(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getadaptiveColorThreshold(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setadaptiveColorThreshold(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getadaptiveRadiusThreshold(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setadaptiveRadiusThreshold(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getapertureSize(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setapertureSize(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getfocalLength(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfocalLength(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getsensorSize(IntPtr thisPtr, out Vector2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setsensorSize(IntPtr thisPtr, ref Vector2 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getbokehOcclusion(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setbokehOcclusion(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getocclusionDepthRange(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setocclusionDepthRange(IntPtr thisPtr, float value);
	}
}
