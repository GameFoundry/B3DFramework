//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12BarrierUtility.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	bool IsDirectQueueLayout(D3D12_BARRIER_LAYOUT layout)
	{
		return layout >= D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COMMON && layout <= D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COPY_DEST;
	}

	bool IsComputeQueueLayout(D3D12_BARRIER_LAYOUT layout)
	{
		return layout >= D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COMMON && layout <= D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COPY_DEST;
	}

	D3D12_BARRIER_LAYOUT GetQueueSpecificLayout(GpuQueueType queueType, D3D12_BARRIER_LAYOUT directLayout, D3D12_BARRIER_LAYOUT computeLayout, D3D12_BARRIER_LAYOUT fallbackLayout)
	{
		if(queueType == GQT_GRAPHICS)
			return directLayout;

		if(queueType == GQT_COMPUTE)
			return computeLayout;

		return fallbackLayout;
	}

	bool IsLayoutSupportedOnQueue(D3D12_BARRIER_LAYOUT layout, GpuQueueType queueType)
	{
		if(IsDirectQueueLayout(layout))
			return queueType == GQT_GRAPHICS;

		if(IsComputeQueueLayout(layout))
			return queueType == GQT_COMPUTE;

		if(layout == D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ_COMPUTE_QUEUE_ACCESSIBLE)
			return queueType == GQT_GRAPHICS || queueType == GQT_COMPUTE;

		if(queueType == GQT_TRANSFER)
			return layout == D3D12_BARRIER_LAYOUT_COMMON;

		if(queueType == GQT_GRAPHICS)
			return true;

		return layout == D3D12_BARRIER_LAYOUT_UNDEFINED || layout == D3D12_BARRIER_LAYOUT_COMMON ||
			layout == D3D12_BARRIER_LAYOUT_COPY_SOURCE || layout == D3D12_BARRIER_LAYOUT_COPY_DEST ||
			layout == D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS || layout == D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
	}
}

D3D12_BARRIER_SYNC D3D12BarrierUtility::GetStageSync(GpuStageFlags stages)
{
	if(stages.IsSet(GpuStageFlag::All))
		return D3D12_BARRIER_SYNC_ALL;

	D3D12_BARRIER_SYNC sync = D3D12_BARRIER_SYNC_NONE;

	if(stages.IsSet(GpuStageFlag::DrawIndirect))
		sync |= D3D12_BARRIER_SYNC_EXECUTE_INDIRECT;
	if(stages.IsSet(GpuStageFlag::VertexInputAttributes))
		sync |= D3D12_BARRIER_SYNC_VERTEX_SHADING;
	if(stages.IsSet(GpuStageFlag::VertexInputIndices))
		sync |= D3D12_BARRIER_SYNC_INDEX_INPUT;
	if(stages.IsSetAny(GpuStageFlag::VertexShaderNonUniform | GpuStageFlag::VertexShaderUniform))
		sync |= D3D12_BARRIER_SYNC_VERTEX_SHADING;
	if(stages.IsSetAny(GpuStageFlag::FragmentShaderNonUniform | GpuStageFlag::FragmentShaderUniform))
		sync |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
	if(stages.IsSetAny(GpuStageFlag::ComputeShaderNonUniform | GpuStageFlag::ComputeShaderUniform))
		sync |= D3D12_BARRIER_SYNC_COMPUTE_SHADING;
	if(stages.IsSetAny(GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests))
		sync |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;
	if(stages.IsSet(GpuStageFlag::ColorAttachment))
		sync |= D3D12_BARRIER_SYNC_RENDER_TARGET;
	if(stages.IsSet(GpuStageFlag::Transfer))
		sync |= D3D12_BARRIER_SYNC_COPY;
	if(stages.IsSet(GpuStageFlag::Resolve))
		sync |= D3D12_BARRIER_SYNC_RESOLVE;
	if(stages.IsSet(GpuStageFlag::Host))
		sync |= D3D12_BARRIER_SYNC_ALL;

	return sync;
}

