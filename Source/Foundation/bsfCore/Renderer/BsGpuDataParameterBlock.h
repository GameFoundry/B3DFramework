//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsCoreApplication.h"
#include "RenderAPI/BsGpuProgramParameterDescription.h"
#include "RenderAPI/BsGpuParameters.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "RenderAPI/BsGpuDevice.h"
#include "RenderAPI/BsGpuDeviceCapabilities.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Renderer-Internal
		 *  @{
		 */

		/** Wrapper for a single parameter in a parameter block buffer. */
		template <class T>
		class GpuDataParameterBlockElement
		{
		public:
			GpuDataParameterBlockElement() = default;

			GpuDataParameterBlockElement(const GpuDataParameterInformation& parameterInformation)
				: mParameterInformation(parameterInformation)
			{}

			/**
			 * Sets the parameter in the provided parameter block buffer. Caller is responsible for ensuring the param block
			 * buffer contains this parameter.
			 */
			void Set(const SPtr<GpuBuffer>& parameterBlockBuffer, const T& value, u32 arrayIdx = 0) const
			{
#if B3D_DEBUG
				if(arrayIdx >= mParameterInformation.ArraySize)
				{
					B3D_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " + ToString(mParameterInformation.ArraySize) + ". Requested size: " + ToString(arrayIdx));
				}
#endif

				const GpuDataParameterTypeInformation& typeInformation = bs::GpuParameters::kParamSizes.Lookup[mParameterInformation.Type];

				const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
				const GpuBackendConventions& gpuBackendConventions = gpuDevice->GetCapabilities().Conventions;

				const bool transposeMatrices = gpuBackendConventions.MatrixOrder == GpuBackendConventions::MatrixOrder::ColumnMajor;
				if(TransposePolicy<T>::TransposeEnabled(transposeMatrices))
				{
					auto transposed = TransposePolicy<T>::Transpose(value);
					parameterBlockBuffer->WriteCachedType((mParameterInformation.CpuOffset + arrayIdx * mParameterInformation.ArrayElementStride) * sizeof(u32), typeInformation, &transposed);
				}
				else
					parameterBlockBuffer->WriteCachedType((mParameterInformation.CpuOffset + arrayIdx * mParameterInformation.ArrayElementStride) * sizeof(u32), typeInformation, &value);
			}

			/**
			 * Gets the parameter in the provided parameter block buffer. Caller is responsible for ensuring the param block
			 * buffer contains this parameter.
			 */
			T Get(const SPtr<GpuBuffer>& parameterBlockBuffer, u32 arrayIdx = 0) const
			{
#if B3D_DEBUG
				if(arrayIdx >= mParameterInformation.ArraySize)
				{
					B3D_LOG(Error, Material, "Array index out of range. Array size: {0}. Requested size: {1}", mParameterInformation.ArraySize, arrayIdx);
					return T();
				}
#endif

				u32 elementSizeBytes = mParameterInformation.ElementSize * sizeof(u32);
				u32 sizeBytes = std::min(elementSizeBytes, (u32)sizeof(T));

				T value;
				parameterBlockBuffer->ReadCached((mParameterInformation.CpuOffset + arrayIdx * mParameterInformation.ArrayElementStride) * sizeof(u32), sizeBytes, &value);

				return value;
			}

		protected:
			GpuDataParameterInformation mParameterInformation;
		};

		/** Base class for all parameter blocks. */
		struct B3D_CORE_EXPORT GpuDataParameterBlock
		{
			virtual ~GpuDataParameterBlock();
			virtual void Initialize() = 0;
		};

		/**
		 * Takes care of initializing param block definitions in a delayed manner since they depend on engine systems yet
		 * are usually used as global variables which are initialized before engine systems are ready.
		 */
		class B3D_CORE_EXPORT GpuDataParameterBlockManager : public Module<GpuDataParameterBlockManager>
		{
		public:
			GpuDataParameterBlockManager();

			/** Registers a new param block, and initializes it when ready. */
			static void RegisterBlock(GpuDataParameterBlock* parameterBlock);

			/** Removes the param block from the initialization list. */
			static void UnregisterBlock(GpuDataParameterBlock* parameterBlock);

		private:
			static Vector<GpuDataParameterBlock*> sToInitialize;
		};

/**
 * Starts a new custom parameter block. Custom parameter blocks allow you to create C++ structures that map directly
 * to GPU program buffers (for example uniform buffer in OpenGL or constant buffer in DX). Must be followed by
 * B3D_PARAM_BLOCK_END.
 */
#define B3D_PARAM_BLOCK_BEGIN(Name)                                                                                                                          \
	struct Name : GpuDataParameterBlock                                                                                                                      \
	{                                                                                                                                                        \
		Name()                                                                                                                                               \
		{                                                                                                                                                    \
			GpuDataParameterBlockManager::RegisterBlock(this);                                                                                               \
		}                                                                                                                                                    \
                                                                                                                                                             \
		SPtr<GpuBuffer> CreateBuffer(GpuBufferFlags flags = GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::AllowWriteCachingOnCPU) const            \
		{                                                                                                                                                    \
			const SPtr<GpuDevice> gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();                                                                    \
			if(gpuDevice)                                                                                                                                    \
				return gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateUniform(mBlockSize, flags));                                             \
                                                                                                                                                             \
			return nullptr;                                                                                                                                  \
		}                                                                                                                                                    \
                                                                                                                                                             \
		SPtr<GpuBuffer> CreateBuffer(u32 count, GpuBufferFlags flags = GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::AllowWriteCachingOnCPU) const \
		{                                                                                                                                                    \
			const SPtr<GpuDevice> gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();                                                                    \
			if(gpuDevice)                                                                                                                                    \
				return gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateUniform(Math::CeilToMultiple(mBlockSize, gpuDevice->GetCapabilities().MinimumUniformBufferOffsetAlignment) * count, flags));                                     \
                                                                                                                                                             \
			return nullptr;                                                                                                                                  \
		}                                                                                                                                                    \
                                                                                                                                                             \
		u32 GetSize() const                                                                                                                                  \
		{                                                                                                                                                    \
			return mBlockSize;                                                                                                                               \
		}                                                                                                                                                    \
                                                                                                                                                             \
	private:                                                                                                                                                 \
		friend class ParamBlockManager;                                                                                                                      \
                                                                                                                                                             \
		void Initialize() override                                                                                                                           \
		{                                                                                                                                                    \
			mParams = GetEntries();                                                                                                                          \
			const SPtr<GpuDevice> gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();                                                                    \
			if(gpuDevice)                                                                                                                                    \
			{                                                                                                                                                \
                                                                                                                                                             \
				GpuDataParameterBlockInformation blockInformation = gpuDevice->GenerateUniformBlockInformation(#Name, mParams);                              \
				mBlockSize = blockInformation.BlockSize * sizeof(u32);                                                                                       \
			}                                                                                                                                                \
			else                                                                                                                                             \
			{                                                                                                                                                \
				mBlockSize = 0;                                                                                                                              \
			}                                                                                                                                                \
                                                                                                                                                             \
			InitEntries();                                                                                                                                   \
		}                                                                                                                                                    \
                                                                                                                                                             \
		struct META_FirstEntry                                                                                                                               \
		{};                                                                                                                                                  \
		static void META_GetPrevEntries(Vector<GpuDataParameterInformation>& params, META_FirstEntry id)                                                     \
		{}                                                                                                                                                   \
		void META_InitPrevEntry(const Vector<GpuDataParameterInformation>& params, u32 idx, META_FirstEntry id)                                              \
		{}                                                                                                                                                   \
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
		Name_ = GpuDataParameterBlockElement<Type_>(params[idx]);                                       \
	}                                                                                                   \
                                                                                                        \
public:                                                                                                 \
	GpuDataParameterBlockElement<Type_> Name_;                                                          \
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
