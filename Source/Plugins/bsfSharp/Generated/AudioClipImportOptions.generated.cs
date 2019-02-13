//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
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
			Internal_create(this);
		}

		/// <summary>Audio format to import the audio clip as.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public AudioFormat Format
		{
			get { return Internal_getformat(mCachedPtr); }
			set { Internal_setformat(mCachedPtr, value); }
		}

		/// <summary>Determines how is audio data loaded into memory.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public AudioReadMode ReadMode
		{
			get { return Internal_getreadMode(mCachedPtr); }
			set { Internal_setreadMode(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the clip be played as spatial (3D) audio or as normal audio. 3D clips will be converted to mono on 
		/// import.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Is3D
		{
			get { return Internal_getis3D(mCachedPtr); }
			set { Internal_setis3D(mCachedPtr, value); }
		}

		/// <summary>Size of a single sample in bits. The clip will be converted to this bit depth on import.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public int BitDepth
		{
			get { return Internal_getbitDepth(mCachedPtr); }
			set { Internal_setbitDepth(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AudioFormat Internal_getformat(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setformat(IntPtr thisPtr, AudioFormat value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AudioReadMode Internal_getreadMode(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setreadMode(IntPtr thisPtr, AudioReadMode value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getis3D(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setis3D(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getbitDepth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setbitDepth(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(AudioClipImportOptions managedInstance);
	}

	/** @} */
#endif
}
