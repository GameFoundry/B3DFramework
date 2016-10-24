//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGpuParam.h"
#include "BsGpuParams.h"
#include "BsGpuParamBlockBuffer.h"
#include "BsGpuParamDesc.h"
#include "BsRenderAPI.h"
#include "BsDebug.h"
#include "BsException.h"
#include "BsVectorNI.h"
#include "BsVector2I.h"

namespace BansheeEngine
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
	void TGpuDataParam<T, Core>::set(const T& value, UINT32 arrayIdx) const
	{
		if (mParent == nullptr)
			return;

		GpuParamBufferType paramBlock = mParent->getParamBlockBuffer(mParamDesc->paramBlockSet, mParamDesc->paramBlockSlot);
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

		bool transposeMatrices = RenderAPICore::instance().getAPIInfo().getGpuProgramHasColumnMajorMatrices();
		if (TransposePolicy<T>::transposeEnabled(transposeMatrices))
		{
			T transposed = TransposePolicy<T>::transpose(value);
			paramBlock->write((mParamDesc->cpuMemOffset + arrayIdx * mParamDesc->arrayElementStride) * sizeof(UINT32), &transposed, sizeBytes);
		}
		else
			paramBlock->write((mParamDesc->cpuMemOffset + arrayIdx * mParamDesc->arrayElementStride) * sizeof(UINT32), &value, sizeBytes);

		// Set unused bytes to 0
		if (sizeBytes < elementSizeBytes)
		{
			UINT32 diffSize = elementSizeBytes - sizeBytes;
			paramBlock->zeroOut((mParamDesc->cpuMemOffset + arrayIdx * mParamDesc->arrayElementStride)  * sizeof(UINT32) + sizeBytes, diffSize);
		}

		mParent->_markCoreDirty();
	}

	template<class T, bool Core>
	T TGpuDataParam<T, Core>::get(UINT32 arrayIdx) const
	{
		if (mParent == nullptr)
			return T();

		GpuParamBufferType paramBlock = mParent->getParamBlockBuffer(mParamDesc->paramBlockSet, mParamDesc->paramBlockSlot);
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
		paramBlock->read((mParamDesc->cpuMemOffset + arrayIdx * mParamDesc->arrayElementStride) * sizeof(UINT32), &value, sizeBytes);

		bool transposeMatrices = RenderAPICore::instance().getAPIInfo().getGpuProgramHasColumnMajorMatrices();
		if (TransposePolicy<T>::transposeEnabled(transposeMatrices))
			return TransposePolicy<T>::transpose(value);
		else
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
	void TGpuParamStruct<Core>::set(const void* value, UINT32 sizeBytes, UINT32 arrayIdx) const
	{
		if (mParent == nullptr)
			return;

		GpuParamBufferType paramBlock = mParent->getParamBlockBuffer(mParamDesc->paramBlockSet, mParamDesc->paramBlockSlot);
		if (paramBlock == nullptr)
			return;

		UINT32 elementSizeBytes = mParamDesc->elementSize * sizeof(UINT32);

#if BS_DEBUG_MODE
		if (sizeBytes > elementSizeBytes)
		{
			LOGWRN("Provided element size larger than maximum element size. Maximum size: " +
				toString(elementSizeBytes) + ". Supplied size: " + toString(sizeBytes));
		}

		if (arrayIdx >= mParamDesc->arraySize)
		{
			BS_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " +
				toString(mParamDesc->arraySize) + ". Requested size: " + toString(arrayIdx));
		}
#endif

		sizeBytes = std::min(elementSizeBytes, sizeBytes);

		paramBlock->write((mParamDesc->cpuMemOffset + arrayIdx * mParamDesc->arrayElementStride) * sizeof(UINT32), value, sizeBytes);

		// Set unused bytes to 0
		if (sizeBytes < elementSizeBytes)
		{
			UINT32 diffSize = elementSizeBytes - sizeBytes;
			paramBlock->zeroOut((mParamDesc->cpuMemOffset + arrayIdx * mParamDesc->arrayElementStride)  * sizeof(UINT32) + sizeBytes, diffSize);
		}

		mParent->_markCoreDirty();
	}

	template<bool Core>
	void TGpuParamStruct<Core>::get(void* value, UINT32 sizeBytes, UINT32 arrayIdx) const
	{
		if (mParent == nullptr)
			return;

		GpuParamBufferType paramBlock = mParent->getParamBlockBuffer(mParamDesc->paramBlockSet, mParamDesc->paramBlockSlot);
		if (paramBlock == nullptr)
			return;

		UINT32 elementSizeBytes = mParamDesc->elementSize * sizeof(UINT32);

#if BS_DEBUG_MODE
		if (sizeBytes > elementSizeBytes)
		{
			LOGWRN("Provided element size larger than maximum element size. Maximum size: " +
				toString(elementSizeBytes) + ". Supplied size: " + toString(sizeBytes));
		}

		if (arrayIdx >= mParamDesc->arraySize)
		{
			BS_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " +
				toString(mParamDesc->arraySize) + ". Requested size: " + toString(arrayIdx));
		}
