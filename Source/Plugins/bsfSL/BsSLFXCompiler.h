//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsSLPrerequisites.h"
#include "Material/BsShader.h"
#include "RenderAPI/BsGpuProgram.h"
#include "RenderAPI/BsRasterizerState.h"
#include "RenderAPI/BsDepthStencilState.h"
#include "RenderAPI/BsBlendState.h"

extern "C" {
#include "BsASTFX.h"
}

namespace bs
{
	/** @addtogroup BansheeSL
	 *  @{
	 */

	/**	Contains the results of compilation returned from the BSLFXCompiler. */
	struct BSLFXCompileResult
	{
		SPtr<Shader> shader; /**< Resulting shader if compilation was successful. Null if error occurred. */
		String errorMessage; /**< Error message if compilation failed. */
		int errorLine = 0; /**< Line of the error if one occurred. */
		int errorColumn = 0; /**< Column of the error if one occurred. */
		String errorFile; /**< File in which the error occurred. Empty if root file. */
	};

	/**	Transforms a source file written in BSL FX syntax into a Shader object. */
	class BSLFXCompiler
	{
		/**	Possible types of code blocks within a shader. */
		enum class CodeBlockType
		{
			Vertex, Fragment, Geometry, Hull, Domain, Compute, Common
		};

		/**	Temporary data describing a pass during parsing. */
		struct PassData
		{
			BLEND_STATE_DESC blendDesc;
			RASTERIZER_STATE_DESC rasterizerDesc;
			DEPTH_STENCIL_STATE_DESC depthStencilDesc;
			UINT32 stencilRefValue = 0;
			UINT32 seqIdx = 0;

			bool blendIsDefault = true;
			bool rasterizerIsDefault = true;
			bool depthStencilIsDefault = true;

			String code; // Parsed code block

			String vertexCode;
			String fragmentCode;
			String geometryCode;
			String hullCode;
			String domainCode;
			String computeCode;
		};

		/** Information about different variations of a single shader. */
		struct VariationData
		{
			String identifier;
			Vector<UINT32> values;
		};

		/** Information describing a shader/mixin node, without the actual contents. */
		struct ShaderMetaData
		{
			String name;
			Vector<String> includes;
			bool isMixin;

			String language;
			String featureSet;

			Vector<StringID> tags;
			Vector<VariationData> variations;
		};

		/** Temporary data for describing a shader/mixin node during parsing. */
		struct ShaderData
		{
			ShaderMetaData metaData;
			Vector<PassData> passes;
		};

		/** Temporary data describing a sub-shader during parsing. */
		struct SubShaderData
		{
			String name;
			Vector<std::pair<ASTFXNode*, ShaderMetaData>> mixins;
		};

	public:
		/**	Transforms a source file written in BSL FX syntax into a Shader object. */
		static BSLFXCompileResult compile(const String& name, const String& source, 
			const UnorderedMap<String, String>& defines);

	private:
		/** Converts the provided source into an abstract syntax tree using the lexer & parser for BSL FX syntax. */
		static BSLFXCompileResult parseFX(ParseState* parseState, const char* source, 
			const UnorderedMap<String, String>& defines);

		/** Parses the shader/mixin node and outputs the relevant meta-data. */
		static ShaderMetaData parseShaderMetaData(ASTFXNode* shader);

		/** 
		 * Parses the root AST node and outputs a list of all mixins/shaders and their meta-data, sub-shader meta-data, 
		 * as well as any global shader options.
		 */
		static BSLFXCompileResult parseMetaDataAndOptions(ASTFXNode* rootNode, 
			Vector<std::pair<ASTFXNode*, ShaderMetaData>>& metaData, Vector<SubShaderData>& subShaders, 
			SHADER_DESC& shaderDesc);

		/** Parses the sub-shader node and outputs the relevant data. */
		static SubShaderData parseSubShader(ASTFXNode* subShader);

		/** Parses shader variations and writes them to the provided meta-data object. */
		static void parseVariations(ShaderMetaData& metaData, ASTFXNode* variations);

		/**	Maps BSL queue sort type enum into in-engine queue sort type mode. */
		static QueueSortType parseSortType(CullAndSortModeValue sortType);

