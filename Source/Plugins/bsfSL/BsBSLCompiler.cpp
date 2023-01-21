//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsBSLCompiler.h"
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
#include "BsBSLParser.h"
#include "Xsc/Xsc.h"

using namespace std;
using namespace bs;

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
				String s = ToString((u32)reports.size()) + " " + headline;
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

GpuParameterObjectType ReflTypeToTextureType(Xsc::Reflection::BufferType type)
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

GpuParameterObjectType ReflTypeToBufferType(Xsc::Reflection::BufferType type)
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

GpuDataParameterType ReflTypeToDataType(Xsc::Reflection::DataType type)
{
	switch(type)
	{
	case Xsc::Reflection::DataType::Bool: return GPDT_BOOL;
	case Xsc::Reflection::DataType::Float: return GPDT_FLOAT1;
	case Xsc::Reflection::DataType::Float2: return GPDT_FLOAT2;
	case Xsc::Reflection::DataType::Float3: return GPDT_FLOAT3;
	case Xsc::Reflection::DataType::Float4: return GPDT_FLOAT4;
	case Xsc::Reflection::DataType::Double: return GPDT_DOUBLE1;
	case Xsc::Reflection::DataType::Double2: return GPDT_DOUBLE2;
	case Xsc::Reflection::DataType::Double3: return GPDT_DOUBLE3;
	case Xsc::Reflection::DataType::Double4: return GPDT_DOUBLE4;
	case Xsc::Reflection::DataType::Half: return GPDT_HALF1;
	case Xsc::Reflection::DataType::Half2: return GPDT_HALF2;
	case Xsc::Reflection::DataType::Half3: return GPDT_HALF3;
	case Xsc::Reflection::DataType::Half4: return GPDT_HALF4;
	case Xsc::Reflection::DataType::Int: return GPDT_INT1;
	case Xsc::Reflection::DataType::Int2: return GPDT_INT2;
	case Xsc::Reflection::DataType::Int3: return GPDT_INT3;
	case Xsc::Reflection::DataType::Int4: return GPDT_INT4;
	case Xsc::Reflection::DataType::UInt: return GPDT_UINT1;
	case Xsc::Reflection::DataType::UInt2: return GPDT_UINT2;
	case Xsc::Reflection::DataType::UInt3: return GPDT_UINT3;
	case Xsc::Reflection::DataType::UInt4: return GPDT_UINT4;
	case Xsc::Reflection::DataType::Float2x2: return GPDT_MATRIX_2X2;
	case Xsc::Reflection::DataType::Float2x3: return GPDT_MATRIX_2X3;
	case Xsc::Reflection::DataType::Float2x4: return GPDT_MATRIX_2X4;
	case Xsc::Reflection::DataType::Float3x2: return GPDT_MATRIX_3X4;
	case Xsc::Reflection::DataType::Float3x3: return GPDT_MATRIX_3X3;
	case Xsc::Reflection::DataType::Float3x4: return GPDT_MATRIX_3X4;
	case Xsc::Reflection::DataType::Float4x2: return GPDT_MATRIX_4X2;
	case Xsc::Reflection::DataType::Float4x3: return GPDT_MATRIX_4X3;
	case Xsc::Reflection::DataType::Float4x4: return GPDT_MATRIX_4X4;
	case Xsc::Reflection::DataType::Double2x2: return GPDT_DOUBLE_MATRIX_2X2;
	case Xsc::Reflection::DataType::Double2x3: return GPDT_DOUBLE_MATRIX_2X3;
	case Xsc::Reflection::DataType::Double2x4: return GPDT_DOUBLE_MATRIX_2X4;
	case Xsc::Reflection::DataType::Double3x2: return GPDT_DOUBLE_MATRIX_3X4;
	case Xsc::Reflection::DataType::Double3x3: return GPDT_DOUBLE_MATRIX_3X3;
	case Xsc::Reflection::DataType::Double3x4: return GPDT_DOUBLE_MATRIX_3X4;
	case Xsc::Reflection::DataType::Double4x2: return GPDT_DOUBLE_MATRIX_4X2;
	case Xsc::Reflection::DataType::Double4x3: return GPDT_DOUBLE_MATRIX_4X3;
	case Xsc::Reflection::DataType::Double4x4: return GPDT_DOUBLE_MATRIX_4X4;
	case Xsc::Reflection::DataType::Half2x2: return GPDT_HALF_MATRIX_2X2;
	case Xsc::Reflection::DataType::Half2x3: return GPDT_HALF_MATRIX_2X3;
	case Xsc::Reflection::DataType::Half2x4: return GPDT_HALF_MATRIX_2X4;
	case Xsc::Reflection::DataType::Half3x2: return GPDT_HALF_MATRIX_3X4;
	case Xsc::Reflection::DataType::Half3x3: return GPDT_HALF_MATRIX_3X3;
	case Xsc::Reflection::DataType::Half3x4: return GPDT_HALF_MATRIX_3X4;
	case Xsc::Reflection::DataType::Half4x2: return GPDT_HALF_MATRIX_4X2;
	case Xsc::Reflection::DataType::Half4x3: return GPDT_HALF_MATRIX_4X3;
	case Xsc::Reflection::DataType::Half4x4: return GPDT_HALF_MATRIX_4X4;
	default: return GPDT_UNKNOWN;
	}
}

