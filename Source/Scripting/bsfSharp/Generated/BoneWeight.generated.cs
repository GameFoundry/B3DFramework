//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
		public int Index0;
		public int Index1;
		public int Index2;
		public int Index3;
		public float Weight0;
		public float Weight1;
		public float Weight2;
		public float Weight3;
	}

	/** @} */
}
