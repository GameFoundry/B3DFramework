//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>
	/// Contains animation curves for translation/rotation/scale of scene objects/skeleton bones, as well as curves for 
	/// generic property animation.
	/// </summary>
	[ShowInInspector]
	public partial class AnimationClip : Resource
	{
		private AnimationClip(bool __dummy0, bool __dummy1) { }
		protected AnimationClip() { }

		/// <summary>
		/// Creates an animation clip with no curves. After creation make sure to register some animation curves before using it.
		/// </summary>
		public AnimationClip(bool isAdditive = false)
		{
			Internal_Create(this, isAdditive);
		}

		/// <summary>Creates an animation clip with specified curves.</summary>
		/// <param name="curves">Curves to initialize the animation with.</param>
		/// <param name="isAdditive">
		/// Determines does the clip contain additive curve data. This will change the behaviour how is the clip blended with 
		/// other animations.
		/// </param>
		/// <param name="sampleRate">
		/// If animation uses evenly spaced keyframes, number of samples per second. Not relevant if keyframes are unevenly 
		/// spaced.
		/// </param>
		/// <param name="rootMotion">
		/// Optional set of curves that can be used for animating the root bone. Not used by the animation system directly but is 
		/// instead provided to the user for manual evaluation.
		/// </param>
		public AnimationClip(AnimationCurves curves, bool isAdditive = false, int sampleRate = 1, RootMotion rootMotion = null)
		{
			Internal_Create0(this, curves, isAdditive, sampleRate, rootMotion);
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public RRef<AnimationClip> Ref
		{
			get { return Internal_GetRef(mCachedPtr); }
		}

		/// <summary>
		/// A set of all curves stored in the animation. Returned value will not be updated if the animation clip curves are 
		/// added or removed, as it is a copy of clip&apos;s internal values.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public AnimationCurves Curves
		{
			get { return Internal_GetCurves(mCachedPtr); }
			set { Internal_SetCurves(mCachedPtr, value); }
		}

		/// <summary>A set of all events to be triggered as the animation is playing.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public AnimationEvent[] Events
		{
			get { return Internal_GetEvents(mCachedPtr); }
			set { Internal_SetEvents(mCachedPtr, value); }
		}

		/// <summary>
		/// Returns a set of curves containing motion of the root bone. This allows the user to evaluate the root bone animation 
		/// curves manually, instead of through the normal animation process. This property is only available if animation clip 
		/// was imported with root motion import enabled.
		/// </summary>
		[NativeWrapper]
		public RootMotion RootMotion
		{
			get { return Internal_GetRootMotion(mCachedPtr); }
		}

		/// <summary>Checks if animation clip has root motion curves separate from the normal animation curves.</summary>
		[NativeWrapper]
		public bool HasRootMotion
		{
			get { return Internal_HasRootMotion(mCachedPtr); }
		}

		/// <summary>
		/// Checks are the curves contained within the clip additive. Additive clips are intended to be added on top of other 
		/// clips.
		/// </summary>
		[NativeWrapper]
		public bool IsAddtive
		{
			get { return Internal_IsAdditive(mCachedPtr); }
		}

		/// <summary>Returns the length of the animation clip, in seconds.</summary>
		[NativeWrapper]
		public float Length
		{
			get { return Internal_GetLength(mCachedPtr); }
		}

		/// <summary>
		/// Number of samples per second the animation clip curves were sampled at. This value is not used by the animation clip 
		/// or curves directly since unevenly spaced keyframes are supported. But it can be of value when determining the 
		/// original sample rate of an imported animation or similar.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int SampleRate
		{
			get { return Internal_GetSampleRate(mCachedPtr); }
			set { Internal_SetSampleRate(mCachedPtr, value); }
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public static implicit operator RRef<AnimationClip>(AnimationClip x)
		{
			if(x != null)
				return Internal_GetRef(x.mCachedPtr);
			else
				return null;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<AnimationClip> Internal_GetRef(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AnimationCurves Internal_GetCurves(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCurves(IntPtr thisPtr, AnimationCurves curves);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AnimationEvent[] Internal_GetEvents(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEvents(IntPtr thisPtr, AnimationEvent[] events);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RootMotion Internal_GetRootMotion(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_HasRootMotion(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsAdditive(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetLength(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetSampleRate(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSampleRate(IntPtr thisPtr, int sampleRate);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(AnimationClip managedInstance, bool isAdditive);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(AnimationClip managedInstance, AnimationCurves curves, bool isAdditive, int sampleRate, RootMotion rootMotion);
	}

	/** @} */
}
