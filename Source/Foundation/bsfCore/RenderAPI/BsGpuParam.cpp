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

using namespace bs;

template <class T, bool Core>
TGpuDataParam<T, Core>::TGpuDataParam()
	: mParamDesc(nullptr)
{}

template <class T, bool Core>
TGpuDataParam<T, Core>::TGpuDataParam(GpuParamDataDesc* paramDesc, const GpuParamsType& parent)
	: mParent(parent), mParamDesc(paramDesc)
{}

template <class T, bool Core>
void TGpuDataParam<T, Core>::Set(const T& value, u32 arrayIdx) const
{
	if(mParent == nullptr)
		return;

	GpuParamBufferType paramBlock = mParent->GetParamBlockBuffer(mParamDesc->ParamBlockSet, mParamDesc->ParamBlockSlot);
	if(paramBlock == nullptr)
		return;

#if BS_DEBUG_MODE
	if(arrayIdx >= mParamDesc->ArraySize)
	{
		BS_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " + toString(mParamDesc->ArraySize) + ". Requested size: " + toString(arrayIdx));
	}
#endif

	u32 elementSizeBytes = mParamDesc->ElementSize * sizeof(u32);
	u32 sizeBytes = std::min(elementSizeBytes, (u32)sizeof(T)); // Truncate if it doesn't fit within parameter size

	const bool transposeMatrices = ct::gCaps().Conventions.MatrixOrder == Conventions::MatrixOrder::ColumnMajor;
	if(TransposePolicy<T>::TransposeEnabled(transposeMatrices))
	{
		auto transposed = TransposePolicy<T>::Transpose(value);
		paramBlock->Write((mParamDesc->CpuMemOffset + arrayIdx * mParamDesc->ArrayElementStride) * sizeof(u32), &transposed, sizeBytes);
	}
	else
		paramBlock->Write((mParamDesc->CpuMemOffset + arrayIdx * mParamDesc->ArrayElementStride) * sizeof(u32), &value, sizeBytes);

	// Set unused bytes to 0
	if(sizeBytes < elementSizeBytes)
	{
		u32 diffSize = elementSizeBytes - sizeBytes;
		paramBlock->ZeroOut((mParamDesc->CpuMemOffset + arrayIdx * mParamDesc->ArrayElementStride) * sizeof(u32) + sizeBytes, diffSize);
	}

	mParent->MarkCoreDirtyInternal();
}

template <class T, bool Core>
T TGpuDataParam<T, Core>::Get(u32 arrayIdx) const
{
	if(mParent == nullptr)
		return T();

	GpuParamBufferType paramBlock = mParent->GetParamBlockBuffer(mParamDesc->ParamBlockSet, mParamDesc->ParamBlockSlot);
	if(paramBlock == nullptr)
		return T();

#if BS_DEBUG_MODE
	if(arrayIdx >= mParamDesc->ArraySize)
	{
		BS_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " + toString(mParamDesc->ArraySize) + ". Requested size: " + toString(arrayIdx));
	}
#endif

	u32 elementSizeBytes = mParamDesc->ElementSize * sizeof(u32);
	u32 sizeBytes = std::min(elementSizeBytes, (u32)sizeof(T));

	T value;
	paramBlock->Read((mParamDesc->CpuMemOffset + arrayIdx * mParamDesc->ArrayElementStride) * sizeof(u32), &value, sizeBytes);

	return value;
}

template <bool Core>
TGpuParamStruct<Core>::TGpuParamStruct()
	: mParamDesc(nullptr)
{}

template <bool Core>
TGpuParamStruct<Core>::TGpuParamStruct(GpuParamDataDesc* paramDesc, const GpuParamsType& parent)
	: mParent(parent), mParamDesc(paramDesc)
{}

