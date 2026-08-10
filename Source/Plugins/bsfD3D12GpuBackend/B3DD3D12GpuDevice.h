			enum class MemoryPoolType : u32
			{
				DefaultBuffer,
				DefaultTexture,
				DefaultMsaaTexture,
				UploadBuffer,
				ReadbackBuffer,
				Count
			};

			using GpuMemoryAllocator = TGpuTlsfAllocator<D3D12HeapBackend>;

			/** Returns the heap pool compatible with the specified resource, or Count when unsupported. */
			static MemoryPoolType GetMemoryPoolType(const D3D12_RESOURCE_DESC& resourceDesc, D3D12_HEAP_TYPE heapType);

			/** Lazily creates and returns the allocator for a compatible D3D12 heap class. */
			GpuMemoryAllocator& GetOrCreateGpuMemoryAllocator(MemoryPoolType poolType);
