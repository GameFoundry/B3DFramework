//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuParam.h"

#include "BsCoreApplication.h"
#include "BsGpuDevice.h"
#include "BsGpuDeviceCapabilities.h"
#include "RenderAPI/BsGpuParameters.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "RenderAPI/BsGpuProgramParameterDescription.h"
#include "Debug/BsDebug.h"
#include "Error/BsException.h"
#include "Math/BsVector2I.h"

using namespace bs;

template <class T, bool Core>
TGpuParameterPrimitive<T, Core>::TGpuParameterPrimitive()
{}

template <class T, bool Core>
TGpuParameterPrimitive<T, Core>::TGpuParameterPrimitive(const GpuDataParameterInformation* parameterInformation, const GpuParamsType& parent)
	: mParent(parent), mParameterInformation(parameterInformation)
{}

template <class T, bool Core>
void TGpuParameterPrimitive<T, Core>::Set(const T& value, u32 arrayIdx) const
{
	if(mParent == nullptr)
		return;

	GpuParamBufferType paramBlock = mParent->GetUniformBuffer(mParameterInformation->ParentUniformBufferSet, mParameterInformation->ParentUniformBufferSlot);
	if(paramBlock == nullptr)
		return;

#if B3D_DEBUG
	if(arrayIdx >= mParameterInformation->ArraySize)
	{
		B3D_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " + ToString(mParameterInformation->ArraySize) + ". Requested size: " + ToString(arrayIdx));
	}
#endif

	const GpuDataParameterTypeInformation& typeInformation = bs::GpuParameters::kParamSizes.Lookup[mParameterInformation->Type];

	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
	const GpuBackendConventions& gpuBackendConventions = gpuDevice->GetCapabilities().Conventions;

	const bool transposeMatrices = gpuBackendConventions.MatrixOrder == GpuBackendConventions::MatrixOrder::ColumnMajor;
	if(TransposePolicy<T>::TransposeEnabled(transposeMatrices))
	{
		const auto transposed = TransposePolicy<T>::Transpose(value);
		paramBlock->WriteCachedType((mParameterInformation->CpuOffset + arrayIdx * mParameterInformation->ArrayElementStride) * sizeof(u32), typeInformation, &transposed);
	}
	else
		paramBlock->WriteCachedType((mParameterInformation->CpuOffset + arrayIdx * mParameterInformation->ArrayElementStride) * sizeof(u32), typeInformation, &value);

	mParent->MarkCoreDirtyInternal();
}

template <class T, bool Core>
T TGpuParameterPrimitive<T, Core>::Get(u32 arrayIdx) const
{
	if(mParent == nullptr)
		return T();

	GpuParamBufferType paramBlock = mParent->GetUniformBuffer(mParameterInformation->ParentUniformBufferSet, mParameterInformation->ParentUniformBufferSlot);
	if(paramBlock == nullptr)
		return T();

#if B3D_DEBUG
	if(arrayIdx >= mParameterInformation->ArraySize)
	{
		B3D_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " + ToString(mParameterInformation->ArraySize) + ". Requested size: " + ToString(arrayIdx));
	}
#endif

	u32 elementSizeBytes = mParameterInformation->ElementSize * sizeof(u32);
	u32 sizeBytes = std::min(elementSizeBytes, (u32)sizeof(T));

	T value;
	paramBlock->ReadCached((mParameterInformation->CpuOffset + arrayIdx * mParameterInformation->ArrayElementStride) * sizeof(u32), sizeBytes, &value);

	return value;
}

template <bool Core>
TGpuParameterStruct<Core>::TGpuParameterStruct()
{}

template <bool Core>
TGpuParameterStruct<Core>::TGpuParameterStruct(const GpuDataParameterInformation* parameterInformation, const GpuParamsType& parent)
	: mParent(parent), mParameterInformation(parameterInformation)
{}

template <bool Core>
void TGpuParameterStruct<Core>::Set(const void* value, u32 sizeBytes, u32 arrayIdx) const
{
	if(mParent == nullptr)
		return;

	GpuParamBufferType paramBlock = mParent->GetUniformBuffer(mParameterInformation->ParentUniformBufferSet, mParameterInformation->ParentUniformBufferSlot);
	if(paramBlock == nullptr)
		return;

	u32 elementSizeBytes = mParameterInformation->ElementSize * sizeof(u32);

#if B3D_DEBUG
	if(sizeBytes > elementSizeBytes)
	{
		B3D_LOG(Warning, RenderBackend, "Provided element size larger than maximum element size. Maximum size: {0}."
									   " Supplied size: {1}",
			   elementSizeBytes, sizeBytes);
	}

	if(arrayIdx >= mParameterInformation->ArraySize)
	{
		B3D_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " + ToString(mParameterInformation->ArraySize) + ". Requested size: " + ToString(arrayIdx));
	}
#endif

	sizeBytes = std::min(elementSizeBytes, sizeBytes);

	paramBlock->WriteCached((mParameterInformation->CpuOffset + arrayIdx * mParameterInformation->ArrayElementStride) * sizeof(u32), sizeBytes, value);

	// Set unused bytes to 0
	if(sizeBytes < elementSizeBytes)
	{
		u32 diffSize = elementSizeBytes - sizeBytes;
		paramBlock->ZeroOutCached((mParameterInformation->CpuOffset + arrayIdx * mParameterInformation->ArrayElementStride) * sizeof(u32) + sizeBytes, diffSize);
	}

	mParent->MarkCoreDirtyInternal();
}

