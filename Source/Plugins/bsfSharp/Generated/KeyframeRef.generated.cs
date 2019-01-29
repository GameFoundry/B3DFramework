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
	/** @addtogroup Animation
	 *  @{
	 */

	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct KeyframeRef
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static KeyframeRef Default()
		{
			KeyframeRef value = new KeyframeRef();
			value.curveIdx = 0;
			value.keyIdx = 0;

			return value;
		}

		public KeyframeRef(int curveIdx, int keyIdx)
		{
			this.curveIdx = curveIdx;
			this.keyIdx = keyIdx;
		}

		public int curveIdx;
		public int keyIdx;
	}

	/** @} */
}
