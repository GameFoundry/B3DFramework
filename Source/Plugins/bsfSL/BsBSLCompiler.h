//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsSLPrerequisites.h"
#include "Material/BsShader.h"
#include "RenderAPI/BsGpuProgram.h"
#include "Importer/BsShaderImportOptions.h"
#include "Material/BsShader.h"

namespace bs
{
	struct BSLParsedShaderMetaData;
	struct BSLParsedShaderData;

	/** @addtogroup bsfSL
	 *  @{
	 */

	/** Meta-data for a shader compiled from BSL. Can be used for compiling specific variations of the shader. */
	struct BSLFXShaderMetaData
	{
		String Name;
		ShaderCreateInformation ShaderInformation; /**< Information used to create the Shader object. **/
		SmallVector<GpuProgramType, 2> GPUProgramTypes; /**< Types of GPU programs used by the shader. */
		Vector<ShaderVariation> Variations; /**< Sets of defines controlling which variations of the shader are present. */
		Vector<String> Includes; /**< All shader header files referenced by the shader. */
		UnorderedMap<String, String> Defines; /**< Optional list of defines to provide when compiling the shader variations. This is added along with the shader variation defines. */
		bool HasGPUProgramMetaData = true; /**< True if GPU program information has been initialized. This will only be true after the first variation for the shader has been compiled. */
	};

	/**	Transforms a source file written in BSL into a shader and shader variations usable by the engine. */
	class BSLCompiler
	{
	public:
		/**
		 * Compiles the shader from BSL and outputs a Shader object with all possible variations compiled and ready to use.
		 *
		 * @param		name				Name used to identify the shader.
		 * @param		source				BSL source to compile.
		 * @param		defines				An optional set of defines to set during compilation.
		 * @param		languages			Low-level languages to compile individual variations for. Each language will result in another set of variations.
		 * @param		outShader			Shader if the compilation is successful, null otherwise.
		 * @return							A result object containing an error message if not successful.
		 */
		static BSLResult Compile(const String& name, const String& source, const UnorderedMap<String, String>& defines, ShadingLanguageFlags languages, SPtr<Shader>& outShader);

		/**
		 * Parses the BSL source and outputs meta-data that may be used for constructing the Shader object and compiling individual variations. After creating the Shader this way, you must manually compile the techniques by calling CompileVariation().
		 *
		 * @param		source				BSL source to parse.
		 * @param		defines				An optional set of defines to set before parsing the source.
		 * @param		outShaderMetaData	Shader meta-data if the parsing is successful, null otherwise.
		 * @return							A result object containing an error message if not successful.
		 */
		static BSLResult CompileMetaData(const String& source, const UnorderedMap<String, String>& defines, SPtr<BSLFXShaderMetaData>& outShaderMetaData);

		/**
		 * Compiles a particular shader variation.
		 *
		 * @param		source					BSL source to compile.
		 * @param		variation				Specific variation to compile.
		 * @param		language				Language to compile the variation for. Must be a single language, rather than a mask of multiple languages.
		 * @param		inOutShaderMetaData		Shader meta-data as returned from CompileMetaData(). Mutable as internally it caches values so that it can speed up compilation of other variations.
		 * @param		outVariation			Compiled variation if successful, null otherwise.
		 * @return								A result object containing an error message if not successful.
		 */
		static BSLResult CompileVariation(const String& source, const ShaderVariation& variation, ShadingLanguageFlag language, BSLFXShaderMetaData& inOutShaderMetaData, SPtr<Technique>& outVariation);

	private:
		/**
		 * Compiles a particular shader variation.
		 *
		 * @param	name			Name used to identify the shader.
		 * @param	parsedShader	Parsed shader information, as output by the BSL parser.
		 * @param	shaderMetaData	Meta-data about the shader we're compiling the variation for.
		 * @param	variation		Variation to compile for. Must be the same variation that was provided when parsing the shader.
		 * @param	language		Language to compile the variation for. Must be a single language, rather than a mask of multiple languages.
		 * @param	outVariation	Compiled variation if successful, null otherwise.
		 * @return					A result object containing an error message if not successful.
		 */
		static BSLResult CompileVariation(const String& name, const BSLParsedShaderData& parsedShader, const BSLFXShaderMetaData& shaderMetaData, const ShaderVariation& variation, ShadingLanguageFlag language, SPtr<Technique>& outVariation);

		/** Converts internal variation representations in the shader meta-data into a set of ShaderVariation objects. */
		static Vector<ShaderVariation> CreateShaderVariations(const BSLParsedShaderMetaData& shaderMetaData);
	};

	/** @} */
} // namespace bs
