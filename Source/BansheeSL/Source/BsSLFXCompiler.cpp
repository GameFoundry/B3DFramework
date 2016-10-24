//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSLFXCompiler.h"
#include "BsGpuProgram.h"
#include <regex>
#include "BsShader.h"
#include "BsTechnique.h"
#include "BsPass.h"
#include "BsSamplerState.h"
#include "BsRenderAPI.h"
#include "BsDebug.h"
#include "BsShaderManager.h"
#include "BsShaderInclude.h"
#include "BsMatrix4.h"
#include "BsBuiltinResources.h"

extern "C" {
#include "BsMMAlloc.h"
#include "BsParserFX.h"
#include "BsLexerFX.h"
}

using namespace std;

namespace BansheeEngine
{
	// Print out the FX AST, only for debug purposes
	void SLFXDebugPrint(ASTFXNode* node, String indent)
	{
		LOGDBG(indent + "NODE " + toString(node->type));

		for (int i = 0; i < node->options->count; i++)
		{
			OptionDataType odt = OPTION_LOOKUP[(int)node->options->entries[i].type].dataType;
			if (odt == ODT_Complex)
			{
				LOGDBG(indent + toString(i) + ". " + toString(node->options->entries[i].type));
				SLFXDebugPrint(node->options->entries[i].value.nodePtr, indent + "\t");
				continue;
			}

			String value;
			switch (odt)
			{
			case ODT_Bool:
				value = toString(node->options->entries[i].value.intValue != 0);
				break;
			case ODT_Int:
				value = toString(node->options->entries[i].value.intValue);
				break;
			case ODT_Float:
				value = toString(node->options->entries[i].value.floatValue);
				break;
			case ODT_String:
				value = node->options->entries[i].value.strValue;
				break;
			case ODT_Matrix:
			{
				Matrix4 mat4 = *(Matrix4*)(node->options->entries[i].value.matrixValue);
				value = toString(mat4);
			}
				break;
			default:
				break;
			}

			LOGDBG(indent + toString(i) + ". " + toString(node->options->entries[i].type) + " = " + value);
		}
	}

	BSLFXCompileResult BSLFXCompiler::compile(const String& source, const UnorderedMap<String, String>& defines)
	{
		BSLFXCompileResult output;

		String parsedSource = source;

		ParseState* parseState = parseStateCreate();
		for(auto& define : defines)
		{
			if (define.first.size() == 0)
				continue;

			addDefine(parseState, define.first.c_str());

			if(define.second.size() > 0)
				addDefineExpr(parseState, define.second.c_str());
		}

		parseFX(parseState, parsedSource.c_str());

		if (parseState->hasError > 0)
		{
			output.errorMessage = parseState->errorMessage;
			output.errorLine = parseState->errorLine;
			output.errorColumn = parseState->errorColumn;

			if(parseState->errorFile != nullptr)
				output.errorFile = parseState->errorFile;

			parseStateDelete(parseState);
		}
		else
		{
			// Only enable for debug purposes
			//SLFXDebugPrint(parseState->rootNode, "");

			Vector<String> codeBlocks;
			CodeString* codeString = parseState->codeStrings;
			while(codeString != nullptr)
			{
				while ((INT32)codeBlocks.size() <= codeString->index)
					codeBlocks.push_back(String());

				codeBlocks[codeString->index] = String(codeString->code, codeString->size);
				codeString = codeString->next;
			}

			output = parseShader("Shader", parseState, codeBlocks);

			StringStream gpuProgError;
			bool hasError = false;
			if (output.shader != nullptr)
			{
				Vector<SPtr<Technique>> techniques = output.shader->getCompatibleTechniques();

				for (auto& technique : techniques)
				{
					UINT32 numPasses = technique->getNumPasses();

					for (UINT32 i = 0; i < numPasses; i++)
					{
						SPtr<Pass> pass = technique->getPass(i);

						auto checkCompileStatus = [&](const String& prefix, const SPtr<GpuProgram>& prog)
						{
							if (prog != nullptr)
							{
								prog->blockUntilCoreInitialized();

								if (!prog->isCompiled())
								{
									hasError = true;
									gpuProgError << prefix << ": " << prog->getCompileErrorMessage() << std::endl;
								}
							}
						};

						checkCompileStatus("Vertex program", pass->getVertexProgram());
						checkCompileStatus("Fragment program", pass->getFragmentProgram());
						checkCompileStatus("Geometry program", pass->getGeometryProgram());
						checkCompileStatus("Hull program", pass->getHullProgram());
						checkCompileStatus("Domain program", pass->getDomainProgram());
						checkCompileStatus("Compute program", pass->getComputeProgram());
					}
				}
			}

			if (hasError)
			{
				output.shader = nullptr;
				output.errorMessage = "Failed compiling GPU program(s): " + gpuProgError.str();
				output.errorLine = 0;
				output.errorColumn = 0;
			}
		}

		return output;
	}

	void BSLFXCompiler::parseFX(ParseState* parseState, const char* source)
	{
		yyscan_t scanner;
		YY_BUFFER_STATE state;

		if (yylex_init_extra(parseState, &scanner))
			return;

		// If debug output from lexer is needed uncomment this and add %debug option to lexer file
		//yyset_debug(true, scanner);

		// If debug output from parser is needed uncomment this and add %debug option to parser file
		//yydebug = true;

		state = yy_scan_string(source, scanner);

		if (yyparse(parseState, scanner))
			return;

		yy_delete_buffer(state, scanner);
		yylex_destroy(scanner);
	}

