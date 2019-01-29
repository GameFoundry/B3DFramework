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
	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>
	/// Defines a 1D blend where multiple animation clips are blended between each other using linear interpolation.
	/// </summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct Blend1DInfo
	{
		public BlendClipInfo[] clips;
	}

	/** @} */
}
