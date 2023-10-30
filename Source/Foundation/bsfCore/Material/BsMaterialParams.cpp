//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/BsMaterialParams.h"
#include "Private/RTTI/BsMaterialParamsRTTI.h"
#include "Material/BsShader.h"
#include "Image/BsTexture.h"
#include "Image/BsSpriteTexture.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "RenderAPI/BsSamplerState.h"
#include "Image/BsColorGradient.h"
#include "Animation/BsAnimationCurve.h"
#include "Allocators/BsPoolAlloc.h"
#include "CoreObject/BsCoreObjectSync.h"

using namespace bs;

SPtr<ct::Texture> GetSpriteTextureAtlas(const SPtr<ct::SpriteTexture>& spriteTexture)
{
	if(spriteTexture)
		return spriteTexture->GetTexture();

	return nullptr;
}

HTexture GetSpriteTextureAtlas(const HSpriteTexture& spriteTexture)
{
	if(spriteTexture.IsLoaded())
		return spriteTexture->GetTexture();

	return HTexture();
}

MaterialParamsBase::MaterialParamsBase(
	const Map<String, ShaderDataParameterInformation>& dataParams,
	const Map<String, ShaderObjectParameterInformation>& textureParams,
	const Map<String, ShaderObjectParameterInformation>& bufferParams,
	const Map<String, ShaderObjectParameterInformation>& samplerParams,
	u64 initialParamVersion)
	: mParamVersion(initialParamVersion)
{
	mDataSize = 0;

	for(auto& param : dataParams)
	{
		if(param.second.Type == GPDT_UNKNOWN)
			continue;

		u32 arraySize = param.second.ArraySize > 1 ? param.second.ArraySize : 1;
		if(param.second.Type == GPDT_STRUCT)
			mNumStructParams += arraySize;
		else
		{
			const GpuDataParameterTypeInformation& typeInfo = GpuParameters::kParamSizes.Lookup[(int)param.second.Type];
			u32 paramSize = typeInfo.NumColumns * typeInfo.NumRows * typeInfo.BaseTypeSize;

			mDataSize += arraySize * paramSize;
			mNumDataParams += arraySize;
		}
	}

	mNumTextureParams = (u32)textureParams.size();
	mNumBufferParams = (u32)bufferParams.size();
	mNumSamplerParams = (u32)samplerParams.size();

	mDataParamsBuffer = mAlloc.Alloc(mDataSize);
	memset(mDataParamsBuffer, 0, mDataSize);

	mDataParams = (DataParamInfo*)mAlloc.Alloc(mNumDataParams * sizeof(DataParamInfo));
	memset(mDataParams, 0, mNumDataParams * sizeof(DataParamInfo));

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
			structParamIdx += arraySize;
		}
		else
		{
			dataParam.Index = dataParamIdx;

			const GpuDataParameterTypeInformation& typeInfo = GpuParameters::kParamSizes.Lookup[(int)dataParam.DataType];
			const u32 paramSize = typeInfo.NumColumns * typeInfo.NumRows * typeInfo.BaseTypeSize;
			for(u32 i = 0; i < arraySize; i++)
			{
				mDataParams[dataParamIdx].Offset = dataBufferIdx;
				mDataParams[dataParamIdx].SpriteTextureIdx = (u32)-1;

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

MaterialParamsBase::~MaterialParamsBase()
{
	for(u32 i = 0; i < mNumDataParams; i++)
	{
		DataParamInfo& paramInfo = mDataParams[i];

		if(paramInfo.FloatCurve)
		{
			B3DPoolFree(paramInfo.FloatCurve);
			paramInfo.FloatCurve = nullptr;
		}

		if(paramInfo.ColorGradient)
		{
			B3DPoolFree(paramInfo.ColorGradient);
			paramInfo.ColorGradient = nullptr;
		}
	}

	mAlloc.Free(mDataParamsBuffer);
	mAlloc.Free(mDataParams);

	mAlloc.Clear();
}

const ColorGradientHDR& MaterialParamsBase::GetColorGradientParam(const String& name, u32 arrayIdx) const
{
	static ColorGradientHDR EMPTY_GRADIENT;

	const ParamData* param = nullptr;
	auto result = GetParamData(name, ParamType::Data, GPDT_COLOR, arrayIdx, &param);
	if(result != GetParamResult::Success)
		return EMPTY_GRADIENT;

	return GetColorGradientParam(*param, arrayIdx);
}

void MaterialParamsBase::SetColorGradientParam(const String& name, u32 arrayIdx, const ColorGradientHDR& input) const
{
	const ParamData* param = nullptr;
	auto result = GetParamData(name, ParamType::Data, GPDT_COLOR, arrayIdx, &param);
	if(result != GetParamResult::Success)
		return;

	SetColorGradientParam(*param, arrayIdx, input);
}

const ColorGradientHDR& MaterialParamsBase::GetColorGradientParam(const ParamData& param, u32 arrayIdx) const
{
	const DataParamInfo& paramInfo = mDataParams[param.Index + arrayIdx];
	if(paramInfo.ColorGradient)
		return *paramInfo.ColorGradient;

	static ColorGradientHDR EMPTY_GRADIENT;
	return EMPTY_GRADIENT;
}

void MaterialParamsBase::SetColorGradientParam(const ParamData& param, u32 arrayIdx, const ColorGradientHDR& input) const
{
	DataParamInfo& paramInfo = mDataParams[param.Index + arrayIdx];
	if(paramInfo.ColorGradient)
		B3DPoolFree(paramInfo.ColorGradient);

	paramInfo.ColorGradient = B3DPoolNew<ColorGradientHDR>(input);

	param.Version = ++mParamVersion;
}

u32 MaterialParamsBase::GetParamIndex(const String& name) const
{
	auto iterFind = mParamLookup.find(name);
	if(iterFind == mParamLookup.end())
		return (u32)-1;

	return iterFind->second;
}

MaterialParamsBase::GetParamResult MaterialParamsBase::GetParamIndex(const String& name, ParamType type, GpuDataParameterType dataType, u32 arrayIdx, u32& output) const
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

MaterialParamsBase::GetParamResult MaterialParamsBase::GetParamData(const String& name, ParamType type, GpuDataParameterType dataType, u32 arrayIdx, const ParamData** output) const
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

void MaterialParamsBase::ReportGetParamError(GetParamResult errorCode, const String& name, u32 arrayIdx) const
{
	switch(errorCode)
	{
	case GetParamResult::NotFound:
		B3D_LOG(Warning, Material, "Material doesn't have a parameter named {0}.", name);
		break;
	case GetParamResult::InvalidType:
		B3D_LOG(Warning, Material, "Parameter \"{0}\" is not of the requested type.", name);
		break;
	case GetParamResult::IndexOutOfBounds:
		B3D_LOG(Warning, Material, "Parameter \"{0}\" array index {1} out of range.", name, arrayIdx);
		break;
	default:
		break;
	}
}

RTTITypeBase* MaterialParamStructData::GetRttiStatic()
{
	return MaterialParamStructDataRTTI::Instance();
}

RTTITypeBase* MaterialParamStructData::GetRtti() const
{
	return GetRttiStatic();
}

RTTITypeBase* MaterialParamTextureData::GetRttiStatic()
{
	return MaterialParamTextureDataRTTI::Instance();
}

RTTITypeBase* MaterialParamTextureData::GetRtti() const
{
	return GetRttiStatic();
}

template <bool IsRenderProxz>
TMaterialParams<IsRenderProxz>::TMaterialParams(const ShaderType& shader, u64 initialParamVersion)
	: MaterialParamsBase(
		  shader->GetDataParams(),
		  shader->GetTextureParams(),
		  shader->GetBufferParams(),
		  shader->GetSamplerParams(),
		  initialParamVersion)
{
	mStructParams = mAlloc.Construct<ParamStructDataType>(mNumStructParams);
	mTextureParams = mAlloc.Construct<ParamTextureDataType>(mNumTextureParams);
	mBufferParams = mAlloc.Construct<ParamBufferDataType>(mNumBufferParams);
	mSamplerStateParams = mAlloc.Construct<ParamSamplerStateDataType>(mNumSamplerParams);
	mDefaultTextureParams = mAlloc.Construct<TextureType>(mNumTextureParams);
	mDefaultSamplerStateParams = mAlloc.Construct<SPtr<SamplerState>>(mNumSamplerParams);

	auto& textureParams = shader->GetTextureParams();
	u32 textureIdx = 0;
	for(auto& entry : textureParams)
	{
		ParamTextureDataType& param = mTextureParams[textureIdx];
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
	u32 structIdx = 0;
	for(auto& entry : dataParams)
	{
		if(entry.second.Type == GPDT_STRUCT)
		{
			u32 arraySize = entry.second.ArraySize > 1 ? entry.second.ArraySize : 1;
			for(u32 i = 0; i < arraySize; i++)
			{
				ParamStructDataType& param = mStructParams[structIdx];
				param.DataSize = entry.second.ElementSize;
				param.Data = mAlloc.Alloc(param.DataSize);

				structIdx++;
			}
		}
		else
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

						DataParamInfo& dataParamInfo = mDataParams[paramData.Index];
						dataParamInfo.SpriteTextureIdx = findIterTex->second;
					}
				}

				attribIdx = attrib.NextParameterIndex;
			}
		}
	}
}

