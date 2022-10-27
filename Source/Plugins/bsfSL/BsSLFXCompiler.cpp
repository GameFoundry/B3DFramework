//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsSLFXCompiler.h"
#include "RenderAPI/BsGpuProgram.h"
#include <regex>
#include "Material/BsShader.h"
#include "Material/BsTechnique.h"
#include "Material/BsPass.h"
#include "RenderAPI/BsSamplerState.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Debug/BsDebug.h"
#include "Material/BsShaderManager.h"
#include "Material/BsShaderInclude.h"
#include "Math/BsMatrix4.h"
#include "Resources/BsBuiltinResources.h"
#include "Material/BsShaderVariation.h"
#include "Renderer/BsRenderer.h"
#include "Renderer/BsRendererManager.h"
#include "FileSystem/BsFileSystem.h"
#include "FileSystem/BsDataStream.h"

#define XSC_ENABLE_LANGUAGE_EXT 1
#include "Xsc/Xsc.h"

extern "C" {
#include "BsMMAlloc.h"

#define YY_NO_UNISTD_H 1
#include "BsParserFX.h"
#include "BsLexerFX.h"
}

using namespace std;

using namespace bs;

// Print out the FX AST, only for debug purposes
void SLFXDebugPrint(ASTFXNode* node, String indent)
{
	BS_LOG(Info, BSLCompiler, indent + "NODE {0}", node->Type);

	for(int i = 0; i < node->Options->Count; i++)
	{
		OptionDataType odt = OPTION_LOOKUP[(int)node->Options->Entries[i].Type].DataType;
		if(odt == ODT_Complex)
		{
			BS_LOG(Info, BSLCompiler, "{0}{1}. {2}", indent, i, node->Options->Entries[i].Type);
			SLFXDebugPrint(node->Options->Entries[i].Value.NodePtr, indent + "\t");
			continue;
		}

		String value;
		switch(odt)
		{
		case ODT_Bool:
			value = toString(node->Options->Entries[i].Value.IntValue != 0);
			break;
		case ODT_Int:
			value = toString(node->Options->Entries[i].Value.IntValue);
			break;
		case ODT_Float:
			value = toString(node->Options->Entries[i].Value.FloatValue);
			break;
		case ODT_String:
			value = node->Options->Entries[i].Value.StrValue;
			break;
		case ODT_Matrix:
			{
				Matrix4 mat4 = *(Matrix4*)(node->Options->Entries[i].Value.MatrixValue);
				value = toString(mat4);
			}
			break;
		default:
			break;
		}

		BS_LOG(Info, BSLCompiler, "{0}{1}. {2} = {3}", indent, i, node->Options->Entries[i].Type, value);
	}
}

class XscLog : public Xsc::Log
{
public:
	void SubmitReport(const Xsc::Report& report) override
	{
		switch(report.Type())
		{
		case Xsc::ReportTypes::Info:
			mInfos.push_back({ FullIndent(), report });
			break;
		case Xsc::ReportTypes::Warning:
			mWarnings.push_back({ FullIndent(), report });
			break;
		case Xsc::ReportTypes::Error:
			mErrors.push_back({ FullIndent(), report });
			break;
		}
	}

	void GetMessages(StringStream& output)
	{
		PrintAndClearReports(output, mInfos);
		PrintAndClearReports(output, mWarnings, (mWarnings.size() == 1 ? "WARNING" : "WARNINGS"));
		PrintAndClearReports(output, mErrors, (mErrors.size() == 1 ? "ERROR" : "ERRORS"));
	}

private:
	struct IndentReport
	{
		std::string Indent;
		Xsc::Report Report;
	};

	static void PrintMultiLineString(StringStream& output, const std::string& str, const std::string& indent)
	{
		// Determine at which position the actual text begins (excluding the "error (X:Y) : " or the like)
		auto textStartPos = str.find(" : ");
		if(textStartPos != std::string::npos)
			textStartPos += 3;
		else
			textStartPos = 0;

		std::string newLineIndent(textStartPos, ' ');

		size_t start = 0;
		bool useNewLineIndent = false;
		while(start < str.size())
		{
			output << indent;

			if(useNewLineIndent)
				output << newLineIndent;

			// Print next line
			auto end = str.find('\n', start);

			if(end != std::string::npos)
			{
				output << str.substr(start, end - start);
				start = end + 1;
			}
			else
			{
				output << str.substr(start);
				start = end;
			}

			output << std::endl;
			useNewLineIndent = true;
		}
	}

	void PrintReport(StringStream& output, const IndentReport& r)
	{
		// Print optional context description
		if(!r.Report.Context().empty())
			PrintMultiLineString(output, r.Report.Context(), r.Indent);

		// Print report message
		const auto& msg = r.Report.Message();
		PrintMultiLineString(output, msg, r.Indent);

		// Print optional line and line-marker
		if(r.Report.HasLine())
		{
			const auto& line = r.Report.Line();
			const auto& marker = r.Report.Marker();

			// Print line
			output << r.Indent << line << std::endl;

			// Print line marker
			output << r.Indent << marker << std::endl;
		}

		// Print optional hints
		for(const auto& hint : r.Report.GetHints())
			output << r.Indent << hint << std::endl;
	}

	void PrintAndClearReports(StringStream& output, Vector<IndentReport>& reports, const String& headline = "")
	{
		if(!reports.empty())
		{
			if(!headline.empty())
			{
				String s = toString((u32)reports.size()) + " " + headline;
				output << s << std::endl;
				output << String(s.size(), '-') << std::endl;
			}

			for(const auto& r : reports)
				PrintReport(output, r);

			reports.clear();
		}
	}

	Vector<IndentReport> mInfos;
	Vector<IndentReport> mWarnings;
	Vector<IndentReport> mErrors;
};

GpuParamObjectType ReflTypeToTextureType(Xsc::Reflection::BufferType type)
{
	switch(type)
	{
	case Xsc::Reflection::BufferType::RWTexture1D: return GPOT_RWTEXTURE1D;
	case Xsc::Reflection::BufferType::RWTexture1DArray: return GPOT_RWTEXTURE1DARRAY;
	case Xsc::Reflection::BufferType::RWTexture2D: return GPOT_RWTEXTURE2D;
	case Xsc::Reflection::BufferType::RWTexture2DArray: return GPOT_RWTEXTURE2DARRAY;
	case Xsc::Reflection::BufferType::RWTexture3D: return GPOT_RWTEXTURE3D;
	case Xsc::Reflection::BufferType::Texture1D: return GPOT_TEXTURE1D;
	case Xsc::Reflection::BufferType::Texture1DArray: return GPOT_TEXTURE1DARRAY;
	case Xsc::Reflection::BufferType::Texture2D: return GPOT_TEXTURE2D;
	case Xsc::Reflection::BufferType::Texture2DArray: return GPOT_TEXTURE2DARRAY;
	case Xsc::Reflection::BufferType::Texture3D: return GPOT_TEXTURE3D;
	case Xsc::Reflection::BufferType::TextureCube: return GPOT_TEXTURECUBE;
	case Xsc::Reflection::BufferType::TextureCubeArray: return GPOT_TEXTURECUBEARRAY;
	case Xsc::Reflection::BufferType::Texture2DMS: return GPOT_TEXTURE2DMS;
	case Xsc::Reflection::BufferType::Texture2DMSArray: return GPOT_TEXTURE2DMSARRAY;
	default: return GPOT_UNKNOWN;
	}
}

GpuParamObjectType ReflTypeToBufferType(Xsc::Reflection::BufferType type)
{
	switch(type)
	{
	case Xsc::Reflection::BufferType::Buffer: return GPOT_RWTYPED_BUFFER;
	case Xsc::Reflection::BufferType::StructuredBuffer: return GPOT_STRUCTURED_BUFFER;
	case Xsc::Reflection::BufferType::ByteAddressBuffer: return GPOT_BYTE_BUFFER;
	case Xsc::Reflection::BufferType::RWBuffer: return GPOT_RWTYPED_BUFFER;
	case Xsc::Reflection::BufferType::RWStructuredBuffer: return GPOT_RWSTRUCTURED_BUFFER;
	case Xsc::Reflection::BufferType::RWByteAddressBuffer: return GPOT_RWBYTE_BUFFER;
	case Xsc::Reflection::BufferType::AppendStructuredBuffer: return GPOT_RWAPPEND_BUFFER;
	case Xsc::Reflection::BufferType::ConsumeStructuredBuffer: return GPOT_RWCONSUME_BUFFER;
	default: return GPOT_UNKNOWN;
	}
}

