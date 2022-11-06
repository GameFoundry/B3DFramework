//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
#if !IS_B3D
	/** @addtogroup Importer
	 *  @{
	 */

	/// <summary>Contains import options you may use to control how an audio clip is imported.</summary>
	[ShowInInspector]
	public partial class AudioClipImportOptions : ImportOptions
	{
		private AudioClipImportOptions(bool __dummy0) { }

		/// <summary>Creates a new import options object that allows you to customize how are audio clips imported.</summary>
		public AudioClipImportOptions()
		{
			Internal_Create(this);
		}

		/// <summary>Audio format to import the audio clip as.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public AudioFormat Format
		{
			get { return Internal_GetFormat(mCachedPtr); }
			set { Internal_SetFormat(mCachedPtr, value); }
		}

		/// <summary>Determines how is audio data loaded into memory.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public AudioReadMode ReadMode
		{
			get { return Internal_GetReadMode(mCachedPtr); }
			set { Internal_SetReadMode(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the clip be played as spatial (3D) audio or as normal audio. 3D clips will be converted to mono on 
		/// import.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Is3D
		{
			get { return Internal_GetIs3D(mCachedPtr); }
			set { Internal_SetIs3D(mCachedPtr, value); }
		}

		/// <summary>Size of a single sample in bits. The clip will be converted to this bit depth on import.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public int BitDepth
		{
			get { return Internal_GetBitDepth(mCachedPtr); }
			set { Internal_SetBitDepth(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AudioFormat Internal_GetFormat(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFormat(IntPtr thisPtr, AudioFormat value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AudioReadMode Internal_GetReadMode(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetReadMode(IntPtr thisPtr, AudioReadMode value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetIs3D(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetIs3D(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetBitDepth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetBitDepth(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(AudioClipImportOptions managedInstance);
	}

	/** @} */
#endif
}
