//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup Audio
     *  @{
     */

    /// <summary>
    /// Valid internal engine audio formats.
    /// </summary>
    public enum AudioFormat // Note: Must match C++ enum AudioFormat
    {
        /// <summary>Pulse code modulation audio ("raw" uncompressed audio).</summary>
        PCM,
        /// <summary>Vorbis compressed audio.</summary>
	    VORBIS
    }

    /// <summary>
    /// Modes that determine how and when is audio data read.
    /// </summary>
    public enum AudioReadMode // Note: Must match C++ enum AudioReadMode
    {
        /// <summary>
        /// Entire audio clip will be loaded and decompressed. Uses most memory but has lowest CPU impact.
        /// </summary>
        LoadDecompressed, 
        /// <summary>
        /// Entire audio clip will be loaded, but will be decompressed while playing. Uses medium amount of memory and has
        /// the highest CPU impact. 
        /// </summary>
		LoadCompressed,
        /// <summary>
        /// Audio will be slowly streamed from the disk, and decompressed if needed. Uses very little memory, and has either
        /// low or high CPU impact depending if the audio is in a compressed format. Since data is streamed from the disk,
        /// read speeds could also be a bottleneck.
        /// </summary>
		Stream
    }

    /// <summary>
    /// Numbers of bits per audio sample.
    /// </summary>
    public enum AudioBitDepth
    {
        Bits8 = 8,
        Bits16 = 16,
        Bits24 = 24,
        Bits32 = 32
    }

    /// <summary>
    /// Audio clip stores audio data in a compressed or uncompressed format. Clips can be provided to audio sources or
    /// other audio methods to be played.
    /// </summary>
    public class AudioClip : Resource
    {
        /// <summary>
        /// Constructor for internal use by the runtime.
        /// </summary>
        private AudioClip()
        { }

        /// <summary>
        /// Returns the size of a single sample, in bits.
        /// </summary>
        public int BitDepth
        {
            get { return Internal_GetBitDepth(mCachedPtr); }
        }

        /// <summary>
        /// Returns how many samples per second is the audio encoded in.
        /// </summary>
        public int SampleRate
        {
            get { return Internal_GetSampleRate(mCachedPtr); }
        }

        /// <summary>
        /// Returns the number of channels provided by the clip.
        /// </summary>
        public int NumChannels
        {
            get { return Internal_GetNumChannels(mCachedPtr); }
        }

        /// <summary>
        /// Returns in which format is audio data stored in. 
        /// </summary>
        public AudioFormat Format
        {
            get { return Internal_GetAudioFormat(mCachedPtr); }
        }

        /// <summary>
        /// Returns how is the audio data read/decoded. 
        /// </summary>
        public AudioReadMode ReadMode
        {
            get { return Internal_GetReadMode(mCachedPtr); }
        }

        /// <summary>
        /// Returns the total number of samples in the clip (includes all channels).
        /// </summary>
        public int NumSamples
        {
            get { return Internal_GetNumSamples(mCachedPtr); }
        }

        /// <summary>
        /// Returns the length of the audio clip, in seconds.
        /// </summary>
        public float Duration
        {
            get { return Internal_GetDuration(mCachedPtr); }
        }

        /// <summary>
        /// Determines will the clip be played a spatial 3D sound, or as a normal sound (for example music).
        /// </summary>
        public bool Is3D
        {
            get { return Internal_GetIs3D(mCachedPtr); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetBitDepth(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetSampleRate(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetNumChannels(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern AudioFormat Internal_GetAudioFormat(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern AudioReadMode Internal_GetReadMode(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetNumSamples(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetDuration(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetIs3D(IntPtr thisPtr);
    }

    /** @} */
}