GpuParamDataType ReflTypeToDataType(Xsc::Reflection::DataType type)
{
	switch(type)
	{
	case Xsc::Reflection::DataType::Bool: return GPDT_BOOL;
	case Xsc::Reflection::DataType::Float: return GPDT_FLOAT1;
	case Xsc::Reflection::DataType::Float2: return GPDT_FLOAT2;
	case Xsc::Reflection::DataType::Float3: return GPDT_FLOAT3;
	case Xsc::Reflection::DataType::Float4: return GPDT_FLOAT4;
	case Xsc::Reflection::DataType::UInt:
	case Xsc::Reflection::DataType::Int:
		return GPDT_INT1;
	case Xsc::Reflection::DataType::UInt2:
	case Xsc::Reflection::DataType::Int2:
		return GPDT_INT2;
	case Xsc::Reflection::DataType::UInt3:
	case Xsc::Reflection::DataType::Int3:
		return GPDT_INT3;
	case Xsc::Reflection::DataType::UInt4:
	case Xsc::Reflection::DataType::Int4:
		return GPDT_INT4;
	case Xsc::Reflection::DataType::Float2x2: return GPDT_MATRIX_2X2;
	case Xsc::Reflection::DataType::Float2x3: return GPDT_MATRIX_2X3;
	case Xsc::Reflection::DataType::Float2x4: return GPDT_MATRIX_2X4;
	case Xsc::Reflection::DataType::Float3x2: return GPDT_MATRIX_3X4;
	case Xsc::Reflection::DataType::Float3x3: return GPDT_MATRIX_3X3;
	case Xsc::Reflection::DataType::Float3x4: return GPDT_MATRIX_3X4;
	case Xsc::Reflection::DataType::Float4x2: return GPDT_MATRIX_4X2;
	case Xsc::Reflection::DataType::Float4x3: return GPDT_MATRIX_4X3;
	case Xsc::Reflection::DataType::Float4x4: return GPDT_MATRIX_4X4;
	default: return GPDT_UNKNOWN;
	}
}

HTexture getBuiltinTexture(u32 idx)
{
	if(idx == 1)
		return BuiltinResources::GetTexture(BuiltinTexture::White);
	else if(idx == 2)
		return BuiltinResources::GetTexture(BuiltinTexture::Black);
	else if(idx == 3)
		return BuiltinResources::GetTexture(BuiltinTexture::Normal);

	return HTexture();
}

u32 getStructSize(i32 structIdx, const std::vector<Xsc::Reflection::Struct>& structLookup)
{
	if(structIdx < 0 || structIdx >= (i32)structLookup.size())
		return 0;

	u32 size = 0;

	const Xsc::Reflection::Struct& structInfo = structLookup[structIdx];
	for(auto& entry : structInfo.members)
	{
		if(entry.type == Xsc::Reflection::VariableType::Variable)
		{
			// Note: We're ignoring any padding. Since we can't guarantee the padding will be same for structs across
			// different render backends it's expected for the user to set up structs in such a way so padding is not
			// needed (i.e. add padding variables manually).
			GpuParamDataType type = ReflTypeToDataType((Xsc::Reflection::DataType)entry.baseType);

			const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.Lookup[(int)type];
			size += typeInfo.NumColumns * typeInfo.NumRows * typeInfo.BaseTypeSize * entry.arraySize;
		}
		else if(entry.type == Xsc::Reflection::VariableType::Struct)
			size += getStructSize(entry.baseType, structLookup);
	}

	return size;
}

TextureAddressingMode parseTexAddrMode(Xsc::Reflection::TextureAddressMode addrMode)
{
	switch(addrMode)
	{
	case Xsc::Reflection::TextureAddressMode::Border:
		return TAM_BORDER;
	case Xsc::Reflection::TextureAddressMode::Clamp:
		return TAM_CLAMP;
	case Xsc::Reflection::TextureAddressMode::Mirror:
	case Xsc::Reflection::TextureAddressMode::MirrorOnce:
		return TAM_MIRROR;
	case Xsc::Reflection::TextureAddressMode::Wrap:
	default:
		return TAM_WRAP;
	}
}

CompareFunction parseCompFunction(Xsc::Reflection::ComparisonFunc compFunc)
{
	switch(compFunc)
	{
	case Xsc::Reflection::ComparisonFunc::Always:
	default:
		return CMPF_ALWAYS_PASS;
	case Xsc::Reflection::ComparisonFunc::Never:
		return CMPF_ALWAYS_FAIL;
	case Xsc::Reflection::ComparisonFunc::Equal:
		return CMPF_EQUAL;
	case Xsc::Reflection::ComparisonFunc::Greater:
		return CMPF_GREATER;
	case Xsc::Reflection::ComparisonFunc::GreaterEqual:
		return CMPF_GREATER_EQUAL;
	case Xsc::Reflection::ComparisonFunc::Less:
		return CMPF_LESS;
	case Xsc::Reflection::ComparisonFunc::LessEqual:
		return CMPF_LESS_EQUAL;
	case Xsc::Reflection::ComparisonFunc::NotEqual:
		return CMPF_NOT_EQUAL;
	}
}

SPtr<SamplerState> parseSamplerState(const Xsc::Reflection::SamplerState& sampState)
{
	SAMPLER_STATE_DESC desc;

	desc.AddressMode.U = parseTexAddrMode(sampState.addressU);
	desc.AddressMode.V = parseTexAddrMode(sampState.addressV);
	desc.AddressMode.W = parseTexAddrMode(sampState.addressW);

	desc.BorderColor[0] = sampState.borderColor[0];
	desc.BorderColor[1] = sampState.borderColor[1];
	desc.BorderColor[2] = sampState.borderColor[2];
	desc.BorderColor[3] = sampState.borderColor[3];

	desc.ComparisonFunc = parseCompFunction(sampState.comparisonFunc);
	desc.MaxAniso = sampState.maxAnisotropy;
	desc.MipMax = sampState.maxLOD;
	desc.MipMin = sampState.minLOD;
	desc.MipmapBias = sampState.mipLODBias;

	switch(sampState.filter)
	{
	case Xsc::Reflection::Filter::MinMagMipPoint:
	case Xsc::Reflection::Filter::ComparisonMinMagMipPoint:
		desc.MinFilter = FO_POINT;
		desc.MagFilter = FO_POINT;
		desc.MipFilter = FO_POINT;
		break;
	case Xsc::Reflection::Filter::MinMagPointMipLinear:
	case Xsc::Reflection::Filter::ComparisonMinMagPointMipLinear:
		desc.MinFilter = FO_POINT;
		desc.MagFilter = FO_POINT;
		desc.MipFilter = FO_LINEAR;
		break;
	case Xsc::Reflection::Filter::MinPointMagLinearMipPoint:
	case Xsc::Reflection::Filter::ComparisonMinPointMagLinearMipPoint:
		desc.MinFilter = FO_POINT;
		desc.MagFilter = FO_LINEAR;
		desc.MipFilter = FO_POINT;
		break;
	case Xsc::Reflection::Filter::MinPointMagMipLinear:
	case Xsc::Reflection::Filter::ComparisonMinPointMagMipLinear:
		desc.MinFilter = FO_POINT;
		desc.MagFilter = FO_LINEAR;
		desc.MipFilter = FO_LINEAR;
		break;
	case Xsc::Reflection::Filter::MinLinearMagMipPoint:
	case Xsc::Reflection::Filter::ComparisonMinLinearMagMipPoint:
		desc.MinFilter = FO_LINEAR;
		desc.MagFilter = FO_POINT;
		desc.MipFilter = FO_POINT;
		break;
	case Xsc::Reflection::Filter::MinLinearMagPointMipLinear:
	case Xsc::Reflection::Filter::ComparisonMinLinearMagPointMipLinear:
		desc.MinFilter = FO_LINEAR;
		desc.MagFilter = FO_POINT;
		desc.MipFilter = FO_LINEAR;
		break;
	case Xsc::Reflection::Filter::MinMagLinearMipPoint:
	case Xsc::Reflection::Filter::ComparisonMinMagLinearMipPoint:
		desc.MinFilter = FO_LINEAR;
		desc.MagFilter = FO_LINEAR;
		desc.MipFilter = FO_POINT;
		break;
	case Xsc::Reflection::Filter::MinMagMipLinear:
	case Xsc::Reflection::Filter::ComparisonMinMagMipLinear:
		desc.MinFilter = FO_LINEAR;
		desc.MagFilter = FO_LINEAR;
		desc.MipFilter = FO_LINEAR;
		break;
	case Xsc::Reflection::Filter::Anisotropic:
	case Xsc::Reflection::Filter::ComparisonAnisotropic:
		desc.MinFilter = FO_ANISOTROPIC;
		desc.MagFilter = FO_ANISOTROPIC;
		desc.MipFilter = FO_ANISOTROPIC;
		break;
	default:
		break;
	}

	return SamplerState::Create(desc);
}

