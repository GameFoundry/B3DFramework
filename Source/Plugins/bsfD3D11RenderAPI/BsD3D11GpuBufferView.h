//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"

namespace bs
{
	namespace ct
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
			u32 FirstElement;
			u32 ElementWidth;
			u32 NumElements;
			bool UseCounter;
			GpuViewUsage Usage;
			GpuBufferFormat Format;
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

			GpuBufferView() = default;
			virtual ~GpuBufferView();

			/**
			 * Initializes a new buffer view for the specified buffer. Descriptor structure defines which portion of the buffer,
			 * and how will its contents be represented by the view.
			 */
			void Initialize(D3D11GpuBuffer* buffer, GPU_BUFFER_VIEW_DESC& desc);

			/** Returns a descriptor structure used for creating the view. */
			const GPU_BUFFER_VIEW_DESC& GetDesc() const { return mDesc; }

			/**	Returns the buffer this view was created for. */
			D3D11GpuBuffer* GetBuffer() const { return mBuffer; }

			/** Returns index of first element in the buffer that this view provides access to. */
			u32 GetFirstElement() const { return mDesc.FirstElement; }

			/** Returns width of an element in the buffer, in bytes. */
			u32 GetElementWidth() const { return mDesc.ElementWidth; }

			/**	Returns the total number of elements this buffer provides access to. */
			u32 GetNumElements() const { return mDesc.NumElements; }

			/**	Returns true if this view allows a GPU program to use counters on the bound buffer. */
			bool GetUseCounter() const { return mDesc.UseCounter; }

			/** Returns view usage which determines where in the pipeline can the view be bound. */
			GpuViewUsage GetUsage() const { return mDesc.Usage; }

			/** Returns the DX11 shader resource view object for the buffer. */
			ID3D11ShaderResourceView* GetSrv() const { return mSRV; }

			/** Returns the DX11 unordered access view object for the buffer. */
			ID3D11UnorderedAccessView* GetUav() const { return mUAV; }

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
			ID3D11ShaderResourceView* CreateSrv(D3D11GpuBuffer* buffer, u32 firstElement, u32 elementWidth, u32 numElements);

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
			ID3D11UnorderedAccessView* CreateUav(D3D11GpuBuffer* buffer, u32 firstElement, u32 numElements, bool useCounter);

			ID3D11ShaderResourceView* mSRV = nullptr;
			ID3D11UnorderedAccessView* mUAV = nullptr;

			GPU_BUFFER_VIEW_DESC mDesc;
			D3D11GpuBuffer* mBuffer;
		};

		/** @} */
	} // namespace ct
} // namespace bs
