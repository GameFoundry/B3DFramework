//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/B3DGpuParameter.h"

#include "B3DApplication.h"
#include "B3DGpuDevice.h"
#include "B3DGpuDeviceCapabilities.h"
#include "RenderAPI/B3DGpuParameterSet.h"
#include "RenderAPI/B3DGpuBuffer.h"
#include "RenderAPI/B3DGpuProgramParameterDescription.h"
#include "Debug/B3DDebug.h"

using namespace b3d;

template <class T, bool IsRenderProxy>
TGpuParameterPrimitive<T, IsRenderProxy>::TGpuParameterPrimitive()
{}

template <class T, bool IsRenderProxy>
TGpuParameterPrimitive<T, IsRenderProxy>::TGpuParameterPrimitive(const GpuUniformBufferMemberInformation* parameterInformation, const GpuParamsType& parent)
	: mParent(parent), mParameterInformation(parameterInformation)
{
	B3D_ASSERT(mParent->GetSet() == mParameterInformation->ParentUniformBufferSet);
}

template <class T, bool IsRenderProxy>
void TGpuParameterPrimitive<T, IsRenderProxy>::Set(const T& value, u32 arrayIdx) const
{
	if(mParent == nullptr)
		return;

	GpuParamBufferType uniformBuffer = mParent->GetUniformBuffer(mParameterInformation->ParentUniformBufferSlot);
	if(uniformBuffer == nullptr)
		return;

	if(!B3D_ENSURE_LOG(arrayIdx < mParameterInformation->ArraySize, "Array index out of range. Array size: {0}. Requested size: {1}", mParameterInformation->ArraySize, arrayIdx))
		return;

	const GpuDataParameterTypeInformation& typeInformation = b3d::GpuParameterSet::kParamSizes.Lookup[mParameterInformation->Type];

	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	const GpuBackendConventions& gpuBackendConventions = gpuDevice->GetCapabilities().Conventions;

	const bool transposeMatrices = gpuBackendConventions.MatrixOrder == GpuBackendConventions::MatrixOrder::ColumnMajor;
	if(TransposePolicy<T>::TransposeEnabled(transposeMatrices))
	{
		const auto transposed = TransposePolicy<T>::Transpose(value);
		uniformBuffer->WriteTyped((mParameterInformation->CpuOffset + arrayIdx * mParameterInformation->ArrayElementStride) * sizeof(u32), typeInformation, &transposed);
	}
	else
		uniformBuffer->WriteTyped((mParameterInformation->CpuOffset + arrayIdx * mParameterInformation->ArrayElementStride) * sizeof(u32), typeInformation, &value);

	mParent->MarkRenderProxyDataDirtyInternal();
}

template <class T, bool IsRenderProxy>
T TGpuParameterPrimitive<T, IsRenderProxy>::Get(u32 arrayIdx) const
{
	if(mParent == nullptr)
		return T();

	GpuParamBufferType uniformBuffer = mParent->GetUniformBuffer(mParameterInformation->ParentUniformBufferSlot);
	if(uniformBuffer == nullptr)
		return T();

	if(!B3D_ENSURE_LOG(arrayIdx < mParameterInformation->ArraySize, "Array index out of range. Array size: {0}. Requested size: {1}", mParameterInformation->ArraySize, arrayIdx))
		return T();

	u32 elementSizeBytes = mParameterInformation->ElementSize * sizeof(u32);
	u32 sizeBytes = std::min(elementSizeBytes, (u32)sizeof(T));

	T value;
	uniformBuffer->Read((mParameterInformation->CpuOffset + arrayIdx * mParameterInformation->ArrayElementStride) * sizeof(u32), sizeBytes, &value);

	return value;
}

template <bool IsRenderProxy>
TGpuParameterStruct<IsRenderProxy>::TGpuParameterStruct()
{}

template <bool IsRenderProxy>
TGpuParameterStruct<IsRenderProxy>::TGpuParameterStruct(const GpuUniformBufferMemberInformation* parameterInformation, const GpuParamsType& parent)
	: mParent(parent), mParameterInformation(parameterInformation)
{
	B3D_ASSERT(mParent->GetSet() == mParameterInformation->ParentUniformBufferSet);
}

