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
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>Information about a single probe in the light probe volume.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct LightProbeInfo
	{
		public int handle;
		public Vector3 position;
	}

	/** @} */
}
