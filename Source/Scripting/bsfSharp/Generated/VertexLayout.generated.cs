//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>
	/// Available vertex layouts that specify what data is provided per-vertex in a mesh. Combinations other than those 
	/// provided are allowed.
	/// </summary>
	public enum VertexLayout
	{
		Tangent = 8,
		PNU = 37,
		Normal = 4,
		Position = 1,
		BoneWeights = 16,
		Color = 2,
		UV0 = 32,
		UV1 = 64,
		PC = 3,
		PU = 33,
		PCU = 35,
		PCN = 7,
		PCNU = 39,
		PCNT = 15,
		PCNTU = 47,
		PN = 5,
		PNT = 13,
		PNTU = 45
	}

	/** @} */
}
