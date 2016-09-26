//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @cond INTEROP */

    /// <summary>
    /// Wrapper around the native Animation class.
    /// <see cref="Animation"/>
    /// </summary>
    internal class NativeAnimation : ScriptObject
    {
        public AnimWrapMode WrapMode
        {
            set { Internal_SetWrapMode(mCachedPtr, value); }
        }

        public float Speed
        {
            set { Internal_SetSpeed(mCachedPtr, value); }
        }

        public int NumClips
        {
            get { return Internal_GetNumClips(mCachedPtr); }
        }

        public AABox Bounds
        {
            set { Internal_SetBounds(mCachedPtr, ref value); }
        }

        public bool Cull
        {
            set { Internal_SetCull(mCachedPtr, value); }
        }

        public Action<AnimationClip, string> OnEventTriggered;

        public void Play(AnimationClip clip)
        {
            IntPtr clipPtr = IntPtr.Zero;
            if (clip != null)
                clipPtr = clip.GetCachedPtr();

            Internal_Play(mCachedPtr, clipPtr);
        }

        public void BlendAdditive(AnimationClip clip, float weight, float fadeLength, int layer)
        {
            IntPtr clipPtr = IntPtr.Zero;
            if (clip != null)
                clipPtr = clip.GetCachedPtr();

            Internal_BlendAdditive(mCachedPtr, clipPtr, weight, fadeLength, layer);
        }

        public void Blend1D(Blend1DInfo info, float t)
        {
            if (info == null)
                return;

            Internal_Blend1D(mCachedPtr, info, t);
        }

        public void Blend2D(Blend2DInfo info, Vector2 t)
        {
            if (info == null)
                return;

            Internal_Blend2D(mCachedPtr, info, ref t);
        }

        public void CrossFade(AnimationClip clip, float fadeLength)
        {
            IntPtr clipPtr = IntPtr.Zero;
            if (clip != null)
                clipPtr = clip.GetCachedPtr();

            Internal_CrossFade(mCachedPtr, clipPtr, fadeLength);
        }

        public void Sample(AnimationClip clip, float time)
        {
            IntPtr clipPtr = IntPtr.Zero;
            if (clip != null)
                clipPtr = clip.GetCachedPtr();

            Internal_Sample(mCachedPtr, clipPtr, time);
        }

        public void Stop(int layer)
        {
            Internal_Stop(mCachedPtr, layer);
        }

        public void StopAll()
        {
            Internal_StopAll(mCachedPtr);
        }

        public bool IsPlaying()
        {
            return Internal_IsPlaying(mCachedPtr);
        }

        public AnimationClip GetClip(int index)
        {
            return Internal_GetClip(mCachedPtr, index);
        }

        public bool GetState(AnimationClip clip, out AnimationClipState state)
        {
            IntPtr clipPtr = IntPtr.Zero;
            if (clip != null)
                clipPtr = clip.GetCachedPtr();

            return Internal_GetState(mCachedPtr, clipPtr, out state);
        }

        public void SetState(AnimationClip clip, AnimationClipState state)
        {
            IntPtr clipPtr = IntPtr.Zero;
            if (clip != null)
                clipPtr = clip.GetCachedPtr();

            Internal_SetState(mCachedPtr, clipPtr, ref state);
        }

        public void SetMorphChannelWeight(int idx, float weight)
        {
            Internal_SetMorphChannelWeight(mCachedPtr, idx, weight);
        }

        public bool GetGenericCurveValue(int curveIdx, out float value)
        {
            return Internal_GetGenericCurveValue(mCachedPtr, curveIdx, out value);
        }

        public void MapCurveToSceneObject(string curve, SceneObject sceneObject)
        {
            if (curve == null || sceneObject == null)
                return;

            Internal_MapCurveToSceneObject(mCachedPtr, curve, sceneObject.GetCachedPtr());
        }

        public void UnmapSceneObject(SceneObject sceneObject)
        {
            if (sceneObject == null)
                return;

            Internal_UnmapSceneObject(mCachedPtr, sceneObject.GetCachedPtr());
        }

        internal NativeAnimation()
        {
            Internal_Create(this);
        }
        
        internal void Destroy()
        {
            Internal_Destroy(mCachedPtr);
        }

        private void Internal_OnEventTriggered(AnimationClip clip, string name)
        {
            OnEventTriggered?.Invoke(clip, name);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Create(NativeAnimation instance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Destroy(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetWrapMode(IntPtr thisPtr, AnimWrapMode wrapMode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetSpeed(IntPtr thisPtr, float speed);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Play(IntPtr thisPtr, IntPtr clipPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_BlendAdditive(IntPtr thisPtr, IntPtr clipPtr, float weight, float fadeLength,
            int layer);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Blend1D(IntPtr thisPtr, Blend1DInfo info, float t);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Blend2D(IntPtr thisPtr, Blend2DInfo info, ref Vector2 t);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CrossFade(IntPtr thisPtr, IntPtr clipPtr, float fadeLength);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Sample(IntPtr thisPtr, IntPtr clipPtr, float time);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Stop(IntPtr thisPtr, int layer);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_StopAll(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_IsPlaying(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetState(IntPtr thisPtr, IntPtr clipPtr, out AnimationClipState state);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetState(IntPtr thisPtr, IntPtr clipPtr, ref AnimationClipState state);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetMorphChannelWeight(IntPtr thisPtr, int idx, float weight);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetNumClips(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern AnimationClip Internal_GetClip(IntPtr thisPtr, int idx);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetGenericCurveValue(IntPtr thisPtr, int curveIdx, out float value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_MapCurveToSceneObject(IntPtr thisPtr, string curve, IntPtr sceneObjectPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_UnmapSceneObject(IntPtr thisPtr, IntPtr sceneObjectPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetBounds(IntPtr thisPtr, ref AABox bounds);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetCull(IntPtr thisPtr, bool cull);
    }

    /** @endcond */
}