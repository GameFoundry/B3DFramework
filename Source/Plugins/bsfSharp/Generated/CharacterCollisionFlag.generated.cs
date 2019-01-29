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
	/** @addtogroup Physics
	 *  @{
	 */

	/// <summary>Reports in which directions is the character colliding with other objects.</summary>
	public enum CharacterCollisionFlag
	{
		/// <summary>Character is colliding with its sides.</summary>
		Sides = 1,
		/// <summary>Character is colliding with the ceiling.</summary>
		Up = 2,
		/// <summary>Character is colliding with the ground.</summary>
		Down = 4
	}

	/** @} */
}