#endif
		sizeBytes = std::min(elementSizeBytes, sizeBytes);

		paramBlock->read((mParamDesc->cpuMemOffset + arrayIdx * mParamDesc->arrayElementStride) * sizeof(UINT32), value, sizeBytes);
	}

	template<bool Core>
	UINT32 TGpuParamStruct<Core>::getElementSize() const
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
	void TGpuParamTexture<Core>::set(const TextureType& texture) const
	{
		if (mParent == nullptr)
			return;

		mParent->setTexture(mParamDesc->set, mParamDesc->slot, texture);

		mParent->_markResourcesDirty();
		mParent->_markCoreDirty();
	}

	template<bool Core>
	typename TGpuParamTexture<Core>::TextureType TGpuParamTexture<Core>::get() const
	{
		if (mParent == nullptr)
			return TextureType();

		return mParent->getTexture(mParamDesc->set, mParamDesc->slot);
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
	void TGpuParamBuffer<Core>::set(const BufferType& buffer) const
	{
		if (mParent == nullptr)
			return;

		mParent->setBuffer(mParamDesc->set, mParamDesc->slot, buffer);

		mParent->_markResourcesDirty();
		mParent->_markCoreDirty();
	}

	template<bool Core>
	typename TGpuParamBuffer<Core>::BufferType TGpuParamBuffer<Core>::get() const
	{
		if (mParent == nullptr)
			return BufferType();

		return mParent->getBuffer(mParamDesc->set, mParamDesc->slot);
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
	void TGpuParamLoadStoreTexture<Core>::set(const TextureType& texture, const TextureSurface& surface) const
	{
		if (mParent == nullptr)
			return;

		mParent->setLoadStoreTexture(mParamDesc->set, mParamDesc->slot, texture, surface);

		mParent->_markResourcesDirty();
		mParent->_markCoreDirty();
	}

	template<bool Core>
	typename TGpuParamLoadStoreTexture<Core>::TextureType TGpuParamLoadStoreTexture<Core>::get() const
	{
		if (mParent == nullptr)
			return TextureType();

		return mParent->getTexture(mParamDesc->set, mParamDesc->slot);
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
	void TGpuParamSampState<Core>::set(const SamplerStateType& samplerState) const
	{
		if (mParent == nullptr)
			return;

		mParent->setSamplerState(mParamDesc->set, mParamDesc->slot, samplerState);

		mParent->_markResourcesDirty();
		mParent->_markCoreDirty();
	}

	template<bool Core>
	typename TGpuParamSampState<Core>::SamplerStateType TGpuParamSampState<Core>::get() const
	{
		if (mParent == nullptr)
			return SamplerStateType();

		return mParent->getSamplerState(mParamDesc->set, mParamDesc->slot);
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