void parseParameters(const Xsc::Reflection::ReflectionData& reflData, SHADER_DESC& desc)
{
	for(auto& entry : reflData.uniforms)
	{
		if((entry.flags & Xsc::Reflection::Uniform::Flags::Internal) != 0)
			continue;

		String ident = entry.ident.c_str();
		auto parseCommonAttributes = [&entry, &ident, &desc]()
		{
			if(!entry.readableName.empty())
			{
				SHADER_PARAM_ATTRIBUTE attribute;
				attribute.Value.assign(entry.readableName.data(), entry.readableName.size());
				attribute.NextParamIdx = (u32)-1;
				attribute.Type = ShaderParamAttributeType::Name;

				desc.SetParameterAttribute(ident, attribute);
			}

			if((entry.flags & Xsc::Reflection::Uniform::Flags::HideInInspector) != 0)
			{
				SHADER_PARAM_ATTRIBUTE attribute;
				attribute.NextParamIdx = (u32)-1;
				attribute.Type = ShaderParamAttributeType::HideInInspector;

				desc.SetParameterAttribute(ident, attribute);
			}

			if((entry.flags & Xsc::Reflection::Uniform::Flags::HDR) != 0)
			{
				SHADER_PARAM_ATTRIBUTE attribute;
				attribute.NextParamIdx = (u32)-1;
				attribute.Type = ShaderParamAttributeType::HDR;

				desc.SetParameterAttribute(ident, attribute);
			}
		};

		switch(entry.type)
		{
		case Xsc::Reflection::VariableType::UniformBuffer:
			desc.SetParamBlockAttribs(entry.ident.c_str(), false, GBU_STATIC);
			break;
		case Xsc::Reflection::VariableType::Buffer:
			{
				GpuParamObjectType objType = ReflTypeToTextureType((Xsc::Reflection::BufferType)entry.baseType);
				if(objType != GPOT_UNKNOWN)
				{
					// Ignore parameters that were already registered in some previous variation. Note that this implies
					// you cannot have same names for different parameters in different variations.
					if(desc.TextureParams.find(ident) != desc.TextureParams.end())
						continue;

					if(entry.defaultValue == -1)
						desc.AddParameter(SHADER_OBJECT_PARAM_DESC(ident, ident, objType));
					else
					{
						const Xsc::Reflection::DefaultValue& defVal = reflData.defaultValues[entry.defaultValue];
						desc.AddParameter(SHADER_OBJECT_PARAM_DESC(ident, ident, objType), getBuiltinTexture(defVal.integer));
					}

					parseCommonAttributes();
				}
				else
				{
					// Ignore parameters that were already registered in some previous variation. Note that this implies
					// you cannot have same names for different parameters in different variations.
					if(desc.BufferParams.find(ident) != desc.BufferParams.end())
						continue;

					objType = ReflTypeToBufferType((Xsc::Reflection::BufferType)entry.baseType);
					desc.AddParameter(SHADER_OBJECT_PARAM_DESC(ident, ident, objType));

					parseCommonAttributes();
				}
			}
			break;
		case Xsc::Reflection::VariableType::Sampler:
			{
				auto findIter = reflData.samplerStates.find(entry.ident);
				if(findIter != reflData.samplerStates.end())
				{
					// Ignore parameters that were already registered in some previous variation. Note that this implies
					// you cannot have same names for different parameters in different variations.
					if(desc.SamplerParams.find(ident) != desc.SamplerParams.end())
						continue;

					String alias = findIter->second.alias.c_str();

					if(findIter->second.isNonDefault)
					{
						SPtr<SamplerState> defaultVal = parseSamplerState(findIter->second);
						desc.AddParameter(SHADER_OBJECT_PARAM_DESC(ident, ident, GPOT_SAMPLER2D), defaultVal);

						if(!alias.empty())
							desc.AddParameter(SHADER_OBJECT_PARAM_DESC(ident, alias, GPOT_SAMPLER2D), defaultVal);
					}
					else
					{
						desc.AddParameter(SHADER_OBJECT_PARAM_DESC(ident, ident, GPOT_SAMPLER2D));

						if(!alias.empty())
							desc.AddParameter(SHADER_OBJECT_PARAM_DESC(ident, alias, GPOT_SAMPLER2D));
					}
				}
				else
				{
					desc.AddParameter(SHADER_OBJECT_PARAM_DESC(ident, ident, GPOT_SAMPLER2D));
				}
				break;
			}
		case Xsc::Reflection::VariableType::Variable:
			{
				bool isBlockInternal = false;
				if(entry.uniformBlock != -1)
				{
					std::string blockName = reflData.constantBuffers[entry.uniformBlock].ident;
					for(auto& uniform : reflData.uniforms)
					{
						if(uniform.type == Xsc::Reflection::VariableType::UniformBuffer && uniform.ident == blockName)
						{
							isBlockInternal = (uniform.flags & Xsc::Reflection::Uniform::Flags::Internal) != 0;
							break;
						}
					}
				}

				if(!isBlockInternal)
				{
					GpuParamDataType type = ReflTypeToDataType((Xsc::Reflection::DataType)entry.baseType);
					if((entry.flags & Xsc::Reflection::Uniform::Flags::Color) != 0 &&
					   (type == GPDT_FLOAT3 || type == GPDT_FLOAT4))
					{
						type = GPDT_COLOR;
					}

					u32 arraySize = entry.arraySize;

					if(entry.defaultValue == -1)
						desc.AddParameter(SHADER_DATA_PARAM_DESC(ident, ident, type, StringID::NONE, arraySize));
					else
					{
						const Xsc::Reflection::DefaultValue& defVal = reflData.defaultValues[entry.defaultValue];

						desc.AddParameter(SHADER_DATA_PARAM_DESC(ident, ident, type, StringID::NONE, arraySize, 0), (u8*)defVal.matrix);
					}

					if(!entry.spriteUVRef.empty() && (type == GPDT_FLOAT4))
					{
						SHADER_PARAM_ATTRIBUTE attribute;
						attribute.Value.assign(entry.spriteUVRef.data(), entry.spriteUVRef.size());
						attribute.NextParamIdx = (u32)-1;
						attribute.Type = ShaderParamAttributeType::SpriteUV;

						desc.SetParameterAttribute(ident, attribute);
					}

					parseCommonAttributes();
				}
			}
			break;
		case Xsc::Reflection::VariableType::Struct:
			{
				i32 structIdx = entry.baseType;
				u32 structSize = getStructSize(structIdx, reflData.structs);

				desc.AddParameter(SHADER_DATA_PARAM_DESC(ident, ident, GPDT_STRUCT, StringID::NONE, entry.arraySize, structSize));
			}
			break;
		default:;
		}
	}
}

/** Types of supported code output when cross compiling HLSL to GLSL. */
enum class CrossCompileOutput
{
	GLSL45,
	GLSL41,
	VKSL45,
	MVKSL
};

