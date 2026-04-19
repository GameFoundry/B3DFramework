//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DSLPrerequisites.h"
#include "Material/B3DShader.h"
#include "Material/B3DShaderCompiler.h"

namespace b3d
{
	struct BSLParsedShaderMetaData;
	struct BSLParsedShaderData;

	/** @addtogroup bsfSL
	 *  @{
	 */

	/**	Transforms a source file written in BSL into a shader and shader variations usable by the engine. */
	class BSLCompiler : public IShaderCompiler
	{
	public:
		ShaderCompilerResult Compile(const String& name, const String& source, const UnorderedMap<String, String>& defines, ShadingLanguageFlags languages, bool compileVariations, SPtr<Shader>& outShader) override
		{
			return TCompile<false>(name, source, defines, languages, compileVariations, outShader);
		}

		ShaderCompilerResult Compile(const String& name, const String& source, const UnorderedMap<String, String>& defines, ShadingLanguageFlags languages, bool compileVariations, SPtr<render::Shader>& outShader) override
		{
			return TCompile<true>(name, source, defines, languages, compileVariations, outShader);
		}

		ShaderCompilerResult CompileVariation(const Shader& shader, const ShaderVariationParameters& variationParameters, ShadingLanguageFlag language, Variation& inOutVariation) override
		{
			return TCompileVariation<false>(shader, variationParameters, language, inOutVariation);
		}

		ShaderCompilerResult CompileVariation(const render::Shader& shader, const ShaderVariationParameters& variationParameters, ShadingLanguageFlag language, render::Variation& inOutVariation) override
		{
			return TCompileVariation<true>(shader, variationParameters, language, inOutVariation);
		}
	private:
		/** Templated version of Compile() for both main and render thread. */
		template <bool IsRenderProxy>
		ShaderCompilerResult TCompile(const String& name, const String& source, const UnorderedMap<String, String>& defines, ShadingLanguageFlags languages, bool compileVariations, SPtr<CoreVariantType<Shader, IsRenderProxy>>& outShader);

		/** Templated version of CompileVariation() for both main and render thread. */
		template <bool IsRenderProxy>
		ShaderCompilerResult TCompileVariation(const CoreVariantType<Shader, IsRenderProxy>& shader, const ShaderVariationParameters& variationParameters, ShadingLanguageFlag language, CoreVariantType<Variation, IsRenderProxy>& inOutVariation);

		/**
		 * Compiles a particular shader variation into the low level shader source from the parsed shader data.
		 *
		 * @param	name			Name used to identify the shader.
		 * @param	parsedShader	Parsed shader information, as output by the BSL parser.
		 * @param	shaderMetaData	Meta-data about the shader we're compiling the variation for.
		 * @param	language		Language to compile the variation for. Must be a single language, rather than a mask of multiple languages.
		 * @param	inOutVariation	Object on which to assigned the compiled data if successful.
		 * @return					A result object containing an error message if not successful.
		 */
		template<bool IsRenderProxy>
		static ShaderCompilerResult TCompileVariation(const String& name, const BSLParsedShaderData& parsedShader, const ShaderCompilerMetaData& shaderMetaData, ShadingLanguageFlag language, CoreVariantType<Variation, IsRenderProxy>& inOutVariation);

		/** Converts internal variation representations in the shader meta-data into a set of ShaderVariation objects. */
		static Vector<ShaderVariationParameters> CreateShaderVariations(const BSLParsedShaderMetaData& shaderMetaData);
	};

	/** @} */
} // namespace b3d
