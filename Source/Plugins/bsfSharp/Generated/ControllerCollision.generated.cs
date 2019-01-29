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

	/// <summary>Contains data about a collision of a character controller and another object.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ControllerCollision
	{
		/// <summary>Contact position.</summary>
		public Vector3 position;
		/// <summary>Contact normal.</summary>
		public Vector3 normal;
		/// <summary>Direction of motion after the hit.</summary>
		public Vector3 motionDir;
		/// <summary>Magnitude of motion after the hit.</summary>
		public float motionAmount;
	}

	/** @} */
}
