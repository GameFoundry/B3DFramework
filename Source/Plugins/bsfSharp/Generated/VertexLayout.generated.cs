//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
		PU = 33,
		Position = 1,
		Color = 2,
		Normal = 4,
		UV1 = 64,
		UV0 = 32,
		BoneWeights = 16,
		Tangent = 8,
		PC = 3,
		PCU = 35,
		PCN = 7,
		PCNU = 39,
		PCNT = 15,
		PCNTU = 47,
		PN = 5,
		PNU = 37,
		PNT = 13,
		PNTU = 45
	}

	/** @} */
}
