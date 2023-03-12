//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "CoreThread/BsCoreObject.h"
#include "RenderAPI/BsGpuBuffer.h"

namespace bs
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/** Descriptor structure used for initialization of a GenericGpuBuffer. */
	struct GenericGpuBufferCreateInformation
{
		/** Number of elements in the buffer. */
		u32 ElementCount = 0;

		/**
		 * Size of each individual element in the buffer, in bytes. Only needed if using non-standard buffer. If using
		 * standard buffers element size is calculated from format and this must be zero.
		 */
		u32 ElementSize = 0;

		/** Type of the buffer. Determines how is buffer seen by the GPU program and in what ways can it be used. */
		GenericGpuBufferType Type = GBT_STANDARD;

		/** Format if the data in the buffer. Only relevant for standard buffers, must be BF_UNKNOWN otherwise. */
		GpuBufferFormat Format = BF_32X4F;

		/** Flags that control the behavior of the buffer. */
		GpuBufferFlags Flags = GpuBufferFlag::StoreOnGPU;
	};

	/**
	 * Information about a GenericGpuBuffer. Allows core and non-core versions of GenericGpuBuffer to share the same structure for
	 * properties.
	 */
	class B3D_CORE_EXPORT GenericGpuBufferProperties
	{
	public:
		GenericGpuBufferProperties(const GenericGpuBufferCreateInformation& desc);

		/**
		 * Returns the type of the GPU buffer. Type determines which kind of views (if any) can be created for the buffer,
		 * and how is data read or modified in it.
		 */
		GenericGpuBufferType GetType() const { return mDesc.Type; }

		/** Returns format used by the buffer. Only relevant for standard buffers. */
		GpuBufferFormat GetFormat() const { return mDesc.Format; }

		/** Returns buffer usage which determines how are planning on updating the buffer contents. */
		GpuBufferFlags GetFlags() const { return mDesc.Flags; }

		/**	Returns number of elements in the buffer. */
		u32 GetElementCount() const { return mDesc.ElementCount; }

		/**	Returns size of a single element in the buffer in bytes. */
		u32 GetElementSize() const { return mDesc.ElementSize; }

	protected:
		friend class GenericGpuBuffer;

		GenericGpuBufferCreateInformation mDesc;
	};

	/**
	 * Handles a generic GPU buffer that you may use for storing any kind of sequential data you wish to be accessible to
	 * the GPU.
	 *
	 * @note	Sim thread only.
	 */
	class B3D_CORE_EXPORT GenericGpuBuffer : public CoreObject
	{
	public:
		virtual ~GenericGpuBuffer() = default;

		/** Returns properties describing the buffer. */
		const GenericGpuBufferProperties& GetProperties() const { return mProperties; }

		/** Retrieves a core implementation of a GPU buffer usable only from the core thread. */
		SPtr<ct::GenericGpuBuffer> GetCore() const;

		/** Returns the size of a single element in the buffer, of the provided format, in bytes. */
		static u32 GetFormatSize(GpuBufferFormat format);

		/** @copydoc HardwareBufferManager::CreateGpuBuffer */
		static SPtr<GenericGpuBuffer> Create(const GenericGpuBufferCreateInformation& desc);

	protected:
		friend class HardwareBufferManager;

		GenericGpuBuffer(const GenericGpuBufferCreateInformation& desc);

		SPtr<ct::CoreObject> CreateCore() const override;

		GenericGpuBufferProperties mProperties;
	};

	/** @} */

	namespace ct
	{
		/** @addtogroup RenderAPI-Internal
		 *  @{
		 */

		/**
		 * Core thread version of a bs::GenericGpuBuffer.
		 *
		 * @note	Core thread only.
		 */
		class B3D_CORE_EXPORT GenericGpuBuffer : public CoreObject, public GpuBuffer
	{
		public:
			virtual ~GenericGpuBuffer();

			/** Returns properties describing the buffer. */
			const GenericGpuBufferProperties& GetProperties() const { return mProperties; }

			void ReadData(u32 offset, u32 length, void* dest, u32 deviceIdx = 0, u32 queueIdx = 0) override;
			void WriteData(u32 offset, u32 length, const void* source, BufferWriteType writeFlags = BWT_NORMAL, u32 queueIdx = 0) override;
			void CopyData(GpuBuffer& srcBuffer, u32 srcOffset, u32 dstOffset, u32 length, bool discardWholeBuffer = false, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

			/**
			 * Returns a view of this buffer with specified format/type.
			 *
			 * @param[in]	type			Type of buffer to view the contents as. Only supported values are GBT_STANDARD and
			 *								GBT_STRUCTURED.
			 * @param[in]	format			Format of the data in the buffer. Size of the underlying buffer must be divisible by
			 *								the	size of an individual element of this format. Must be BF_UNKNOWN if buffer type
			 *								is GBT_STRUCTURED.
			 * @param[in]	elementSize		Size of the individual element in the buffer. Size of the underlying buffer must be
			 *								divisible by this size. Must be 0 if buffer type is GBT_STANDARD (element size gets
			 *								deduced from format).
			 * @return						New view of the buffer, using the provided format and type.
			 */
			SPtr<GenericGpuBuffer> GetView(GenericGpuBufferType type, GpuBufferFormat format, u32 elementSize = 0);

			/** @copydoc bs::HardwareBufferManager::CreateGpuBuffer */
			static SPtr<GenericGpuBuffer> Create(const GenericGpuBufferCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/**
			 * Creates a view of an existing hardware buffer. No internal buffer will be allocated and the provided buffer
			 * will be used for all internal operations instead. Information provided in @p desc (such as element size and
			 * count) must match the provided @p underlyingBuffer.
			 */
			static SPtr<GenericGpuBuffer> Create(const GenericGpuBufferCreateInformation& desc, SPtr<GpuBuffer> underlyingBuffer);

		protected:
			friend class HardwareBufferManager;

			GenericGpuBuffer(const GenericGpuBufferCreateInformation& desc, GpuDeviceFlags deviceMask);
			GenericGpuBuffer(const GenericGpuBufferCreateInformation& desc, SPtr<GpuBuffer> underlyingBuffer);

			void* Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx = 0, u32 queueIdx = 0) override;
			void Unmap() override;
			void Initialize() override;

			GenericGpuBufferProperties mProperties;

			GpuBuffer* mBuffer = nullptr;
			SPtr<GpuBuffer> mSharedBuffer;
			bool mIsExternalBuffer = false;

			typedef void (*Deleter)(GpuBuffer*);
			Deleter mBufferDeleter = nullptr;
		};

		/** @} */
	} // namespace ct
} // namespace bs
