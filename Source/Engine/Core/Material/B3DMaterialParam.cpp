//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/B3DMaterialParam.h"
#include "Material/B3DMaterialParameters.h"
#include "Material/B3DMaterial.h"
#include "Image/B3DColorGradient.h"

using namespace b3d;

template <int DATA_TYPE, bool IsRenderProxy>
TMaterialDataCommon<DATA_TYPE, IsRenderProxy>::TMaterialDataCommon(const String& name, const MaterialPtrType& material)
	: mParamIndex(0), mArraySize(0), mMaterial(nullptr)
{
	if(material != nullptr)
	{
		SPtr<MaterialParamsType> params = material->GetMaterialParameters();

		u32 paramIndex;
		auto result = params->GetParamIndex(name, MaterialParameters::ParamType::Data, (GpuDataParameterType)DATA_TYPE, 0, paramIndex);

		if(result == MaterialParameters::GetParamResult::Success)
		{
			const MaterialParameters::ParamData* data = params->GetParamData(paramIndex);

			mMaterial = material;
			mParamIndex = paramIndex;
			mArraySize = data->ArraySize;
		}
		else
			params->ReportGetParamError(result, name, 0);
	}
}

template <class T, bool IsRenderProxy>
void TMaterialParameterPrimitive<T, IsRenderProxy>::Set(const T& value, u32 arrayIdx) const
{
	if(this->mMaterial == nullptr)
		return;

	if(arrayIdx >= this->mArraySize)
	{
		B3D_LOG(Warning, LogMaterial, "Array index out of range. Provided index was {0} but array length is {1}", arrayIdx, this->mArraySize);
		return;
	}

	SPtr<typename Base::MaterialParamsType> params = this->mMaterial->GetMaterialParameters();
	const MaterialParameters::ParamData* data = params->GetParamData(this->mParamIndex);

	params->SetDataParam(*data, arrayIdx, value);
	this->mMaterial->MarkRenderProxyDataDirtyInternal();
}

template <class T, bool IsRenderProxy>
T TMaterialParameterPrimitive<T, IsRenderProxy>::Get(u32 arrayIdx) const
{
	T output{};
	if(this->mMaterial == nullptr || arrayIdx >= this->mArraySize)
		return output;

	SPtr<typename Base::MaterialParamsType> params = this->mMaterial->GetMaterialParameters();
	const MaterialParameters::ParamData* data = params->GetParamData(this->mParamIndex);

	params->GetDataParam(*data, arrayIdx, output);
	return output;
}

template <class T, bool IsRenderProxy>
void TMaterialParameterCurve<T, IsRenderProxy>::Set(TAnimationCurve<T> value, u32 arrayIdx) const
{
	if(this->mMaterial == nullptr)
		return;

	if(arrayIdx >= this->mArraySize)
	{
		B3D_LOG(Warning, LogMaterial, "Array index out of range. Provided index was {0} but array length is {1}", arrayIdx, this->mArraySize);
		return;
	}

	SPtr<typename Base::MaterialParamsType> params = this->mMaterial->GetMaterialParameters();
	const MaterialParameters::ParamData* data = params->GetParamData(this->mParamIndex);

	params->SetCurveParam(*data, arrayIdx, std::move(value));
	this->mMaterial->MarkRenderProxyDataDirtyInternal();
}

template <class T, bool IsRenderProxy>
const TAnimationCurve<T>& TMaterialParameterCurve<T, IsRenderProxy>::Get(u32 arrayIdx) const
{
	static TAnimationCurve<T> EMPTY_CURVE;

	if(this->mMaterial == nullptr || arrayIdx >= this->mArraySize)
		return EMPTY_CURVE;

	SPtr<typename Base::MaterialParamsType> params = this->mMaterial->GetMaterialParameters();
	const MaterialParameters::ParamData* data = params->GetParamData(this->mParamIndex);

	return params->template GetCurveParam<T>(*data, arrayIdx);
}

