//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Audio
	 *  @{
	 */

	[ShowInInspector]
	public partial class AudioSource : Component
	{
		private AudioSource(bool __dummy0) { }
		protected AudioSource() { }

		[ShowInInspector]
		[NativeWrapper]
		public RRef<AudioClip> Clip
		{
			get { return Internal_GetClip(mCachedPtr); }
			set { Internal_SetClip(mCachedPtr, value); }
		}

		[ShowInInspector]
		[Range(0f, 1f, true)]
		[NativeWrapper]
		public float Volume
		{
			get { return Internal_GetVolume(mCachedPtr); }
			set { Internal_SetVolume(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float Pitch
		{
			get { return Internal_GetPitch(mCachedPtr); }
			set { Internal_SetPitch(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public bool Loop
		{
			get { return Internal_GetIsLooping(mCachedPtr); }
			set { Internal_SetIsLooping(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public int Priority
		{
			get { return Internal_GetPriority(mCachedPtr); }
			set { Internal_SetPriority(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float MinDistance
		{
			get { return Internal_GetMinDistance(mCachedPtr); }
			set { Internal_SetMinDistance(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float Attenuation
		{
			get { return Internal_GetAttenuation(mCachedPtr); }
			set { Internal_SetAttenuation(mCachedPtr, value); }
		}

		[NativeWrapper]
		public float Time
		{
			get { return Internal_GetTime(mCachedPtr); }
			set { Internal_SetTime(mCachedPtr, value); }
		}

		/// <summary>Sets whether playback should start as soon as the component is enabled.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool PlayOnStart
		{
			get { return Internal_GetPlayOnStart(mCachedPtr); }
			set { Internal_SetPlayOnStart(mCachedPtr, value); }
		}

		[NativeWrapper]
		public AudioSourceState State
		{
			get { return Internal_GetState(mCachedPtr); }
		}

		public void Play()
		{
			Internal_Play(mCachedPtr);
		}

		public void Pause()
		{
			Internal_Pause(mCachedPtr);
		}

		public void Stop()
		{
			Internal_Stop(mCachedPtr);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetClip(IntPtr thisPtr, RRef<AudioClip> clip);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<AudioClip> Internal_GetClip(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetVolume(IntPtr thisPtr, float volume);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetVolume(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetPitch(IntPtr thisPtr, float pitch);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetPitch(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetIsLooping(IntPtr thisPtr, bool loop);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetIsLooping(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetPriority(IntPtr thisPtr, int priority);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetPriority(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMinDistance(IntPtr thisPtr, float distance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetMinDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetAttenuation(IntPtr thisPtr, float attenuation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetAttenuation(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTime(IntPtr thisPtr, float time);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetTime(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetPlayOnStart(IntPtr thisPtr, bool enable);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetPlayOnStart(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Play(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Pause(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Stop(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AudioSourceState Internal_GetState(IntPtr thisPtr);
	}

	/** @} */
}
