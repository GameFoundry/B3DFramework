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

	/// <summary>Represents a quaternion used for 3D rotations.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct Quaternion
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static Quaternion Default()
		{
			Quaternion value = new Quaternion();
			value.X = 0;
			value.Y = 0;
			value.Z = 0;
			value.W = 1;

			return value;
		}

		public Quaternion(float w, float x, float y, float z)
		{
			this.X = x;
			this.Y = y;
			this.Z = z;
			this.W = w;
		}

		public float X;
		public float Y;
		public float Z;
		public float W;
	}

	/** @} */

	/** @addtogroup Math
	 *  @{
	 */

	/// <summary>Represents a quaternion used for 3D rotations.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct QuaternionD
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static QuaternionD Default()
		{
			QuaternionD value = new QuaternionD();
			value.X = 0;
			value.Y = 0;
			value.Z = 0;
			value.W = 0;

			return value;
		}

		public QuaternionD(double w, double x, double y, double z)
		{
			this.X = x;
			this.Y = y;
			this.Z = z;
			this.W = w;
		}

		public double X;
		public double Y;
		public double Z;
		public double W;
	}

	/** @} */
}