	BSLFXCompiler::TechniqueMetaData BSLFXCompiler::parseTechniqueMetaData(ASTFXNode* technique)
	{
		TechniqueMetaData metaData;

		metaData.renderer = RendererAny;
		metaData.language = "Any";

		for (int i = 0; i < technique->options->count; i++)
		{
			NodeOption* option = &technique->options->entries[i];

			switch (option->type)
			{
			case OT_Renderer:
				metaData.renderer = parseRenderer(removeQuotes(option->value.strValue));
				break;
			case OT_Language:
				parseLanguage(removeQuotes(option->value.strValue), metaData.renderAPI, metaData.language);
				break;
			case OT_Tags:
			{
				ASTFXNode* tagsNode = option->value.nodePtr;
				for (int j = 0; j < tagsNode->options->count; j++)
				{
					NodeOption* tagOption = &tagsNode->options->entries[j];

					if (tagOption->type == OT_TagValue)
						metaData.tags.push_back(removeQuotes(tagOption->value.strValue));
				}
			}
				break;
			case OT_Base:
				metaData.baseName = removeQuotes(option->value.strValue);
				break;
			case OT_Inherits:
				metaData.inherits.push_back(removeQuotes(option->value.strValue));
				break;
			default:
				break;
			}
		}

		return metaData;
	}

	StringID BSLFXCompiler::parseRenderer(const String& name)
	{
		if (name == "Any")
			return RendererAny;
		else if (name == "Default")
			return RendererDefault;

		return RendererAny;
	}

	void BSLFXCompiler::parseLanguage(const String& name, StringID& renderAPI, String& language)
	{
		if (name == "HLSL" || name == "HLSL11")
		{
			renderAPI = RenderAPIDX11;
			language = "hlsl";
		}
		else if (name == "HLSL9")
		{
			renderAPI = RenderAPIDX9;
			language = "hlsl9";
		}
		else if (name == "GLSL")
		{
			renderAPI = RenderAPIOpenGL;
			language = "glsl";
		}
		else // "Any"
		{
			renderAPI = RenderAPIAny;
			language = "";
		}
	}

	GpuParamBlockUsage BSLFXCompiler::parseBlockUsage(BufferUsageValue usage)
	{
		if (usage == BUV_Dynamic)
			return GPBU_DYNAMIC;

		return GPBU_STATIC;
	}

	UINT32 BSLFXCompiler::parseFilterMode(FilterValue filter)
	{
		switch (filter)
		{
		case FV_Point:
			return FO_POINT;
		case FV_Linear:
			return FO_LINEAR;
		case FV_Anisotropic:
			return FO_ANISOTROPIC;
		case FV_PointCmp:
			return FO_POINT | FO_USE_COMPARISON;
		case FV_LinearCmp:
			return FO_LINEAR | FO_USE_COMPARISON;
		case FV_AnisotropicCmp:
			return FO_ANISOTROPIC | FO_USE_COMPARISON;
		default:
			break;
		}

		return FO_NONE;
	}

	QueueSortType BSLFXCompiler::parseSortType(QueueSortTypeValue sortType)
	{
		switch (sortType)
		{
		case QST_BackToFront:
			return QueueSortType::BackToFront;
		case QST_FrontToBack:
			return QueueSortType::FrontToBack;
		case QST_None:
			return QueueSortType::None;
		}

		return QueueSortType::None;
	}

	CompareFunction BSLFXCompiler::parseCompFunc(CompFuncValue compFunc)
	{
		switch (compFunc)
		{
		case CFV_Pass:
			return CMPF_ALWAYS_PASS;
		case CFV_Fail:
			return CMPF_ALWAYS_FAIL;
		case CFV_LT:
			return CMPF_LESS;
		case CFV_LTE:
			return CMPF_LESS_EQUAL;
		case CFV_EQ:
			return CMPF_EQUAL;
		case CFV_NEQ:
			return CMPF_NOT_EQUAL;
		case CFV_GT:
			return CMPF_GREATER;
		case CFV_GTE:
			return CMPF_GREATER_EQUAL;
		}

		return CMPF_ALWAYS_PASS;
	}

	TextureAddressingMode BSLFXCompiler::parseAddrMode(AddrModeValue addrMode)
	{
		switch (addrMode)
		{
		case AMV_Wrap:
			return TAM_WRAP;
		case AMV_Mirror:
			return TAM_MIRROR;
		case AMV_Clamp:
			return TAM_CLAMP;
		case AMV_Border:
			return TAM_BORDER;
		}

		return TAM_WRAP;
	}

	BlendFactor BSLFXCompiler::parseBlendFactor(OpValue factor)
	{
		switch (factor)
		{
		case OV_One:
			return BF_ONE;
		case OV_Zero:
			return BF_ZERO;
		case OV_DestColor:
			return BF_DEST_COLOR;
		case OV_SrcColor:
			return BF_SOURCE_COLOR;
		case OV_InvDestColor:
			return BF_INV_DEST_COLOR;
		case OV_InvSrcColor:
			return BF_INV_SOURCE_COLOR;
		case OV_DestAlpha:
			return BF_DEST_ALPHA;
		case OV_SrcAlpha:
			return BF_SOURCE_ALPHA;
		case OV_InvDestAlpha:
			return BF_INV_DEST_ALPHA;
		case OV_InvSrcAlpha:
			return BF_INV_SOURCE_ALPHA;
		default:
			break;
		}

		return BF_ONE;
	}

	BlendOperation BSLFXCompiler::parseBlendOp(BlendOpValue op)
	{
		switch (op)
		{
		case BOV_Add:
			return BO_ADD;
		case BOV_Max:
			return BO_MAX;
		case BOV_Min:
			return BO_MIN;
		case BOV_Subtract:
			return BO_SUBTRACT;
		case BOV_RevSubtract:
			return BO_REVERSE_SUBTRACT;
		}

		return BO_ADD;
	}

