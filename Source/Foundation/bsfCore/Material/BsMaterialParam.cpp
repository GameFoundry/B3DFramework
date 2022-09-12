//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/BsMaterialParam.h"
#include "Math/BsVector2I.h"
#include "Material/BsMaterialParams.h"
#include "Material/BsMaterial.h"
#include "Image/BsColorGradient.h"

namespace bs
{
	template<int DATA_TYPE, bool Core>
	TMaterialDataCommon<DATA_TYPE, Core>::TMaterialDataCommon(const String& name, const MaterialPtrType& material)
		:mParamIndex(0), mArraySize(0), mMaterial(nullptr)
	{
		if(material != nullptr)
		{
			SPtr<MaterialParamsType> params = material->_getInternalParams();

			UINT32 paramIndex;
			auto result = params->GetParamIndex(name, MaterialParams::ParamType::Data, (GpuParamDataType)DATA_TYPE, 0,
				paramIndex);

			if (result == MaterialParams::GetParamResult::Success)
			{
				const MaterialParams::ParamData* data = params->GetParamData(paramIndex);

				mMaterial = material;
				mParamIndex = paramIndex;
				mArraySize = data->arraySize;
			}
			else
				params->ReportGetParamError(result, name, 0);
		}
	}

	template<class T, bool Core>
	void TMaterialDataParam<T, Core>::set(const T& value, UINT32 arrayIdx) const
	{
		if (this->mMaterial == nullptr)
			return;

		if(arrayIdx >= this->mArraySize)
		{
			BS_LOG(Warning, Material, "Array index out of range. Provided index was {0} but array length is {1}",
				arrayIdx, this->mArraySize);
			return;
		}

		SPtr<typename Base::MaterialParamsType> params = this->mMaterial->_getInternalParams();
		const MaterialParams::ParamData* data = params->GetParamData(this->mParamIndex);

		params->SetDataParam(*data, arrayIdx, value);
		this->mMaterial->_markCoreDirty();
	}

	template<class T, bool Core>
	T TMaterialDataParam<T, Core>::get(UINT32 arrayIdx) const
	{
		T output{};
		if (this->mMaterial == nullptr || arrayIdx >= this->mArraySize)
			return output;

		SPtr<typename Base::MaterialParamsType> params = this->mMaterial->_getInternalParams();
		const MaterialParams::ParamData* data = params->GetParamData(this->mParamIndex);

		params->GetDataParam(*data, arrayIdx, output);
		return output;
	}

	template<class T, bool Core>
	void TMaterialCurveParam<T, Core>::set(TAnimationCurve<T> value, UINT32 arrayIdx) const
	{
		if (this->mMaterial == nullptr)
			return;

		if(arrayIdx >= this->mArraySize)
		{
			BS_LOG(Warning, Material, "Array index out of range. Provided index was {0} but array length is {1}",
				arrayIdx, this->mArraySize);
			return;
		}

		SPtr<typename Base::MaterialParamsType> params = this->mMaterial->_getInternalParams();
		const MaterialParams::ParamData* data = params->GetParamData(this->mParamIndex);

		params->SetCurveParam(*data, arrayIdx, std::move(value));
		this->mMaterial->_markCoreDirty();
	}

	template<class T, bool Core>
	const TAnimationCurve<T>& TMaterialCurveParam<T, Core>::get(UINT32 arrayIdx) const
	{
		static TAnimationCurve<T> EMPTY_CURVE;

		if (this->mMaterial == nullptr || arrayIdx >= this->mArraySize)
			return EMPTY_CURVE;

		SPtr<typename Base::MaterialParamsType> params = this->mMaterial->_getInternalParams();
		const MaterialParams::ParamData* data = params->GetParamData(this->mParamIndex);

		return params->template getCurveParam<T>(*data, arrayIdx);
	}

	template<bool Core>
	void TMaterialColorGradientParam<Core>::set(const ColorGradientHDR& value, UINT32 arrayIdx) const
	{
		if (this->mMaterial == nullptr)
			return;

		if(arrayIdx >= this->mArraySize)
		{
			BS_LOG(Warning, Material, "Array index out of range. Provided index was {0} but array length is {1}",
				arrayIdx, this->mArraySize);
			return;
		}

		SPtr<typename Base::MaterialParamsType> params = this->mMaterial->_getInternalParams();
		const MaterialParams::ParamData* data = params->GetParamData(this->mParamIndex);

		params->SetColorGradientParam(*data, arrayIdx, value);
		this->mMaterial->_markCoreDirty();
	}

