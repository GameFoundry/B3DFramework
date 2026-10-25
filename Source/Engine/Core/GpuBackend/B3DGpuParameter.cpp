//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuParameter.h"

#include "B3DApplication.h"
#include "B3DGpuDevice.h"
#include "B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "Debug/B3DDebug.h"

using namespace b3d;

GpuParameterHandleBase::GpuParameterHandleBase([[maybe_unused]] const GpuPipelineParameterSetLayout* layout)
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	mLayout = layout;
#endif
}

void GpuParameterHandleBase::CheckSet([[maybe_unused]] const GpuParametersSetBase& set, [[maybe_unused]] u32 setIndex) const
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	B3D_ASSERT(set.GetLayout().get() == mLayout && set.GetSet() == setIndex);
#endif
}

template <class T, bool IsRenderProxy>
TGpuParameterPrimitive<T, IsRenderProxy>::TGpuParameterPrimitive()
{}

template <class T, bool IsRenderProxy>
TGpuParameterPrimitive<T, IsRenderProxy>::TGpuParameterPrimitive(const GpuUniformBufferMemberInformation* parameterInformation, const GpuPipelineParameterSetLayout* layout)
	: GpuParameterHandleBase(layout), mParameterInformation(parameterInformation)
{ }

template <class T, bool IsRenderProxy>
void TGpuParameterPrimitive<T, IsRenderProxy>::Set(const GpuParamsType& set, const T& value, u32 arrayIdx) const
{
	if(mParameterInformation == nullptr || set == nullptr)
		return;

	CheckSet(*set, mParameterInformation->ParentUniformBufferSet);

	GpuParamBufferType uniformBuffer = set->GetUniformBuffer(mParameterInformation->ParentUniformBufferSlot);
	if(uniformBuffer == nullptr)
		return;

	if(!B3D_ENSURE_LOG(arrayIdx < mParameterInformation->ArraySize, "Array index out of range. Array size: {0}. Requested size: {1}", mParameterInformation->ArraySize, arrayIdx))
		return;

	const GpuDataParameterTypeInformation& typeInformation = b3d::GpuParameterSet::kParamSizes.Lookup[mParameterInformation->Type];

	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	const GpuBackendConventions& gpuBackendConventions = gpuDevice->GetCapabilities().Conventions;

	const bool transposeMatrices = gpuBackendConventions.MatrixOrder == GpuBackendConventions::MatrixOrder::ColumnMajor;
	if(TransposePolicy<T>::TransposeEnabled(transposeMatrices))
	{
		const auto transposed = TransposePolicy<T>::Transpose(value);
		uniformBuffer->WriteTyped((mParameterInformation->CpuOffset + arrayIdx * mParameterInformation->ArrayElementStride) * sizeof(u32), typeInformation, &transposed);
	}
	else
		uniformBuffer->WriteTyped((mParameterInformation->CpuOffset + arrayIdx * mParameterInformation->ArrayElementStride) * sizeof(u32), typeInformation, &value);

	set->MarkRenderProxyDataDirtyInternal();
}

