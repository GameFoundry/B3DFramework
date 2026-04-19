//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DSLPrerequisites.h"
#include "Material/B3DShaderCompiler.h"

namespace b3d
{
	/** @addtogroup bsfSL
	 *  @{
	 */

	/** Types of supported code output when cross compiling HLSL to GLSL. */
	enum class HLSLCrossCompileOutput
	{
		GLSL45, /**< Output GLSL version 4.5 */
		GLSL41, /**< Output GLSL version 4.1 */
		VKSL45, /**< Output GLSL compatible with Vulkan's GLSL->SPIR-V cross compiler. */
		MVKSL /**< Output GLSL compatible with Metal's GLSL->MSL cross compiler. */
	};

	/**	Transforms HLSL into other shading languages, and also outputs reflection data. */
	class HLSLCrossCompiler
	{
	public:
		/**
		 * Cross compiles the provided HLSL source code into the requested output language.
		 *
		 * @param	hlsl					HLSL to cross compile.
		 * @param	type					GPU program stage that the HLSL code represents.
		 * @param	outputType				Language to cross-compile to.
		 * @param	inOutStartBindingSlot	Slot at which to start assigning resource bindings. This is important if your shader has multiple GPU program types, in which case you wish to set
		 *									this to 0 for the first program type, and keep passing the value to any next program, which ensures that each program of the shader gets a unique set of bindings.
		 * @param	outSource				Cross compiled shader source, if successful.
		 * @return							A result object containing an error message if not successful.
		 */
		static ShaderCompilerResult CrossCompile(const String& hlsl, GpuProgramType type, HLSLCrossCompileOutput outputType, u32& inOutStartBindingSlot, String& outSource);

		/**
		 * Parses the provided HLSL source code and outputs reflection information.
		 *
		 * @param	hlsl							HLSL to reflect.
		 * @param	outShaderCreateInformation		Object which will be appended with information about GPU program parameters.
		 * @param	outEntryPoints					A list of all detected entry points in the sshader.
		 * @return									A result object containing an error message if not successful.
		 */
		static ShaderCompilerResult Reflect(const String& hlsl, ShaderCreateInformation& outShaderCreateInformation, TInlineArray<GpuProgramType, 2>& outEntryPoints)
		{
			return TReflect<false>(hlsl, outShaderCreateInformation, outEntryPoints);
		}

		/** @copydoc Reflect(const String&, ShaderCreateInformation&, SmallVector<GpuProgramType, 2>&) */
		static ShaderCompilerResult Reflect(const String& hlsl, render::ShaderCreateInformation& outShaderCreateInformation, TInlineArray<GpuProgramType, 2>& outEntryPoints)
		{
			return TReflect<true>(hlsl, outShaderCreateInformation, outEntryPoints);
		}

	private:
		template<bool IsRenderProxy>
		static ShaderCompilerResult TReflect(const String& hlsl, CoreVariantType<ShaderCreateInformation, IsRenderProxy>& outShaderCreateInformation, TInlineArray<GpuProgramType, 2>& outEntryPoints); // TODO - Output reflection information in a more generalized form, rather than ShaderCreateInformation
	};

	/** @} */
} // namespace b3d