template <bool IsRenderProxy>
void TMaterialParameterColorGradient<IsRenderProxy>::Set(const ColorGradientHDR& value, u32 arrayIdx) const
{
	if(this->mMaterial == nullptr)
		return;

	if(arrayIdx >= this->mArraySize)
	{
		B3D_LOG(Warning, LogMaterial, "Array index out of range. Provided index was {0} but array length is {1}", arrayIdx, this->mArraySize);
		return;
	}

	SPtr<typename Base::MaterialParamsType> params = this->mMaterial->GetMaterialParameters();
	const MaterialParameters::ParamData* data = params->GetParamData(this->mParamIndex);

	params->SetColorGradientParam(*data, arrayIdx, value);
	this->mMaterial->MarkRenderProxyDataDirtyInternal();
}

template <bool IsRenderProxy>
const ColorGradientHDR& TMaterialParameterColorGradient<IsRenderProxy>::Get(u32 arrayIdx) const
{
	static ColorGradientHDR EMPTY_GRADIENT;

	if(this->mMaterial == nullptr || arrayIdx >= this->mArraySize)
		return EMPTY_GRADIENT;

	SPtr<typename Base::MaterialParamsType> params = this->mMaterial->GetMaterialParameters();
	const MaterialParameters::ParamData* data = params->GetParamData(this->mParamIndex);

	return params->GetColorGradientParam(*data, arrayIdx);
}

template <bool IsRenderProxy>
void TMaterialParameterStruct<IsRenderProxy>::Set(const void* value, u32 sizeBytes, u32 arrayIdx) const
{
	if(this->mMaterial == nullptr)
		return;

	if(arrayIdx >= this->mArraySize)
	{
		B3D_LOG(Warning, LogMaterial, "Array index out of range. Provided index was {0} but array length is {1}", arrayIdx, this->mArraySize);
		return;
	}

	SPtr<typename Base::MaterialParamsType> params = this->mMaterial->GetMaterialParameters();
	const MaterialParameters::ParamData* data = params->GetParamData(this->mParamIndex);

	params->SetStructData(*data, value, sizeBytes, arrayIdx);
	this->mMaterial->MarkRenderProxyDataDirtyInternal();
}

template <bool IsRenderProxy>
void TMaterialParameterStruct<IsRenderProxy>::Get(void* value, u32 sizeBytes, u32 arrayIdx) const
{
	if(this->mMaterial == nullptr || arrayIdx >= this->mArraySize)
		return;

	SPtr<typename Base::MaterialParamsType> params = this->mMaterial->GetMaterialParameters();
	const MaterialParameters::ParamData* data = params->GetParamData(this->mParamIndex);

	params->GetStructData(*data, value, sizeBytes, arrayIdx);
}

template <bool IsRenderProxy>
u32 TMaterialParameterStruct<IsRenderProxy>::GetElementSize() const
{
	if(this->mMaterial == nullptr)
		return 0;

	SPtr<typename Base::MaterialParamsType> params = this->mMaterial->GetMaterialParameters();
	const MaterialParameters::ParamData* data = params->GetParamData(this->mParamIndex);

	return params->GetStructSize(*data);
}

template <bool IsRenderProxy>
TMaterialParameterSampledTexture<IsRenderProxy>::TMaterialParameterSampledTexture(const String& name, const MaterialPtrType& material)
	: mParamIndex(0), mMaterial(nullptr)
{
	if(material != nullptr)
	{
		SPtr<MaterialParamsType> params = material->GetMaterialParameters();

		u32 paramIndex;
		auto result = params->GetParamIndex(name, MaterialParameters::ParamType::Texture, GPDT_UNKNOWN, 0, paramIndex);

		if(result == MaterialParameters::GetParamResult::Success)
		{
			mMaterial = material;
			mParamIndex = paramIndex;
		}
		else
			params->ReportGetParamError(result, name, 0);
	}
}