static HTexture GetBuiltinTexture(u32 idx)
{
	if(idx == 1)
		return BuiltinResources::GetTexture(BuiltinTexture::White);
	else if(idx == 2)
		return BuiltinResources::GetTexture(BuiltinTexture::Black);
	else if(idx == 3)
		return BuiltinResources::GetTexture(BuiltinTexture::Normal);

	return HTexture();
}

static HTexture GetBuiltin3DTexture(u32 index)
{
	if (index == 1)
		return BuiltinResources::GetTexture(BuiltinTexture::White3D);
	else if (index == 2)
		return BuiltinResources::GetTexture(BuiltinTexture::Black3D);

	return HTexture();
}


u32 GetStructSize(i32 structIdx, const std::vector<Xsc::Reflection::Struct>& structLookup)
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
			GpuDataParameterType type = ReflTypeToDataType((Xsc::Reflection::DataType)entry.baseType);

			const GpuDataParameterTypeInformation& typeInfo = GpuParams::kParamSizes.Lookup[(int)type];
			size += typeInfo.NumColumns * typeInfo.NumRows * typeInfo.BaseTypeSize * entry.arraySize;
		}
		else if(entry.type == Xsc::Reflection::VariableType::Struct)
			size += GetStructSize(entry.baseType, structLookup);
	}

	return size;
}

TextureAddressingMode ParseTexAddrMode(Xsc::Reflection::TextureAddressMode addrMode)
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

CompareFunction ParseCompFunction(Xsc::Reflection::ComparisonFunc compFunc)
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

static SAMPLER_STATE_DESC ParseSamplerState(const Xsc::Reflection::SamplerState& samplerReflectionInformation)
{
	SAMPLER_STATE_DESC samplerCreateInformation;

	samplerCreateInformation.AddressMode.U = ParseTexAddrMode(samplerReflectionInformation.addressU);
	samplerCreateInformation.AddressMode.V = ParseTexAddrMode(samplerReflectionInformation.addressV);
	samplerCreateInformation.AddressMode.W = ParseTexAddrMode(samplerReflectionInformation.addressW);

	samplerCreateInformation.BorderColor[0] = samplerReflectionInformation.borderColor[0];
	samplerCreateInformation.BorderColor[1] = samplerReflectionInformation.borderColor[1];
	samplerCreateInformation.BorderColor[2] = samplerReflectionInformation.borderColor[2];
	samplerCreateInformation.BorderColor[3] = samplerReflectionInformation.borderColor[3];

	samplerCreateInformation.ComparisonFunc = ParseCompFunction(samplerReflectionInformation.comparisonFunc);
	samplerCreateInformation.MaxAniso = samplerReflectionInformation.maxAnisotropy;
	samplerCreateInformation.MipMax = samplerReflectionInformation.maxLOD;
	samplerCreateInformation.MipMin = samplerReflectionInformation.minLOD;
	samplerCreateInformation.MipmapBias = samplerReflectionInformation.mipLODBias;

	switch(samplerReflectionInformation.filter)
	{
	case Xsc::Reflection::Filter::MinMagMipPoint:
	case Xsc::Reflection::Filter::ComparisonMinMagMipPoint:
		samplerCreateInformation.MinFilter = FO_POINT;
		samplerCreateInformation.MagFilter = FO_POINT;
		samplerCreateInformation.MipFilter = FO_POINT;
		break;
	case Xsc::Reflection::Filter::MinMagPointMipLinear:
	case Xsc::Reflection::Filter::ComparisonMinMagPointMipLinear:
		samplerCreateInformation.MinFilter = FO_POINT;
		samplerCreateInformation.MagFilter = FO_POINT;
		samplerCreateInformation.MipFilter = FO_LINEAR;
		break;
	case Xsc::Reflection::Filter::MinPointMagLinearMipPoint:
	case Xsc::Reflection::Filter::ComparisonMinPointMagLinearMipPoint:
		samplerCreateInformation.MinFilter = FO_POINT;
		samplerCreateInformation.MagFilter = FO_LINEAR;
		samplerCreateInformation.MipFilter = FO_POINT;
		break;
	case Xsc::Reflection::Filter::MinPointMagMipLinear:
	case Xsc::Reflection::Filter::ComparisonMinPointMagMipLinear:
		samplerCreateInformation.MinFilter = FO_POINT;
		samplerCreateInformation.MagFilter = FO_LINEAR;
		samplerCreateInformation.MipFilter = FO_LINEAR;
		break;
	case Xsc::Reflection::Filter::MinLinearMagMipPoint:
	case Xsc::Reflection::Filter::ComparisonMinLinearMagMipPoint:
		samplerCreateInformation.MinFilter = FO_LINEAR;
		samplerCreateInformation.MagFilter = FO_POINT;
		samplerCreateInformation.MipFilter = FO_POINT;
		break;
	case Xsc::Reflection::Filter::MinLinearMagPointMipLinear:
	case Xsc::Reflection::Filter::ComparisonMinLinearMagPointMipLinear:
		samplerCreateInformation.MinFilter = FO_LINEAR;
		samplerCreateInformation.MagFilter = FO_POINT;
		samplerCreateInformation.MipFilter = FO_LINEAR;
		break;
	case Xsc::Reflection::Filter::MinMagLinearMipPoint:
	case Xsc::Reflection::Filter::ComparisonMinMagLinearMipPoint:
		samplerCreateInformation.MinFilter = FO_LINEAR;
		samplerCreateInformation.MagFilter = FO_LINEAR;
		samplerCreateInformation.MipFilter = FO_POINT;
		break;
	case Xsc::Reflection::Filter::MinMagMipLinear:
	case Xsc::Reflection::Filter::ComparisonMinMagMipLinear:
		samplerCreateInformation.MinFilter = FO_LINEAR;
		samplerCreateInformation.MagFilter = FO_LINEAR;
		samplerCreateInformation.MipFilter = FO_LINEAR;
		break;
	case Xsc::Reflection::Filter::Anisotropic:
	case Xsc::Reflection::Filter::ComparisonAnisotropic:
		samplerCreateInformation.MinFilter = FO_ANISOTROPIC;
		samplerCreateInformation.MagFilter = FO_ANISOTROPIC;
		samplerCreateInformation.MipFilter = FO_ANISOTROPIC;
		break;
	default:
		break;
	}

	return samplerCreateInformation;
}