D3D12BarrierScope D3D12BarrierUtility::GetBufferScope(GpuStageFlags stages, GpuAccessFlags access, D3D12_RESOURCE_FLAGS resourceFlags)
{
	D3D12BarrierScope scope;
	scope.Sync = GetStageSync(stages);
	scope.Access = GetBufferAccess(stages, access, resourceFlags);

	return scope;
}

D3D12_BARRIER_ACCESS D3D12BarrierUtility::GetBufferAccess(GpuStageFlags stages, GpuAccessFlags access, D3D12_RESOURCE_FLAGS resourceFlags)
{
	if(!access.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
		return D3D12_BARRIER_ACCESS_NO_ACCESS;

	D3D12_BARRIER_ACCESS nativeAccess = D3D12_BARRIER_ACCESS_COMMON;
	const bool reads = access.IsSet(GpuAccessFlag::Read);
	const bool writes = access.IsSet(GpuAccessFlag::Write);

	if(stages.IsSet(GpuStageFlag::VertexInputAttributes) && reads)
		nativeAccess |= D3D12_BARRIER_ACCESS_VERTEX_BUFFER;

	if(stages.IsSet(GpuStageFlag::VertexInputIndices) && reads)
		nativeAccess |= D3D12_BARRIER_ACCESS_INDEX_BUFFER;

	if(stages.IsSetAny(GpuStageFlag::VertexShaderUniform | GpuStageFlag::FragmentShaderUniform | GpuStageFlag::ComputeShaderUniform) && reads)
		nativeAccess |= D3D12_BARRIER_ACCESS_CONSTANT_BUFFER;

	if(stages.IsSet(GpuStageFlag::DrawIndirect) && reads)
		nativeAccess |= D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT;

	if(stages.IsSetAny(GpuStageFlag::VertexShaderNonUniform | GpuStageFlag::FragmentShaderNonUniform | GpuStageFlag::ComputeShaderNonUniform))
		nativeAccess |= writes ? D3D12_BARRIER_ACCESS_UNORDERED_ACCESS : D3D12_BARRIER_ACCESS_SHADER_RESOURCE;

	if(stages.IsSetAny(GpuStageFlag::Transfer | GpuStageFlag::Host))
	{
		if(reads)
			nativeAccess |= D3D12_BARRIER_ACCESS_COPY_SOURCE;

		if(writes)
			nativeAccess |= D3D12_BARRIER_ACCESS_COPY_DEST;
	}

	// A logical scope can combine shader-reader stages with a write elsewhere; only UAV-capable pages may name UAV access.
	if((resourceFlags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) == 0)
		nativeAccess = (D3D12_BARRIER_ACCESS)(nativeAccess & ~D3D12_BARRIER_ACCESS_UNORDERED_ACCESS);

	return nativeAccess;
}

D3D12_BARRIER_ACCESS D3D12BarrierUtility::GetTextureAccess(GpuStageFlags stages, GpuAccessFlags access, GpuImageLayout layout, GpuTextureAspectFlags aspects)
{
	if(!access.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
		return D3D12_BARRIER_ACCESS_NO_ACCESS;

	const bool writes = access.IsSet(GpuAccessFlag::Write);
	switch(layout)
	{
	case GpuImageLayout::General:
		// GENERAL maps to the native UAV layout, whose compatible shader access is UAV for reads as well as writes.
		return D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
	case GpuImageLayout::ColorAttachment:
		return D3D12_BARRIER_ACCESS_RENDER_TARGET;
	case GpuImageLayout::DepthStencilAttachment:
		return writes ? D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE : D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
	case GpuImageLayout::DepthStencilReadOnly:
	case GpuImageLayout::DepthReadOnlyStencilAttachment:
	case GpuImageLayout::DepthAttachmentStencilReadOnly:
	{
		bool readOnlyAspect = layout == GpuImageLayout::DepthStencilReadOnly;
		if(!readOnlyAspect)
		{
			const bool depthOnly = aspects.IsSet(GpuTextureAspectFlag::Depth) && !aspects.IsSet(GpuTextureAspectFlag::Stencil);
			const bool stencilOnly = aspects.IsSet(GpuTextureAspectFlag::Stencil) && !aspects.IsSet(GpuTextureAspectFlag::Depth);
			readOnlyAspect = layout == GpuImageLayout::DepthReadOnlyStencilAttachment ? depthOnly : stencilOnly;
		}

		if(!readOnlyAspect)
			return D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;

		D3D12_BARRIER_ACCESS nativeAccess = D3D12_BARRIER_ACCESS_COMMON;
		if(stages.IsSetAny(GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests))
			nativeAccess |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;

		if(stages.IsSetAny(GpuStageFlag::VertexShaderNonUniform | GpuStageFlag::FragmentShaderNonUniform | GpuStageFlag::ComputeShaderNonUniform))
			nativeAccess |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;

		return nativeAccess != D3D12_BARRIER_ACCESS_COMMON ? nativeAccess : D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
	}
	case GpuImageLayout::ShaderReadOnly:
		return D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
	case GpuImageLayout::TransferSource:
		return D3D12_BARRIER_ACCESS_COPY_SOURCE;
	case GpuImageLayout::TransferDestination:
		return D3D12_BARRIER_ACCESS_COPY_DEST;
	case GpuImageLayout::ResolveSource:
		return D3D12_BARRIER_ACCESS_RESOLVE_SOURCE;
	case GpuImageLayout::ResolveDestination:
		return D3D12_BARRIER_ACCESS_RESOLVE_DEST;
	case GpuImageLayout::Present:
		return D3D12_BARRIER_ACCESS_COMMON;
	case GpuImageLayout::Undefined:
	default:
		return D3D12_BARRIER_ACCESS_NO_ACCESS;
	}
}

D3D12BarrierScope D3D12BarrierUtility::GetTextureScope(GpuStageFlags stages, GpuAccessFlags access, GpuImageLayout layout, GpuTextureAspectFlags aspects)
{
	D3D12BarrierScope scope;
	scope.Access = GetTextureAccess(stages, access, layout, aspects);
	scope.Sync = GetTextureSync(stages, scope.Access);
	return scope;
}

D3D12_BARRIER_SYNC D3D12BarrierUtility::GetTextureSync(GpuStageFlags stages, D3D12_BARRIER_ACCESS access)
{
	const D3D12_BARRIER_SYNC stageSync = GetStageSync(stages);
	if(access == D3D12_BARRIER_ACCESS_NO_ACCESS)
		return stageSync;

	if(stageSync == D3D12_BARRIER_SYNC_ALL)
		return D3D12_BARRIER_SYNC_ALL;

	if(stageSync == D3D12_BARRIER_SYNC_NONE)
	{
		B3D_ASSERT(false && "D3D12 texture access requires a synchronization stage.");
		return D3D12_BARRIER_SYNC_ALL;
	}

	if(access == D3D12_BARRIER_ACCESS_COMMON)
		return stageSync;

	D3D12_BARRIER_SYNC requiredSync = D3D12_BARRIER_SYNC_NONE;
	if((access & D3D12_BARRIER_ACCESS_RENDER_TARGET) != 0)
		requiredSync |= D3D12_BARRIER_SYNC_RENDER_TARGET;

	if((access & (D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ | D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE)) != 0)
		requiredSync |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;

	if((access & (D3D12_BARRIER_ACCESS_COPY_SOURCE | D3D12_BARRIER_ACCESS_COPY_DEST)) != 0)
		requiredSync |= D3D12_BARRIER_SYNC_COPY;

	if((access & (D3D12_BARRIER_ACCESS_RESOLVE_SOURCE | D3D12_BARRIER_ACCESS_RESOLVE_DEST)) != 0)
		requiredSync |= D3D12_BARRIER_SYNC_RESOLVE;

	if((access & (D3D12_BARRIER_ACCESS_SHADER_RESOURCE | D3D12_BARRIER_ACCESS_UNORDERED_ACCESS)) != 0)
	{
		const GpuStageFlags shaderStages = stages & (GpuStageFlag::VertexShaderNonUniform | GpuStageFlag::FragmentShaderNonUniform | GpuStageFlag::ComputeShaderNonUniform);
		const D3D12_BARRIER_SYNC shaderSync = GetStageSync(shaderStages);
		if(shaderSync == D3D12_BARRIER_SYNC_NONE)
		{
			// TODO - Submission preludes can establish a shader-readable layout before a leading primary barrier. Once
			// entry barriers are absorbed into the prelude, the exact shader stage must always be available here.
			return D3D12_BARRIER_SYNC_ALL_SHADING;
		}

		requiredSync |= shaderSync;
	}

	if(requiredSync != D3D12_BARRIER_SYNC_NONE)
		return requiredSync;

	B3D_ASSERT(false && "D3D12 texture access has no compatible synchronization stage.");
	return D3D12_BARRIER_SYNC_ALL;
}

D3D12BarrierScope D3D12BarrierUtility::GetTextureLayoutScope(GpuImageLayout layout, const D3D12TextureLayout& nativeLayout, GpuTextureAspectFlags aspects, GpuStageFlags preferredStages)
{
	// TODO - Deriving access type from layout might be too broad, need to investigate if we can do this more narrowly
	GpuStageFlags stages = GpuStageFlag::None;
	GpuAccessFlags access = GpuAccessFlag::None;
	switch(layout)
	{
	case GpuImageLayout::General:
	case GpuImageLayout::ShaderReadOnly:
		stages = preferredStages & GpuStageFlag::AllShader;
		if(stages == GpuStageFlag::None)
		{
			const D3D12_BARRIER_LAYOUT aspectLayout = nativeLayout.GetLayout(aspects);
			stages = IsComputeQueueLayout(aspectLayout) ? GpuStageFlag::ComputeShaderNonUniform : GpuStageFlag::FragmentShaderNonUniform;
		}
		access = layout == GpuImageLayout::General ? GpuAccessFlag::Read | GpuAccessFlag::Write : GpuAccessFlag::Read;
		break;
	case GpuImageLayout::ColorAttachment:
		stages = GpuStageFlag::ColorAttachment;
		access = GpuAccessFlag::Read | GpuAccessFlag::Write;
		break;
	case GpuImageLayout::DepthStencilAttachment:
	case GpuImageLayout::DepthReadOnlyStencilAttachment:
	case GpuImageLayout::DepthAttachmentStencilReadOnly:
		stages = GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests;
		access = GpuAccessFlag::Read | GpuAccessFlag::Write;
		break;
	case GpuImageLayout::DepthStencilReadOnly:
		stages = GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests;
		access = GpuAccessFlag::Read;
		break;
	case GpuImageLayout::TransferSource:
		stages = GpuStageFlag::Transfer;
		access = GpuAccessFlag::Read;
		break;
	case GpuImageLayout::TransferDestination:
		stages = GpuStageFlag::Transfer;
		access = GpuAccessFlag::Write;
		break;
	case GpuImageLayout::ResolveSource:
		stages = GpuStageFlag::Resolve;
		access = GpuAccessFlag::Read;
		break;
	case GpuImageLayout::ResolveDestination:
		stages = GpuStageFlag::Resolve;
		access = GpuAccessFlag::Write;
		break;
	case GpuImageLayout::Present:
		return { D3D12_BARRIER_SYNC_ALL, D3D12_BARRIER_ACCESS_COMMON };
	case GpuImageLayout::Undefined:
	default:
		return GetTextureScope(preferredStages, GpuAccessFlag::None, layout, aspects);
	}

	return GetTextureScope(stages, access, layout, aspects);
}

D3D12TextureLayout D3D12BarrierUtility::GetTextureLayout(GpuImageLayout layout, GpuQueueType queueType, GpuTextureAspectFlags aspects, bool allowConcurrentQueueReads)
{
	switch(layout)
	{
	case GpuImageLayout::General:
		return D3D12TextureLayout(GetQueueSpecificLayout(queueType,
			D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_UNORDERED_ACCESS,
			D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_UNORDERED_ACCESS,
			D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS));
	case GpuImageLayout::ColorAttachment:
		return D3D12TextureLayout(D3D12_BARRIER_LAYOUT_RENDER_TARGET);
	case GpuImageLayout::DepthStencilAttachment:
		return D3D12TextureLayout(D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE);
	case GpuImageLayout::DepthStencilReadOnly:
		if(allowConcurrentQueueReads)
			return D3D12TextureLayout(D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ_COMPUTE_QUEUE_ACCESSIBLE);

		return D3D12TextureLayout(GetQueueSpecificLayout(queueType,
			D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ,
			D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_SHADER_RESOURCE,
			D3D12_BARRIER_LAYOUT_SHADER_RESOURCE));
	case GpuImageLayout::DepthReadOnlyStencilAttachment:
		return D3D12TextureLayout(D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ, D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE);
	case GpuImageLayout::DepthAttachmentStencilReadOnly:
		return D3D12TextureLayout(D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE, D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ);
	case GpuImageLayout::ShaderReadOnly:
		if(allowConcurrentQueueReads)
			return D3D12TextureLayout(D3D12_BARRIER_LAYOUT_SHADER_RESOURCE);

		return D3D12TextureLayout(GetQueueSpecificLayout(queueType,
			D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_SHADER_RESOURCE,
			D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_SHADER_RESOURCE,
			D3D12_BARRIER_LAYOUT_SHADER_RESOURCE));
	case GpuImageLayout::TransferSource:
		if(queueType == GQT_TRANSFER)
			return D3D12TextureLayout::Common();

		return D3D12TextureLayout(GetQueueSpecificLayout(queueType,
			D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COPY_SOURCE,
			D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COPY_SOURCE,
			D3D12_BARRIER_LAYOUT_COPY_SOURCE));
	case GpuImageLayout::TransferDestination:
		if(queueType == GQT_TRANSFER)
			return D3D12TextureLayout::Common();

		return D3D12TextureLayout(GetQueueSpecificLayout(queueType,
			D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COPY_DEST,
			D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COPY_DEST,
			D3D12_BARRIER_LAYOUT_COPY_DEST));
	case GpuImageLayout::ResolveSource:
		return D3D12TextureLayout(D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE);
	case GpuImageLayout::ResolveDestination:
		return D3D12TextureLayout(D3D12_BARRIER_LAYOUT_RESOLVE_DEST);
	case GpuImageLayout::Present:
		return D3D12TextureLayout::Present();
	case GpuImageLayout::Undefined:
	default:
		return D3D12TextureLayout::Undefined();
	}
}

bool D3D12BarrierUtility::IsTextureLayoutSupportedOnQueue(const D3D12TextureLayout& layout, GpuTextureAspectFlags aspects, GpuQueueType queueType)
{
	if(aspects.IsSet(GpuTextureAspectFlag::Color) && !IsLayoutSupportedOnQueue(layout.GetLayout(GpuTextureAspectFlag::Color), queueType))
		return false;

	if(aspects.IsSet(GpuTextureAspectFlag::Depth) && !IsLayoutSupportedOnQueue(layout.GetLayout(GpuTextureAspectFlag::Depth), queueType))
		return false;

	return !aspects.IsSet(GpuTextureAspectFlag::Stencil) || IsLayoutSupportedOnQueue(layout.GetLayout(GpuTextureAspectFlag::Stencil), queueType);
}

bool D3D12BarrierUtility::CanTransitionTextureLayoutOnQueue(const D3D12TextureLayout& layout, GpuTextureAspectFlags aspects, GpuQueueType queueType)
{
	if(queueType == GQT_TRANSFER)
		return false;

	auto fnCanTransition = [queueType](D3D12_BARRIER_LAYOUT nativeLayout)
	{
		if(nativeLayout == D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ_COMPUTE_QUEUE_ACCESSIBLE)
			return queueType == GQT_GRAPHICS;

		return IsLayoutSupportedOnQueue(nativeLayout, queueType);
	};

	if(aspects.IsSet(GpuTextureAspectFlag::Color) && !fnCanTransition(layout.GetLayout(GpuTextureAspectFlag::Color)))
		return false;

	if(aspects.IsSet(GpuTextureAspectFlag::Depth) && !fnCanTransition(layout.GetLayout(GpuTextureAspectFlag::Depth)))
		return false;

	return !aspects.IsSet(GpuTextureAspectFlag::Stencil) || fnCanTransition(layout.GetLayout(GpuTextureAspectFlag::Stencil));
}
