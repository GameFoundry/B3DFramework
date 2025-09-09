//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
	/** @addtogroup Audio
	 *  @{
	 */

	/// <summary>
	/// Represents a source for emitting audio. Audio can be played spatially (gun shot), or normally (music). Each audio 
	/// source must have an AudioClip to play-back, and it can also have a position in the case of spatial (3D) audio.
	///
	/// Whether or not an audio source is spatial is controlled by the assigned AudioClip. The volume and the pitch of a 
	/// spatial audio source is controlled by its position and the AudioListener&apos;s position/direction/velocity.
	/// </summary>
	[ShowInInspector]
	public partial class AudioSource : Component
	{
		private AudioSource(bool __dummy0) { }
		protected AudioSource() { }

		/// <summary>Audio clip to play.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public RRef<AudioClip> Clip
		{
			get { return Internal_GetClip(mCachedPtr); }
			set { Internal_SetClip(mCachedPtr, value); }
		}

		/// <summary>Volume of the audio played from this source, in [0, 1] range.</summary>
		[ShowInInspector]
		[Range(0f, 1f, true)]
		[NativeWrapper]
		public float Volume
		{
			get { return Internal_GetVolume(mCachedPtr); }
			set { Internal_SetVolume(mCachedPtr, value); }
		}

		/// <summary>Determines the pitch of the played audio. 1 is the default.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Pitch
		{
			get { return Internal_GetPitch(mCachedPtr); }
			set { Internal_SetPitch(mCachedPtr, value); }
		}

		/// <summary>Determines whether the audio clip should loop when it finishes playing.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Loop
		{
			get { return Internal_GetIsLooping(mCachedPtr); }
			set { Internal_SetIsLooping(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the priority of the audio source. If more audio sources are playing than supported by the hardware, some 
		/// might get disabled. By setting a higher priority the audio source is guaranteed to be disabled after sources with 
		/// lower priority.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int Priority
		{
			get { return Internal_GetPriority(mCachedPtr); }
			set { Internal_SetPriority(mCachedPtr, value); }
		}

		/// <summary>
		/// Minimum distance at which audio attenuation starts. When the listener is closer to the source than this value, audio 
		/// is heard at full volume. Once farther away the audio starts attenuating.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float MinDistance
		{
			get { return Internal_GetMinDistance(mCachedPtr); }
			set { Internal_SetMinDistance(mCachedPtr, value); }
		}

		/// <summary>
		/// Attenuation that controls how quickly does audio volume drop off as the listener moves further from the source.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Attenuation
		{
			get { return Internal_GetAttenuation(mCachedPtr); }
			set { Internal_SetAttenuation(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the current time of playback. If playback hasn&apos;t yet started, it specifies the time at which playback 
		/// will start at. The time is in seconds, in range [0, clipLength].
		/// </summary>
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

		/// <summary>Returns the current state of the audio playback (playing/paused/stopped).</summary>
		[NativeWrapper]
		public AudioSourceState State
		{
			get { return Internal_GetState(mCachedPtr); }
		}

		/// <summary>Starts playing the currently assigned audio clip.</summary>
		public void Play()
		{
			Internal_Play(mCachedPtr);
		}

		/// <summary>Pauses the audio playback.</summary>
		public void Pause()
		{
			Internal_Pause(mCachedPtr);
		}

		/// <summary>Stops audio playback, rewinding it to the start.</summary>
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
