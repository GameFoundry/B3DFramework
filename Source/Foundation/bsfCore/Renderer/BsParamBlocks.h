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

			ParamBlockParam(const GpuDataParameterInformation& paramDesc)
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

				const bool transposeMatrices = GetGpuDeviceCapabilities().Conventions.MatrixOrder == GpuBackendConventions::MatrixOrder::ColumnMajor;
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
			 * Writes parameter data to the provided memory location.
			 *
			 * @param	destination		Parameter block memory origin. The per-parameter offset is applied internally to determine the final destination.
			 * @param	value			Value to write to the memory.
			 * @param	arrayIndex		Optional array index, in case the parameter represents an array.
			 */
			void Set(u8* const destination, const T& value, u32 arrayIndex = 0) const
			{
#if B3D_DEBUG
				if(arrayIndex >= mParamDesc.ArraySize)
				{
					B3D_LOG(Error, RenderBackend, "Array index out of range. Array size: {0}. Requested size: {1}", mParamDesc.ArraySize, arrayIndex);
					return;
				}
#endif

				const u32 elementSizeBytes = mParamDesc.ElementSize * sizeof(u32);
				const u32 sizeBytes = std::min(elementSizeBytes, (u32)sizeof(T)); // Truncate if it doesn't fit within parameter size
				const u32 offset = (mParamDesc.CpuMemOffset + arrayIndex * mParamDesc.ArrayElementStride) * sizeof(u32);

				const bool transposeMatrices = GetGpuDeviceCapabilities().Conventions.MatrixOrder == Conventions::MatrixOrder::ColumnMajor;
				if(TransposePolicy<T>::TransposeEnabled(transposeMatrices))
				{
					auto transposed = TransposePolicy<T>::Transpose(value);
					memcpy(destination + offset, &transposed, sizeBytes);
				}
				else
				{
					memcpy(destination + offset, &value, sizeBytes);
				}

				// Set unused bytes to 0
				if(sizeBytes < elementSizeBytes)
				{
					const u32 diffSize = elementSizeBytes - sizeBytes;
					memset(destination + offset + sizeBytes, 0, diffSize);
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
			GpuDataParameterInformation mParamDesc;
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
 * B3D_PARAM_BLOCK_END.
 */
#define B3D_PARAM_BLOCK_BEGIN(Name)                                                                  \
	struct Name : ParamBlock                                                                         \
	{                                                                                                \
		Name()                                                                                       \
		{                                                                                            \
			ParamBlockManager::RegisterBlock(this);                                                  \
		}                                                                                            \
                                                                                                     \
		SPtr<GpuParamBlockBuffer> CreateBuffer(GpuBufferUsage usage = GBU_DYNAMIC) const             \
		{                                                                                            \
			return GpuParamBlockBuffer::Create(mBlockSize, usage);                                   \
		}                                                                                            \
																									 \
		u32 GetSize() const																			 \
		{																						     \
			return mBlockSize;																		 \
		}																							 \
                                                                                                     \
	private:                                                                                         \
		friend class ParamBlockManager;                                                              \
                                                                                                     \
		void Initialize() override                                                                   \
		{                                                                                            \
			mParams = GetEntries();                                                                  \
			RenderAPI& rapi = RenderAPI::Instance();                                                 \
                                                                                                     \
			GpuParameterBlockInformation blockInformation = rapi.GenerateParamBlockDesc(#Name, mParams);  \
			mBlockSize = blockInformation.BlockSize * sizeof(u32);                                   \
                                                                                                     \
			InitEntries();                                                                           \
		}                                                                                            \
                                                                                                     \
		struct META_FirstEntry                                                                       \
		{};                                                                                          \
		static void META_GetPrevEntries(Vector<GpuDataParameterInformation>& params, META_FirstEntry id)        \
		{}                                                                                           \
		void META_InitPrevEntry(const Vector<GpuDataParameterInformation>& params, u32 idx, META_FirstEntry id) \
		{}                                                                                           \
                                                                                                     \
		typedef META_FirstEntry

/**
 * Registers a new entry in a parameter block. Must be called in between B3D_PARAM_BLOCK_BEGIN and B3D_PARAM_BLOCK_END calls.
 */
#define B3D_PARAM_BLOCK_ENTRY_ARRAY(Type_, Name_, ElementCount)                                          \
	META_Entry_##Name_;                                                                                 \
                                                                                                        \
	struct META_NextEntry_##Name_                                                                       \
	{};                                                                                                 \
	static void META_GetPrevEntries(Vector<GpuDataParameterInformation>& params, META_NextEntry_##Name_ id)        \
	{                                                                                                   \
		META_GetPrevEntries(params, META_Entry_##Name_());                                              \
                                                                                                        \
		params.push_back(GpuDataParameterInformation());                                                \
		GpuDataParameterInformation& newEntry = params.back();                                          \
		newEntry.Name = #Name_;                                                                         \
		newEntry.Type = (GpuDataParameterType)TGpuDataParamInfo<Type_>::TypeId;                         \
		newEntry.ArraySize = ElementCount;                                                              \
		newEntry.ElementSize = sizeof(Type_);                                                           \
	}                                                                                                   \
                                                                                                        \
	void META_InitPrevEntry(const Vector<GpuDataParameterInformation>& params, u32 idx, META_NextEntry_##Name_ id) \
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
 * Registers a new entry in a parameter block. Must be called in between B3D_PARAM_BLOCK_BEGIN and B3D_PARAM_BLOCK_END calls.
 */
#define B3D_PARAM_BLOCK_ENTRY(Type, Name) B3D_PARAM_BLOCK_ENTRY_ARRAY(Type, Name, 1)

/** Ends parameter block definition. See B3D_PARAM_BLOCK_BEGIN. */
#define B3D_PARAM_BLOCK_END                                                     \
	META_LastEntry;                                                             \
                                                                                \
	static Vector<GpuDataParameterInformation> GetEntries()                     \
	{                                                                           \
		Vector<GpuDataParameterInformation> entries;                            \
		META_GetPrevEntries(entries, META_LastEntry());                         \
		return entries;                                                         \
	}                                                                           \
                                                                                \
	void InitEntries()                                                          \
	{                                                                           \
		META_InitPrevEntry(mParams, (u32)mParams.size() - 1, META_LastEntry()); \
	}                                                                           \
                                                                                \
	Vector<GpuDataParameterInformation> mParams;                                \
	u32 mBlockSize;                                                             \
	}                                                                           \
	;

		/** @} */
	} // namespace ct
} // namespace bs
