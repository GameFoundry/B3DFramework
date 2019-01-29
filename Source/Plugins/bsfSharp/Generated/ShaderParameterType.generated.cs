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

	/// <summary>Type of parameters that can be defined by a shader.</summary>
	public enum ShaderParameterType
	{
		Texture3D = 8,
		Float = 0,
		Vector2 = 1,
		Vector3 = 2,
		Vector4 = 3,
		Color = 4,
		Matrix3 = 5,
		Matrix4 = 6,
		Texture2D = 7,
		TextureCube = 9,
		Sampler = 10
	}

	/** @} */
}