String crossCompile(const String& hlsl, GpuProgramType type, CrossCompileOutput outputType, bool optionalEntry, u32& startBindingSlot, SHADER_DESC* shaderDesc = nullptr, Vector<GpuProgramType>* detectedTypes = nullptr)
{
	SPtr<StringStream> input = bs_shared_ptr_new<StringStream>();

	bool isVKSL = outputType == CrossCompileOutput::VKSL45 || outputType == CrossCompileOutput::MVKSL;
	switch(outputType)
	{
	case CrossCompileOutput::GLSL41:
	case CrossCompileOutput::GLSL45:
		*input << "#define OPENGL 1" << std::endl;
		break;
	case CrossCompileOutput::VKSL45:
		*input << "#define VULKAN 1" << std::endl;
		break;
	case CrossCompileOutput::MVKSL:
		*input << "#define METAL 1" << std::endl;
		break;
	}

	*input << hlsl;

	Xsc::ShaderInput inputDesc;
	inputDesc.shaderVersion = Xsc::InputShaderVersion::HLSL5;
	inputDesc.sourceCode = input;
	inputDesc.extensions = Xsc::Extensions::LayoutAttribute;

	switch(type)
	{
	case GPT_VERTEX_PROGRAM:
		inputDesc.shaderTarget = Xsc::ShaderTarget::VertexShader;
		inputDesc.entryPoint = "vsmain";
		break;
	case GPT_GEOMETRY_PROGRAM:
		inputDesc.shaderTarget = Xsc::ShaderTarget::GeometryShader;
		inputDesc.entryPoint = "gsmain";
		break;
	case GPT_HULL_PROGRAM:
		inputDesc.shaderTarget = Xsc::ShaderTarget::TessellationControlShader;
		inputDesc.entryPoint = "hsmain";
		break;
	case GPT_DOMAIN_PROGRAM:
		inputDesc.shaderTarget = Xsc::ShaderTarget::TessellationEvaluationShader;
		inputDesc.entryPoint = "dsmain";
		break;
	case GPT_FRAGMENT_PROGRAM:
		inputDesc.shaderTarget = Xsc::ShaderTarget::FragmentShader;
		inputDesc.entryPoint = "fsmain";
		break;
	case GPT_COMPUTE_PROGRAM:
		inputDesc.shaderTarget = Xsc::ShaderTarget::ComputeShader;
		inputDesc.entryPoint = "csmain";
		break;
	default:
		break;
	}

	StringStream output;

	Xsc::ShaderOutput outputDesc;
	outputDesc.sourceCode = &output;
	outputDesc.options.autoBinding = isVKSL;
	outputDesc.options.autoBindingStartSlot = startBindingSlot;
	outputDesc.options.fragmentLocations = true;
	outputDesc.options.separateShaders = true;
	outputDesc.options.separateSamplers = isVKSL;
	outputDesc.options.allowExtensions = true;
	outputDesc.nameMangling.inputPrefix = "bs_";
	outputDesc.nameMangling.outputPrefix = "bs_";
	outputDesc.nameMangling.useAlwaysSemantics = true;
	outputDesc.nameMangling.renameBufferFields = true;

	switch(outputType)
	{
	case CrossCompileOutput::GLSL45:
		outputDesc.shaderVersion = Xsc::OutputShaderVersion::GLSL450;
		break;
	case CrossCompileOutput::GLSL41:
		outputDesc.shaderVersion = Xsc::OutputShaderVersion::GLSL410;
		break;
	case CrossCompileOutput::VKSL45:
		outputDesc.shaderVersion = Xsc::OutputShaderVersion::VKSL450;
		break;
	case CrossCompileOutput::MVKSL:
		outputDesc.shaderVersion = Xsc::OutputShaderVersion::VKSL450;
		break;
	}

	XscLog log;
	Xsc::Reflection::ReflectionData reflectionData;
	bool compileSuccess = Xsc::CompileShader(inputDesc, outputDesc, &log, &reflectionData);
	if(!compileSuccess)
	{
		// If enabled, don't fail if entry point isn't found
		bool done = true;
		if(optionalEntry)
		{
			bool entryFound = false;
			for(auto& entry : reflectionData.functions)
			{
				if(entry.ident == inputDesc.entryPoint)
				{
					entryFound = true;
					break;
				}
			}

			if(!entryFound)
				done = false;
		}

		if(done)
		{
			StringStream logOutput;
			log.GetMessages(logOutput);

			BS_LOG(Error, BSLCompiler, "Shader cross compilation failed. Log: \n\n{0}", logOutput.str());
			return "";
		}
	}

	for(auto& entry : reflectionData.constantBuffers)
		startBindingSlot = std::max(startBindingSlot, entry.location + 1u);

	for(auto& entry : reflectionData.textures)
		startBindingSlot = std::max(startBindingSlot, entry.location + 1u);

	for(auto& entry : reflectionData.storageBuffers)
		startBindingSlot = std::max(startBindingSlot, entry.location + 1u);

	if(detectedTypes != nullptr)
	{
		for(auto& entry : reflectionData.functions)
		{
			if(entry.ident == "vsmain")
				detectedTypes->push_back(GPT_VERTEX_PROGRAM);
			else if(entry.ident == "fsmain")
				detectedTypes->push_back(GPT_FRAGMENT_PROGRAM);
			else if(entry.ident == "gsmain")
				detectedTypes->push_back(GPT_GEOMETRY_PROGRAM);
			else if(entry.ident == "dsmain")
				detectedTypes->push_back(GPT_DOMAIN_PROGRAM);
			else if(entry.ident == "hsmain")
				detectedTypes->push_back(GPT_HULL_PROGRAM);
			else if(entry.ident == "csmain")
				detectedTypes->push_back(GPT_COMPUTE_PROGRAM);
		}

		// If no entry points found, and error occurred, report error
		if(!compileSuccess && detectedTypes->empty())
		{
			StringStream logOutput;
			log.GetMessages(logOutput);

			BS_LOG(Error, BSLCompiler, "Shader cross compilation failed. Log: \n\n{0}", logOutput.str());
			return "";
		}
	}

	if(shaderDesc != nullptr)
		parseParameters(reflectionData, *shaderDesc);

	return output.str();
}

String crossCompile(const String& hlsl, GpuProgramType type, CrossCompileOutput outputType, u32& startBindingSlot)
{
	return crossCompile(hlsl, type, outputType, false, startBindingSlot);
}

void reflectHLSL(const String& hlsl, SHADER_DESC& shaderDesc, Vector<GpuProgramType>& entryPoints)
{
	u32 dummy = 0;
	crossCompile(hlsl, GPT_VERTEX_PROGRAM, CrossCompileOutput::GLSL45, true, dummy, &shaderDesc, &entryPoints);
}

BSLFXCompileResult BSLFXCompiler::Compile(const String& name, const String& source, const UnorderedMap<String, String>& defines, ShadingLanguageFlags languages)
{
	// Parse global shader options & shader meta-data
	SHADER_DESC shaderDesc;
	Vector<String> includes;

	BSLFXCompileResult output = CompileShader(source, defines, languages, shaderDesc, includes);

	// Generate a shader from the parsed information
	output.Shader = Shader::CreatePtrInternal(name, shaderDesc);
	output.Shader->SetIncludeFiles(includes);

	return output;
}

BSLFXCompileResult BSLFXCompiler::ParseFx(ParseState* parseState, const char* source, const UnorderedMap<String, String>& defines)
{
	for(auto& define : defines)
	{
		if(define.first.size() == 0)
			continue;

		addDefine(parseState, define.first.c_str());

		if(define.second.size() > 0)
			addDefineExpr(parseState, define.second.c_str());
	}

	yyscan_t scanner;
	YY_BUFFER_STATE state;

	BSLFXCompileResult output;

	if(yylex_init_extra(parseState, &scanner))
	{
		output.ErrorMessage = "Internal error: Lexer failed to initialize.";
		return output;
	}

	// If debug output from lexer is needed uncomment this and add %debug option to lexer file
	// yyset_debug(true, scanner);

	// If debug output from parser is needed uncomment this and add %debug option to parser file
	// yydebug = true;

	state = yy_scan_string(source, scanner);

	bool parsingFailed = yyparse(parseState, scanner) > 0;

	if(parseState->HasError > 0)
	{
		output.ErrorMessage = parseState->ErrorMessage;
		output.ErrorLine = parseState->ErrorLine;
		output.ErrorColumn = parseState->ErrorColumn;

		if(parseState->ErrorFile != nullptr)
			output.ErrorFile = parseState->ErrorFile;

		goto cleanup;
	}
	else if(parsingFailed)
	{
		output.ErrorMessage = "Internal error: Parsing failed.";
		goto cleanup;
	}

cleanup:
	yy_delete_buffer(state, scanner);
	yylex_destroy(scanner);

	return output;
}

