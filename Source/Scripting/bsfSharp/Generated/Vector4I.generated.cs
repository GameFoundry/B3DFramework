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

	/// <summary>A four dimensional vector with integer coordinates.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct Vector4I
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static Vector4I Default()
		{
			Vector4I value = new Vector4I();
			value.X = 0;
			value.Y = 0;
			value.Z = 0;
			value.W = 0;

			return value;
		}

		public Vector4I(int x, int y, int z, int w)
		{
			this.X = x;
			this.Y = y;
			this.Z = z;
			this.W = w;
		}

		public int X;
		public int Y;
		public int Z;
		public int W;
	}

	/** @} */
}