template <bool IsRenderProxz>
TMaterialParams<IsRenderProxz>::~TMaterialParams()
{
	if(mStructParams != nullptr)
	{
		for(u32 i = 0; i < mNumStructParams; i++)
			mAlloc.Free(mStructParams[i].Data);
	}

	mAlloc.Destruct(mStructParams, mNumStructParams);
	mAlloc.Destruct(mTextureParams, mNumTextureParams);
	mAlloc.Destruct(mBufferParams, mNumBufferParams);
	mAlloc.Destruct(mSamplerStateParams, mNumSamplerParams);

	if(mDefaultTextureParams != nullptr)
		mAlloc.Destruct(mDefaultTextureParams, mNumTextureParams);

	if(mDefaultSamplerStateParams != nullptr)
		mAlloc.Destruct(mDefaultSamplerStateParams, mNumSamplerParams);
}

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::GetStructData(const String& name, void* value, u32 size, u32 arrayIdx) const
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

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::SetStructData(const String& name, const void* value, u32 size, u32 arrayIdx)
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

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::GetTexture(const String& name, TextureType& value, TextureSurface& surface) const
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

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::SetTexture(const String& name, const TextureType& value, const TextureSurface& surface)
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

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::GetSpriteTexture(const String& name, SpriteTextureType& value) const
{
	const ParamData* param = nullptr;
	GetParamResult result = GetParamData(name, ParamType::Texture, GPDT_UNKNOWN, 0, &param);
	if(result != GetParamResult::Success)
	{
		ReportGetParamError(result, name, 0);
		return;
	}

	GetSpriteTexture(*param, value);
}

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::SetSpriteTexture(const String& name, const SpriteTextureType& value)
{
	const ParamData* param = nullptr;
	GetParamResult result = GetParamData(name, ParamType::Texture, GPDT_UNKNOWN, 0, &param);
	if(result != GetParamResult::Success)
	{
		ReportGetParamError(result, name, 0);
		return;
	}

	SetSpriteTexture(*param, value);
}

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::GetStorageTexture(const String& name, TextureType& value, TextureSurface& surface) const
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

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::SetStorageTexture(const String& name, const TextureType& value, const TextureSurface& surface)
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

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::GetBuffer(const String& name, BufferType& value) const
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

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::SetBuffer(const String& name, const BufferType& value)
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

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::GetSamplerState(const String& name, SPtr<SamplerState>& value) const
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

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::SetSamplerState(const String& name, const SPtr<SamplerState>& value)
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