	void BSLFXCompiler::parseParamType(ParamType type, bool& isObjType, UINT32& typeId)
	{
		struct ParamData
		{
			UINT32 type;
			bool isObjType;
		};

		static bool initialized = false;
		static ParamData lookup[PT_Count];

		if (!initialized)
		{
			lookup[PT_Float] = { GPDT_FLOAT1, false };
			lookup[PT_Float2] = { GPDT_FLOAT2, false };
			lookup[PT_Float3] = { GPDT_FLOAT3, false };
			lookup[PT_Float4] = { GPDT_FLOAT4, false };

			lookup[PT_Int] = { GPDT_INT1, false };
			lookup[PT_Int2] = { GPDT_INT2, false };
			lookup[PT_Int3] = { GPDT_INT3, false };
			lookup[PT_Int4] = { GPDT_INT4, false };
			lookup[PT_Color] = { GPDT_COLOR, false };

			lookup[PT_Mat2x2] = { GPDT_MATRIX_2X2, false };
			lookup[PT_Mat2x3] = { GPDT_MATRIX_2X3, false };
			lookup[PT_Mat2x4] = { GPDT_MATRIX_2X4, false };

			lookup[PT_Mat3x2] = { GPDT_MATRIX_3X2, false };
			lookup[PT_Mat3x3] = { GPDT_MATRIX_3X3, false };
			lookup[PT_Mat3x4] = { GPDT_MATRIX_3X4, false };

			lookup[PT_Mat4x2] = { GPDT_MATRIX_4X2, false };
			lookup[PT_Mat4x3] = { GPDT_MATRIX_4X3, false };
			lookup[PT_Mat4x4] = { GPDT_MATRIX_4X4, false };

			lookup[PT_Sampler1D] = { GPOT_SAMPLER1D, true };
			lookup[PT_Sampler2D] = { GPOT_SAMPLER2D, true };
			lookup[PT_Sampler3D] = { GPOT_SAMPLER3D, true };
			lookup[PT_SamplerCUBE] = { GPOT_SAMPLERCUBE, true };
			lookup[PT_Sampler2DMS] = { GPOT_SAMPLER2DMS, true };

			lookup[PT_Texture1D] = { GPOT_TEXTURE1D, true };
			lookup[PT_Texture2D] = { GPOT_TEXTURE2D, true };
			lookup[PT_Texture3D] = { GPOT_TEXTURE3D, true };
			lookup[PT_TextureCUBE] = { GPOT_TEXTURECUBE, true };
			lookup[PT_Texture2DMS] = { GPOT_TEXTURE2DMS, true };

			lookup[PT_RWTexture1D] = { GPOT_RWTEXTURE1D, true };
			lookup[PT_RWTexture2D] = { GPOT_RWTEXTURE2D, true };
			lookup[PT_RWTexture3D] = { GPOT_RWTEXTURE3D, true };
			lookup[PT_RWTexture2DMS] = { GPOT_RWTEXTURE2DMS, true };

			lookup[PT_ByteBuffer] = { GPOT_BYTE_BUFFER, true };
			lookup[PT_StructBuffer] = { GPOT_STRUCTURED_BUFFER, true };
			lookup[PT_TypedBufferRW] = { GPOT_RWTYPED_BUFFER, true };
			lookup[PT_ByteBufferRW] = { GPOT_RWBYTE_BUFFER, true };
			lookup[PT_StructBufferRW] = { GPOT_RWSTRUCTURED_BUFFER, true };
			lookup[PT_AppendBuffer] = { GPOT_RWAPPEND_BUFFER, true };
			lookup[PT_ConsumeBuffer] = { GPOT_RWCONSUME_BUFFER, true };

			initialized = true;
		}

		isObjType = lookup[type].isObjType;
		typeId = lookup[type].type;
	}

	StencilOperation BSLFXCompiler::parseStencilOp(OpValue op)
	{
		switch (op)
		{
		case OV_Keep:
			return SOP_KEEP;
		case OV_Zero:
			return SOP_ZERO;
		case OV_Replace:
			return SOP_REPLACE;
		case OV_Incr:
			return SOP_INCREMENT;
		case OV_Decr:
			return SOP_DECREMENT;
		case OV_IncrWrap:
			return SOP_INCREMENT_WRAP;
		case OV_DecrWrap:
			return SOP_DECREMENT_WRAP;
		case OV_Invert:
			return SOP_INVERT;
		default:
			break;
		}

		return SOP_KEEP;
	}

	CullingMode BSLFXCompiler::parseCullMode(CullModeValue cm)
	{
		switch (cm)
		{
		case CMV_None:
			return CULL_NONE;
		case CMV_CW:
			return CULL_CLOCKWISE;
		case CMV_CCW:
			return CULL_COUNTERCLOCKWISE;
		}

		return CULL_COUNTERCLOCKWISE;
	}

	PolygonMode BSLFXCompiler::parseFillMode(FillModeValue fm)
	{
		if (fm == FMV_Wire)
			return PM_WIREFRAME;

		return PM_SOLID;
	}

	void BSLFXCompiler::parseStencilFront(DEPTH_STENCIL_STATE_DESC& desc, ASTFXNode* stencilOpNode)
	{
		if (stencilOpNode == nullptr || stencilOpNode->type != NT_StencilOp)
			return;

		for (int i = 0; i < stencilOpNode->options->count; i++)
		{
			NodeOption* option = &stencilOpNode->options->entries[i];

			switch (option->type)
			{
			case OT_Fail:
				desc.frontStencilFailOp = parseStencilOp((OpValue)option->value.intValue);
				break;
			case OT_ZFail:
				desc.frontStencilZFailOp = parseStencilOp((OpValue)option->value.intValue);
				break;
			case OT_PassOp:
				desc.frontStencilPassOp = parseStencilOp((OpValue)option->value.intValue);
				break;
			case OT_CompareFunc:
				desc.frontStencilComparisonFunc = parseCompFunc((CompFuncValue)option->value.intValue);
				break;
			default:
				break;
			}
		}
	}

