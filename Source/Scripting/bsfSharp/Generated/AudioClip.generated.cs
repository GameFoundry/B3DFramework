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

	/// <summary>
	/// Audio clip stores audio data in a compressed or uncompressed format. Clips can be provided to audio sources or other 
	/// audio methods to be played.
	/// </summary>
	[ShowInInspector]
	public partial class AudioClip : Resource
	{
		private AudioClip(bool __dummy0) { }
		protected AudioClip() { }

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public RRef<AudioClip> Ref
		{
			get { return Internal_GetRef(mCachedPtr); }
		}

		/// <summary>Returns the size of a single sample, in bits.</summary>
		[NativeWrapper]
		public int BitDepth
		{
			get { return Internal_GetBitDepth(mCachedPtr); }
		}

		/// <summary>Returns how many samples per second is the audio encoded in.</summary>
		[NativeWrapper]
		public int SampleRate
		{
			get { return Internal_GetFrequency(mCachedPtr); }
		}

		/// <summary>Returns the number of channels provided by the clip.</summary>
		[NativeWrapper]
		public int NumChannels
		{
			get { return Internal_GetNumChannels(mCachedPtr); }
		}

		/// <summary>Returns in which format is audio data stored in.</summary>
		[NativeWrapper]
		public AudioFormat Format
		{
			get { return Internal_GetFormat(mCachedPtr); }
		}

		/// <summary>Returns how is the audio data read/decoded.</summary>
		[NativeWrapper]
		public AudioReadMode ReadMode
		{
			get { return Internal_GetReadMode(mCachedPtr); }
		}

		/// <summary>Returns the length of the audio clip, in seconds.</summary>
		[NativeWrapper]
		public float Duration
		{
			get { return Internal_GetLength(mCachedPtr); }
		}

		/// <summary>Returns the total number of samples in the clip (includes all channels).</summary>
		[NativeWrapper]
		public int NumSamples
		{
			get { return Internal_GetNumSamples(mCachedPtr); }
		}

		/// <summary>Determines will the clip be played a spatial 3D sound, or as a normal sound (for example music).</summary>
		[NativeWrapper]
		public bool Is3D
		{
			get { return Internal_Is3D(mCachedPtr); }
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public static implicit operator RRef<AudioClip>(AudioClip x)
		{
			if(x != null)
				return Internal_GetRef(x.mCachedPtr);
			else
				return null;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<AudioClip> Internal_GetRef(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetBitDepth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetFrequency(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetNumChannels(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AudioFormat Internal_GetFormat(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AudioReadMode Internal_GetReadMode(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetLength(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetNumSamples(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_Is3D(IntPtr thisPtr);
	}

	/** @} */
}
