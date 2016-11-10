//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsGpuBuffer.h"
#include "BsD3D11GpuBufferView.h"

namespace BansheeEngine 
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	DirectX 11 implementation of a generic GPU buffer. */
	class BS_D3D11_EXPORT D3D11GpuBufferCore : public GpuBufferCore
    {
    public:
		~D3D11GpuBufferCore();

		/** @copydoc GpuBufferCore::lock */
		void* lock(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 queueIdx = 1) override;

		/** @copydoc GpuBufferCore::unlock */
		void unlock() override;

		/** @copydoc GpuBufferCore::readData */
		void readData(UINT32 offset, UINT32 length, void* dest, UINT32 queueIdx = 1) override;

		/** @copydoc GpuBufferCore::writeData */
        void writeData(UINT32 offset, UINT32 length, const void* pSource,
			BufferWriteType writeFlags = BWT_NORMAL, UINT32 queueIdx = 1) override;

		/** @copydoc GpuBufferCore::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, 
			UINT32 dstOffset, UINT32 length, bool discardWholeBuffer = false, UINT32 queueIdx = 1) override;

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
		static GpuBufferView* requestView(const SPtr<D3D11GpuBufferCore>& buffer, UINT32 firstElement, 
			UINT32 numElements, GpuViewUsage usage);

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
		friend class D3D11HardwareBufferCoreManager;

		D3D11GpuBufferCore(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);

		/**	Destroys all buffer views regardless if their reference count is zero or not. */
		void clearBufferViews();

		/** @copydoc GpuBufferCore::initialize */
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

		D3D11HardwareBuffer* mBuffer;
		GpuBufferView* mBufferView;

		UnorderedMap<GPU_BUFFER_VIEW_DESC, GpuBufferReference*, GpuBufferView::HashFunction, GpuBufferView::EqualFunction> mBufferViews;
    };

	/** @} */
}
