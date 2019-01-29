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
	/// <summary>Valid types of a mesh used for physics.</summary>
	public enum PhysicsMeshType
	{
		/// <summary>
		/// A regular triangle mesh. Mesh can be of arbitrary size but cannot be used for triggers and non-kinematic  objects. 
		/// Incurrs a significantly larger performance impact than convex meshes.
		/// </summary>
		Triangle = 0,
		/// <summary>
		/// Mesh representing a convex shape. Mesh will not have more than 256 vertices. Incurrs a significantly lower 
		/// performance impact than triangle meshes.
		/// </summary>
		Convex = 1
	}
}
