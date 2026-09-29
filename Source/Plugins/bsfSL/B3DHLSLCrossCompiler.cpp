//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DHLSLCrossCompiler.h"
#if B3D_PLATFORM_MACOS
#include "B3DMetalSourceCompiler.h"
#endif
#include "GpuBackend/B3DGpuPushConstants.h"
#include "GpuBackend/B3DSamplerState.h"
#include "Material/B3DShader.h"
#include "Resources/B3DBuiltinResources.h"

#define XSC_ENABLE_LANGUAGE_EXT 1
#include "GpuBackend/B3DGpuParameterSet.h"

// Xsc::Report derives from std::exception across the DLL boundary, which is fine for our use
#if B3D_COMPILER_MSVC
#	pragma warning(push)
#	pragma warning(disable : 4275)
#endif
#include "Xsc/Xsc.h"
#if B3D_COMPILER_MSVC
#	pragma warning(pop)
#endif

using namespace std;
using namespace b3d;

/** Reports error from XSC library. */
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

static GpuParameterObjectType XSCConvertTextureType(Xsc::Reflection::BufferType type)
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

static GpuParameterObjectType XSCConvertBufferType(Xsc::Reflection::BufferType type)
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

static GpuDataParameterType XSCConvertDataType(Xsc::Reflection::DataType type)
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

