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
	/** @addtogroup Physics
	 *  @{
	 */

	/// <summary>Specifies first or second body referenced by a Joint.</summary>
	public enum JointBody
	{
		/// <summary>Body the joint is influencing.</summary>
		Target = 0,
		/// <summary>Body the joint is attached to (if any).</summary>
		Anchor = 1
	}

	/** @} */
}
