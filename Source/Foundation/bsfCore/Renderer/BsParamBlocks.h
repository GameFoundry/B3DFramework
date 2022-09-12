//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include "RenderAPI/BsGpuParams.h"
#include "RenderAPI/BsRenderAPI.h"
#include "RenderAPI/BsGpuParamBlockBuffer.h"

namespace bs { namespace ct
{
	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	/** Wrapper for a single parameter in a parameter block buffer. */
	template<class T>
	class ParamBlockParam
	{
	public:
		ParamBlockParam() = default;
		ParamBlockParam(const GpuParamDataDesc& paramDesc)
			:mParamDesc(paramDesc)
		{ }

		/**
		 * Sets the parameter in the provided parameter block buffer. Caller is responsible for ensuring the param block
		 * buffer contains this parameter.
		 */
		void Set(const SPtr<GpuParamBlockBuffer>& paramBlock, const T& value, UINT32 arrayIdx = 0) const
		{
#if BS_DEBUG_MODE
			if (arrayIdx >= mParamDesc.arraySize)
			{
				BS_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " +
						ToString(mParamDesc.arraySize) + ". Requested size: " +
													  ToString(arrayIdx));
			}
#endif

			UINT32 elementSizeBytes = mParamDesc.elementSize * sizeof(UINT32);
			UINT32 sizeBytes = std::min(elementSizeBytes, (UINT32)sizeof(T)); // Truncate if it doesn't fit within parameter size

			const bool transposeMatrices = gCaps().conventions.matrixOrder == Conventions::MatrixOrder::ColumnMajor;
			if (TransposePolicy<T>::transposeEnabled(transposeMatrices))
			{
				auto transposed = TransposePolicy<T>::transpose(value);
				paramBlock->Write((mParamDesc.cpuMemOffset + arrayIdx * mParamDesc.arrayElementStride) * sizeof(UINT32),
					&transposed, sizeBytes);
			}
			else
				paramBlock->Write((mParamDesc.cpuMemOffset + arrayIdx * mParamDesc.arrayElementStride) * sizeof(UINT32),
					&value, sizeBytes);

			// Set unused bytes to 0
			if (sizeBytes < elementSizeBytes)
			{
				UINT32 diffSize = elementSizeBytes - sizeBytes;
				paramBlock->ZeroOut((mParamDesc.cpuMemOffset + arrayIdx * mParamDesc.arrayElementStride) * sizeof(UINT32) +
					sizeBytes, diffSize);
			}
		}

		/**
		 * Gets the parameter in the provided parameter block buffer. Caller is responsible for ensuring the param block
		 * buffer contains this parameter.
		 */
		T Get(const SPtr<GpuParamBlockBuffer>& paramBlock, UINT32 arrayIdx = 0) const
		{
#if BS_DEBUG_MODE
			if (arrayIdx >= mParamDesc.arraySize)
			{
				BS_LOG(Error, Material, "Array index out of range. Array size: {0}. Requested size: {1}",
					mParamDesc.arraySize, arrayIdx);
				return T();
			}
#endif

			UINT32 elementSizeBytes = mParamDesc.elementSize * sizeof(UINT32);
			UINT32 sizeBytes = std::min(elementSizeBytes, (UINT32)sizeof(T));

			T value;
			paramBlock->Read((mParamDesc.cpuMemOffset + arrayIdx * mParamDesc.arrayElementStride) * sizeof(UINT32), &value,
				sizeBytes);

			return value;
		}

	protected:
		GpuParamDataDesc mParamDesc;
	};

	/** Base class for all parameter blocks. */
	struct BS_CORE_EXPORT ParamBlock
	{
		virtual ~ParamBlock();
		virtual void Initialize() = 0;
	};

	/**
	 * Takes care of initializing param block definitions in a delayed manner since they depend on engine systems yet
	 * are usually used as global variables which are initialized before engine systems are ready.
	 */
	class BS_CORE_EXPORT ParamBlockManager : public Module<ParamBlockManager>
	{
	public:
		ParamBlockManager();

		/** Registers a new param block, and initializes it when ready. */
		static void RegisterBlock(ParamBlock* paramBlock);

