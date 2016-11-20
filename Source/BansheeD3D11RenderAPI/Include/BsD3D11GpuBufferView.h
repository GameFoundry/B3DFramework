//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"

namespace bs
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**
	 * Descriptor structure used for initializing a GpuBufferView.
	 *
	 * @see		GpuBuffer
	 */
	struct BS_CORE_EXPORT GPU_BUFFER_VIEW_DESC
	{
		UINT32 firstElement;
		UINT32 elementWidth;
		UINT32 numElements;
		bool useCounter;
		GpuViewUsage usage;
		GpuBufferFormat format;
	};

	/**
	 * Represents a specific view of a GpuBuffer. Different views all of the same buffer be used in different situations
	 * (for example for reading from a shader, or for a unordered read/write operation).
	 */
	class GpuBufferView
	{
	public:
		class HashFunction
		{
		public:
			size_t operator()(const GPU_BUFFER_VIEW_DESC& key) const;
		};

		class EqualFunction
		{
		public:
			bool operator()(const GPU_BUFFER_VIEW_DESC& a, const GPU_BUFFER_VIEW_DESC& b) const;
		};

		GpuBufferView();
		virtual ~GpuBufferView();

		/** 
		 * Initializes a new buffer view for the specified buffer. Descriptor structure defines which portion of the buffer,
		 * and how will its contents be represented by the view.
		 */
		void initialize(const SPtr<D3D11GpuBufferCore>& buffer, GPU_BUFFER_VIEW_DESC& desc);

		/** Returns a descriptor structure used for creating the view. */
		const GPU_BUFFER_VIEW_DESC& getDesc() const { return mDesc; }

		/**	Returns the buffer this view was created for. */
		SPtr<D3D11GpuBufferCore> getBuffer() const { return mBuffer; }

		/** Returns index of first element in the buffer that this view provides access to. */
		UINT32 getFirstElement() const { return mDesc.firstElement; }

		/** Returns width of an element in the buffer, in bytes. */
		UINT32 getElementWidth() const { return mDesc.elementWidth; }

		/**	Returns the total number of elements this buffer provides access to. */
		UINT32 getNumElements() const { return mDesc.numElements; }

		/**	Returns true if this view allows a GPU program to use counters on the bound buffer. */
		bool getUseCounter() const { return mDesc.useCounter; }

		/** Returns view usage which determines where in the pipeline can the view be bound. */
		GpuViewUsage getUsage() const { return mDesc.usage; }

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

		GPU_BUFFER_VIEW_DESC mDesc;
		SPtr<D3D11GpuBufferCore> mBuffer;
	};

	/** @} */
}