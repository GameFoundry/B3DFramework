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

	/// <summary>Encapsulates width/height in a single structure.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct TSize2<T>
	{
		public T Width;
		public T Height;
	}

	/** @} */

}