	void BSLFXCompiler::parseStencilBack(DEPTH_STENCIL_STATE_DESC& desc, ASTFXNode* stencilOpNode)
	{
		if (stencilOpNode == nullptr || stencilOpNode->type != NT_StencilOp)
			return;

		for (int i = 0; i < stencilOpNode->options->count; i++)
		{
			NodeOption* option = &stencilOpNode->options->entries[i];

			switch (option->type)
			{
			case OT_Fail:
				desc.backStencilFailOp = parseStencilOp((OpValue)option->value.intValue);
				break;
			case OT_ZFail:
				desc.backStencilZFailOp = parseStencilOp((OpValue)option->value.intValue);
				break;
			case OT_PassOp:
				desc.backStencilPassOp = parseStencilOp((OpValue)option->value.intValue);
				break;
			case OT_CompareFunc:
				desc.backStencilComparisonFunc = parseCompFunc((CompFuncValue)option->value.intValue);
				break;
			default:
				break;
			}
		}
	}

	void BSLFXCompiler::parseAddrMode(SAMPLER_STATE_DESC& desc, ASTFXNode* addrModeNode)
	{
		if (addrModeNode == nullptr || addrModeNode->type != NT_AddrMode)
			return;

		for (int i = 0; i < addrModeNode->options->count; i++)
		{
			NodeOption* option = &addrModeNode->options->entries[i];

			switch (option->type)
			{
			case OT_U:
				desc.addressMode.u = parseAddrMode((AddrModeValue)option->value.intValue);
				break;
			case OT_V:
				desc.addressMode.v = parseAddrMode((AddrModeValue)option->value.intValue);
				break;
			case OT_W:
				desc.addressMode.w = parseAddrMode((AddrModeValue)option->value.intValue);
				break;
			default:
				break;
			}
		}
	}

	void BSLFXCompiler::parseColorBlendDef(RENDER_TARGET_BLEND_STATE_DESC& desc, ASTFXNode* blendDefNode)
	{
		if (blendDefNode == nullptr || blendDefNode->type != NT_BlendDef)
			return;

		for (int i = 0; i < blendDefNode->options->count; i++)
		{
			NodeOption* option = &blendDefNode->options->entries[i];

			switch (option->type)
			{
			case OT_Source:
				desc.srcBlend = parseBlendFactor((OpValue)option->value.intValue);
				break;
			case OT_Dest:
				desc.dstBlend = parseBlendFactor((OpValue)option->value.intValue);
				break;
			case OT_Op:
				desc.blendOp = parseBlendOp((BlendOpValue)option->value.intValue);
				break;
			default:
				break;
			}
		}
	}

	void BSLFXCompiler::parseAlphaBlendDef(RENDER_TARGET_BLEND_STATE_DESC& desc, ASTFXNode* blendDefNode)
	{
		if (blendDefNode == nullptr || blendDefNode->type != NT_BlendDef)
			return;

		for (int i = 0; i < blendDefNode->options->count; i++)
		{
			NodeOption* option = &blendDefNode->options->entries[i];

			switch (option->type)
			{
			case OT_Source:
				desc.srcBlendAlpha = parseBlendFactor((OpValue)option->value.intValue);
				break;
			case OT_Dest:
				desc.dstBlendAlpha = parseBlendFactor((OpValue)option->value.intValue);
				break;
			case OT_Op:
				desc.blendOpAlpha = parseBlendOp((BlendOpValue)option->value.intValue);
				break;
			default:
				break;
			}
		}
	}

	void BSLFXCompiler::parseRenderTargetBlendState(BLEND_STATE_DESC& desc, ASTFXNode* targetNode)
	{
		if (targetNode == nullptr || targetNode->type != NT_Target)
			return;

		UINT32 index = 0;

		for (int i = 0; i < targetNode->options->count; i++)
		{
			NodeOption* option = &targetNode->options->entries[i];

			switch (option->type)
			{
			case OT_Index:
				index = option->value.intValue;
				break;
			default:
				break;
			}
		}

		if (index >= BS_MAX_MULTIPLE_RENDER_TARGETS)
			return;

		RENDER_TARGET_BLEND_STATE_DESC& rtDesc = desc.renderTargetDesc[index];
		for (int i = 0; i < targetNode->options->count; i++)
		{
			NodeOption* option = &targetNode->options->entries[i];

			switch (option->type)
			{
			case OT_Blend:
				rtDesc.blendEnable = option->value.intValue > 0;
				break;
			case OT_Color:
				parseColorBlendDef(rtDesc, option->value.nodePtr);
				break;
			case OT_Alpha:
				parseAlphaBlendDef(rtDesc, option->value.nodePtr);
				break;
			case OT_WriteMask:
				rtDesc.renderTargetWriteMask = option->value.intValue;
				break;
			default:
				break;
			}
		}
	}

	bool BSLFXCompiler::parseBlendState(BLEND_STATE_DESC& desc, ASTFXNode* passNode)
	{
		if (passNode == nullptr || (passNode->type != NT_Pass && passNode->type != NT_Technique))
			return false;

		bool isDefault = true;

		for (int i = 0; i < passNode->options->count; i++)
		{
			NodeOption* option = &passNode->options->entries[i];

			switch (option->type)
			{
			case OT_AlphaToCoverage:
				desc.alphaToCoverageEnable = option->value.intValue > 0;
				isDefault = false;
				break;
			case OT_IndependantBlend:
				desc.independantBlendEnable = option->value.intValue > 0;
				isDefault = false;
				break;
			case OT_Target:
				parseRenderTargetBlendState(desc, option->value.nodePtr);
				isDefault = false;
				break;
			default:
				break;
			}
		}

		return !isDefault;
	}

