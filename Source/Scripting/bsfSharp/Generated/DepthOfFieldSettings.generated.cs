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

		/// <summary>Enables or disables the depth of field effect.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Enabled
		{
			get { return Internal_getenabled(mCachedPtr); }
			set { Internal_setenabled(mCachedPtr, value); }
		}

		/// <summary>
		/// Distance from the camera at which the focal plane is located in. Objects at this distance will be fully in focus.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FocalDistance
		{
			get { return Internal_getfocalDistance(mCachedPtr); }
			set { Internal_setfocalDistance(mCachedPtr, value); }
		}

		/// <summary>
		/// Range within which the objects remain fully in focus. This range is applied relative to the focal distance.  Only 
		/// relevant if Gaussian depth of field is used as other methods don&apos;t use a constant in-focus range.
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
		/// Only relevant for Gaussian depth of field.
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
		/// Only relevant for Gaussian depth of field.
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

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_DepthOfFieldSettings(DepthOfFieldSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getenabled(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setenabled(IntPtr thisPtr, bool value);
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
	}
}
