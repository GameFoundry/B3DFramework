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
	/** @addtogroup Audio
	 *  @{
	 */

	/// <summary>Identifier for a device that can be used for playing audio.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct AudioDevice
	{
		public string name;
	}

	/** @} */
}
