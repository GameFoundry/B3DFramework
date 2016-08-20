//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsMaterialParam.h"
#include "BsVector2I.h"
#include "BsVectorNI.h"
#include "BsMaterialParams.h"

namespace BansheeEngine
{
	template<class T, bool Core>
	TMaterialDataParam<T, Core>::TMaterialDataParam(const String& name, const SPtr<MaterialParamsType>& params)
		:mParamIndex(0), mArraySize(0), mMaterialParams(nullptr)
	{
		if(params != nullptr)
		{
			const MaterialParams::ParamData* data = nullptr;
			auto result = params->getParamData(name, MaterialParams::ParamType::Data, 
				(GpuParamDataType)TGpuDataParamInfo<T>::TypeId, 0, &data);

			if (result == MaterialParams::GetParamResult::Success)
			{
				mMaterialParams = params;
				mParamIndex = data->index;
				mArraySize = data->arraySize;
			}
			else
				params->reportGetParamError(result, name, 0);
		}
	}

	template<class T, bool Core>
	void TMaterialDataParam<T, Core>::set(const T& value, UINT32 arrayIdx) const
	{
		if (mMaterialParams == nullptr)
			return;

		if(arrayIdx >= mArraySize)
		{
			LOGWRN("Array index out of range. Provided index was " + toString(arrayIdx) + 
				" but array length is " + toString(mArraySize));
			return;
		}

		mMaterialParams->setDataParam(mParamIndex, arrayIdx, value);
	}

	template<class T, bool Core>
	T TMaterialDataParam<T, Core>::get(UINT32 arrayIdx) const
	{
		T output = T();
		if (mMaterialParams == nullptr || arrayIdx >= mArraySize)
			return output;

		mMaterialParams->getDataParam(mParamIndex, arrayIdx, output);
		return output;
	}

	template<bool Core>
	TMaterialParamStruct<Core>::TMaterialParamStruct(const String& name, const SPtr<MaterialParamsType>& params)
		:mParamIndex(0), mArraySize(0), mMaterialParams(nullptr)
	{
		if (params != nullptr)
		{
			const MaterialParams::ParamData* data = nullptr;
			auto result = params->getParamData(name, MaterialParams::ParamType::Data, GPDT_STRUCT, 0, &data);

			if (result == MaterialParams::GetParamResult::Success)
			{
				mMaterialParams = params;
				mParamIndex = data->index;
				mArraySize = data->arraySize;
			}
			else
				params->reportGetParamError(result, name, 0);
		}
	}

	template<bool Core>
	void TMaterialParamStruct<Core>::set(const void* value, UINT32 sizeBytes, UINT32 arrayIdx) const
	{
		if (mMaterialParams == nullptr)
			return;

		if (arrayIdx >= mArraySize)
		{
			LOGWRN("Array index out of range. Provided index was " + toString(arrayIdx) +
				" but array length is " + toString(mArraySize));
			return;
		}

		mMaterialParams->setStructData(mParamIndex + arrayIdx, value, sizeBytes);
	}

	template<bool Core>
	void TMaterialParamStruct<Core>::get(void* value, UINT32 sizeBytes, UINT32 arrayIdx) const
	{
		if (mMaterialParams == nullptr || arrayIdx >= mArraySize)
			return;

		mMaterialParams->getStructData(mParamIndex + arrayIdx, value, sizeBytes);
	}

	template<bool Core>
	UINT32 TMaterialParamStruct<Core>::getElementSize() const
	{
		return mMaterialParams->getStructSize(mParamIndex);
	}

	template<bool Core>
	TMaterialParamTexture<Core>::TMaterialParamTexture(const String& name, const SPtr<MaterialParamsType>& params)
		:mParamIndex(0), mMaterialParams(nullptr)
	{
		if (params != nullptr)
		{
			const MaterialParams::ParamData* data = nullptr;
			auto result = params->getParamData(name, MaterialParams::ParamType::Texture, GPDT_UNKNOWN, 0, &data);

			if (result == MaterialParams::GetParamResult::Success)
			{
				mMaterialParams = params;
				mParamIndex = data->index;
			}
			else
				params->reportGetParamError(result, name, 0);
		}
	}

	template<bool Core>
	void TMaterialParamTexture<Core>::set(const TextureType& texture) const
	{
		if (mMaterialParams == nullptr)
			return;

		// If there is a default value, assign that instead of null
		TextureType newValue = texture;
		if (newValue == nullptr)
			mMaterialParams->getDefaultTexture(mParamIndex, newValue);

		mMaterialParams->setTexture(mParamIndex, newValue);
	}

	template<bool Core>
	typename TMaterialParamTexture<Core>::TextureType TMaterialParamTexture<Core>::get() const
	{
		TextureType texture;
		if (mMaterialParams == nullptr)
			return texture;

		mMaterialParams->getTexture(mParamIndex, texture);

		return texture;
	}
	
	template<bool Core>
	TMaterialParamLoadStoreTexture<Core>::TMaterialParamLoadStoreTexture(const String& name, 
		const SPtr<MaterialParamsType>& params)
		:mParamIndex(0), mMaterialParams(nullptr)
	{
		if (params != nullptr)
		{
			const MaterialParams::ParamData* data = nullptr;
			auto result = params->getParamData(name, MaterialParams::ParamType::Texture, GPDT_UNKNOWN, 0, &data);

			if (result == MaterialParams::GetParamResult::Success)
			{
				mMaterialParams = params;
				mParamIndex = data->index;
			}
			else
				params->reportGetParamError(result, name, 0);
		}
	}

	template<bool Core>
	void TMaterialParamLoadStoreTexture<Core>::set(const TextureType& texture, const TextureSurface& surface) const
	{
		if (mMaterialParams == nullptr)
			return;

		mMaterialParams->setLoadStoreTexture(mParamIndex, texture, surface);
	}

