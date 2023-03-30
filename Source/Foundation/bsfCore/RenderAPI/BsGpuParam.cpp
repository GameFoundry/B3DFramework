//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuParam.h"
#include "RenderAPI/BsGpuParameters.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "RenderAPI/BsGpuProgramParameterDescription.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Debug/BsDebug.h"
#include "Error/BsException.h"
#include "Math/BsVector2I.h"

using namespace bs;

template <class T, bool Core>
TGpuParameterPrimitive<T, Core>::TGpuParameterPrimitive()
	: mParameterDescription(nullptr)
{}

template <class T, bool Core>
TGpuParameterPrimitive<T, Core>::TGpuParameterPrimitive(GpuDataParameterInformation* paramDesc, const GpuParamsType& parent)
	: mParent(parent), mParameterDescription(paramDesc)
{}

template <class T, bool Core>
void TGpuParameterPrimitive<T, Core>::Set(const T& value, u32 arrayIdx) const
{
	if(mParent == nullptr)
		return;

	GpuParamBufferType paramBlock = mParent->GetUniformBuffer(mParameterDescription->ParamBlockSet, mParameterDescription->ParamBlockSlot);
	if(paramBlock == nullptr)
		return;

#if B3D_DEBUG
	if(arrayIdx >= mParameterDescription->ArraySize)
	{
		B3D_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " + ToString(mParameterDescription->ArraySize) + ". Requested size: " + ToString(arrayIdx));
	}
#endif

	u32 elementSizeBytes = mParameterDescription->ElementSize * sizeof(u32);
	u32 sizeBytes = std::min(elementSizeBytes, (u32)sizeof(T)); // Truncate if it doesn't fit within parameter size

	const bool transposeMatrices = ct::GetGpuDeviceCapabilities().Conventions.MatrixOrder == GpuBackendConventions::MatrixOrder::ColumnMajor;
	if(TransposePolicy<T>::TransposeEnabled(transposeMatrices))
	{
		auto transposed = TransposePolicy<T>::Transpose(value);
		paramBlock->WriteCached((mParameterDescription->CpuOffset + arrayIdx * mParameterDescription->ArrayElementStride) * sizeof(u32), sizeBytes, &transposed);
	}
	else
		paramBlock->WriteCached((mParameterDescription->CpuOffset + arrayIdx * mParameterDescription->ArrayElementStride) * sizeof(u32), sizeBytes, &value);

	// Set unused bytes to 0
	if(sizeBytes < elementSizeBytes)
	{
		u32 diffSize = elementSizeBytes - sizeBytes;
		paramBlock->ZeroOutCached((mParameterDescription->CpuOffset + arrayIdx * mParameterDescription->ArrayElementStride) * sizeof(u32) + sizeBytes, diffSize);
	}

	mParent->MarkCoreDirtyInternal();
}

template <class T, bool Core>
T TGpuParameterPrimitive<T, Core>::Get(u32 arrayIdx) const
{
	if(mParent == nullptr)
		return T();

	GpuParamBufferType paramBlock = mParent->GetUniformBuffer(mParameterDescription->ParamBlockSet, mParameterDescription->ParamBlockSlot);
	if(paramBlock == nullptr)
		return T();

#if B3D_DEBUG
	if(arrayIdx >= mParameterDescription->ArraySize)
	{
		B3D_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " + ToString(mParameterDescription->ArraySize) + ". Requested size: " + ToString(arrayIdx));
	}
#endif

	u32 elementSizeBytes = mParameterDescription->ElementSize * sizeof(u32);
	u32 sizeBytes = std::min(elementSizeBytes, (u32)sizeof(T));

	T value;
	paramBlock->ReadCached((mParameterDescription->CpuOffset + arrayIdx * mParameterDescription->ArrayElementStride) * sizeof(u32), sizeBytes, &value);

	return value;
}

template <bool Core>
TGpuParameterStruct<Core>::TGpuParameterStruct()
	: mParameterDescription(nullptr)
{}

template <bool Core>
TGpuParameterStruct<Core>::TGpuParameterStruct(GpuDataParameterInformation* paramDesc, const GpuParamsType& parent)
	: mParent(parent), mParameterDescription(paramDesc)
{}

template <bool Core>
void TGpuParameterStruct<Core>::Set(const void* value, u32 sizeBytes, u32 arrayIdx) const
{
	if(mParent == nullptr)
		return;

	GpuParamBufferType paramBlock = mParent->GetUniformBuffer(mParameterDescription->ParamBlockSet, mParameterDescription->ParamBlockSlot);
	if(paramBlock == nullptr)
		return;

	u32 elementSizeBytes = mParameterDescription->ElementSize * sizeof(u32);

#if B3D_DEBUG
	if(sizeBytes > elementSizeBytes)
	{
		B3D_LOG(Warning, RenderBackend, "Provided element size larger than maximum element size. Maximum size: {0}."
									   " Supplied size: {1}",
			   elementSizeBytes, sizeBytes);
	}

	if(arrayIdx >= mParameterDescription->ArraySize)
	{
		B3D_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " + ToString(mParameterDescription->ArraySize) + ". Requested size: " + ToString(arrayIdx));
	}
#endif

	sizeBytes = std::min(elementSizeBytes, sizeBytes);

	paramBlock->WriteCached((mParameterDescription->CpuOffset + arrayIdx * mParameterDescription->ArrayElementStride) * sizeof(u32), sizeBytes, value);

	// Set unused bytes to 0
	if(sizeBytes < elementSizeBytes)
	{
		u32 diffSize = elementSizeBytes - sizeBytes;
		paramBlock->ZeroOutCached((mParameterDescription->CpuOffset + arrayIdx * mParameterDescription->ArrayElementStride) * sizeof(u32) + sizeBytes, diffSize);
	}

	mParent->MarkCoreDirtyInternal();
}

