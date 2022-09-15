//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuParam.h"
#include "RenderAPI/BsGpuParams.h"
#include "RenderAPI/BsGpuParamBlockBuffer.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Debug/BsDebug.h"
#include "Error/BsException.h"
#include "Math/BsVector2I.h"

namespace bs
{
	template<class T, bool Core>
	TGpuDataParam<T, Core>::TGpuDataParam()
		:mParamDesc(nullptr)
	{ }

	template<class T, bool Core>
	TGpuDataParam<T, Core>::TGpuDataParam(GpuParamDataDesc* paramDesc, const GpuParamsType& parent)
		:mParent(parent), mParamDesc(paramDesc)
	{ }

	template<class T, bool Core>
	void TGpuDataParam<T, Core>::Set(const T& value, UINT32 arrayIdx) const
	{
		if (mParent == nullptr)
			return;

		GpuParamBufferType paramBlock = mParent->GetParamBlockBuffer(mParamDesc->paramBlockSet, mParamDesc->paramBlockSlot);
		if (paramBlock == nullptr)
			return;

#if BS_DEBUG_MODE
		if (arrayIdx >= mParamDesc->arraySize)
		{
			BS_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " +
				toString(mParamDesc->arraySize) + ". Requested size: " + toString(arrayIdx));
		}
#endif

		UINT32 elementSizeBytes = mParamDesc->elementSize * sizeof(UINT32);
		UINT32 sizeBytes = std::min(elementSizeBytes, (UINT32)sizeof(T)); // Truncate if it doesn't fit within parameter size

		const bool transposeMatrices = ct::gCaps().conventions.matrixOrder == Conventions::MatrixOrder::ColumnMajor;
		if (TransposePolicy<T>::TransposeEnabled(transposeMatrices))
		{
			auto transposed = TransposePolicy<T>::Transpose(value);
			paramBlock->Write((mParamDesc->cpuMemOffset + arrayIdx * mParamDesc->arrayElementStride) * sizeof(UINT32), &transposed, sizeBytes);
		}
		else
			paramBlock->Write((mParamDesc->cpuMemOffset + arrayIdx * mParamDesc->arrayElementStride) * sizeof(UINT32), &value, sizeBytes);

		// Set unused bytes to 0
		if (sizeBytes < elementSizeBytes)
		{
			UINT32 diffSize = elementSizeBytes - sizeBytes;
			paramBlock->ZeroOut((mParamDesc->cpuMemOffset + arrayIdx * mParamDesc->arrayElementStride)  * sizeof(UINT32) + sizeBytes, diffSize);
		}

		mParent->MarkCoreDirtyInternal();
	}

	template<class T, bool Core>
	T TGpuDataParam<T, Core>::Get(UINT32 arrayIdx) const
	{
		if (mParent == nullptr)
			return T();

		GpuParamBufferType paramBlock = mParent->GetParamBlockBuffer(mParamDesc->paramBlockSet, mParamDesc->paramBlockSlot);
		if (paramBlock == nullptr)
			return T();

#if BS_DEBUG_MODE
		if (arrayIdx >= mParamDesc->arraySize)
		{
			BS_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " +
				toString(mParamDesc->arraySize) + ". Requested size: " + toString(arrayIdx));
		}
#endif

		UINT32 elementSizeBytes = mParamDesc->elementSize * sizeof(UINT32);
		UINT32 sizeBytes = std::min(elementSizeBytes, (UINT32)sizeof(T));

		T value;
		paramBlock->Read((mParamDesc->cpuMemOffset + arrayIdx * mParamDesc->arrayElementStride) * sizeof(UINT32), &value, sizeBytes);

		return value;
	}

	template<bool Core>
	TGpuParamStruct<Core>::TGpuParamStruct()
		:mParamDesc(nullptr)
	{ }

	template<bool Core>
	TGpuParamStruct<Core>::TGpuParamStruct(GpuParamDataDesc* paramDesc, const GpuParamsType& parent)
		:mParent(parent), mParamDesc(paramDesc)
	{ }

	template<bool Core>
	void TGpuParamStruct<Core>::Set(const void* value, UINT32 sizeBytes, UINT32 arrayIdx) const
	{
		if (mParent == nullptr)
			return;

		GpuParamBufferType paramBlock = mParent->GetParamBlockBuffer(mParamDesc->paramBlockSet, mParamDesc->paramBlockSlot);
		if (paramBlock == nullptr)
			return;

		UINT32 elementSizeBytes = mParamDesc->elementSize * sizeof(UINT32);

#if BS_DEBUG_MODE
		if (sizeBytes > elementSizeBytes)
		{
			BS_LOG(Warning, RenderBackend, "Provided element size larger than maximum element size. Maximum size: {0}."
				" Supplied size: {1}", elementSizeBytes, sizeBytes);
		}

		if (arrayIdx >= mParamDesc->arraySize)
		{
			BS_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " +
				toString(mParamDesc->arraySize) + ". Requested size: " + toString(arrayIdx));
		}
