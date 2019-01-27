//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	/// <summary>Flags that control options of a Rigidbody object.</summary>
	public enum RigidbodyFlag
	{
		/// <summary>No options.</summary>
		None = 0,
		/// <summary>
		/// Automatically calculate center of mass transform and inertia tensors from child shapes (colliders).
		/// </summary>
		AutoTensors = 1,
		/// <summary>Calculate mass distribution from child shapes (colliders). Only relevant when auto-tensors is on.</summary>
		AutoMass = 2,
		/// <summary>
		/// Enables continous collision detection. This can prevent fast moving bodies from tunneling through each other. This 
		/// must also be enabled globally in Physics otherwise the flag will be ignored.
		/// </summary>
		CCD = 4
	}

	/** @} */
}