template <bool Core>
void TGpuParameterStruct<Core>::Get(void* value, u32 sizeBytes, u32 arrayIdx) const
{
	if(mParent == nullptr)
		return;

	GpuParamBufferType paramBlock = mParent->GetUniformBuffer(mParameterDescription->ParamBlockSet, mParameterDescription->ParamBlockSlot);
	if(paramBlock == nullptr)
		return;

	u32 elementSizeBytes = mParameterDescription->ElementSize * sizeof(u32);

#if B3D_DEBUG
	if(sizeBytes > elementSizeBytes)
	{
		B3D_LOG(Warning, RenderBackend, "Provided element size larger than maximum element size. Maximum size: {0}."
									   " Supplied size: {1}",
			   elementSizeBytes, sizeBytes);
	}

	if(arrayIdx >= mParameterDescription->ArraySize)
	{
		B3D_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " + ToString(mParameterDescription->ArraySize) + ". Requested size: " + ToString(arrayIdx));
	}
#endif
	sizeBytes = std::min(elementSizeBytes, sizeBytes);

	paramBlock->ReadCached((mParameterDescription->CpuOffset + arrayIdx * mParameterDescription->ArrayElementStride) * sizeof(u32), sizeBytes, value);
}

template <bool Core>
u32 TGpuParameterStruct<Core>::GetElementSize() const
{
	if(mParent == nullptr)
		return 0;

	return mParameterDescription->ElementSize * sizeof(u32);
}

template <bool Core>
TGpuParameterSampledTexture<Core>::TGpuParameterSampledTexture()
	: mParameterDescription(nullptr)
{}

template <bool Core>
TGpuParameterSampledTexture<Core>::TGpuParameterSampledTexture(GpuObjectParameterInformation* paramDesc, const GpuParamsType& parent)
	: mParent(parent), mParameterDescription(paramDesc)
{}

template <bool Core>
void TGpuParameterSampledTexture<Core>::Set(const TextureType& texture, const TextureSurface& surface, u32 arrayIndex) const
{
	if(mParent == nullptr)
		return;

	mParent->SetSampledTexture(mParameterDescription->Set, mParameterDescription->Slot, texture, surface, arrayIndex);

	mParent->MarkResourcesDirtyInternal();
	mParent->MarkCoreDirtyInternal();
}

template <bool Core>
typename TGpuParameterSampledTexture<Core>::TextureType TGpuParameterSampledTexture<Core>::Get(u32 arrayIndex) const
{
	if(mParent == nullptr)
		return TextureType();

	return mParent->GetSampledTexture(mParameterDescription->Set, mParameterDescription->Slot, arrayIndex);
}

template <bool Core>
TGpuParameterBuffer<Core>::TGpuParameterBuffer()
	: mParameterDescription(nullptr)
{}

template <bool Core>
TGpuParameterBuffer<Core>::TGpuParameterBuffer(GpuObjectParameterInformation* paramDesc, const GpuParamsType& parent)
	: mParent(parent), mParameterDescription(paramDesc)
{}

template <bool Core>
void TGpuParameterBuffer<Core>::Set(const BufferType& buffer, u32 arrayIndex, GpuStorageBufferViewInformation view) const
{
	if(mParent == nullptr)
		return;

	mParent->SetStorageBuffer(mParameterDescription->Set, mParameterDescription->Slot, buffer, arrayIndex, view);

	mParent->MarkResourcesDirtyInternal();
	mParent->MarkCoreDirtyInternal();
}

template <bool Core>
typename TGpuParameterBuffer<Core>::BufferType TGpuParameterBuffer<Core>::Get(u32 arrayIndex) const
{
	if(mParent == nullptr)
		return BufferType();

	return mParent->GetStorageBuffer(mParameterDescription->Set, mParameterDescription->Slot, arrayIndex);
}

template <bool Core>
TGpuParameterStorageTexture<Core>::TGpuParameterStorageTexture()
	: mParameterDescription(nullptr)
{}

template <bool Core>
TGpuParameterStorageTexture<Core>::TGpuParameterStorageTexture(GpuObjectParameterInformation* paramDesc, const GpuParamsType& parent)
	: mParent(parent), mParameterDescription(paramDesc)
{}