	template<bool Core>
	const ColorGradientHDR& TMaterialColorGradientParam<Core>::get(UINT32 arrayIdx) const
	{
		static ColorGradientHDR EMPTY_GRADIENT;

		if (this->mMaterial == nullptr || arrayIdx >= this->mArraySize)
			return EMPTY_GRADIENT;

		SPtr<typename Base::MaterialParamsType> params = this->mMaterial->_getInternalParams();
		const MaterialParams::ParamData* data = params->GetParamData(this->mParamIndex);

		return params->GetColorGradientParam(*data, arrayIdx);
	}

	template<bool Core>
	void TMaterialParamStruct<Core>::set(const void* value, UINT32 sizeBytes, UINT32 arrayIdx) const
	{
		if (this->mMaterial == nullptr)
			return;

		if (arrayIdx >= this->mArraySize)
		{
			BS_LOG(Warning, Material, "Array index out of range. Provided index was {0} but array length is {1}",
				arrayIdx, this->mArraySize);
			return;
		}

		SPtr<typename Base::MaterialParamsType> params = this->mMaterial->_getInternalParams();
		const MaterialParams::ParamData* data = params->GetParamData(this->mParamIndex);

		params->SetStructData(*data, value, sizeBytes, arrayIdx);
		this->mMaterial->_markCoreDirty();
	}

	template<bool Core>
	void TMaterialParamStruct<Core>::get(void* value, UINT32 sizeBytes, UINT32 arrayIdx) const
	{
		if (this->mMaterial == nullptr || arrayIdx >= this->mArraySize)
			return;

		SPtr<typename Base::MaterialParamsType> params = this->mMaterial->_getInternalParams();
		const MaterialParams::ParamData* data = params->GetParamData(this->mParamIndex);

		params->GetStructData(*data, value, sizeBytes, arrayIdx);
	}

	template<bool Core>
	UINT32 TMaterialParamStruct<Core>::getElementSize() const
	{
		if (this->mMaterial == nullptr)
			return 0;

		SPtr<typename Base::MaterialParamsType> params = this->mMaterial->_getInternalParams();
		const MaterialParams::ParamData* data = params->GetParamData(this->mParamIndex);

		return params->GetStructSize(*data);
	}

	template<bool Core>
	TMaterialParamTexture<Core>::TMaterialParamTexture(const String& name, const MaterialPtrType& material)
		:mParamIndex(0), mMaterial(nullptr)
	{
		if (material != nullptr)
		{
			SPtr<MaterialParamsType> params = material->_getInternalParams();

			UINT32 paramIndex;
			auto result = params->GetParamIndex(name, MaterialParams::ParamType::Texture, GPDT_UNKNOWN, 0, paramIndex);

			if (result == MaterialParams::GetParamResult::Success)
			{
				mMaterial = material;
				mParamIndex = paramIndex;
			}
			else
				params->ReportGetParamError(result, name, 0);
		}
	}

	template<bool Core>
	void TMaterialParamTexture<Core>::set(const TextureType& texture, const TextureSurface& surface) const
	{
		if (mMaterial == nullptr)
			return;

		SPtr<MaterialParamsType> params = mMaterial->_getInternalParams();
		const MaterialParams::ParamData* data = params->GetParamData(mParamIndex);

		// If there is a default value, assign that instead of null
		TextureType newValue = texture;
		if (newValue == nullptr)
			params->GetDefaultTexture(*data, newValue);

		params->SetTexture(*data, newValue, surface);
		mMaterial->_markCoreDirty();
		mMaterial->_markDependenciesDirty();
		mMaterial->_markResourcesDirty();
	}

	template<bool Core>
	typename TMaterialParamTexture<Core>::TextureType TMaterialParamTexture<Core>::get() const
	{
		TextureType texture;
		if (mMaterial == nullptr)
			return texture;

		TextureSurface surface;

		SPtr<MaterialParamsType> params = mMaterial->_getInternalParams();
		const MaterialParams::ParamData* data = params->GetParamData(mParamIndex);

		params->GetTexture(*data, texture, surface);
		return texture;
	}
	
