//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	/// <summary>Contains common values used by all Joint limit types.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct LimitCommon
	{
		public LimitCommon(float contactDist = -1f)
		{
			this.ContactDist = contactDist;
			this.Restitution = 0f;
			this.Spring = Spring.Default();
		}

		public LimitCommon(Spring spring, float restitution = 0f)
		{
			this.ContactDist = -1f;
			this.Restitution = restitution;
			this.Spring = spring;
		}

		/// <summary>
		/// Distance from the limit at which it becomes active. Allows the solver to activate earlier than the limit is reached 
		/// to avoid breaking the limit.
		/// </summary>
		public float ContactDist;
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