		/** Removes the param block from the initialization list. */
		static void UnregisterBlock(ParamBlock* paramBlock);

	private:
		static Vector<ParamBlock*> sToInitialize;
	};

/**
 * Starts a new custom parameter block. Custom parameter blocks allow you to create C++ structures that map directly
 * to GPU program buffers (for example uniform buffer in OpenGL or constant buffer in DX). Must be followed by
 * BS_PARAM_BLOCK_END.
 */
#define BS_PARAM_BLOCK_BEGIN(Name)																							\
	struct Name	: ParamBlock																								\
	{																														\
		Name()																												\
		{																													\
			ParamBlockManager::registerBlock(this);																			\
		}																													\
																															\
		SPtr<GpuParamBlockBuffer> CreateBuffer() const { return GpuParamBlockBuffer::create(mBlockSize); }					\
																															\
	private:																												\
		friend class ParamBlockManager;																						\
																															\
		void Initialize() override																							\
		{																													\
			mParams = getEntries();																							\
			RenderAPI& rapi = RenderAPI::instance();																		\
																															\
			GpuParamBlockDesc blockDesc = rapi.GenerateParamBlockDesc(#Name, mParams);										\
			mBlockSize = blockDesc.blockSize * sizeof(UINT32);																\
																															\
			initEntries();																									\
		}																													\
																															\
		struct META_FirstEntry {};																							\
		static void META_GetPrevEntries(Vector<GpuParamDataDesc>& params, META_FirstEntry id) { }							\
		void META_InitPrevEntry(const Vector<GpuParamDataDesc>& params, UINT32 idx, META_FirstEntry id) { }					\
																															\
		typedef META_FirstEntry

/**
 * Registers a new entry in a parameter block. Must be called in between BS_PARAM_BLOCK_BEGIN and BS_PARAM_BLOCK_END calls.
 */
#define BS_PARAM_BLOCK_ENTRY_ARRAY(Type, Name, NumElements)																	\
		META_Entry_##Name;																									\
																															\
		struct META_NextEntry_##Name {};																					\
		static void META_GetPrevEntries(Vector<GpuParamDataDesc>& params, META_NextEntry_##Name id)							\
		{																													\
			META_GetPrevEntries(params, META_Entry_##Name());																\
																															\
			params.push_back(GpuParamDataDesc());																			\
			GpuParamDataDesc& newEntry = params.back();																		\
			newEntry.name = #Name;																							\
			newEntry.type = (GpuParamDataType)TGpuDataParamInfo<Type>::TypeId;												\
			newEntry.arraySize = NumElements;																				\
			newEntry.elementSize = sizeof(Type);																			\
		}																													\
																															\
		void META_InitPrevEntry(const Vector<GpuParamDataDesc>& params, UINT32 idx, META_NextEntry_##Name id)				\
		{																													\
			META_InitPrevEntry(params, idx - 1, META_Entry_##Name());														\
			Name = ParamBlockParam<Type>(params[idx]);																		\
		}																													\
																															\
	public:																													\
		ParamBlockParam<Type> Name;																							\
																															\
	private:																												\
		typedef META_NextEntry_##Name

/**
 * Registers a new entry in a parameter block. Must be called in between BS_PARAM_BLOCK_BEGIN and BS_PARAM_BLOCK_END calls.
 */
#define BS_PARAM_BLOCK_ENTRY(Type, Name) BS_PARAM_BLOCK_ENTRY_ARRAY(Type, Name, 1)

/** Ends parameter block definition. See BS_PARAM_BLOCK_BEGIN. */
#define BS_PARAM_BLOCK_END																									\
		META_LastEntry;																										\
																															\
		static Vector<GpuParamDataDesc> GetEntries()																		\
		{																													\
			Vector<GpuParamDataDesc> entries;																				\
			META_GetPrevEntries(entries, META_LastEntry());																	\
			return entries;																									\
		}																													\
																															\
		void InitEntries()																									\
		{																													\
			META_InitPrevEntry(mParams, (UINT32)mParams.size() - 1, META_LastEntry());										\
		}																													\
																															\
		Vector<GpuParamDataDesc> mParams;																					\
		UINT32 mBlockSize;																									\
	};

	/** @} */
}}
