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

	/// <summary>Type of force that can be applied to a rigidbody at an arbitrary point.</summary>
	public enum PointForceMode
	{
		/// <summary>Value applied is a force.</summary>
		Force = 0,
		/// <summary>Value applied is an impulse (a direct change in its linear or angular momentum).</summary>
		Impulse = 1
	}

	/** @} */
}