template <bool Core>
void TGpuParameterStorageTexture<Core>::Set(const TextureType& texture, const TextureSurface& surface, u32 arrayIndex) const
{
	if(mParent == nullptr)
		return;

	mParent->SetStorageTexture(mParameterDescription->Set, mParameterDescription->Slot, texture, surface, arrayIndex);

	mParent->MarkResourcesDirtyInternal();
	mParent->MarkCoreDirtyInternal();
}

template <bool Core>
typename TGpuParameterStorageTexture<Core>::TextureType TGpuParameterStorageTexture<Core>::Get(u32 arrayIndex) const
{
	if(mParent == nullptr)
		return TextureType();

	return mParent->GetSampledTexture(mParameterDescription->Set, mParameterDescription->Slot, arrayIndex);
}

template <bool Core>
TGpuParameterSampler<Core>::TGpuParameterSampler()
	: mParamDesc(nullptr)
{}

template <bool Core>
TGpuParameterSampler<Core>::TGpuParameterSampler(GpuObjectParameterInformation* paramDesc, const GpuParamsType& parent)
	: mParent(parent), mParamDesc(paramDesc)
{}

template <bool Core>
void TGpuParameterSampler<Core>::Set(const SamplerStateType& samplerState, u32 arrayIndex) const
{
	if(mParent == nullptr)
		return;

	mParent->SetSamplerState(mParamDesc->Set, mParamDesc->Slot, samplerState, arrayIndex);

	mParent->MarkResourcesDirtyInternal();
	mParent->MarkCoreDirtyInternal();
}

template <bool Core>
typename TGpuParameterSampler<Core>::SamplerStateType TGpuParameterSampler<Core>::Get(u32 arrayIndex) const
{
	if(mParent == nullptr)
		return SamplerStateType();

	return mParent->GetSamplerState(mParamDesc->Set, mParamDesc->Slot, arrayIndex);
}

template class TGpuParameterPrimitive<float, false>;
template class TGpuParameterPrimitive<double, false>;
template class TGpuParameterPrimitive<Color, false>;
template class TGpuParameterPrimitive<Vector2, false>;
template class TGpuParameterPrimitive<Vector3, false>;
template class TGpuParameterPrimitive<Vector4, false>;
template class TGpuParameterPrimitive<i32, false>;
template class TGpuParameterPrimitive<Vector2I, false>;
template class TGpuParameterPrimitive<Vector3I, false>;
template class TGpuParameterPrimitive<Vector4I, false>;
template class TGpuParameterPrimitive<u32, false>;
template class TGpuParameterPrimitive<Vector2UI, false>;
template class TGpuParameterPrimitive<Vector3UI, false>;
template class TGpuParameterPrimitive<Vector4UI, false>;
template class TGpuParameterPrimitive<Matrix2, false>;
template class TGpuParameterPrimitive<Matrix2x3, false>;
template class TGpuParameterPrimitive<Matrix2x4, false>;
template class TGpuParameterPrimitive<Matrix3, false>;
template class TGpuParameterPrimitive<Matrix3x2, false>;
template class TGpuParameterPrimitive<Matrix3x4, false>;
template class TGpuParameterPrimitive<Matrix4, false>;
template class TGpuParameterPrimitive<Matrix4x2, false>;
template class TGpuParameterPrimitive<Matrix4x3, false>;

template class TGpuParameterPrimitive<float, true>;
template class TGpuParameterPrimitive<double, true>;
template class TGpuParameterPrimitive<Color, true>;
template class TGpuParameterPrimitive<Vector2, true>;
template class TGpuParameterPrimitive<Vector3, true>;
template class TGpuParameterPrimitive<Vector4, true>;
template class TGpuParameterPrimitive<i32, true>;
template class TGpuParameterPrimitive<Vector2I, true>;
template class TGpuParameterPrimitive<Vector3I, true>;
template class TGpuParameterPrimitive<Vector4I, true>;
template class TGpuParameterPrimitive<u32, true>;
template class TGpuParameterPrimitive<Vector2UI, true>;
template class TGpuParameterPrimitive<Vector3UI, true>;
template class TGpuParameterPrimitive<Vector4UI, true>;
template class TGpuParameterPrimitive<Matrix2, true>;
template class TGpuParameterPrimitive<Matrix2x3, true>;
template class TGpuParameterPrimitive<Matrix2x4, true>;
template class TGpuParameterPrimitive<Matrix3, true>;
template class TGpuParameterPrimitive<Matrix3x2, true>;
template class TGpuParameterPrimitive<Matrix3x4, true>;
template class TGpuParameterPrimitive<Matrix4, true>;
template class TGpuParameterPrimitive<Matrix4x2, true>;
template class TGpuParameterPrimitive<Matrix4x3, true>;

template class TGpuParameterStruct<false>;
template class TGpuParameterStruct<true>;

template class TGpuParameterSampledTexture<false>;
template class TGpuParameterSampledTexture<true>;

template class TGpuParameterBuffer<false>;
template class TGpuParameterBuffer<true>;

template class TGpuParameterSampler<false>;
template class TGpuParameterSampler<true>;

template class TGpuParameterStorageTexture<false>;
template class TGpuParameterStorageTexture<true>;
