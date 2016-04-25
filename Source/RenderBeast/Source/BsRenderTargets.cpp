//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRenderTargets.h"
#include "BsRenderTexturePool.h"
#include "BsViewport.h"
#include "BsRenderAPI.h"
#include "BsTextureManager.h"
#include "BsRendererUtility.h"

namespace BansheeEngine
{
	RenderTargets::RenderTargets(const SPtr<ViewportCore>& viewport, bool hdr, UINT32 numSamples)
		:mViewport(viewport), mNumSamples(numSamples), mHDR(hdr)
	{
		// Note: Consider customizable HDR format via options? e.g. smaller PF_FLOAT_R11G11B10 or larger 32-bit format
		mSceneColorFormat = hdr ? PF_FLOAT16_RGBA : PF_B8G8R8X8;
		mDiffuseFormat = PF_B8G8R8X8; // Note: Also consider customizable format (e.g. 16-bit float?)
		mNormalFormat = PF_UNORM_R10G10B10A2; // Note: Also consider customizable format (e.g. 16-bit float?)
	}

	SPtr<RenderTargets> RenderTargets::create(const SPtr<ViewportCore>& viewport, bool hdr, UINT32 numSamples)
	{
		return bs_shared_ptr<RenderTargets>(new (bs_alloc<RenderTargets>()) RenderTargets(viewport, hdr, numSamples));
	}

	void RenderTargets::allocate()
	{
		RenderTexturePool& texPool = RenderTexturePool::instance();

		UINT32 width = getWidth();
		UINT32 height = getHeight();

		SPtr<PooledRenderTexture> newColorRT = texPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(mSceneColorFormat, width, height, TU_RENDERTARGET,
			mNumSamples, false));
		SPtr<PooledRenderTexture> newAlbedoRT = texPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(mDiffuseFormat, width, 
			height, TU_RENDERTARGET, mNumSamples, false));
		SPtr<PooledRenderTexture> newNormalRT = texPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(mNormalFormat, width, 
			height, TU_RENDERTARGET, mNumSamples, false));
		SPtr<PooledRenderTexture> newDepthRT = texPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_D24S8, width, height, 
			TU_DEPTHSTENCIL, mNumSamples, false));

		bool rebuildTargets = newColorRT != mSceneColorTex || newAlbedoRT != mAlbedoTex || newNormalRT != mNormalTex || newDepthRT != mDepthTex;

		mSceneColorTex = newColorRT;
		mAlbedoTex = newAlbedoRT;
		mNormalTex = newNormalRT;
		mDepthTex = newDepthRT;

		if (mGBufferRT == nullptr || mSceneColorRT == nullptr || rebuildTargets)
		{
			MULTI_RENDER_TEXTURE_CORE_DESC gbufferDesc;
			gbufferDesc.colorSurfaces.resize(3);

			gbufferDesc.colorSurfaces[0].texture = mSceneColorTex->texture;
			gbufferDesc.colorSurfaces[0].face = 0;
			gbufferDesc.colorSurfaces[0].mipLevel = 0;

			gbufferDesc.colorSurfaces[1].texture = mAlbedoTex->texture;
			gbufferDesc.colorSurfaces[1].face = 0;
			gbufferDesc.colorSurfaces[1].mipLevel = 0;

			gbufferDesc.colorSurfaces[2].texture = mNormalTex->texture;
			gbufferDesc.colorSurfaces[2].face = 0;
			gbufferDesc.colorSurfaces[2].mipLevel = 0;

			gbufferDesc.depthStencilSurface.texture = mDepthTex->texture;
			gbufferDesc.depthStencilSurface.face = 0;
			gbufferDesc.depthStencilSurface.mipLevel = 0;

			mGBufferRT = TextureCoreManager::instance().createMultiRenderTexture(gbufferDesc);

			RENDER_TEXTURE_CORE_DESC sceneColorDesc;
			sceneColorDesc.colorSurface.texture = mSceneColorTex->texture;
			sceneColorDesc.colorSurface.face = 0;
			sceneColorDesc.colorSurface.mipLevel = 0;

			sceneColorDesc.depthStencilSurface.texture = mDepthTex->texture;
			sceneColorDesc.depthStencilSurface.face = 0;
			sceneColorDesc.depthStencilSurface.mipLevel = 0;

			mSceneColorRT = TextureCoreManager::instance().createRenderTexture(sceneColorDesc);
		}
	}

	void RenderTargets::release()
	{
		RenderAPICore& rapi = RenderAPICore::instance();
		rapi.setRenderTarget(nullptr);

		RenderTexturePool& texPool = RenderTexturePool::instance();

		texPool.release(mSceneColorTex);
		texPool.release(mAlbedoTex);
		texPool.release(mNormalTex);
		texPool.release(mDepthTex);
	}

	void RenderTargets::bindGBuffer()
	{
		RenderAPICore& rapi = RenderAPICore::instance();
		rapi.setRenderTarget(mGBufferRT);

		Rect2 area(0.0f, 0.0f, 1.0f, 1.0f);
		rapi.setViewport(area);

		UINT32 clearBuffers = 0;
		if (mViewport->getRequiresColorClear())
			clearBuffers |= FBT_COLOR;

		if (mViewport->getRequiresDepthClear())
			clearBuffers |= FBT_DEPTH;

		if (mViewport->getRequiresStencilClear())
			clearBuffers |= FBT_STENCIL;

		// Clear scene color, depth, stencil according to user defined values
		if (clearBuffers != 0)
		{
			RenderAPICore::instance().clearViewport(clearBuffers, mViewport->getClearColor(),
				mViewport->getClearDepthValue(), mViewport->getClearStencilValue(), 0x01);
		}

		// Clear all others
		RenderAPICore::instance().clearViewport(FBT_COLOR, Color::ZERO, 1.0f, 0, 0xFF & ~0x01);
	}

	void RenderTargets::bindSceneColor(bool readOnlyDepthStencil)
	{
		RenderAPICore& rapi = RenderAPICore::instance();
		rapi.setRenderTarget(mSceneColorRT, readOnlyDepthStencil);

		Rect2 area(0.0f, 0.0f, 1.0f, 1.0f);
		rapi.setViewport(area);
	}

	SPtr<TextureCore> RenderTargets::getTextureA() const
	{
		return mAlbedoTex->texture;
	}

	SPtr<TextureCore> RenderTargets::getTextureB() const
	{
		return mNormalTex->texture;
	}

	SPtr<TextureCore> RenderTargets::getTextureDepth() const
	{
		return mDepthTex->texture;
	}

	UINT32 RenderTargets::getWidth() const
	{
		return (UINT32)mViewport->getWidth();
	}

	UINT32 RenderTargets::getHeight() const
	{
		return (UINT32)mViewport->getHeight();
	}
}