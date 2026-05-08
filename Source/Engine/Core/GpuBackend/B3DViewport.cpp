//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DViewport.h"
#include "RTTI/B3DViewportRTTI.h"
#include "GpuBackend/B3DRenderTarget.h"
#include "Math/B3DMath.h"
#include "CoreObject/B3DCoreObjectSync.h"

using namespace b3d;

const Color ViewportBase::kDefaultClearColor = Color(0.0f, 0.3685f, 0.7969f);

ViewportBase::ViewportBase(float x, float y, float width, float height)
	: mNormArea(x, y, width, height), mClearFlags(ClearFlagBits::Color | ClearFlagBits::Depth), mClearColorValue(kDefaultClearColor), mClearDepthValue(1.0f), mClearStencilValue(0)
{
}

void ViewportBase::SetArea(const Area2& area)
{
	mNormArea = area;

	MarkRenderProxyDataDirtyInternal();
}

Area2I ViewportBase::GetPixelArea() const
{
	float width = (float)GetTargetWidth();
	float height = (float)GetTargetHeight();

	Area2I area;
	area.X = (int)(mNormArea.X * width);
	area.Y = (int)(mNormArea.Y * height);
	area.Width = (int)(mNormArea.Width * width);
	area.Height = (int)(mNormArea.Height * height);

	return area;
}

void ViewportBase::SetClearFlags(ClearFlags flags)
{
	mClearFlags = flags;

	MarkRenderProxyDataDirtyInternal();
}

void ViewportBase::SetClearValues(const Color& clearColor, float clearDepth, u16 clearStencil)
{
	mClearColorValue = clearColor;
	mClearDepthValue = clearDepth;
	mClearStencilValue = clearStencil;

	MarkRenderProxyDataDirtyInternal();
}

void ViewportBase::SetClearColorValue(const Color& color)
{
	mClearColorValue = color;

	MarkRenderProxyDataDirtyInternal();
}

void ViewportBase::SetClearDepthValue(float depth)
{
	mClearDepthValue = depth;

	MarkRenderProxyDataDirtyInternal();
}

void ViewportBase::SetClearStencilValue(u16 value)
{
	mClearStencilValue = value;

	MarkRenderProxyDataDirtyInternal();
}

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(Viewport, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mNormArea)
		B3D_SYNC_BLOCK_ENTRY(mClearFlags)
		B3D_SYNC_BLOCK_ENTRY(mClearColorValue)
		B3D_SYNC_BLOCK_ENTRY(mClearDepthValue)
		B3D_SYNC_BLOCK_ENTRY(mClearStencilValue)
		B3D_SYNC_BLOCK_ENTRY(mTarget)
	B3D_SYNC_BLOCK_END
}

Viewport::Viewport(const TShared<RenderTarget>& target, float x, float y, float width, float height)
	: TViewport(target, x, y, width, height)
{
}

void Viewport::SetTarget(const TShared<RenderTarget>& target)
{
	mTarget = target;

	MarkDependenciesDirty();
	MarkRenderProxyDataDirtyInternal();
}

void Viewport::MarkRenderProxyDataDirtyInternal()
{
	MarkRenderProxyDataDirty();
}

u32 Viewport::GetTargetWidth() const
{
	if(mTarget != nullptr)
		return mTarget->GetProperties().Width;

	return 0;
}

u32 Viewport::GetTargetHeight() const
{
	if(mTarget != nullptr)
		return mTarget->GetProperties().Height;

	return 0;
}

TShared<render::RenderProxy> Viewport::CreateRenderProxy() const
{
	TShared<render::RenderTarget> targetRenderProxy = B3DGetRenderProxy(mTarget);

	render::Viewport* renderProxy = new(B3DAllocate<render::Viewport>())
		render::Viewport(targetRenderProxy, mNormArea.X, mNormArea.Y, mNormArea.Width, mNormArea.Height);

	TShared<render::Viewport> renderProxyShared = B3DMakeSharedFromExisting<render::Viewport>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

RenderProxySyncPacket* Viewport::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	return allocator.Construct<SyncPacket>(*this, allocator, flags);
}

void Viewport::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	if(mTarget != nullptr)
		dependencies.push_back(mTarget.get());
}

TShared<Viewport> Viewport::Create(const TShared<RenderTarget>& target, float x, float y, float width, float height)
{
	Viewport* viewport = new(B3DAllocate<Viewport>()) Viewport(target, x, y, width, height);
	TShared<Viewport> viewportPtr = B3DMakeSharedFromExisting<Viewport>(viewport);
	viewportPtr->SetShared(viewportPtr);
	viewportPtr->Initialize();

	return viewportPtr;
}

TShared<Viewport> Viewport::CreateEmpty()
{
	Viewport* viewport = new(B3DAllocate<Viewport>()) Viewport();
	TShared<Viewport> viewportPtr = B3DMakeSharedFromExisting<Viewport>(viewport);
	viewportPtr->SetShared(viewportPtr);

	return viewportPtr;
}

RTTIType* Viewport::GetRttiStatic()
{
	return ViewportRTTI::Instance();
}

RTTIType* Viewport::GetRtti() const
{
	return Viewport::GetRttiStatic();
}

namespace b3d { namespace render
{
Viewport::Viewport(const TShared<RenderTarget>& target, float x, float y, float width, float height)
	: TViewport(target, x, y, width, height)
{}

TShared<Viewport> Viewport::Create(const TShared<RenderTarget>& target, float x, float y, float width, float height)
{
	Viewport* viewport = new(B3DAllocate<Viewport>()) Viewport(target, x, y, width, height);

	TShared<Viewport> viewportPtr = B3DMakeSharedFromExisting<Viewport>(viewport);
	viewportPtr->SetShared(viewportPtr);
	viewportPtr->Initialize();

	return viewportPtr;
}

u32 Viewport::GetTargetWidth() const
{
	if(mTarget != nullptr)
		return mTarget->GetProperties().Width;

	return 0;
}

u32 Viewport::GetTargetHeight() const
{
	if(mTarget != nullptr)
		return mTarget->GetProperties().Height;

	return 0;
}

void Viewport::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<b3d::Viewport::SyncPacket>();
	if(!syncPacket)
		return;

	syncPacket->ApplySyncData(this);
}
}}
