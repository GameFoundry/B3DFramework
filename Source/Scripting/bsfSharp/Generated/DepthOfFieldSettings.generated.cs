//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
			get { return Internal_GetBokehShape(mCachedPtr); }
			set { Internal_SetBokehShape(mCachedPtr, value); }
		}

		/// <summary>Enables or disables the depth of field effect.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Enabled
		{
			get { return Internal_GetEnabled(mCachedPtr); }
			set { Internal_SetEnabled(mCachedPtr, value); }
		}

		/// <summary>Type of depth of field effect to use.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public DepthOfFieldType Type
		{
			get { return Internal_GetType(mCachedPtr); }
			set { Internal_SetType(mCachedPtr, value); }
		}

		/// <summary>
		/// Distance from the camera at which the focal plane is located in. Objects at this distance will be fully in focus. In 
		/// world units (meters).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FocalDistance
		{
			get { return Internal_GetFocalDistance(mCachedPtr); }
			set { Internal_SetFocalDistance(mCachedPtr, value); }
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
			get { return Internal_GetFocalRange(mCachedPtr); }
			set { Internal_SetFocalRange(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the size of the range within which objects transition from focused to fully unfocused, at the near plane. 
		/// Only relevant for Gaussian and Bokeh depth of field. In world units (meters).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float NearTransitionRange
		{
			get { return Internal_GetNearTransitionRange(mCachedPtr); }
			set { Internal_SetNearTransitionRange(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the size of the range within which objects transition from focused to fully unfocused, at the far plane. 
		/// Only relevant for Gaussian and Bokeh depth of field. In world units (meters).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FarTransitionRange
		{
			get { return Internal_GetFarTransitionRange(mCachedPtr); }
			set { Internal_SetFarTransitionRange(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the amount of blur to apply to fully unfocused objects that are closer to camera than the in-focus zone. 
		/// Set to zero to disable near-field blur. Only relevant for Gaussian depth of field.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float NearBlurAmount
		{
			get { return Internal_GetNearBlurAmount(mCachedPtr); }
			set { Internal_SetNearBlurAmount(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the amount of blur to apply to fully unfocused objects that are farther away from camera than the in-focus 
		/// zone. Set to zero to disable far-field blur. Only relevant for Gaussian depth of field.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FarBlurAmount
		{
			get { return Internal_GetFarBlurAmount(mCachedPtr); }
			set { Internal_SetFarBlurAmount(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the maximum size of the blur kernel, in percent of view size. Larger values cost more performance. Only 
		/// relevant when using Bokeh depth of field.
		/// </summary>
		[ShowInInspector]
		[Range(0f, 1f, false)]
		[NativeWrapper]
		public float MaxBokehSize
		{
			get { return Internal_GetMaxBokehSize(mCachedPtr); }
			set { Internal_SetMaxBokehSize(mCachedPtr, value); }
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
			get { return Internal_GetAdaptiveColorThreshold(mCachedPtr); }
			set { Internal_SetAdaptiveColorThreshold(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the minimum circle of confusion size before higher fidelity sampling is triggered. Small values trigger 
		/// high fidelity sampling because they can otherwise produce aliasing, and they are small enough so they don&apos;t cost 
		/// much. Increasing this value can improve performance as less higher fidelity samples will be required, but may 
		/// decrease quality of the effect. Only relevant when using Bokeh depth of field.
		/// </summary>
		[ShowInInspector]
		[Range(0f, 1f, false)]
		[NativeWrapper]
		public float AdaptiveRadiusThreshold
		{
			get { return Internal_GetAdaptiveRadiusThreshold(mCachedPtr); }
			set { Internal_SetAdaptiveRadiusThreshold(mCachedPtr, value); }
		}

		/// <summary>Camera aperture size in mm. Only relevant when using Bokeh depth of field.</summary>
		[ShowInInspector]
		[Range(1f, 200f, false)]
		[NativeWrapper]
		public float ApertureSize
		{
			get { return Internal_GetApertureSize(mCachedPtr); }
			set { Internal_SetApertureSize(mCachedPtr, value); }
		}

		/// <summary>Focal length of the camera&apos;s lens (e.g. 75mm). Only relevant when using Bokeh depth of field.</summary>
		[ShowInInspector]
		[Range(1f, 500f, false)]
		[NativeWrapper]
		public float FocalLength
		{
			get { return Internal_GetFocalLength(mCachedPtr); }
			set { Internal_SetFocalLength(mCachedPtr, value); }
		}

		/// <summary>
		/// Camera sensor width and height, in mm. Used for controlling the size of the circle of confusion. Only relevant when 
		/// using Bokeh depth of field.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector2 SensorSize
		{
			get
			{
				Vector2 temp;
				Internal_GetSensorSize(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetSensorSize(mCachedPtr, ref value); }
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
			get { return Internal_GetBokehOcclusion(mCachedPtr); }
			set { Internal_SetBokehOcclusion(mCachedPtr, value); }
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
			get { return Internal_GetOcclusionDepthRange(mCachedPtr); }
			set { Internal_SetOcclusionDepthRange(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_DepthOfFieldSettings(DepthOfFieldSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Texture> Internal_GetBokehShape(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetBokehShape(IntPtr thisPtr, RRef<Texture> value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetEnabled(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEnabled(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern DepthOfFieldType Internal_GetType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetType(IntPtr thisPtr, DepthOfFieldType value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFocalDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFocalDistance(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFocalRange(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFocalRange(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetNearTransitionRange(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetNearTransitionRange(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFarTransitionRange(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFarTransitionRange(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetNearBlurAmount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetNearBlurAmount(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFarBlurAmount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFarBlurAmount(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetMaxBokehSize(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMaxBokehSize(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetAdaptiveColorThreshold(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetAdaptiveColorThreshold(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetAdaptiveRadiusThreshold(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetAdaptiveRadiusThreshold(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetApertureSize(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetApertureSize(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFocalLength(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFocalLength(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSensorSize(IntPtr thisPtr, out Vector2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSensorSize(IntPtr thisPtr, ref Vector2 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetBokehOcclusion(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetBokehOcclusion(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetOcclusionDepthRange(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetOcclusionDepthRange(IntPtr thisPtr, float value);
	}
}