template <bool Core>
void TGpuParamStruct<Core>::Set(const void* value, u32 sizeBytes, u32 arrayIdx) const
{
	if(mParent == nullptr)
		return;

	GpuParamBufferType paramBlock = mParent->GetParamBlockBuffer(mParamDesc->ParamBlockSet, mParamDesc->ParamBlockSlot);
	if(paramBlock == nullptr)
		return;

	u32 elementSizeBytes = mParamDesc->ElementSize * sizeof(u32);

#if BS_DEBUG_MODE
	if(sizeBytes > elementSizeBytes)
	{
		BS_LOG(Warning, RenderBackend, "Provided element size larger than maximum element size. Maximum size: {0}."
									   " Supplied size: {1}",
			   elementSizeBytes, sizeBytes);
	}

	if(arrayIdx >= mParamDesc->ArraySize)
	{
		BS_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " + toString(mParamDesc->ArraySize) + ". Requested size: " + toString(arrayIdx));
	}
#endif

	sizeBytes = std::min(elementSizeBytes, sizeBytes);

	paramBlock->Write((mParamDesc->CpuMemOffset + arrayIdx * mParamDesc->ArrayElementStride) * sizeof(u32), value, sizeBytes);

	// Set unused bytes to 0
	if(sizeBytes < elementSizeBytes)
	{
		u32 diffSize = elementSizeBytes - sizeBytes;
		paramBlock->ZeroOut((mParamDesc->CpuMemOffset + arrayIdx * mParamDesc->ArrayElementStride) * sizeof(u32) + sizeBytes, diffSize);
	}

	mParent->MarkCoreDirtyInternal();
}

template <bool Core>
void TGpuParamStruct<Core>::Get(void* value, u32 sizeBytes, u32 arrayIdx) const
{
	if(mParent == nullptr)
		return;

	GpuParamBufferType paramBlock = mParent->GetParamBlockBuffer(mParamDesc->ParamBlockSet, mParamDesc->ParamBlockSlot);
	if(paramBlock == nullptr)
		return;

	u32 elementSizeBytes = mParamDesc->ElementSize * sizeof(u32);

#if BS_DEBUG_MODE
	if(sizeBytes > elementSizeBytes)
	{
		BS_LOG(Warning, RenderBackend, "Provided element size larger than maximum element size. Maximum size: {0}."
									   " Supplied size: {1}",
			   elementSizeBytes, sizeBytes);
	}

	if(arrayIdx >= mParamDesc->ArraySize)
	{
		BS_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " + toString(mParamDesc->ArraySize) + ". Requested size: " + toString(arrayIdx));
	}
#endif
	sizeBytes = std::min(elementSizeBytes, sizeBytes);

	paramBlock->Read((mParamDesc->CpuMemOffset + arrayIdx * mParamDesc->ArrayElementStride) * sizeof(u32), value, sizeBytes);
}

template <bool Core>
u32 TGpuParamStruct<Core>::GetElementSize() const
{
	if(mParent == nullptr)
		return 0;

	return mParamDesc->ElementSize * sizeof(u32);
}

template <bool Core>
TGpuParamTexture<Core>::TGpuParamTexture()
	: mParamDesc(nullptr)
{}

template <bool Core>
TGpuParamTexture<Core>::TGpuParamTexture(GpuParamObjectDesc* paramDesc, const GpuParamsType& parent)
	: mParent(parent), mParamDesc(paramDesc)
{}

template <bool Core>
void TGpuParamTexture<Core>::Set(const TextureType& texture, const TextureSurface& surface) const
{
	if(mParent == nullptr)
		return;

	mParent->SetTexture(mParamDesc->Set, mParamDesc->Slot, texture, surface);

	mParent->MarkResourcesDirtyInternal();
	mParent->MarkCoreDirtyInternal();
}

template <bool Core>
typename TGpuParamTexture<Core>::TextureType TGpuParamTexture<Core>::Get() const
{
	if(mParent == nullptr)
		return TextureType();

	return mParent->GetTexture(mParamDesc->Set, mParamDesc->Slot);
}

template <bool Core>
TGpuParamBuffer<Core>::TGpuParamBuffer()
	: mParamDesc(nullptr)
{}

template <bool Core>
TGpuParamBuffer<Core>::TGpuParamBuffer(GpuParamObjectDesc* paramDesc, const GpuParamsType& parent)
	: mParent(parent), mParamDesc(paramDesc)
{}

template <bool Core>
void TGpuParamBuffer<Core>::Set(const BufferType& buffer) const
{
	if(mParent == nullptr)
		return;

	mParent->SetBuffer(mParamDesc->Set, mParamDesc->Slot, buffer);

	mParent->MarkResourcesDirtyInternal();
	mParent->MarkCoreDirtyInternal();
}

template <bool Core>
typename TGpuParamBuffer<Core>::BufferType TGpuParamBuffer<Core>::Get() const
{
	if(mParent == nullptr)
		return BufferType();

	return mParent->GetBuffer(mParamDesc->Set, mParamDesc->Slot);
}