template <bool Core>
void TGpuParameterStruct<Core>::Get(void* value, u32 sizeBytes, u32 arrayIdx) const
{
	if(mParent == nullptr)
		return;

	GpuParamBufferType paramBlock = mParent->GetUniformBuffer(mParameterInformation->ParentUniformBufferSet, mParameterInformation->ParentUniformBufferSlot);
	if(paramBlock == nullptr)
		return;

	u32 elementSizeBytes = mParameterInformation->ElementSize * sizeof(u32);

#if B3D_DEBUG
	if(sizeBytes > elementSizeBytes)
	{
		B3D_LOG(Warning, RenderBackend, "Provided element size larger than maximum element size. Maximum size: {0}."
									   " Supplied size: {1}",
			   elementSizeBytes, sizeBytes);
	}

	if(arrayIdx >= mParameterInformation->ArraySize)
	{
		B3D_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " + ToString(mParameterInformation->ArraySize) + ". Requested size: " + ToString(arrayIdx));
	}
#endif
	sizeBytes = std::min(elementSizeBytes, sizeBytes);

	paramBlock->ReadCached((mParameterInformation->CpuOffset + arrayIdx * mParameterInformation->ArrayElementStride) * sizeof(u32), sizeBytes, value);
}

template <bool Core>
u32 TGpuParameterStruct<Core>::GetElementSize() const
{
	if(mParent == nullptr)
		return 0;

	return mParameterInformation->ElementSize * sizeof(u32);
}

template <bool Core>
TGpuParameterSampledTexture<Core>::TGpuParameterSampledTexture()
{}

template <bool Core>
TGpuParameterSampledTexture<Core>::TGpuParameterSampledTexture(const GpuParameterBinding& binding, const GpuParamsType& parent)
	: mParent(parent), mBinding(binding)
{}

template <bool Core>
void TGpuParameterSampledTexture<Core>::Set(const TextureType& texture, const TextureSurface& surface, u32 arrayIndex) const
{
	if(mParent == nullptr)
		return;

	mParent->SetSampledTexture(mBinding.Set, mBinding.Slot, texture, surface, arrayIndex);

	mParent->MarkResourcesDirtyInternal();
	mParent->MarkCoreDirtyInternal();
}

template <bool Core>
typename TGpuParameterSampledTexture<Core>::TextureType TGpuParameterSampledTexture<Core>::Get(u32 arrayIndex) const
{
	if(mParent == nullptr)
		return TextureType();

	return mParent->GetSampledTexture(mBinding.Set, mBinding.Slot, arrayIndex);
}

template <bool Core>
TGpuParameterBuffer<Core>::TGpuParameterBuffer()
{}

template <bool Core>
TGpuParameterBuffer<Core>::TGpuParameterBuffer(const GpuParameterBinding& binding, const GpuParamsType& parent)
	: mParent(parent), mBinding(binding)
{}

template <bool Core>
void TGpuParameterBuffer<Core>::Set(const BufferType& buffer, u32 arrayIndex, GpuBufferViewInformation view) const
{
	if(mParent == nullptr)
		return;

	mParent->SetStorageBuffer(mBinding.Set, mBinding.Slot, buffer, arrayIndex, view);

	mParent->MarkResourcesDirtyInternal();
	mParent->MarkCoreDirtyInternal();
}

template <bool Core>
typename TGpuParameterBuffer<Core>::BufferType TGpuParameterBuffer<Core>::Get(u32 arrayIndex) const
{
	if(mParent == nullptr)
		return BufferType();

	return mParent->GetStorageBuffer(mBinding.Set, mBinding.Slot, arrayIndex);
}

template <bool Core>
TGpuParameterStorageTexture<Core>::TGpuParameterStorageTexture()
{}

template <bool Core>
TGpuParameterStorageTexture<Core>::TGpuParameterStorageTexture(const GpuParameterBinding& binding, const GpuParamsType& parent)
	: mParent(parent), mBinding(binding)
{}

template <bool Core>
void TGpuParameterStorageTexture<Core>::Set(const TextureType& texture, const TextureSurface& surface, u32 arrayIndex) const
{
	if(mParent == nullptr)
		return;

	mParent->SetStorageTexture(mBinding.Set, mBinding.Slot, texture, surface, arrayIndex);

	mParent->MarkResourcesDirtyInternal();
	mParent->MarkCoreDirtyInternal();
}

template <bool Core>
typename TGpuParameterStorageTexture<Core>::TextureType TGpuParameterStorageTexture<Core>::Get(u32 arrayIndex) const
{
	if(mParent == nullptr)
		return TextureType();

	return mParent->GetSampledTexture(mBinding.Set, mBinding.Slot, arrayIndex);
}

template <bool Core>
TGpuParameterSampler<Core>::TGpuParameterSampler()
{}

template <bool Core>
TGpuParameterSampler<Core>::TGpuParameterSampler(const GpuParameterBinding& binding, const GpuParamsType& parent)
	: mParent(parent), mBinding(binding)
{}

template <bool Core>
void TGpuParameterSampler<Core>::Set(const SPtr<SamplerState>& samplerState, u32 arrayIndex) const
{
	if(mParent == nullptr)
		return;

	mParent->SetSamplerState(mBinding.Set, mBinding.Slot, samplerState, arrayIndex);

	mParent->MarkResourcesDirtyInternal();
	mParent->MarkCoreDirtyInternal();
}

template <bool Core>
SPtr<SamplerState> TGpuParameterSampler<Core>::Get(u32 arrayIndex) const
{
	if (mParent == nullptr)
		return nullptr;

	return mParent->GetSamplerState(mBinding.Set, mBinding.Slot, arrayIndex);
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
