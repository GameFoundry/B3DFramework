//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "Material/BsShader.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	/** Type of parameters that can be defined by a shader. */
	enum class BS_SCRIPT_EXPORT(m:Rendering) ShaderParameterType
	{
		Float, Vector2, Vector3, Vector4, Color,
		Matrix3, Matrix4, Texture2D,
		Texture3D, TextureCube, Sampler
	};

	/** Flags used to further describe a shader parameter. */
	enum class BS_SCRIPT_EXPORT(m:Rendering) ShaderParameterFlag
	{
		None = 0,

		/** Parameter is for internal use by the renderer and isn't expected to be set by the user. */
		Internal = 1 << 0,

		/** Parameter should not be displayed in the editor inspector. */
		HideInInspector = 1 << 1,

		/** Allows the color parameter to be edited using the HDR color picker. */
		HDR = 1 << 2
	};

	using ShaderParameterFlags = Flags<ShaderParameterFlag>;
	BS_FLAGS_OPERATORS(ShaderParameterFlag)

	/** Contains information about a single shader parameter. */
	struct BS_SCRIPT_EXPORT(m:Rendering,pl:true) ShaderParameter
	{
		/** Name of the parameter variable. */
		String name;

		/** Variable identifier of the parameter. */
		String identifier;

		/** Data type of the parameter. */
		ShaderParameterType type;

		/** Flags used to further describe the parameter. */
		ShaderParameterFlags flags;
	};

	/** Extension class for Shader, for adding additional functionality for the script version of the class. */
	class BS_SCRIPT_EXPORT(e:Shader) ShaderEx
	{
	public:
		/** Returns information about all parameters available in the shader. */
		BS_SCRIPT_EXPORT(e:Shader,pr:getter,n:Parameters)
		static Vector<ShaderParameter> getParameters(const HShader& thisPtr);
	};

	/** @endcond */
	/** @} */
}