template <bool IsRenderProxz>
bool TMaterialParams<IsRenderProxz>::IsAnimated(const String& name, u32 arrayIdx)
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

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::GetStructData(const ParamData& param, void* value, u32 size, u32 arrayIdx) const
{
	const ParamStructDataType& structParam = mStructParams[param.Index + arrayIdx];
	if(structParam.DataSize != size)
	{
		B3D_LOG(Warning, Material, "Size mismatch when writing to a struct. Provided size was {0} bytes but the struct "
								  "size is {1} bytes",
			   size, structParam.DataSize);
		return;
	}

	memcpy(value, structParam.Data, structParam.DataSize);
}

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::SetStructData(const ParamData& param, const void* value, u32 size, u32 arrayIdx)
{
	const ParamStructDataType& structParam = mStructParams[param.Index + arrayIdx];
	if(structParam.DataSize != size)
	{
		B3D_LOG(Warning, Material, "Size mismatch when writing to a struct. Provided size was {0} bytes but the struct "
								  "size is {1} bytes",
			   size, structParam.DataSize);
		return;
	}

	memcpy(structParam.Data, value, structParam.DataSize);
	param.Version = ++mParamVersion;
}

template <bool IsRenderProxz>
u32 TMaterialParams<IsRenderProxz>::GetStructSize(const ParamData& param) const
{
	const ParamStructDataType& structParam = mStructParams[param.Index];
	return structParam.DataSize;
}

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::GetTexture(const ParamData& param, TextureType& value, TextureSurface& surface) const
{
	ParamTextureDataType& textureParam = mTextureParams[param.Index];

	if(textureParam.Texture)
		value = textureParam.Texture;
	else if(textureParam.SpriteTexture)
		value = GetSpriteTextureAtlas(textureParam.SpriteTexture);

	surface = textureParam.Surface;
}

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::SetTexture(const ParamData& param, const TextureType& value, const TextureSurface& surface)
{
	ParamTextureDataType& textureParam = mTextureParams[param.Index];
	textureParam.Texture = value;
	textureParam.SpriteTexture = nullptr;
	textureParam.IsLoadStore = false;
	textureParam.Surface = surface;

	param.Version = ++mParamVersion;
}

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::GetSpriteTexture(const ParamData& param, SpriteTextureType& value) const
{
	ParamTextureDataType& textureParam = mTextureParams[param.Index];
	value = textureParam.SpriteTexture;
}

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::SetSpriteTexture(const ParamData& param, const SpriteTextureType& value)
{
	ParamTextureDataType& textureParam = mTextureParams[param.Index];
	textureParam.Texture = nullptr;
	textureParam.SpriteTexture = value;
	textureParam.IsLoadStore = false;
	textureParam.Surface = TextureSurface::kComplete;

	param.Version = ++mParamVersion;
}

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::GetBuffer(const ParamData& param, BufferType& value) const
{
	value = mBufferParams[param.Index].Value;
}

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::SetBuffer(const ParamData& param, const BufferType& value)
{
	mBufferParams[param.Index].Value = value;

	param.Version = ++mParamVersion;
}

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::GetStorageTexture(const ParamData& param, TextureType& value, TextureSurface& surface) const
{
	ParamTextureDataType& textureParam = mTextureParams[param.Index];
	value = textureParam.Texture;
	surface = textureParam.Surface;
}

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::SetStorageTexture(const ParamData& param, const TextureType& value, const TextureSurface& surface)
{
	ParamTextureDataType& textureParam = mTextureParams[param.Index];
	textureParam.Texture = value;
	textureParam.SpriteTexture = nullptr;
	textureParam.IsLoadStore = true;
	textureParam.Surface = surface;

	param.Version = ++mParamVersion;
}

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::GetSamplerState(const ParamData& param, SPtr<SamplerState>& value) const
{
	value = mSamplerStateParams[param.Index].Value;
}

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::SetSamplerState(const ParamData& param, const SPtr<SamplerState>& value)
{
	mSamplerStateParams[param.Index].Value = value;

	param.Version = ++mParamVersion;
}

