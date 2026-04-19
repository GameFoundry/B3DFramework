//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DSLPrerequisites.h"
#include "Material/B3DShader.h"
#include "RenderAPI/B3DGpuProgram.h"
#include "Importer/B3DShaderImportOptions.h"
#include "Material/B3DShaderCompiler.h"
#include "RenderAPI/B3DGpuPipelineState.h"

extern "C" {
#include "B3DASTFX.h"
}

namespace b3d
{
	/** @addtogroup bsfSL
	 *  @{
	 */

	/** Value of a single variation option along with an optional name. */
	struct BSLParsedVariationOption
	{
		String Name;
		u32 Value;
	};

	/** Information about different variations of a single shader. */
	struct BSLParsedVariationData
	{
		String Name;
		String Identifier;
		bool Internal = true;
		Vector<BSLParsedVariationOption> Values;
	};

	/**	Information about a single parsed pass node. */
	struct BSLParsedShaderPassData
	{
		BlendStateInformation BlendStateInformation;
		RasterizerStateInformation RasterizerStateInformation;
		DepthStencilStateInformation DepthStencilStateInformation;
		u32 StencilReferenceValue = 0;
		u32 SequentialIndex = 0;

		bool BlendStateIsDefault = true;
		bool RasterizerStateIsDefault = true;
		bool DepthStencilStateIsDefault = true;

		String Code; // Parsed code block
	};

	/** Information describing a shader/mixin node, without the actual contents. */
	struct BSLParsedShaderMetaData
	{
		String Name;
		Vector<String> Includes;
		bool IsMixin = false;

		String Language;
		String FeatureSet;

		Vector<StringID> Tags;
		Vector<BSLParsedVariationData> Variations;
	};

	/** Temporary data for describing a shader/mixin node during parsing. */
	struct BSLParsedShaderData
	{
		BSLParsedShaderMetaData MetaData;
		Vector<BSLParsedShaderPassData> Passes;
	};

	/**	Parses BSL code into meta-data and low-level shader code which can then be compiled into an actual shader and shader variations. */
	class BSLParser
	{
	public:
		/**
		 * Parses the BSL source and outputs meta-data for every shader and mixin node.
		 *
		 * @param	source					BSL source to parse.
		 * @param	defines					An optional set of defines to set before parsing the source.
		 * @param	inOutShaderInformation	Object to append shader reflection data to.
		 * @param	outShaderMetaData		Parsed shader meta-data if parse was successful.
		 * @param	outIncludes				A list of all includes files included by the BSL source.
		 * @return							A result object containing an error message if not successful.
		 */
		static ShaderCompilerResult ParseMetaData(const String& source, const UnorderedMap<String, String>& defines, ShaderCreateInformation& inOutShaderInformation, BSLParsedShaderMetaData& outShaderMetaData, Vector<String>& outIncludes)
		{
			return TParseMetaData<false>(source, defines, inOutShaderInformation, outShaderMetaData, outIncludes);
		}

		/** @copydoc ParseMetaData(const String&, const UnorderedMap<String, String>&, ShaderCreateInformation&, BSLParsedShaderMetaData&, Vector<String>&). */
		static ShaderCompilerResult ParseMetaData(const String& source, const UnorderedMap<String, String>& defines, render::ShaderCreateInformation& inOutShaderInformation, BSLParsedShaderMetaData& outShaderMetaData, Vector<String>& outIncludes)
		{
			return TParseMetaData<true>(source, defines, inOutShaderInformation, outShaderMetaData, outIncludes);
		}

		/**
		 * Parses the BSL shader source for a specific variation.
		 *
		 * @param	name				Name used to identify the shader.
		 * @param	source				BSL source to parse.
		 * @param	variation			Variation to enable when parsing the source.
		 * @param	defines				Optional set of defines to enable during parse.
		 * @param	outParsedShader		Output information about the parsed variation if succesful.
		 * @return						A result object containing an error message if not successful.
		 */
		static ShaderCompilerResult ParseVariation(const String& name, const String& source, const ShaderVariationParameters& variation, const UnorderedMap<String, String>& defines, BSLParsedShaderData& outParsedShader);

