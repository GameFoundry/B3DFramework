//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>Type of parameters that can be defined by a shader.</summary>
	public enum ShaderParameterType
	{
		Matrix3 = 5,
		Float = 0,
		Matrix4 = 6,
		Color = 4,
		Vector2 = 1,
		Vector3 = 2,
		Vector4 = 3,
		Texture2D = 7,
		Texture3D = 8,
		TextureCube = 9,
		Sampler = 10
	}

	/** @} */
}
