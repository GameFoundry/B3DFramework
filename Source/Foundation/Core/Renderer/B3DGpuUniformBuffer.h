//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DApplication.h"
#include "RenderAPI/B3DGpuProgramParameterDescription.h"
#include "RenderAPI/B3DGpuParameterSet.h"
#include "RenderAPI/B3DGpuBuffer.h"
#include "RenderAPI/B3DGpuBufferPool.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "RenderAPI/B3DGpuDeviceCapabilities.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Renderer-Internal
		 *  @{
		 */

		/** Wrapper for a single member in an uniform buffer. */
		template <class T>
		class GpuUniformBufferMember
		{
		public:
			GpuUniformBufferMember() = default;

			GpuUniformBufferMember(const GpuUniformBufferMemberInformation& memberInformation)
				: mMemberInformation(memberInformation)
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
				if(!B3D_ENSURE(arrayIndex < mMemberInformation.ArraySize))
					return;

				if(!B3D_ENSURE(suballocationIndex < uniformBuffer->GetInformation().SuballocationCount))
					return;
#endif

				const u32 offset = CalculateSuballocationOffset(uniformBuffer, suballocationIndex, arrayIndex);
				const GpuDataParameterTypeInformation& typeInformation = b3d::GpuParameterSet::kParamSizes.Lookup[mMemberInformation.Type];

				const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
				const GpuBackendConventions& gpuBackendConventions = gpuDevice->GetCapabilities().Conventions;
				const bool isWriteCached = uniformBuffer->GetInformation().Flags.IsSet(GpuBufferFlag::AllowWriteCachingOnCPU);

				const bool transposeMatrices = gpuBackendConventions.MatrixOrder == GpuBackendConventions::MatrixOrder::ColumnMajor;
				if(TransposePolicy<T>::TransposeEnabled(transposeMatrices))
				{
					auto transposed = TransposePolicy<T>::Transpose(value);
	
					if(isWriteCached)
						uniformBuffer->WriteCachedType(offset, typeInformation, &transposed);
					else
						uniformBuffer->WriteTyped(offset, typeInformation, &transposed);
				}
				else
				{
					if(isWriteCached)
						uniformBuffer->WriteCachedType(offset, typeInformation, &value);
					else
						uniformBuffer->WriteTyped(offset, typeInformation, &value);
				}
			}

			/**
			 * Sets parameter value in the uniform buffer in the provided pool suballocation.
			 *
			 * @param suballocation		Buffer suballocation handle.
			 * @param value				Value to set.
			 * @param arrayIndex		Index in the array to set the value for (if the parameter is an array).
			 */
			void Set(const GpuBufferSuballocation& suballocation, const T& value, u32 arrayIndex = 0) const
			{
				B3D_ASSERT(suballocation.IsValid());

				// Delegate to existing method with extracted buffer and index
				Set(suballocation.GetBuffer(), value, arrayIndex, suballocation.GetSuballocationIndex());
			}

			/**
			 * Sets parameter value directly to mapped memory via a GpuMappedRegion.
			 *
			 * @param mappedRegion		Active mapping containing the mapped memory pointer of the buffer in which to set the value.
			 * @param value				Value to set.
			 * @param arrayIndex		Index in the array (if parameter is an array).
			 */
			void Set(const GpuBufferMappedScope& mappedRegion, const T& value, u32 arrayIndex = 0) const
			{
				B3D_ASSERT(mappedRegion.IsValid());

#if B3D_DEBUG
				if(!B3D_ENSURE(arrayIndex < mMemberInformation.ArraySize))
					return;
#endif

				const u32 parameterOffset = (mMemberInformation.CpuOffset + arrayIndex * mMemberInformation.ArrayElementStride) * sizeof(u32);
				const GpuDataParameterTypeInformation& typeInformation = b3d::GpuParameterSet::kParamSizes.Lookup[mMemberInformation.Type];

				const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
				const GpuBackendConventions& gpuBackendConventions = gpuDevice->GetCapabilities().Conventions;
				const bool transposeMatrices = gpuBackendConventions.MatrixOrder == GpuBackendConventions::MatrixOrder::ColumnMajor;

				u8* const destination = static_cast<u8*>(mappedRegion.GetMappedMemory()) + parameterOffset;

				if(TransposePolicy<T>::TransposeEnabled(transposeMatrices))
				{
					auto transposed = TransposePolicy<T>::Transpose(value);
					WriteTypedToMemory(destination, typeInformation, &transposed);
				}
				else
					WriteTypedToMemory(destination, typeInformation, &value);
			}

			/**
			 * Gets the parameter in the provided uniform buffer. Caller is responsible for ensuring the uniform buffer contains this parameter.
			 *
			 * @param mappedRegion			Active mapping containing the mapped memory pointer of the buffer from which to get the value.
			 * @param arrayIndex			Index in the array to get the value for (if the parameter is an array).
			 */
			T Get(const GpuBufferMappedScope& mappedRegion, u32 arrayIndex = 0) const
			{
#if B3D_DEBUG
				if(!B3D_ENSURE(arrayIndex < mMemberInformation.ArraySize))
					return T();
#endif

				const u32 parameterOffset = (mMemberInformation.CpuOffset + arrayIndex * mMemberInformation.ArrayElementStride) * sizeof(u32);
				const GpuDataParameterTypeInformation& typeInformation = b3d::GpuParameterSet::kParamSizes.Lookup[mMemberInformation.Type];

				const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
				const GpuBackendConventions& gpuBackendConventions = gpuDevice->GetCapabilities().Conventions;
				const bool transposeMatrices = gpuBackendConventions.MatrixOrder == GpuBackendConventions::MatrixOrder::ColumnMajor;

				const u8* const source = static_cast<const u8*>(mappedRegion.GetMappedMemory()) + parameterOffset;

				T value;
				ReadTypedFromMemory(source, typeInformation, &value);

				if(TransposePolicy<T>::TransposeEnabled(transposeMatrices))
					return TransposePolicy<T>::Transpose(value);

				return value;
			}

		protected:
			/** Calculates the byte offset for a parameter accounting for sub-allocations and array indices. */
			u32 CalculateSuballocationOffset(const SPtr<GpuBuffer>& buffer, u32 suballocationIndex, u32 arrayIndex) const
			{
				// Calculate base parameter offset within a single uniform block
				const u32 parameterOffset = (mMemberInformation.CpuOffset + arrayIndex * mMemberInformation.ArrayElementStride) * sizeof(u32);

				const u32 suballocationStride = buffer->GetSuballocationSize();
				return suballocationIndex * suballocationStride + parameterOffset;
			}

			/** Writes typed data to memory with proper alignment/padding. */
			static void WriteTypedToMemory(void* destination, const GpuDataParameterTypeInformation& typeInformation, const void* source)
			{
				const u8* sourceBytes = static_cast<const u8*>(source);
				u8* destinationBytes = static_cast<u8*>(destination);

				for(u32 row = 0; row < typeInformation.NumRows; ++row)
				{
					const u32 rowSize = typeInformation.NumColumns * typeInformation.BaseTypeSize;
					memcpy(destinationBytes, sourceBytes, rowSize);

					destinationBytes += typeInformation.Alignment;
					sourceBytes += rowSize;
				}
			}

			/** Reads typed data to memory with proper alignment/padding. */
			static void ReadTypedFromMemory(const void* source, const GpuDataParameterTypeInformation& typeInformation, void* destination)
			{
				const u8* sourceBytes = static_cast<const u8*>(source);
				u8* destinationBytes = static_cast<u8*>(destination);

				for(u32 row = 0; row < typeInformation.NumRows; ++row)
				{
					const u32 rowSize = typeInformation.NumColumns * typeInformation.BaseTypeSize;
					memcpy(destinationBytes, sourceBytes, rowSize);

					destinationBytes += rowSize;
					sourceBytes += typeInformation.Alignment;
				}
			}

			GpuUniformBufferMemberInformation mMemberInformation;
		};

		/** Base class for all uniform buffers. */
		struct B3D_EXPORT GpuUniformBuffer
		{
			virtual ~GpuUniformBuffer();
			virtual void Initialize() = 0;

			/** Returns the size of the uniform buffer, in bytes. */
			u32 GetSize() const { return mBufferSize; }

			/**
			 * Allocates a new buffer that can store all the members defined in this uniform buffer. For buffers that are modified every frame
			 * prefer using AllocateTransient() instead.
			 */
			SPtr<GpuBuffer> CreateBuffer(GpuBufferFlags flags = GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::AllowWriteCachingOnCPU) const
			{
				const SPtr<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();
				if(gpuDevice)
					return gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateUniform(mBufferSize, flags, 1));

				return nullptr;
			}

			/**
			 * Allocates a new buffer that can store all the members defined in this uniform buffer. The buffer will have multiple sub-allocations, so
			 * it may store multiple instances of the data (e.g. one for each different objects). For buffers that are modified every frame
			 * prefer using AllocateTransient() instead.
			 */
			SPtr<GpuBuffer> CreateBuffer(u32 count, GpuBufferFlags flags = GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::AllowWriteCachingOnCPU) const
			{
				const SPtr<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();
				if(gpuDevice)
					return gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateUniform(mBufferSize, flags, count));

				return nullptr;
			}

			/**
			 * Allocates a transient uniform buffer suballocation from the internal pool. The transient allocation will remain valid
			 * for RenderThread::kMaximumFramesInFlight frames, after which it may be reused for other allocations. You should use
			 * this for uniform buffers that are updated every frame.
			 *
			 * Note that transient buffer allocations might be larger than size returned by GetSize(), due to alignment requirements.
			 */
			GpuBufferSuballocation AllocateTransient()
			{
				return mTransientAllocationPool.Allocate();
			}

		protected:
			friend class GpuUniformBufferManager;

			u32 mBufferSize;
			TArray<GpuUniformBufferMemberInformation> mMembers;

			TransientGpuBufferPool mTransientAllocationPool;
		};

		/**
		 * Takes care of initializing uniform buffers definitions in a delayed manner since they depend on engine systems yet
		 * are usually used as global variables which are initialized before engine systems are ready.
		 */
		class B3D_EXPORT GpuUniformBufferManager : public Module<GpuUniformBufferManager>
		{
		public:
			GpuUniformBufferManager();

			/** Notifies the uniform buffer pools that a new frame has begun. */
			void AdvanceFrame();

			/** Removes the uniform buffer from the active buffer list. */
			void UnregisterBuffer(GpuUniformBuffer* buffer);

			/** Registers a new uniform buffer, and initializes it when ready. */
			static void RegisterBuffer(GpuUniformBuffer* buffer);

		private:
			/** Retrieves the list of parameter blocks to initialize when the module is started. */
			static TArray<GpuUniformBuffer*>& GetToInitializeList()
			{
				static TArray<GpuUniformBuffer*> sToInitialize;
				return sToInitialize;
			}

			TArray<GpuUniformBuffer*> mActiveBuffers;
		};

