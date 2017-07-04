//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRenderTargets.h"
#include "BsGpuResourcePool.h"
#include "BsViewport.h"
#include "BsRenderAPI.h"
#include "BsTextureManager.h"
#include "BsRendererUtility.h"
#include "BsGpuBuffer.h"

namespace bs { namespace ct
{
	RenderTargets::RenderTargets(const RENDERER_VIEW_TARGET_DESC& view, bool hdr)
		:mViewTarget(view), mHDR(hdr), mWidth(view.targetWidth), mHeight(view.targetHeight)
	{
		// Note: Consider customizable HDR format via options? e.g. smaller PF_FLOAT_R11G11B10 or larger 32-bit format
		mSceneColorFormat = PF_FLOAT16_RGBA;
		mAlbedoFormat = PF_R8G8B8A8; // Note: Also consider customizable format (e.g. 16-bit float?)
		mNormalFormat = PF_UNORM_R10G10B10A2; // Note: Also consider customizable format (e.g. 16-bit float?)
	}

	SPtr<RenderTargets> RenderTargets::create(const RENDERER_VIEW_TARGET_DESC& view, bool hdr)
	{
		return bs_shared_ptr<RenderTargets>(new (bs_alloc<RenderTargets>()) RenderTargets(view, hdr));
	}

