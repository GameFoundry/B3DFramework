//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
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
			this.contactDist = contactDist;
			this.restitution = 0f;
			this.spring = new Spring();
		}

		public LimitCommon(Spring spring, float restitution = 0f)
		{
			this.contactDist = -1f;
			this.restitution = restitution;
			this.spring = spring;
		}

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