template <bool IsRenderProxy>
void TMaterialParameterSampledTexture<IsRenderProxy>::Set(const TextureType& texture, const TextureSurface& surface) const
{
	if(mMaterial == nullptr)
		return;

	SPtr<MaterialParamsType> params = mMaterial->GetMaterialParameters();
	const MaterialParameters::ParamData* data = params->GetParamData(mParamIndex);

	// If there is a default value, assign that instead of null
	TextureType newValue = texture;
	if(newValue == nullptr)
		params->GetDefaultTexture(*data, newValue);

	params->SetTexture(*data, newValue, surface);
	mMaterial->MarkRenderProxyDataDirtyInternal();
	mMaterial->MarkDependenciesDirtyInternal();
	mMaterial->MarkResourcesDirtyInternal();
}

template <bool IsRenderProxy>
typename TMaterialParameterSampledTexture<IsRenderProxy>::TextureType TMaterialParameterSampledTexture<IsRenderProxy>::Get() const
{
	TextureType texture;
	if(mMaterial == nullptr)
		return texture;

	TextureSurface surface;

	SPtr<MaterialParamsType> params = mMaterial->GetMaterialParameters();
	const MaterialParameters::ParamData* data = params->GetParamData(mParamIndex);

	params->GetTexture(*data, texture, surface);
	return texture;
}

template <bool IsRenderProxy>
TMaterialParamSpriteImage<IsRenderProxy>::TMaterialParamSpriteImage(const String& name, const MaterialPtrType& material)
	: mParamIndex(0), mMaterial(nullptr)
{
	if(material != nullptr)
	{
		SPtr<MaterialParamsType> params = material->GetMaterialParameters();

		u32 paramIndex;
		auto result = params->GetParamIndex(name, MaterialParameters::ParamType::Texture, GPDT_UNKNOWN, 0, paramIndex);

		if(result == MaterialParameters::GetParamResult::Success)
		{
			mMaterial = material;
			mParamIndex = paramIndex;
		}
		else
			params->ReportGetParamError(result, name, 0);
	}
}

template <bool IsRenderProxy>
void TMaterialParamSpriteImage<IsRenderProxy>::Set(const SpriteImageType& image) const
{
	if(mMaterial == nullptr)
		return;

	SPtr<MaterialParamsType> params = mMaterial->GetMaterialParameters();
	const MaterialParameters::ParamData* data = params->GetParamData(mParamIndex);

	if(image == nullptr)
	{
		// If there is a default value, assign that instead of null
		TextureType newValue;
		params->GetDefaultTexture(*data, newValue);
		params->SetTexture(*data, newValue, TextureSurface::kComplete);
	}
	else
		params->SetSpriteImage(*data, image);

	mMaterial->MarkRenderProxyDataDirtyInternal();
	mMaterial->MarkDependenciesDirtyInternal();
	mMaterial->MarkResourcesDirtyInternal();
}

template <bool IsRenderProxy>
typename TMaterialParamSpriteImage<IsRenderProxy>::SpriteImageType TMaterialParamSpriteImage<IsRenderProxy>::Get() const
{
	SpriteImageType texture;
	if(mMaterial == nullptr)
		return texture;

	SPtr<MaterialParamsType> params = mMaterial->GetMaterialParameters();
	const MaterialParameters::ParamData* data = params->GetParamData(mParamIndex);

	params->GetSpriteImage(*data, texture);
	return texture;
}

template <bool IsRenderProxy>
TMaterialParameterStorageTexture<IsRenderProxy>::TMaterialParameterStorageTexture(const String& name, const MaterialPtrType& material)
	: mParamIndex(0), mMaterial(nullptr)
{
	if(material != nullptr)
	{
		SPtr<MaterialParamsType> params = material->GetMaterialParameters();

		u32 paramIndex;
		auto result = params->GetParamIndex(name, MaterialParameters::ParamType::Texture, GPDT_UNKNOWN, 0, paramIndex);

		if(result == MaterialParameters::GetParamResult::Success)
		{
			mMaterial = material;
			mParamIndex = paramIndex;
		}
		else
			params->ReportGetParamError(result, name, 0);
	}
}

