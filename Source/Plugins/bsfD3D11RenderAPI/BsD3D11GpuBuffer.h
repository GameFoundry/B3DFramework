//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "BsD3D11GpuBufferView.h"

namespace bs { namespace ct
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	DirectX 11 implementation of a generic GPU buffer. */
	class D3D11GpuBuffer : public GpuBuffer
	{
	public:
		~D3D11GpuBuffer();

		/** @copydoc GpuBuffer::lock */
		void* lock(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx = 0,
				   UINT32 queueIdx = 0) override;

		/** @copydoc GpuBuffer::unlock */
		void unlock() override;

		/** @copydoc GpuBuffer::readData */
		void readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx = 0, UINT32 queueIdx = 0) override;

		/** @copydoc GpuBuffer::writeData */
		void writeData(UINT32 offset, UINT32 length, const void* source,
			BufferWriteType writeFlags = BWT_NORMAL, UINT32 queueIdx = 0) override;

		/** @copydoc GpuBuffer::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, 
			bool discardWholeBuffer = false, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/**
		 * Creates a buffer view that may be used for binding a buffer to a slot in the pipeline. Views allow you to specify
		 * how is data in the buffer organized to make it easier for the pipeline to interpret.
		 *
		 * @param[in]	buffer			Buffer to create the view for.
		 * @param[in]	firstElement	Position of the first element visible by the view.
		 * @param[in]	numElements		Number of elements to bind to the view.
		 * @param[in]	usage			Determines type of the view we are creating, and which slots in the pipeline will
		 *								the view be bindable to.
		 *
		 * @note If a view with this exact parameters already exists, it will be returned and new one will not be created.
		 * @note Only Default and RandomWrite views are supported for this type of buffer. 
		 */
		// TODO Low Priority: Perhaps reflect usage flag limitation by having an enum with only the supported two options?
		static GpuBufferView* requestView(D3D11GpuBuffer* buffer, UINT32 firstElement, UINT32 numElements, 
			GpuViewUsage usage);

		/**
		 * Releases a view created with requestView. 
		 *
		 * @note	View will only truly get released once all references to it are released.
		 */
		static void releaseView(GpuBufferView* view);

		/**	Returns the internal DX11 GPU buffer object. */
		ID3D11Buffer* getDX11Buffer() const;

		/** Returns the DX11 shader resource view object for the buffer. */
		ID3D11ShaderResourceView* getSRV() const;

		/** Returns the DX11 unordered access view object for the buffer. */
		ID3D11UnorderedAccessView* getUAV() const;

	protected:
		friend class D3D11HardwareBufferManager;

		D3D11GpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);
		D3D11GpuBuffer(const GPU_BUFFER_DESC& desc, const SPtr<D3D11HardwareBuffer>& underlyingBuffer);

		/**	Destroys all buffer views regardless if their reference count is zero or not. */
		void clearBufferViews();

		/** @copydoc GpuBuffer::initialize */
		void initialize() override;

	private:
		/**	Helper class to help with reference counting for GPU buffer views. */
		struct GpuBufferReference
		{
			GpuBufferReference(GpuBufferView* _view)
				:view(_view), refCount(0)
			{ }

			GpuBufferView* view;
			UINT32 refCount;
		};

		D3D11HardwareBuffer* mBuffer = nullptr;
		GpuBufferView* mBufferView;

		UnorderedMap<GPU_BUFFER_VIEW_DESC, GpuBufferReference*, GpuBufferView::HashFunction, GpuBufferView::EqualFunction> mBufferViews;
	};

	/** @} */
}}
