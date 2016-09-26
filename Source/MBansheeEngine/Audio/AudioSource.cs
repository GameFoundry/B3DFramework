//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
namespace BansheeEngine
{
    /** @addtogroup Audio
     *  @{
     */

    /// <summary>
    /// Valid states in which AudioSource can be in.
    /// </summary>
    public enum AudioSourceState // Note: Must match C++ enum AudioSourceState
    {
        /// <summary>
        /// Source is currently playing.
        /// </summary>
        Playing,
        /// <summary>
        /// Source is currently paused (play will resume from paused point).
        /// </summary>
		Paused,
        /// <summary>
        /// Source is currently stopped (play will resume from start).
        /// </summary>
		Stopped
    }

    /// <summary>
    /// Represents a source for emitting audio. Audio can be played spatially (gun shot), or normally (music). Each audio
    /// source must have an AudioClip to play-back, and it can also have a position in the case of spatial(3D) audio.
    /// 
    /// Whether or not an audio source is spatial is controlled by the assigned AudioClip.The volume and the pitch of a
    /// spatial audio source is controlled by its position and the AudioListener's position/direction/velocity.
    /// </summary>
    public abstract class AudioSource : Component
    {
        internal NativeAudioSource native;

        [SerializeField]
        internal SerializableData serializableData = new SerializableData();

        private Vector3 lastPosition;
        private Vector3 velocity;

        /// <summary>
        /// Audio clip that will be played.
        /// </summary>
        public AudioClip Clip
        {
            get { return serializableData.audioClip; }
            set
            {
                if (serializableData.audioClip == value)
                    return;

                serializableData.audioClip = value;

                if (native != null)
                    native.Clip = value;
            }
        }

        /// <summary>
        /// Volume of the audio source, in [0, 1] range.
        /// </summary>
        public float Volume
        {
            get { return serializableData.volume; }
            set
            {
                if (serializableData.volume == value)
                    return;

                serializableData.volume = value;

                if (native != null)
                    native.Volume = value;
            }
        }

        /// <summary>
        /// Pitch of the audio source.
        /// </summary>
        public float Pitch
        {
            get { return serializableData.pitch; }
            set
            {
                if (serializableData.pitch == value)
                    return;

                serializableData.pitch = value;

                if (native != null)
                    native.Pitch = value;
            }
        }

        /// <summary>
        /// Determines whether the audio clip should loop when it finishes playing.
        /// </summary>
        public bool Loop
        {
            get { return serializableData.loop; }
            set
            {
                if (serializableData.loop == value)
                    return;

                serializableData.loop = value;

                if (native != null)
                    native.Loop = value;
            }
        }

        /// <summary>
        /// Sets the priority of the audio source. If more audio sources are playing than supported by the hardware, some
        /// might get disabled. By setting a higher priority the audio source is guaranteed to be disabled after sources
        /// with lower priority.
        /// </summary>
        public uint Priority
        {
            get { return serializableData.priority; }
            set
            {
                if (serializableData.priority == value)
                    return;

                serializableData.priority = value;

                if (native != null)
                    native.Priority = value;
            }
        }

        /// <summary>
        /// Minimum distance at which audio attenuation starts. When the listener is closer to the source than this value, 
        /// audio is heard at full volume. Once farther away the audio starts attenuating.
        /// </summary>
        public float MinDistance
        {
            get { return serializableData.minDistance; }
            set
            {
                if (serializableData.minDistance == value)
                    return;

                serializableData.minDistance = value;

                if (native != null)
                    native.MinDistance = value;
            }
        }

        /// <summary>
        /// Determines how quickly does audio volume drop off as the listener moves further from the source.
        /// </summary>
        public float Attenuation
        {
            get { return serializableData.attenuation; }
            set
            {
                if (serializableData.attenuation == value)
                    return;

                serializableData.attenuation = value;

                if (native != null)
                    native.Attenuation = value;
            }
        }

        /// <summary>
        /// Playback position in seconds.
        /// </summary>
        public float Time
        {
            get
            {
                if (native != null)
                    return native.Time;

                return 0.0f;
            }
            set
            {
                if (native != null)
                    native.Time = value;
            }
        }

        /// <summary>
        /// Determines if the playback of the audio clip should start as soon as the component is initialized.
        /// </summary>
        public bool PlayOnStart
        {
            get { return serializableData.playOnStart; }
            set { serializableData.playOnStart = value; }
        }

        /// <summary>
        /// Returns the current state of the audio playback (playing/paused/stopped).
        /// </summary>
        public AudioSourceState State
        {
            get
            {
                if (native != null)
                    return native.State;

                return AudioSourceState.Stopped;
            }
        }

        /// <summary>
        /// Starts playing the currently assigned audio clip.
        /// </summary>
        public void Play()
        {
            if (native != null)
                native.Play();
        }

        /// <summary>
        /// Pauses audio playback.
        /// </summary>
        public void Pause()
        {
            if (native != null)
                native.Pause();
        }

        /// <summary>
        /// Stops audio playback, rewinding it to the start.
        /// </summary>
        public void Stop()
        {
            if (native != null)
                native.Stop();
        }

        /// <summary>
        /// Updates the transform of the internal source representation from the transform of the component's scene
        /// object.
        /// </summary>
        protected void UpdateTransform()
        {
            native.Position = SceneObject.Position;
            native.Velocity = velocity;
        }

        private void OnInitialize()
        {
            NotifyFlags = TransformChangedFlags.Transform;
        }

        private void OnUpdate()
        {
            Vector3 worldPos = SceneObject.Position;
            velocity = (worldPos - lastPosition) / BansheeEngine.Time.FrameDelta;
            lastPosition = worldPos;
        }

        private void OnEnable()
        {
            RestoreNative();

            if (serializableData.playOnStart)
                Play();
        }

        private void OnDisable()
        {
            DestroyNative();
        }

        private void OnDestroy()
        {
            DestroyNative();
        }

        private void OnTransformChanged(TransformChangedFlags flags)
        {
            if (!SceneObject.Active)
                return;

            if ((flags & (TransformChangedFlags.Parent | TransformChangedFlags.Transform)) != 0)
                UpdateTransform();
        }

        /// <summary>
        /// Destroys the internal source representation.
        /// </summary>
        private void DestroyNative()
        {
            if (native != null)
            {
                native.Destroy();
                native = null;
            }
        }

        /// <summary>
        /// Creates the internal representation of the source and restores the values saved by the component.
        /// </summary>
        private void RestoreNative()
        {
            native = new NativeAudioSource();

            native.Clip = serializableData.audioClip;
            native.Volume = serializableData.volume;
            native.Pitch = serializableData.pitch;
            native.Loop = serializableData.loop;
            native.Priority = serializableData.priority;
            native.MinDistance = serializableData.minDistance;
            native.Attenuation = serializableData.attenuation;

            UpdateTransform();
        }

        /// <summary>
        /// Holds all data the listener component needs to persist through serialization.
        /// </summary>
        [SerializeObject]
        internal class SerializableData
        {
            public AudioClip audioClip;
            public float volume = 1.0f;
            public float pitch = 1.0f;
            public bool loop = false;
            public uint priority = 0;
            public float minDistance = 1.0f;
            public float attenuation = 1.0f;
            public bool playOnStart = true;
        }
    }

    /** @} */
}
