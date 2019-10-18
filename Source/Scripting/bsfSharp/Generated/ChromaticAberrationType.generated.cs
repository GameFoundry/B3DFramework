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

	/// <summary>Types of available chromatic aberration effects.</summary>
	public enum ChromaticAberrationType
	{
		/// <summary>Simple chromatic aberration effect that is fast to execute.</summary>
		Simple = 0,
		/// <summary>
		/// More complex chromatic aberration effect that takes longer to execute but may yield more visually pleasing results 
		/// than the simple variant.
		/// </summary>
		Complex = 1
	}

	/** @} */
}