template <bool IsRenderProxy>
void TGpuParameterStruct<IsRenderProxy>::Set(const void* value, u32 sizeBytes, u32 arrayIdx) const
{
	if(mParent == nullptr)
		return;

	GpuParamBufferType uniformBuffer = mParent->GetUniformBuffer(mParameterInformation->ParentUniformBufferSlot);
	if(uniformBuffer == nullptr)
		return;

	u32 elementSizeBytes = mParameterInformation->ElementSize * sizeof(u32);

	if(!B3D_ENSURE_LOG(sizeBytes <= elementSizeBytes, "Provided element size larger than maximum element size. Maximum size: {0}. Supplied size: {1}", elementSizeBytes, sizeBytes))
		return;

	if(!B3D_ENSURE_LOG(arrayIdx < mParameterInformation->ArraySize, "Array index out of range. Array size: {0}. Requested size: {1}", mParameterInformation->ArraySize, arrayIdx))
		return;

	sizeBytes = std::min(elementSizeBytes, sizeBytes);

	uniformBuffer->Write((mParameterInformation->CpuOffset + arrayIdx * mParameterInformation->ArrayElementStride) * sizeof(u32), sizeBytes, value);

	// Set unused bytes to 0
	if(sizeBytes < elementSizeBytes)
	{
		u32 diffSize = elementSizeBytes - sizeBytes;
		uniformBuffer->ZeroOut((mParameterInformation->CpuOffset + arrayIdx * mParameterInformation->ArrayElementStride) * sizeof(u32) + sizeBytes, diffSize);
	}

	mParent->MarkRenderProxyDataDirtyInternal();
}

template <bool IsRenderProxy>
void TGpuParameterStruct<IsRenderProxy>::Get(void* value, u32 sizeBytes, u32 arrayIdx) const
{
	if(mParent == nullptr)
		return;

	GpuParamBufferType uniformBuffer = mParent->GetUniformBuffer(mParameterInformation->ParentUniformBufferSlot);
	if(uniformBuffer == nullptr)
		return;

	u32 elementSizeBytes = mParameterInformation->ElementSize * sizeof(u32);

	if(!B3D_ENSURE_LOG(sizeBytes <= elementSizeBytes, "Provided element size larger than maximum element size. Maximum size: {0}. Supplied size: {1}", elementSizeBytes, sizeBytes))
		return;

	if(!B3D_ENSURE_LOG(arrayIdx < mParameterInformation->ArraySize, "Array index out of range. Array size: {0}. Requested size: {1}", mParameterInformation->ArraySize, arrayIdx))
		return;

	sizeBytes = std::min(elementSizeBytes, sizeBytes);

	uniformBuffer->Read((mParameterInformation->CpuOffset + arrayIdx * mParameterInformation->ArrayElementStride) * sizeof(u32), sizeBytes, value);
}

template <bool IsRenderProxy>
u32 TGpuParameterStruct<IsRenderProxy>::GetElementSize() const
{
	if(mParent == nullptr)
		return 0;

	return mParameterInformation->ElementSize * sizeof(u32);
}

template <bool IsRenderProxy>
TGpuParameterSampledTexture<IsRenderProxy>::TGpuParameterSampledTexture()
{ }

template <bool IsRenderProxy>
TGpuParameterSampledTexture<IsRenderProxy>::TGpuParameterSampledTexture(const GpuParameterBinding& binding, const GpuParamsType& parent)
	: mParent(parent), mBinding(binding)
{
	B3D_ASSERT(mParent->GetSet() == mBinding.Set);
}

template <bool IsRenderProxy>
void TGpuParameterSampledTexture<IsRenderProxy>::Set(const TextureType& texture, const TextureSurface& surface, u32 arrayIndex) const
{
	if(mParent == nullptr)
		return;

	mParent->SetSampledTexture(mBinding.Slot, texture, surface, arrayIndex);

	mParent->MarkResourcesDirtyInternal();
	mParent->MarkRenderProxyDataDirtyInternal();
}

template <bool IsRenderProxy>
typename TGpuParameterSampledTexture<IsRenderProxy>::TextureType TGpuParameterSampledTexture<IsRenderProxy>::Get(u32 arrayIndex) const
{
	if(mParent == nullptr)
		return TextureType();

	return mParent->GetSampledTexture(mBinding.Slot, arrayIndex);
}

template <bool IsRenderProxy>
TGpuParameterStorageBuffer<IsRenderProxy>::TGpuParameterStorageBuffer()
{}

template <bool IsRenderProxy>
TGpuParameterStorageBuffer<IsRenderProxy>::TGpuParameterStorageBuffer(const GpuParameterBinding& binding, const GpuParamsType& parent)
	: mParent(parent), mBinding(binding)
{
	B3D_ASSERT(mParent->GetSet() == mBinding.Set);
}

template <bool IsRenderProxy>
void TGpuParameterStorageBuffer<IsRenderProxy>::Set(const BufferType& buffer, u32 arrayIndex, GpuBufferViewInformation view) const
{
	if(mParent == nullptr)
		return;

	mParent->SetStorageBuffer(mBinding.Slot, buffer, arrayIndex, view);

	mParent->MarkResourcesDirtyInternal();
	mParent->MarkRenderProxyDataDirtyInternal();
}

