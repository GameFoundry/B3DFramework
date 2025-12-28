//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/B3DMaterialParameters.h"
#include "RTTI/B3DMaterialParametersRTTI.h"
#include "Material/B3DShader.h"
#include "Image/B3DTexture.h"
#include "Image/B3DSpriteTexture.h"
#include "RenderAPI/B3DGpuBuffer.h"
#include "RenderAPI/B3DSamplerState.h"
#include "Image/B3DColorGradient.h"
#include "Animation/B3DAnimationCurve.h"
#include "Allocators/B3DPoolAlloc.h"
#include "CoreObject/B3DCoreObjectSync.h"

using namespace b3d;

SPtr<render::Texture> GetSpriteImageAtlas(const SPtr<render::SpriteImage>& spriteImage)
{
	if(spriteImage)
		return spriteImage->GetDefaultAllocatedImage().GetTexture();

	return nullptr;
}

HTexture GetSpriteImageAtlas(const HSpriteImage& spriteImage)
{
	if(spriteImage.IsLoaded())
		return spriteImage->GetDefaultAllocatedImage().GetTexture();

	return HTexture();
}

MaterialParametersBase::MaterialParametersBase(
	const Map<String, ShaderDataParameterInformation>& dataParams,
	const Map<String, ShaderObjectParameterInformation>& textureParams,
	const Map<String, ShaderObjectParameterInformation>& bufferParams,
	const Map<String, ShaderObjectParameterInformation>& samplerParams,
	u64 initialParamVersion)
	: mParamVersion(initialParamVersion)
{
	mDataSize = 0;

	u32 dataParameterCount = 0;
	u32 structParameterCount = 0;
	for(auto& param : dataParams)
	{
		if(param.second.Type == GPDT_UNKNOWN)
			continue;

		u32 arraySize = param.second.ArraySize > 1 ? param.second.ArraySize : 1;
		if(param.second.Type == GPDT_STRUCT)
		{
			mDataSize += arraySize * param.second.ElementSize;
			structParameterCount += arraySize;
		}
		else
		{
			const GpuDataParameterTypeInformation& typeInfo = GpuParameterSet::kParamSizes.Lookup[(int)param.second.Type];
			u32 paramSize = typeInfo.NumColumns * typeInfo.NumRows * typeInfo.BaseTypeSize;

			mDataSize += arraySize * paramSize;
			dataParameterCount += arraySize;
		}
	}

	mTextureParameterCount = (u32)textureParams.size();
	mBufferParameterCount = (u32)bufferParams.size();
	mSamplerParameterCount = (u32)samplerParams.size();

	mDataParamsBuffer = mAlloc.Alloc(mDataSize);
	memset(mDataParamsBuffer, 0, mDataSize);

	mDataParameterMetaData.resize(dataParameterCount);
	mStructParameterMetaData.resize(structParameterCount);

	u32 dataParamIdx = 0;
	u32 dataBufferIdx = 0;
	u32 structParamIdx = 0;

	for(auto& entry : dataParams)
	{
		if(entry.second.Type == GPDT_UNKNOWN)
			continue;

		const auto paramIdx = (u32)mParams.size();
		mParams.push_back(ParamData());
		mParamLookup[entry.first] = paramIdx;

		ParamData& dataParam = mParams.back();

		const u32 arraySize = entry.second.ArraySize > 1 ? entry.second.ArraySize : 1;
		dataParam.ArraySize = arraySize;
		dataParam.Type = ParamType::Data;
		dataParam.DataType = entry.second.Type;
		dataParam.Version = 1;

		if(entry.second.Type == GPDT_STRUCT)
		{
			dataParam.Index = structParamIdx;

			for(u32 i = 0; i < arraySize; i++)
			{
				StructParameterMetaData& param = mStructParameterMetaData[structParamIdx];
				param.DataSize = entry.second.ElementSize;
				param.Offset = dataBufferIdx;

				dataBufferIdx += param.DataSize;
				structParamIdx++;
			}
		}
		else
		{
			dataParam.Index = dataParamIdx;

			const GpuDataParameterTypeInformation& typeInfo = GpuParameterSet::kParamSizes.Lookup[(int)dataParam.DataType];
			const u32 paramSize = typeInfo.NumColumns * typeInfo.NumRows * typeInfo.BaseTypeSize;
			for(u32 i = 0; i < arraySize; i++)
			{
				mDataParameterMetaData[dataParamIdx].Offset = dataBufferIdx;
				mDataParameterMetaData[dataParamIdx].SpriteTextureIdx = (u32)-1;

				dataBufferIdx += paramSize;
				dataParamIdx++;
			}
		}
	}

	u32 textureIdx = 0;
	for(auto& entry : textureParams)
	{
		u32 paramIdx = (u32)mParams.size();
		mParams.push_back(ParamData());
		mParamLookup[entry.first] = paramIdx;

		ParamData& dataParam = mParams.back();

		dataParam.ArraySize = 1;
		dataParam.Type = ParamType::Texture;
		dataParam.DataType = GPDT_UNKNOWN;
		dataParam.Index = textureIdx;
		dataParam.Version = 1;

		textureIdx++;
	}

	u32 bufferIdx = 0;
	for(auto& entry : bufferParams)
	{
		u32 paramIdx = (u32)mParams.size();
		mParams.push_back(ParamData());
		mParamLookup[entry.first] = paramIdx;

		ParamData& dataParam = mParams.back();

		dataParam.ArraySize = 1;
		dataParam.Type = ParamType::Buffer;
		dataParam.DataType = GPDT_UNKNOWN;
		dataParam.Index = bufferIdx;
		dataParam.Version = 1;

		bufferIdx++;
	}

	u32 samplerIdx = 0;
	for(auto& entry : samplerParams)
	{
		u32 paramIdx = (u32)mParams.size();
		mParams.push_back(ParamData());
		mParamLookup[entry.first] = paramIdx;

		ParamData& dataParam = mParams.back();

		dataParam.ArraySize = 1;
		dataParam.Type = ParamType::Sampler;
		dataParam.DataType = GPDT_UNKNOWN;
		dataParam.Index = samplerIdx;
		dataParam.Version = 1;

		samplerIdx++;
	}
}

