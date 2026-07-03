//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12BarrierUtility.h"

using namespace b3d;
using namespace b3d::render;

D3D12_RESOURCE_STATES D3D12BarrierUtility::GetResourceState(GpuResourceUseFlags usage, GpuAccessFlags access, bool isTexture)
{
	const bool isWrite = access.IsSet(GpuAccessFlag::Write);

	// Attachment usages take priority: they are exclusive write/read-target states that cannot be combined
	// with the read-only states below.
	if(usage.IsSet(GpuResourceUseFlag::ColorAttachment))
		return D3D12_RESOURCE_STATE_RENDER_TARGET;

	if(usage.IsSet(GpuResourceUseFlag::DepthStencilAttachment))
		return isWrite ? D3D12_RESOURCE_STATE_DEPTH_WRITE : D3D12_RESOURCE_STATE_DEPTH_READ;

	// Transfer maps to the D3D12 copy states. Host readback of a GPU-written resource is serviced through a copy
	// into a READBACK-heap buffer, so a Host destination behaves like a copy source.
	if(usage.IsSet(GpuResourceUseFlag::Transfer))
		return isWrite ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_COPY_SOURCE;

	if(usage.IsSet(GpuResourceUseFlag::Host))
		return D3D12_RESOURCE_STATE_COPY_SOURCE;

	// Combinable read states are OR-ed together so a single barrier can express e.g. index + vertex buffer use.
	D3D12_RESOURCE_STATES state = (D3D12_RESOURCE_STATES)0;

	if(usage.IsSet(GpuResourceUseFlag::ShaderAccess))
	{
		if(isWrite)
		{
			// Unordered access (storage image / RWBuffer). Write access forces UAV regardless of stage.
			state |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		}
		else
		{
			// Sampled/SRV read. Split between pixel and non-pixel shader visibility based on the stage flags.
			// When no stage is provided we conservatively expose it to all stages.
			const bool fragmentStage = usage.IsSet(GpuResourceUseFlag::StageFragmentShader);
			const bool nonFragmentStage = usage.IsSet(GpuResourceUseFlag::StageVertexShader) || usage.IsSet(GpuResourceUseFlag::StageComputeShader);
			const bool anyStageSpecified = fragmentStage || nonFragmentStage;

			if(fragmentStage || !anyStageSpecified)
				state |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

			if(nonFragmentStage || !anyStageSpecified)
				state |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		}
	}

	// Buffer-only read states.
	if(!isTexture)
	{
		if(usage.IsSet(GpuResourceUseFlag::IndexBuffer))
			state |= D3D12_RESOURCE_STATE_INDEX_BUFFER;

		if(usage.IsSet(GpuResourceUseFlag::VertexBuffer) || usage.IsSet(GpuResourceUseFlag::UniformBuffer))
			state |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	}

	if(state == (D3D12_RESOURCE_STATES)0)
		return D3D12_RESOURCE_STATE_COMMON;

	return state;
}

D3D12_RESOURCE_STATES D3D12BarrierUtility::GetResourceStateFromLayout(GpuImageLayout layout, GpuAccessFlags access)
{
	const bool isWrite = access.IsSet(GpuAccessFlag::Write);

	switch(layout)
	{
	case GpuImageLayout::ColorAttachment:
		return D3D12_RESOURCE_STATE_RENDER_TARGET;
	case GpuImageLayout::DepthStencilAttachment:
	case GpuImageLayout::DepthReadOnlyStencilAttachment:
	case GpuImageLayout::DepthAttachmentStencilReadOnly:
		return D3D12_RESOURCE_STATE_DEPTH_WRITE;
	case GpuImageLayout::DepthStencilReadOnly:
		return D3D12_RESOURCE_STATE_DEPTH_READ;
	case GpuImageLayout::ShaderReadOnly:
		return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	case GpuImageLayout::TransferSource:
		return D3D12_RESOURCE_STATE_COPY_SOURCE;
	case GpuImageLayout::TransferDestination:
		return D3D12_RESOURCE_STATE_COPY_DEST;
	case GpuImageLayout::Present:
		return D3D12_RESOURCE_STATE_PRESENT; // Equal to COMMON (value 0)
	case GpuImageLayout::General:
		// Storage/general access maps to UAV for writes; a general read has no dedicated D3D12 state so it stays COMMON.
		return isWrite ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS : D3D12_RESOURCE_STATE_COMMON;
	case GpuImageLayout::Undefined:
	default:
		return D3D12_RESOURCE_STATE_COMMON;
	}
}

bool D3D12BarrierUtility::IsReadOnlyState(D3D12_RESOURCE_STATES state)
{
	// COMMON (0) and PRESENT are read-combinable. Any of the exclusive write/target states disqualifies it.
	const D3D12_RESOURCE_STATES writeStates =
		D3D12_RESOURCE_STATE_RENDER_TARGET |
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS |
		D3D12_RESOURCE_STATE_DEPTH_WRITE |
		D3D12_RESOURCE_STATE_COPY_DEST |
		D3D12_RESOURCE_STATE_STREAM_OUT;

	return (state & writeStates) == 0;
}
