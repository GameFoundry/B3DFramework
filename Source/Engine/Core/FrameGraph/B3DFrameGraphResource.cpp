#include "B3DFrameGraphResource.h"

using namespace b3d;
using namespace b3d::render;

FrameGraphResource::FrameGraphResource(
	FrameGraphResourceId id,
	const StringView& name,
	FrameGraphResourceType type)
	: mId(id)
	, mName(name)
	, mType(type)
	, mIsTransient(false)
{
}

FrameGraphTextureResource::FrameGraphTextureResource(
	FrameGraphResourceId id,
	const StringView& name,
	const SPtr<Texture>& texture)
	: FrameGraphResource(id, name, FrameGraphResourceType::Texture)
	, mTexture(texture)
{
	B3D_ENSURE(texture != nullptr);
	mIsTransient = false;
}

FrameGraphTextureResource::FrameGraphTextureResource(
	FrameGraphResourceId id,
	const StringView& name)
	: FrameGraphResource(id, name, FrameGraphResourceType::Texture)
	, mTexture(nullptr)
{
	mIsTransient = true;
}

FrameGraphBufferResource::FrameGraphBufferResource(
	FrameGraphResourceId id,
	const StringView& name,
	const SPtr<GpuBuffer>& buffer)
	: FrameGraphResource(id, name, FrameGraphResourceType::Buffer)
	, mBuffer(buffer)
{
	B3D_ENSURE(buffer != nullptr);
	mIsTransient = false;
}

FrameGraphBufferResource::FrameGraphBufferResource(
	FrameGraphResourceId id,
	const StringView& name)
	: FrameGraphResource(id, name, FrameGraphResourceType::Buffer)
	, mBuffer(nullptr)
{
	mIsTransient = true;
}

FrameGraphRenderTargetResource::FrameGraphRenderTargetResource(
	FrameGraphResourceId id,
	const StringView& name,
	const SPtr<RenderTarget>& renderTarget,
	RenderSurfaceMaskBits surface)
	: FrameGraphResource(id, name, FrameGraphResourceType::RenderTarget)
	, mRenderTarget(renderTarget)
	, mSurface(surface)
{
	B3D_ENSURE(renderTarget != nullptr);
}
