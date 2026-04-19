//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DBSLParser.h"
#include "Material/B3DShader.h"
#include "Material/B3DPass.h"
#include "Debug/B3DDebug.h"
#include "Math/B3DMatrix4.h"
#include "Resources/B3DBuiltinResources.h"
#include "Material/B3DShaderVariation.h"
#include "Renderer/B3DRenderer.h"

extern "C" {
#include "B3DMMAlloc.h"

#define YY_NO_UNISTD_H 1
#include "B3DParserFX.h"
#include "B3DLexerFX.h"
}

using namespace std;
using namespace b3d;

// Print out the FX AST, only for debug purposes
void SLFXDebugPrint(ASTFXNode* node, String indent)
{
	B3D_LOG(Info, LogBSLCompiler, indent + "NODE {0}", node->Type);

	for(int i = 0; i < node->Options->Count; i++)
	{
		OptionDataType odt = OPTION_LOOKUP[(int)node->Options->Entries[i].Type].DataType;
		if(odt == ODT_Complex)
		{
			B3D_LOG(Info, LogBSLCompiler, "{0}{1}. {2}", indent, i, node->Options->Entries[i].Type);
			SLFXDebugPrint(node->Options->Entries[i].Value.NodePtr, indent + "\t");
			continue;
		}

		String value;
		switch(odt)
		{
		case ODT_Bool:
			value = ToString(node->Options->Entries[i].Value.IntValue != 0);
			break;
		case ODT_Int:
			value = ToString(node->Options->Entries[i].Value.IntValue);
			break;
		case ODT_Float:
			value = ToString(node->Options->Entries[i].Value.FloatValue);
			break;
		case ODT_String:
			value = node->Options->Entries[i].Value.StrValue;
			break;
		case ODT_Matrix:
			{
				Matrix4 mat4 = *(Matrix4*)(node->Options->Entries[i].Value.MatrixValue);
				value = ToString(mat4);
			}
			break;
		default:
			break;
		}

		B3D_LOG(Info, LogBSLCompiler, "{0}{1}. {2} = {3}", indent, i, node->Options->Entries[i].Type, value);
	}
}

ShaderCompilerResult BSLParser::RunParser(ParseState* parseState, const char* source, const UnorderedMap<String, String>& defines)
{
	for(auto& define : defines)
	{
		if(define.first.size() == 0)
			continue;

		AddDefine(parseState, define.first.c_str());

		if(define.second.size() > 0)
			AddDefineExpr(parseState, define.second.c_str());
	}

	yyscan_t scanner;
	YY_BUFFER_STATE state;

	ShaderCompilerResult parseResult;
	if(yylex_init_extra(parseState, &scanner))
	{
		parseResult.ErrorMessage = "Internal error: Lexer failed to initialize.";
		return parseResult;
	}

	// If debug output from lexer is needed uncomment this and add %debug option to lexer file
	// yyset_debug(true, scanner);

	// If debug output from parser is needed uncomment this and add %debug option to parser file
	// yydebug = true;

	state = yy_scan_string(source, scanner);

	bool parsingFailed = yyparse(parseState, scanner) > 0;

	if(parseState->HasError > 0)
	{
		parseResult.ErrorMessage = parseState->ErrorMessage;
		parseResult.ErrorLine = parseState->ErrorLine;
		parseResult.ErrorColumn = parseState->ErrorColumn;

		if(parseState->ErrorFile != nullptr)
			parseResult.ErrorFile = parseState->ErrorFile;

		goto cleanup;
	}
	else if(parsingFailed)
	{
		parseResult.ErrorMessage = "Internal error: Parsing failed.";
		goto cleanup;
	}

cleanup:
	yy_delete_buffer(state, scanner);
	yylex_destroy(scanner);

	return parseResult;
}

BSLParsedShaderMetaData BSLParser::ParseShaderMetaData(ASTFXNode* shader)
{
	BSLParsedShaderMetaData metaData;

	metaData.Language = "hlsl";
	metaData.IsMixin = shader->Type == NT_Mixin;

	for(int i = 0; i < shader->Options->Count; i++)
	{
		NodeOption* option = &shader->Options->Entries[i];

		switch(option->Type)
		{
		case OT_Tags:
			{
				ASTFXNode* tagsNode = option->Value.NodePtr;
				for(int j = 0; j < tagsNode->Options->Count; j++)
				{
					NodeOption* tagOption = &tagsNode->Options->Entries[j];

					if(tagOption->Type == OT_TagValue)
						metaData.Tags.push_back(RemoveQuotes(tagOption->Value.StrValue));
				}
			}
			break;
		case OT_Variations:
			{
				ASTFXNode* variationsNode = option->Value.NodePtr;
				for(int j = 0; j < variationsNode->Options->Count; j++)
				{
					NodeOption* variationOption = &variationsNode->Options->Entries[j];

					if(variationOption->Type == OT_Variation)
						ParseVariations(metaData, variationOption->Value.NodePtr);
				}
			}
			break;
		case OT_Identifier:
			metaData.Name = option->Value.StrValue;
			break;
		case OT_Mixin:
			metaData.Includes.push_back(option->Value.StrValue);
			break;
		default:
			break;
		}
	}

	return metaData;
}