template <bool IsRenderProxy>
void TMaterialParameterStorageTexture<IsRenderProxy>::Set(const TextureType& texture, const TextureSurface& surface) const
{
	if(mMaterial == nullptr)
		return;

	SPtr<MaterialParamsType> params = mMaterial->GetMaterialParameters();
	const MaterialParameters::ParamData* data = params->GetParamData(mParamIndex);

	params->SetStorageTexture(*data, texture, surface);
	mMaterial->MarkRenderProxyDataDirtyInternal();
	mMaterial->MarkDependenciesDirtyInternal();
	mMaterial->MarkResourcesDirtyInternal();
}

template <bool IsRenderProxy>
typename TMaterialParameterStorageTexture<IsRenderProxy>::TextureType TMaterialParameterStorageTexture<IsRenderProxy>::Get() const
{
	TextureType texture;
	if(mMaterial == nullptr)
		return texture;

	TextureSurface surface;

	SPtr<MaterialParamsType> params = mMaterial->GetMaterialParameters();
	const MaterialParameters::ParamData* data = params->GetParamData(mParamIndex);

	params->GetStorageTexture(*data, texture, surface);

	return texture;
}

template <bool IsRenderProxy>
TMaterialParameterBuffer<IsRenderProxy>::TMaterialParameterBuffer(const String& name, const MaterialPtrType& material)
	: mParamIndex(0), mMaterial(nullptr)
{
	if(material != nullptr)
	{
		SPtr<MaterialParamsType> params = material->GetMaterialParameters();

		u32 paramIndex;
		auto result = params->GetParamIndex(name, MaterialParameters::ParamType::Buffer, GPDT_UNKNOWN, 0, paramIndex);

		if(result == MaterialParameters::GetParamResult::Success)
		{
			mMaterial = material;
			mParamIndex = paramIndex;
		}
		else
			params->ReportGetParamError(result, name, 0);
	}
}

template <bool IsRenderProxy>
void TMaterialParameterBuffer<IsRenderProxy>::Set(const BufferType& buffer) const
{
	if(mMaterial == nullptr)
		return;

	SPtr<MaterialParamsType> params = mMaterial->GetMaterialParameters();
	const MaterialParameters::ParamData* data = params->GetParamData(mParamIndex);

	params->SetBuffer(*data, buffer);
	mMaterial->MarkRenderProxyDataDirtyInternal();
	mMaterial->MarkDependenciesDirtyInternal();
}

template <bool IsRenderProxy>
typename TMaterialParameterBuffer<IsRenderProxy>::BufferType TMaterialParameterBuffer<IsRenderProxy>::Get() const
{
	BufferType buffer;
	if(mMaterial == nullptr)
		return buffer;

	SPtr<MaterialParamsType> params = mMaterial->GetMaterialParameters();
	const MaterialParameters::ParamData* data = params->GetParamData(mParamIndex);
	params->GetBuffer(*data, buffer);

	return buffer;
}

template <bool IsRenderProxy>
TMaterialParameterSampler<IsRenderProxy>::TMaterialParameterSampler(const String& name, const MaterialPtrType& material)
	: mParamIndex(0), mMaterial(nullptr)
{
	if(material != nullptr)
	{
		SPtr<MaterialParamsType> params = material->GetMaterialParameters();

		u32 paramIndex;
		auto result = params->GetParamIndex(name, MaterialParameters::ParamType::Sampler, GPDT_UNKNOWN, 0, paramIndex);

		if(result == MaterialParameters::GetParamResult::Success)
		{
			mMaterial = material;
			mParamIndex = paramIndex;
		}
		else
			params->ReportGetParamError(result, name, 0);
	}
}

template <bool IsRenderProxy>
void TMaterialParameterSampler<IsRenderProxy>::Set(const SPtr<SamplerState>& sampState) const
{
	if(mMaterial == nullptr)
		return;

	SPtr<MaterialParamsType> params = mMaterial->GetMaterialParameters();
	const MaterialParameters::ParamData* data = params->GetParamData(mParamIndex);

	// If there is a default value, assign that instead of null
	SPtr<SamplerState> newValue = sampState;
	if(newValue == nullptr)
		params->GetDefaultSamplerState(*data, newValue);

	params->SetSamplerState(*data, newValue);
	mMaterial->MarkRenderProxyDataDirtyInternal();
	mMaterial->MarkDependenciesDirtyInternal();
}

