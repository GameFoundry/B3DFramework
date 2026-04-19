//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Material/B3DShader.h"

namespace b3d
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	/** Type of parameters that can be defined by a shader. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) ShaderParameterType
	{
		Float,
		Vector2,
		Vector3,
		Vector4,
		Color,
		Matrix3,
		Matrix4,
		Texture2D,
		Texture3D,
		TextureCube,
		Sampler
	};

	/** Flags used to further describe a shader parameter. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) ShaderParameterFlag
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
	B3D_FLAGS_OPERATORS(ShaderParameterFlag)

	/** Contains information about a single shader parameter. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering), ExportAsStruct(true)) ShaderParameter
	{
		/** Name of the parameter variable. */
		String Name;

		/** Variable identifier of the parameter. */
		String Identifier;

		/** Data type of the parameter. */
		ShaderParameterType Type;

		/** Flags used to further describe the parameter. */
		ShaderParameterFlags Flags;
	};

	/** Extension class for Shader, for adding additional functionality for the script version of the class. */
	class B3D_SCRIPT_EXPORT(ExtensionClassForType(Shader)) ShaderEx
	{
	public:
		/** Returns information about all parameters available in the shader. */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Shader), Property(Getter), ExportName(Parameters))
		static Vector<ShaderParameter> GetParameters(const HShader& thisPtr);
	};

	/** @endcond */
	/** @} */
} // namespace b3d
