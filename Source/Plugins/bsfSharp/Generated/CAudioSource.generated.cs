//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Audio
	 *  @{
	 */

	/// <summary>
	/// Represents a source for emitting audio. Audio can be played spatially (gun shot), or normally (music). Each audio 
	/// source must have an AudioClip to play-back, and it can also have a position in the case of spatial (3D) audio.
	///
	/// Whether or not an audio source is spatial is controlled by the assigned AudioClip. The volume and the pitch of a 
	/// spatial audio source is controlled by its position and the AudioListener's position/direction/velocity.
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
			get { return Internal_getClip(mCachedPtr); }
			set { Internal_setClip(mCachedPtr, value); }
		}

		/// <summary>Volume of the audio played from this source, in [0, 1] range.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Volume
		{
			get { return Internal_getVolume(mCachedPtr); }
			set { Internal_setVolume(mCachedPtr, value); }
		}

		/// <summary>Determines the pitch of the played audio. 1 is the default.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Pitch
		{
			get { return Internal_getPitch(mCachedPtr); }
			set { Internal_setPitch(mCachedPtr, value); }
		}

		/// <summary>Determines whether the audio clip should loop when it finishes playing.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Loop
		{
			get { return Internal_getIsLooping(mCachedPtr); }
			set { Internal_setIsLooping(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the priority of the audio source. If more audio sources are playing than supported by the hardware,  some 
		/// might get disabled. By setting a higher priority the audio source is guaranteed to be disabled after sources with 
		/// lower priority.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public uint Priority
		{
			get { return Internal_getPriority(mCachedPtr); }
			set { Internal_setPriority(mCachedPtr, value); }
		}

		/// <summary>
		/// Minimum distance at which audio attenuation starts. When the listener is closer to the source than this value, audio 
		/// is heard at full volume. Once farther away the audio starts attenuating.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float MinDistance
		{
			get { return Internal_getMinDistance(mCachedPtr); }
			set { Internal_setMinDistance(mCachedPtr, value); }
		}

		/// <summary>
		/// Attenuation that controls how quickly does audio volume drop off as the listener moves further from the source.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Attenuation
		{
			get { return Internal_getAttenuation(mCachedPtr); }
			set { Internal_setAttenuation(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the current time of playback. If playback hasn't yet started, it specifies the time at which playback will 
		/// start at. The time is in seconds, in range [0, clipLength].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Time
		{
			get { return Internal_getTime(mCachedPtr); }
			set { Internal_setTime(mCachedPtr, value); }
		}

		/// <summary>Sets whether playback should start as soon as the component is enabled.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool PlayOnStart
		{
			get { return Internal_getPlayOnStart(mCachedPtr); }
			set { Internal_setPlayOnStart(mCachedPtr, value); }
		}

		/// <summary>Returns the current state of the audio playback (playing/paused/stopped).</summary>
		[ShowInInspector]
		[NativeWrapper]
		public AudioSourceState State
		{
			get { return Internal_getState(mCachedPtr); }
		}

		/// <summary>Starts playing the currently assigned audio clip.</summary>
		public void Play()
		{
			Internal_play(mCachedPtr);
		}

		/// <summary>Pauses the audio playback.</summary>
		public void Pause()
		{
			Internal_pause(mCachedPtr);
		}

		/// <summary>Stops audio playback, rewinding it to the start.</summary>
		public void Stop()
		{
			Internal_stop(mCachedPtr);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setClip(IntPtr thisPtr, RRef<AudioClip> clip);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<AudioClip> Internal_getClip(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setVolume(IntPtr thisPtr, float volume);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getVolume(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setPitch(IntPtr thisPtr, float pitch);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getPitch(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setIsLooping(IntPtr thisPtr, bool loop);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getIsLooping(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setPriority(IntPtr thisPtr, uint priority);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern uint Internal_getPriority(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setMinDistance(IntPtr thisPtr, float distance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getMinDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setAttenuation(IntPtr thisPtr, float attenuation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getAttenuation(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setTime(IntPtr thisPtr, float time);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getTime(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setPlayOnStart(IntPtr thisPtr, bool enable);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getPlayOnStart(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_play(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_pause(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_stop(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AudioSourceState Internal_getState(IntPtr thisPtr);
	}

	/** @} */
}