template <bool IsRenderProxy>
typename TGpuParameterStorageBuffer<IsRenderProxy>::BufferType TGpuParameterStorageBuffer<IsRenderProxy>::Get(u32 arrayIndex) const
{
	if(mParent == nullptr)
		return BufferType();

	return mParent->GetStorageBuffer(mBinding.Slot, arrayIndex);
}

template <bool IsRenderProxy>
TGpuParameterUniformBuffer<IsRenderProxy>::TGpuParameterUniformBuffer()
{ }

template <bool IsRenderProxy>
TGpuParameterUniformBuffer<IsRenderProxy>::TGpuParameterUniformBuffer(const GpuParameterBinding& binding, const GpuParamsType& parent)
	: mParent(parent), mBinding(binding)
{
	B3D_ASSERT(mParent->GetSet() == mBinding.Set);
}

template <bool IsRenderProxy>
void TGpuParameterUniformBuffer<IsRenderProxy>::Set(const BufferType& buffer) const
{
	if(mParent == nullptr)
		return;

	mParent->SetUniformBuffer(mBinding.Slot, buffer);

	mParent->MarkResourcesDirtyInternal();
	mParent->MarkRenderProxyDataDirtyInternal();
}

template<>
template<>
B3D_EXPORT void TGpuParameterUniformBuffer<true>::Set(const render::GpuBufferSuballocation& bufferSuballocation) const
{
	if(mParent == nullptr)
		return;

	mParent->SetUniformBuffer(mBinding.Slot, bufferSuballocation);

	mParent->MarkResourcesDirtyInternal();
	mParent->MarkRenderProxyDataDirtyInternal();
}

template <bool IsRenderProxy>
typename TGpuParameterUniformBuffer<IsRenderProxy>::BufferType TGpuParameterUniformBuffer<IsRenderProxy>::Get() const
{
	if(mParent == nullptr)
		return BufferType();

	return mParent->GetUniformBuffer(mBinding.Slot);
}

template <bool IsRenderProxy>
TGpuParameterStorageTexture<IsRenderProxy>::TGpuParameterStorageTexture()
{}

template <bool IsRenderProxy>
TGpuParameterStorageTexture<IsRenderProxy>::TGpuParameterStorageTexture(const GpuParameterBinding& binding, const GpuParamsType& parent)
	: mParent(parent), mBinding(binding)
{
	B3D_ASSERT(mParent->GetSet() == mBinding.Set);
}

template <bool IsRenderProxy>
void TGpuParameterStorageTexture<IsRenderProxy>::Set(const TextureType& texture, const TextureSurface& surface, u32 arrayIndex) const
{
	if(mParent == nullptr)
		return;

	mParent->SetStorageTexture(mBinding.Slot, texture, surface, arrayIndex);

	mParent->MarkResourcesDirtyInternal();
	mParent->MarkRenderProxyDataDirtyInternal();
}

template <bool IsRenderProxy>
typename TGpuParameterStorageTexture<IsRenderProxy>::TextureType TGpuParameterStorageTexture<IsRenderProxy>::Get(u32 arrayIndex) const
{
	if(mParent == nullptr)
		return TextureType();

	return mParent->GetSampledTexture(mBinding.Slot, arrayIndex);
}

template <bool IsRenderProxy>
TGpuParameterSampler<IsRenderProxy>::TGpuParameterSampler()
{}

template <bool IsRenderProxy>
TGpuParameterSampler<IsRenderProxy>::TGpuParameterSampler(const GpuParameterBinding& binding, const GpuParamsType& parent)
	: mParent(parent), mBinding(binding)
{
	B3D_ASSERT(mParent->GetSet() == mBinding.Set);
}

template <bool IsRenderProxy>
void TGpuParameterSampler<IsRenderProxy>::Set(const SPtr<SamplerState>& samplerState, u32 arrayIndex) const
{
	if(mParent == nullptr)
		return;

	mParent->SetSamplerState(mBinding.Slot, samplerState, arrayIndex);

	mParent->MarkResourcesDirtyInternal();
	mParent->MarkRenderProxyDataDirtyInternal();
}

template <bool IsRenderProxy>
SPtr<SamplerState> TGpuParameterSampler<IsRenderProxy>::Get(u32 arrayIndex) const
{
	if (mParent == nullptr)
		return nullptr;

	return mParent->GetSamplerState(mBinding.Slot, arrayIndex);
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

template class TGpuParameterStorageBuffer<false>;
template class TGpuParameterStorageBuffer<true>;

template class TGpuParameterUniformBuffer<false>;
template class TGpuParameterUniformBuffer<true>;

template class TGpuParameterSampler<false>;
template class TGpuParameterSampler<true>;

template class TGpuParameterStorageTexture<false>;
template class TGpuParameterStorageTexture<true>;
