//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>
	/// Handles animation playback. Takes one or multiple animation clips as input and evaluates them every animation update 
	/// tick depending on set properties. The evaluated data is used by the render thread for skeletal animation, by the main 
	/// thread for updating attached scene objects and bones (if skeleton is attached), or the data is made available for 
	/// manual queries in the case of generic animation.
	/// </summary>
	[ShowInInspector]
	public partial class Animation : Component
	{
		private Animation(bool __dummy0) { }
		protected Animation() { }

		/// <summary>
		/// Determines the default clip to play as soon as the component is enabled. If more control over playing clips is needed 
		/// use the Play(), Blend*(), CrossFade() methods to queue clips for playback manually, and SetState() method to modify 
		/// their states individually.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public RRef<AnimationClip> DefaultClip
		{
			get { return Internal_GetDefaultClip(mCachedPtr); }
			set { Internal_SetDefaultClip(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the wrap mode for all active animations. Wrap mode determines what happens when animation reaches the 
		/// first or last frame.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public AnimationWrapMode WrapMode
		{
			get { return Internal_GetWrapMode(mCachedPtr); }
			set { Internal_SetWrapMode(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the speed for all animations. The default value is 1.0f. Use negative values to play-back in reverse.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Speed
		{
			get { return Internal_GetSpeed(mCachedPtr); }
			set { Internal_SetSpeed(mCachedPtr, value); }
		}

		/// <summary>Checks if any animation clips are currently playing.</summary>
		[NativeWrapper]
		public bool IsPlaying
		{
			get { return Internal_IsPlaying(mCachedPtr); }
		}

		/// <summary>
		/// Determines bounds that will be used for animation and mesh culling. Only relevant if SetUseCustomBounds() is set to 
		/// true.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public AABox CustomBounds
		{
			get
			{
				AABox temp;
				Internal_GetCustomBounds(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetCustomBounds(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Determines should animation bounds be used for visibility determination (culling). If false the bounds of the mesh 
		/// attached to the relevant Renderable component will be used instead.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool UseCustomBounds
		{
			get { return Internal_GetUseCustomBounds(mCachedPtr); }
			set { Internal_SetUseCustomBounds(mCachedPtr, value); }
		}

		/// <summary>
		/// Enables or disables culling of the animation when out of view. Culled animation will not be evaluated.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Cull
		{
			get { return Internal_GetEnableCull(mCachedPtr); }
			set { Internal_SetEnableCull(mCachedPtr, value); }
		}

		/// <summary>
		/// Triggered when the list of properties animated via generic animation curves needs to be recreated (script only).
		/// </summary>
		partial void Callback_RebuildFloatProperties(RRef<AnimationClip> p0);

		/// <summary>
		/// Triggered when generic animation curves values need be applied to the properties they effect (script only).
		/// </summary>
		partial void Callback__UpdateFloatProperties();

		/// <summary>Triggers a callback in script code when animation event is triggered (script only).</summary>
		partial void Callback_EventTriggered(RRef<AnimationClip> p0, string p1);

		/// <summary>Plays the specified animation clip.</summary>
		public void Play(RRef<AnimationClip> clip)
		{
			Internal_Play(mCachedPtr, clip);
		}

		/// <summary>
		/// Plays the specified animation clip on top of the animation currently playing in the main layer. Multiple such clips 
		/// can be playing at once, as long as you ensure each is given its own layer. Each animation can also have a weight that 
		/// determines how much it influences the main animation.
		/// </summary>
		/// <param name="clip">Clip to additively blend. Must contain additive animation curves.</param>
		/// <param name="weight">
		/// Determines how much of an effect will the blended animation have on the final output. In range [0, 1].
		/// </param>
		/// <param name="fadeLength">
		/// Applies the blend over a specified time period, increasing the weight as the time passes. Set to zero to blend 
		/// immediately. In seconds.
		/// </param>
		/// <param name="layer">
		/// Layer to play the clip in. Multiple additive clips can be playing at once in separate layers and each layer has its 
		/// own weight.
		/// </param>
		public void BlendAdditive(RRef<AnimationClip> clip, float weight, float fadeLength = 0f, int layer = 0)
		{
			Internal_BlendAdditive(mCachedPtr, clip, weight, fadeLength, layer);
		}

		/// <summary>
		/// Blend multiple animation clips between each other using linear interpolation. Unlike normal animations these 
		/// animations are not advanced with the progress of time, and is instead expected the user manually changes the 
		/// <paramref name="alpha"/> parameter.
		/// </summary>
		/// <param name="info">
		/// Information about the clips to blend. Clip positions must be sorted from lowest to highest.
		/// </param>
		/// <param name="alpha">
		/// Parameter that controls the blending. Range depends on the positions of the provided animation clips.
		/// </param>
		public void Blend1D(Blend1DInfo info, float alpha)
		{
			Internal_Blend1D(mCachedPtr, ref info, alpha);
		}

		/// <summary>
		/// Blend four animation clips between each other using bilinear interpolation. Unlike normal animations these animations 
		/// are not advanced with the progress of time, and is instead expected the user manually changes the <paramref 
		/// name="alpha"/> parameter.
		/// </summary>
		/// <param name="info">Information about the clips to blend.</param>
		/// <param name="alpha">
		/// Parameter that controls the blending, in range [(0, 0), (1, 1)]. alpha = (0, 0) means top left animation has full 
		/// influence, alpha = (1, 0) means top right animation has full influence, alpha = (0, 1) means bottom left animation 
		/// has full influence, alpha = (1, 1) means bottom right animation has full influence.
		/// </param>
		public void Blend2D(Blend2DInfo info, TVector2<float> alpha)
		{
			Internal_Blend2D(mCachedPtr, ref info, ref alpha);
		}

		/// <summary>
		/// Fades the specified animation clip in, while fading other playing animation out, over the specified time period.
		/// </summary>
		/// <param name="clip">Clip to fade in.</param>
		/// <param name="fadeLength">Determines the time period over which the fade occurs. In seconds.</param>
		public void CrossFade(RRef<AnimationClip> clip, float fadeLength)
		{
			Internal_CrossFade(mCachedPtr, clip, fadeLength);
		}

		/// <summary>
		/// Samples an animation clip at the specified time, displaying only that particular frame without further playback.
		/// </summary>
		/// <param name="clip">Animation clip to sample.</param>
		/// <param name="time">Time to sample the clip at.</param>
		public void Sample(RRef<AnimationClip> clip, float time)
		{
			Internal_Sample(mCachedPtr, clip, time);
		}

		/// <summary>
		/// Stops playing all animations on the provided layer. Specify ~0u to stop animation on the main layer (non-additive 
		/// animations).
		/// </summary>
		public void Stop(int layer)
		{
			Internal_Stop(mCachedPtr, layer);
		}

		/// <summary>Stops playing all animations.</summary>
		public void StopAll()
		{
			Internal_StopAll(mCachedPtr);
		}

		/// <summary>Retrieves detailed information about a currently playing animation clip.</summary>
		/// <param name="clip">Clip to retrieve the information for.</param>
		/// <param name="outState">
		/// Animation clip state containing the requested information. Only valid if the method returns true.
		/// </param>
		/// <returns>True if the state was found (animation clip is playing), false otherwise.</returns>
		public bool GetState(RRef<AnimationClip> clip, out AnimationClipState outState)
		{
			return Internal_GetState(mCachedPtr, clip, out outState);
		}

		/// <summary>
		/// Changes the state of a playing animation clip. If animation clip is not currently playing the playback is started for 
		/// the clip.
		/// </summary>
		/// <param name="clip">Clip to change the state for.</param>
		/// <param name="state">New state of the animation (e.g. changing the time for seeking).</param>
		public void SetState(RRef<AnimationClip> clip, AnimationClipState state)
		{
			Internal_SetState(mCachedPtr, clip, ref state);
		}

		/// <summary>
		/// Changes a weight of a single morph channel, determining how much of it to apply on top of the base mesh.
		/// </summary>
		/// <param name="name">
		/// Name of the morph channel to modify. This depends on the mesh the animation is currently animating.
		/// </param>
		/// <param name="weight">Weight that determines how much of the channel to apply to the mesh, in range [0, 1].</param>
		public void SetMorphChannelWeight(string name, float weight)
		{
			Internal_SetMorphChannelWeight(mCachedPtr, name, weight);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetDefaultClip(IntPtr thisPtr, RRef<AnimationClip> clip);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<AnimationClip> Internal_GetDefaultClip(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetWrapMode(IntPtr thisPtr, AnimationWrapMode wrapMode);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AnimationWrapMode Internal_GetWrapMode(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSpeed(IntPtr thisPtr, float speed);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetSpeed(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Play(IntPtr thisPtr, RRef<AnimationClip> clip);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_BlendAdditive(IntPtr thisPtr, RRef<AnimationClip> clip, float weight, float fadeLength, int layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Blend1D(IntPtr thisPtr, ref Blend1DInfo info, float alpha);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Blend2D(IntPtr thisPtr, ref Blend2DInfo info, ref TVector2<float> alpha);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_CrossFade(IntPtr thisPtr, RRef<AnimationClip> clip, float fadeLength);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Sample(IntPtr thisPtr, RRef<AnimationClip> clip, float time);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Stop(IntPtr thisPtr, int layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_StopAll(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsPlaying(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetState(IntPtr thisPtr, RRef<AnimationClip> clip, out AnimationClipState outState);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetState(IntPtr thisPtr, RRef<AnimationClip> clip, ref AnimationClipState state);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMorphChannelWeight(IntPtr thisPtr, string name, float weight);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCustomBounds(IntPtr thisPtr, ref AABox bounds);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetCustomBounds(IntPtr thisPtr, out AABox __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetUseCustomBounds(IntPtr thisPtr, bool enable);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetUseCustomBounds(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEnableCull(IntPtr thisPtr, bool enable);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetEnableCull(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetClipCount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<AnimationClip> Internal_GetClip(IntPtr thisPtr, int index);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RefreshClipMappingsInternal(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetGenericCurveValueInternal(IntPtr thisPtr, int curveIndex, out float outValue);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_TogglePreviewModeInternal(IntPtr thisPtr, bool enabled);
		private void Internal_ScriptRebuildFloatPropertiesInternal(RRef<AnimationClip> p0)
		{
			Callback_RebuildFloatProperties(p0);
		}
		private void Internal_ScriptUpdateFloatPropertiesInternal()
		{
			Callback__UpdateFloatProperties();
		}
		private void Internal_ScriptOnEventTriggeredInternal(RRef<AnimationClip> p0, string p1)
		{
			Callback_EventTriggered(p0, p1);
		}
	}

	/** @} */
}