template<bool IsRenderProxy>
ShaderCompilerResult BSLParser::TParseMetaDataAndOptions(ASTFXNode* rootNode, Vector<std::pair<ASTFXNode*, BSLParsedShaderMetaData>>& shaderMetaData, CoreVariantType<ShaderCreateInformation, IsRenderProxy>& shaderCreateInformation)
{
	ShaderCompilerResult parseResult;

	// Only enable for debug purposes
	// SLFXDebugPrint(parseState->rootNode, "");

	if(rootNode == nullptr || rootNode->Type != NT_Root)
	{
		parseResult.ErrorMessage = "Root is null or not a shader.";
		return parseResult;
	}

	// Parse global shader options & shader meta-data
	//// Go in reverse because options are added in reverse order during parsing
	for(int i = rootNode->Options->Count - 1; i >= 0; i--)
	{
		NodeOption* option = &rootNode->Options->Entries[i];

		switch(option->Type)
		{
		case OT_Options:
			TParseOptions<IsRenderProxy>(option->Value.NodePtr, shaderCreateInformation);
			break;
		case OT_Shader:
			{
				// We initially parse only meta-data, so we can handle out-of-order mixin/shader definitions
				BSLParsedShaderMetaData metaData = ParseShaderMetaData(option->Value.NodePtr);
				shaderMetaData.push_back(std::make_pair(option->Value.NodePtr, metaData));

				break;
			}
		case OT_SubShader:
			// No longer supported
			break;
		default:
			break;
		}
	}

	return parseResult;
}

void BSLParser::ParseVariations(BSLParsedShaderMetaData& metaData, ASTFXNode* variations)
{
	B3D_ASSERT(variations->Type == NT_Variation);

	BSLParsedVariationData variationData;
	for(int i = 0; i < variations->Options->Count; i++)
	{
		NodeOption* option = &variations->Options->Entries[i];

		switch(option->Type)
		{
		case OT_Identifier:
			variationData.Identifier = option->Value.StrValue;
			break;
		case OT_VariationOption:
			variationData.Values.push_back(ParseVariationOption(option->Value.NodePtr));
			break;
		case OT_Attributes:
			{
				AttributeData attribs = ParseAttributes(option->Value.NodePtr);

				for(auto& entry : attribs.Attributes)
				{
					if(entry.first == OT_AttrName)
						variationData.Name = entry.second;
					else if(entry.first == OT_AttrShow)
						variationData.Internal = false;
				}
			}
		default:
			break;
		}
	}

	if(!variationData.Identifier.empty())
		metaData.Variations.push_back(variationData);
}

BSLParsedVariationOption BSLParser::ParseVariationOption(ASTFXNode* variationOption)
{
	B3D_ASSERT(variationOption->Type == NT_VariationOption);

	BSLParsedVariationOption output;
	for(int i = 0; i < variationOption->Options->Count; i++)
	{
		NodeOption* option = &variationOption->Options->Entries[i];

		switch(option->Type)
		{
		case OT_VariationValue:
			output.Value = option->Value.IntValue;
			break;
		case OT_Attributes:
			{
				AttributeData attribs = ParseAttributes(option->Value.NodePtr);

				for(auto& entry : attribs.Attributes)
				{
					if(entry.first == OT_AttrName)
						output.Name = entry.second;
				}
			}
		default:
			break;
		}
	}

	return output;
}

BSLParser::AttributeData BSLParser::ParseAttributes(ASTFXNode* attributes)
{
	B3D_ASSERT(attributes->Type == NT_Attributes);

	AttributeData attributeData;
	for(int i = 0; i < attributes->Options->Count; i++)
	{
		NodeOption* option = &attributes->Options->Entries[i];

		switch(option->Type)
		{
		case OT_AttrName:
			attributeData.Attributes.push_back(std::pair<i32, String>(OT_AttrName, RemoveQuotes(option->Value.StrValue)));
			break;
		case OT_AttrShow:
			attributeData.Attributes.push_back(std::pair<i32, String>(OT_AttrShow, ""));
			break;
		default:
			break;
		}
	}

	return attributeData;
}

QueueSortType BSLParser::ParseSortType(CullAndSortModeValue sortType)
{
	switch(sortType)
	{
	case CASV_BackToFront:
		return QueueSortType::BackToFront;
	case CASV_FrontToBack:
		return QueueSortType::FrontToBack;
	case CASV_None:
		return QueueSortType::None;
	default:
		break;
	}

	return QueueSortType::None;
}

