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

	/// <summary>
	/// The &quot;positive side&quot; of the plane is the half space to which the plane normal points. The &quot;negative 
	/// side&quot; is the other half space. The flag &quot;no side&quot; indicates the plane itself.
	/// </summary>
	public enum PlaneSide
	{
		None = 0,
		Both = 3,
		Negative = 2,
		Positive = 1
	}

	/** @} */
}