static bool ParseParameters(const Xsc::Reflection::ReflectionData& reflectionData, BSLResult& outCompileResult, ShaderCreateInformation& outShaderCreateInformation)
{
	for(auto& entry : reflectionData.uniforms)
	{
		if((entry.flags & Xsc::Reflection::Uniform::Flags::Internal) != 0)
			continue;

		String ident = entry.ident.c_str();
		bool isBlockHiddenInInspector = false;
		auto parseCommonAttributes = [&entry, &ident, &outShaderCreateInformation, &isBlockHiddenInInspector]()
		{
			if(!entry.readableName.empty())
			{
				ShaderParameterAttribute attribute;
				attribute.Value.assign(entry.readableName.data(), entry.readableName.size());
				attribute.NextParameterIndex = (u32)-1;
				attribute.Type = ShaderParamAttributeType::Name;

				outShaderCreateInformation.SetParameterAttribute(ident, attribute);
			}

			if((entry.flags & Xsc::Reflection::Uniform::Flags::HideInInspector) != 0 || isBlockHiddenInInspector)
			{
				ShaderParameterAttribute attribute;
				attribute.NextParameterIndex = (u32)-1;
				attribute.Type = ShaderParamAttributeType::HideInInspector;

				outShaderCreateInformation.SetParameterAttribute(ident, attribute);
			}

			if((entry.flags & Xsc::Reflection::Uniform::Flags::HDR) != 0)
			{
				ShaderParameterAttribute attribute;
				attribute.NextParameterIndex = (u32)-1;
				attribute.Type = ShaderParamAttributeType::HDR;

				outShaderCreateInformation.SetParameterAttribute(ident, attribute);
			}
		};

		switch(entry.type)
		{
		case Xsc::Reflection::VariableType::UniformBuffer:
			outShaderCreateInformation.SetParamBlockAttribs(entry.ident.c_str(), false, GBU_STATIC);
			break;
		case Xsc::Reflection::VariableType::Buffer:
			{
				GpuParameterObjectType objType = ReflTypeToTextureType((Xsc::Reflection::BufferType)entry.baseType);
				if(objType != GPOT_UNKNOWN)
				{
					const bool hasDefaultValue = entry.defaultValue == -1;
					HTexture defaultValue;

					if (!hasDefaultValue)
					{
						const Xsc::Reflection::DefaultValue& reflectedDefaultValue = reflectionData.defaultValues[entry.defaultValue];
						defaultValue = objType == GPOT_TEXTURE3D ? GetBuiltin3DTexture(reflectedDefaultValue.integer) : GetBuiltinTexture(reflectedDefaultValue.integer);
					}

					// Warn if parameter was already registered in some previous variation with a different value
					if(auto foundTextureParameter = outShaderCreateInformation.TextureParams.find(ident); foundTextureParameter != outShaderCreateInformation.TextureParams.end())
					{
						const bool isExistingValueDefault = foundTextureParameter->second.DefaultValueIndex == ~0u;
						if (hasDefaultValue != isExistingValueDefault)
						{
							outCompileResult.ErrorMessage = StringUtil::Format("Shader cross compilation failed. Texture parameter '{0}' has a different default value across variations.", entry.ident.c_str());
							return false;
						}

						if (!hasDefaultValue)
						{
							const HTexture existingTexture = outShaderCreateInformation.TextureDefaultValues[foundTextureParameter->second.DefaultValueIndex];
							if (existingTexture != defaultValue)
							{
								outCompileResult.ErrorMessage = StringUtil::Format("Shader cross compilation failed. Texture parameter '{0}' has a different default value across variations.", entry.ident.c_str());
								return false;
							}
						}

						continue;
					}

					if(entry.defaultValue == -1)
						outShaderCreateInformation.AddParameter(ShaderObjectParameterInformation(ident, ident, objType, StringID::kNone, entry.arraySize));
					else
						outShaderCreateInformation.AddParameter(ShaderObjectParameterInformation(ident, ident, objType, StringID::kNone, entry.arraySize), defaultValue);

					parseCommonAttributes();
				}
				else
				{
					// Ignore parameters that were already registered in some previous variation. Note that this implies
					// you cannot have same names for different parameters in different variations.
					if(outShaderCreateInformation.BufferParams.find(ident) != outShaderCreateInformation.BufferParams.end())
						continue;

					objType = ReflTypeToBufferType((Xsc::Reflection::BufferType)entry.baseType);
					outShaderCreateInformation.AddParameter(ShaderObjectParameterInformation(ident, ident, objType, StringID::kNone, entry.arraySize));

					parseCommonAttributes();
				}
			}
			break;
		case Xsc::Reflection::VariableType::Sampler:
			{
				if(auto foundSamplerReflectionData = reflectionData.samplerStates.find(entry.ident); foundSamplerReflectionData != reflectionData.samplerStates.end())
				{
					SAMPLER_STATE_DESC defaultSamplerStateCreateInformation;
					if (foundSamplerReflectionData->second.isNonDefault)
						defaultSamplerStateCreateInformation = ParseSamplerState(foundSamplerReflectionData->second);

					if (auto foundSamplerParameter = outShaderCreateInformation.SamplerParams.find(ident); foundSamplerParameter != outShaderCreateInformation.SamplerParams.end())
					{
						const bool isExistingValueNonDefault = foundSamplerParameter->second.DefaultValueIndex != ~0u;
						if (foundSamplerReflectionData->second.isNonDefault != isExistingValueNonDefault)
						{
							outCompileResult.ErrorMessage = StringUtil::Format("Shader cross compilation failed. Sampler parameter '{0}' has a different default value across variations.", entry.ident.c_str());
							return false;
						}

						if (foundSamplerReflectionData->second.isNonDefault)
						{
							const SPtr<const SamplerState> existingSamplerState = outShaderCreateInformation.SamplerDefaultValues[foundSamplerParameter->second.DefaultValueIndex];
							if (existingSamplerState->GetProperties().GetDesc() != defaultSamplerStateCreateInformation)
							{
								outCompileResult.ErrorMessage = StringUtil::Format("Shader cross compilation failed. Sampler parameter '{0}' has a different default value across variations.", entry.ident.c_str());
								return false;
							}
						}

						continue;
					}

					const String alias = foundSamplerReflectionData->second.alias.c_str();
					if(foundSamplerReflectionData->second.isNonDefault)
					{
						const SPtr<SamplerState> defaultValue = SamplerState::Create(defaultSamplerStateCreateInformation);
						outShaderCreateInformation.AddParameter(ShaderObjectParameterInformation(ident, ident, GPOT_SAMPLER2D), defaultValue);

						if(!alias.empty())
							outShaderCreateInformation.AddParameter(ShaderObjectParameterInformation(ident, alias, GPOT_SAMPLER2D), defaultValue);
					}
					else
					{
						// Ignore parameters that were already registered in some previous variation. Note that this implies
						// you cannot have same names for different parameters in different variations.
						if (outShaderCreateInformation.SamplerParams.find(ident) != outShaderCreateInformation.SamplerParams.end())
							continue;

						outShaderCreateInformation.AddParameter(ShaderObjectParameterInformation(ident, ident, GPOT_SAMPLER2D));

						if(!alias.empty())
							outShaderCreateInformation.AddParameter(ShaderObjectParameterInformation(ident, alias, GPOT_SAMPLER2D));
					}
				}
				else
				{
					outShaderCreateInformation.AddParameter(ShaderObjectParameterInformation(ident, ident, GPOT_SAMPLER2D));
				}
				break;
			}
		case Xsc::Reflection::VariableType::Variable:
			{
				bool isBlockInternal = false;
				if(entry.uniformBlock != -1)
				{
					std::string blockName = reflectionData.constantBuffers[entry.uniformBlock].ident;
					for(auto& uniform : reflectionData.uniforms)
					{
						if(uniform.type == Xsc::Reflection::VariableType::UniformBuffer && uniform.ident == blockName)
						{
							isBlockInternal = (uniform.flags & Xsc::Reflection::Uniform::Flags::Internal) != 0;
							isBlockHiddenInInspector = (uniform.flags & Xsc::Reflection::Uniform::Flags::HideInInspector) != 0;
							break;
						}
					}
				}

				if(!isBlockInternal)
				{
					GpuDataParameterType type = ReflTypeToDataType((Xsc::Reflection::DataType)entry.baseType);
					if((entry.flags & Xsc::Reflection::Uniform::Flags::Color) != 0 &&
					   (type == GPDT_FLOAT3 || type == GPDT_FLOAT4))
					{
						type = GPDT_COLOR;
					}

					u32 arraySize = entry.arraySize;

					if(entry.defaultValue == -1)
						outShaderCreateInformation.AddParameter(ShaderDataParameterInformation(ident, ident, type, StringID::kNone, arraySize));
					else
					{
						const Xsc::Reflection::DefaultValue& defVal = reflectionData.defaultValues[entry.defaultValue];

						outShaderCreateInformation.AddParameter(ShaderDataParameterInformation(ident, ident, type, StringID::kNone, arraySize, 0), (u8*)defVal.matrix);
					}

					if(!entry.spriteUVRef.empty() && (type == GPDT_FLOAT4))
					{
						ShaderParameterAttribute attribute;
						attribute.Value.assign(entry.spriteUVRef.data(), entry.spriteUVRef.size());
						attribute.NextParameterIndex = (u32)-1;
						attribute.Type = ShaderParamAttributeType::SpriteUV;

						outShaderCreateInformation.SetParameterAttribute(ident, attribute);
					}

					parseCommonAttributes();
				}
			}
			break;
		case Xsc::Reflection::VariableType::Struct:
			{
				i32 structIdx = entry.baseType;
				u32 structSize = GetStructSize(structIdx, reflectionData.structs);

				outShaderCreateInformation.AddParameter(ShaderDataParameterInformation(ident, ident, GPDT_STRUCT, StringID::kNone, entry.arraySize, structSize));
			}
			break;
		default:;
		}
	}

	return true;
}

