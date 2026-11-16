//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuShaderBindingTable.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		class D3D12Buffer;

		/**
		 * DirectX 12 implementation of a shader binding table. Its records are the shader identifiers of its pipeline, in one buffer the CPU
		 * writes once at creation.
		 */
		class D3D12GpuShaderBindingTable : public GpuShaderBindingTable
		{
		public:
			/**
			 * Creates the buffer of the table and writes the records into it. GetBuffer() is null if the buffer couldn't be created.
			 *
			 * @param	device				Device that creates the table.
			 * @param	createInformation	Records of the table, which the device validated. Its pipeline must have a state object.
			 */
			D3D12GpuShaderBindingTable(D3D12GpuDevice& device, const GpuShaderBindingTableCreateInformation& createInformation);
			~D3D12GpuShaderBindingTable() override;

			/** Returns the buffer that holds the records. */
			D3D12Buffer* GetBuffer() const { return mBuffer; }

			/** Returns ray generation record @p recordIndex as the range DispatchRays() takes. */
			D3D12_GPU_VIRTUAL_ADDRESS_RANGE GetRayGenerationRecordRange(u32 recordIndex) const;

			/** Returns @p region of the table as the range DispatchRays() takes. Zeros if @p region is empty. */
			D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE GetAddressRange(const GpuShaderBindingTableRegion& region) const;

		private:
			D3D12Buffer* mBuffer = nullptr;
		};

		/** @} */
	} // namespace render
} // namespace b3d
