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

	/// <summary>A two dimensional vector.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct Vector2
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static Vector2 Default()
		{
			Vector2 value = new Vector2();
			value.X = 0;
			value.Y = 0;

			return value;
		}

		public Vector2(float x, float y)
		{
			this.X = x;
			this.Y = y;
		}

		public float X;
		public float Y;
	}

	/** @} */

	/** @addtogroup Math
	 *  @{
	 */

	/// <summary>A two dimensional vector.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct Vector2D
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static Vector2D Default()
		{
			Vector2D value = new Vector2D();
			value.X = 0;
			value.Y = 0;

			return value;
		}

		public Vector2D(double x, double y)
		{
			this.X = x;
			this.Y = y;
		}

		public double X;
		public double Y;
	}

	/** @} */

	/** @addtogroup Math
	 *  @{
	 */

	/// <summary>A two dimensional vector.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct Vector2I
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static Vector2I Default()
		{
			Vector2I value = new Vector2I();
			value.X = 0;
			value.Y = 0;

			return value;
		}

		public Vector2I(int x, int y)
		{
			this.X = x;
			this.Y = y;
		}

		public int X;
		public int Y;
	}

	/** @} */

	/** @addtogroup Math
	 *  @{
	 */

	/// <summary>A two dimensional vector.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct Vector2UI
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static Vector2UI Default()
		{
			Vector2UI value = new Vector2UI();
			value.X = 0;
			value.Y = 0;

			return value;
		}

		public Vector2UI(int x, int y)
		{
			this.X = x;
			this.Y = y;
		}

		public int X;
		public int Y;
	}

	/** @} */
}
