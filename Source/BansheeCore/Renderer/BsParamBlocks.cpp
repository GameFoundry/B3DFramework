//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Renderer/BsParamBlocks.h"
#include "RenderAPI/BsGpuParam.h"

namespace bs { namespace ct
{
	template<class T>
	ParamBlockParam<T>::ParamBlockParam(const GpuParamDataDesc& paramDesc)
		:mParamDesc(paramDesc)
	{ }

	template<class T>
	void ParamBlockParam<T>::set(const SPtr<GpuParamBlockBuffer>& paramBlock, const T& value, UINT32 arrayIdx) const
	{
#if BS_DEBUG_MODE
		if (arrayIdx >= mParamDesc.arraySize)
		{
			BS_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " +
					  toString(mParamDesc.arraySize) + ". Requested size: " + toString(arrayIdx));
		}
#endif

		UINT32 elementSizeBytes = mParamDesc.elementSize * sizeof(UINT32);
		UINT32 sizeBytes = std::min(elementSizeBytes, (UINT32)sizeof(T)); // Truncate if it doesn't fit within parameter size

		bool transposeMatrices = RenderAPI::instance().getAPIInfo().isFlagSet(RenderAPIFeatureFlag::ColumnMajorMatrices);
		if (TransposePolicy<T>::transposeEnabled(transposeMatrices))
		{
			auto transposed = TransposePolicy<T>::transpose(value);
			paramBlock->write((mParamDesc.cpuMemOffset + arrayIdx * mParamDesc.arrayElementStride) * sizeof(UINT32), 
				&transposed, sizeBytes);
		}
		else
			paramBlock->write((mParamDesc.cpuMemOffset + arrayIdx * mParamDesc.arrayElementStride) * sizeof(UINT32), 
				&value, sizeBytes);

		// Set unused bytes to 0
		if (sizeBytes < elementSizeBytes)
		{
			UINT32 diffSize = elementSizeBytes - sizeBytes;
			paramBlock->zeroOut((mParamDesc.cpuMemOffset + arrayIdx * mParamDesc.arrayElementStride) * sizeof(UINT32) + 
				sizeBytes, diffSize);
		}
	}

	template<class T>
	T ParamBlockParam<T>::get(const SPtr<GpuParamBlockBuffer>& paramBlock, UINT32 arrayIdx) const
	{
#if BS_DEBUG_MODE
		if (arrayIdx >= mParamDesc.arraySize)
		{
			LOGERR("Array index out of range. Array size: " + toString(mParamDesc.arraySize) + ". Requested size: " + 
				toString(arrayIdx));
			return T();
		}
#endif

		UINT32 elementSizeBytes = mParamDesc.elementSize * sizeof(UINT32);
		UINT32 sizeBytes = std::min(elementSizeBytes, (UINT32)sizeof(T));

		T value;
		paramBlock->read((mParamDesc.cpuMemOffset + arrayIdx * mParamDesc.arrayElementStride) * sizeof(UINT32), &value, 
			sizeBytes);

		return value;
	}

	template class ParamBlockParam<float>;
	template class ParamBlockParam<int>;
	template class ParamBlockParam<Color>;
	template class ParamBlockParam<Vector2>;
	template class ParamBlockParam<Vector3>;
	template class ParamBlockParam<Vector4>;
	template class ParamBlockParam<Vector2I>;
	template class ParamBlockParam<Vector3I>;
	template class ParamBlockParam<Vector4I>;
	template class ParamBlockParam<Matrix2>;
	template class ParamBlockParam<Matrix2x3>;
	template class ParamBlockParam<Matrix2x4>;
	template class ParamBlockParam<Matrix3>;
	template class ParamBlockParam<Matrix3x2>;
	template class ParamBlockParam<Matrix3x4>;
	template class ParamBlockParam<Matrix4>;
	template class ParamBlockParam<Matrix4x2>;
	template class ParamBlockParam<Matrix4x3>;

	ParamBlock::~ParamBlock()
	{
		ParamBlockManager::unregisterBlock(this);
	}

	Vector<ParamBlock*> ParamBlockManager::sToInitialize;

	ParamBlockManager::ParamBlockManager()
	{
		for (auto& entry : sToInitialize)
			entry->initialize();

		sToInitialize.clear();
	}

	void ParamBlockManager::registerBlock(ParamBlock* paramBlock)
	{
		if (isStarted())
			paramBlock->initialize();
		else
			sToInitialize.push_back(paramBlock);
	}

	void ParamBlockManager::unregisterBlock(ParamBlock* paramBlock)
	{
		auto iterFind = std::find(sToInitialize.begin(), sToInitialize.end(), paramBlock);
		if (iterFind != sToInitialize.end())
			sToInitialize.erase(iterFind);
	}
}}