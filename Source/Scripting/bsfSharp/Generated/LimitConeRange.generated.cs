//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
	/** @addtogroup Physics
	 *  @{
	 */

	/// <summary>Represents a joint limit that contraints movement to within an elliptical cone.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct LimitConeRange
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static LimitConeRange Default()
		{
			LimitConeRange value = new LimitConeRange();
			value.YLimitAngle = new Radian(1.57079637f);
			value.ZLimitAngle = new Radian(1.57079637f);
			value.Restitution = 0f;
			value.Spring = Spring.Default();

			return value;
		}

		/// <summary>
		/// Constructs a hard limit. Once the limit is reached the movement of the attached bodies will come to a stop.
		/// </summary>
		/// <param name="yLimitAngle">
		/// Y angle of the cone. Movement is constrainted between 0 and this angle on the Y axis.
		/// </param>
		/// <param name="zLimitAngle">
		/// Z angle of the cone. Movement is constrainted between 0 and this angle on the Z axis.
		/// </param>
		public LimitConeRange(Radian yLimitAngle, Radian zLimitAngle)
		{
			this.YLimitAngle = yLimitAngle;
			this.ZLimitAngle = zLimitAngle;
			this.Restitution = 0f;
			this.Spring = Spring.Default();
		}

		/// <summary>
		/// Constructs a soft limit. Once the limit is reached the bodies will bounce back according to the resitution parameter 
		/// and will be pulled back towards the limit by the provided spring.
		/// </summary>
		/// <param name="yLimitAngle">
		/// Y angle of the cone. Movement is constrainted between 0 and this angle on the Y axis.
		/// </param>
		/// <param name="zLimitAngle">
		/// Z angle of the cone. Movement is constrainted between 0 and this angle on the Z axis.
		/// </param>
		/// <param name="spring">
		/// Spring that controls how are the bodies pulled back towards the limit when they breach it.
		/// </param>
		/// <param name="restitution">
		/// Controls how do objects react when the limit is reached, values closer to zero specify non-ellastic collision, while 
		/// those closer to one specify more ellastic (i.e bouncy) collision. Must be in [0, 1] range.
		/// </param>
		public LimitConeRange(Radian yLimitAngle, Radian zLimitAngle, Spring spring, float restitution = 0f)
		{
			this.YLimitAngle = yLimitAngle;
			this.ZLimitAngle = zLimitAngle;
			this.Restitution = 0f;
			this.Spring = Spring.Default();
		}

		///<summary>
		/// Returns a subset of this struct. This subset usually contains common fields shared with another struct.
		///</summary>
		public LimitCommon GetBase()
		{
			LimitCommon value;
			value.Restitution = Restitution;
			value.Spring = Spring;
			return value;
		}

		///<summary>
		/// Assigns values to a subset of fields of this struct. This subset usually contains common field shared with 
		/// another struct.
		///</summary>
		public void SetBase(LimitCommon value)
		{
			Restitution = value.Restitution;
			Spring = value.Spring;
		}

		/// <summary>Y angle of the cone. Movement is constrainted between 0 and this angle on the Y axis.</summary>
		[Range(0f, 180f, false)]
		public Radian YLimitAngle;
		/// <summary>Z angle of the cone. Movement is constrainted between 0 and this angle on the Z axis.</summary>
		[Range(0f, 180f, false)]
		public Radian ZLimitAngle;
		/// <summary>
		/// Controls how do objects react when the limit is reached, values closer to zero specify non-ellastic collision, while 
		/// those closer to one specify more ellastic (i.e bouncy) collision. Must be in [0, 1] range.
		/// </summary>
		public float Restitution;
		/// <summary>Spring that controls how are the bodies pulled back towards the limit when they breach it.</summary>
		public Spring Spring;
	}

	/** @} */
}