	bool BSLFXCompiler::parseRasterizerState(RASTERIZER_STATE_DESC& desc, ASTFXNode* passNode)
	{
		if (passNode == nullptr || (passNode->type != NT_Pass && passNode->type != NT_Technique))
			return false;

		bool isDefault = true;

		for (int i = 0; i < passNode->options->count; i++)
		{
			NodeOption* option = &passNode->options->entries[i];

			switch (option->type)
			{
			case OT_FillMode:
				desc.polygonMode = parseFillMode((FillModeValue)option->value.intValue);
				isDefault = false;
				break;
			case OT_CullMode:
				desc.cullMode = parseCullMode((CullModeValue)option->value.intValue);
				isDefault = false;
				break;
			case OT_DepthBias:
				desc.depthBias = option->value.floatValue;
				isDefault = false;
				break;
			case OT_SDepthBias:
				desc.slopeScaledDepthBias = option->value.floatValue;
				isDefault = false;
				break;
			case OT_DepthClip:
				desc.depthClipEnable = option->value.intValue > 0;
				isDefault = false;
				break;
			case OT_Scissor:
				desc.scissorEnable = option->value.intValue > 0;
				isDefault = false;
				break;
			case OT_Multisample:
				desc.multisampleEnable = option->value.intValue > 0;
				isDefault = false;
				break;
			case OT_AALine:
				desc.antialiasedLineEnable = option->value.intValue > 0;
				isDefault = false;
				break;
			default:
				break;
			}
		}

		return !isDefault;
	}

	bool BSLFXCompiler::parseDepthStencilState(DEPTH_STENCIL_STATE_DESC& desc, ASTFXNode* passNode)
	{
		if (passNode == nullptr || (passNode->type != NT_Pass && passNode->type != NT_Technique))
			return false;

		bool isDefault = true;

		for (int i = 0; i < passNode->options->count; i++)
		{
			NodeOption* option = &passNode->options->entries[i];

			switch (option->type)
			{
			case OT_DepthRead:
				desc.depthReadEnable = option->value.intValue > 0;
				isDefault = false;
				break;
			case OT_DepthWrite:
				desc.depthWriteEnable = option->value.intValue > 0;
				isDefault = false;
				break;
			case OT_CompareFunc:
				desc.depthComparisonFunc = parseCompFunc((CompFuncValue)option->value.intValue);
				isDefault = false;
				break;
			case OT_Stencil:
				desc.stencilEnable = option->value.intValue > 0;
				isDefault = false;
				break;
			case OT_StencilReadMask:
				desc.stencilReadMask = (UINT8)option->value.intValue;
				isDefault = false;
				break;
			case OT_StencilWriteMask:
				desc.stencilWriteMask = (UINT8)option->value.intValue;
				isDefault = false;
				break;
			case OT_StencilOpFront:
				parseStencilFront(desc, option->value.nodePtr);
				isDefault = false;
				break;
			case OT_StencilOpBack:
				parseStencilBack(desc, option->value.nodePtr);
				isDefault = false;
				break;
			default:
				break;
			}
		}

		return !isDefault;
	}

	SPtr<SamplerState> BSLFXCompiler::parseSamplerState(ASTFXNode* samplerStateNode)
	{
		if (samplerStateNode == nullptr || samplerStateNode->type != NT_SamplerState)
			return nullptr;

		SAMPLER_STATE_DESC desc;
		bool isDefault = true;

		for (int i = 0; i < samplerStateNode->options->count; i++)
		{
			NodeOption* option = &samplerStateNode->options->entries[i];

			switch (option->type)
			{
			case OT_AddrMode:
				parseAddrMode(desc, option->value.nodePtr);
				isDefault = false;
				break;
			case OT_MinFilter:
				desc.minFilter = (FilterOptions)parseFilterMode((FilterValue)option->value.intValue);
				isDefault = false;
				break;
			case OT_MagFilter:
				desc.magFilter = (FilterOptions)parseFilterMode((FilterValue)option->value.intValue);
				isDefault = false;
				break;
			case OT_MipFilter:
				desc.mipFilter = (FilterOptions)parseFilterMode((FilterValue)option->value.intValue);
				isDefault = false;
				break;
			case OT_MaxAniso:
				desc.maxAniso = option->value.intValue;
				isDefault = false;
				break;
			case OT_MipBias:
				desc.mipmapBias = option->value.floatValue;
				isDefault = false;
				break;
			case OT_MipMin:
				desc.mipMin = option->value.floatValue;
				isDefault = false;
				break;
			case OT_MipMax:
				desc.mipMax = option->value.floatValue;
				isDefault = false;
				break;
			case OT_BorderColor:
				desc.borderColor = Color(option->value.matrixValue[0], option->value.matrixValue[1],
					option->value.matrixValue[2], option->value.matrixValue[3]);
				isDefault = false;
				break;
			case OT_CompareFunc:
				desc.comparisonFunc = parseCompFunc((CompFuncValue)option->value.intValue);
				isDefault = false;
				break;
			default:
				break;
			}
		}

		if (isDefault)
			return nullptr;

		return SamplerState::create(desc);
	}

	void BSLFXCompiler::parseCodeBlock(ASTFXNode* codeNode, const Vector<String>& codeBlocks, PassData& passData)
	{
		if (codeNode == nullptr || (codeNode->type != NT_CodeCommon && codeNode->type != NT_CodeVertex && 
			codeNode->type != NT_CodeFragment && codeNode->type != NT_CodeGeometry && codeNode->type != NT_CodeHull &&
			codeNode->type != NT_CodeDomain && codeNode->type != NT_CodeCompute))
		{
			return;
		}

		UINT32 index = (UINT32)-1;
		for (int j = 0; j < codeNode->options->count; j++)
		{
			if (codeNode->options->entries[j].type == OT_Index)
				index = codeNode->options->entries[j].value.intValue;
		}

		if (index != (UINT32)-1 && index < (UINT32)codeBlocks.size())
		{
			switch (codeNode->type)
			{
			case NT_CodeVertex:
				passData.vertexCode += codeBlocks[index];
				break;
			case NT_CodeFragment:
				passData.fragmentCode += codeBlocks[index];
				break;
			case NT_CodeGeometry:
				passData.geometryCode += codeBlocks[index];
				break;
			case NT_CodeHull:
				passData.hullCode += codeBlocks[index];
				break;
			case NT_CodeDomain:
				passData.domainCode += codeBlocks[index];
				break;
			case NT_CodeCompute:
				passData.computeCode += codeBlocks[index];
				break;
			case NT_CodeCommon:
				passData.commonCode += codeBlocks[index];
				break;
			default:
				break;
			}
		}
	}