MaterialParametersBase::~MaterialParametersBase()
{
	for(auto& entry : mDataParameterMetaData)
	{
		if(entry.FloatCurve)
		{
			B3DPoolFree(entry.FloatCurve);
			entry.FloatCurve = nullptr;
		}

		if(entry.ColorGradient)
		{
			B3DPoolFree(entry.ColorGradient);
			entry.ColorGradient = nullptr;
		}
	}

	mAlloc.Free(mDataParamsBuffer);
	mAlloc.Clear();
}

const ColorGradientHDR& MaterialParametersBase::GetColorGradientParam(const String& name, u32 arrayIdx) const
{
	static ColorGradientHDR EMPTY_GRADIENT;

	const ParamData* param = nullptr;
	auto result = GetParamData(name, ParamType::Data, GPDT_COLOR, arrayIdx, &param);
	if(result != GetParamResult::Success)
		return EMPTY_GRADIENT;

	return GetColorGradientParam(*param, arrayIdx);
}

void MaterialParametersBase::SetColorGradientParam(const String& name, u32 arrayIdx, const ColorGradientHDR& input)
{
	const ParamData* param = nullptr;
	auto result = GetParamData(name, ParamType::Data, GPDT_COLOR, arrayIdx, &param);
	if(result != GetParamResult::Success)
		return;

	SetColorGradientParam(*param, arrayIdx, input);
}

const ColorGradientHDR& MaterialParametersBase::GetColorGradientParam(const ParamData& param, u32 arrayIdx) const
{
	const DataParamInfo& paramInfo = mDataParameterMetaData[param.Index + arrayIdx];
	if(paramInfo.ColorGradient)
		return *paramInfo.ColorGradient;

	static ColorGradientHDR EMPTY_GRADIENT;
	return EMPTY_GRADIENT;
}

void MaterialParametersBase::SetColorGradientParam(const ParamData& param, u32 arrayIdx, const ColorGradientHDR& input)
{
	DataParamInfo& paramInfo = mDataParameterMetaData[param.Index + arrayIdx];
	if(paramInfo.ColorGradient)
		B3DPoolFree(paramInfo.ColorGradient);

	paramInfo.ColorGradient = B3DPoolNew<ColorGradientHDR>(input);

	param.Version = ++mParamVersion;
}

u32 MaterialParametersBase::GetParamIndex(const String& name) const
{
	auto iterFind = mParamLookup.find(name);
	if(iterFind == mParamLookup.end())
		return (u32)-1;

	return iterFind->second;
}

MaterialParametersBase::GetParamResult MaterialParametersBase::GetParamIndex(const String& name, ParamType type, GpuDataParameterType dataType, u32 arrayIdx, u32& output) const
{
	auto iterFind = mParamLookup.find(name);
	if(iterFind == mParamLookup.end())
		return GetParamResult::NotFound;

	u32 index = iterFind->second;
	const ParamData& param = mParams[index];

	if(param.Type != type || (type == ParamType::Data && param.DataType != dataType))
		return GetParamResult::InvalidType;

	if(arrayIdx >= param.ArraySize)
		return GetParamResult::IndexOutOfBounds;

	output = index;
	return GetParamResult::Success;
}

MaterialParametersBase::GetParamResult MaterialParametersBase::GetParamData(const String& name, ParamType type, GpuDataParameterType dataType, u32 arrayIdx, const ParamData** output) const
{
	auto iterFind = mParamLookup.find(name);
	if(iterFind == mParamLookup.end())
		return GetParamResult::NotFound;

	u32 index = iterFind->second;
	const ParamData& param = mParams[index];
	*output = &param;

	if(param.Type != type || (type == ParamType::Data && param.DataType != dataType))
		return GetParamResult::InvalidType;

	if(arrayIdx >= param.ArraySize)
		return GetParamResult::IndexOutOfBounds;

	return GetParamResult::Success;
}

void MaterialParametersBase::ReportGetParamError(GetParamResult errorCode, const String& name, u32 arrayIdx) const
{
	switch(errorCode)
	{
	case GetParamResult::NotFound:
		B3D_LOG(Warning, LogMaterial, "Material doesn't have a parameter named {0}.", name);
		break;
	case GetParamResult::InvalidType:
		B3D_LOG(Warning, LogMaterial, "Parameter \"{0}\" is not of the requested type.", name);
		break;
	case GetParamResult::IndexOutOfBounds:
		B3D_LOG(Warning, LogMaterial, "Parameter \"{0}\" array index {1} out of range.", name, arrayIdx);
		break;
	default:
		break;
	}
}

