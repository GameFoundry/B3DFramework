//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Particles
	 *  @{
	 */

	/// <summary>Determines type of distribution used by distribution properties.</summary>
	public enum PropertyDistributionType
	{
		/// <summary>The distribution is a costant value.</summary>
		Constant = 0,
		/// <summary>The distribution is a random value in a specified constant range.</summary>
		RandomRange = 1,
		/// <summary>The distribution is a time-varying value.</summary>
		Curve = 2,
		/// <summary>The distribution is a random value in a specified time-varying range.</summary>
		RandomCurveRange = 3
	}

	/** @} */
}
