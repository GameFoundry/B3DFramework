//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsSLPrerequisites.h"
#include "Material/BsShader.h"
#include "Material/BsShaderCompiler.h"

namespace bs
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

		ShaderCompilerResult Compile(const String& name, const String& source, const UnorderedMap<String, String>& defines, ShadingLanguageFlags languages, bool compileVariations, SPtr<ct::Shader>& outShader) override
		{
			return TCompile<true>(name, source, defines, languages, compileVariations, outShader);
		}

		ShaderCompilerResult CompileVariation(const Shader& shader, const ShaderVariationParameters& variationParameters, ShadingLanguageFlag language, Technique& inOutVariation) override
		{
			return TCompileVariation<false>(shader, variationParameters, language, inOutVariation);
		}

		ShaderCompilerResult CompileVariation(const ct::Shader& shader, const ShaderVariationParameters& variationParameters, ShadingLanguageFlag language, ct::Technique& inOutVariation) override
		{
			return TCompileVariation<true>(shader, variationParameters, language, inOutVariation);
		}
	private:
		/** Templated version of Compile() for both main and render thread. */
		template <bool Core>
		ShaderCompilerResult TCompile(const String& name, const String& source, const UnorderedMap<String, String>& defines, ShadingLanguageFlags languages, bool compileVariations, SPtr<CoreVariantType<Shader, Core>>& outShader);

		/** Templated version of CompileVariation() for both main and render thread. */
		template <bool Core>
		ShaderCompilerResult TCompileVariation(const CoreVariantType<Shader, Core>& shader, const ShaderVariationParameters& variationParameters, ShadingLanguageFlag language, CoreVariantType<Technique, Core>& inOutVariation);

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
		template<bool Core>
		static ShaderCompilerResult TCompileVariation(const String& name, const BSLParsedShaderData& parsedShader, const ShaderCompilerMetaData& shaderMetaData, ShadingLanguageFlag language, CoreVariantType<Technique, Core>& inOutVariation);

		/** Converts internal variation representations in the shader meta-data into a set of ShaderVariation objects. */
		static Vector<ShaderVariationParameters> CreateShaderVariations(const BSLParsedShaderMetaData& shaderMetaData);
	};

	/** @} */
} // namespace bs