#endif

		sizeBytes = std::min(elementSizeBytes, sizeBytes);

		paramBlock->Write((mParamDesc->cpuMemOffset + arrayIdx * mParamDesc->arrayElementStride) * sizeof(UINT32), value, sizeBytes);

		// Set unused bytes to 0
		if (sizeBytes < elementSizeBytes)
		{
			UINT32 diffSize = elementSizeBytes - sizeBytes;
			paramBlock->ZeroOut((mParamDesc->cpuMemOffset + arrayIdx * mParamDesc->arrayElementStride)  * sizeof(UINT32) + sizeBytes, diffSize);
		}

		mParent->MarkCoreDirtyInternal();
	}

	template<bool Core>
	void TGpuParamStruct<Core>::Get(void* value, UINT32 sizeBytes, UINT32 arrayIdx) const
	{
		if (mParent == nullptr)
			return;

		GpuParamBufferType paramBlock = mParent->GetParamBlockBuffer(mParamDesc->paramBlockSet, mParamDesc->paramBlockSlot);
		if (paramBlock == nullptr)
			return;

		UINT32 elementSizeBytes = mParamDesc->elementSize * sizeof(UINT32);

#if BS_DEBUG_MODE
		if (sizeBytes > elementSizeBytes)
		{
			BS_LOG(Warning, RenderBackend, "Provided element size larger than maximum element size. Maximum size: {0}."
				" Supplied size: {1}", elementSizeBytes, sizeBytes);
		}

		if (arrayIdx >= mParamDesc->arraySize)
		{
			BS_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " +
				toString(mParamDesc->arraySize) + ". Requested size: " + toString(arrayIdx));
		}
