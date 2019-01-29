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

	/// <summary>Flags that control options for the spherical joint</summary>
	public enum SphericalJointFlag
	{
		/// <summary>Enables the cone range limit.</summary>
		Limit = 1
	}

	/** @} */
}