template <bool Core>
TGpuParamLoadStoreTexture<Core>::TGpuParamLoadStoreTexture()
	: mParamDesc(nullptr)
{}

template <bool Core>
TGpuParamLoadStoreTexture<Core>::TGpuParamLoadStoreTexture(GpuParamObjectDesc* paramDesc, const GpuParamsType& parent)
	: mParent(parent), mParamDesc(paramDesc)
{}

template <bool Core>
void TGpuParamLoadStoreTexture<Core>::Set(const TextureType& texture, const TextureSurface& surface) const
{
	if(mParent == nullptr)
		return;

	mParent->SetLoadStoreTexture(mParamDesc->Set, mParamDesc->Slot, texture, surface);

	mParent->MarkResourcesDirtyInternal();
	mParent->MarkCoreDirtyInternal();
}

template <bool Core>
typename TGpuParamLoadStoreTexture<Core>::TextureType TGpuParamLoadStoreTexture<Core>::Get() const
{
	if(mParent == nullptr)
		return TextureType();

	return mParent->GetTexture(mParamDesc->Set, mParamDesc->Slot);
}

template <bool Core>
TGpuParamSampState<Core>::TGpuParamSampState()
	: mParamDesc(nullptr)
{}

template <bool Core>
TGpuParamSampState<Core>::TGpuParamSampState(GpuParamObjectDesc* paramDesc, const GpuParamsType& parent)
	: mParent(parent), mParamDesc(paramDesc)
{}

template <bool Core>
void TGpuParamSampState<Core>::Set(const SamplerStateType& samplerState) const
{
	if(mParent == nullptr)
		return;

	mParent->SetSamplerState(mParamDesc->Set, mParamDesc->Slot, samplerState);

	mParent->MarkResourcesDirtyInternal();
	mParent->MarkCoreDirtyInternal();
}

template <bool Core>
typename TGpuParamSampState<Core>::SamplerStateType TGpuParamSampState<Core>::Get() const
{
	if(mParent == nullptr)
		return SamplerStateType();

	return mParent->GetSamplerState(mParamDesc->Set, mParamDesc->Slot);
}

template class TGpuDataParam<float, false>;
template class TGpuDataParam<int, false>;
template class TGpuDataParam<Color, false>;
template class TGpuDataParam<Vector2, false>;
template class TGpuDataParam<Vector3, false>;
template class TGpuDataParam<Vector4, false>;
template class TGpuDataParam<Vector2I, false>;
template class TGpuDataParam<Vector3I, false>;
template class TGpuDataParam<Vector4I, false>;
template class TGpuDataParam<Matrix2, false>;
template class TGpuDataParam<Matrix2x3, false>;
template class TGpuDataParam<Matrix2x4, false>;
template class TGpuDataParam<Matrix3, false>;
template class TGpuDataParam<Matrix3x2, false>;
template class TGpuDataParam<Matrix3x4, false>;
template class TGpuDataParam<Matrix4, false>;
template class TGpuDataParam<Matrix4x2, false>;
template class TGpuDataParam<Matrix4x3, false>;

template class TGpuDataParam<float, true>;
template class TGpuDataParam<int, true>;
template class TGpuDataParam<Color, true>;
template class TGpuDataParam<Vector2, true>;
template class TGpuDataParam<Vector3, true>;
template class TGpuDataParam<Vector4, true>;
template class TGpuDataParam<Vector2I, true>;
template class TGpuDataParam<Vector3I, true>;
template class TGpuDataParam<Vector4I, true>;
template class TGpuDataParam<Matrix2, true>;
template class TGpuDataParam<Matrix2x3, true>;
template class TGpuDataParam<Matrix2x4, true>;
template class TGpuDataParam<Matrix3, true>;
template class TGpuDataParam<Matrix3x2, true>;
template class TGpuDataParam<Matrix3x4, true>;
template class TGpuDataParam<Matrix4, true>;
template class TGpuDataParam<Matrix4x2, true>;
template class TGpuDataParam<Matrix4x3, true>;

template class TGpuParamStruct<false>;
template class TGpuParamStruct<true>;

template class TGpuParamTexture<false>;
template class TGpuParamTexture<true>;

template class TGpuParamBuffer<false>;
template class TGpuParamBuffer<true>;

template class TGpuParamSampState<false>;
template class TGpuParamSampState<true>;

template class TGpuParamLoadStoreTexture<false>;
template class TGpuParamLoadStoreTexture<true>;