/**
 * Starts a new uniform buffer definition. These definitions allow you to create C++ structures that map directly
 * to GPU uniform buffers. Must be followed by B3D_UNIFORM_BUFFER_END.
 */
#define B3D_UNIFORM_BUFFER_BEGIN(Name)                                                                                                                                                                        \
	struct Name : GpuUniformBuffer                                                                                                                                                                         \
	{                                                                                                                                                                                                      \
		Name()                                                                                                                                                                                             \
		{                                                                                                                                                                                                  \
			GpuUniformBufferManager::RegisterBuffer(this);                                                                                                                                                 \
		}                                                                                                                                                                                                  \
                                                                                                                                                                                                           \
	private:                                                                                                                                                                                               \
		friend class GpuUniformBufferManager;                                                                                                                                                              \
                                                                                                                                                                                                           \
		void Initialize() override                                                                                                                                                                         \
		{                                                                                                                                                                                                  \
			mMembers = GetEntries();                                                                                                                                                                       \
			const SPtr<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();                                                                                                                      \
			if(gpuDevice)                                                                                                                                                                                  \
			{                                                                                                                                                                                              \
				GpuUniformBufferInformation bufferInformation = gpuDevice->GenerateUniformBufferInformation(#Name, mMembers);                                                                              \
				mBufferSize = bufferInformation.Size * sizeof(u32);                                                                                                                                        \
			}                                                                                                                                                                                              \
			else                                                                                                                                                                                           \
			{                                                                                                                                                                                              \
				mBufferSize = 0;                                                                                                                                                                           \
			}                                                                                                                                                                                              \
                                                                                                                                                                                                           \
			mTransientAllocationPool.Initialize(*gpuDevice, GpuBufferCreateInformation::CreateUniform(mBufferSize, GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::AllowWriteCachingOnCPU), 4);    \
                                                                                                                                                                                                           \
			InitEntries();                                                                                                                                                                                 \
		}                                                                                                                                                                                                  \
                                                                                                                                                                                                           \
		struct META_FirstEntry                                                                                                                                                                             \
		{};                                                                                                                                                                                                \
		static void META_GetPrevEntries(TArray<GpuUniformBufferMemberInformation>& members, META_FirstEntry id)                                                                                            \
		{}                                                                                                                                                                                                 \
		void META_InitPrevEntry(const TArray<GpuUniformBufferMemberInformation>& members, u32 idx, META_FirstEntry id)                                                                                     \
		{}                                                                                                                                                                                                 \
                                                                                                                                                                                                           \
		typedef META_FirstEntry

/**
 * Registers a new entry in a uniform buffer. Must be called in between B3D_UNIFORM_BUFFER_BEGIN and B3D_UNIFORM_BUFFER_END calls.
 */
#define B3D_UNIFORM_BUFFER_MEMBER_ARRAY(Type_, Name_, ElementCount)                                                             \
	META_Entry_##Name_;                                                                                                     \
                                                                                                                            \
	struct META_NextEntry_##Name_                                                                                           \
	{};                                                                                                                     \
	static void META_GetPrevEntries(TArray<GpuUniformBufferMemberInformation>& members, META_NextEntry_##Name_ id)          \
	{                                                                                                                       \
		META_GetPrevEntries(members, META_Entry_##Name_());                                                                 \
                                                                                                                            \
		members.Add(GpuUniformBufferMemberInformation());                                                                   \
		GpuUniformBufferMemberInformation& newEntry = members.back();                                                       \
		newEntry.Name = #Name_;                                                                                             \
		newEntry.Type = (GpuDataParameterType)TGpuDataParamInfo<Type_>::TypeId;                                             \
		newEntry.ArraySize = ElementCount;                                                                                  \
		newEntry.ElementSize = sizeof(Type_);                                                                               \
	}                                                                                                                       \
                                                                                                                            \
	void META_InitPrevEntry(const TArray<GpuUniformBufferMemberInformation>& members, u32 index, META_NextEntry_##Name_ id) \
	{                                                                                                                       \
		META_InitPrevEntry(members, index - 1, META_Entry_##Name_());                                                       \
		Name_ = GpuUniformBufferMember<Type_>(members[index]);                                                              \
	}                                                                                                                       \
                                                                                                                            \
public:                                                                                                                     \
	GpuUniformBufferMember<Type_> Name_;                                                                                    \
                                                                                                                            \
private:                                                                                                                    \
	typedef META_NextEntry_##Name_

/**
 * Registers a new entry in a parameter block. Must be called in between B3D_UNIFORM_BUFFER_BEGIN and B3D_UNIFORM_BUFFER_END calls.
 */
#define B3D_UNIFORM_BUFFER_MEMBER(Type, Name) B3D_UNIFORM_BUFFER_MEMBER_ARRAY(Type, Name, 1)

/** Ends uniform buffer definition. See B3D_UNIFORM_BUFFER_BEGIN. */
#define B3D_UNIFORM_BUFFER_END                                                       \
	META_LastEntry;                                                               \
                                                                                  \
	static TArray<GpuUniformBufferMemberInformation> GetEntries()                 \
	{                                                                             \
		TArray<GpuUniformBufferMemberInformation> entries;                        \
		META_GetPrevEntries(entries, META_LastEntry());                           \
		return entries;                                                           \
	}                                                                             \
                                                                                  \
	void InitEntries()                                                            \
	{                                                                             \
		META_InitPrevEntry(mMembers, (u32)mMembers.size() - 1, META_LastEntry()); \
	}                                                                             \
	}                                                                             \
	;

		/** @} */
	} // namespace render
} // namespace b3d