		/**	Maps BSL comparison function enum into in-engine compare function. */
		static CompareFunction parseCompFunc(CompFuncValue compFunc);

		/**	Maps BSL operation to in-engine blend factor. */
		static BlendFactor parseBlendFactor(OpValue factor);

		/**	Maps BSL blend operation to in-engine blend operation. */
		static BlendOperation parseBlendOp(BlendOpValue op);

		/**	Maps BSL operation to in-engine stencil operation. */
		static StencilOperation parseStencilOp(OpValue op);
		
		/**	Maps BSL cull mode enum to in-engine cull mode. */
		static CullingMode parseCullMode(CullAndSortModeValue cm);

		/**	Maps BSL fill mode enum to in-engine fill mode. */
		static PolygonMode parseFillMode(FillModeValue fm);

		/**
		 * Populates the front facing operation portion of the depth-stencil state descriptor from the provided stencil-op
		 * AST node.
		 */
		static void parseStencilFront(DEPTH_STENCIL_STATE_DESC& desc, ASTFXNode* stencilOpNode);

		/**
		 * Populates the back backing operation portion of the depth-stencil state descriptor from the provided stencil-op
		 * AST node.
		 */
		static void parseStencilBack(DEPTH_STENCIL_STATE_DESC& desc, ASTFXNode* stencilOpNode);

		/** Populates the color (RGB) portion of the blend state descriptor from the provided blend definition AST node. */
		static void parseColorBlendDef(RENDER_TARGET_BLEND_STATE_DESC& desc, ASTFXNode* blendDefNode);

		/** Populates the alpha portion of the blend state descriptor from the provided blend definition AST node. */
		static void parseAlphaBlendDef(RENDER_TARGET_BLEND_STATE_DESC& desc, ASTFXNode* blendDefNode);

		/**
		 * Populates blend state descriptor for a single render target from the provided AST node. Which target gets 
		 * updated depends on the index set in the AST node.
		 */
		static void parseRenderTargetBlendState(BLEND_STATE_DESC& desc, ASTFXNode* targetNode);

		/**
		 * Parses the blend state AST node and populates the pass' blend state descriptor. Returns false if the descriptor
		 * wasn't modified.
		 */
		static bool parseBlendState(PassData& passData, ASTFXNode* blendNode);

		/**
		 * Parses the rasterizer state AST node and populates the pass' rasterizer state descriptor. Returns false if the
		 * descriptor wasn't modified.
		 */
		static bool parseRasterizerState(PassData& passData, ASTFXNode* rasterNode);

		/**
		 * Parses the depth state AST node and populates the pass' depth-stencil state descriptor. Returns false if the 
		 * descriptor wasn't modified.
		 */
		static bool parseDepthState(PassData& passData, ASTFXNode* depthNode);

		/**
		* Parses the stencil state AST node and populates the pass' depth-stencil state descriptor. Returns false if the
		* descriptor wasn't modified.
		*/
		static bool parseStencilState(PassData& passData, ASTFXNode* stencilNode);

		/**
		 * Parses a code AST node and outputs the result in one of the streams within the provided pass data.
		 *
		 * @param[in]	codeNode	AST node to parse
		 * @param[in]	codeBlocks	GPU program source code.
		 * @param[in]	passData	Pass data containing temporary pass data, including the code streams that the code 
		 *							block code will be written to.
		 */
		static void parseCodeBlock(ASTFXNode* codeNode, const Vector<String>& codeBlocks, PassData& passData);

		/**
		 * Parses the pass AST node and populates the provided @p passData with all relevant pass parameters.
		 *
		 * @param[in]	passNode		Node to parse.
		 * @param[in]	codeBlocks		GPU program source code.
		 * @param[out]	passData		Will contain pass data after parsing. 
		 */
		static void parsePass(ASTFXNode* passNode, const Vector<String>& codeBlocks, PassData& passData);

		/**
		 * Parses the shader AST node and generates a single shader object.
		 *
		 * @param[in]	shaderNode		Node to parse.
		 * @param[in]	codeBlocks		GPU program source code.
		 * @param[out]	shaderData		Will contain shader data after parsing.
		 */
		static void parseShader(ASTFXNode* shaderNode, const Vector<String>& codeBlocks, ShaderData& shaderData);