template <bool IsRenderProxz>
MateralParamTextureType TMaterialParams<IsRenderProxz>::GetTextureType(const ParamData& param) const
{
	if(mTextureParams[param.Index].IsLoadStore)
		return MateralParamTextureType::LoadStore;

	if(mTextureParams[param.Index].SpriteTexture)
		return MateralParamTextureType::Sprite;

	return MateralParamTextureType::Normal;
}

template <bool IsRenderProxz>
bool TMaterialParams<IsRenderProxz>::IsAnimated(const ParamData& param, u32 arrayIdx) const
{
	const DataParamInfo& paramInfo = mDataParams[param.Index + arrayIdx];

	return paramInfo.FloatCurve || paramInfo.ColorGradient || paramInfo.SpriteTextureIdx != (u32)-1;
}

template <bool IsRenderProxz>
typename TMaterialParams<IsRenderProxz>::SpriteTextureType TMaterialParams<IsRenderProxz>::GetOwningSpriteTexture(const ParamData& param) const
{
	SpriteTextureType output;

	const DataParamInfo& paramInfo = mDataParams[param.Index];
	if(paramInfo.SpriteTextureIdx == (u32)-1)
		return output;

	const ParamData* spriteTexParamData = GetParamData(paramInfo.SpriteTextureIdx);
	if(spriteTexParamData)
		GetSpriteTexture(*spriteTexParamData, output);

	return output;
}

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::GetDefaultTexture(const ParamData& param, TextureType& value) const
{
	value = mDefaultTextureParams[param.Index];
}

