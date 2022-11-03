//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include "RenderAPI/BsGpuParams.h"
#include "RenderAPI/BsRenderAPI.h"
#include "RenderAPI/BsGpuParamBlockBuffer.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Renderer-Internal
		 *  @{
		 */

		/** Wrapper for a single parameter in a parameter block buffer. */
		template <class T>
		class ParamBlockParam
		{
		public:
			ParamBlockParam() = default;

			ParamBlockParam(const GpuParamDataDesc& paramDesc)
				: mParamDesc(paramDesc)
			{}

			/**
			 * Sets the parameter in the provided parameter block buffer. Caller is responsible for ensuring the param block
			 * buffer contains this parameter.
			 */
			void Set(const SPtr<GpuParamBlockBuffer>& paramBlock, const T& value, u32 arrayIdx = 0) const
			{
#if B3D_DEBUG
				if(arrayIdx >= mParamDesc.ArraySize)
				{
					B3D_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " + ToString(mParamDesc.ArraySize) + ". Requested size: " + ToString(arrayIdx));
				}
#endif

				u32 elementSizeBytes = mParamDesc.ElementSize * sizeof(u32);
				u32 sizeBytes = std::min(elementSizeBytes, (u32)sizeof(T)); // Truncate if it doesn't fit within parameter size

				const bool transposeMatrices = GetRenderBackendCapabilities().Conventions.MatrixOrder == Conventions::MatrixOrder::ColumnMajor;
				if(TransposePolicy<T>::TransposeEnabled(transposeMatrices))
				{
					auto transposed = TransposePolicy<T>::Transpose(value);
					paramBlock->Write((mParamDesc.CpuMemOffset + arrayIdx * mParamDesc.ArrayElementStride) * sizeof(u32), &transposed, sizeBytes);
				}
				else
					paramBlock->Write((mParamDesc.CpuMemOffset + arrayIdx * mParamDesc.ArrayElementStride) * sizeof(u32), &value, sizeBytes);

				// Set unused bytes to 0
				if(sizeBytes < elementSizeBytes)
				{
					u32 diffSize = elementSizeBytes - sizeBytes;
					paramBlock->ZeroOut((mParamDesc.CpuMemOffset + arrayIdx * mParamDesc.ArrayElementStride) * sizeof(u32) + sizeBytes, diffSize);
				}
			}

			/**
			 * Gets the parameter in the provided parameter block buffer. Caller is responsible for ensuring the param block
			 * buffer contains this parameter.
			 */
			T Get(const SPtr<GpuParamBlockBuffer>& paramBlock, u32 arrayIdx = 0) const
			{
#if B3D_DEBUG
				if(arrayIdx >= mParamDesc.ArraySize)
				{
					B3D_LOG(Error, Material, "Array index out of range. Array size: {0}. Requested size: {1}", mParamDesc.ArraySize, arrayIdx);
					return T();
				}
#endif

				u32 elementSizeBytes = mParamDesc.ElementSize * sizeof(u32);
				u32 sizeBytes = std::min(elementSizeBytes, (u32)sizeof(T));

				T value;
				paramBlock->Read((mParamDesc.CpuMemOffset + arrayIdx * mParamDesc.ArrayElementStride) * sizeof(u32), &value, sizeBytes);

				return value;
			}

		protected:
			GpuParamDataDesc mParamDesc;
		};

		/** Base class for all parameter blocks. */
		struct B3D_CORE_EXPORT ParamBlock
		{
			virtual ~ParamBlock();
			virtual void Initialize() = 0;
		};

		/**
		 * Takes care of initializing param block definitions in a delayed manner since they depend on engine systems yet
		 * are usually used as global variables which are initialized before engine systems are ready.
		 */
		class B3D_CORE_EXPORT ParamBlockManager : public Module<ParamBlockManager>
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
#define BS_PARAM_BLOCK_BEGIN(Name)                                                                   \
	struct Name : ParamBlock                                                                         \
	{                                                                                                \
		Name()                                                                                       \
		{                                                                                            \
			ParamBlockManager::RegisterBlock(this);                                                  \
		}                                                                                            \
                                                                                                     \
		SPtr<GpuParamBlockBuffer> CreateBuffer() const                                               \
		{                                                                                            \
			return GpuParamBlockBuffer::Create(mBlockSize);                                          \
		}                                                                                            \
                                                                                                     \
	private:                                                                                         \
		friend class ParamBlockManager;                                                              \
                                                                                                     \
		void Initialize() override                                                                   \
		{                                                                                            \
			mParams = GetEntries();                                                                  \
			RenderAPI& rapi = RenderAPI::Instance();                                                 \
                                                                                                     \
			GpuParamBlockDesc blockDesc = rapi.GenerateParamBlockDesc(#Name, mParams);               \
			mBlockSize = blockDesc.BlockSize * sizeof(u32);                                          \
                                                                                                     \
			InitEntries();                                                                           \
		}                                                                                            \
                                                                                                     \
		struct META_FirstEntry                                                                       \
		{};                                                                                          \
		static void META_GetPrevEntries(Vector<GpuParamDataDesc>& params, META_FirstEntry id)        \
		{}                                                                                           \
		void META_InitPrevEntry(const Vector<GpuParamDataDesc>& params, u32 idx, META_FirstEntry id) \
		{}                                                                                           \
                                                                                                     \
		typedef META_FirstEntry

/**
 * Registers a new entry in a parameter block. Must be called in between BS_PARAM_BLOCK_BEGIN and BS_PARAM_BLOCK_END calls.
 */
#define BS_PARAM_BLOCK_ENTRY_ARRAY(Type_, Name_, ElementCount)                                          \
	META_Entry_##Name_;                                                                                 \
                                                                                                        \
	struct META_NextEntry_##Name_                                                                       \
	{};                                                                                                 \
	static void META_GetPrevEntries(Vector<GpuParamDataDesc>& params, META_NextEntry_##Name_ id)        \
	{                                                                                                   \
		META_GetPrevEntries(params, META_Entry_##Name_());                                              \
                                                                                                        \
		params.push_back(GpuParamDataDesc());                                                           \
		GpuParamDataDesc& newEntry = params.back();                                                     \
		newEntry.Name = #Name_;                                                                         \
		newEntry.Type = (GpuParamDataType)TGpuDataParamInfo<Type_>::TypeId;                             \
		newEntry.ArraySize = ElementCount;                                                              \
		newEntry.ElementSize = sizeof(Type_);                                                           \
	}                                                                                                   \
                                                                                                        \
	void META_InitPrevEntry(const Vector<GpuParamDataDesc>& params, u32 idx, META_NextEntry_##Name_ id) \
	{                                                                                                   \
		META_InitPrevEntry(params, idx - 1, META_Entry_##Name_());                                      \
		Name_ = ParamBlockParam<Type_>(params[idx]);                                                    \
	}                                                                                                   \
                                                                                                        \
public:                                                                                                 \
	ParamBlockParam<Type_> Name_;                                                                       \
                                                                                                        \
private:                                                                                                \
	typedef META_NextEntry_##Name_

/**
 * Registers a new entry in a parameter block. Must be called in between BS_PARAM_BLOCK_BEGIN and BS_PARAM_BLOCK_END calls.
 */
#define BS_PARAM_BLOCK_ENTRY(Type, Name) BS_PARAM_BLOCK_ENTRY_ARRAY(Type, Name, 1)

/** Ends parameter block definition. See BS_PARAM_BLOCK_BEGIN. */
#define BS_PARAM_BLOCK_END                                                      \
	META_LastEntry;                                                             \
                                                                                \
	static Vector<GpuParamDataDesc> GetEntries()                                \
	{                                                                           \
		Vector<GpuParamDataDesc> entries;                                       \
		META_GetPrevEntries(entries, META_LastEntry());                         \
		return entries;                                                         \
	}                                                                           \
                                                                                \
	void InitEntries()                                                          \
	{                                                                           \
		META_InitPrevEntry(mParams, (u32)mParams.size() - 1, META_LastEntry()); \
	}                                                                           \
                                                                                \
	Vector<GpuParamDataDesc> mParams;                                           \
	u32 mBlockSize;                                                             \
	}                                                                           \
	;

		/** @} */
	} // namespace ct
} // namespace bs