template <bool IsRenderProxy>
SPtr<SamplerState> TMaterialParameterSampler<IsRenderProxy>::Get() const
{
	SPtr<SamplerState> samplerState;
	if(mMaterial == nullptr)
		return samplerState;

	SPtr<MaterialParamsType> params = mMaterial->GetMaterialParameters();
	const MaterialParameters::ParamData* data = params->GetParamData(mParamIndex);

	params->GetSamplerState(*data, samplerState);
	return samplerState;
}

#define MATERIAL_DATA_PARAM_INSTATIATE(type)                                    \
	template class TMaterialDataCommon<TGpuDataParamInfo<type>::TypeId, false>; \
	template class TMaterialDataCommon<TGpuDataParamInfo<type>::TypeId, true>;  \
	template class TMaterialParameterPrimitive<type, false>;                    \
	template class TMaterialParameterPrimitive<type, true>;

MATERIAL_DATA_PARAM_INSTATIATE(float)
MATERIAL_DATA_PARAM_INSTATIATE(double)
MATERIAL_DATA_PARAM_INSTATIATE(Color)
MATERIAL_DATA_PARAM_INSTATIATE(Vector2)
MATERIAL_DATA_PARAM_INSTATIATE(Vector3)
MATERIAL_DATA_PARAM_INSTATIATE(Vector4)
MATERIAL_DATA_PARAM_INSTATIATE(i32)
MATERIAL_DATA_PARAM_INSTATIATE(Vector2I)
MATERIAL_DATA_PARAM_INSTATIATE(Vector3I)
MATERIAL_DATA_PARAM_INSTATIATE(Vector4I)
MATERIAL_DATA_PARAM_INSTATIATE(u32)
MATERIAL_DATA_PARAM_INSTATIATE(Vector2UI)
MATERIAL_DATA_PARAM_INSTATIATE(Vector3UI)
MATERIAL_DATA_PARAM_INSTATIATE(Vector4UI)
MATERIAL_DATA_PARAM_INSTATIATE(Matrix2)
MATERIAL_DATA_PARAM_INSTATIATE(Matrix2x3)
MATERIAL_DATA_PARAM_INSTATIATE(Matrix2x4)
MATERIAL_DATA_PARAM_INSTATIATE(Matrix3)
MATERIAL_DATA_PARAM_INSTATIATE(Matrix3x2)
MATERIAL_DATA_PARAM_INSTATIATE(Matrix3x4)
MATERIAL_DATA_PARAM_INSTATIATE(Matrix4)
MATERIAL_DATA_PARAM_INSTATIATE(Matrix4x2)
MATERIAL_DATA_PARAM_INSTATIATE(Matrix4x3)

#undef MATERIAL_DATA_PARAM_INSTATIATE

template class TMaterialDataCommon<GPDT_STRUCT, false>;
template class TMaterialDataCommon<GPDT_STRUCT, true>;
template class TMaterialParameterStruct<false>;
template class TMaterialParameterStruct<true>;

template class TMaterialParameterCurve<float, false>;
template class TMaterialParameterCurve<float, true>;

template class TMaterialParameterColorGradient<false>;
template class TMaterialParameterColorGradient<true>;

template class TMaterialParameterSampledTexture<false>;
template class TMaterialParameterSampledTexture<true>;

template class TMaterialParamSpriteImage<false>;
template class TMaterialParamSpriteImage<true>;

template class TMaterialParameterStorageTexture<false>;
template class TMaterialParameterStorageTexture<true>;

template class TMaterialParameterBuffer<false>;
template class TMaterialParameterBuffer<true>;

template class TMaterialParameterSampler<false>;
template class TMaterialParameterSampler<true>;
