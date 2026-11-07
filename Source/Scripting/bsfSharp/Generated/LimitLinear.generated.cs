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

	/// <summary>Represents a joint limit between zero a single distance value.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct LimitLinear
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static LimitLinear Default()
		{
			LimitLinear value = new LimitLinear();
			value.Extent = 0f;
			value.Restitution = 0f;
			value.Spring = Spring.Default();

			return value;
		}

		/// <summary>
		/// Constructs a hard limit. Once the limit is reached the movement of the attached bodies will come to a stop.
		/// </summary>
		/// <param name="extent">Distance at which the limit becomes active.</param>
		public LimitLinear(float extent)
		{
			this.Extent = extent;
			this.Restitution = 0f;
			this.Spring = Spring.Default();
		}

		/// <summary>
		/// Constructs a soft limit. Once the limit is reached the bodies will bounce back according to the resitution parameter 
		/// and will be pulled back towards the limit by the provided spring.
		/// </summary>
		/// <param name="extent">Distance at which the limit becomes active.</param>
		/// <param name="spring">
		/// Spring that controls how are the bodies pulled back towards the limit when they breach it.
		/// </param>
		/// <param name="restitution">
		/// Controls how do objects react when the limit is reached, values closer to zero specify non-ellastic collision, while 
		/// those closer to one specify more ellastic (i.e bouncy) collision. Must be in [0, 1] range.
		/// </param>
		public LimitLinear(float extent, Spring spring, float restitution = 0f)
		{
			this.Extent = extent;
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

		/// <summary>Distance at which the limit becomes active.</summary>
		public float Extent;
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
