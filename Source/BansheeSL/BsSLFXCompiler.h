//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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

		/** Information about different variations of a single technique. */
		struct VariationData
		{
			String identifier;
			Vector<UINT32> values;
		};

		/** Information describing a technique, without the actual contents. */
		struct TechniqueMetaData
		{
			String name;
			Vector<String> includes;
			bool isMixin;

			String language;
			StringID renderer = RendererAny;

			Vector<StringID> tags;
			Vector<VariationData> variations;
		};

		/** Temporary data for describing a technique during parsing. */
		struct TechniqueData
		{
			TechniqueMetaData metaData;
			Vector<PassData> passes;
		};

	public:
		/**	Transforms a source file written in BSL FX syntax into a Shader object. */
		static BSLFXCompileResult compile(const String& name, const String& source, 
			const UnorderedMap<String, String>& defines);

	private:
		/** Converts the provided source into an abstract syntax tree using the lexer & parser for BSL FX syntax. */
		static BSLFXCompileResult parseFX(ParseState* parseState, const char* source, 
			const UnorderedMap<String, String>& defines);

		/** Parses the technique node and outputs the relevant meta-data. */
		static TechniqueMetaData parseTechniqueMetaData(ASTFXNode* technique);

		/** Parses technique variations and writes them to the provided meta-data object. */
		static void parseVariations(TechniqueMetaData& metaData, ASTFXNode* variations);

		/**	Converts FX renderer name into an in-engine renderer identifier. */
		static StringID parseRenderer(const String& name);

		/**	Maps FX queue sort type enum into in-engine queue sort type mode. */
		static QueueSortType parseSortType(CullAndSortModeValue sortType);

		/**	Maps FX comparison function enum into in-engine compare function. */
		static CompareFunction parseCompFunc(CompFuncValue compFunc);

		/**	Maps FX operation to in-engine blend factor. */
		static BlendFactor parseBlendFactor(OpValue factor);

		/**	Maps FX blend operation to in-engine blend operation. */
		static BlendOperation parseBlendOp(BlendOpValue op);

		/**	Maps FX operation to in-engine stencil operation. */
		static StencilOperation parseStencilOp(OpValue op);
		
		/**	Maps FX cull mode enum to in-engine cull mode. */
		static CullingMode parseCullMode(CullAndSortModeValue cm);

		/**	Maps FX fill mode enum to in-engine fill mode. */
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
		 * Parses the technique AST node and generates a single technique object.
		 *
		 * @param[in]	techniqueNode	Node to parse.
		 * @param[in]	codeBlocks		GPU program source code.
		 * @param[out]	techniqueData	Will contain technique data after parsing.
		 */
		static void parseTechnique(ASTFXNode* techniqueNode, const Vector<String>& codeBlocks, TechniqueData& techniqueData);

		/**
		 * Parser the options AST node that contains global shader options.
		 * 
		 * @param[in]	optionsNode			Node to parse.
		 * @param[in]	shaderDesc			Descriptor to apply the found options to.
		 */
		static void parseOptions(ASTFXNode* optionsNode, SHADER_DESC& shaderDesc);

		/**
		 * Parses the AST node hierarchy and generates a variation of the of the named technique.
		 *
		 * @param[in, out]	parseState	Parser state object that has previously been initialized with the AST using 
		 *								parseFX().
		 * @param[in]	name			Name of the technique to generate the variation for.
		 * @param[in]	codeBlocks		Blocks containing GPU program source code that are referenced by the AST.
		 * @param[in]	variation		Shader variation the AST was parsed with.
		 * @param[out]	techniques		Vector to append newly found techniques to.
		 * @param[out]	includes		Set to append newly found includes to.
		 * @param[out]	shaderDesc		Shader descriptor too append new parameters to.
		 * @return						A result object containing an error message if not successful.
		 */
		static BSLFXCompileResult parseTechnique(ParseState* parseState, const String& name, 
			const Vector<String>& codeBlocks, const ShaderVariation& variation, Vector<SPtr<Technique>>& techniques, 
			UnorderedSet<String>& includes, SHADER_DESC& shaderDesc);

		/**
		 * Converts a null-terminated string into a standard string, and eliminates quotes that are assumed to be at the 
		 * first and last index.
		 */
		static String removeQuotes(const char* input);
	};

	/** @} */
}