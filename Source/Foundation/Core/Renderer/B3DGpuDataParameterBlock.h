//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DApplication.h"
#include "RenderAPI/B3DGpuProgramParameterDescription.h"
#include "RenderAPI/B3DGpuParameters.h"
#include "RenderAPI/B3DGpuBuffer.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "RenderAPI/B3DGpuDeviceCapabilities.h"

namespace b3d
{
	namespace render
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

			GpuDataParameterBlockElement(const GpuUniformBufferMemberInformation& parameterInformation)
				: mParameterInformation(parameterInformation)
			{}

			/**
			 * Sets the parameter in the provided uniform buffer. Caller is responsible for ensuring the uniform buffer contains this parameter.
			 *
			 * @param uniformBuffer			Uniform buffer to set the parameter in.
			 * @param value					Value to set.
			 * @param arrayIndex			Index in the array to set the value for (if the parameter is an array).
			 * @param suballocationIndex	Index of the sub-allocation in the uniform buffer to set the value for, if the buffer contains multiple sub-allocated buffers.
			 */
			void Set(const SPtr<GpuBuffer>& uniformBuffer, const T& value, u32 arrayIndex = 0, u32 suballocationIndex = 0) const
			{
#if B3D_DEBUG
				if(!B3D_ENSURE(arrayIndex < mParameterInformation.ArraySize))
					return;

				if(!B3D_ENSURE(suballocationIndex < uniformBuffer->GetInformation().SuballocationCount))
					return;
#endif

				const u32 offset = CalculateSuballocationOffset(uniformBuffer, suballocationIndex, arrayIndex);
				const GpuDataParameterTypeInformation& typeInformation = b3d::GpuParameters::kParamSizes.Lookup[mParameterInformation.Type];

				const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
				const GpuBackendConventions& gpuBackendConventions = gpuDevice->GetCapabilities().Conventions;

				const bool transposeMatrices = gpuBackendConventions.MatrixOrder == GpuBackendConventions::MatrixOrder::ColumnMajor;
				if(TransposePolicy<T>::TransposeEnabled(transposeMatrices))
				{
					auto transposed = TransposePolicy<T>::Transpose(value);
					uniformBuffer->WriteCachedType(offset, typeInformation, &transposed);
				}
				else
					uniformBuffer->WriteCachedType(offset, typeInformation, &value);
			}

			/**
			 * Gets the parameter in the provided uniform buffer. Caller is responsible for ensuring the uniform buffer contains this parameter.
			 *
			 * @param uniformBuffer			Uniform buffer to get the parameter from.
			 * @param arrayIndex			Index in the array to get the value for (if the parameter is an array).
			 * @param suballocationIndex	Index of the sub-allocation in the uniform buffer to get the value for, if the buffer contains multiple sub-allocated buffers.
			 */
			T Get(const SPtr<GpuBuffer>& uniformBuffer, u32 arrayIndex = 0, u32 suballocationIndex = 0) const
			{
#if B3D_DEBUG
				if(!B3D_ENSURE(arrayIndex < mParameterInformation.ArraySize))
					return T();

				if(!B3D_ENSURE(suballocationIndex < uniformBuffer->GetInformation().SuballocationCount))
					return T();
#endif

				const u32 offset = CalculateSuballocationOffset(uniformBuffer, suballocationIndex, arrayIndex);
				u32 elementSizeBytes = mParameterInformation.ElementSize * sizeof(u32);
				u32 sizeBytes = std::min(elementSizeBytes, (u32)sizeof(T));

				T value;
				uniformBuffer->ReadCached(offset, sizeBytes, &value);

				return value;
			}

		protected:
			/**
			 * Calculates the byte offset for a parameter accounting for sub-allocations and array indices.
			 */
			u32 CalculateSuballocationOffset(const SPtr<GpuBuffer>& buffer, u32 suballocationIndex, u32 arrayIndex) const
			{
				// Calculate base parameter offset within a single uniform block
				const u32 parameterOffset = (mParameterInformation.CpuOffset + arrayIndex * mParameterInformation.ArrayElementStride) * sizeof(u32);

				// If buffer has sub-allocations, add sub-allocation stride
				const GpuBufferInformation& bufferInfo = buffer->GetInformation();

				const u32 suballocationStride = buffer->GetSuballocationSize();
				return suballocationIndex * suballocationStride + parameterOffset;
			}

