//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsSLPrerequisites.h"
#include "BsShader.h"
#include "BsGpuProgram.h"
#include "BsRasterizerState.h"
#include "BsDepthStencilState.h"
#include "BsBlendState.h"

extern "C" {
#include "BsASTFX.h"
}

namespace BansheeEngine
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

			String commonCode;
			String vertexCode;
			String fragmentCode;
			String geometryCode;
			String hullCode;
			String domainCode;
			String computeCode;
		};

		/** Information describing a technique, without the actual contents. */
		struct TechniqueMetaData
		{
			StringID renderer = RendererAny;
			StringID renderAPI = RenderAPIAny;
			Vector<StringID> tags;
			String language;

			String baseName;
			Vector<String> inherits;
		};

		/** Temporary data for describing a technique during parsing. */
		struct TechniqueData
		{
			TechniqueMetaData metaData;
			Vector<PassData> passes;
		};

	public:
		/**	Transforms a source file written in BSL FX syntax into a Shader object. */
		static BSLFXCompileResult compile(const String& source, const UnorderedMap<String, String>& defines);

	private:
		/** Converts the provided source into an abstract syntax tree using the lexer & parser for BSL FX syntax. */
		static void parseFX(ParseState* parseState, const char* source);

		/** Parses the technique node and outputs the relevant meta-data. */
		static TechniqueMetaData parseTechniqueMetaData(ASTFXNode* technique);

		/**	Converts FX renderer name into an in-engine renderer identifier. */
		static StringID parseRenderer(const String& name);

		/**
		 * Converts FX language into an in-engine shader language (for example hlsl, glsl) and a rendering API that supports
		 * the provided language.
		 */
		static void parseLanguage(const String& name, StringID& renderAPI, String& language);

		/**	Maps FX buffer usage enum into in-engine param block usage. */
		static GpuParamBlockUsage parseBlockUsage(BufferUsageValue usage);

		/**	Maps FX filter mode enum into in-engine filter mode. */
		static UINT32 parseFilterMode(FilterValue filter);

		/**	Maps FX queue sort type enum into in-engine queue sort type mode. */
		static QueueSortType parseSortType(QueueSortTypeValue sortType);

		/**	Maps FX comparison function enum into in-engine compare function. */
		static CompareFunction parseCompFunc(CompFuncValue compFunc);

		/**	Maps FX addressing mode enum into in-engine addressing mode. */
		static TextureAddressingMode parseAddrMode(AddrModeValue addrMode);

		/**	Maps FX operation to in-engine blend factor. */
		static BlendFactor parseBlendFactor(OpValue factor);

		/**	Maps FX blend operation to in-engine blend operation. */
		static BlendOperation parseBlendOp(BlendOpValue op);

		/**
		 * Maps FX parameter type to in-engine shader parameter.
		 *
		 * @param[in]	type		Input FX parameter type.
		 * @param[in]	isObjType	Output parameter signaling whether the in-engine parameter is a data or an object type.
		 * @param[in]	typeId		Type ID corresponding to a value of in-game GpuParamDataType or GpuParamObjectType
		 *							enum (depending on isObjType()).
		 */
		static void parseParamType(ParamType type, bool& isObjType, UINT32& typeId);

		/**	Maps FX operation to in-engine stencil operation. */
		static StencilOperation parseStencilOp(OpValue op);
		
		/**	Maps FX cull mode enum to in-engine cull mode. */
		static CullingMode parseCullMode(CullModeValue cm);

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

		/**
		 * Populates the addressing mode portion of the sampler state descriptor for U/V/W axes from the provided addressing
		 * mode AST node.
		 */
		static void parseAddrMode(SAMPLER_STATE_DESC& desc, ASTFXNode* addrModeNode);

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
		 * Parses the blend state AST node and outputs a blend state descriptor. Returns false if the descriptor wasn't 
		 * modified.
		 */
		static bool parseBlendState(BLEND_STATE_DESC& desc, ASTFXNode* passNode);

		/**
		 * Parses the rasterizer state AST node and outputs a rasterizer state descriptor. Returns false if the descriptor
		 * wasn't modified.
		 */
		static bool parseRasterizerState(RASTERIZER_STATE_DESC& desc, ASTFXNode* passNode);

		/**
		 * Parses the depth-stencil state AST node and outputs a depth-stencil state descriptor. Returns false if the 
		 * descriptor wasn't modified.
		 */
		static bool parseDepthStencilState(DEPTH_STENCIL_STATE_DESC& desc, ASTFXNode* passNode);

		/** Parses the sampler state AST node and outputs a sampler state object, or a nullptr in case AST node is empty. */
		static SPtr<SamplerState> parseSamplerState(ASTFXNode* samplerStateNode);

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
		 * Parses the pass AST node and populates the provided @passData with all relevant pass parameters.
		 *
		 * @param[in]	passNode		Node to parse.
		 * @param[in]	codeBlocks		GPU program source code.
		 * @param[out]	passData		Will contain pass data after parsing. 
		 */
		static void parsePass(ASTFXNode* passNode, const Vector<String>& codeBlocks, PassData& passData);

		/**
		 * Parses the technique AST node and generates a single technique object. Returns null if no technique can be 
		 * parsed.
		 *
		 * @param[in]	techniqueNode	Node to parse.
		 * @param[in]	codeBlocks		GPU program source code.
		 * @param[out]	techniqueData	Will contain technique data after parsing.
		 */
		static void parseTechnique(ASTFXNode* techniqueNode, const Vector<String>& codeBlocks, TechniqueData& techniqueData);

		/**
		 * Parses the parameters AST node and populates the shader descriptor with information about GPU program parameters
		 * and their default values.
		 */
		static void parseParameters(SHADER_DESC& desc, ASTFXNode* parametersNode);

		/**
		 * Parses the blocks AST node and populates the shader descriptor with information about GPU program parameter 
		 * blocks.
		 */
		static void parseBlocks(SHADER_DESC& desc, ASTFXNode* blocksNode);

		/**
		 * Parses the AST node hierarchy and generates a shader object.
		 *
		 * @param[in]		name		Optional name for the shader.
		 * @param[in, out]	parseState	Parser state object that has previously been initialized with the AST using 
		 *								parseFX().
		 * @param	codeBlocks			GPU program source code.
		 * @return						A result object containing the shader if successful, or error message if not.
		 */
		static BSLFXCompileResult parseShader(const String& name, ParseState* parseState, Vector<String>& codeBlocks);

		/**
		 * Converts a null-terminated string into a standard string, and eliminates quotes that are assumed to be at the 
		 * first and last index.
		 */
		static String removeQuotes(const char* input);

		/**	Retrieves a GPU program profile to use with the specified API and GPU program type. */
		static GpuProgramProfile getProfile(const StringID& renderAPI, GpuProgramType type);

		/** Returns one of the builtin textures based on their name. */
		static HTexture getBuiltinTexture(const String& name);
	};

	/** @} */
}