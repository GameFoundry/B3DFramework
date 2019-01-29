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

	/// <summary>Specifies type of constraint placed on a specific axis.</summary>
	public enum D6JointMotion
	{
		/// <summary>Axis is immovable.</summary>
		Locked = 0,
		/// <summary>Axis will be constrained by the specified limits.</summary>
		Limited = 1,
		/// <summary>Axis will not be constrained.</summary>
		Free = 2,
		Count = 3
	}

	/** @} */
}