	template<bool Core>
	TMaterialParamSpriteTexture<Core>::TMaterialParamSpriteTexture(const String& name, const MaterialPtrType& material)
		:mParamIndex(0), mMaterial(nullptr)
	{
		if (material != nullptr)
		{
			SPtr<MaterialParamsType> params = material->_getInternalParams();

			UINT32 paramIndex;
			auto result = params->GetParamIndex(name, MaterialParams::ParamType::Texture, GPDT_UNKNOWN, 0, paramIndex);

			if (result == MaterialParams::GetParamResult::Success)
			{
				mMaterial = material;
				mParamIndex = paramIndex;
			}
			else
				params->ReportGetParamError(result, name, 0);
		}
	}

	template<bool Core>
	void TMaterialParamSpriteTexture<Core>::set(const SpriteTextureType& texture) const
	{
		if (mMaterial == nullptr)
			return;

		SPtr<MaterialParamsType> params = mMaterial->_getInternalParams();
		const MaterialParams::ParamData* data = params->GetParamData(mParamIndex);

		if(texture == nullptr)
		{
			// If there is a default value, assign that instead of null
			TextureType newValue;
			params->GetDefaultTexture(*data, newValue);
			params->SetTexture(*data, newValue, TextureSurface::COMPLETE);
		}
		else
			params->SetSpriteTexture(*data, texture);
		
		mMaterial->_markCoreDirty();
		mMaterial->_markDependenciesDirty();
		mMaterial->_markResourcesDirty();
	}

	template<bool Core>
	typename TMaterialParamSpriteTexture<Core>::SpriteTextureType TMaterialParamSpriteTexture<Core>::get() const
	{
		SpriteTextureType texture;
		if (mMaterial == nullptr)
			return texture;

		SPtr<MaterialParamsType> params = mMaterial->_getInternalParams();
		const MaterialParams::ParamData* data = params->GetParamData(mParamIndex);

		params->GetSpriteTexture(*data, texture);
		return texture;
	}

	template<bool Core>
	TMaterialParamLoadStoreTexture<Core>::TMaterialParamLoadStoreTexture(const String& name,
		const MaterialPtrType& material)
		:mParamIndex(0), mMaterial(nullptr)
	{
		if (material != nullptr)
		{
			SPtr<MaterialParamsType> params = material->_getInternalParams();

			UINT32 paramIndex;
			auto result = params->GetParamIndex(name, MaterialParams::ParamType::Texture, GPDT_UNKNOWN, 0, paramIndex);

			if (result == MaterialParams::GetParamResult::Success)
			{
				mMaterial = material;
				mParamIndex = paramIndex;
			}
			else
				params->ReportGetParamError(result, name, 0);
		}
	}

	template<bool Core>
	void TMaterialParamLoadStoreTexture<Core>::set(const TextureType& texture, const TextureSurface& surface) const
	{
		if (mMaterial == nullptr)
			return;

		SPtr<MaterialParamsType> params = mMaterial->_getInternalParams();
		const MaterialParams::ParamData* data = params->GetParamData(mParamIndex);

		params->SetLoadStoreTexture(*data, texture, surface);
		mMaterial->_markCoreDirty();
		mMaterial->_markDependenciesDirty();
		mMaterial->_markResourcesDirty();
	}

	template<bool Core>
	typename TMaterialParamLoadStoreTexture<Core>::TextureType TMaterialParamLoadStoreTexture<Core>::get() const
	{
		TextureType texture;
		if (mMaterial == nullptr)
			return texture;

		TextureSurface surface;

		SPtr<MaterialParamsType> params = mMaterial->_getInternalParams();
		const MaterialParams::ParamData* data = params->GetParamData(mParamIndex);

		params->GetLoadStoreTexture(*data, texture, surface);

		return texture;
	}
	
	template<bool Core>
	TMaterialParamBuffer<Core>::TMaterialParamBuffer(const String& name, const MaterialPtrType& material)
		:mParamIndex(0), mMaterial(nullptr)
	{
		if (material != nullptr)
		{
			SPtr<MaterialParamsType> params = material->_getInternalParams();

			UINT32 paramIndex;
			auto result = params->GetParamIndex(name, MaterialParams::ParamType::Buffer, GPDT_UNKNOWN, 0, paramIndex);

			if (result == MaterialParams::GetParamResult::Success)
			{
				mMaterial = material;
				mParamIndex = paramIndex;
			}
			else
				params->ReportGetParamError(result, name, 0);
		}
	}