/** Types of supported code output when cross compiling HLSL to GLSL. */
enum class CrossCompileOutput
{
	GLSL45,
	GLSL41,
	VKSL45,
	MVKSL
};

String CrossCompile(const String& hlsl, GpuProgramType type, CrossCompileOutput outputType, bool optionalEntry, u32& startBindingSlot, BSLResult& outCompileResult, ShaderCreateInformation* shaderDesc = nullptr, SmallVector<GpuProgramType, 2>* detectedTypes = nullptr)
{
	SPtr<StringStream> input = B3DMakeShared<StringStream>();

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

	// Clear '\r' as it's breaking XShaderCompiler when used in mutiline preprocessor statements
	for (const char& currentCharacter : hlsl)
	{
		if (currentCharacter == '\r')
			continue;

		*input << currentCharacter;
	}

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

			outCompileResult.ErrorMessage = StringUtil::Format("Shader cross compilation failed. Log: \n\n{0}", logOutput.str());
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
				detectedTypes->Add(GPT_VERTEX_PROGRAM);
			else if(entry.ident == "fsmain")
				detectedTypes->Add(GPT_FRAGMENT_PROGRAM);
			else if(entry.ident == "gsmain")
				detectedTypes->Add(GPT_GEOMETRY_PROGRAM);
			else if(entry.ident == "dsmain")
				detectedTypes->Add(GPT_DOMAIN_PROGRAM);
			else if(entry.ident == "hsmain")
				detectedTypes->Add(GPT_HULL_PROGRAM);
			else if(entry.ident == "csmain")
				detectedTypes->Add(GPT_COMPUTE_PROGRAM);
		}

		// If no entry points found, and error occurred, report error
		if(!compileSuccess && detectedTypes->Empty())
		{
			StringStream logOutput;
			log.GetMessages(logOutput);

			outCompileResult.ErrorMessage = StringUtil::Format("Shader cross compilation failed. Log: \n\n{0}", logOutput.str());
			return "";
		}
	}

	if (shaderDesc != nullptr)
	{
		if (!ParseParameters(reflectionData, outCompileResult, *shaderDesc))
			return "";
	}

	return output.str();
}

