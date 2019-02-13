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
#if !IS_B3D
	/** @addtogroup Importer
	 *  @{
	 */

	/// <summary>Controls what type of collision mesh should be imported during mesh import.</summary>
	public enum CollisionMeshType
	{
		/// <summary>No collision mesh will be imported.</summary>
		None = 0,
		/// <summary>Normal triangle mesh will be imported.</summary>
		Normal = 1,
		/// <summary>A convex hull will be generated from the source mesh.</summary>
		Convex = 2
	}

	/** @} */
#endif
}