template <bool IsRenderProxz>
void TMaterialParams<IsRenderProxz>::GetDefaultSamplerState(const ParamData& param, SPtr<SamplerState>& value) const
{
	value = mDefaultSamplerStateParams[param.Index];
}

template class TMaterialParams<true>;
template class TMaterialParams<false>;

namespace bs
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
		SPtr<ct::Texture> Texture;
		SPtr<ct::SpriteTexture> SpriteTexture;
		bool IsLoadStore;
		TextureSurface Surface;
	};

	struct MaterialParametersBufferParameter
	{
		u32 ParameterIndex = ~0u;
		SPtr<ct::GpuBuffer> Buffer;
	};

	struct MaterialParametersSamplerStateParameter
	{
		u32 ParameterIndex = ~0u;
		SPtr<SamplerState> SamplerState;
	};

	B3D_SYNC_BLOCK_BEGIN(MaterialParams, SyncPacket)
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

MaterialParams::MaterialParams(const HShader& shader, u64 initialParamVersion)
	: TMaterialParams(shader, initialParamVersion), mLastSyncVersion(1)
{}

MaterialParams::SyncPacket* MaterialParams::CreateSyncPacket(FrameAllocator& allocator, bool forceAll)
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
					const ParamStructDataType& paramData = mStructParams[param.Index];

					MaterialParametersDataParameter dirtyParameter;
					dirtyParameter.ParameterIndex = parameterIndex;
					dirtyParameter.DataOffset = (u32)syncPacket->DataParameterData.size();

					// Param data
					for(u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
					{
						const u8* const parameterdata = mStructParams[param.Index + arrayIndex].Data;
						syncPacket->DataParameterData.insert(syncPacket->DataParameterData.end(), parameterdata, parameterdata + paramData.DataSize);
					}

					syncPacket->DataParameters.push_back(dirtyParameter);
				}
				else
				{
					const GpuDataParameterTypeInformation& typeInfo = GpuParameters::kParamSizes.Lookup[(int)param.DataType];
					const u32 paramSize = typeInfo.NumColumns * typeInfo.NumRows * typeInfo.BaseTypeSize;

					const u32 dataSize = arraySize * paramSize;
					const DataParamInfo& paramInfo = mDataParams[param.Index];

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
						const DataParamInfo& arrayEntryParameterInformation = mDataParams[param.Index + arrayIndex];
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
				const MaterialParamTextureData& textureParameterData = mTextureParams[param.Index];

				MaterialParametersTextureParameter dirtyParameter;
				dirtyParameter.ParameterIndex = parameterIndex;
				dirtyParameter.IsLoadStore = textureParameterData.IsLoadStore;
				dirtyParameter.Surface = textureParameterData.Surface;
				dirtyParameter.Texture = B3DGetRenderProxy(textureParameterData.Texture);
				dirtyParameter.SpriteTexture = B3DGetRenderProxy(textureParameterData.SpriteTexture);

				syncPacket->TextureParameters.push_back(dirtyParameter);
			}
			break;
		case ParamType::Buffer:
			{
				const MaterialParamBufferData& bufferParameterData = mBufferParams[param.Index];

				MaterialParametersBufferParameter dirtyParameter;
				dirtyParameter.ParameterIndex = parameterIndex;
				dirtyParameter.Buffer = B3DGetRenderProxy(bufferParameterData.Value);

				syncPacket->BufferParameters.push_back(dirtyParameter);
			}
			break;
		case ParamType::Sampler:
			{
				const MaterialParamSamplerStateData& samplerStateParameterData = mSamplerStateParams[param.Index];

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

void MaterialParams::GetResourceDependencies(Vector<HResource>& resources)
{
	for(u32 i = 0; i < (u32)mParams.size(); i++)
	{
		ParamData& param = mParams[i];
		if(param.Type != ParamType::Texture)
			continue;

		const MaterialParamTextureData& textureData = mTextureParams[param.Index];
		if(textureData.Texture != nullptr)
			resources.push_back(textureData.Texture);

		if(textureData.SpriteTexture != nullptr)
			resources.push_back(textureData.SpriteTexture);
	}
}

void MaterialParams::GetCoreObjectDependencies(Vector<CoreObject*>& coreObjects)
{
	for(u32 i = 0; i < (u32)mParams.size(); i++)
	{
		ParamData& param = mParams[i];

		switch(param.Type)
		{
		case ParamType::Texture:
			{
				const MaterialParamTextureData& textureData = mTextureParams[param.Index];

				if(textureData.Texture.IsLoaded())
					coreObjects.push_back(textureData.Texture.Get());

				if(textureData.SpriteTexture.IsLoaded())
					coreObjects.push_back(textureData.SpriteTexture.Get());
			}
			break;
		case ParamType::Buffer:
			{
				const MaterialParamBufferData& bufferData = mBufferParams[param.Index];

				if(bufferData.Value != nullptr)
					coreObjects.push_back(bufferData.Value.get());
			}
			break;
		default:
			break;
		}
	}
}

RTTITypeBase* MaterialParams::GetRttiStatic()
{
	return MaterialParamsRTTI::Instance();
}

RTTITypeBase* MaterialParams::GetRtti() const
{
	return MaterialParams::GetRttiStatic();
}

namespace bs { namespace ct
{
MaterialParams::MaterialParams(const SPtr<Shader>& shader, u64 initialParamVersion)
	: TMaterialParams(shader, initialParamVersion)
{}

MaterialParams::MaterialParams(const SPtr<Shader>& shader, const SPtr<bs::MaterialParams>& params)
	: TMaterialParams(shader, 1)
{
	memcpy(mDataParamsBuffer, params->mDataParamsBuffer, mDataSize);

	for(auto& param : mParams)
	{
		switch(param.Type)
		{
		case ParamType::Data:
			{
				const u32 arraySize = param.ArraySize > 1 ? param.ArraySize : 1;

				if(param.DataType == GPDT_STRUCT)
				{
					for(u32 i = 0; i < arraySize; i++)
					{
						const MaterialParamStructData& srcParamInfo = params->mStructParams[param.Index + i];
						MaterialParamStructDataRenderProxy& dstParamInfo = mStructParams[param.Index + i];

						memcpy(dstParamInfo.Data, srcParamInfo.Data, srcParamInfo.DataSize);
					}
				}
				else
				{
					for(u32 i = 0; i < arraySize; i++)
					{
						DataParamInfo& srcParamInfo = params->mDataParams[param.Index + i];
						DataParamInfo& dstParamInfo = mDataParams[param.Index + i];

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
				HTexture texture = params->mTextureParams[param.Index].Texture;
				HSpriteTexture spriteTexture = params->mTextureParams[param.Index].SpriteTexture;

				mTextureParams[param.Index].Texture = B3DGetRenderProxy(texture);
				mTextureParams[param.Index].SpriteTexture = B3DGetRenderProxy(spriteTexture);
				mTextureParams[param.Index].IsLoadStore = params->mTextureParams[param.Index].IsLoadStore;
				mTextureParams[param.Index].Surface = params->mTextureParams[param.Index].Surface;
			}
			break;
		case ParamType::Buffer:
			{
				SPtr<bs::GpuBuffer> buffer = params->mBufferParams[param.Index].Value;
				mBufferParams[param.Index].Value = B3DGetRenderProxy(buffer);
			}
			break;
		case ParamType::Sampler:
			{
				SPtr<SamplerState> sampState = params->mSamplerStateParams[param.Index].Value;
				mSamplerStateParams[param.Index].Value = sampState;
			}
			break;
		default:
			break;
		}
	}
}

void MaterialParams::ApplyAndDestroySyncPacket(FrameAllocator& allocator, const bs::MaterialParams::SyncPacket& syncPacket)
{
	mParamVersion++;

	for(const auto& dirtyParameter : syncPacket.DataParameters)
	{
		ParamData& parameterData = mParams[dirtyParameter.ParameterIndex];
		parameterData.Version = mParamVersion;

		const u32 arraySize = parameterData.ArraySize > 1 ? parameterData.ArraySize : 1;

		if(parameterData.DataType == GPDT_STRUCT)
		{
			const ParamStructDataType& structParameterData = mStructParams[parameterData.Index];

			// Param data
			for(u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
			{
				if(B3D_ENSURE((dirtyParameter.DataOffset + structParameterData.DataSize) <= syncPacket.DataParameterData.size()))
				{
					memcpy(mStructParams[parameterData.Index + arrayIndex].Data, &syncPacket.DataParameterData[dirtyParameter.DataOffset], structParameterData.DataSize);
				}
			}
		}
		else
		{
			const GpuDataParameterTypeInformation& typeInfo = bs::GpuParameters::kParamSizes.Lookup[(int)parameterData.DataType];
			const u32 paramSize = typeInfo.NumColumns * typeInfo.NumRows * typeInfo.BaseTypeSize;

			const DataParamInfo& parameterInformation = mDataParams[parameterData.Index];
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

				DataParamInfo& arrayEntryParameterInformation = mDataParams[parameterData.Index + dirtyCurveMetaData.ArrayIndex];
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

		mTextureParams[parameterData.Index].IsLoadStore = dirtyParameter.IsLoadStore;
		mTextureParams[parameterData.Index].Surface = dirtyParameter.Surface;
		mTextureParams[parameterData.Index].Texture = dirtyParameter.Texture;
		mTextureParams[parameterData.Index].SpriteTexture = dirtyParameter.SpriteTexture;
	}

	for(const auto& dirtyParameter : syncPacket.BufferParameters)
	{
		ParamData& parameterData = mParams[dirtyParameter.ParameterIndex];
		parameterData.Version = mParamVersion;

		mBufferParams[parameterData.Index].Value = dirtyParameter.Buffer;
	}

	for(const auto& dirtyParameter : syncPacket.SamplerStateParameters)
	{
		ParamData& parameterData = mParams[dirtyParameter.ParameterIndex];
		parameterData.Version = mParamVersion;

		mSamplerStateParams[parameterData.Index].Value = dirtyParameter.SamplerState;
	}

	allocator.Destruct(&syncPacket);
}
}}