BSLFXCompiler::ShaderMetaData BSLFXCompiler::ParseShaderMetaData(ASTFXNode* shader)
{
	ShaderMetaData metaData;

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

BSLFXCompileResult BSLFXCompiler::ParseMetaDataAndOptions(ASTFXNode* rootNode, Vector<std::pair<ASTFXNode*, ShaderMetaData>>& shaderMetaData, Vector<SubShaderData>& subShaderData, SHADER_DESC& shaderDesc)
{
	BSLFXCompileResult output;

	// Only enable for debug purposes
	// SLFXDebugPrint(parseState->rootNode, "");

	if(rootNode == nullptr || rootNode->Type != NT_Root)
	{
		output.ErrorMessage = "Root is null or not a shader.";
		return output;
	}

	// Parse global shader options & shader meta-data
	//// Go in reverse because options are added in reverse order during parsing
	for(int i = rootNode->Options->Count - 1; i >= 0; i--)
	{
		NodeOption* option = &rootNode->Options->Entries[i];

		switch(option->Type)
		{
		case OT_Options:
			ParseOptions(option->Value.NodePtr, shaderDesc);
			break;
		case OT_Shader:
			{
				// We initially parse only meta-data, so we can handle out-of-order mixin/shader definitions
				ShaderMetaData metaData = ParseShaderMetaData(option->Value.NodePtr);
				shaderMetaData.push_back(std::make_pair(option->Value.NodePtr, metaData));

				break;
			}
		case OT_SubShader:
			{
				SubShaderData data = ParseSubShader(option->Value.NodePtr);
				subShaderData.push_back(data);

				break;
			}
		default:
			break;
		}
	}

	return output;
}

void BSLFXCompiler::ParseVariations(ShaderMetaData& metaData, ASTFXNode* variations)
{
	assert(variations->Type == NT_Variation);

	VariationData variationData;
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

BSLFXCompiler::VariationOption BSLFXCompiler::ParseVariationOption(ASTFXNode* variationOption)
{
	assert(variationOption->Type == NT_VariationOption);

	VariationOption output;
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

BSLFXCompiler::AttributeData BSLFXCompiler::ParseAttributes(ASTFXNode* attributes)
{
	assert(attributes->Type == NT_Attributes);

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

QueueSortType BSLFXCompiler::ParseSortType(CullAndSortModeValue sortType)
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

CompareFunction BSLFXCompiler::ParseCompFunc(CompFuncValue compFunc)
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

BlendFactor BSLFXCompiler::ParseBlendFactor(OpValue factor)
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

BlendOperation BSLFXCompiler::ParseBlendOp(BlendOpValue op)
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

StencilOperation BSLFXCompiler::ParseStencilOp(OpValue op)
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

CullingMode BSLFXCompiler::ParseCullMode(CullAndSortModeValue cm)
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

PolygonMode BSLFXCompiler::ParseFillMode(FillModeValue fm)
{
	if(fm == FMV_Wire)
		return PM_WIREFRAME;

	return PM_SOLID;
}

void BSLFXCompiler::ParseStencilFront(DEPTH_STENCIL_STATE_DESC& desc, ASTFXNode* stencilOpNode)
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

void BSLFXCompiler::ParseStencilBack(DEPTH_STENCIL_STATE_DESC& desc, ASTFXNode* stencilOpNode)
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

void BSLFXCompiler::ParseColorBlendDef(RENDER_TARGET_BLEND_STATE_DESC& desc, ASTFXNode* blendDefNode)
{
	if(blendDefNode == nullptr || blendDefNode->Type != NT_BlendDef)
		return;

	for(int i = 0; i < blendDefNode->Options->Count; i++)
	{
		NodeOption* option = &blendDefNode->Options->Entries[i];

		switch(option->Type)
		{
		case OT_Source:
			desc.SrcBlend = ParseBlendFactor((OpValue)option->Value.IntValue);
			break;
		case OT_Dest:
			desc.DstBlend = ParseBlendFactor((OpValue)option->Value.IntValue);
			break;
		case OT_Op:
			desc.BlendOp = ParseBlendOp((BlendOpValue)option->Value.IntValue);
			break;
		default:
			break;
		}
	}
}

void BSLFXCompiler::ParseAlphaBlendDef(RENDER_TARGET_BLEND_STATE_DESC& desc, ASTFXNode* blendDefNode)
{
	if(blendDefNode == nullptr || blendDefNode->Type != NT_BlendDef)
		return;

	for(int i = 0; i < blendDefNode->Options->Count; i++)
	{
		NodeOption* option = &blendDefNode->Options->Entries[i];

		switch(option->Type)
		{
		case OT_Source:
			desc.SrcBlendAlpha = ParseBlendFactor((OpValue)option->Value.IntValue);
			break;
		case OT_Dest:
			desc.DstBlendAlpha = ParseBlendFactor((OpValue)option->Value.IntValue);
			break;
		case OT_Op:
			desc.BlendOpAlpha = ParseBlendOp((BlendOpValue)option->Value.IntValue);
			break;
		default:
			break;
		}
	}
}

void BSLFXCompiler::ParseRenderTargetBlendState(BLEND_STATE_DESC& desc, ASTFXNode* targetNode, u32& index)
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

	if(index >= BS_MAX_MULTIPLE_RENDER_TARGETS)
		return;

	RENDER_TARGET_BLEND_STATE_DESC& rtDesc = desc.RenderTargetDesc[index];
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

bool BSLFXCompiler::ParseBlendState(PassData& desc, ASTFXNode* blendNode)
{
	if(blendNode == nullptr || blendNode->Type != NT_Blend)
		return false;

	bool isDefault = true;
	SmallVector<ASTFXNode*, 8> targets;

	for(int i = 0; i < blendNode->Options->Count; i++)
	{
		NodeOption* option = &blendNode->Options->Entries[i];

		switch(option->Type)
		{
		case OT_AlphaToCoverage:
			desc.BlendDesc.AlphaToCoverageEnable = option->Value.IntValue > 0;
			isDefault = false;
			break;
		case OT_IndependantBlend:
			desc.BlendDesc.IndependantBlendEnable = option->Value.IntValue > 0;
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
		ParseRenderTargetBlendState(desc.BlendDesc, *iter, index);

	return !isDefault;
}

bool BSLFXCompiler::ParseRasterizerState(PassData& desc, ASTFXNode* rasterNode)
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
			desc.RasterizerDesc.PolygonMode = ParseFillMode((FillModeValue)option->Value.IntValue);
			isDefault = false;
			break;
		case OT_CullMode:
			desc.RasterizerDesc.CullMode = ParseCullMode((CullAndSortModeValue)option->Value.IntValue);
			isDefault = false;
			break;
		case OT_DepthBias:
			desc.RasterizerDesc.DepthBias = option->Value.FloatValue;
			isDefault = false;
			break;
		case OT_SDepthBias:
			desc.RasterizerDesc.SlopeScaledDepthBias = option->Value.FloatValue;
			isDefault = false;
			break;
		case OT_DepthClip:
			desc.RasterizerDesc.DepthClipEnable = option->Value.IntValue > 0;
			isDefault = false;
			break;
		case OT_Scissor:
			desc.RasterizerDesc.ScissorEnable = option->Value.IntValue > 0;
			isDefault = false;
			break;
		case OT_Multisample:
			desc.RasterizerDesc.MultisampleEnable = option->Value.IntValue > 0;
			isDefault = false;
			break;
		case OT_AALine:
			desc.RasterizerDesc.AntialiasedLineEnable = option->Value.IntValue > 0;
			isDefault = false;
			break;
		default:
			break;
		}
	}

	return !isDefault;
}

bool BSLFXCompiler::ParseDepthState(PassData& passData, ASTFXNode* depthNode)
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
			passData.DepthStencilDesc.DepthReadEnable = option->Value.IntValue > 0;
			isDefault = false;
			break;
		case OT_DepthWrite:
			passData.DepthStencilDesc.DepthWriteEnable = option->Value.IntValue > 0;
			isDefault = false;
			break;
		case OT_CompareFunc:
			passData.DepthStencilDesc.DepthComparisonFunc = ParseCompFunc((CompFuncValue)option->Value.IntValue);
			isDefault = false;
			break;
		default:
			break;
		}
	}

	return !isDefault;
}

bool BSLFXCompiler::ParseStencilState(PassData& passData, ASTFXNode* stencilNode)
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
			passData.DepthStencilDesc.StencilEnable = option->Value.IntValue > 0;
			isDefault = false;
			break;
		case OT_StencilReadMask:
			passData.DepthStencilDesc.StencilReadMask = (u8)option->Value.IntValue;
			isDefault = false;
			break;
		case OT_StencilWriteMask:
			passData.DepthStencilDesc.StencilWriteMask = (u8)option->Value.IntValue;
			isDefault = false;
			break;
		case OT_StencilOpFront:
			ParseStencilFront(passData.DepthStencilDesc, option->Value.NodePtr);
			isDefault = false;
			break;
		case OT_StencilOpBack:
			ParseStencilBack(passData.DepthStencilDesc, option->Value.NodePtr);
			isDefault = false;
			break;
		case OT_StencilRef:
			passData.StencilRefValue = option->Value.IntValue;
			break;
		default:
			break;
		}
	}

	return !isDefault;
}

void BSLFXCompiler::ParseCodeBlock(ASTFXNode* codeNode, const Vector<String>& codeBlocks, PassData& passData)
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

void BSLFXCompiler::ParsePass(ASTFXNode* passNode, const Vector<String>& codeBlocks, PassData& passData)
{
	if(passNode == nullptr || passNode->Type != NT_Pass)
		return;

	for(int i = 0; i < passNode->Options->Count; i++)
	{
		NodeOption* option = &passNode->Options->Entries[i];

		switch(option->Type)
		{
		case OT_Blend:
			passData.BlendIsDefault &= !ParseBlendState(passData, option->Value.NodePtr);
			break;
		case OT_Raster:
			passData.RasterizerIsDefault &= !ParseRasterizerState(passData, option->Value.NodePtr);
			break;
		case OT_Depth:
			passData.DepthStencilIsDefault &= !ParseDepthState(passData, option->Value.NodePtr);
			break;
		case OT_Stencil:
			passData.DepthStencilIsDefault &= !ParseStencilState(passData, option->Value.NodePtr);
			break;
		case OT_Code:
			ParseCodeBlock(option->Value.NodePtr, codeBlocks, passData);
			break;
		default:
			break;
		}
	}
}

void BSLFXCompiler::ParseShader(ASTFXNode* shaderNode, const Vector<String>& codeBlocks, ShaderData& shaderData)
{
	if(shaderNode == nullptr || (shaderNode->Type != NT_Shader && shaderNode->Type != NT_Mixin))
		return;

	// There must always be at least one pass
	if(shaderData.Passes.empty())
	{
		shaderData.Passes.push_back(PassData());
		shaderData.Passes.back().SeqIdx = 0;
	}

	PassData combinedCommonPassData;

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
				PassData* passData = nullptr;
				for(auto& entry : shaderData.Passes)
				{
					if(entry.SeqIdx == passIdx)
						passData = &entry;
				}

				if(passData == nullptr)
				{
					shaderData.Passes.push_back(PassData());
					passData = &shaderData.Passes.back();

					passData->SeqIdx = passIdx;
				}

				nextPassIdx = std::max(nextPassIdx, passIdx) + 1;
				passData->Code = combinedCommonPassData.Code + passData->Code;

				ASTFXNode* passNode = option->Value.NodePtr;
				ParsePass(passNode, codeBlocks, *passData);
			}
			break;
		case OT_Code:
			{
				PassData commonPassData;
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
				passData.BlendIsDefault &= !ParseBlendState(passData, option->Value.NodePtr);
			break;
		case OT_Raster:
			for(auto& passData : shaderData.Passes)
				passData.RasterizerIsDefault &= !ParseRasterizerState(passData, option->Value.NodePtr);
			break;
		case OT_Depth:
			for(auto& passData : shaderData.Passes)
				passData.DepthStencilIsDefault &= !ParseDepthState(passData, option->Value.NodePtr);
			break;
		case OT_Stencil:
			for(auto& passData : shaderData.Passes)
				passData.DepthStencilIsDefault &= !ParseStencilState(passData, option->Value.NodePtr);
			break;
		default:
			break;
		}
	}
}

