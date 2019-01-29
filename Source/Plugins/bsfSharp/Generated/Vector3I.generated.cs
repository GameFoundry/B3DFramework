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
	/** @addtogroup Math
	 *  @{
	 */

	/// <summary>A three dimensional vector with integer coordinates.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct Vector3I
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static Vector3I Default()
		{
			Vector3I value = new Vector3I();
			value.x = 0;
			value.y = 0;
			value.z = 0;

			return value;
		}

		public Vector3I(int x, int y, int z)
		{
			this.x = x;
			this.y = y;
			this.z = z;
		}

		public int x;
		public int y;
		public int z;
	}

	/** @} */
}
