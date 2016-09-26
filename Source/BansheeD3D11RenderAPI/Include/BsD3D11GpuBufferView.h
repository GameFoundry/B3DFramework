//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsGpuBufferView.h"

namespace BansheeEngine
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**
	 * Represents a specific view of a GpuBuffer. Different views all of the same buffer be used in different situations
	 * (for example for reading from a shader, or for a unordered read/write operation).
	 */
	class BS_D3D11_EXPORT D3D11GpuBufferView : public GpuBufferView
	{
	public:
		D3D11GpuBufferView();
		virtual ~D3D11GpuBufferView();

		/** @copydoc GpuBufferView::initialize */
		void initialize(const SPtr<GpuBufferCore>& buffer, GPU_BUFFER_DESC& desc) override;

		/** Returns the DX11 shader resource view object for the buffer. */
		ID3D11ShaderResourceView* getSRV() const { return mSRV; }

		/** Returns the DX11 unordered access view object for the buffer. */
		ID3D11UnorderedAccessView* getUAV() const { return mUAV; }

	private:
		/**
		 * Creates a DX11 shader resource view that allows a buffer to be bound to a shader for reading (the most common
		 * option). 
		 *
		 * @param[in]	buffer			Buffer to create the view for.
		 * @param[in]	firstElement	Index of the first element the view manages. Interpretation of this value depends on
		 *								exact buffer type. It may be byte offset for raw buffers, or number of elements for
		 *								structured buffers.
		 * @param[in]	elementWidth	Width of a single element in the buffer. Size of the structure in structured buffers
		 *								and ignored for raw buffers as they always operate on single byte basis.
		 * @param[in]	numElements		Number of elements the view manages, starting after the "firstElement". This means
		 *								number of bytes for raw buffers or number of structures for structured buffers.
		 * @return						Constructed DX11 shader resource view object.
		 */
		ID3D11ShaderResourceView* createSRV(D3D11GpuBufferCore* buffer, UINT32 firstElement, UINT32 elementWidth, UINT32 numElements);

		/**
		 * Creates a DX11 unordered access view that allows a buffer to be bound to a shader for random reading or writing.
		 *
		 * @param[in]	buffer			Buffer to create the view for.
		 * @param[in]	firstElement	Index of the first element the view manages. Interpretation of this value depends on
		 *								exact buffer type. It may be byte offset for raw buffers, or number of elements for
		 *								structured buffers.
		 * @param[in]	numElements		Number of elements the view manages, starting after the @p firstElement. This means
		 *								number of bytes for raw buffers or number of structures for structured buffers.
		 * @return						Constructed DX11 unordered access view object.
		 */
		ID3D11UnorderedAccessView* createUAV(D3D11GpuBufferCore* buffer, UINT32 firstElement, UINT32 numElements, bool useCounter);

		ID3D11ShaderResourceView* mSRV;
		ID3D11UnorderedAccessView* mUAV;
	};

	/** @} */
}