BSLFXCompiler::SubShaderData BSLFXCompiler::ParseSubShader(ASTFXNode* subShader)
{
	SubShaderData subShaderData;

	//// Go in reverse because options are added in reverse order during parsing
	for(int i = subShader->Options->Count - 1; i >= 0; i--)
	{
		NodeOption* option = &subShader->Options->Entries[i];

		switch(option->Type)
		{
		case OT_Identifier:
			subShaderData.Name = option->Value.StrValue;
			break;
		case OT_Index:
			subShaderData.CodeBlockIndex = option->Value.IntValue;
		default:
			break;
		}
	}

	return subShaderData;
}

void BSLFXCompiler::ParseOptions(ASTFXNode* optionsNode, SHADER_DESC& shaderDesc)
{
	if(optionsNode == nullptr || optionsNode->Type != NT_Options)
		return;

	for(int i = optionsNode->Options->Count - 1; i >= 0; i--)
	{
		NodeOption* option = &optionsNode->Options->Entries[i];

		switch(option->Type)
		{
		case OT_Separable:
			shaderDesc.SeparablePasses = option->Value.IntValue > 1;
			break;
		case OT_Sort:
			shaderDesc.QueueSortType = ParseSortType((CullAndSortModeValue)option->Value.IntValue);
			break;
		case OT_Priority:
			shaderDesc.QueuePriority = option->Value.IntValue;
			break;
		case OT_Transparent:
			shaderDesc.Flags |= ShaderFlag::Transparent;
			break;
		case OT_Forward:
			shaderDesc.Flags |= ShaderFlag::Forward;
			break;
		default:
			break;
		}
	}
}

BSLFXCompileResult BSLFXCompiler::PopulateVariations(Vector<std::pair<ASTFXNode*, ShaderMetaData>>& shaderMetaData)
{
	BSLFXCompileResult output;

	// Inherit variations from mixins
	bool* mixinWasParsed = bs_stack_alloc<bool>((u32)shaderMetaData.size());

	std::function<bool(const ShaderMetaData&, ShaderMetaData&)> parseInherited =
		[&](const ShaderMetaData& metaData, ShaderMetaData& combinedMetaData)
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
				output.ErrorMessage = "Mixin \"" + include + "\" cannot be found.";
				return false;
			}
		}

		return true;
	};

	for(auto& entry : shaderMetaData)
	{
		const ShaderMetaData& metaData = entry.second;
		if(metaData.IsMixin)
			continue;

		bs_zero_out(mixinWasParsed, shaderMetaData.size());
		ShaderMetaData combinedMetaData = metaData;
		if(!parseInherited(metaData, combinedMetaData))
		{
			bs_stack_free(mixinWasParsed);
			return output;
		}

		entry.second = combinedMetaData;
	}

	bs_stack_free(mixinWasParsed);

	return output;
}

void BSLFXCompiler::PopulateVariationParamInfos(const ShaderMetaData& shaderMetaData, SHADER_DESC& desc)
{
	for(auto& entry : shaderMetaData.Variations)
	{
		ShaderVariationParamInfo paramInfo;
		paramInfo.IsInternal = entry.Internal;
		paramInfo.Name = entry.Name;
		paramInfo.Identifier = entry.Identifier;

		for(auto& value : entry.Values)
		{
			ShaderVariationParamValue paramValue;
			paramValue.Name = value.Name;
			paramValue.Value = value.Value;

			paramInfo.Values.Add(paramValue);
		}

		desc.VariationParams.push_back(paramInfo);
	}
}

BSLFXCompileResult BSLFXCompiler::CompileTechniques(
	const Vector<std::pair<ASTFXNode*, ShaderMetaData>>& shaderMetaData, const String& source,
	const UnorderedMap<String, String>& defines, ShadingLanguageFlags languages, SHADER_DESC& shaderDesc,
	Vector<String>& includes)
{
	BSLFXCompileResult output;

	// Build a list of different variations and re-parse the source using the relevant defines
	UnorderedSet<String> includeSet;
	for(auto& entry : shaderMetaData)
	{
		const ShaderMetaData& metaData = entry.second;
		if(metaData.IsMixin)
			continue;

		// Generate a list of variations
		Vector<ShaderVariation> variations;

		if(metaData.Variations.empty())
			variations.push_back(ShaderVariation());
		else
		{
			Vector<const VariationData*> todo;
			for(u32 i = 0; i < (u32)metaData.Variations.size(); i++)
				todo.push_back(&metaData.Variations[i]);

			while(!todo.empty())
			{
				const VariationData* current = todo.back();
				todo.erase(todo.end() - 1);

				// Variation parameter that's either defined or isn't
				if(current->Values.empty())
				{
					// This is the first variation parameter, register new variations
					if(variations.empty())
					{
						ShaderVariation a;
						ShaderVariation b;

						b.AddParam(ShaderVariation::Param(current->Identifier, 1));

						variations.push_back(a);
						variations.push_back(b);
					}
					else // Duplicate existing variations, and add the parameter
					{
						u32 numVariations = (u32)variations.size();
						for(u32 i = 0; i < numVariations; i++)
						{
							// Make a copy
							variations.push_back(variations[i]);

							// Add the parameter to existing variation
							variations[i].AddParam(ShaderVariation::Param(current->Identifier, 1));
						}
					}
				}
				else // Variation parameter with multiple values
				{
					// This is the first variation parameter, register new variations
					if(variations.empty())
					{
						for(u32 i = 0; i < (u32)current->Values.size(); i++)
						{
							ShaderVariation variation;
							variation.AddParam(ShaderVariation::Param(current->Identifier, current->Values[i].Value));

							variations.push_back(variation);
						}
					}
					else // Duplicate existing variations, and add the parameter
					{
						u32 numVariations = (u32)variations.size();
						for(u32 i = 0; i < numVariations; i++)
						{
							for(u32 j = 1; j < (u32)current->Values.size(); j++)
							{
								ShaderVariation copy = variations[i];
								copy.AddParam(ShaderVariation::Param(current->Identifier, current->Values[j].Value));

								variations.push_back(copy);
							}

							variations[i].AddParam(ShaderVariation::Param(current->Identifier, current->Values[0].Value));
						}
					}
				}
			}
		}

		// For every variation, re-parse the file with relevant defines
		for(auto& variation : variations)
		{
			UnorderedMap<String, String> globalDefines = defines;
			UnorderedMap<String, String> variationDefines = variation.GetDefines().GetAll();

			for(auto& define : variationDefines)
				globalDefines[define.first] = define.second;

			ParseState* variationParseState = parseStateCreate();
			output = ParseFx(variationParseState, source.c_str(), globalDefines);

			if(!output.ErrorMessage.empty())
				parseStateDelete(variationParseState);
			else
			{
				Vector<String> codeBlocks;
				RawCode* rawCode = variationParseState->RawCodeBlock[RCT_CodeBlock];
				while(rawCode != nullptr)
				{
					while((i32)codeBlocks.size() <= rawCode->Index)
						codeBlocks.push_back(String());

					codeBlocks[rawCode->Index] = String(rawCode->Code, rawCode->Size);
					rawCode = rawCode->Next;
				}

				output = CompileTechniques(variationParseState, entry.second.Name, codeBlocks, variation, languages, includeSet, shaderDesc);

				if(!output.ErrorMessage.empty())
					return output;
			}
		}
	}

	// Generate a shader from the parsed techniques
	for(auto& entry : includeSet)
		includes.push_back(entry);

	// Verify techniques compile correctly
	bool hasError = false;
	StringStream gpuProgError;
	for(auto& technique : shaderDesc.Techniques)
	{
		if(!technique->IsSupported())
			continue;

		u32 numPasses = technique->GetNumPasses();
		technique->Compile();

		for(u32 i = 0; i < numPasses; i++)
		{
			SPtr<Pass> pass = technique->GetPass(i);

			auto checkCompileStatus = [&](const String& prefix, const SPtr<GpuProgram>& prog)
			{
				if(prog != nullptr)
				{
					prog->BlockUntilCoreInitialized();

					if(!prog->IsCompiled())
					{
						hasError = true;
						gpuProgError << prefix << ": " << prog->GetCompileErrorMessage() << std::endl;
					}
				}
			};

			const SPtr<GraphicsPipelineState>& graphicsPipeline = pass->GetGraphicsPipelineState();
			if(graphicsPipeline)
			{
				checkCompileStatus("Vertex program", graphicsPipeline->GetVertexProgram());
				checkCompileStatus("Fragment program", graphicsPipeline->GetFragmentProgram());
				checkCompileStatus("Geometry program", graphicsPipeline->GetGeometryProgram());
				checkCompileStatus("Hull program", graphicsPipeline->GetHullProgram());
				checkCompileStatus("Domain program", graphicsPipeline->GetDomainProgram());
			}

			const SPtr<ComputePipelineState>& computePipeline = pass->GetComputePipelineState();
			if(computePipeline)
				checkCompileStatus("Compute program", computePipeline->GetProgram());
		}
	}

	if(hasError)
	{
		output.ErrorMessage = "Failed compiling GPU program(s): " + gpuProgError.str();
		output.ErrorLine = 0;
		output.ErrorColumn = 0;
	}

	return output;
}