	private:
		/**	Possible types of code blocks within a shader. */
		enum class CodeBlockType
		{
			Vertex,
			Fragment,
			Geometry,
			Hull,
			Domain,
			Compute,
			Common
		};

		/** A set of attributes describing a BSL construct. */
		struct AttributeData
		{
			Vector<std::pair<i32, String>> Attributes;
		};

		/** Converts the provided source into an abstract syntax tree using the lexer & parser for BSL FX syntax. */
		static ShaderCompilerResult RunParser(ParseState* parseState, const char* source, const UnorderedMap<String, String>& defines);

		/** Templated version of ParseMetaData for both main and render thread. */
		template<bool IsRenderProxy>
		static ShaderCompilerResult TParseMetaData(const String& source, const UnorderedMap<String, String>& defines, CoreVariantType<ShaderCreateInformation, IsRenderProxy>& inOutShaderInformation, BSLParsedShaderMetaData& outShaderMetaData, Vector<String>& outIncludes);

		/** Parses the shader/mixin node and outputs the relevant meta-data. */
		static BSLParsedShaderMetaData ParseShaderMetaData(ASTFXNode* shader);

		/**
		 * Parses the root AST node and outputs a list of all mixins/shaders and their meta-data, sub-shader meta-data,
		 * as well as any global shader options.
		 */
		template<bool IsRenderProxy>
		static ShaderCompilerResult TParseMetaDataAndOptions(ASTFXNode* rootNode, Vector<std::pair<ASTFXNode*, BSLParsedShaderMetaData>>& metaData, CoreVariantType<ShaderCreateInformation, IsRenderProxy>& shaderCreateInformation);

		/** Parses shader variations and writes them to the provided meta-data object. */
		static void ParseVariations(BSLParsedShaderMetaData& metaData, ASTFXNode* variations);

		/** Parses a single variation option node. */
		static BSLParsedVariationOption ParseVariationOption(ASTFXNode* variationOption);

		/** Parses BSL attributes. */
		static AttributeData ParseAttributes(ASTFXNode* attributes);

		/**	Maps BSL queue sort type enum into in-engine queue sort type mode. */
		static QueueSortType ParseSortType(CullAndSortModeValue sortType);

		/**	Maps BSL comparison function enum into in-engine compare function. */
		static CompareFunction ParseCompFunc(CompFuncValue compFunc);

		/**	Maps BSL operation to in-engine blend factor. */
		static BlendFactor ParseBlendFactor(OpValue factor);

		/**	Maps BSL blend operation to in-engine blend operation. */
		static BlendOperation ParseBlendOp(BlendOpValue op);

		/**	Maps BSL operation to in-engine stencil operation. */
		static StencilOperation ParseStencilOp(OpValue op);

		/**	Maps BSL cull mode enum to in-engine cull mode. */
		static CullingMode ParseCullMode(CullAndSortModeValue cm);

		/**	Maps BSL fill mode enum to in-engine fill mode. */
		static PolygonMode ParseFillMode(FillModeValue fm);

		/**
		 * Populates the front facing operation portion of the depth-stencil state descriptor from the provided stencil-op
		 * AST node.
		 */
		static void ParseStencilFront(DepthStencilStateInformation& desc, ASTFXNode* stencilOpNode);

		/**
		 * Populates the back backing operation portion of the depth-stencil state descriptor from the provided stencil-op
		 * AST node.
		 */
		static void ParseStencilBack(DepthStencilStateInformation& desc, ASTFXNode* stencilOpNode);

		/** Populates the color (RGB) portion of the blend state descriptor from the provided blend definition AST node. */
		static void ParseColorBlendDef(RenderTargetBlendStateInformation& desc, ASTFXNode* blendDefNode);

		/** Populates the alpha portion of the blend state descriptor from the provided blend definition AST node. */
		static void ParseAlphaBlendDef(RenderTargetBlendStateInformation& desc, ASTFXNode* blendDefNode);