String CrossCompile(const String& hlsl, GpuProgramType type, CrossCompileOutput outputType, u32& startBindingSlot, BSLResult& outCompileResult)
{
	return CrossCompile(hlsl, type, outputType, false, startBindingSlot, outCompileResult);
}

void ReflectHlsl(const String& hlsl, ShaderCreateInformation& shaderDesc, SmallVector<GpuProgramType, 2>& entryPoints, BSLResult& outCompileResult)
{
	u32 dummy = 0;
	CrossCompile(hlsl, GPT_VERTEX_PROGRAM, CrossCompileOutput::GLSL45, true, dummy, outCompileResult, &shaderDesc, &entryPoints);
}

BSLResult BSLCompiler::Compile(const String& name, const String& source, const UnorderedMap<String, String>& defines, ShadingLanguageFlags languages, SPtr<Shader>& outShader)
{
	SPtr<BSLFXShaderMetaData> shaderMetaData;
	BSLResult compileStatus = CompileMetaData(source, defines, shaderMetaData);
	if(!compileStatus.ErrorMessage.empty())
		return compileStatus;

	B3D_ASSERT(shaderMetaData != nullptr);

		BSLResult compileResult;

	SmallVector<ShadingLanguageFlag, (u32)ShadingLanguageFlag::Count> requiredLanguageSet;
	for(u32 shadingLanguageIndex = 0; shadingLanguageIndex < (u32)ShadingLanguageFlag::Count; shadingLanguageIndex++)
	{
		if(languages.IsSet((ShadingLanguageFlag)(1 << shadingLanguageIndex)))
			requiredLanguageSet.Add((ShadingLanguageFlag)(1 << shadingLanguageIndex));
	}

	// For every variation, re-parse the file with relevant defines
	for(auto& variation : shaderMetaData->Variations)
	{
		BSLParsedShaderData parsedNode;
		compileResult = BSLParser::ParseVariation(shaderMetaData->Name, source, variation, defines, parsedNode);

		if(!compileResult.ErrorMessage.empty())
			return compileResult;

		for(u32 languageIndex = 0; languageIndex < requiredLanguageSet.size(); ++languageIndex)
		{
			const auto passCount = (u32)parsedNode.Passes.size();
			for(u32 passIndex = 0; passIndex < passCount; passIndex++)
			{
				const BSLParsedShaderPassData& parsedShaderPassNode = parsedNode.Passes[passIndex];

				// Find valid entry points and parameters
				// Note: Ideally we don't need to do a full reflection pass for each GPU program type (i.e. by adding some kind of AST caching to XShaderCompiler)
				ReflectHlsl(parsedShaderPassNode.Code, shaderMetaData->ShaderInformation, shaderMetaData->GPUProgramTypes, compileStatus);
			}

			SPtr<Technique> compiledVariation;
			compileStatus = CompileVariation(shaderMetaData->Name, parsedNode, *shaderMetaData, variation, requiredLanguageSet[languageIndex], compiledVariation);

			if(!compileStatus.ErrorMessage.empty())
				return compileStatus;

			shaderMetaData->ShaderInformation.Techniques.push_back(compiledVariation);
		}
	}

	// Verify techniques compile correctly
	bool hasError = false;
	StringStream gpuProgError;
	for(auto& technique : shaderMetaData->ShaderInformation.Techniques)
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
		compileResult.ErrorMessage = "Failed compiling GPU program(s): " + gpuProgError.str();
		compileResult.ErrorLine = 0;
		compileResult.ErrorColumn = 0;
	}

	if(compileStatus.ErrorMessage.empty())
	{
		outShader = Shader::CreateShared(name, shaderMetaData->ShaderInformation);
		outShader->SetIncludeFiles(shaderMetaData->Includes);
	}

	return compileResult;
}