RTTIType* MaterialParamTextureData::GetRttiStatic()
{
	return MaterialParamTextureDataRTTI::Instance();
}

RTTIType* MaterialParamTextureData::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* MaterialParamBufferData::GetRttiStatic()
{
	return MaterialParamBufferDataRTTI::Instance();
}

RTTIType* MaterialParamBufferData::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* MaterialParamSamplerStateData::GetRttiStatic()
{
	return MaterialParamSamplerStateDataRTTI::Instance();
}

RTTIType* MaterialParamSamplerStateData::GetRtti() const
{
	return GetRttiStatic();
}

template <bool IsRenderProxy>
TMaterialParameters<IsRenderProxy>::TMaterialParameters(const ShaderType& shader, u64 initialParamVersion)
	: MaterialParametersBase(
		  shader->GetDataParams(),
		  shader->GetTextureParams(),
		  shader->GetBufferParams(),
		  shader->GetSamplerParams(),
		  initialParamVersion)
{
	mTextureParameters.resize(mTextureParameterCount);
	mBufferParameters.resize(mBufferParameterCount);
	mSamplerParameters.resize(mSamplerParameterCount);
	mDefaultTextureParams = mAlloc.Construct<TextureType>(mTextureParameterCount);
	mDefaultSamplerStateParams = mAlloc.Construct<SPtr<SamplerState>>(mSamplerParameterCount);

	auto& textureParams = shader->GetTextureParams();
	u32 textureIdx = 0;
	for(auto& entry : textureParams)
	{
		ParamTextureDataType& param = mTextureParameters[textureIdx];
		param.IsLoadStore = false;

		if(entry.second.DefaultValueIndex != ~0u)
		{
			const TextureType texture = entry.second.Type == GPOT_TEXTURE3D ? shader->GetDefault3DTexture(entry.second.DefaultValueIndex) : shader->GetDefault2DTexture(entry.second.DefaultValueIndex);
			mDefaultTextureParams[textureIdx] = texture;
		}

		textureIdx++;
	}

	auto& samplerParams = shader->GetSamplerParams();
	u32 samplerIdx = 0;
	for(auto& entry : samplerParams)
	{
		if(entry.second.DefaultValueIndex != (u32)-1)
			mDefaultSamplerStateParams[samplerIdx] = shader->GetDefaultSampler(entry.second.DefaultValueIndex);

		samplerIdx++;
	}

	// Note: Make sure to process data parameters after textures, in order to handle SpriteUV data parameters
	auto& dataParams = shader->GetDataParams();
	auto& paramAttributes = shader->GetParamAttributes();
	for(auto& entry : dataParams)
	{
		if(entry.second.Type != GPDT_STRUCT)
		{
			// Check for SpriteUV attribute
			u32 attribIdx = entry.second.AttributeIndex;
			while(attribIdx != (u32)-1)
			{
				const ShaderParameterAttribute& attrib = paramAttributes[attribIdx];
				if(attrib.Type == ShaderParamAttributeType::SpriteUV)
				{
					// Find referenced texture
					const auto findIterTex = mParamLookup.find(attrib.Value);
					const auto findIterParam = mParamLookup.find(entry.first);
					if(findIterTex != mParamLookup.end() && findIterParam != mParamLookup.end())
					{
						ParamData& paramData = mParams[findIterParam->second];

						DataParamInfo& dataParamInfo = mDataParameterMetaData[paramData.Index];
						dataParamInfo.SpriteTextureIdx = findIterTex->second;
					}
				}

				attribIdx = attrib.NextParameterIndex;
			}
		}
	}
}