BSLFXCompileResult BSLFXCompiler::CompileShader(String source, const UnorderedMap<String, String>& defines, ShadingLanguageFlags languages, SHADER_DESC& shaderDesc, Vector<String>& includes)
{
	SPtr<ct::Renderer> renderer = RendererManager::Instance().GetActive();

	// Run the lexer/parser and generate the AST
	ParseState* parseState = parseStateCreate();
	BSLFXCompileResult output = ParseFx(parseState, source.c_str(), defines);

	if(!output.ErrorMessage.empty())
	{
		parseStateDelete(parseState);
		return output;
	}

	// Parse global shader options & shader meta-data
	Vector<pair<ASTFXNode*, ShaderMetaData>> shaderMetaData;
	Vector<SubShaderData> subShaderData;

	output = ParseMetaDataAndOptions(parseState->RootNode, shaderMetaData, subShaderData, shaderDesc);

	if(!output.ErrorMessage.empty())
	{
		parseStateDelete(parseState);
		return output;
	}

	// Parse sub-shader code blocks
	Vector<String> subShaderCodeBlocks;
	RawCode* rawCode = parseState->RawCodeBlock[RCT_SubShaderBlock];
	while(rawCode != nullptr)
	{
		while((i32)subShaderCodeBlocks.size() <= rawCode->Index)
			subShaderCodeBlocks.push_back(String());

		subShaderCodeBlocks[rawCode->Index] = String(rawCode->Code, rawCode->Size);
		rawCode = rawCode->Next;
	}

	parseStateDelete(parseState);

	output = PopulateVariations(shaderMetaData);

	if(!output.ErrorMessage.empty())
		return output;

	// Note: Must be called after populateVariations, to ensure variations from mixins are inherited
	for(auto& entry : shaderMetaData)
	{
		if(entry.second.IsMixin)
			continue;

		PopulateVariationParamInfos(entry.second, shaderDesc);
	}

	output = CompileTechniques(shaderMetaData, source, defines, languages, shaderDesc, includes);

	if(!output.ErrorMessage.empty())
		return output;

	// Parse sub-shaders
	for(auto& entry : subShaderData)
	{
		if(entry.CodeBlockIndex > (u32)subShaderCodeBlocks.size())
			continue;

		const String& subShaderCode = subShaderCodeBlocks[entry.CodeBlockIndex];

		ct::ShaderExtensionPointInfo extPointInfo = renderer->GetShaderExtensionPointInfo(entry.Name);
		for(auto& extPointShader : extPointInfo.Shaders)
		{
			Path path = gBuiltinResources().GetRawShaderFolder();
			path.Append(extPointShader.Path);
			path.SetExtension(path.GetExtension());

			StringStream subShaderSource;
			const UnorderedMap<String, String> subShaderDefines = extPointShader.Defines.GetAll();
			{
				Lock fileLock = FileScheduler::GetLock(path);

				SPtr<DataStream> stream = FileSystem::OpenFile(path);
				if(stream)
					subShaderSource << stream->GetAsString();
			}

			subShaderSource << "\n";
			subShaderSource << subShaderCode;

			SHADER_DESC subShaderDesc;
			Vector<String> subShaderIncludes;
			BSLFXCompileResult subShaderOutput = CompileShader(subShaderSource.str(), subShaderDefines, languages, subShaderDesc, subShaderIncludes);

			if(!subShaderOutput.ErrorMessage.empty())
				return subShaderOutput;

			// Clear the sub-shader descriptor of any data other than techniques
			Vector<SPtr<Technique>> techniques = subShaderDesc.Techniques;
			subShaderDesc = SHADER_DESC();
			subShaderDesc.Techniques = techniques;

			SubShader subShader;
			subShader.Name = extPointShader.Name;
			subShader.Shader = Shader::CreatePtrInternal(subShader.Name, subShaderDesc);

			shaderDesc.SubShaders.push_back(subShader);
		}
	}

	return output;
}

