//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"
#include "GpuBackend/B3DGpuProgram.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/** DirectX 12 implementation of GPU pipeline parameter layout. */
		class D3D12GpuPipelineParameterLayout : public GpuPipelineParameterLayout
		{
		public:
			D3D12GpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation, D3D12GpuDevice& device);
			~D3D12GpuPipelineParameterLayout() override;

			/** Returns the D3D12 root signature. */
			ID3D12RootSignature* GetRootSignature() const { return mRootSignature.Get(); }

			/**
			 * Returns the index of the first root parameter belonging to the given set. Root parameters are numbered
			 * flat across all sets (see CreateRootSignature()), while each D3D12GpuParameters computes indices local to
			 * its own set - this base offset bridges the two at descriptor-table bind time.
			 */
			u32 GetSetRootParameterOffset(u32 setIndex) const { return mSetRootParameterOffsets[setIndex]; }

			/**
			 * Returns the slot-keyed union of every stage's reflected resource bindings for the given set, sorted by
			 * slot. The D3D shader compiler reflects resource usage per stage (stripping unused declarations), so each
			 * stage's table is a subset of the set's resources; the union describes the full set. Empty when no stage
			 * binds resources in the set.
			 */
			TArrayView<const GpuDescriptorTableEntry> GetReflectedSetEntries(u32 setIndex) const { return mReflectedSetEntries[setIndex]; }

		private:
			/** Creates the D3D12 root signature. */
			void CreateRootSignature();

			/** Populates mReflectedSetEntries by merging the per-stage reflected resource tables by slot. */
			void MergeReflectedSetTables(const GpuPipelineParameterLayoutCreateInformation& createInformation);

			D3D12GpuDevice& mDevice;
			ComPtr<ID3D12RootSignature> mRootSignature;
			Vector<u32> mSetRootParameterOffsets; /**< First root parameter index per set. */
			Vector<Vector<GpuDescriptorTableEntry>> mReflectedSetEntries; /**< Per-set slot-keyed union of stage-reflected bindings. */
		};

		/** @} */
	} // namespace render
} // namespace b3d
