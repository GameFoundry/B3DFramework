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
	/** @addtogroup Audio
	 *  @{
	 */

	/// <summary>Valid internal engine audio formats.</summary>
	public enum AudioFormat
	{
		/// <summary>Pulse code modulation audio ("raw" uncompressed audio).</summary>
		PCM = 0,
		/// <summary>Vorbis compressed audio.</summary>
		VORBIS = 1
	}

	/** @} */
}