BSLResult BSLCompiler::CompileMetaData(const String& source, const UnorderedMap<String, String>& defines, SPtr<BSLFXShaderMetaData>& outShaderMetaData)
{
	SPtr<BSLFXShaderMetaData> shaderMetaData = B3DMakeShared<BSLFXShaderMetaData>();

	BSLParsedShaderMetaData parsedShaderMetaData;
	BSLResult compileResult = BSLParser::ParseMetaData(source, defines, shaderMetaData->ShaderInformation, parsedShaderMetaData, shaderMetaData->Includes);

	if(!compileResult.ErrorMessage.empty())
		return compileResult;

	shaderMetaData->Variations = CreateShaderVariations(parsedShaderMetaData);
	shaderMetaData->Name = parsedShaderMetaData.Name;
	shaderMetaData->Defines = defines;

	outShaderMetaData = shaderMetaData;
	return compileResult;
}

BSLResult BSLCompiler::CompileVariation(const String& source, const ShaderVariation& variation, ShadingLanguageFlag language, BSLFXShaderMetaData& inOutShaderMetaData, SPtr<Technique>& outVariation)
{
	BSLParsedShaderData parsedNode;
	BSLResult compileResult = BSLParser::ParseVariation(inOutShaderMetaData.Name, source, variation, inOutShaderMetaData.Defines, parsedNode);

	if(!compileResult.ErrorMessage.empty())
		return compileResult;

	if(!inOutShaderMetaData.HasGPUProgramMetaData)
	{
		const auto passCount = (u32)parsedNode.Passes.size();
		for(u32 passIndex = 0; passIndex < passCount; passIndex++)
		{
			const BSLParsedShaderPassData& parsedShaderPassNode = parsedNode.Passes[passIndex];

			// Find valid entry points and parameters
			// Note: Ideally we don't need to do a full reflection pass for each GPU program type (i.e. by adding some kind of AST caching to XShaderCompiler)
			ReflectHlsl(parsedShaderPassNode.Code, inOutShaderMetaData.ShaderInformation, inOutShaderMetaData.GPUProgramTypes, compileResult);
		}

		inOutShaderMetaData.HasGPUProgramMetaData = true;
	}

	return CompileVariation(inOutShaderMetaData.Name, parsedNode, inOutShaderMetaData, variation, language, outVariation);
}