			GpuUniformBufferMemberInformation mParameterInformation;
		};

		/** Base class for all parameter blocks. */
		struct B3D_EXPORT GpuDataParameterBlock
		{
			virtual ~GpuDataParameterBlock();
			virtual void Initialize() = 0;
		};

		/**
		 * Takes care of initializing param block definitions in a delayed manner since they depend on engine systems yet
		 * are usually used as global variables which are initialized before engine systems are ready.
		 */
		class B3D_EXPORT GpuDataParameterBlockManager : public Module<GpuDataParameterBlockManager>
		{
		public:
			GpuDataParameterBlockManager();

			/** Registers a new param block, and initializes it when ready. */
			static void RegisterBlock(GpuDataParameterBlock* parameterBlock);

			/** Removes the param block from the initialization list. */
			static void UnregisterBlock(GpuDataParameterBlock* parameterBlock);

		private:
			/** Retrieves the list of parameter blocks to initialize when the module is started. */
			static Vector<GpuDataParameterBlock*>& GetToInitializeList()
			{
				static Vector<GpuDataParameterBlock*> sToInitialize;
				return sToInitialize;
			}
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
			const SPtr<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();                                                                        \
			if(gpuDevice)                                                                                                                                    \
				return gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateUniform(mBlockSize, flags, 1));                                          \
                                                                                                                                                             \
			return nullptr;                                                                                                                                  \
		}                                                                                                                                                    \
                                                                                                                                                             \
		SPtr<GpuBuffer> CreateBuffer(u32 count, GpuBufferFlags flags = GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::AllowWriteCachingOnCPU) const \
		{                                                                                                                                                    \
			const SPtr<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();                                                                        \
			if(gpuDevice)                                                                                                                                    \
				return gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateUniform(mBlockSize, flags, count));                                      \
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
			const SPtr<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();                                                                        \
			if(gpuDevice)                                                                                                                                    \
			{                                                                                                                                                \
                                                                                                                                                             \
				GpuUniformBufferInformation bufferInformation = gpuDevice->GenerateUniformBlockInformation(#Name, mParams);                                  \
				mBlockSize = bufferInformation.BlockSize * sizeof(u32);                                                                                      \
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
		static void META_GetPrevEntries(Vector<GpuUniformBufferMemberInformation>& params, META_FirstEntry id)                                               \
		{}                                                                                                                                                   \
		void META_InitPrevEntry(const Vector<GpuUniformBufferMemberInformation>& params, u32 idx, META_FirstEntry id)                                        \
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
	static void META_GetPrevEntries(Vector<GpuUniformBufferMemberInformation>& params, META_NextEntry_##Name_ id)        \
	{                                                                                                   \
		META_GetPrevEntries(params, META_Entry_##Name_());                                              \
                                                                                                        \
		params.push_back(GpuUniformBufferMemberInformation());                                                \
		GpuUniformBufferMemberInformation& newEntry = params.back();                                          \
		newEntry.Name = #Name_;                                                                         \
		newEntry.Type = (GpuDataParameterType)TGpuDataParamInfo<Type_>::TypeId;                         \
		newEntry.ArraySize = ElementCount;                                                              \
		newEntry.ElementSize = sizeof(Type_);                                                           \
	}                                                                                                   \
                                                                                                        \
	void META_InitPrevEntry(const Vector<GpuUniformBufferMemberInformation>& params, u32 idx, META_NextEntry_##Name_ id) \
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
	static Vector<GpuUniformBufferMemberInformation> GetEntries()               \
	{                                                                           \
		Vector<GpuUniformBufferMemberInformation> entries;                      \
		META_GetPrevEntries(entries, META_LastEntry());                         \
		return entries;                                                         \
	}                                                                           \
                                                                                \
	void InitEntries()                                                          \
	{                                                                           \
		META_InitPrevEntry(mParams, (u32)mParams.size() - 1, META_LastEntry()); \
	}                                                                           \
                                                                                \
	Vector<GpuUniformBufferMemberInformation> mParams;                          \
	u32 mBlockSize;                                                             \
	}                                                                           \
	;

		/** @} */
	} // namespace render
} // namespace b3d