	void BSLFXCompiler::parsePass(ASTFXNode* passNode, const Vector<String>& codeBlocks, PassData& passData)
	{
		if (passNode == nullptr || passNode->type != NT_Pass)
			return;

		passData.blendIsDefault &= !parseBlendState(passData.blendDesc, passNode);
		passData.rasterizerIsDefault &= !parseRasterizerState(passData.rasterizerDesc, passNode);
		passData.depthStencilIsDefault &= !parseDepthStencilState(passData.depthStencilDesc, passNode);

		for (int i = 0; i < passNode->options->count; i++)
		{
			NodeOption* option = &passNode->options->entries[i];

			switch (option->type)
			{
			case OT_StencilRef:
				passData.stencilRefValue = option->value.intValue;
				break;
			case OT_Code:
				parseCodeBlock(option->value.nodePtr, codeBlocks, passData);
				break;
			default:
				break;
			}
		}
	}

	void BSLFXCompiler::parseTechnique(ASTFXNode* techniqueNode, const Vector<String>& codeBlocks, TechniqueData& techniqueData)
	{
		if (techniqueNode == nullptr || techniqueNode->type != NT_Technique)
			return;

		PassData combinedCommonPassData;

		UINT32 nextPassIdx = 0;
		// Go in reverse because options are added in reverse order during parsing
		for (int i = techniqueNode->options->count - 1; i >= 0; i--)
		{
			NodeOption* option = &techniqueNode->options->entries[i];

			switch (option->type)
			{
			case OT_Pass:
			{
				UINT32 passIdx = nextPassIdx;

				ASTFXNode* passNode = option->value.nodePtr;
				for (int j = 0; j < passNode->options->count; j++)
				{
					NodeOption* passOption = &passNode->options->entries[j];

					switch (passOption->type)
					{
					case OT_Index:
						passIdx = passOption->value.intValue;
						break;
					default:
						break;
					}
				}

				PassData* passData = nullptr;
				for (auto& entry : techniqueData.passes)
				{
					if (entry.seqIdx == passIdx)
						passData = &entry;
				}

				if (passData == nullptr)
				{
					techniqueData.passes.push_back(PassData());
					passData = &techniqueData.passes.back();

					passData->seqIdx = passIdx;
				}

				nextPassIdx = std::max(nextPassIdx, passIdx) + 1;

				passData->vertexCode = combinedCommonPassData.vertexCode + passData->vertexCode;
				passData->fragmentCode = combinedCommonPassData.fragmentCode + passData->fragmentCode;
				passData->geometryCode = combinedCommonPassData.geometryCode + passData->geometryCode;
				passData->hullCode = combinedCommonPassData.hullCode + passData->hullCode;
				passData->domainCode = combinedCommonPassData.domainCode + passData->domainCode;
				passData->computeCode = combinedCommonPassData.computeCode + passData->computeCode;
				passData->commonCode = combinedCommonPassData.commonCode + passData->commonCode;
				
				parsePass(passNode, codeBlocks, *passData);
			}
				break;
			case OT_Code:
			{
				PassData commonPassData;
				parseCodeBlock(option->value.nodePtr, codeBlocks, commonPassData);

				for (auto& passData : techniqueData.passes)
				{
					passData.vertexCode += commonPassData.vertexCode;
					passData.fragmentCode += commonPassData.fragmentCode;
					passData.geometryCode += commonPassData.geometryCode;
					passData.hullCode += commonPassData.hullCode;
					passData.domainCode += commonPassData.domainCode;
					passData.computeCode += commonPassData.computeCode;
					passData.commonCode += commonPassData.commonCode;
				}

				combinedCommonPassData.vertexCode += commonPassData.vertexCode;
				combinedCommonPassData.fragmentCode += commonPassData.fragmentCode;
				combinedCommonPassData.geometryCode += commonPassData.geometryCode;
				combinedCommonPassData.hullCode += commonPassData.hullCode;
				combinedCommonPassData.domainCode += commonPassData.domainCode;
				combinedCommonPassData.computeCode += commonPassData.computeCode;
				combinedCommonPassData.commonCode += commonPassData.commonCode;
			}
				break;
			default:
				break;
			}
		}

		// Parse common pass states
		for (auto& passData : techniqueData.passes)
		{
			passData.blendIsDefault &= !parseBlendState(passData.blendDesc, techniqueNode);
			passData.rasterizerIsDefault &= !parseRasterizerState(passData.rasterizerDesc, techniqueNode);
			passData.depthStencilIsDefault &= !parseDepthStencilState(passData.depthStencilDesc, techniqueNode);
		}
	}

