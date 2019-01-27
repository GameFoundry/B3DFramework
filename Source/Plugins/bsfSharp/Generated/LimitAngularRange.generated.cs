//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	/// <summary>Represents a joint limit between two angles.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct LimitAngularRange
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static LimitAngularRange Default()
		{
			LimitAngularRange value = new LimitAngularRange();
			value.lower = new Radian();
			value.upper = new Radian();
			value.contactDist = -1f;
			value.restitution = 0f;
			value.spring = new Spring();

			return value;
		}

		/// <summary>
		/// Constructs a hard limit. Once the limit is reached the movement of the attached bodies will come to a stop.
		/// </summary>
		/// <param name="lower">Lower angle of the limit. Must be less than <paramref name="upper"/>.</param>
		/// <param name="upper">Upper angle of the limit. Must be more than <paramref name="lower"/>.</param>
		/// <param name="contactDist">
		/// Distance from the limit at which it becomes active. Allows the solver to activate earlier than the limit is reached 
		/// to avoid breaking the limit. Specify -1 for the default.
		/// </param>
		public LimitAngularRange(Radian lower, Radian upper, float contactDist = -1f)
		{
			this.lower = lower;
			this.upper = upper;
			this.contactDist = -1f;
			this.restitution = 0f;
			this.spring = new Spring();
		}

		/// <summary>
		/// Constructs a soft limit. Once the limit is reached the bodies will bounce back according to the resitution parameter 
		/// and will be pulled back towards the limit by the provided spring.
		/// </summary>
		/// <param name="lower">Lower angle of the limit. Must be less than <paramref name="upper"/>.</param>
		/// <param name="upper">Upper angle of the limit. Must be more than <paramref name="lower"/>.</param>
		/// <param name="spring">
		/// Spring that controls how are the bodies pulled back towards the limit when they breach it.
		/// </param>
		/// <param name="restitution">
		/// Controls how do objects react when the limit is reached, values closer to zero specify non-ellastic collision, while 
		/// those closer to one specify more ellastic (i.e bouncy) collision. Must be in [0, 1] range.
		/// </param>
		public LimitAngularRange(Radian lower, Radian upper, Spring spring, float restitution = 0f)
		{
			this.lower = lower;
			this.upper = upper;
			this.contactDist = -1f;
			this.restitution = 0f;
			this.spring = new Spring();
		}

		///<summary>
		/// Returns a subset of this struct. This subset usually contains common fields shared with another struct.
		///</summary>
		public LimitCommon GetBase()
		{
			LimitCommon value;
			value.contactDist = contactDist;
			value.restitution = restitution;
			value.spring = spring;
			return value;
		}

		///<summary>
		/// Assigns values to a subset of fields of this struct. This subset usually contains common field shared with 
		/// another struct.
		///</summary>
		public void SetBase(LimitCommon value)
		{
			contactDist = value.contactDist;
			restitution = value.restitution;
			spring = value.spring;
		}

		/// <summary>Lower angle of the limit. Must be less than #upper.</summary>
		public Radian lower;
		/// <summary>Upper angle of the limit. Must be less than #lower.</summary>
		public Radian upper;
		/// <summary>
		/// Distance from the limit at which it becomes active. Allows the solver to activate earlier than the limit is reached 
		/// to avoid breaking the limit.
		/// </summary>
		public float contactDist;
		/// <summary>
		/// Controls how do objects react when the limit is reached, values closer to zero specify non-ellastic collision, while 
		/// those closer to one specify more ellastic (i.e bouncy) collision. Must be in [0, 1] range.
		/// </summary>
		public float restitution;
		/// <summary>Spring that controls how are the bodies pulled back towards the limit when they breach it.</summary>
		public Spring spring;
	}

	/** @} */
}