		/**
		 * Populates blend state descriptor for a single render target from the provided AST node. Which target gets
		 * updated depends on the index set in the AST node.
		 */
		static void ParseRenderTargetBlendState(BlendStateInformation& desc, ASTFXNode* targetNode, u32& index);

		/**
		 * Parses the blend state AST node and populates the pass' blend state descriptor. Returns false if the descriptor
		 * wasn't modified.
		 */
		static bool ParseBlendState(BSLParsedShaderPassData& passData, ASTFXNode* blendNode);

		/**
		 * Parses the rasterizer state AST node and populates the pass' rasterizer state descriptor. Returns false if the
		 * descriptor wasn't modified.
		 */
		static bool ParseRasterizerState(BSLParsedShaderPassData& passData, ASTFXNode* rasterNode);

		/**
		 * Parses the depth state AST node and populates the pass' depth-stencil state descriptor. Returns false if the
		 * descriptor wasn't modified.
		 */
		static bool ParseDepthState(BSLParsedShaderPassData& passData, ASTFXNode* depthNode);

		/**
		 * Parses the stencil state AST node and populates the pass' depth-stencil state descriptor. Returns false if the
		 * descriptor wasn't modified.
		 */
		static bool ParseStencilState(BSLParsedShaderPassData& passData, ASTFXNode* stencilNode);

		/**
		 * Parses a code AST node and outputs the result in one of the streams within the provided pass data.
		 *
		 * @param[in]	codeNode	AST node to parse
		 * @param[in]	codeBlocks	GPU program source code.
		 * @param[in]	passData	Pass data containing temporary pass data, including the code streams that the code
		 *							block code will be written to.
		 */
		static void ParseCodeBlock(ASTFXNode* codeNode, const Vector<String>& codeBlocks, BSLParsedShaderPassData& passData);

		/**
		 * Parses the pass AST node and populates the provided @p passData with all relevant pass parameters.
		 *
		 * @param[in]	passNode		Node to parse.
		 * @param[in]	codeBlocks		GPU program source code.
		 * @param[out]	passData		Will contain pass data after parsing.
		 */
		static void ParsePass(ASTFXNode* passNode, const Vector<String>& codeBlocks, BSLParsedShaderPassData& passData);

		/**
		 * Parses the shader AST node and generates a single shader object.
		 *
		 * @param[in]	shaderNode		Node to parse.
		 * @param[in]	codeBlocks		GPU program source code.
		 * @param[out]	shaderData		Will contain shader data after parsing.
		 */
		static void ParseShader(ASTFXNode* shaderNode, const Vector<String>& codeBlocks, BSLParsedShaderData& shaderData);

		/**
		 * Parser the options AST node that contains global shader options.
		 *
		 * @param	optionsNode						Node to parse.
		 * @param	outShaderCreateInformation		Descriptor to apply the found options to.
		 */
		template<bool IsRenderProxy>
		static void TParseOptions(ASTFXNode* optionsNode, CoreVariantType<ShaderCreateInformation, IsRenderProxy>& outShaderCreateInformation);

		/**
		 * Iterates over all provided mixins/shaders and inherits any variations. The variations are written in-place, to
		 * the @p shaderMetaData array, for any non-mixins.
		 */
		static ShaderCompilerResult PopulateVariations(Vector<std::pair<ASTFXNode*, BSLParsedShaderMetaData>>& shaderMetaData);

		/** Populates the information about variation parameters and their values. */
		template<bool IsRenderProxy>
		static void TPopulateVariationParameters(const BSLParsedShaderMetaData& shaderMetaData, CoreVariantType<ShaderCreateInformation, IsRenderProxy>& shaderCreateInformation);

		/**
		 * Converts a null-terminated string into a standard string, and eliminates quotes that are assumed to be at the
		 * first and last index.
		 */
		static String RemoveQuotes(const char* input);
	};

	/** @} */
} // namespace b3d