static TextureAddressingMode XSCConvertTextureAddressingMode(Xsc::Reflection::TextureAddressMode addressingMode)
{
	switch(addressingMode)
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

static CompareFunction XSCConvertComparisonFunction(Xsc::Reflection::ComparisonFunc comparisonFunction)
{
	switch(comparisonFunction)
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

static ShaderDefaultTextureType GetBuiltinTexture(u32 index)
{
	if(index == 1)
		return ShaderDefaultTextureType::White;
	else if(index == 2)
		return ShaderDefaultTextureType::Black;
	else if(index == 3)
		return ShaderDefaultTextureType::Normal;

	return ShaderDefaultTextureType::None;
}

static u32 CalculateStructSize(i32 structIndex, const std::vector<Xsc::Reflection::Struct>& structLookup)
{
	if(structIndex < 0 || structIndex >= (i32)structLookup.size())
		return 0;

	u32 size = 0;

	const Xsc::Reflection::Struct& structInfo = structLookup[structIndex];
	for(auto& entry : structInfo.members)
	{
		if(entry.type == Xsc::Reflection::VariableType::Variable)
		{
			// Note: We're ignoring any padding. Since we can't guarantee the padding will be same for structs across
			// different render backends it's expected for the user to set up structs in such a way so padding is not
			// needed (i.e. add padding variables manually).
			GpuDataParameterType type = XSCConvertDataType((Xsc::Reflection::DataType)entry.baseType);

			const GpuDataParameterTypeInformation& typeInfo = GpuParameterSet::kParamSizes.Lookup[(int)type];
			size += typeInfo.NumColumns * typeInfo.NumRows * typeInfo.BaseTypeSize * entry.arraySize;
		}
		else if(entry.type == Xsc::Reflection::VariableType::Struct)
			size += CalculateStructSize(entry.baseType, structLookup);
	}

	return size;
}

static SamplerStateCreateInformation ParseSamplerState(const Xsc::Reflection::SamplerState& samplerReflectionInformation)
{
	SamplerStateCreateInformation samplerCreateInformation;

	samplerCreateInformation.AddressMode.U = XSCConvertTextureAddressingMode(samplerReflectionInformation.addressU);
	samplerCreateInformation.AddressMode.V = XSCConvertTextureAddressingMode(samplerReflectionInformation.addressV);
	samplerCreateInformation.AddressMode.W = XSCConvertTextureAddressingMode(samplerReflectionInformation.addressW);

	samplerCreateInformation.BorderColor[0] = samplerReflectionInformation.borderColor[0];
	samplerCreateInformation.BorderColor[1] = samplerReflectionInformation.borderColor[1];
	samplerCreateInformation.BorderColor[2] = samplerReflectionInformation.borderColor[2];
	samplerCreateInformation.BorderColor[3] = samplerReflectionInformation.borderColor[3];

	samplerCreateInformation.ComparisonFunc = XSCConvertComparisonFunction(samplerReflectionInformation.comparisonFunc);
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

static bool ParseParameters(const Xsc::Reflection::ReflectionData& reflectionData, ShaderCompilerResult& outCompileResult, ShaderParameterDescription& outParameters, bool includeInternal)
{
	for(auto& entry : reflectionData.uniforms)
	{
		String ident = entry.ident.c_str();
		bool isInternal = (entry.flags & Xsc::Reflection::Uniform::Flags::Internal) != 0;
		bool isBlockHiddenInInspector = false;
		if(entry.uniformBlock >= 0)
		{
			const std::string& blockName = reflectionData.constantBuffers[entry.uniformBlock].ident;
			for(const auto& uniform : reflectionData.uniforms)
			{
				if(uniform.type == Xsc::Reflection::VariableType::UniformBuffer && uniform.ident == blockName)
				{
					isInternal |= (uniform.flags & Xsc::Reflection::Uniform::Flags::Internal) != 0;
					isBlockHiddenInInspector = (uniform.flags & Xsc::Reflection::Uniform::Flags::HideInInspector) != 0;

					break;
				}
			}
		}

		if(!includeInternal && isInternal)
			continue;

		auto fnParseCommonAttributes = [&entry, &ident, &outParameters, &isBlockHiddenInInspector]()
		{
			if(!entry.readableName.empty())
			{
				ShaderParameterAttribute attribute;
				attribute.Value.assign(entry.readableName.data(), entry.readableName.size());
				attribute.NextParameterIndex = (u32)-1;
				attribute.Type = ShaderParamAttributeType::Name;

				outParameters.SetParameterAttribute(ident, attribute);
			}

			if((entry.flags & Xsc::Reflection::Uniform::Flags::HideInInspector) != 0 || isBlockHiddenInInspector)
			{
				ShaderParameterAttribute attribute;
				attribute.NextParameterIndex = (u32)-1;
				attribute.Type = ShaderParamAttributeType::HideInInspector;

				outParameters.SetParameterAttribute(ident, attribute);
			}

			if((entry.flags & Xsc::Reflection::Uniform::Flags::HDR) != 0)
			{
				ShaderParameterAttribute attribute;
				attribute.NextParameterIndex = (u32)-1;
				attribute.Type = ShaderParamAttributeType::HDR;

				outParameters.SetParameterAttribute(ident, attribute);
			}
		};

		switch(entry.type)
		{
		case Xsc::Reflection::VariableType::UniformBuffer:
			{
				ShaderUniformBufferInformation information;
				information.Name = ident;
				information.UsesDynamicOffset = (entry.flags & Xsc::Reflection::Uniform::Flags::DynamicOffset) != 0;
				outParameters.AddUniformBuffer(std::move(information));
			}
			break;
		case Xsc::Reflection::VariableType::Buffer:
			{
				GpuParameterObjectType objType = XSCConvertTextureType((Xsc::Reflection::BufferType)entry.baseType);
				if(objType != GPOT_UNKNOWN)
				{
					if(entry.defaultValue == -1)
						outParameters.AddParameter(ShaderObjectParameterInformation(ident, ident, objType, StringID::kNone, entry.arraySize));
					else
						outParameters.AddParameter(ShaderObjectParameterInformation(ident, ident, objType, StringID::kNone, entry.arraySize), GetBuiltinTexture(reflectionData.defaultValues[entry.defaultValue].integer));

					fnParseCommonAttributes();
				}
				else
				{
					objType = XSCConvertBufferType((Xsc::Reflection::BufferType)entry.baseType);
					outParameters.AddParameter(ShaderObjectParameterInformation(ident, ident, objType, StringID::kNone, entry.arraySize));

					fnParseCommonAttributes();
				}
			}
			break;
		case Xsc::Reflection::VariableType::Sampler:
			{
				if(auto foundSamplerReflectionData = reflectionData.samplerStates.find(entry.ident); foundSamplerReflectionData != reflectionData.samplerStates.end())
				{
					SamplerStateCreateInformation defaultSamplerStateCreateInformation;
					if (foundSamplerReflectionData->second.isNonDefault)
						defaultSamplerStateCreateInformation = ParseSamplerState(foundSamplerReflectionData->second);

					const String alias = foundSamplerReflectionData->second.alias.c_str();
					if(foundSamplerReflectionData->second.isNonDefault)
					{
						outParameters.AddParameter(ShaderObjectParameterInformation(ident, ident, GPOT_SAMPLER2D), defaultSamplerStateCreateInformation);

						if(!alias.empty())
							outParameters.AddParameter(ShaderObjectParameterInformation(ident, alias, GPOT_SAMPLER2D), defaultSamplerStateCreateInformation);
					}
					else
					{
						outParameters.AddParameter(ShaderObjectParameterInformation(ident, ident, GPOT_SAMPLER2D));

						if(!alias.empty())
							outParameters.AddParameter(ShaderObjectParameterInformation(ident, alias, GPOT_SAMPLER2D));
					}
				}
				else
				{
					outParameters.AddParameter(ShaderObjectParameterInformation(ident, ident, GPOT_SAMPLER2D));
				}
				break;
			}
		case Xsc::Reflection::VariableType::Variable:
			{
				GpuDataParameterType type = XSCConvertDataType((Xsc::Reflection::DataType)entry.baseType);
				if((entry.flags & Xsc::Reflection::Uniform::Flags::Color) != 0 &&
				   (type == GPDT_FLOAT3 || type == GPDT_FLOAT4))
				{
					type = GPDT_COLOR;
				}

				u32 arraySize = entry.arraySize;

				if(entry.defaultValue == -1)
					outParameters.AddParameter(ShaderDataParameterInformation(ident, ident, type, StringID::kNone, arraySize));
				else
				{
					const Xsc::Reflection::DefaultValue& defaultValue = reflectionData.defaultValues[entry.defaultValue];

					const u32 defaultValueSize = Shader::GetDataParameterSize(type);
					if(arraySize != 1 || defaultValueSize > sizeof(defaultValue))
					{
						outCompileResult.ErrorMessage = StringUtility::Format("Unsupported reflected default value for parameter '{0}'.", ident);
						return false;
					}

					outParameters.AddParameter(ShaderDataParameterInformation(ident, ident, type), TArrayView<const u8>((const u8*)&defaultValue, defaultValueSize));
				}

				if(!entry.spriteUVRef.empty() && (type == GPDT_FLOAT4))
				{
					ShaderParameterAttribute attribute;
					attribute.Value.assign(entry.spriteUVRef.data(), entry.spriteUVRef.size());
					attribute.NextParameterIndex = (u32)-1;
					attribute.Type = ShaderParamAttributeType::SpriteUV;

					outParameters.SetParameterAttribute(ident, attribute);
				}

				fnParseCommonAttributes();
			}
			break;
		case Xsc::Reflection::VariableType::Struct:
			{
				i32 structIdx = entry.baseType;
				u32 structSize = CalculateStructSize(structIdx, reflectionData.structs);

				outParameters.AddParameter(ShaderDataParameterInformation(ident, ident, GPDT_STRUCT, StringID::kNone, entry.arraySize, structSize));
			}
			break;
		default:;
		}

		outParameters.SetParameterInternal(ident, isInternal);
	}

	for(const auto& binding : reflectionData.constantBuffers)
		outParameters.SetParameterBinding(binding.ident.c_str(), (u32)binding.set, (u32)binding.location);

	for(const auto& binding : reflectionData.textures)
		outParameters.SetParameterBinding(binding.ident.c_str(), (u32)binding.set, (u32)binding.location);

	for(const auto& binding : reflectionData.storageBuffers)
		outParameters.SetParameterBinding(binding.ident.c_str(), (u32)binding.set, (u32)binding.location);

	for(const auto& uniform : reflectionData.uniforms)
	{
		if(uniform.uniformBlock >= 0)
		{
			const auto& binding = reflectionData.constantBuffers[uniform.uniformBlock];
			outParameters.SetParameterBinding(uniform.ident.c_str(), (u32)binding.set, (u32)binding.location);
		}
	}

	return true;
}

// TODO - B3DShaderCompiler is not thread safe, so we must limit only one call at a time.
static Mutex& GetXscCompileMutex()
{
	static Mutex mutex;
	return mutex;
}

static String CrossCompile(const String& hlsl, GpuProgramType type, const HLSLCrossCompileTarget& target, bool optionalEntry, u32& startBindingSlot, ShaderCompilerResult& outCompileResult,
	ShaderReflection& outReflection, bool reflectParameters = false)
{
	TShared<StringStream> input = B3DMakeShared<StringStream>();

	if(target.PreprocessorDefine != nullptr)
		*input << "#define " << target.PreprocessorDefine << " 1" << std::endl;

	// Clear '\r' as it's breaking XShaderCompiler when used in mutiline preprocessor statements
	for (const char& currentCharacter : hlsl)
	{
		if (currentCharacter == '\r')
			continue;

		*input << currentCharacter;
	}

	Xsc::ShaderInput inputDesc;
	inputDesc.shaderVersion = Xsc::InputShaderVersion::HLSL6;
	inputDesc.sourceCode = input;
	inputDesc.extensions =
		Xsc::Extensions::LayoutAttribute |
		Xsc::Extensions::SrtSignature |
		Xsc::Extensions::HLSLTemplates |
		Xsc::Extensions::PushConstants |
		Xsc::Extensions::DynamicOffsets;

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
	outputDesc.options.autoBinding = true;
	outputDesc.options.autoBindingStartSlot = startBindingSlot;
	outputDesc.options.maxPushConstantSize = kMaxPushConstantSizeInBytes;
	outputDesc.options.pushConstantHLSLRegister = kPushConstantHlslRegister;
	outputDesc.options.pushConstantHLSLRegisterSpace = kPushConstantHlslRegisterSpace;
	outputDesc.options.fragmentLocations = true;
	outputDesc.options.separateShaders = true;
	outputDesc.options.separateSamplers = true;
	outputDesc.options.allowExtensions = true;
	outputDesc.nameMangling.inputPrefix = "bs_";
	outputDesc.nameMangling.outputPrefix = "bs_";
	outputDesc.nameMangling.useAlwaysSemantics = true;
	outputDesc.nameMangling.renameBufferFields = true;
	outputDesc.targetLanguage = target.TargetLanguage;

	XscLog log;
	Xsc::Reflection::ReflectionData reflectionData;

	// Serialize the actual Xsc invocation against all other compiles in the process - see GetXscCompileMutex() above
	// for why this is required (Xsc is not thread safe). Only the CompileShader() call touches Xsc's shared global
	// state; the surrounding input/output/reflection handling operates on locals, so the lock scope stays narrow.
	bool compileSuccess;
	{
		Lock lock(GetXscCompileMutex());
		compileSuccess = Xsc::CompileShader(inputDesc, outputDesc, &log, &reflectionData);
	}

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

			outCompileResult.ErrorMessage = StringUtility::Format("Shader cross compilation failed. Log: \n\n{0}", logOutput.str());
			return "";
		}
	}

	for(auto& entry : reflectionData.constantBuffers)
		startBindingSlot = std::max(startBindingSlot, entry.location + 1u);

	for(auto& entry : reflectionData.textures)
		startBindingSlot = std::max(startBindingSlot, entry.location + 1u);

	for(auto& entry : reflectionData.storageBuffers)
		startBindingSlot = std::max(startBindingSlot, entry.location + 1u);

	if(reflectParameters || compileSuccess)
	{
		// Build parameter layout to calculate hash from
		StringStream layout;

		std::function<void(const Xsc::Reflection::Variable&)> fnWriteVariable;
		fnWriteVariable = [&layout, &reflectionData, &fnWriteVariable](const Xsc::Reflection::Variable& variable)
		{
			layout << variable.ident << ':' << (u32)variable.type << ':' << variable.arraySize << ':';

			if(variable.type == Xsc::Reflection::VariableType::Struct)
			{
				layout << '{';

				for(const auto& member : reflectionData.structs[variable.baseType].members)
					fnWriteVariable(member);

				layout << '}';
			}
			else
				layout << variable.baseType;

			layout << ';';
		};

		for(const auto& uniform : reflectionData.uniforms)
		{
			fnWriteVariable(uniform);
			layout << uniform.flags << ':' << uniform.uniformBlock << ';';
		}

		const auto fnWriteBindings = [&layout](const auto& bindings)
		{
			layout << '[';

			for(const auto& binding : bindings)
				layout << binding.ident << ':' << binding.set << ':' << binding.location << ':' << binding.usesDynamicOffset << ';';

			layout << ']';
		};

		fnWriteBindings(reflectionData.constantBuffers);
		fnWriteBindings(reflectionData.textures);
		fnWriteBindings(reflectionData.storageBuffers);

		for(const auto& buffer : reflectionData.pushConstantBuffers)
		{
			layout << buffer.ident << ':' << buffer.size << '{';

			for(const auto& member : buffer.members)
				layout << member.ident << ':' << member.offset << ':' << member.size << ';';

			layout << '}';
		}

		// Ensure that all stages in a shader have the same parameters
		const Array<u64, 2> layoutHash = Shader::ComputeHash(layout.str());
		if(outReflection.Parameters != nullptr && outReflection.ParameterLayoutHash != layoutHash)
		{
			outCompileResult.ErrorMessage = "Shader parameter declarations or bindings differ between stages of the same pass.";
			return "";
		}

		outReflection.ParameterLayoutHash = layoutHash;
		if(outReflection.Parameters == nullptr)
		{
			// Parse parameters for the first stage that gets processed
			outReflection.Parameters = B3DMakeShared<ShaderParameterDescription>();
			if(!ParseParameters(reflectionData, outCompileResult, *outReflection.Parameters, !reflectParameters))
				return "";
		}
	}

	if(reflectParameters)
	{
		for(const auto& entry : reflectionData.functions)
		{
			ShaderEntryPointReflection entryPoint;
			if(entry.ident == "vsmain")
				entryPoint.Type = GPT_VERTEX_PROGRAM;
			else if(entry.ident == "fsmain")
				entryPoint.Type = GPT_FRAGMENT_PROGRAM;
			else if(entry.ident == "gsmain")
				entryPoint.Type = GPT_GEOMETRY_PROGRAM;
			else if(entry.ident == "hsmain")
				entryPoint.Type = GPT_HULL_PROGRAM;
			else if(entry.ident == "dsmain")
				entryPoint.Type = GPT_DOMAIN_PROGRAM;
			else if(entry.ident == "csmain")
				entryPoint.Type = GPT_COMPUTE_PROGRAM;
			else
				continue;

			outReflection.EntryPoints.emplace(entry.ident.c_str(), std::move(entryPoint));
		}

		// A missing vertex entry is allowed during discovery, but a shader must declare some entry point.
		if(!compileSuccess && outReflection.EntryPoints.empty())
		{
			StringStream logOutput;
			log.GetMessages(logOutput);

			outCompileResult.ErrorMessage = StringUtility::Format("Shader cross compilation failed. Log: \n\n{0}", logOutput.str());
			return "";
		}
	}
	else if(compileSuccess)
	{
		ShaderEntryPointReflection entryPoint;
		entryPoint.Type = type;
		entryPoint.ThreadGroupSize[0] = reflectionData.numThreads.x > 0 ? (u32)reflectionData.numThreads.x : 1;
		entryPoint.ThreadGroupSize[1] = reflectionData.numThreads.y > 0 ? (u32)reflectionData.numThreads.y : 1;
		entryPoint.ThreadGroupSize[2] = reflectionData.numThreads.z > 0 ? (u32)reflectionData.numThreads.z : 1;

		B3D_ASSERT(reflectionData.pushConstantBuffers.size() <= 1);
		entryPoint.PushConstantBufferSize = reflectionData.pushConstantBuffers.empty() ? 0u : (u32)reflectionData.pushConstantBuffers.front().size;

		const String entryPointName = target.KeepsEntryPointNames ? inputDesc.entryPoint.c_str() : "main";
		outReflection.EntryPoints.emplace(entryPointName, std::move(entryPoint));
	}

	return output.str();
}

static UnorderedMap<String, HLSLCrossCompileTarget>& GetCrossCompileTargetRegistry()
{
	static UnorderedMap<String, HLSLCrossCompileTarget> registry;
	return registry;
}

void HLSLCrossCompiler::RegisterTarget(const String& language, const HLSLCrossCompileTarget& target)
{
	GetCrossCompileTargetRegistry()[language] = target;
}

const HLSLCrossCompileTarget* HLSLCrossCompiler::GetTarget(const String& language)
{
	const UnorderedMap<String, HLSLCrossCompileTarget>& registry = GetCrossCompileTargetRegistry();
	const auto found = registry.find(language);

	return found != registry.end() ? &found->second : nullptr;
}

ShaderCompilerResult HLSLCrossCompiler::CrossCompile(const String& hlsl, GpuProgramType type, const HLSLCrossCompileTarget& target, u32& startBindingSlot, ShaderCrossCompileOutput& output, const ShaderReflection* sharedReflection)
{
	ShaderCompilerResult compileResult;
	output.Source.clear();
	output.Reflection = B3DMakeShared<ShaderReflection>();
	if(sharedReflection != nullptr)
	{
		output.Reflection->Parameters = sharedReflection->Parameters;
		output.Reflection->ParameterLayoutHash = sharedReflection->ParameterLayoutHash;
	}

	HLSLCrossCompileTarget xscTarget = target;
#if B3D_PLATFORM_MACOS
	const bool compileMetalSource = target.TargetLanguage != nullptr && StringView(target.TargetLanguage) == "MSL";
	if(compileMetalSource)
	{
		xscTarget.TargetLanguage = Xsc::TargetLanguage::VKSL450;
		xscTarget.KeepsEntryPointNames = false;
	}
#endif

	// Entry points discovered without a target may be excluded by the target's preprocessor define.
	output.Source = ::CrossCompile(hlsl, type, xscTarget, true, startBindingSlot, compileResult, *output.Reflection);

#if B3D_PLATFORM_MACOS
	if(compileResult.ErrorMessage.empty() && compileMetalSource && !output.Source.empty())
	{
		String metalSource;
		compileResult = MetalSourceCompiler::Compile(output.Source, type, output.Reflection, metalSource);
		if(compileResult.ErrorMessage.empty())
			output.Source = std::move(metalSource);
	}
#endif

	return compileResult;
}

ShaderCompilerResult HLSLCrossCompiler::Reflect(const String& hlsl, ShaderReflection& outReflection, const HLSLCrossCompileTarget* target)
{
	ShaderCompilerResult compileResult;
	u32 startBindingSlot = 0;
	outReflection.Parameters = nullptr;
	outReflection.EntryPoints.clear();

	HLSLCrossCompileTarget reflectionTarget;
	reflectionTarget.TargetLanguage = Xsc::TargetLanguage::GLSL450;
	reflectionTarget.PreprocessorDefine = "OPENGL";

	if(target != nullptr)
		reflectionTarget = *target;

#if B3D_PLATFORM_MACOS
	if(reflectionTarget.TargetLanguage != nullptr && StringView(reflectionTarget.TargetLanguage) == "MSL")
		reflectionTarget.TargetLanguage = Xsc::TargetLanguage::VKSL450;
#endif

	::CrossCompile(hlsl, GPT_VERTEX_PROGRAM, reflectionTarget, true, startBindingSlot, compileResult, outReflection, true);

	// If no vsmain found, try with an entry point that was actually detected
	if(compileResult.ErrorMessage.empty() && outReflection.EntryPoints.count("vsmain") == 0 && !outReflection.EntryPoints.empty())
	{
		const GpuProgramType type = outReflection.EntryPoints.begin()->second.Type;

		startBindingSlot = 0;
		outReflection.Parameters = nullptr;
		outReflection.EntryPoints.clear();
		::CrossCompile(hlsl, type, reflectionTarget, false, startBindingSlot, compileResult, outReflection, true);
	}

	return compileResult;
}
