//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsViewport.h"
#include "Private/RTTI/BsViewportRTTI.h"
#include "Error/BsException.h"
#include "RenderAPI/BsRenderTarget.h"
#include "Math/BsMath.h"
#include "RenderAPI/BsRenderAPI.h"
#include "CoreThread/BsCoreObjectSync.h"

using namespace bs;

const Color ViewportBase::kDefaultClearColor = Color(0.0f, 0.3685f, 0.7969f);

ViewportBase::ViewportBase(float x, float y, float width, float height)
	: mNormArea(x, y, width, height), mClearFlags(ClearFlagBits::Color | ClearFlagBits::Depth), mClearColorValue(kDefaultClearColor), mClearDepthValue(1.0f), mClearStencilValue(0)
{
}

void ViewportBase::SetArea(const Rect2& area)
{
	mNormArea = area;

	MarkCoreDirtyInternal();
}

Rect2I ViewportBase::GetPixelArea() const
{
	float width = (float)GetTargetWidth();
	float height = (float)GetTargetHeight();

	Rect2I area;
	area.X = (int)(mNormArea.X * width);
	area.Y = (int)(mNormArea.Y * height);
	area.Width = (int)(mNormArea.Width * width);
	area.Height = (int)(mNormArea.Height * height);

	return area;
}

void ViewportBase::SetClearFlags(ClearFlags flags)
{
	mClearFlags = flags;

	MarkCoreDirtyInternal();
}

void ViewportBase::SetClearValues(const Color& clearColor, float clearDepth, u16 clearStencil)
{
	mClearColorValue = clearColor;
	mClearDepthValue = clearDepth;
	mClearStencilValue = clearStencil;

	MarkCoreDirtyInternal();
}

void ViewportBase::SetClearColorValue(const Color& color)
{
	mClearColorValue = color;

	MarkCoreDirtyInternal();
}

void ViewportBase::SetClearDepthValue(float depth)
{
	mClearDepthValue = depth;

	MarkCoreDirtyInternal();
}

void ViewportBase::SetClearStencilValue(u16 value)
{
	mClearStencilValue = value;

	MarkCoreDirtyInternal();
}

template <bool Core>
template <class P>
void TViewport<Core>::RttiEnumFields(P p)
{
	p(mNormArea);
	p(mClearFlags);
	p(mClearColorValue);
	p(mClearDepthValue);
	p(mClearStencilValue);
	p(mTarget);
}

Viewport::Viewport(const SPtr<RenderTarget>& target, float x, float y, float width, float height)
	: TViewport(target, x, y, width, height)
{
}

void Viewport::SetTarget(const SPtr<RenderTarget>& target)
{
	mTarget = target;

	MarkDependenciesDirty();
	MarkCoreDirtyInternal();
}

SPtr<ct::Viewport> Viewport::GetCore() const
{
	return std::static_pointer_cast<ct::Viewport>(mCoreSpecific);
}

void Viewport::MarkCoreDirtyInternal()
{
	MarkCoreDirty();
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

SPtr<ct::CoreObject> Viewport::CreateCore() const
{
	SPtr<ct::RenderTarget> targetCore;
	if(mTarget != nullptr)
		targetCore = mTarget->GetCore();

	ct::Viewport* viewport = new(B3DAllocate<ct::Viewport>())
		ct::Viewport(targetCore, mNormArea.X, mNormArea.Y, mNormArea.Width, mNormArea.Height);

	SPtr<ct::Viewport> viewportPtr = B3DMakeSharedFromExisting<ct::Viewport>(viewport);
	viewportPtr->SetThisPtrInternal(viewportPtr);

	return viewportPtr;
}

CoreSyncData Viewport::SyncToCore(FrameAlloc* allocator)
{
	u32 size = csync_size(*this);

	u8* buffer = allocator->Alloc(size);
	Bitstream stream(buffer, size);

	csync_write(*this, stream);

	return CoreSyncData(buffer, size);
}

void Viewport::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	if(mTarget != nullptr)
		dependencies.push_back(mTarget.get());
}

SPtr<Viewport> Viewport::Create(const SPtr<RenderTarget>& target, float x, float y, float width, float height)
{
	Viewport* viewport = new(B3DAllocate<Viewport>()) Viewport(target, x, y, width, height);
	SPtr<Viewport> viewportPtr = B3DMakeCoreFromExisting<Viewport>(viewport);
	viewportPtr->SetThisPtrInternal(viewportPtr);
	viewportPtr->Initialize();

	return viewportPtr;
}

SPtr<Viewport> Viewport::CreateEmpty()
{
	Viewport* viewport = new(B3DAllocate<Viewport>()) Viewport();
	SPtr<Viewport> viewportPtr = B3DMakeCoreFromExisting<Viewport>(viewport);
	viewportPtr->SetThisPtrInternal(viewportPtr);

	return viewportPtr;
}

RTTITypeBase* Viewport::GetRttiStatic()
{
	return ViewportRTTI::Instance();
}

RTTITypeBase* Viewport::GetRtti() const
{
	return Viewport::GetRttiStatic();
}

namespace bs { namespace ct
{
Viewport::Viewport(const SPtr<RenderTarget>& target, float x, float y, float width, float height)
	: TViewport(target, x, y, width, height)
{}

SPtr<Viewport> Viewport::Create(const SPtr<RenderTarget>& target, float x, float y, float width, float height)
{
	Viewport* viewport = new(B3DAllocate<Viewport>()) Viewport(target, x, y, width, height);

	SPtr<Viewport> viewportPtr = B3DMakeSharedFromExisting<Viewport>(viewport);
	viewportPtr->SetThisPtrInternal(viewportPtr);
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

void Viewport::SyncToCore(const CoreSyncData& data)
{
	Bitstream stream(data.GetBuffer(), data.GetBufferSize());
	csync_read(*this, stream);
}
}}
