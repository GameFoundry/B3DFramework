//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/// <summary>
	/// Contains a number value and an associated unit. Used primarily to prevent implicit conversion between numbers of 
	/// different units.
	/// </summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct TUnitValue<T, Unit>
	{
		public T Value;
	}

	/** @} */

}