template <bool IsRenderProxy>
TMaterialParameters<IsRenderProxy>::~TMaterialParameters()
{
	if(mDefaultTextureParams != nullptr)
		mAlloc.Destruct(mDefaultTextureParams, mTextureParameterCount);

	if(mDefaultSamplerStateParams != nullptr)
		mAlloc.Destruct(mDefaultSamplerStateParams, mSamplerParameterCount);
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::GetStructData(const String& name, void* value, u32 size, u32 arrayIdx) const
{
	const ParamData* param = nullptr;
	GetParamResult result = GetParamData(name, ParamType::Data, GPDT_STRUCT, arrayIdx, &param);
	if(result != GetParamResult::Success)
	{
		ReportGetParamError(result, name, arrayIdx);
		return;
	}

	GetStructData(*param, value, size, arrayIdx);
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::SetStructData(const String& name, const void* value, u32 size, u32 arrayIdx)
{
	const ParamData* param = nullptr;
	GetParamResult result = GetParamData(name, ParamType::Data, GPDT_STRUCT, arrayIdx, &param);
	if(result != GetParamResult::Success)
	{
		ReportGetParamError(result, name, arrayIdx);
		return;
	}

	SetStructData(*param, value, size, arrayIdx);
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::GetTexture(const String& name, TextureType& value, TextureSurface& surface) const
{
	const ParamData* param = nullptr;
	GetParamResult result = GetParamData(name, ParamType::Texture, GPDT_UNKNOWN, 0, &param);
	if(result != GetParamResult::Success)
	{
		ReportGetParamError(result, name, 0);
		return;
	}

	GetTexture(*param, value, surface);
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::SetTexture(const String& name, const TextureType& value, const TextureSurface& surface)
{
	const ParamData* param = nullptr;
	GetParamResult result = GetParamData(name, ParamType::Texture, GPDT_UNKNOWN, 0, &param);
	if(result != GetParamResult::Success)
	{
		ReportGetParamError(result, name, 0);
		return;
	}

	SetTexture(*param, value, surface);
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::GetSpriteImage(const String& name, SpriteImageType& value) const
{
	const ParamData* param = nullptr;
	GetParamResult result = GetParamData(name, ParamType::Texture, GPDT_UNKNOWN, 0, &param);
	if(result != GetParamResult::Success)
	{
		ReportGetParamError(result, name, 0);
		return;
	}

	GetSpriteImage(*param, value);
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::SetSpriteImage(const String& name, const SpriteImageType& value)
{
	const ParamData* param = nullptr;
	GetParamResult result = GetParamData(name, ParamType::Texture, GPDT_UNKNOWN, 0, &param);
	if(result != GetParamResult::Success)
	{
		ReportGetParamError(result, name, 0);
		return;
	}

	SetSpriteImage(*param, value);
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::GetStorageTexture(const String& name, TextureType& value, TextureSurface& surface) const
{
	const ParamData* param = nullptr;
	GetParamResult result = GetParamData(name, ParamType::Texture, GPDT_UNKNOWN, 0, &param);
	if(result != GetParamResult::Success)
	{
		ReportGetParamError(result, name, 0);
		return;
	}

	GetStorageTexture(*param, value, surface);
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::SetStorageTexture(const String& name, const TextureType& value, const TextureSurface& surface)
{
	const ParamData* param = nullptr;
	GetParamResult result = GetParamData(name, ParamType::Texture, GPDT_UNKNOWN, 0, &param);
	if(result != GetParamResult::Success)
	{
		ReportGetParamError(result, name, 0);
		return;
	}

	SetStorageTexture(*param, value, surface);
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::GetBuffer(const String& name, BufferType& value) const
{
	const ParamData* param = nullptr;
	GetParamResult result = GetParamData(name, ParamType::Buffer, GPDT_UNKNOWN, 0, &param);
	if(result != GetParamResult::Success)
	{
		ReportGetParamError(result, name, 0);
		return;
	}

	GetBuffer(*param, value);
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::SetBuffer(const String& name, const BufferType& value)
{
	const ParamData* param = nullptr;
	GetParamResult result = GetParamData(name, ParamType::Buffer, GPDT_UNKNOWN, 0, &param);
	if(result != GetParamResult::Success)
	{
		ReportGetParamError(result, name, 0);
		return;
	}

	SetBuffer(*param, value);
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::GetSamplerState(const String& name, SPtr<SamplerState>& value) const
{
	const ParamData* param = nullptr;
	GetParamResult result = GetParamData(name, ParamType::Sampler, GPDT_UNKNOWN, 0, &param);
	if(result != GetParamResult::Success)
	{
		ReportGetParamError(result, name, 0);
		return;
	}

	GetSamplerState(*param, value);
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::SetSamplerState(const String& name, const SPtr<SamplerState>& value)
{
	const ParamData* param = nullptr;
	GetParamResult result = GetParamData(name, ParamType::Sampler, GPDT_UNKNOWN, 0, &param);
	if(result != GetParamResult::Success)
	{
		ReportGetParamError(result, name, 0);
		return;
	}

	SetSamplerState(*param, value);
}

template <bool IsRenderProxy>
bool TMaterialParameters<IsRenderProxy>::IsAnimated(const String& name, u32 arrayIdx)
{
	auto iterFind = mParamLookup.find(name);
	if(iterFind == mParamLookup.end())
		return false;

	u32 index = iterFind->second;
	const ParamData& param = mParams[index];

	if(param.Type != ParamType::Data)
		return false;

	if(arrayIdx >= param.ArraySize)
		return false;

	return IsAnimated(param, arrayIdx);
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::GetStructData(const ParamData& param, void* value, u32 size, u32 arrayIdx) const
{
	const StructParameterMetaData& structParam = mStructParameterMetaData[param.Index + arrayIdx];
	if(structParam.DataSize != size)
	{
		B3D_LOG(Warning, LogMaterial, "Size mismatch when writing to a struct. Provided size was {0} bytes but the struct "
								  "size is {1} bytes",
			   size, structParam.DataSize);
		return;
	}

	memcpy(value, &mDataParamsBuffer[structParam.Offset], structParam.DataSize);
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::SetStructData(const ParamData& param, const void* value, u32 size, u32 arrayIdx)
{
	const StructParameterMetaData& structParam = mStructParameterMetaData[param.Index + arrayIdx];
	if(structParam.DataSize != size)
	{
		B3D_LOG(Warning, LogMaterial, "Size mismatch when writing to a struct. Provided size was {0} bytes but the struct "
								  "size is {1} bytes",
			   size, structParam.DataSize);
		return;
	}

	memcpy(&mDataParamsBuffer[structParam.Offset], value, structParam.DataSize);
	param.Version = ++mParamVersion;
}

template <bool IsRenderProxy>
u32 TMaterialParameters<IsRenderProxy>::GetStructSize(const ParamData& param) const
{
	const StructParameterMetaData& structParam = mStructParameterMetaData[param.Index];
	return structParam.DataSize;
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::GetTexture(const ParamData& param, TextureType& value, TextureSurface& surface) const
{
	const ParamTextureDataType& textureParam = mTextureParameters[param.Index];

	if(textureParam.Texture)
		value = textureParam.Texture;
	else if(textureParam.SpriteImage)
		value = GetSpriteImageAtlas(textureParam.SpriteImage);

	surface = textureParam.Surface;
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::SetTexture(const ParamData& param, const TextureType& value, const TextureSurface& surface)
{
	ParamTextureDataType& textureParam = mTextureParameters[param.Index];
	textureParam.Texture = value;
	textureParam.SpriteImage = nullptr;
	textureParam.IsLoadStore = false;
	textureParam.Surface = surface;

	param.Version = ++mParamVersion;
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::GetSpriteImage(const ParamData& param, SpriteImageType& value) const
{
	const ParamTextureDataType& textureParam = mTextureParameters[param.Index];
	value = textureParam.SpriteImage;
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::SetSpriteImage(const ParamData& param, const SpriteImageType& value)
{
	ParamTextureDataType& textureParam = mTextureParameters[param.Index];
	textureParam.Texture = nullptr;
	textureParam.SpriteImage = value;
	textureParam.IsLoadStore = false;
	textureParam.Surface = TextureSurface::kComplete;

	param.Version = ++mParamVersion;
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::GetBuffer(const ParamData& param, BufferType& value) const
{
	value = mBufferParameters[param.Index].Value;
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::SetBuffer(const ParamData& param, const BufferType& value)
{
	mBufferParameters[param.Index].Value = value;

	param.Version = ++mParamVersion;
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::GetStorageTexture(const ParamData& param, TextureType& value, TextureSurface& surface) const
{
	const ParamTextureDataType& textureParam = mTextureParameters[param.Index];
	value = textureParam.Texture;
	surface = textureParam.Surface;
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::SetStorageTexture(const ParamData& param, const TextureType& value, const TextureSurface& surface)
{
	ParamTextureDataType& textureParam = mTextureParameters[param.Index];
	textureParam.Texture = value;
	textureParam.SpriteImage = nullptr;
	textureParam.IsLoadStore = true;
	textureParam.Surface = surface;

	param.Version = ++mParamVersion;
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::GetSamplerState(const ParamData& param, SPtr<SamplerState>& value) const
{
	value = mSamplerParameters[param.Index].Value;
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::SetSamplerState(const ParamData& param, const SPtr<SamplerState>& value)
{
	mSamplerParameters[param.Index].Value = value;

	param.Version = ++mParamVersion;
}

template <bool IsRenderProxy>
MateralParamTextureType TMaterialParameters<IsRenderProxy>::GetTextureType(const ParamData& param) const
{
	if(mTextureParameters[param.Index].IsLoadStore)
		return MateralParamTextureType::LoadStore;

	if(mTextureParameters[param.Index].SpriteImage)
		return MateralParamTextureType::Sprite;

	return MateralParamTextureType::Normal;
}

template <bool IsRenderProxy>
bool TMaterialParameters<IsRenderProxy>::IsAnimated(const ParamData& param, u32 arrayIdx) const
{
	const DataParamInfo& paramInfo = mDataParameterMetaData[param.Index + arrayIdx];

	return paramInfo.FloatCurve || paramInfo.ColorGradient || paramInfo.SpriteTextureIdx != (u32)-1;
}

template <bool IsRenderProxy>
typename TMaterialParameters<IsRenderProxy>::SpriteImageType TMaterialParameters<IsRenderProxy>::GetOwningSpriteImage(const ParamData& param) const
{
	SpriteImageType output;

	const DataParamInfo& paramInfo = mDataParameterMetaData[param.Index];
	if(paramInfo.SpriteTextureIdx == (u32)-1)
		return output;

	const ParamData* spriteTexParamData = GetParamData(paramInfo.SpriteTextureIdx);
	if(spriteTexParamData)
		GetSpriteImage(*spriteTexParamData, output);

	return output;
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::GetDefaultTexture(const ParamData& param, TextureType& value) const
{
	value = mDefaultTextureParams[param.Index];
}

template <bool IsRenderProxy>
void TMaterialParameters<IsRenderProxy>::GetDefaultSamplerState(const ParamData& param, SPtr<SamplerState>& value) const
{
	value = mDefaultSamplerStateParams[param.Index];
}

template class TMaterialParameters<true>;
template class TMaterialParameters<false>;

namespace b3d
{
	struct MaterialParametersDataParameter
	{
		u32 ParameterIndex = ~0u;
		u32 DataOffset = ~0u;
		u32 CurveMetaDataIndex = ~0u;
		u32 DirtyCurveCount = 0;
	};

	struct MaterialParametersCurveMetaData
	{
		u32 ArrayIndex = ~0u;
		u32 CurveIndex = ~0u;
	};

	struct MaterialParametersTextureParameter
	{
		u32 ParameterIndex = ~0u;
		SPtr<render::Texture> Texture;
		SPtr<render::SpriteImage> SpriteImage;
		bool IsLoadStore;
		TextureSurface Surface;
	};

	struct MaterialParametersBufferParameter
	{
		u32 ParameterIndex = ~0u;
		SPtr<render::GpuBuffer> Buffer;
	};

	struct MaterialParametersSamplerStateParameter
	{
		u32 ParameterIndex = ~0u;
		SPtr<SamplerState> SamplerState;
	};

	B3D_SYNC_BLOCK_BEGIN(MaterialParameters, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Vector<MaterialParametersDataParameter>, DataParameters)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Vector<u8>, DataParameterData)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Vector<MaterialParametersCurveMetaData>, CurveMetaData)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Vector<TAnimationCurve<float>>, FloatCurves)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Vector<ColorGradientHDR>, ColorGradients)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Vector<MaterialParametersTextureParameter>, TextureParameters)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Vector<MaterialParametersBufferParameter>, BufferParameters)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Vector<MaterialParametersSamplerStateParameter>, SamplerStateParameters)
	B3D_SYNC_BLOCK_END
}

MaterialParameters::MaterialParameters(const HShader& shader, u64 initialParamVersion)
	: TMaterialParameters(shader, initialParamVersion), mLastSyncVersion(1)
{}

MaterialParameters::SyncPacket* MaterialParameters::CreateSyncPacket(FrameAllocator& allocator, bool forceAll)
{
	SyncPacket* const syncPacket = allocator.Construct<SyncPacket>(*this, allocator);

	for(u32 parameterIndex = 0; parameterIndex < (u32)mParams.size(); parameterIndex++)
	{
		const ParamData& param = mParams[parameterIndex];
		if(param.Version <= mLastSyncVersion && !forceAll)
			continue;

		switch(param.Type)
		{
		case ParamType::Data:
			{
				const u32 arraySize = param.ArraySize > 1 ? param.ArraySize : 1;

				if(param.DataType == GPDT_STRUCT)
				{
					const StructParameterMetaData& paramData = mStructParameterMetaData[param.Index];

					MaterialParametersDataParameter dirtyParameter;
					dirtyParameter.ParameterIndex = parameterIndex;
					dirtyParameter.DataOffset = (u32)syncPacket->DataParameterData.size();

					// Param data
					for(u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
					{
						const StructParameterMetaData& arrayEntryParameterMetaData = mStructParameterMetaData[param.Index + arrayIndex];
						const u8* const parameterData = &mDataParamsBuffer[arrayEntryParameterMetaData.Offset];

						syncPacket->DataParameterData.insert(syncPacket->DataParameterData.end(), parameterData, parameterData + paramData.DataSize);
					}

					syncPacket->DataParameters.push_back(dirtyParameter);
				}
				else
				{
					const GpuDataParameterTypeInformation& typeInfo = GpuParameterSet::kParamSizes.Lookup[(int)param.DataType];
					const u32 paramSize = typeInfo.NumColumns * typeInfo.NumRows * typeInfo.BaseTypeSize;

					const u32 dataSize = arraySize * paramSize;
					const DataParamInfo& paramInfo = mDataParameterMetaData[param.Index];

					MaterialParametersDataParameter dirtyParameter;
					dirtyParameter.ParameterIndex = parameterIndex;
					dirtyParameter.DataOffset = (u32)syncPacket->DataParameterData.size();
					dirtyParameter.CurveMetaDataIndex = (u32)syncPacket->CurveMetaData.size();

					// Note: This relies on the fact that all data params in the array are sequential
					const u8* const parameterData = &mDataParamsBuffer[paramInfo.Offset];
					syncPacket->DataParameterData.insert(syncPacket->DataParameterData.end(), parameterData, parameterData + dataSize);

					// Param curves
					for(u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
					{
						const DataParamInfo& arrayEntryParameterInformation = mDataParameterMetaData[param.Index + arrayIndex];
						if(arrayEntryParameterInformation.FloatCurve && param.DataType == GPDT_FLOAT1)
						{
							MaterialParametersCurveMetaData curveMetaData;
							curveMetaData.ArrayIndex = arrayIndex;
							curveMetaData.CurveIndex = (u32)syncPacket->FloatCurves.size();

							syncPacket->FloatCurves.push_back(*arrayEntryParameterInformation.FloatCurve);
							syncPacket->CurveMetaData.push_back(curveMetaData);
							dirtyParameter.DirtyCurveCount++;
						}
						else if(arrayEntryParameterInformation.ColorGradient && param.DataType == GPDT_COLOR)
						{
							MaterialParametersCurveMetaData curveMetaData;
							curveMetaData.ArrayIndex = arrayIndex;
							curveMetaData.CurveIndex = (u32)syncPacket->ColorGradients.size();

							syncPacket->ColorGradients.push_back(*arrayEntryParameterInformation.ColorGradient);
							syncPacket->CurveMetaData.push_back(curveMetaData);
							dirtyParameter.DirtyCurveCount++;
						}
					}

					syncPacket->DataParameters.push_back(dirtyParameter);
				}
			}
			break;
		case ParamType::Texture:
			{
				const MaterialParamTextureData& textureParameterData = mTextureParameters[param.Index];

				MaterialParametersTextureParameter dirtyParameter;
				dirtyParameter.ParameterIndex = parameterIndex;
				dirtyParameter.IsLoadStore = textureParameterData.IsLoadStore;
				dirtyParameter.Surface = textureParameterData.Surface;
				dirtyParameter.Texture = B3DGetRenderProxy(textureParameterData.Texture);
				dirtyParameter.SpriteImage = B3DGetRenderProxy(textureParameterData.SpriteImage);

				syncPacket->TextureParameters.push_back(dirtyParameter);
			}
			break;
		case ParamType::Buffer:
			{
				const MaterialParamBufferData& bufferParameterData = mBufferParameters[param.Index];

				MaterialParametersBufferParameter dirtyParameter;
				dirtyParameter.ParameterIndex = parameterIndex;
				dirtyParameter.Buffer = B3DGetRenderProxy(bufferParameterData.Value);

				syncPacket->BufferParameters.push_back(dirtyParameter);
			}
			break;
		case ParamType::Sampler:
			{
				const MaterialParamSamplerStateData& samplerStateParameterData = mSamplerParameters[param.Index];

				MaterialParametersSamplerStateParameter dirtyParameter;
				dirtyParameter.ParameterIndex = parameterIndex;
				dirtyParameter.SamplerState = samplerStateParameterData.Value;

				syncPacket->SamplerStateParameters.push_back(dirtyParameter);
			}
			break;
		}
	}

	mLastSyncVersion = mParamVersion;
	return syncPacket;
}

void MaterialParameters::GetResourceDependencies(Vector<HResource>& resources)
{
	for(u32 i = 0; i < (u32)mParams.size(); i++)
	{
		ParamData& param = mParams[i];
		if(param.Type != ParamType::Texture)
			continue;

		const MaterialParamTextureData& textureData = mTextureParameters[param.Index];
		if(textureData.Texture != nullptr)
			resources.push_back(textureData.Texture);

		if(textureData.SpriteImage != nullptr)
			resources.push_back(textureData.SpriteImage);
	}
}

void MaterialParameters::GetCoreObjectDependencies(Vector<CoreObject*>& coreObjects)
{
	for(u32 i = 0; i < (u32)mParams.size(); i++)
	{
		ParamData& param = mParams[i];

		switch(param.Type)
		{
		case ParamType::Texture:
			{
				const MaterialParamTextureData& textureData = mTextureParameters[param.Index];

				if(textureData.Texture.IsLoaded())
					coreObjects.push_back(textureData.Texture.Get());

				if(textureData.SpriteImage.IsLoaded())
					coreObjects.push_back(textureData.SpriteImage.Get());
			}
			break;
		case ParamType::Buffer:
			{
				const MaterialParamBufferData& bufferData = mBufferParameters[param.Index];

				if(bufferData.Value != nullptr)
					coreObjects.push_back(bufferData.Value.get());
			}
			break;
		default:
			break;
		}
	}
}

RTTIType* MaterialParameters::GetRttiStatic()
{
	return MaterialParametersRTTI::Instance();
}

RTTIType* MaterialParameters::GetRtti() const
{
	return MaterialParameters::GetRttiStatic();
}

namespace b3d { namespace render
{
MaterialParameters::MaterialParameters(const SPtr<Shader>& shader, u64 initialParamVersion)
	: TMaterialParameters(shader, initialParamVersion)
{}

MaterialParameters::MaterialParameters(const SPtr<Shader>& shader, const SPtr<b3d::MaterialParameters>& params)
	: TMaterialParameters(shader, 1)
{
	memcpy(mDataParamsBuffer, params->mDataParamsBuffer, mDataSize);

	for(auto& param : mParams)
	{
		switch(param.Type)
		{
		case ParamType::Data:
			{
				const u32 arraySize = param.ArraySize > 1 ? param.ArraySize : 1;

				if(param.DataType != GPDT_STRUCT)
				{
					for(u32 i = 0; i < arraySize; i++)
					{
						DataParamInfo& srcParamInfo = params->mDataParameterMetaData[param.Index + i];
						DataParamInfo& dstParamInfo = mDataParameterMetaData[param.Index + i];

						if(srcParamInfo.FloatCurve)
							dstParamInfo.FloatCurve = B3DPoolNew<TAnimationCurve<float>>(*srcParamInfo.FloatCurve);

						if(srcParamInfo.ColorGradient)
							dstParamInfo.ColorGradient = B3DPoolNew<ColorGradientHDR>(*srcParamInfo.ColorGradient);
					}
				}
			}
			break;
		case ParamType::Texture:
			{
				HTexture texture = params->mTextureParameters[param.Index].Texture;
				HSpriteImage spriteImage = params->mTextureParameters[param.Index].SpriteImage;

				mTextureParameters[param.Index].Texture = B3DGetRenderProxy(texture);
				mTextureParameters[param.Index].SpriteImage = B3DGetRenderProxy(spriteImage);
				mTextureParameters[param.Index].IsLoadStore = params->mTextureParameters[param.Index].IsLoadStore;
				mTextureParameters[param.Index].Surface = params->mTextureParameters[param.Index].Surface;
			}
			break;
		case ParamType::Buffer:
			{
				SPtr<b3d::GpuBuffer> buffer = params->mBufferParameters[param.Index].Value;
				mBufferParameters[param.Index].Value = B3DGetRenderProxy(buffer);
			}
			break;
		case ParamType::Sampler:
			{
				SPtr<SamplerState> sampState = params->mSamplerParameters[param.Index].Value;
				mSamplerParameters[param.Index].Value = sampState;
			}
			break;
		default:
			break;
		}
	}
}

void MaterialParameters::ApplyAndDestroySyncPacket(FrameAllocator& allocator, const b3d::MaterialParameters::SyncPacket& syncPacket)
{
	mParamVersion++;

	for(const auto& dirtyParameter : syncPacket.DataParameters)
	{
		ParamData& parameterData = mParams[dirtyParameter.ParameterIndex];
		parameterData.Version = mParamVersion;

		const u32 arraySize = parameterData.ArraySize > 1 ? parameterData.ArraySize : 1;

		if(parameterData.DataType == GPDT_STRUCT)
		{
			const StructParameterMetaData& structParameterData = mStructParameterMetaData[parameterData.Index];

			// Param data
			for(u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
			{
				if(B3D_ENSURE((dirtyParameter.DataOffset + structParameterData.DataSize) <= syncPacket.DataParameterData.size()))
				{
					const StructParameterMetaData& arrayEntryMetaData = mStructParameterMetaData[parameterData.Index + arrayIndex];
					memcpy(&mDataParamsBuffer[arrayEntryMetaData.Offset], &syncPacket.DataParameterData[dirtyParameter.DataOffset], structParameterData.DataSize);
				}
			}
		}
		else
		{
			const GpuDataParameterTypeInformation& typeInfo = b3d::GpuParameterSet::kParamSizes.Lookup[(int)parameterData.DataType];
			const u32 paramSize = typeInfo.NumColumns * typeInfo.NumRows * typeInfo.BaseTypeSize;

			const DataParamInfo& parameterInformation = mDataParameterMetaData[parameterData.Index];
			const u32 dataParamSize = arraySize * paramSize;

			// Param data
			// Note: This relies on the fact that all data params in the array are sequential
			if(B3D_ENSURE((dirtyParameter.DataOffset + dataParamSize) <= syncPacket.DataParameterData.size()))
			{
				memcpy(&mDataParamsBuffer[parameterInformation.Offset], &syncPacket.DataParameterData[dirtyParameter.DataOffset], dataParamSize);
			}

			// Param curves
			for(u32 dirtyCurveIndex = 0; dirtyCurveIndex < dirtyParameter.DirtyCurveCount; dirtyCurveIndex++)
			{
				const MaterialParametersCurveMetaData& dirtyCurveMetaData = syncPacket.CurveMetaData[dirtyParameter.CurveMetaDataIndex + dirtyCurveIndex];

				DataParamInfo& arrayEntryParameterInformation = mDataParameterMetaData[parameterData.Index + dirtyCurveMetaData.ArrayIndex];
				if(parameterData.DataType == GPDT_FLOAT1)
				{
					if(arrayEntryParameterInformation.FloatCurve)
						B3DPoolFree(arrayEntryParameterInformation.FloatCurve);

					arrayEntryParameterInformation.FloatCurve = B3DPoolNew<TAnimationCurve<float>>();
					*arrayEntryParameterInformation.FloatCurve = syncPacket.FloatCurves[dirtyCurveMetaData.CurveIndex];
				}
				else if(parameterData.DataType == GPDT_COLOR)
				{
					if(arrayEntryParameterInformation.ColorGradient)
						B3DPoolFree(arrayEntryParameterInformation.ColorGradient);

					arrayEntryParameterInformation.ColorGradient = B3DPoolNew<ColorGradientHDR>();
					*arrayEntryParameterInformation.ColorGradient = syncPacket.ColorGradients[dirtyCurveMetaData.CurveIndex];
				}
			}
		}
	}

	for(const auto& dirtyParameter : syncPacket.TextureParameters)
	{
		ParamData& parameterData = mParams[dirtyParameter.ParameterIndex];
		parameterData.Version = mParamVersion;

		mTextureParameters[parameterData.Index].IsLoadStore = dirtyParameter.IsLoadStore;
		mTextureParameters[parameterData.Index].Surface = dirtyParameter.Surface;
		mTextureParameters[parameterData.Index].Texture = dirtyParameter.Texture;
		mTextureParameters[parameterData.Index].SpriteImage = dirtyParameter.SpriteImage;
	}

	for(const auto& dirtyParameter : syncPacket.BufferParameters)
	{
		ParamData& parameterData = mParams[dirtyParameter.ParameterIndex];
		parameterData.Version = mParamVersion;

		mBufferParameters[parameterData.Index].Value = dirtyParameter.Buffer;
	}

	for(const auto& dirtyParameter : syncPacket.SamplerStateParameters)
	{
		ParamData& parameterData = mParams[dirtyParameter.ParameterIndex];
		parameterData.Version = mParamVersion;

		mSamplerParameters[parameterData.Index].Value = dirtyParameter.SamplerState;
	}

	allocator.Destruct(&syncPacket);
}
}}
