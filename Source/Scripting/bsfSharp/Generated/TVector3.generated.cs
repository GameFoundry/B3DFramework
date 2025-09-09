//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
	/** @addtogroup Math
	 *  @{
	 */

	/// <summary>A three dimensional vector.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct Vector3
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static Vector3 Default()
		{
			Vector3 value = new Vector3();
			value.X = 0;
			value.Y = 0;
			value.Z = 0;

			return value;
		}

		public Vector3(float value)
		{
			this.X = 0;
			this.Y = 0;
			this.Z = 0;
		}

		public Vector3(float x, float y, float z)
		{
			this.X = x;
			this.Y = y;
			this.Z = z;
		}

		public Vector3(Vector4 vec)
		{
			this.X = 0;
			this.Y = 0;
			this.Z = 0;
		}

		public float X;
		public float Y;
		public float Z;
	}

	/** @} */

	/** @addtogroup Math
	 *  @{
	 */

	/// <summary>A three dimensional vector.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct Vector3D
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static Vector3D Default()
		{
			Vector3D value = new Vector3D();
			value.X = 0;
			value.Y = 0;
			value.Z = 0;

			return value;
		}

		public Vector3D(double value)
		{
			this.X = 0;
			this.Y = 0;
			this.Z = 0;
		}

		public Vector3D(double x, double y, double z)
		{
			this.X = x;
			this.Y = y;
			this.Z = z;
		}

		public Vector3D(Vector4 vec)
		{
			this.X = 0;
			this.Y = 0;
			this.Z = 0;
		}

		public double X;
		public double Y;
		public double Z;
	}

	/** @} */
}