	void RenderTargets::prepare()
	{
		GpuResourcePool& texPool = GpuResourcePool::instance();

		UINT32 width = mViewTarget.viewRect.width;
		UINT32 height = mViewTarget.viewRect.height;

		mDepthTex = texPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_D32_S8X24, width, height, TU_DEPTHSTENCIL, 
			mViewTarget.numSamples, false));
	}

	void RenderTargets::cleanup()
	{
		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(nullptr);

		GpuResourcePool& texPool = GpuResourcePool::instance();
		texPool.release(mDepthTex);
	}

	void RenderTargets::allocate(RenderTargetType type)
	{
		GpuResourcePool& texPool = GpuResourcePool::instance();

		UINT32 width = mViewTarget.viewRect.width;
		UINT32 height = mViewTarget.viewRect.height;

		// Note: This class is keeping all these textures alive for too long (even after they are done for a frame). We
		// could save on memory by deallocating and reallocating them every frame, but it remains to be seen how much of
		// a performance impact would that have.

		if (type == RTT_GBuffer)
		{
			// Note: Albedo is allocated as SRGB, meaning when reading from textures during depth pass we decode from sRGB
			// into linear, then back into sRGB when writing to albedo, and back to linear when reading from albedo during
			// light pass. This /might/ have a performance impact. In which case we could just use a higher precision albedo
			// buffer, which can then store linear color directly (storing linear in 8bit buffer causes too much detail to
			// be lost in the blacks).
			mAlbedoTex = texPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(mAlbedoFormat, width, height, TU_RENDERTARGET,
				mViewTarget.numSamples, true));
			mNormalTex = texPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(mNormalFormat, width, height, TU_RENDERTARGET,
				mViewTarget.numSamples, false));
			mRoughMetalTex = texPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_FLOAT16_RG, width, height, TU_RENDERTARGET,
				mViewTarget.numSamples, false)); // Note: Metal doesn't need 16-bit float

			bool rebuildRT = false;
			if (mGBufferRT != nullptr)
			{
				rebuildRT |= mGBufferRT->getColorTexture(0) != mAlbedoTex->texture;
				rebuildRT |= mGBufferRT->getColorTexture(1) != mNormalTex->texture;
				rebuildRT |= mGBufferRT->getColorTexture(2) != mRoughMetalTex->texture;
				rebuildRT |= mGBufferRT->getDepthStencilTexture() != mDepthTex->texture;
			}
			else
				rebuildRT = true;

			if (mGBufferRT == nullptr || rebuildRT)
			{
				RENDER_TEXTURE_DESC gbufferDesc;
				gbufferDesc.colorSurfaces[0].texture = mAlbedoTex->texture;
				gbufferDesc.colorSurfaces[0].face = 0;
				gbufferDesc.colorSurfaces[0].numFaces = 1;
				gbufferDesc.colorSurfaces[0].mipLevel = 0;

				gbufferDesc.colorSurfaces[1].texture = mNormalTex->texture;
				gbufferDesc.colorSurfaces[1].face = 0;
				gbufferDesc.colorSurfaces[1].numFaces = 1;
				gbufferDesc.colorSurfaces[1].mipLevel = 0;

				gbufferDesc.colorSurfaces[2].texture = mRoughMetalTex->texture;
				gbufferDesc.colorSurfaces[2].face = 0;
				gbufferDesc.colorSurfaces[2].numFaces = 1;
				gbufferDesc.colorSurfaces[2].mipLevel = 0;

				gbufferDesc.depthStencilSurface.texture = mDepthTex->texture;
				gbufferDesc.depthStencilSurface.face = 0;
				gbufferDesc.depthStencilSurface.mipLevel = 0;

				mGBufferRT = RenderTexture::create(gbufferDesc);
			}
		}
		else if(type == RTT_SceneColor)
		{
			mSceneColorTex = texPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(mSceneColorFormat, width,
				height, TU_RENDERTARGET | TU_LOADSTORE, mViewTarget.numSamples, false));

			if (mViewTarget.numSamples > 1)
			{
				UINT32 bufferNumElements = width * height * mViewTarget.numSamples;
				mFlattenedSceneColorBuffer = texPool.get(POOLED_STORAGE_BUFFER_DESC::createStandard(BF_16X4F, bufferNumElements));
			}

			bool rebuildRT = false;
			if (mSceneColorRT != nullptr)
			{
				rebuildRT |= mSceneColorRT->getColorTexture(0) != mSceneColorTex->texture;
				rebuildRT |= mSceneColorRT->getDepthStencilTexture() != mDepthTex->texture;
			}
			else
				rebuildRT = true;

			if (rebuildRT)
			{
				RENDER_TEXTURE_DESC sceneColorDesc;
				sceneColorDesc.colorSurfaces[0].texture = mSceneColorTex->texture;
				sceneColorDesc.colorSurfaces[0].face = 0;
				sceneColorDesc.colorSurfaces[0].numFaces = 1;
				sceneColorDesc.colorSurfaces[0].mipLevel = 0;

				sceneColorDesc.depthStencilSurface.texture = mDepthTex->texture;
				sceneColorDesc.depthStencilSurface.face = 0;
				sceneColorDesc.depthStencilSurface.numFaces = 1;
				sceneColorDesc.depthStencilSurface.mipLevel = 0;

				mSceneColorRT = TextureManager::instance().createRenderTexture(sceneColorDesc);
			}
		}
		else if(type == RTT_LightAccumulation)
		{
			if (mViewTarget.numSamples > 1)
			{
				UINT32 bufferNumElements = width * height * mViewTarget.numSamples;
				mFlattenedLightAccumulationBuffer =
					texPool.get(POOLED_STORAGE_BUFFER_DESC::createStandard(BF_16X4F, bufferNumElements));

				SPtr<GpuBuffer> buffer = mFlattenedLightAccumulationBuffer->buffer;

				auto& bufferProps = buffer->getProperties();

				UINT32 bufferSize = bufferProps.getElementSize() * bufferProps.getElementCount();
				UINT16* data = (UINT16*)buffer->lock(0, bufferSize, GBL_WRITE_ONLY_DISCARD);
				{
					memset(data, 0, bufferSize);
				}
				buffer->unlock();
			}

			mLightAccumulationTex = texPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(mSceneColorFormat, width,
				height, TU_LOADSTORE | TU_RENDERTARGET, mViewTarget.numSamples, false));

			bool rebuildRT;
			if (mLightAccumulationRT != nullptr)
				rebuildRT = mLightAccumulationRT->getColorTexture(0) != mLightAccumulationTex->texture;
			else
				rebuildRT = true;

			if (rebuildRT)
			{
				RENDER_TEXTURE_DESC lightAccumulationRTDesc;
				lightAccumulationRTDesc.colorSurfaces[0].texture = mLightAccumulationTex->texture;
				lightAccumulationRTDesc.colorSurfaces[0].face = 0;
				lightAccumulationRTDesc.colorSurfaces[0].numFaces = 1;
				lightAccumulationRTDesc.colorSurfaces[0].mipLevel = 0;

				mLightAccumulationRT = TextureManager::instance().createRenderTexture(lightAccumulationRTDesc);
			}
		}
		else if(type == RTT_LightOcclusion)
		{
			mLightOcclusionTex = texPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_R8, width,
				height, TU_RENDERTARGET, mViewTarget.numSamples, false));

			bool rebuildRT = false;
			if (mLightOcclusionRT != nullptr)
			{
				rebuildRT |= mLightOcclusionRT->getColorTexture(0) != mLightOcclusionTex->texture;
				rebuildRT |= mLightOcclusionRT->getDepthStencilTexture() != mDepthTex->texture;
			}
			else
				rebuildRT = true;

			if (rebuildRT)
			{
				RENDER_TEXTURE_DESC lightOcclusionRTDesc;
				lightOcclusionRTDesc.colorSurfaces[0].texture = mLightOcclusionTex->texture;
				lightOcclusionRTDesc.colorSurfaces[0].face = 0;
				lightOcclusionRTDesc.colorSurfaces[0].numFaces = 1;
				lightOcclusionRTDesc.colorSurfaces[0].mipLevel = 0;

				lightOcclusionRTDesc.depthStencilSurface.texture = mDepthTex->texture;
				lightOcclusionRTDesc.depthStencilSurface.face = 0;
				lightOcclusionRTDesc.depthStencilSurface.numFaces = 1;
				lightOcclusionRTDesc.depthStencilSurface.mipLevel = 0;

				mLightOcclusionRT = TextureManager::instance().createRenderTexture(lightOcclusionRTDesc);
			}
		}
		else if(type == RTT_ResolvedSceneColor)
		{
			// If not using MSAA, this is equivalent to default scene color texture
			if(mViewTarget.numSamples > 1)
			{
				mResolvedSceneColorTex1 = texPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(mSceneColorFormat, width, height,
					TU_RENDERTARGET, 1, false));
			}
		}
		else if(type == RTT_ResolvedSceneColorSecondary)
		{
			mResolvedSceneColorTex2 = texPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(mSceneColorFormat, 
				width, height, TU_RENDERTARGET, 1, false));
		}
		else if(type == RTT_HiZ)
		{
			mHiZ = GpuResourcePool::instance().get(
				BuildHiZ::getHiZTextureDesc(
					mViewTarget.viewRect.width, 
					mViewTarget.viewRect.height)
			);
		}
		else if(type == RTT_ResolvedDepth)
		{
			// If not using MSAA, this is equivalent to default depth texture
			if(mViewTarget.numSamples > 1)
			{
				mResolvedDepthTex = texPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_D32, width, height, TU_RENDERTARGET,
					1, false));
			}
		}
		else if(type == RTT_AmbientOcclusion)
		{
			mAmbientOcclusionTex = texPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_R8, width, height, TU_RENDERTARGET));
		}
	}

	void RenderTargets::release(RenderTargetType type)
	{
		GpuResourcePool& texPool = GpuResourcePool::instance();

		if (type == RTT_GBuffer)
		{
			texPool.release(mSceneColorTex);
			texPool.release(mAlbedoTex);
			texPool.release(mNormalTex);
			texPool.release(mRoughMetalTex);
		}
		else if(type == RTT_SceneColor)
		{
			texPool.release(mSceneColorTex);

			if (mFlattenedSceneColorBuffer != nullptr)
				texPool.release(mFlattenedSceneColorBuffer);
		}
		else if(type == RTT_LightAccumulation)
		{
			if (mLightAccumulationTex != nullptr)
				texPool.release(mLightAccumulationTex);

			if (mFlattenedLightAccumulationBuffer != nullptr)
				texPool.release(mFlattenedLightAccumulationBuffer);
		}
		else if(type == RTT_LightOcclusion)
		{
			if (mLightOcclusionTex != nullptr)
				texPool.release(mLightOcclusionTex);
		}
		else if(type == RTT_ResolvedSceneColor)
		{
			if (mResolvedSceneColorTex1 != nullptr)
				texPool.release(mResolvedSceneColorTex1);
		}
		else if(type == RTT_ResolvedSceneColorSecondary)
		{
			if (mResolvedSceneColorTex2 != nullptr)
				texPool.release(mResolvedSceneColorTex2);
		}
		else if(type == RTT_HiZ)
		{
			if (mHiZ != nullptr)
				texPool.release(mHiZ);
		}
		else if(type == RTT_ResolvedDepth)
		{
			if (mResolvedDepthTex != nullptr)
				texPool.release(mResolvedDepthTex);
		}
		else if(type == RTT_AmbientOcclusion)
		{
			if (mAmbientOcclusionTex != nullptr)
				texPool.release(mAmbientOcclusionTex);
		}
	}

	void RenderTargets::bind(RenderTargetType type, bool readOnlyDepthStencil)
	{
		RenderAPI& rapi = RenderAPI::instance();
		switch(type)
		{
		case RTT_GBuffer:
		{
			rapi.setRenderTarget(mGBufferRT);

			Rect2 area(0.0f, 0.0f, 1.0f, 1.0f);
			rapi.setViewport(area);

			// Clear depth & stencil according to user defined values, don't clear color as all values will get written to
			UINT32 clearFlags = mViewTarget.clearFlags & ~FBT_COLOR;
			if (clearFlags != 0)
			{
				RenderAPI::instance().clearViewport(clearFlags, mViewTarget.clearColor,
					mViewTarget.clearDepthValue, mViewTarget.clearStencilValue, 0x01);
			}

			// Clear all non primary targets (Note: I could perhaps clear all but albedo, since it stores a per-pixel write mask)
			RenderAPI::instance().clearViewport(FBT_COLOR, Color::ZERO, 1.0f, 0, 0xFF & ~0x01);
		}
		break;
		case RTT_SceneColor:
		{
			int readOnlyFlags = 0;
			if (readOnlyDepthStencil)
				readOnlyFlags = FBT_DEPTH | FBT_STENCIL;

			rapi.setRenderTarget(mSceneColorRT, readOnlyFlags, RT_COLOR0 | RT_DEPTH_STENCIL);

			Rect2 area(0.0f, 0.0f, 1.0f, 1.0f);
			rapi.setViewport(area);
			
		}
		break;
		case RTT_LightAccumulation:
			rapi.setRenderTarget(mLightAccumulationRT, FBT_DEPTH | FBT_STENCIL, RT_COLOR0 | RT_DEPTH_STENCIL);
		break;
		case RTT_LightOcclusion:
		{
			rapi.setRenderTarget(mLightOcclusionRT, FBT_DEPTH, RT_DEPTH_STENCIL);

			Rect2 area(0.0f, 0.0f, 1.0f, 1.0f);
			rapi.setViewport(area);

			RenderAPI::instance().clearViewport(FBT_COLOR, Color::ZERO);
		}
		break;
		default:
		{
			int readOnlyFlags = 0;
			if (readOnlyDepthStencil)
				readOnlyFlags = FBT_DEPTH | FBT_STENCIL;

			SPtr<RenderTexture> rt = getRT(type);
			if (rt)
				rapi.setRenderTarget(rt, readOnlyFlags);
		}
		break;
		}
	}

	void RenderTargets::generate(RenderTargetType type)
	{
		switch(type)
		{
		case RTT_HiZ:
			mBuildHiZ.execute(mViewTarget, get(RTT_ResolvedDepth), mHiZ->texture);
		break;
		case RTT_ResolvedSceneColor:
			if (mViewTarget.numSamples > 1)
			{
				RenderAPI::instance().setRenderTarget(mResolvedSceneColorTex1->renderTexture);
				gRendererUtility().blit(mDepthTex->texture);
			}
			break;
		default:
			break;
		}
	}

	SPtr<Texture> RenderTargets::get(RenderTargetType type, RenderSurfaceMaskBits surface) const
	{
		switch(type)
		{
		case RTT_ResolvedDepth:
			if (mViewTarget.numSamples > 1)
				return mResolvedDepthTex->texture;

			return mDepthTex->texture;
		case RTT_ResolvedSceneColor:
			if (mViewTarget.numSamples > 1)
				return mResolvedSceneColorTex1->texture;
			else
				return mSceneColorTex->texture;
		default:
		{
			SPtr<PooledRenderTexture> pooledTex = getPooledTexture(type, surface);
			if(pooledTex)
				return pooledTex->texture;

			return nullptr;
		}
		}
	}

	SPtr<RenderTexture> RenderTargets::getRT(RenderTargetType type) const
	{
		switch(type)
		{
		case RTT_GBuffer:
			return mGBufferRT;
		case RTT_SceneColor:
			return mSceneColorRT;
		case RTT_LightOcclusion:
			return mLightOcclusionRT;
		case RTT_LightAccumulation:
			return mLightAccumulationRT;
		case RTT_ResolvedSceneColor:
			if (mViewTarget.numSamples > 1)
				return mResolvedSceneColorTex1->renderTexture;
			else
				return mSceneColorRT;
		default:
		{
			SPtr<PooledRenderTexture> pooledTex = getPooledTexture(type);

			if(pooledTex)
				return pooledTex->renderTexture;

			return nullptr;
		}
		}
	}

	SPtr<PooledRenderTexture> RenderTargets::getPooledTexture(RenderTargetType type, RenderSurfaceMaskBits surface) const
	{
		switch(type)
		{
		case RTT_GBuffer:
		{
			switch (surface)
			{
			default:
			case RT_COLOR0:
				return mAlbedoTex;
			case RT_COLOR1:
				return mNormalTex;
			case RT_COLOR2:
				return mRoughMetalTex;
			case RT_DEPTH_STENCIL:
			case RT_DEPTH:
				return mDepthTex;
			}
		}
		case RTT_SceneColor:
			return mSceneColorTex;
		case RTT_LightAccumulation:
			return mLightAccumulationTex;
		case RTT_LightOcclusion:
			return mLightOcclusionTex;
		case RTT_ResolvedSceneColor:
			return mResolvedSceneColorTex1;
		case RTT_ResolvedSceneColorSecondary:
			return mResolvedSceneColorTex1;
		case RTT_HiZ:
			return mHiZ;
		case RTT_ResolvedDepth:
			return mResolvedDepthTex;
		case RTT_AmbientOcclusion:
			return mAmbientOcclusionTex;
		default:
			return nullptr;
		}
	}

	SPtr<GpuBuffer> RenderTargets::getSceneColorBuffer() const
	{
		return mFlattenedSceneColorBuffer->buffer;
	}

	SPtr<GpuBuffer> RenderTargets::getLightAccumulationBuffer() const
	{
		return mFlattenedLightAccumulationBuffer->buffer;
	}
}}