CompareFunction BSLParser::ParseCompFunc(CompFuncValue compFunc)
{
	switch(compFunc)
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

BlendFactor BSLParser::ParseBlendFactor(OpValue factor)
{
	switch(factor)
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

BlendOperation BSLParser::ParseBlendOp(BlendOpValue op)
{
	switch(op)
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

StencilOperation BSLParser::ParseStencilOp(OpValue op)
{
	switch(op)
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

CullingMode BSLParser::ParseCullMode(CullAndSortModeValue cm)
{
	switch(cm)
	{
	case CASV_None:
		return CULL_NONE;
	case CASV_CW:
		return CULL_CLOCKWISE;
	case CASV_CCW:
		return CULL_COUNTERCLOCKWISE;
	default:
		break;
	}

	return CULL_COUNTERCLOCKWISE;
}

PolygonMode BSLParser::ParseFillMode(FillModeValue fm)
{
	if(fm == FMV_Wire)
		return PM_WIREFRAME;

	return PM_SOLID;
}

void BSLParser::ParseStencilFront(DepthStencilStateInformation& desc, ASTFXNode* stencilOpNode)
{
	if(stencilOpNode == nullptr || stencilOpNode->Type != NT_StencilOp)
		return;

	for(int i = 0; i < stencilOpNode->Options->Count; i++)
	{
		NodeOption* option = &stencilOpNode->Options->Entries[i];

		switch(option->Type)
		{
		case OT_Fail:
			desc.FrontStencilFailOp = ParseStencilOp((OpValue)option->Value.IntValue);
			break;
		case OT_ZFail:
			desc.FrontStencilZFailOp = ParseStencilOp((OpValue)option->Value.IntValue);
			break;
		case OT_PassOp:
			desc.FrontStencilPassOp = ParseStencilOp((OpValue)option->Value.IntValue);
			break;
		case OT_CompareFunc:
			desc.FrontStencilComparisonFunc = ParseCompFunc((CompFuncValue)option->Value.IntValue);
			break;
		default:
			break;
		}
	}
}

void BSLParser::ParseStencilBack(DepthStencilStateInformation& desc, ASTFXNode* stencilOpNode)
{
	if(stencilOpNode == nullptr || stencilOpNode->Type != NT_StencilOp)
		return;

	for(int i = 0; i < stencilOpNode->Options->Count; i++)
	{
		NodeOption* option = &stencilOpNode->Options->Entries[i];

		switch(option->Type)
		{
		case OT_Fail:
			desc.BackStencilFailOp = ParseStencilOp((OpValue)option->Value.IntValue);
			break;
		case OT_ZFail:
			desc.BackStencilZFailOp = ParseStencilOp((OpValue)option->Value.IntValue);
			break;
		case OT_PassOp:
			desc.BackStencilPassOp = ParseStencilOp((OpValue)option->Value.IntValue);
			break;
		case OT_CompareFunc:
			desc.BackStencilComparisonFunc = ParseCompFunc((CompFuncValue)option->Value.IntValue);
			break;
		default:
			break;
		}
	}
}

void BSLParser::ParseColorBlendDef(RenderTargetBlendStateInformation& desc, ASTFXNode* blendDefNode)
{
	if(blendDefNode == nullptr || blendDefNode->Type != NT_BlendDef)
		return;

	for(int i = 0; i < blendDefNode->Options->Count; i++)
	{
		NodeOption* option = &blendDefNode->Options->Entries[i];

		switch(option->Type)
		{
		case OT_Source:
			desc.ColorSourceFactor = ParseBlendFactor((OpValue)option->Value.IntValue);
			break;
		case OT_Dest:
			desc.ColorDestinationFactor = ParseBlendFactor((OpValue)option->Value.IntValue);
			break;
		case OT_Op:
			desc.ColorBlendOperation = ParseBlendOp((BlendOpValue)option->Value.IntValue);
			break;
		default:
			break;
		}
	}
}

void BSLParser::ParseAlphaBlendDef(RenderTargetBlendStateInformation& desc, ASTFXNode* blendDefNode)
{
	if(blendDefNode == nullptr || blendDefNode->Type != NT_BlendDef)
		return;

	for(int i = 0; i < blendDefNode->Options->Count; i++)
	{
		NodeOption* option = &blendDefNode->Options->Entries[i];

		switch(option->Type)
		{
		case OT_Source:
			desc.AlphaSourceFactor = ParseBlendFactor((OpValue)option->Value.IntValue);
			break;
		case OT_Dest:
			desc.AlphaDestinationFactor = ParseBlendFactor((OpValue)option->Value.IntValue);
			break;
		case OT_Op:
			desc.AlphaBlendOperation = ParseBlendOp((BlendOpValue)option->Value.IntValue);
			break;
		default:
			break;
		}
	}
}

void BSLParser::ParseRenderTargetBlendState(BlendStateInformation& desc, ASTFXNode* targetNode, u32& index)
{
	if(targetNode == nullptr || targetNode->Type != NT_Target)
		return;

	for(int i = 0; i < targetNode->Options->Count; i++)
	{
		NodeOption* option = &targetNode->Options->Entries[i];

		switch(option->Type)
		{
		case OT_Index:
			index = option->Value.IntValue;
			break;
		default:
			break;
		}
	}

	if(index >= B3D_MAXIMUM_RENDER_TARGET_COUNT)
		return;

	RenderTargetBlendStateInformation& rtDesc = desc.RenderTargets[index];
	for(int i = 0; i < targetNode->Options->Count; i++)
	{
		NodeOption* option = &targetNode->Options->Entries[i];

		switch(option->Type)
		{
		case OT_Enabled:
			rtDesc.BlendEnable = option->Value.IntValue > 0;
			break;
		case OT_Color:
			ParseColorBlendDef(rtDesc, option->Value.NodePtr);
			break;
		case OT_Alpha:
			ParseAlphaBlendDef(rtDesc, option->Value.NodePtr);
			break;
		case OT_WriteMask:
			rtDesc.RenderTargetWriteMask = option->Value.IntValue;
			break;
		default:
			break;
		}
	}

	index++;
}

bool BSLParser::ParseBlendState(BSLParsedShaderPassData& desc, ASTFXNode* blendNode)
{
	if(blendNode == nullptr || blendNode->Type != NT_Blend)
		return false;

	bool isDefault = true;
	TInlineArray<ASTFXNode*, 8> targets;

	for(int i = 0; i < blendNode->Options->Count; i++)
	{
		NodeOption* option = &blendNode->Options->Entries[i];

		switch(option->Type)
		{
		case OT_AlphaToCoverage:
			desc.BlendStateInformation.EnableAlphaToCoverage = option->Value.IntValue > 0;
			isDefault = false;
			break;
		case OT_IndependantBlend:
			desc.BlendStateInformation.EnableIndependantBlend = option->Value.IntValue > 0;
			isDefault = false;
			break;
		case OT_Target:
			targets.Add(option->Value.NodePtr);
			isDefault = false;
			break;
		default:
			break;
		}
	}

	// Parse targets in reverse as their order matters and we want to visit them in the top-down order as defined in
	// the source code
	u32 index = 0;
	for(auto iter = targets.rbegin(); iter != targets.rend(); ++iter)
		ParseRenderTargetBlendState(desc.BlendStateInformation, *iter, index);

	return !isDefault;
}

bool BSLParser::ParseRasterizerState(BSLParsedShaderPassData& desc, ASTFXNode* rasterNode)
{
	if(rasterNode == nullptr || rasterNode->Type != NT_Raster)
		return false;

	bool isDefault = true;

	for(int i = 0; i < rasterNode->Options->Count; i++)
	{
		NodeOption* option = &rasterNode->Options->Entries[i];

		switch(option->Type)
		{
		case OT_FillMode:
			desc.RasterizerStateInformation.PolygonMode = ParseFillMode((FillModeValue)option->Value.IntValue);
			isDefault = false;
			break;
		case OT_CullMode:
			desc.RasterizerStateInformation.CullMode = ParseCullMode((CullAndSortModeValue)option->Value.IntValue);
			isDefault = false;
			break;
		case OT_DepthBias:
			desc.RasterizerStateInformation.DepthBias = option->Value.FloatValue;
			isDefault = false;
			break;
		case OT_SDepthBias:
			desc.RasterizerStateInformation.SlopeScaledDepthBias = option->Value.FloatValue;
			isDefault = false;
			break;
		case OT_DepthClip:
			desc.RasterizerStateInformation.DepthClipEnable = option->Value.IntValue > 0;
			isDefault = false;
			break;
		case OT_Scissor:
			desc.RasterizerStateInformation.ScissorEnable = option->Value.IntValue > 0;
			isDefault = false;
			break;
		case OT_Multisample:
			desc.RasterizerStateInformation.MultisampleEnable = option->Value.IntValue > 0;
			isDefault = false;
			break;
		case OT_AALine:
			desc.RasterizerStateInformation.AntialiasedLineEnable = option->Value.IntValue > 0;
			isDefault = false;
			break;
		default:
			break;
		}
	}

	return !isDefault;
}

bool BSLParser::ParseDepthState(BSLParsedShaderPassData& passData, ASTFXNode* depthNode)
{
	if(depthNode == nullptr || depthNode->Type != NT_Depth)
		return false;

	bool isDefault = true;

	for(int i = 0; i < depthNode->Options->Count; i++)
	{
		NodeOption* option = &depthNode->Options->Entries[i];

		switch(option->Type)
		{
		case OT_DepthRead:
			passData.DepthStencilStateInformation.DepthReadEnable = option->Value.IntValue > 0;
			isDefault = false;
			break;
		case OT_DepthWrite:
			passData.DepthStencilStateInformation.DepthWriteEnable = option->Value.IntValue > 0;
			isDefault = false;
			break;
		case OT_CompareFunc:
			passData.DepthStencilStateInformation.DepthComparisonFunc = ParseCompFunc((CompFuncValue)option->Value.IntValue);
			isDefault = false;
			break;
		default:
			break;
		}
	}

	return !isDefault;
}

bool BSLParser::ParseStencilState(BSLParsedShaderPassData& passData, ASTFXNode* stencilNode)
{
	if(stencilNode == nullptr || stencilNode->Type != NT_Stencil)
		return false;

	bool isDefault = true;

	for(int i = 0; i < stencilNode->Options->Count; i++)
	{
		NodeOption* option = &stencilNode->Options->Entries[i];

		switch(option->Type)
		{
		case OT_Enabled:
			passData.DepthStencilStateInformation.StencilEnable = option->Value.IntValue > 0;
			isDefault = false;
			break;
		case OT_StencilReadMask:
			passData.DepthStencilStateInformation.StencilReadMask = (u8)option->Value.IntValue;
			isDefault = false;
			break;
		case OT_StencilWriteMask:
			passData.DepthStencilStateInformation.StencilWriteMask = (u8)option->Value.IntValue;
			isDefault = false;
			break;
		case OT_StencilOpFront:
			ParseStencilFront(passData.DepthStencilStateInformation, option->Value.NodePtr);
			isDefault = false;
			break;
		case OT_StencilOpBack:
			ParseStencilBack(passData.DepthStencilStateInformation, option->Value.NodePtr);
			isDefault = false;
			break;
		case OT_StencilRef:
			passData.StencilReferenceValue = option->Value.IntValue;
			break;
		default:
			break;
		}
	}

	return !isDefault;
}

void BSLParser::ParseCodeBlock(ASTFXNode* codeNode, const Vector<String>& codeBlocks, BSLParsedShaderPassData& passData)
{
	if(codeNode == nullptr || (codeNode->Type != NT_Code))
	{
		return;
	}

	u32 index = (u32)-1;
	for(int j = 0; j < codeNode->Options->Count; j++)
	{
		if(codeNode->Options->Entries[j].Type == OT_Index)
			index = codeNode->Options->Entries[j].Value.IntValue;
	}

	if(index != (u32)-1 && index < (u32)codeBlocks.size())
	{
		passData.Code += codeBlocks[index];
	}
}

void BSLParser::ParsePass(ASTFXNode* passNode, const Vector<String>& codeBlocks, BSLParsedShaderPassData& passData)
{
	if(passNode == nullptr || passNode->Type != NT_Pass)
		return;

	for(int i = 0; i < passNode->Options->Count; i++)
	{
		NodeOption* option = &passNode->Options->Entries[i];

		switch(option->Type)
		{
		case OT_Blend:
			passData.BlendStateIsDefault &= !ParseBlendState(passData, option->Value.NodePtr);
			break;
		case OT_Raster:
			passData.RasterizerStateIsDefault &= !ParseRasterizerState(passData, option->Value.NodePtr);
			break;
		case OT_Depth:
			passData.DepthStencilStateIsDefault &= !ParseDepthState(passData, option->Value.NodePtr);
			break;
		case OT_Stencil:
			passData.DepthStencilStateIsDefault &= !ParseStencilState(passData, option->Value.NodePtr);
			break;
		case OT_Code:
			ParseCodeBlock(option->Value.NodePtr, codeBlocks, passData);
			break;
		default:
			break;
		}
	}
}

void BSLParser::ParseShader(ASTFXNode* shaderNode, const Vector<String>& codeBlocks, BSLParsedShaderData& shaderData)
{
	if(shaderNode == nullptr || (shaderNode->Type != NT_Shader && shaderNode->Type != NT_Mixin))
		return;

	// There must always be at least one pass
	if(shaderData.Passes.empty())
	{
		shaderData.Passes.push_back(BSLParsedShaderPassData());
		shaderData.Passes.back().SequentialIndex = 0;
	}

	BSLParsedShaderPassData combinedCommonPassData;

	u32 nextPassIdx = 0;
	// Go in reverse because options are added in reverse order during parsing
	for(int i = shaderNode->Options->Count - 1; i >= 0; i--)
	{
		NodeOption* option = &shaderNode->Options->Entries[i];

		switch(option->Type)
		{
		case OT_Pass:
			{
				u32 passIdx = nextPassIdx;
				BSLParsedShaderPassData* passData = nullptr;
				for(auto& entry : shaderData.Passes)
				{
					if(entry.SequentialIndex == passIdx)
						passData = &entry;
				}

				if(passData == nullptr)
				{
					shaderData.Passes.push_back(BSLParsedShaderPassData());
					passData = &shaderData.Passes.back();

					passData->SequentialIndex = passIdx;
				}

				nextPassIdx = std::max(nextPassIdx, passIdx) + 1;
				passData->Code = combinedCommonPassData.Code + passData->Code;

				ASTFXNode* passNode = option->Value.NodePtr;
				ParsePass(passNode, codeBlocks, *passData);
			}
			break;
		case OT_Code:
			{
				BSLParsedShaderPassData commonPassData;
				ParseCodeBlock(option->Value.NodePtr, codeBlocks, commonPassData);

				for(auto& passData : shaderData.Passes)
					passData.Code += commonPassData.Code;

				combinedCommonPassData.Code += commonPassData.Code;
			}
			break;
		case OT_FeatureSet:
			shaderData.MetaData.FeatureSet = option->Value.StrValue;
			break;
		default:
			break;
		}
	}

	// Parse common pass states
	for(int i = 0; i < shaderNode->Options->Count; i++)
	{
		NodeOption* option = &shaderNode->Options->Entries[i];

		switch(option->Type)
		{
		case OT_Blend:
			for(auto& passData : shaderData.Passes)
				passData.BlendStateIsDefault &= !ParseBlendState(passData, option->Value.NodePtr);
			break;
		case OT_Raster:
			for(auto& passData : shaderData.Passes)
				passData.RasterizerStateIsDefault &= !ParseRasterizerState(passData, option->Value.NodePtr);
			break;
		case OT_Depth:
			for(auto& passData : shaderData.Passes)
				passData.DepthStencilStateIsDefault &= !ParseDepthState(passData, option->Value.NodePtr);
			break;
		case OT_Stencil:
			for(auto& passData : shaderData.Passes)
				passData.DepthStencilStateIsDefault &= !ParseStencilState(passData, option->Value.NodePtr);
			break;
		default:
			break;
		}
	}
}

template<bool IsRenderProxy>
void BSLParser::TParseOptions(ASTFXNode* optionsNode, CoreVariantType<ShaderCreateInformation, IsRenderProxy>& outShaderCreateInformation)
{
	if(optionsNode == nullptr || optionsNode->Type != NT_Options)
		return;

	for(int i = optionsNode->Options->Count - 1; i >= 0; i--)
	{
		NodeOption* option = &optionsNode->Options->Entries[i];

		switch(option->Type)
		{
		case OT_Separable:
			outShaderCreateInformation.SeparablePasses = option->Value.IntValue > 1;
			break;
		case OT_Sort:
			outShaderCreateInformation.QueueSortType = ParseSortType((CullAndSortModeValue)option->Value.IntValue);
			break;
		case OT_Priority:
			outShaderCreateInformation.QueuePriority = option->Value.IntValue;
			break;
		case OT_Transparent:
			outShaderCreateInformation.Flags |= ShaderFlag::Transparent;
			break;
		case OT_Forward:
			outShaderCreateInformation.Flags |= ShaderFlag::Forward;
			break;
		default:
			break;
		}
	}
}

ShaderCompilerResult BSLParser::PopulateVariations(Vector<std::pair<ASTFXNode*, BSLParsedShaderMetaData>>& shaderMetaData)
{
	ShaderCompilerResult parseResult;

	// Inherit variations from mixins
	bool* mixinWasParsed = B3DStackAllocate<bool>((u32)shaderMetaData.size());

	std::function<bool(const BSLParsedShaderMetaData&, BSLParsedShaderMetaData&)> parseInherited =
		[&](const BSLParsedShaderMetaData& metaData, BSLParsedShaderMetaData& combinedMetaData)
	{
		for(auto riter = metaData.Includes.rbegin(); riter != metaData.Includes.rend(); ++riter)
		{
			const String& include = *riter;

			u32 baseIdx = -1;
			for(u32 i = 0; i < (u32)shaderMetaData.size(); i++)
			{
				auto& entry = shaderMetaData[i];
				if(!entry.second.IsMixin)
					continue;

				if(entry.second.Name == include)
				{
					bool matches = entry.second.Language == metaData.Language || entry.second.Language == "Any";

					// We want the last matching mixin, in order to allow mixins to override each other
					if(matches)
						baseIdx = i;
				}
			}

			if(baseIdx != (u32)-1)
			{
				auto& entry = shaderMetaData[baseIdx];

				// Was already parsed previously, don't parse it multiple times (happens when multiple mixins
				// include the same mixin)
				if(mixinWasParsed[baseIdx])
					continue;

				if(!parseInherited(entry.second, combinedMetaData))
					return false;

				for(auto& variation : entry.second.Variations)
					combinedMetaData.Variations.push_back(variation);

				mixinWasParsed[baseIdx] = true;
			}
			else
			{
				parseResult.ErrorMessage = "Mixin \"" + include + "\" cannot be found.";
				return false;
			}
		}

		return true;
	};

	for(auto& entry : shaderMetaData)
	{
		const BSLParsedShaderMetaData& metaData = entry.second;
		if(metaData.IsMixin)
			continue;

		B3DZeroOut(mixinWasParsed, shaderMetaData.size());
		BSLParsedShaderMetaData combinedMetaData = metaData;
		if(!parseInherited(metaData, combinedMetaData))
		{
			B3DStackFree(mixinWasParsed);
			return parseResult;
		}

		entry.second = combinedMetaData;
	}

	B3DStackFree(mixinWasParsed);

	return parseResult;
}

template<bool IsRenderProxy>
void BSLParser::TPopulateVariationParameters(const BSLParsedShaderMetaData& shaderMetaData, CoreVariantType<ShaderCreateInformation, IsRenderProxy>& shaderCreateInformation)
{
	for(auto& entry : shaderMetaData.Variations)
	{
		ShaderVariationParameterInformation paramInfo;
		paramInfo.IsInternal = entry.Internal;
		paramInfo.Name = entry.Name;
		paramInfo.Identifier = entry.Identifier;

		for(auto& value : entry.Values)
		{
			ShaderVariationParameterValue paramValue;
			paramValue.Name = value.Name;
			paramValue.Value = value.Value;

			paramInfo.Values.Add(paramValue);
		}

		shaderCreateInformation.VariationParameters.push_back(paramInfo);
	}
}

template<bool IsRenderProxy>
ShaderCompilerResult BSLParser::TParseMetaData(const String& source, const UnorderedMap<String, String>& defines, CoreVariantType<ShaderCreateInformation, IsRenderProxy>& outShaderInformation, BSLParsedShaderMetaData& outShaderMetaData, Vector<String>& outIncludes)
{
	ParseState* parseState = ParseStateCreate();
	ShaderCompilerResult parseResult = RunParser(parseState, source.c_str(), defines);

	if(!parseResult.ErrorMessage.empty())
	{
		ParseStateDelete(parseState);
		return parseResult;
	}

	// Parse global shader options & shader meta-data
	Vector<pair<ASTFXNode*, BSLParsedShaderMetaData>> shaderMetaDataWithNodes;
	parseResult = TParseMetaDataAndOptions<IsRenderProxy>(parseState->RootNode, shaderMetaDataWithNodes, outShaderInformation);

	if(!parseResult.ErrorMessage.empty())
	{
		ParseStateDelete(parseState);
		return parseResult;
	}

	// Parse includes
	UnorderedSet<String> includeSet;
	IncludeLink* includeLink = parseState->Includes;
	while(includeLink != nullptr)
	{
		const String& includeFilename = includeLink->Data->Filename;
		includeSet.insert(includeFilename);

		includeLink = includeLink->Next;
	}

	for(auto& entry : includeSet)
		outIncludes.push_back(entry);

	ParseStateDelete(parseState);

	parseResult = PopulateVariations(shaderMetaDataWithNodes);

	if(!parseResult.ErrorMessage.empty())
		return parseResult;

	// Note: Must be called after populateVariations, to ensure variations from mixins are inherited
	bool foundShader = false;
	for(auto& entry : shaderMetaDataWithNodes)
	{
		if(entry.second.IsMixin)
			continue;

		if(foundShader)
		{
			parseResult.ErrorMessage = "Shader compilation failed. Multiple shader nodes found in the same file.";
			continue;
		}

		TPopulateVariationParameters<IsRenderProxy>(entry.second, outShaderInformation);
		outShaderMetaData = entry.second;
		foundShader = true;
	}

	return parseResult;
}

template ShaderCompilerResult BSLParser::TParseMetaData<false>(const String& source, const UnorderedMap<String, String>& defines, CoreVariantType<ShaderCreateInformation, false>& inOutShaderInformation, BSLParsedShaderMetaData& outShaderMetaData, Vector<String>& outIncludes);
template ShaderCompilerResult BSLParser::TParseMetaData<true>(const String& source, const UnorderedMap<String, String>& defines, CoreVariantType<ShaderCreateInformation, true>& inOutShaderInformation, BSLParsedShaderMetaData& outShaderMetaData, Vector<String>& outIncludes);

ShaderCompilerResult BSLParser::ParseVariation(const String& name, const String& source, const ShaderVariationParameters& variation, const UnorderedMap<String, String>& defines, BSLParsedShaderData& outParsedShader)
{
	UnorderedMap<String, String> globalDefines = defines;
	UnorderedMap<String, String> variationDefines = variation.GetDefines().GetAll();

	for(auto& define : variationDefines)
		globalDefines[define.first] = define.second;

	ParseState *const variationParseState = ParseStateCreate();
	ShaderCompilerResult parseResult = RunParser(variationParseState, source.c_str(), globalDefines);

	if(!parseResult.ErrorMessage.empty())
	{
		ParseStateDelete(variationParseState);
		return parseResult;
	}

	Vector<String> codeBlocks;
	RawCode* codeBlock = variationParseState->RawCodeBlock[RCT_CodeBlock];
	while(codeBlock != nullptr)
	{
		while((i32)codeBlocks.size() <= codeBlock->Index)
			codeBlocks.push_back(String());

		codeBlocks[codeBlock->Index] = String(codeBlock->Code, codeBlock->Size);
		codeBlock = codeBlock->Next;
	}

	if(variationParseState->RootNode == nullptr || variationParseState->RootNode->Type != NT_Root)
	{
		ParseStateDelete(variationParseState);

		parseResult.ErrorMessage = "Unable to parse BSL shader. Root node is null or not a shader.";
		return parseResult;
	}

	Vector<std::pair<ASTFXNode*, BSLParsedShaderData>> parsedShaders;

	// Go in reverse because options are added in reverse order during parsing
	for(i32 optionIndex = variationParseState->RootNode->Options->Count - 1; optionIndex >= 0; optionIndex--)
	{
		NodeOption* const option = &variationParseState->RootNode->Options->Entries[optionIndex];

		switch(option->Type)
		{
		case OT_Shader:
			{
				// We initially parse only meta-data, so we can handle out-of-order variation definitions
				BSLParsedShaderMetaData variationMetaData = ParseShaderMetaData(option->Value.NodePtr);

				// Skip all variations except the one we're parsing
				if(variationMetaData.Name != name && !variationMetaData.IsMixin)
					continue;

				parsedShaders.push_back(std::make_pair(option->Value.NodePtr, BSLParsedShaderData()));
				BSLParsedShaderData& parsedShader = parsedShaders.back().second;
				parsedShader.MetaData = variationMetaData;

				break;
			}
		default:
			break;
		}
	}

	bool* parseStatePerMixin = B3DStackAllocate<bool>((u32)parsedShaders.size());
	auto fnEnsureMixinsAreParsed = [parseStatePerMixin, &codeBlocks, &parseResult, &parsedShaders](const BSLParsedShaderMetaData& metaData, BSLParsedShaderData& outShader, auto& fnEnsureMixinsAreParsed) -> bool
	{
		for(auto rit = metaData.Includes.rbegin(); rit != metaData.Includes.rend(); ++rit)
		{
			const String& includes = *rit;

			u32 foundMixinIndex = ~0u;
			for(u32 index = 0; index < (u32)parsedShaders.size(); index++)
			{
				auto& entry = parsedShaders[index];
				if(!entry.second.MetaData.IsMixin)
					continue;

				if(entry.second.MetaData.Name == includes)
				{
					const bool matches = (entry.second.MetaData.Language == metaData.Language || entry.second.MetaData.Language == "Any");

					// We want the last matching mixin, in order to allow mixins to override each other
					if(matches)
						foundMixinIndex = index;
				}
			}

			if(foundMixinIndex != ~0u)
			{
				auto& entry = parsedShaders[foundMixinIndex];

				// Was already parsed previously, don't parse it multiple times (happens when multiple mixins include the same mixin)
				if(parseStatePerMixin[foundMixinIndex])
					continue;

				if(!fnEnsureMixinsAreParsed(entry.second.MetaData, outShader, fnEnsureMixinsAreParsed))
					return false;

				ParseShader(entry.first, codeBlocks, outShader);
				parseStatePerMixin[foundMixinIndex] = true;
			}
			else
			{
				parseResult.ErrorMessage = "Mixin \"" + includes + "\" cannot be found.";
				return false;
			}
		}

		return true;
	};

	// Actually parse shaders
	for(auto& parsedShader : parsedShaders)
	{
		const BSLParsedShaderMetaData& metaData = parsedShader.second.MetaData;
		if(metaData.IsMixin)
			continue;

		B3DZeroOut(parseStatePerMixin, parsedShaders.size());
		if(!fnEnsureMixinsAreParsed(metaData, parsedShader.second, fnEnsureMixinsAreParsed))
		{
			ParseStateDelete(variationParseState);
			B3DStackFree(parseStatePerMixin);
			return parseResult;
		}

		ParseShader(parsedShader.first, codeBlocks, parsedShader.second);
	}

	B3DStackFree(parseStatePerMixin);

	bool foundShader = false;
	for(auto& entry : parsedShaders)
	{
		if(entry.second.MetaData.IsMixin)
			continue;

		if(foundShader)
		{
			parseResult.ErrorMessage = "Shader compilation failed. Multiple shader nodes found in the same file.";
			return parseResult;
		}

		outParsedShader = std::move(entry.second);
		foundShader = true;
	}

	parsedShaders.clear();

	ParseStateDelete(variationParseState);
	return parseResult;
}

String BSLParser::RemoveQuotes(const char* input)
{
	u32 len = (u32)strlen(input);
	String output(len - 2, ' ');

	for(u32 i = 0; i < (len - 2); i++)
		output[i] = input[i + 1];

	return output;
}
