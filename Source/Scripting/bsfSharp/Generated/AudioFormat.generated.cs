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

	/// <summary>Valid internal engine audio formats.</summary>
	public enum AudioFormat
	{
		/// <summary>Pulse code modulation audio (&quot;raw&quot; uncompressed audio).</summary>
		PCM = 0,
		/// <summary>Vorbis compressed audio.</summary>
		VORBIS = 1
	}

	/** @} */
}