BSLResult BSLCompiler::CompileVariation(const String& name, const BSLParsedShaderData& parsedShader, const BSLFXShaderMetaData& shaderMetaData, const ShaderVariation& variation, ShadingLanguageFlag language, SPtr<Technique>& outVariation)
{
	B3D_ASSERT(!parsedShader.MetaData.IsMixin);
	B3D_ASSERT(shaderMetaData.GPUProgramTypes.size() > 0);

	BSLResult compileResult;

	CrossCompileOutput crossCompileOutputLanguage = CrossCompileOutput::VKSL45;
	String crossCompileOutputLanguageName;
	if(language == ShadingLanguageFlag::GLSL)
	{
		crossCompileOutputLanguage = CrossCompileOutput::GLSL45;
		crossCompileOutputLanguageName = "glsl";
	}
	else if(language == ShadingLanguageFlag::VKSL)
	{
		crossCompileOutputLanguage = CrossCompileOutput::VKSL45;
		crossCompileOutputLanguageName = "vksl";
	}
	else if(language == ShadingLanguageFlag::MSL)
	{
		crossCompileOutputLanguage = CrossCompileOutput::MVKSL;
		crossCompileOutputLanguageName = "mvksl";
	}
	else if(language == ShadingLanguageFlag::HLSL)
	{
		// No cross compile needed
		crossCompileOutputLanguageName = "hlsl";
	}

	struct CrossCompilePassOutput
	{
		String ProgramCodePerType[GPT_COUNT];
	};

	Map<u32, SPtr<Pass>, std::greater<u32>> passes;
	const auto passCount = (u32)parsedShader.Passes.size();
	for(u32 passIndex = 0; passIndex < passCount; passIndex++)
	{
		const BSLParsedShaderPassData& parsedShaderPass = parsedShader.Passes[passIndex];

		auto fnCrossCompilePass = [&shaderMetaData, &compileResult](const BSLParsedShaderPassData& parsedShaderPass, CrossCompileOutput language, CrossCompilePassOutput& crossCompiledOutput)
		{
			u32 binding = 0;
			for(auto& type : shaderMetaData.GPUProgramTypes)
			{
				B3D_ASSERT((i32)type < GPT_COUNT);
				crossCompiledOutput.ProgramCodePerType[(i32)type] = CrossCompile(parsedShaderPass.Code, type, language, binding, compileResult);

				if(!compileResult.ErrorMessage.empty())
					return;
			}
		};

		CrossCompilePassOutput crossCompilePassOutput;
		if(language == ShadingLanguageFlag::HLSL)
		{
			// Clean non-standard HLSL
			// Note: Ideally we add a full HLSL output module to XShaderCompiler, instead of using simple regex. This
			// way the syntax could be enhanced with more complex features, while still being able to output pure
			// HLSL.
			static const std::regex attrRegex(
				R"(\[\s*layout\s*\(.*\)\s*\]|\[\s*internal\s*\]|\[\s*color\s*\]|\[\s*alias\s*\(.*\)\s*\]|\[\s*spriteuv\s*\(.*\)\s*\])");
			String parsedCode = regex_replace(parsedShaderPass.Code, attrRegex, "");

			static const std::regex attr2Regex(
				R"(\[\s*hideInInspector\s*\]|\[\s*name\s*\(".*"\)\s*\]|\[\s*hdr\s*\])");
			parsedCode = regex_replace(parsedCode, attr2Regex, "");

			static const std::regex initializerRegex(
				R"((Texture2D|Texture3D)\s*(\S*)\s*=.*;)");
			parsedCode = regex_replace(parsedCode, initializerRegex, "$1 $2;");

			static const std::regex warpWithSyncRegex(
				R"(Warp(Group|Device|All)MemoryBarrierWithWarpSync)");
			parsedCode = regex_replace(parsedCode, warpWithSyncRegex, "$1MemoryBarrierWithGroupSync");

			static const std::regex warpNoSyncRegex(
				R"(Warp(Group|Device|All)MemoryBarrier)");
			parsedCode = regex_replace(parsedCode, warpNoSyncRegex, "$1MemoryBarrier");

			// Note: I'm just copying HLSL code as-is. This code will contain all entry points which could have
			// an effect on compile time. It would be ideal to remove dead code depending on program Type. This would
			// involve adding a HLSL code generator to XShaderCompiler.
			for(auto& type : shaderMetaData.GPUProgramTypes)
			{
				B3D_ASSERT((i32)type < GPT_COUNT);
				crossCompilePassOutput.ProgramCodePerType[(i32)type] = parsedCode;
			}
		}
		else // Need to cross compile to correct low-level language
		{
			fnCrossCompilePass(parsedShaderPass, crossCompileOutputLanguage, crossCompilePassOutput);

			if(!compileResult.ErrorMessage.empty())
				return compileResult;
		}

		PASS_DESC shaderPassInformation;
		shaderPassInformation.BlendStateDesc = parsedShaderPass.BlendStateInformation;
		shaderPassInformation.RasterizerStateDesc = parsedShaderPass.RasterizerStateInformation;
		shaderPassInformation.DepthStencilStateDesc = parsedShaderPass.DepthStencilStateInformation;

		auto fnBuildGpuProgramCreateInformation = [&name](const String& language, const String& entry, const String& code, GpuProgramType type) -> GpuProgramCreateInformation
		{
			const char* typeString;
			switch(type)
			{
			case GPT_VERTEX_PROGRAM:
				typeString = "Vertex";
				break;
			case GPT_FRAGMENT_PROGRAM:
				typeString = "Fragment";
				break;
			case GPT_GEOMETRY_PROGRAM:
				typeString = "Geometry";
				break;
			case GPT_DOMAIN_PROGRAM:
				typeString = "Domain";
				break;
			case GPT_HULL_PROGRAM:
				typeString = "Hull";
				break;
			case GPT_COMPUTE_PROGRAM:
				typeString = "Compute";
				break;
			default:
				typeString = "Unknown";
				break;
			}

			GpuProgramCreateInformation gpuProgramCreateInformation;
			gpuProgramCreateInformation.Name = StringUtil::Format("{0} ({1} Program)", name, typeString);
			gpuProgramCreateInformation.Language = language;
			gpuProgramCreateInformation.EntryPoint = entry;
			gpuProgramCreateInformation.Source = code;
			gpuProgramCreateInformation.Type = type;

			return gpuProgramCreateInformation;
		};

		const bool isHLSL = language == ShadingLanguageFlag::HLSL;
		shaderPassInformation.VertexProgramDesc = fnBuildGpuProgramCreateInformation(
			crossCompileOutputLanguageName,
			isHLSL ? "vsmain" : "main",
			crossCompilePassOutput.ProgramCodePerType[GPT_VERTEX_PROGRAM],
			GPT_VERTEX_PROGRAM);

		shaderPassInformation.FragmentProgramDesc = fnBuildGpuProgramCreateInformation(
			crossCompileOutputLanguageName,
			isHLSL ? "fsmain" : "main",
			crossCompilePassOutput.ProgramCodePerType[GPT_FRAGMENT_PROGRAM],
			GPT_FRAGMENT_PROGRAM);

		shaderPassInformation.GeometryProgramDesc = fnBuildGpuProgramCreateInformation(
			crossCompileOutputLanguageName,
			isHLSL ? "gsmain" : "main",
			crossCompilePassOutput.ProgramCodePerType[GPT_GEOMETRY_PROGRAM],
			GPT_GEOMETRY_PROGRAM);

		shaderPassInformation.HullProgramDesc = fnBuildGpuProgramCreateInformation(
			crossCompileOutputLanguageName,
			isHLSL ? "hsmain" : "main",
			crossCompilePassOutput.ProgramCodePerType[GPT_HULL_PROGRAM],
			GPT_HULL_PROGRAM);

		shaderPassInformation.DomainProgramDesc = fnBuildGpuProgramCreateInformation(
			crossCompileOutputLanguageName,
			isHLSL ? "dsmain" : "main",
			crossCompilePassOutput.ProgramCodePerType[GPT_DOMAIN_PROGRAM],
			GPT_DOMAIN_PROGRAM);

		shaderPassInformation.ComputeProgramDesc = fnBuildGpuProgramCreateInformation(
			crossCompileOutputLanguageName,
			isHLSL ? "csmain" : "main",
			crossCompilePassOutput.ProgramCodePerType[GPT_COMPUTE_PROGRAM],
			GPT_COMPUTE_PROGRAM);

		shaderPassInformation.StencilRefValue = parsedShaderPass.StencilReferenceValue;

		const SPtr<Pass> pass = Pass::Create(shaderPassInformation);
		if(pass != nullptr)
		{
			passes[parsedShaderPass.SequentialIndex] = pass;
		}
	}

	Vector<SPtr<Pass>> orderedPasses;
	for(auto& KVP : passes)
		orderedPasses.push_back(KVP.second);

	if(!orderedPasses.empty())
		outVariation = Technique::Create(crossCompileOutputLanguageName, parsedShader.MetaData.Tags, variation, orderedPasses);

	return compileResult;
}