BSLFXCompileResult BSLFXCompiler::CompileTechniques(ParseState* parseState, const String& name, const Vector<String>& codeBlocks, const ShaderVariation& variation, ShadingLanguageFlags languages, UnorderedSet<String>& includes, SHADER_DESC& shaderDesc)
{
	BSLFXCompileResult output;

	if(parseState->RootNode == nullptr || parseState->RootNode->Type != NT_Root)
	{
		parseStateDelete(parseState);

		output.ErrorMessage = "Root is null or not a shader.";
		return output;
	}

	Vector<pair<ASTFXNode*, ShaderData>> shaderData;

	// Go in reverse because options are added in reverse order during parsing
	for(int i = parseState->RootNode->Options->Count - 1; i >= 0; i--)
	{
		NodeOption* option = &parseState->RootNode->Options->Entries[i];

		switch(option->Type)
		{
		case OT_Shader:
			{
				// We initially parse only meta-data, so we can handle out-of-order technique definitions
				ShaderMetaData metaData = ParseShaderMetaData(option->Value.NodePtr);

				// Skip all techniques except the one we're parsing
				if(metaData.Name != name && !metaData.IsMixin)
					continue;

				shaderData.push_back(std::make_pair(option->Value.NodePtr, ShaderData()));
				ShaderData& data = shaderData.back().second;
				data.MetaData = metaData;

				break;
			}
		default:
			break;
		}
	}

	bool* mixinWasParsed = bs_stack_alloc<bool>((u32)shaderData.size());
	std::function<bool(const ShaderMetaData&, ShaderData&)> parseInherited =
		[&](const ShaderMetaData& metaData, ShaderData& outShader)
	{
		for(auto riter = metaData.Includes.rbegin(); riter != metaData.Includes.rend(); ++riter)
		{
			const String& includes = *riter;

			u32 baseIdx = -1;
			for(u32 i = 0; i < (u32)shaderData.size(); i++)
			{
				auto& entry = shaderData[i];
				if(!entry.second.MetaData.IsMixin)
					continue;

				if(entry.second.MetaData.Name == includes)
				{
					bool matches =
						(entry.second.MetaData.Language == metaData.Language ||
						 entry.second.MetaData.Language == "Any");

					// We want the last matching mixin, in order to allow mixins to override each other
					if(matches)
						baseIdx = i;
				}
			}

			if(baseIdx != (u32)-1)
			{
				auto& entry = shaderData[baseIdx];

				// Was already parsed previously, don't parse it multiple times (happens when multiple mixins
				// include the same mixin)
				if(mixinWasParsed[baseIdx])
					continue;

				if(!parseInherited(entry.second.MetaData, outShader))
					return false;

				ParseShader(entry.first, codeBlocks, outShader);
				mixinWasParsed[baseIdx] = true;
			}
			else
			{
				output.ErrorMessage = "Mixin \"" + includes + "\" cannot be found.";
				return false;
			}
		}

		return true;
	};

	// Actually parse shaders
	for(auto& entry : shaderData)
	{
		const ShaderMetaData& metaData = entry.second.MetaData;
		if(metaData.IsMixin)
			continue;

		bs_zero_out(mixinWasParsed, shaderData.size());
		if(!parseInherited(metaData, entry.second))
		{
			parseStateDelete(parseState);
			bs_stack_free(mixinWasParsed);
			return output;
		}

		ParseShader(entry.first, codeBlocks, entry.second);
	}

	bs_stack_free(mixinWasParsed);

	IncludeLink* includeLink = parseState->Includes;
	while(includeLink != nullptr)
	{
		String includeFilename = includeLink->Data->Filename;

		auto iterFind = std::find(includes.begin(), includes.end(), includeFilename);
		if(iterFind == includes.end())
			includes.insert(includeFilename);

		includeLink = includeLink->Next;
	}

	parseStateDelete(parseState);

	// Parse extended HLSL code and generate per-program code, also convert to GLSL/VKSL/MSL
	const auto end = (u32)shaderData.size();
	Vector<pair<ASTFXNode*, ShaderData>> outputShaderData;
	for(u32 i = 0; i < end; i++)
	{
		const ShaderMetaData& metaData = shaderData[i].second.MetaData;
		if(metaData.IsMixin)
			continue;

		ShaderData& shaderDataEntry = shaderData[i].second;

		ShaderData hlslShaderData = shaderData[i].second;
		ShaderData glslShaderData = shaderData[i].second;

		// When working with OpenGL, lower-end feature sets are supported. For other backends, high-end is always assumed.
		CrossCompileOutput glslVersion = CrossCompileOutput::GLSL41;
		if(glslShaderData.MetaData.FeatureSet == "HighEnd")
		{
			glslShaderData.MetaData.Language = "glsl";
			glslVersion = CrossCompileOutput::GLSL45;
		}
		else
			glslShaderData.MetaData.Language = "glsl4_1";

		ShaderData vkslShaderData = shaderData[i].second;
		vkslShaderData.MetaData.Language = "vksl";

		ShaderData mvksl = shaderData[i].second;
		mvksl.MetaData.Language = "mvksl";

		const auto numPasses = (u32)shaderDataEntry.Passes.size();
		for(u32 j = 0; j < numPasses; j++)
		{
			PassData& passData = shaderDataEntry.Passes[j];

			// Find valid entry points and parameters
			// Note: XShaderCompiler needs to do a full pass when doing reflection, and for each individual program
			// type. If performance is ever important here it could be good to update XShaderCompiler so it can
			// somehow save the AST and then re-use it for multiple actions.
			Vector<GpuProgramType> types;
			reflectHLSL(passData.Code, shaderDesc, types);

			auto crossCompilePass = [&types](PassData& passData, CrossCompileOutput language)
			{
				u32 binding = 0;

				for(auto& type : types)
				{
					switch(type)
					{
					case GPT_VERTEX_PROGRAM:
						passData.VertexCode = crossCompile(passData.Code, GPT_VERTEX_PROGRAM, language, binding);
						break;
					case GPT_FRAGMENT_PROGRAM:
						passData.FragmentCode = crossCompile(passData.Code, GPT_FRAGMENT_PROGRAM, language, binding);
						break;
					case GPT_GEOMETRY_PROGRAM:
						passData.GeometryCode = crossCompile(passData.Code, GPT_GEOMETRY_PROGRAM, language, binding);
						break;
					case GPT_HULL_PROGRAM:
						passData.HullCode = crossCompile(passData.Code, GPT_HULL_PROGRAM, language, binding);
						break;
					case GPT_DOMAIN_PROGRAM:
						passData.DomainCode = crossCompile(passData.Code, GPT_DOMAIN_PROGRAM, language, binding);
						break;
					case GPT_COMPUTE_PROGRAM:
						passData.ComputeCode = crossCompile(passData.Code, GPT_COMPUTE_PROGRAM, language, binding);
						break;
					default:
						break;
					}
				}
			};

			if(languages.IsSet(ShadingLanguageFlag::GLSL))
				crossCompilePass(glslShaderData.Passes[j], glslVersion);

			if(languages.IsSet(ShadingLanguageFlag::VKSL))
				crossCompilePass(vkslShaderData.Passes[j], CrossCompileOutput::VKSL45);

			if(languages.IsSet(ShadingLanguageFlag::MSL))
				crossCompilePass(mvksl.Passes[j], CrossCompileOutput::MVKSL);

			if(languages.IsSet(ShadingLanguageFlag::HLSL))
			{
				PassData& hlslPassData = hlslShaderData.Passes[j];

				// Clean non-standard HLSL
				// Note: Ideally we add a full HLSL output module to XShaderCompiler, instead of using simple regex. This
				// way the syntax could be enhanced with more complex features, while still being able to output pure
				// HLSL.
				static const std::regex attrRegex(
					R"(\[\s*layout\s*\(.*\)\s*\]|\[\s*internal\s*\]|\[\s*color\s*\]|\[\s*alias\s*\(.*\)\s*\]|\[\s*spriteuv\s*\(.*\)\s*\])");
				hlslPassData.Code = regex_replace(hlslPassData.Code, attrRegex, "");

				static const std::regex attr2Regex(
					R"(\[\s*hideInInspector\s*\]|\[\s*name\s*\(".*"\)\s*\]|\[\s*hdr\s*\])");
				hlslPassData.Code = regex_replace(hlslPassData.Code, attr2Regex, "");

				static const std::regex initializerRegex(
					R"(Texture2D\s*(\S*)\s*=.*;)");
				hlslPassData.Code = regex_replace(hlslPassData.Code, initializerRegex, "Texture2D $1;");

				static const std::regex warpWithSyncRegex(
					R"(Warp(Group|Device|All)MemoryBarrierWithWarpSync)");
				hlslPassData.Code = regex_replace(hlslPassData.Code, warpWithSyncRegex, "$1MemoryBarrierWithGroupSync");

				static const std::regex warpNoSyncRegex(
					R"(Warp(Group|Device|All)MemoryBarrier)");
				hlslPassData.Code = regex_replace(hlslPassData.Code, warpNoSyncRegex, "$1MemoryBarrier");

				// Note: I'm just copying HLSL code as-is. This code will contain all entry points which could have
				// an effect on compile time. It would be ideal to remove dead code depending on program type. This would
				// involve adding a HLSL code generator to XShaderCompiler.
				for(auto& type : types)
				{
					switch(type)
					{
					case GPT_VERTEX_PROGRAM:
						hlslPassData.VertexCode = hlslPassData.Code;
						break;
					case GPT_FRAGMENT_PROGRAM:
						hlslPassData.FragmentCode = hlslPassData.Code;
						break;
					case GPT_GEOMETRY_PROGRAM:
						hlslPassData.GeometryCode = hlslPassData.Code;
						break;
					case GPT_HULL_PROGRAM:
						hlslPassData.HullCode = hlslPassData.Code;
						break;
					case GPT_DOMAIN_PROGRAM:
						hlslPassData.DomainCode = hlslPassData.Code;
						break;
					case GPT_COMPUTE_PROGRAM:
						hlslPassData.ComputeCode = hlslPassData.Code;
						break;
					default:
						break;
					}
				}
			}
		}

		outputShaderData.push_back(std::make_pair(nullptr, hlslShaderData));
		outputShaderData.push_back(std::make_pair(nullptr, glslShaderData));
		outputShaderData.push_back(std::make_pair(nullptr, vkslShaderData));
		outputShaderData.push_back(std::make_pair(nullptr, mvksl));
	}

	for(auto& entry : outputShaderData)
	{
		const ShaderMetaData& metaData = entry.second.MetaData;
		if(metaData.IsMixin)
			continue;

		Map<u32, SPtr<Pass>, std::greater<u32>> passes;
		for(auto& passData : entry.second.Passes)
		{
			PASS_DESC passDesc;
			passDesc.BlendStateDesc = passData.BlendDesc;
			passDesc.RasterizerStateDesc = passData.RasterizerDesc;
			passDesc.DepthStencilStateDesc = passData.DepthStencilDesc;

			auto createProgram =
				[](const String& language, const String& entry, const String& code, GpuProgramType type) -> GPU_PROGRAM_DESC
			{
				GPU_PROGRAM_DESC desc;
				desc.Language = language;
				desc.EntryPoint = entry;
				desc.Source = code;
				desc.Type = type;

				return desc;
			};

			bool isHLSL = metaData.Language == "hlsl";
			passDesc.VertexProgramDesc = createProgram(
				metaData.Language,
				isHLSL ? "vsmain" : "main",
				passData.VertexCode,
				GPT_VERTEX_PROGRAM);

			passDesc.FragmentProgramDesc = createProgram(
				metaData.Language,
				isHLSL ? "fsmain" : "main",
				passData.FragmentCode,
				GPT_FRAGMENT_PROGRAM);

			passDesc.GeometryProgramDesc = createProgram(
				metaData.Language,
				isHLSL ? "gsmain" : "main",
				passData.GeometryCode,
				GPT_GEOMETRY_PROGRAM);

			passDesc.HullProgramDesc = createProgram(
				metaData.Language,
				isHLSL ? "hsmain" : "main",
				passData.HullCode,
				GPT_HULL_PROGRAM);

			passDesc.DomainProgramDesc = createProgram(
				metaData.Language,
				isHLSL ? "dsmain" : "main",
				passData.DomainCode,
				GPT_DOMAIN_PROGRAM);

			passDesc.ComputeProgramDesc = createProgram(
				metaData.Language,
				isHLSL ? "csmain" : "main",
				passData.ComputeCode,
				GPT_COMPUTE_PROGRAM);

			passDesc.StencilRefValue = passData.StencilRefValue;

			SPtr<Pass> pass = Pass::Create(passDesc);
			if(pass != nullptr)
				passes[passData.SeqIdx] = pass;
		}

		Vector<SPtr<Pass>> orderedPasses;
		for(auto& KVP : passes)
			orderedPasses.push_back(KVP.second);

		if(!orderedPasses.empty())
		{
			SPtr<Technique> technique = Technique::Create(metaData.Language, metaData.Tags, variation, orderedPasses);
			shaderDesc.Techniques.push_back(technique);
		}
	}

	return output;
}

String BSLFXCompiler::RemoveQuotes(const char* input)
{
	u32 len = (u32)strlen(input);
	String output(len - 2, ' ');

	for(u32 i = 0; i < (len - 2); i++)
		output[i] = input[i + 1];

	return output;
}
