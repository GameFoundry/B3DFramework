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

	/// <summary>
	/// Controls climbing behaviour for a capsule character controller. Normally the character controller will not 
	/// automatically climb when heights are greater than the assigned step offset. However due to the shape of the capsule it 
	/// might automatically climb over slightly larger heights than assigned step offsets.
	/// </summary>
	public enum CharacterClimbingMode
	{
		/// <summary>
		/// Normal behaviour. Capsule character controller will be able to auto-step even above the step offset.
		/// </summary>
		Normal = 0,
		/// <summary>The system will attempt to limit auto-step to the provided step offset and no higher.</summary>
		Constrained = 1
	}

	/** @} */
}
