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
	/// Defines a 2D blend where two animation clips are blended between each other using bilinear interpolation.
	/// </summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct Blend2DInfo
	{
		public RRef<AnimationClip> topLeftClip;
		public RRef<AnimationClip> topRightClip;
		public RRef<AnimationClip> botLeftClip;
		public RRef<AnimationClip> botRightClip;
	}

	/** @} */
}