		/**
		 * Parser the options AST node that contains global shader options.
		 * 
		 * @param[in]	optionsNode		Node to parse.
		 * @param[in]	shaderDesc		Descriptor to apply the found options to.
		 */
		static void parseOptions(ASTFXNode* optionsNode, SHADER_DESC& shaderDesc);

		/**
		 * Iterates over all provided mixins/shaders and inherits any variations. The variations are written in-place, to
		 * the @p shaderMetaData array, for any non-mixins.
		 */
		static BSLFXCompileResult populateVariations(Vector<std::pair<ASTFXNode*, ShaderMetaData>>& shaderMetaData);

		/**
		 * Parses the provided source and generates a SHADER_DESC containing techniques for all shader variations, any
		 * present sub-shaders, as well as shader parameters. Also outputs a set of includes included by the shader code.
		 * 
		 * @param[in]	source				BSL source that needs to be parsed.
		 * @param[in]	defines				An optional set of defines to set before parsing the source, that is to be
		 *									applied to all variations.
		 * @param[in]	parentSubShader		Information about an optional parent sub-shader. If present the sub-shader
		 *									mixins will be appended to the parsed shader source, overriding any existing
		 *									mixins with the same name.
		 * @param[out]	shaderDesc			Shader descriptor that resulting techniques, sub-shaders, and parameters will be
		 *									registered with.
		 * @param[out]	includes			A list of all include files included by the BSL source.
		 * @return							A result object containing an error message if not successful.
		 */
		static BSLFXCompileResult compileShader(String source, const UnorderedMap<String, String>& defines, 
				const SubShaderData& parentSubShader, SHADER_DESC& shaderDesc, Vector<String>& includes);

		/**
		 * Uses the provided list of shaders/mixins to generate a list of techniques. A technique is generated for
		 * every variation and render backend.
		 * 
		 * @param[in]	shaderMetaData		A list of mixins and shaders. Shaders should contain a list of variations to
		 *									generate (usually populated via a previous call to populateVariations()).
		 * @param[in]	source				Original BSL source the mixins/shaders were parsed from. Required as the source
		 *									needs to be re-parsed due to variations.
		 * @param[in]	defines				An optional set of defines to set before parsing the source, that is to be
		 *									applied to all variations.
		 * @param[out]	shaderDesc			Shader descriptor that resulting techniques, and non-internal parameters will be
		 *									registered with.
		 * @param[out]	includes			A list of all include files included by the BSL source.
		 * @return							A result object containing an error message if not successful.
		 */
		static BSLFXCompileResult compileTechniques(const Vector<std::pair<ASTFXNode*, ShaderMetaData>>& shaderMetaData,
			const String& source, const UnorderedMap<String, String>& defines, SHADER_DESC& shaderDesc, 
			Vector<String>& includes);

		/**
		 * Generates a set of techniques for a single variation. Uses AST parse state as input, which must be created using
		 * the defines of the relevant variation.
		 *
		 * @param[in, out]	parseState	Parser state object that has previously been initialized with the AST using 
		 *								parseFX().
		 * @param[in]	name			Name of the shader to generate the variation for.
		 * @param[in]	codeBlocks		Blocks containing GPU program source code that are referenced by the AST.
		 * @param[in]	variation		Shader variation the AST was parsed with.
		 * @param[out]	includes		Set to append newly found includes to.
		 * @param[out]	shaderDesc		Shader descriptor that resulting techniques, and non-internal parameters will be
		 *								registered with.
		 * @return						A result object containing an error message if not successful.
		 */
		static BSLFXCompileResult compileTechniques(ParseState* parseState, const String& name, 
			const Vector<String>& codeBlocks, const ShaderVariation& variation, UnorderedSet<String>& includes, 
			SHADER_DESC& shaderDesc);

		/**
		 * Converts a null-terminated string into a standard string, and eliminates quotes that are assumed to be at the 
		 * first and last index.
		 */
		static String removeQuotes(const char* input);
	};

	/** @} */
}