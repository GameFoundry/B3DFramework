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

	/// <summary>Type of mesh indices used, used for determining maximum number of vertices in a mesh.</summary>
	public enum IndexType
	{
		/// <summary>16-bit indices.</summary>
		Index16 = 0,
		/// <summary>32-bit indices.</summary>
		Index32 = 1
	}

	/** @} */
}