	template<bool Core>
	typename TMaterialParamLoadStoreTexture<Core>::TextureType TMaterialParamLoadStoreTexture<Core>::get() const
	{
		TextureType texture;
		if (mMaterialParams == nullptr)
			return texture;

		TextureSurface surface;
		mMaterialParams->getLoadStoreTexture(mParamIndex, texture, surface);

		return texture;
	}
	
	template<bool Core>
	TMaterialParamBuffer<Core>::TMaterialParamBuffer(const String& name, const SPtr<MaterialParamsType>& params)
		:mParamIndex(0), mMaterialParams(nullptr)
	{
		if (params != nullptr)
		{
			const MaterialParams::ParamData* data = nullptr;
			auto result = params->getParamData(name, MaterialParams::ParamType::Buffer, GPDT_UNKNOWN, 0, &data);

			if (result == MaterialParams::GetParamResult::Success)
			{
				mMaterialParams = params;
				mParamIndex = data->index;
			}
			else
				params->reportGetParamError(result, name, 0);
		}
	}

	template<bool Core>
	void TMaterialParamBuffer<Core>::set(const BufferType& buffer) const
	{
		if (mMaterialParams == nullptr)
			return;

		mMaterialParams->setBuffer(mParamIndex, buffer);
	}

	template<bool Core>
	typename TMaterialParamBuffer<Core>::BufferType TMaterialParamBuffer<Core>::get() const
	{
		BufferType buffer;
		if (mMaterialParams == nullptr)
			return buffer;

		mMaterialParams->getBuffer(mParamIndex, buffer);

		return buffer;
	}

	template<bool Core>
	TMaterialParamSampState<Core>::TMaterialParamSampState(const String& name, const SPtr<MaterialParamsType>& params)
		:mParamIndex(0), mMaterialParams(nullptr)
	{
		if (params != nullptr)
		{
			const MaterialParams::ParamData* data = nullptr;
			auto result = params->getParamData(name, MaterialParams::ParamType::Sampler, GPDT_UNKNOWN, 0, &data);

			if (result == MaterialParams::GetParamResult::Success)
			{
				mMaterialParams = params;
				mParamIndex = data->index;
			}
			else
				params->reportGetParamError(result, name, 0);
		}
	}

	template<bool Core>
	void TMaterialParamSampState<Core>::set(const SamplerStateType& sampState) const
	{
		if (mMaterialParams == nullptr)
			return;

		// If there is a default value, assign that instead of null
		SamplerStateType newValue = sampState;
		if (newValue == nullptr)
			mMaterialParams->getDefaultSamplerState(mParamIndex, newValue);

		mMaterialParams->setSamplerState(mParamIndex, newValue);
	}

	template<bool Core>
	typename TMaterialParamSampState<Core>::SamplerStateType TMaterialParamSampState<Core>::get() const
	{
		SamplerStateType samplerState;
		if (mMaterialParams == nullptr)
			return samplerState;

		mMaterialParams->getSamplerState(mParamIndex, samplerState);
		return samplerState;
	}

	template class TMaterialDataParam<float, false>;
	template class TMaterialDataParam<int, false>;
	template class TMaterialDataParam<Color, false>;
	template class TMaterialDataParam<Vector2, false>;
	template class TMaterialDataParam<Vector3, false>;
	template class TMaterialDataParam<Vector4, false>;
	template class TMaterialDataParam<Vector2I, false>;
	template class TMaterialDataParam<Vector3I, false>;
	template class TMaterialDataParam<Vector4I, false>;
	template class TMaterialDataParam<Matrix2, false>;
	template class TMaterialDataParam<Matrix2x3, false>;
	template class TMaterialDataParam<Matrix2x4, false>;
	template class TMaterialDataParam<Matrix3, false>;
	template class TMaterialDataParam<Matrix3x2, false>;
	template class TMaterialDataParam<Matrix3x4, false>;
	template class TMaterialDataParam<Matrix4, false>;
	template class TMaterialDataParam<Matrix4x2, false>;
	template class TMaterialDataParam<Matrix4x3, false>;

	template class TMaterialDataParam<float, true>;
	template class TMaterialDataParam<int, true>;
	template class TMaterialDataParam<Color, true>;
	template class TMaterialDataParam<Vector2, true>;
	template class TMaterialDataParam<Vector3, true>;
	template class TMaterialDataParam<Vector4, true>;
	template class TMaterialDataParam<Vector2I, true>;
	template class TMaterialDataParam<Vector3I, true>;
	template class TMaterialDataParam<Vector4I, true>;
	template class TMaterialDataParam<Matrix2, true>;
	template class TMaterialDataParam<Matrix2x3, true>;
	template class TMaterialDataParam<Matrix2x4, true>;
	template class TMaterialDataParam<Matrix3, true>;
	template class TMaterialDataParam<Matrix3x2, true>;
	template class TMaterialDataParam<Matrix3x4, true>;
	template class TMaterialDataParam<Matrix4, true>;
	template class TMaterialDataParam<Matrix4x2, true>;
	template class TMaterialDataParam<Matrix4x3, true>;

	template class TMaterialParamStruct<false>;
	template class TMaterialParamStruct<true>;

	template class TMaterialParamTexture<false>;
	template class TMaterialParamTexture<true>;

	template class TMaterialParamLoadStoreTexture<false>;
	template class TMaterialParamLoadStoreTexture<true>;

	template class TMaterialParamBuffer<false>;
	template class TMaterialParamBuffer<true>;

	template class TMaterialParamSampState<false>;
	template class TMaterialParamSampState<true>;
}