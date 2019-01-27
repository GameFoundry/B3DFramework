//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Utility
	 *  @{
	 */

	/// <summary>Contains per-vertex bone weights and indexes used for skinning, for up to four bones.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct BoneWeight
	{
		public int index0;
		public int index1;
		public int index2;
		public int index3;
		public float weight0;
		public float weight1;
		public float weight2;
		public float weight3;
	}

	/** @} */
}
