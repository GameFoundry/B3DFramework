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

	/// <summary>Type of playback to use for an animation of a SpriteTexture.</summary>
	public enum SpriteAnimationPlayback
	{
		/// <summary>Do not animate.</summary>
		None = 0,
		/// <summary>Animate once until the end of the animation is reached.</summary>
		Normal = 1,
		/// <summary>Animate to the end of the animation then loop around.</summary>
		Loop = 2,
		/// <summary>Loop the animation but reverse playback when the end is reached.</summary>
		PingPong = 3
	}

	/** @} */
}