	template<bool Core>
	void TMaterialParamBuffer<Core>::set(const BufferType& buffer) const
	{
		if (mMaterial == nullptr)
			return;

		SPtr<MaterialParamsType> params = mMaterial->_getInternalParams();
		const MaterialParams::ParamData* data = params->GetParamData(mParamIndex);

		params->SetBuffer(*data, buffer);
		mMaterial->_markCoreDirty();
		mMaterial->_markDependenciesDirty();
	}

	template<bool Core>
	typename TMaterialParamBuffer<Core>::BufferType TMaterialParamBuffer<Core>::get() const
	{
		BufferType buffer;
		if (mMaterial == nullptr)
			return buffer;

		SPtr<MaterialParamsType> params = mMaterial->_getInternalParams();
		const MaterialParams::ParamData* data = params->GetParamData(mParamIndex);
		params->GetBuffer(*data, buffer);

		return buffer;
	}

	template<bool Core>
	TMaterialParamSampState<Core>::TMaterialParamSampState(const String& name, const MaterialPtrType& material)
		:mParamIndex(0), mMaterial(nullptr)
	{
		if (material != nullptr)
		{
			SPtr<MaterialParamsType> params = material->_getInternalParams();

			UINT32 paramIndex;
			auto result = params->GetParamIndex(name, MaterialParams::ParamType::Sampler, GPDT_UNKNOWN, 0, paramIndex);

			if (result == MaterialParams::GetParamResult::Success)
			{
				mMaterial = material;
				mParamIndex = paramIndex;
			}
			else
				params->ReportGetParamError(result, name, 0);
		}
	}

	template<bool Core>
	void TMaterialParamSampState<Core>::set(const SamplerStateType& sampState) const
	{
		if (mMaterial == nullptr)
			return;

		SPtr<MaterialParamsType> params = mMaterial->_getInternalParams();
		const MaterialParams::ParamData* data = params->GetParamData(mParamIndex);

		// If there is a default value, assign that instead of null
		SamplerStateType newValue = sampState;
		if (newValue == nullptr)
			params->GetDefaultSamplerState(*data, newValue);

		params->SetSamplerState(*data, newValue);
		mMaterial->_markCoreDirty();
		mMaterial->_markDependenciesDirty();
	}

	template<bool Core>
	typename TMaterialParamSampState<Core>::SamplerStateType TMaterialParamSampState<Core>::get() const
	{
		SamplerStateType samplerState;
		if (mMaterial == nullptr)
			return samplerState;

		SPtr<MaterialParamsType> params = mMaterial->_getInternalParams();
		const MaterialParams::ParamData* data = params->GetParamData(mParamIndex);

		params->GetSamplerState(*data, samplerState);
		return samplerState;
	}

#define MATERIAL_DATA_PARAM_INSTATIATE(type)										\
	template class TMaterialDataCommon<TGpuDataParamInfo<type>::TypeId, false>;		\
	template class TMaterialDataCommon<TGpuDataParamInfo<type>::TypeId, true>;		\
	template class TMaterialDataParam<type, false>;									\
	template class TMaterialDataParam<type, true>;

	MATERIAL_DATA_PARAM_INSTATIATE(float)
	MATERIAL_DATA_PARAM_INSTATIATE(int)
	MATERIAL_DATA_PARAM_INSTATIATE(Color)
	MATERIAL_DATA_PARAM_INSTATIATE(Vector2)
	MATERIAL_DATA_PARAM_INSTATIATE(Vector3)
	MATERIAL_DATA_PARAM_INSTATIATE(Vector4)
	MATERIAL_DATA_PARAM_INSTATIATE(Vector2I)
	MATERIAL_DATA_PARAM_INSTATIATE(Vector3I)
	MATERIAL_DATA_PARAM_INSTATIATE(Vector4I)
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
	template class TMaterialParamStruct<false>;
	template class TMaterialParamStruct<true>;

	template class TMaterialCurveParam<float, false>;
	template class TMaterialCurveParam<float, true>;

	template class TMaterialColorGradientParam<false>;
	template class TMaterialColorGradientParam<true>;

	template class TMaterialParamTexture<false>;
	template class TMaterialParamTexture<true>;

	template class TMaterialParamSpriteTexture<false>;
	template class TMaterialParamSpriteTexture<true>;

	template class TMaterialParamLoadStoreTexture<false>;
	template class TMaterialParamLoadStoreTexture<true>;

	template class TMaterialParamBuffer<false>;
	template class TMaterialParamBuffer<true>;

	template class TMaterialParamSampState<false>;
	template class TMaterialParamSampState<true>;
}
