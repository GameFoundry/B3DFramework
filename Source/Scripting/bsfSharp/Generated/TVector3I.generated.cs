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

	/// <summary>A three dimensional vector with integer coordinates.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct Vector3I
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static Vector3I Default()
		{
			Vector3I value = new Vector3I();
			value.X = 0;
			value.Y = 0;
			value.Z = 0;

			return value;
		}

		public Vector3I(int x, int y, int z)
		{
			this.X = x;
			this.Y = y;
			this.Z = z;
		}

		public Vector3I(int value)
		{
			this.X = 0;
			this.Y = 0;
			this.Z = 0;
		}

		public int X;
		public int Y;
		public int Z;
	}

	/** @} */

	/** @addtogroup Math
	 *  @{
	 */

	/// <summary>A three dimensional vector with integer coordinates.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct Vector3UI
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static Vector3UI Default()
		{
			Vector3UI value = new Vector3UI();
			value.X = 0;
			value.Y = 0;
			value.Z = 0;

			return value;
		}

		public Vector3UI(int x, int y, int z)
		{
			this.X = 0;
			this.Y = 0;
			this.Z = 0;
		}

		public Vector3UI(int value)
		{
			this.X = 0;
			this.Y = 0;
			this.Z = 0;
		}

		public int X;
		public int Y;
		public int Z;
	}

	/** @} */
}