	void BSLFXCompiler::parseParameters(SHADER_DESC& desc, ASTFXNode* parametersNode)
	{
		if (parametersNode == nullptr || parametersNode->type != NT_Parameters)
			return;

		for (int i = 0; i < parametersNode->options->count; i++)
		{
			NodeOption* option = &parametersNode->options->entries[i];

			if (option->type != OT_Parameter)
				continue;

			ASTFXNode* parameter = option->value.nodePtr;

			String name;
			String alias;

			UINT32 typeId = 0;
			bool isObjType = false;
			StringID semantic;
			SPtr<SamplerState> samplerState = nullptr;
			float defaultValue[16];
			HTexture defaultTexture;
			bool hasDefaultValue = false;
			for (int j = 0; j < parameter->options->count; j++)
			{
				NodeOption* paramOption = &parameter->options->entries[j];

				switch (paramOption->type)
				{
				case OT_Identifier:
					name = paramOption->value.strValue;
					break;
				case OT_Alias:
					alias = removeQuotes(paramOption->value.strValue);
					break;
				case OT_ParamType:
					parseParamType((ParamType)paramOption->value.intValue, isObjType, typeId);
					break;
				case OT_ParamValue:
					memcpy(defaultValue, paramOption->value.matrixValue, sizeof(defaultValue));
					hasDefaultValue = true;
					break;
				case OT_ParamStrValue:
				{
					String defaultTextureName = removeQuotes(paramOption->value.strValue);
					defaultTexture = getBuiltinTexture(defaultTextureName);
					hasDefaultValue = true;
				}
					break;
				case OT_Auto:
					semantic = removeQuotes(paramOption->value.strValue);
					break;
				case OT_SamplerState:
					samplerState = parseSamplerState(paramOption->value.nodePtr);
					break;
				default:
					break;
				}
			}

			if (name.empty())
				continue;

			auto addParameter = [&](const String& paramName, const String& gpuVarName)
			{
				if (isObjType)
				{
					GpuParamObjectType objType = (GpuParamObjectType)typeId;

					if (Shader::isSampler(objType))
					{
						if(hasDefaultValue)
							desc.addParameter(paramName, gpuVarName, objType, samplerState, semantic);
						else
							desc.addParameter(paramName, gpuVarName, objType, semantic);
					}
					else if(Shader::isTexture(objType))
					{
						if(hasDefaultValue)
							desc.addParameter(paramName, gpuVarName, objType, defaultTexture, semantic);
						else
							desc.addParameter(paramName, gpuVarName, objType, semantic);
					}
					else
						desc.addParameter(paramName, gpuVarName, objType, semantic);
				}
				else
				{
					if (hasDefaultValue)
						desc.addParameter(paramName, gpuVarName, (GpuParamDataType)typeId, semantic, 1, 0, (UINT8*)defaultValue);
					else
						desc.addParameter(paramName, gpuVarName, (GpuParamDataType)typeId, semantic);
				}
			};

			addParameter(name, name);

			if (!alias.empty())
				addParameter(name, alias);
		}
	}

	void BSLFXCompiler::parseBlocks(SHADER_DESC& desc, ASTFXNode* blocksNode)
	{
		if (blocksNode == nullptr || blocksNode->type != NT_Blocks)
			return;

		for (int i = 0; i < blocksNode->options->count; i++)
		{
			NodeOption* option = &blocksNode->options->entries[i];

			if (option->type != OT_Block)
				continue;

			ASTFXNode* parameter = option->value.nodePtr;

			String name;
			bool shared = false;
			GpuParamBlockUsage usage = GPBU_STATIC;
			StringID semantic;

			for (int j = 0; j < parameter->options->count; j++)
			{
				NodeOption* paramOption = &parameter->options->entries[j];

				switch (paramOption->type)
				{
				case OT_Identifier:
					name = paramOption->value.strValue;
					break;
				case OT_Shared:
					shared = paramOption->value.intValue > 0;
					break;
				case OT_Usage:
					usage = parseBlockUsage((BufferUsageValue)paramOption->value.intValue);
					break;
				case OT_Auto:
					semantic = removeQuotes(paramOption->value.strValue);
					break;
				default:
					break;
				}
			}

			if (name.empty())
				continue;

			desc.setParamBlockAttribs(name, shared, usage, semantic);
		}
	}
	