#endif
		sizeBytes = std::min(elementSizeBytes, sizeBytes);

		paramBlock->Read((mParamDesc->cpuMemOffset + arrayIdx * mParamDesc->arrayElementStride) * sizeof(UINT32), value, sizeBytes);
	}

	template<bool Core>
	UINT32 TGpuParamStruct<Core>::GetElementSize() const
	{
		if (mParent == nullptr)
			return 0;

		return mParamDesc->elementSize * sizeof(UINT32);
	}

	template<bool Core>
	TGpuParamTexture<Core>::TGpuParamTexture()
		:mParamDesc(nullptr)
	{ }

	template<bool Core>
	TGpuParamTexture<Core>::TGpuParamTexture(GpuParamObjectDesc* paramDesc, const GpuParamsType& parent)
		:mParent(parent), mParamDesc(paramDesc)
	{ }

	template<bool Core>
	void TGpuParamTexture<Core>::Set(const TextureType& texture, const TextureSurface& surface) const
	{
		if (mParent == nullptr)
			return;

		mParent->SetTexture(mParamDesc->set, mParamDesc->slot, texture, surface);

		mParent->MarkResourcesDirtyInternal();
		mParent->MarkCoreDirtyInternal();
	}

	template<bool Core>
	typename TGpuParamTexture<Core>::TextureType TGpuParamTexture<Core>::Get() const
	{
		if (mParent == nullptr)
			return TextureType();

		return mParent->GetTexture(mParamDesc->set, mParamDesc->slot);
	}

	template<bool Core>
	TGpuParamBuffer<Core>::TGpuParamBuffer()
		:mParamDesc(nullptr)
	{ }

	template<bool Core>
	TGpuParamBuffer<Core>::TGpuParamBuffer(GpuParamObjectDesc* paramDesc, const GpuParamsType& parent)
		: mParent(parent), mParamDesc(paramDesc)
	{ }

	template<bool Core>
	void TGpuParamBuffer<Core>::Set(const BufferType& buffer) const
	{
		if (mParent == nullptr)
			return;

		mParent->SetBuffer(mParamDesc->set, mParamDesc->slot, buffer);

		mParent->MarkResourcesDirtyInternal();
		mParent->MarkCoreDirtyInternal();
	}

	template<bool Core>
	typename TGpuParamBuffer<Core>::BufferType TGpuParamBuffer<Core>::Get() const
	{
		if (mParent == nullptr)
			return BufferType();

		return mParent->GetBuffer(mParamDesc->set, mParamDesc->slot);
	}

	template<bool Core>
	TGpuParamLoadStoreTexture<Core>::TGpuParamLoadStoreTexture()
		:mParamDesc(nullptr)
	{ }

	template<bool Core>
	TGpuParamLoadStoreTexture<Core>::TGpuParamLoadStoreTexture(GpuParamObjectDesc* paramDesc, const GpuParamsType& parent)
		:mParent(parent), mParamDesc(paramDesc)
	{ }

	template<bool Core>
	void TGpuParamLoadStoreTexture<Core>::Set(const TextureType& texture, const TextureSurface& surface) const
	{
		if (mParent == nullptr)
			return;

		mParent->SetLoadStoreTexture(mParamDesc->set, mParamDesc->slot, texture, surface);

		mParent->MarkResourcesDirtyInternal();
		mParent->MarkCoreDirtyInternal();
	}

	template<bool Core>
	typename TGpuParamLoadStoreTexture<Core>::TextureType TGpuParamLoadStoreTexture<Core>::Get() const
	{
		if (mParent == nullptr)
			return TextureType();

		return mParent->GetTexture(mParamDesc->set, mParamDesc->slot);
	}

	template<bool Core>
	TGpuParamSampState<Core>::TGpuParamSampState()
		:mParamDesc(nullptr)
	{ }

	template<bool Core>
	TGpuParamSampState<Core>::TGpuParamSampState(GpuParamObjectDesc* paramDesc, const GpuParamsType& parent)
		:mParent(parent), mParamDesc(paramDesc)
	{ }

	template<bool Core>
	void TGpuParamSampState<Core>::Set(const SamplerStateType& samplerState) const
	{
		if (mParent == nullptr)
			return;

		mParent->SetSamplerState(mParamDesc->set, mParamDesc->slot, samplerState);

		mParent->MarkResourcesDirtyInternal();
		mParent->MarkCoreDirtyInternal();
	}

	template<bool Core>
	typename TGpuParamSampState<Core>::SamplerStateType TGpuParamSampState<Core>::Get() const
	{
		if (mParent == nullptr)
			return SamplerStateType();

		return mParent->GetSamplerState(mParamDesc->set, mParamDesc->slot);
	}

	template class TGpuDataParam < float, false > ;
	template class TGpuDataParam < int, false >;
	template class TGpuDataParam < Color, false > ;
	template class TGpuDataParam < Vector2, false > ;
	template class TGpuDataParam < Vector3, false > ;
	template class TGpuDataParam < Vector4, false > ;
	template class TGpuDataParam < Vector2I, false > ;
	template class TGpuDataParam < Vector3I, false > ;
	template class TGpuDataParam < Vector4I, false > ;
	template class TGpuDataParam < Matrix2, false >;
	template class TGpuDataParam < Matrix2x3, false >;
	template class TGpuDataParam < Matrix2x4, false >;
	template class TGpuDataParam < Matrix3, false > ;
	template class TGpuDataParam < Matrix3x2, false > ;
	template class TGpuDataParam < Matrix3x4, false > ;
	template class TGpuDataParam < Matrix4, false > ;
	template class TGpuDataParam < Matrix4x2, false >;
	template class TGpuDataParam < Matrix4x3, false >;

	template class TGpuDataParam < float, true > ;
	template class TGpuDataParam < int, true >;
	template class TGpuDataParam < Color, true > ;
	template class TGpuDataParam < Vector2, true > ;
	template class TGpuDataParam < Vector3, true > ;
	template class TGpuDataParam < Vector4, true > ;
	template class TGpuDataParam < Vector2I, true > ;
	template class TGpuDataParam < Vector3I, true > ;
	template class TGpuDataParam < Vector4I, true > ;
	template class TGpuDataParam < Matrix2, true >;
	template class TGpuDataParam < Matrix2x3, true >;
	template class TGpuDataParam < Matrix2x4, true >;
	template class TGpuDataParam < Matrix3, true > ;
	template class TGpuDataParam < Matrix3x2, true >;
	template class TGpuDataParam < Matrix3x4, true >;
	template class TGpuDataParam < Matrix4, true > ;
	template class TGpuDataParam < Matrix4x2, true >;
	template class TGpuDataParam < Matrix4x3, true >;

	template class TGpuParamStruct < false > ;
	template class TGpuParamStruct < true > ;

	template class TGpuParamTexture < false > ;
	template class TGpuParamTexture < true > ;

	template class TGpuParamBuffer < false >;
	template class TGpuParamBuffer < true >;

	template class TGpuParamSampState < false > ;
	template class TGpuParamSampState < true > ;

	template class TGpuParamLoadStoreTexture < false > ;
	template class TGpuParamLoadStoreTexture < true > ;
}
