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

	/// <summary>Contains common values used by all Joint limit types.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct LimitCommon
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static LimitCommon Default()
		{
			LimitCommon value = new LimitCommon();
			value.Restitution = 0f;
			value.Spring = Spring.Default();

			return value;
		}

		public LimitCommon(Spring spring, float restitution = 0f)
		{
			this.Restitution = restitution;
			this.Spring = spring;
		}

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
