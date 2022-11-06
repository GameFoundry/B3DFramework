//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
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
			value.CurveIdx = 0;
			value.KeyIdx = 0;

			return value;
		}

		public KeyframeRef(int curveIdx, int keyIdx)
		{
			this.CurveIdx = curveIdx;
			this.KeyIdx = keyIdx;
		}

		public int CurveIdx;
		public int KeyIdx;
	}

	/** @} */
}
