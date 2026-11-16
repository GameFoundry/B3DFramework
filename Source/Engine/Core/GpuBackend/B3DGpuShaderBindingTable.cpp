//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuShaderBindingTable.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "Utility/B3DBitwise.h"

using namespace b3d;
using namespace b3d::render;

GpuShaderBindingTable::GpuShaderBindingTable(const GpuShaderBindingTableCreateInformation& createInformation, const GpuRayTracingCapabilities& capabilities)
	: mInformation(createInformation), mShaderGroupHandleSize(capabilities.ShaderGroupHandleSize)
{
	// A record is the identifier of its group. Every region starts at a multiple of the table alignment. Ray generation records are that far
	// apart too, because a dispatch passes one of them as a region of its own.
	const u32 tableAlignment = capabilities.ShaderTableAlignment;
	const u32 recordStride = Bitwise::AlignUp(mShaderGroupHandleSize, capabilities.ShaderRecordAlignment);
	const u32 rayGenerationStride = Bitwise::AlignUp(mShaderGroupHandleSize, std::max(capabilities.ShaderRecordAlignment, tableAlignment));

	const auto fnPlaceRegion = [this, tableAlignment](const Vector<u32>& records, u32 stride, GpuShaderBindingTableRegion& region)
	{
		if(records.empty())
			return;

		mSize = Bitwise::AlignUp(mSize, tableAlignment);
		region.Offset = mSize;
		region.Stride = stride;
		region.Size = stride * (u32)records.size();
		mSize += region.Size;
	};

	fnPlaceRegion(mInformation.RayGenerationRecords, rayGenerationStride, mRayGenerationRegion);
	fnPlaceRegion(mInformation.MissRecords, recordStride, mMissRegion);
	fnPlaceRegion(mInformation.HitGroupRecords, recordStride, mHitGroupRegion);
	fnPlaceRegion(mInformation.CallableRecords, recordStride, mCallableRegion);
}

void GpuShaderBindingTable::WriteRecords(u8* destination, const Function<TArrayView<const u8>(u32)>& fnGetGroupIdentifier) const
{
	// Padding and empty hit group records stay zeros, which run no program
	std::memset(destination, 0, mSize);

	const auto fnWriteRecords = [this, destination, &fnGetGroupIdentifier](const Vector<u32>& records, const GpuShaderBindingTableRegion& region, u32 firstGroupIndex)
	{
		for(u32 recordIndex = 0; recordIndex < (u32)records.size(); recordIndex++)
		{
			if(records[recordIndex] == kGpuShaderBindingTableEmptyRecord)
				continue;

			const TArrayView<const u8> identifier = fnGetGroupIdentifier(firstGroupIndex + records[recordIndex]);
			B3D_ASSERT(identifier.size() == mShaderGroupHandleSize);

			std::memcpy(destination + region.Offset + recordIndex * region.Stride, identifier.data(), mShaderGroupHandleSize);
		}
	};

	const GpuRayTracingPipelineStateInformation& programs = mInformation.Pipeline->GetInformation();
	const u32 firstMissGroupIndex = (u32)programs.RayGenerationPrograms.size();
	const u32 firstHitGroupIndex = firstMissGroupIndex + (u32)programs.MissPrograms.size();
	const u32 firstCallableGroupIndex = firstHitGroupIndex + (u32)programs.HitGroups.size();
	fnWriteRecords(mInformation.RayGenerationRecords, mRayGenerationRegion, 0);
	fnWriteRecords(mInformation.MissRecords, mMissRegion, firstMissGroupIndex);
	fnWriteRecords(mInformation.HitGroupRecords, mHitGroupRegion, firstHitGroupIndex);
	fnWriteRecords(mInformation.CallableRecords, mCallableRegion, firstCallableGroupIndex);
}
