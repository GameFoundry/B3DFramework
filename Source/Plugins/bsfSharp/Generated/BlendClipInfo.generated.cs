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

	/// <summary>Represents an animation clip used in 1D blending. Each clip has a position on the number line.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct BlendClipInfo
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static BlendClipInfo Default()
		{
			BlendClipInfo value = new BlendClipInfo();
			value.clip = null;
			value.position = 0f;

			return value;
		}

		public RRef<AnimationClip> clip;
		public float position;
	}

	/** @} */
}
