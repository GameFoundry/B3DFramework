//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/// <summary>A plane represented by a normal and a distance.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct Plane
	{
		public Vector3 Normal;
		public float D;
	}

	/** @} */

	/** @addtogroup Math
	 *  @{
	 */

	/// <summary>A plane represented by a normal and a distance.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct PlaneD
	{
		public Vector3d Normal;
		public double D;
	}

	/** @} */
}