	BSLFXCompileResult BSLFXCompiler::parseShader(const String& name, ParseState* parseState, Vector<String>& codeBlocks)
	{
		BSLFXCompileResult output;

		if (parseState->rootNode == nullptr || parseState->rootNode->type != NT_Shader)
		{
			parseStateDelete(parseState);

			output.errorMessage = "Root not is null or not a shader.";
			return output;
		}

		SHADER_DESC shaderDesc;
		Vector<pair<ASTFXNode*, TechniqueData>> techniqueData;

		// Go in reverse because options are added in reverse order during parsing
		for (int i = parseState->rootNode->options->count - 1; i >= 0; i--)
		{
			NodeOption* option = &parseState->rootNode->options->entries[i];

			switch (option->type)
			{
			case OT_Separable:
				shaderDesc.separablePasses = option->value.intValue > 1;
				break;
			case OT_Sort:
				shaderDesc.queueSortType = parseSortType((QueueSortTypeValue)option->value.intValue);
				break;
			case OT_Priority:
				shaderDesc.queuePriority = option->value.intValue;
				break;
			case OT_Transparent:
				shaderDesc.flags |= (UINT32)ShaderFlags::Transparent;
				break;
			case OT_Technique:
			{
				TechniqueMetaData metaData = parseTechniqueMetaData(option->value.nodePtr);

				techniqueData.push_back(std::make_pair(option->value.nodePtr, TechniqueData()));
				TechniqueData& data = techniqueData.back().second;
				data.metaData = metaData;

				if (data.metaData.baseName.empty())
				{
					std::function<bool(TechniqueMetaData&)> parseInherited = [&](TechniqueMetaData& metaData)
					{
						for (auto riter = metaData.inherits.rbegin(); riter != metaData.inherits.rend(); ++riter)
						{
							const String& inherits = *riter;

							bool foundBase = false;
							for (auto& entry : techniqueData)
							{
								if (entry.second.metaData.baseName == inherits)
								{
									bool matches = entry.second.metaData.language == metaData.language || entry.second.metaData.language == "Any";
									matches &= entry.second.metaData.renderer == metaData.renderer || entry.second.metaData.renderer == RendererAny;

									if (matches)
									{
										if (!parseInherited(entry.second.metaData))
											return false;

										parseTechnique(entry.first, codeBlocks, data);
										foundBase = true;
										break;
									}
								}
							}

							if (!foundBase)
							{
								output.errorMessage = "Base technique \"" + inherits + "\" cannot be found.";
								return false;
							}
						}

						return true;
					};

					if (!parseInherited(metaData))
					{
						parseStateDelete(parseState);
						return output;
					}

					parseTechnique(option->value.nodePtr, codeBlocks, data);
				}
				break;
			}
			case OT_Parameters:
				parseParameters(shaderDesc, option->value.nodePtr);
				break;
			case OT_Blocks:
				parseBlocks(shaderDesc, option->value.nodePtr);
				break;
			default:
				break;
			}
		}

		Vector<SPtr<Technique>> techniques;
		for(auto& entry : techniqueData)
		{
			const TechniqueMetaData& metaData = entry.second.metaData;
			if (!metaData.baseName.empty())
				continue;

			Map<UINT32, SPtr<Pass>, std::greater<UINT32>> passes;
			for (auto& passData : entry.second.passes)
			{
				PASS_DESC passDesc;

				if (!passData.blendIsDefault)
					passDesc.blendState = BlendState::create(passData.blendDesc);

				if (!passData.rasterizerIsDefault)
					passDesc.rasterizerState = RasterizerState::create(passData.rasterizerDesc);

				if (!passData.depthStencilIsDefault)
					passDesc.depthStencilState = DepthStencilState::create(passData.depthStencilDesc);

				GPU_PROGRAM_DESC desc;
				desc.entryPoint = "main";
				desc.language = metaData.language;

				if (!passData.vertexCode.empty())
				{
					desc.source = passData.commonCode + passData.vertexCode;
					desc.type = GPT_VERTEX_PROGRAM;
					desc.profile = getProfile(metaData.renderAPI, GPT_VERTEX_PROGRAM);

					passDesc.vertexProgram = GpuProgram::create(desc);
				}

				if (!passData.fragmentCode.empty())
				{
					desc.source = passData.commonCode + passData.fragmentCode;
					desc.type = GPT_FRAGMENT_PROGRAM;
					desc.profile = getProfile(metaData.renderAPI, GPT_FRAGMENT_PROGRAM);

					passDesc.fragmentProgram = GpuProgram::create(desc);
				}

				if (!passData.geometryCode.empty())
				{
					desc.source = passData.commonCode + passData.geometryCode;
					desc.type = GPT_GEOMETRY_PROGRAM;
					desc.profile = getProfile(metaData.renderAPI, GPT_GEOMETRY_PROGRAM);

					passDesc.geometryProgram = GpuProgram::create(desc);
				}

				if (!passData.hullCode.empty())
				{
					desc.source = passData.commonCode + passData.hullCode;
					desc.type = GPT_HULL_PROGRAM;
					desc.profile = getProfile(metaData.renderAPI, GPT_HULL_PROGRAM);

					passDesc.hullProgram = GpuProgram::create(desc);
				}

				if (!passData.domainCode.empty())
				{
					desc.source = passData.commonCode + passData.domainCode;
					desc.type = GPT_DOMAIN_PROGRAM;
					desc.profile = getProfile(metaData.renderAPI, GPT_DOMAIN_PROGRAM);

					passDesc.domainProgram = GpuProgram::create(desc);
				}

				if (!passData.computeCode.empty())
				{
					desc.source = passData.commonCode + passData.computeCode;
					desc.type = GPT_COMPUTE_PROGRAM;
					desc.profile = getProfile(metaData.renderAPI, GPT_COMPUTE_PROGRAM);

					passDesc.computeProgram = GpuProgram::create(desc);
				}

				passDesc.stencilRefValue = passData.stencilRefValue;

				SPtr<Pass> pass = Pass::create(passDesc);
				if (pass != nullptr)
					passes[passData.seqIdx] = pass;
			}

			Vector<SPtr<Pass>> orderedPasses;
			for (auto& KVP : passes)
				orderedPasses.push_back(KVP.second);

			if (orderedPasses.size() > 0)
			{
				SPtr<Technique> technique = Technique::create(metaData.renderAPI, metaData.renderer,
					metaData.tags, orderedPasses);
				techniques.push_back(technique);
			}
		}

		Vector<String> includes;
		IncludeLink* includeLink = parseState->includes;
		while(includeLink != nullptr)
		{
			String includeFilename = includeLink->data->filename;

			auto iterFind = std::find(includes.begin(), includes.end(), includeFilename);
			if (iterFind == includes.end())
				includes.push_back(includeFilename);

			includeLink = includeLink->next;
		}

		parseStateDelete(parseState);

		output.shader = Shader::_createPtr(name, shaderDesc, techniques);
		output.shader->setIncludeFiles(includes);

		return output;
	}

	String BSLFXCompiler::removeQuotes(const char* input)
	{
		UINT32 len = (UINT32)strlen(input);
		String output(len - 2, ' ');

		for (UINT32 i = 0; i < (len - 2); i++)
			output[i] = input[i + 1];

		return output;
	}

	GpuProgramProfile BSLFXCompiler::getProfile(const StringID& renderAPI, GpuProgramType type)
	{
		StringID target = renderAPI;
		if (target == RenderAPIAny)
			target = RenderAPICore::instance().getName();

		if (target == RenderAPIDX11 || target == RenderAPIOpenGL)
		{
			switch (type)
			{
			case GPT_VERTEX_PROGRAM:
				return GPP_VS_5_0;
			case GPT_FRAGMENT_PROGRAM:
				return GPP_FS_5_0;
			case GPT_GEOMETRY_PROGRAM:
				return GPP_GS_5_0;
			case GPT_HULL_PROGRAM:
				return GPP_HS_5_0;
			case GPT_DOMAIN_PROGRAM:
				return GPP_DS_5_0;
			case GPT_COMPUTE_PROGRAM:
				return GPP_CS_5_0;
			}
		}
		else if (target == RenderAPIDX9)
		{
			switch (type)
			{
			case GPT_VERTEX_PROGRAM:
				return GPP_VS_3_0;
			case GPT_FRAGMENT_PROGRAM:
				return GPP_FS_3_0;
			default:
				break;
			}
		}

		return GPP_NONE;
	}

	HTexture BSLFXCompiler::getBuiltinTexture(const String& name)
	{
		if (StringUtil::compare(name, String("white"), false) == 0)
			return BuiltinResources::getTexture(BuiltinTexture::White);
		else if (StringUtil::compare(name, String("black"), false) == 0)
			return BuiltinResources::getTexture(BuiltinTexture::Black);
		else if (StringUtil::compare(name, String("normal"), false) == 0)
			return BuiltinResources::getTexture(BuiltinTexture::Normal);

		return HTexture();
	}
}