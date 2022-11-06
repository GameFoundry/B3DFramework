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

	[ShowInInspector]
	public partial class Animation : Component
	{
		private Animation(bool __dummy0) { }
		protected Animation() { }

		/// <summary>
		/// Determines the default clip to play as soon as the component is enabled. If more control over playing clips is needed 
		/// use the play(), blend() and crossFade() methods to queue clips for playback manually, and setState() method for 
		/// modify their states individually.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public RRef<AnimationClip> DefaultClip
		{
			get { return Internal_GetDefaultClip(mCachedPtr); }
			set { Internal_SetDefaultClip(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public AnimWrapMode WrapMode
		{
			get { return Internal_GetWrapMode(mCachedPtr); }
			set { Internal_SetWrapMode(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float Speed
		{
			get { return Internal_GetSpeed(mCachedPtr); }
			set { Internal_SetSpeed(mCachedPtr, value); }
		}

		[NativeWrapper]
		public bool IsPlaying
		{
			get { return Internal_IsPlaying(mCachedPtr); }
		}

		/// <summary>
		/// Determines bounds that will be used for animation and mesh culling. Only relevant if setUseBounds() is set to true.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public AABox Bounds
		{
			get
			{
				AABox temp;
				Internal_GetBounds(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetBounds(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Determines should animation bounds be used for visibility determination (culling). If false the bounds of the mesh 
		/// attached to the relevant CRenderable component will be used instead.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool UseBounds
		{
			get { return Internal_GetUseBounds(mCachedPtr); }
			set { Internal_SetUseBounds(mCachedPtr, value); }
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

		public void Play(RRef<AnimationClip> clip)
		{
			Internal_Play(mCachedPtr, clip);
		}

		public void BlendAdditive(RRef<AnimationClip> clip, float weight, float fadeLength = 0f, int layer = 0)
		{
			Internal_BlendAdditive(mCachedPtr, clip, weight, fadeLength, layer);
		}

		public void Blend1D(Blend1DInfo info, float t)
		{
			Internal_Blend1D(mCachedPtr, ref info, t);
		}

		public void Blend2D(Blend2DInfo info, Vector2 t)
		{
			Internal_Blend2D(mCachedPtr, ref info, ref t);
		}

		public void CrossFade(RRef<AnimationClip> clip, float fadeLength)
		{
			Internal_CrossFade(mCachedPtr, clip, fadeLength);
		}

		public void Sample(RRef<AnimationClip> clip, float time)
		{
			Internal_Sample(mCachedPtr, clip, time);
		}

		public void Stop(int layer)
		{
			Internal_Stop(mCachedPtr, layer);
		}

		public void StopAll()
		{
			Internal_StopAll(mCachedPtr);
		}

		public bool GetState(RRef<AnimationClip> clip, out AnimationClipState state)
		{
			return Internal_GetState(mCachedPtr, clip, out state);
		}

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
		private static extern void Internal_SetWrapMode(IntPtr thisPtr, AnimWrapMode wrapMode);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AnimWrapMode Internal_GetWrapMode(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSpeed(IntPtr thisPtr, float speed);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetSpeed(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Play(IntPtr thisPtr, RRef<AnimationClip> clip);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_BlendAdditive(IntPtr thisPtr, RRef<AnimationClip> clip, float weight, float fadeLength, int layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Blend1D(IntPtr thisPtr, ref Blend1DInfo info, float t);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Blend2D(IntPtr thisPtr, ref Blend2DInfo info, ref Vector2 t);
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
		private static extern bool Internal_GetState(IntPtr thisPtr, RRef<AnimationClip> clip, out AnimationClipState state);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetState(IntPtr thisPtr, RRef<AnimationClip> clip, ref AnimationClipState state);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMorphChannelWeight(IntPtr thisPtr, string name, float weight);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetBounds(IntPtr thisPtr, ref AABox bounds);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetBounds(IntPtr thisPtr, out AABox __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetUseBounds(IntPtr thisPtr, bool enable);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetUseBounds(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEnableCull(IntPtr thisPtr, bool enable);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetEnableCull(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetNumClips(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<AnimationClip> Internal_GetClip(IntPtr thisPtr, int idx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RefreshClipMappingsInternal(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetGenericCurveValueInternal(IntPtr thisPtr, int curveIdx, out float value);
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