template <class T, bool IsRenderProxy>
T TGpuParameterPrimitive<T, IsRenderProxy>::Get(const GpuParamsType& set, u32 arrayIdx) const
{
	if(mParameterInformation == nullptr || set == nullptr)
		return T();

	CheckSet(*set, mParameterInformation->ParentUniformBufferSet);

	GpuParamBufferType uniformBuffer = set->GetUniformBuffer(mParameterInformation->ParentUniformBufferSlot);
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
TGpuParameterStruct<IsRenderProxy>::TGpuParameterStruct(const GpuUniformBufferMemberInformation* parameterInformation, const GpuPipelineParameterSetLayout* layout)
	: GpuParameterHandleBase(layout), mParameterInformation(parameterInformation)
{ }

template <bool IsRenderProxy>
void TGpuParameterStruct<IsRenderProxy>::Set(const GpuParamsType& set, const void* value, u32 sizeBytes, u32 arrayIdx) const
{
	if(mParameterInformation == nullptr || set == nullptr)
		return;

	CheckSet(*set, mParameterInformation->ParentUniformBufferSet);

	GpuParamBufferType uniformBuffer = set->GetUniformBuffer(mParameterInformation->ParentUniformBufferSlot);
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

	set->MarkRenderProxyDataDirtyInternal();
}

template <bool IsRenderProxy>
void TGpuParameterStruct<IsRenderProxy>::Get(const GpuParamsType& set, void* value, u32 sizeBytes, u32 arrayIdx) const
{
	if(mParameterInformation == nullptr || set == nullptr)
		return;

	CheckSet(*set, mParameterInformation->ParentUniformBufferSet);

	GpuParamBufferType uniformBuffer = set->GetUniformBuffer(mParameterInformation->ParentUniformBufferSlot);
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
	if(mParameterInformation == nullptr)
		return 0;

	return mParameterInformation->ElementSize * sizeof(u32);
}

template <bool IsRenderProxy>
TGpuParameterSampledTexture<IsRenderProxy>::TGpuParameterSampledTexture()
{ }

template <bool IsRenderProxy>
TGpuParameterSampledTexture<IsRenderProxy>::TGpuParameterSampledTexture(const GpuParameterBinding& binding, const GpuPipelineParameterSetLayout* layout)
	: GpuParameterHandleBase(layout), mBinding(binding)
{ }

template <bool IsRenderProxy>
void TGpuParameterSampledTexture<IsRenderProxy>::Set(const GpuParamsType& set, const TextureType& texture, const TextureSurface& surface, u32 arrayIndex) const
{
	if(!mBinding.IsValid() || set == nullptr)
		return;

	CheckSet(*set, mBinding.Set);
	set->SetSampledTexture(mBinding.Slot, texture, surface, arrayIndex);

	set->MarkResourcesDirtyInternal();
	set->MarkRenderProxyDataDirtyInternal();
}

template <bool IsRenderProxy>
typename TGpuParameterSampledTexture<IsRenderProxy>::TextureType TGpuParameterSampledTexture<IsRenderProxy>::Get(const GpuParamsType& set, u32 arrayIndex) const
{
	if(!mBinding.IsValid() || set == nullptr)
		return TextureType();

	CheckSet(*set, mBinding.Set);
	return set->GetSampledTexture(mBinding.Slot, arrayIndex);
}

template <bool IsRenderProxy>
TGpuParameterStorageBuffer<IsRenderProxy>::TGpuParameterStorageBuffer()
{}

template <bool IsRenderProxy>
TGpuParameterStorageBuffer<IsRenderProxy>::TGpuParameterStorageBuffer(const GpuParameterBinding& binding, const GpuPipelineParameterSetLayout* layout)
	: GpuParameterHandleBase(layout), mBinding(binding)
{ }

template <bool IsRenderProxy>
void TGpuParameterStorageBuffer<IsRenderProxy>::Set(const GpuParamsType& set, const BufferType& buffer, u32 arrayIndex, GpuBufferViewInformation view) const
{
	if(!mBinding.IsValid() || set == nullptr)
		return;

	CheckSet(*set, mBinding.Set);
	set->SetStorageBuffer(mBinding.Slot, buffer, arrayIndex, view);

	set->MarkResourcesDirtyInternal();
	set->MarkRenderProxyDataDirtyInternal();
}

template <bool IsRenderProxy>
typename TGpuParameterStorageBuffer<IsRenderProxy>::BufferType TGpuParameterStorageBuffer<IsRenderProxy>::Get(const GpuParamsType& set, u32 arrayIndex) const
{
	if(!mBinding.IsValid() || set == nullptr)
		return BufferType();

	CheckSet(*set, mBinding.Set);
	return set->GetStorageBuffer(mBinding.Slot, arrayIndex);
}

template <bool IsRenderProxy>
TGpuParameterUniformBuffer<IsRenderProxy>::TGpuParameterUniformBuffer()
{ }

template <bool IsRenderProxy>
TGpuParameterUniformBuffer<IsRenderProxy>::TGpuParameterUniformBuffer(const GpuParameterBinding& binding, const GpuPipelineParameterSetLayout* layout)
	: GpuParameterHandleBase(layout), mBinding(binding)
{ }

template<>
template<>
B3D_EXPORT void TGpuParameterUniformBuffer<true>::Set(const GpuParamsType& set, const render::GpuBufferSuballocation& bufferSuballocation) const
{
	if(!mBinding.IsValid() || set == nullptr)
		return;

	CheckSet(*set, mBinding.Set);
	set->SetUniformBuffer(mBinding.Slot, bufferSuballocation);

	set->MarkResourcesDirtyInternal();
	set->MarkRenderProxyDataDirtyInternal();
}

template <bool IsRenderProxy>
void TGpuParameterUniformBuffer<IsRenderProxy>::Set(const GpuParamsType& set, const BufferType& buffer) const
{
	if(!mBinding.IsValid() || set == nullptr)
		return;

	CheckSet(*set, mBinding.Set);
	set->SetUniformBuffer(mBinding.Slot, buffer);

	set->MarkResourcesDirtyInternal();
	set->MarkRenderProxyDataDirtyInternal();
}

template <bool IsRenderProxy>
typename TGpuParameterUniformBuffer<IsRenderProxy>::BufferType TGpuParameterUniformBuffer<IsRenderProxy>::Get(const GpuParamsType& set) const
{
	if(!mBinding.IsValid() || set == nullptr)
		return BufferType();

	CheckSet(*set, mBinding.Set);
	return set->GetUniformBuffer(mBinding.Slot);
}

template <bool IsRenderProxy>
TGpuParameterStorageTexture<IsRenderProxy>::TGpuParameterStorageTexture()
{}

template <bool IsRenderProxy>
TGpuParameterStorageTexture<IsRenderProxy>::TGpuParameterStorageTexture(const GpuParameterBinding& binding, const GpuPipelineParameterSetLayout* layout)
	: GpuParameterHandleBase(layout), mBinding(binding)
{ }

template <bool IsRenderProxy>
void TGpuParameterStorageTexture<IsRenderProxy>::Set(const GpuParamsType& set, const TextureType& texture, const TextureSurface& surface, u32 arrayIndex) const
{
	if(!mBinding.IsValid() || set == nullptr)
		return;

	CheckSet(*set, mBinding.Set);
	set->SetStorageTexture(mBinding.Slot, texture, surface, arrayIndex);

	set->MarkResourcesDirtyInternal();
	set->MarkRenderProxyDataDirtyInternal();
}

template <bool IsRenderProxy>
typename TGpuParameterStorageTexture<IsRenderProxy>::TextureType TGpuParameterStorageTexture<IsRenderProxy>::Get(const GpuParamsType& set, u32 arrayIndex) const
{
	if(!mBinding.IsValid() || set == nullptr)
		return TextureType();

	CheckSet(*set, mBinding.Set);
	return set->GetStorageTexture(mBinding.Slot, arrayIndex);
}

template <bool IsRenderProxy>
TGpuParameterSampler<IsRenderProxy>::TGpuParameterSampler()
{}

template <bool IsRenderProxy>
TGpuParameterSampler<IsRenderProxy>::TGpuParameterSampler(const GpuParameterBinding& binding, const GpuPipelineParameterSetLayout* layout)
	: GpuParameterHandleBase(layout), mBinding(binding)
{ }

template <bool IsRenderProxy>
void TGpuParameterSampler<IsRenderProxy>::Set(const GpuParamsType& set, const TShared<SamplerState>& samplerState, u32 arrayIndex) const
{
	if(!mBinding.IsValid() || set == nullptr)
		return;

	CheckSet(*set, mBinding.Set);
	set->SetSamplerState(mBinding.Slot, samplerState, arrayIndex);

	set->MarkResourcesDirtyInternal();
	set->MarkRenderProxyDataDirtyInternal();
}

template <bool IsRenderProxy>
TShared<SamplerState> TGpuParameterSampler<IsRenderProxy>::Get(const GpuParamsType& set, u32 arrayIndex) const
{
	if(!mBinding.IsValid() || set == nullptr)
		return nullptr;

	CheckSet(*set, mBinding.Set);
	return set->GetSamplerState(mBinding.Slot, arrayIndex);
}

namespace b3d
{
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
} // namespace b3d