Vector<ShaderVariation> BSLCompiler::CreateShaderVariations(const BSLParsedShaderMetaData& shaderMetaData)
{
	if(shaderMetaData.Variations.empty())
		return { ShaderVariation() };

	Vector<ShaderVariation> variations;

	FrameScope frameScope;
	FrameVector<const BSLParsedVariationData*> variationsToProcess;
	for(u32 variationIndex = 0; variationIndex < (u32)shaderMetaData.Variations.size(); variationIndex++)
		variationsToProcess.push_back(&shaderMetaData.Variations[variationIndex]);

	while(!variationsToProcess.empty())
	{
		const BSLParsedVariationData* currentVariation = variationsToProcess.back();
		variationsToProcess.erase(variationsToProcess.end() - 1);

		// Variation parameter that's either defined or isn't
		if(currentVariation->Values.empty())
		{
			// This is the first variation parameter, register new variations
			if(variations.empty())
			{
				ShaderVariation a;
				ShaderVariation b;

				b.AddParam(ShaderVariation::Param(currentVariation->Identifier, 1));

				variations.push_back(a);
				variations.push_back(b);
			}
			else // Duplicate existing variations, and add the parameter
			{
				const u32 variationCount = (u32)variations.size();
				for(u32 variationIndex = 0; variationIndex < variationCount; variationIndex++)
				{
					// Make a copy
					variations.push_back(variations[variationIndex]);

					// Add the parameter to existing variation
					variations[variationIndex].AddParam(ShaderVariation::Param(currentVariation->Identifier, 1));
				}
			}
		}
		else // Variation parameter with multiple values
		{
			// This is the first variation parameter, register new variations
			if(variations.empty())
			{
				for(u32 variationValueIndex = 0; variationValueIndex < (u32)currentVariation->Values.size(); variationValueIndex++)
				{
					ShaderVariation variation;
					variation.AddParam(ShaderVariation::Param(currentVariation->Identifier, currentVariation->Values[variationValueIndex].Value));

					variations.push_back(variation);
				}
			}
			else // Duplicate existing variations, and add the parameter
			{
				const u32 variationCount = (u32)variations.size();
				for(u32 variationIndex = 0; variationIndex < variationCount; variationIndex++)
				{
					for(u32 variationValueIndex = 1; variationValueIndex < (u32)currentVariation->Values.size(); variationValueIndex++)
					{
						ShaderVariation copy = variations[variationIndex];
						copy.AddParam(ShaderVariation::Param(currentVariation->Identifier, currentVariation->Values[variationValueIndex].Value));

						variations.push_back(copy);
					}

					variations[variationIndex].AddParam(ShaderVariation::Param(currentVariation->Identifier, currentVariation->Values[0].Value));
				}
			}
		}
	}

	return variations;
}
