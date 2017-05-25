//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsShadowRendering.h"
#include "BsRendererView.h"
#include "BsRendererScene.h"
#include "BsLight.h"
#include "BsRendererUtility.h"
#include "BsGpuParamsSet.h"
#include "BsMesh.h"
#include "BsCamera.h"
#include "BsBitwise.h"

namespace bs { namespace ct
{
	ShadowParamsDef gShadowParamsDef;

	ShadowDepthNormalMat::ShadowDepthNormalMat()
	{ }

	void ShadowDepthNormalMat::_initDefines(ShaderDefines& defines)
	{
		// No defines
	}

	void ShadowDepthNormalMat::bind(const SPtr<GpuParamBlockBuffer>& shadowParams)
	{
		mParamsSet->setParamBlockBuffer("ShadowParams", shadowParams);

		gRendererUtility().setPass(mMaterial);
	}
	
	void ShadowDepthNormalMat::setPerObjectBuffer(const SPtr<GpuParamBlockBuffer>& perObjectParams)
	{
		mParamsSet->setParamBlockBuffer("PerObject", perObjectParams);
		gRendererUtility().setPassParams(mParamsSet);
	}

	ShadowDepthDirectionalMat::ShadowDepthDirectionalMat()
	{ }

	void ShadowDepthDirectionalMat::_initDefines(ShaderDefines& defines)
	{
		// No defines
	}

	void ShadowDepthDirectionalMat::bind(const SPtr<GpuParamBlockBuffer>& shadowParams)
	{
		mParamsSet->setParamBlockBuffer("ShadowParams", shadowParams);

		gRendererUtility().setPass(mMaterial);
	}
	
	void ShadowDepthDirectionalMat::setPerObjectBuffer(const SPtr<GpuParamBlockBuffer>& perObjectParams)
	{
		mParamsSet->setParamBlockBuffer("PerObject", perObjectParams);
		gRendererUtility().setPassParams(mParamsSet);
	}

	ShadowCubeMatricesDef gShadowCubeMatricesDef;
	ShadowCubeMasksDef gShadowCubeMasksDef;

	ShadowDepthCubeMat::ShadowDepthCubeMat()
	{ }

	void ShadowDepthCubeMat::_initDefines(ShaderDefines& defines)
	{
		// No defines
	}

	void ShadowDepthCubeMat::bind(const SPtr<GpuParamBlockBuffer>& shadowParams, 
		const SPtr<GpuParamBlockBuffer>& shadowCubeMatrices)
	{
		mParamsSet->setParamBlockBuffer("ShadowParams", shadowParams);
		mParamsSet->setParamBlockBuffer("ShadowCubeMatrices", shadowCubeMatrices);

		gRendererUtility().setPass(mMaterial);
	}

	void ShadowDepthCubeMat::setPerObjectBuffer(const SPtr<GpuParamBlockBuffer>& perObjectParams,
		const SPtr<GpuParamBlockBuffer>& shadowCubeMasks)
	{
		mParamsSet->setParamBlockBuffer("PerObject", perObjectParams);
		mParamsSet->setParamBlockBuffer("ShadowCubeMasks", shadowCubeMasks);

		gRendererUtility().setPassParams(mParamsSet);
	}

	void ShadowInfo::updateNormArea(UINT32 atlasSize)
	{
		normArea.x = area.x / (float)atlasSize;
		normArea.y = area.y / (float)atlasSize;
		normArea.width = area.width / (float)atlasSize;
		normArea.height = area.height / (float)atlasSize;
	}

	ShadowMapAtlas::ShadowMapAtlas(UINT32 size)
		:mLastUsedCounter(0)
	{
		mAtlas = GpuResourcePool::instance().get(
			POOLED_RENDER_TEXTURE_DESC::create2D(PF_D24S8, size, size, TU_DEPTHSTENCIL));
	}

	ShadowMapAtlas::~ShadowMapAtlas()
	{
		GpuResourcePool::instance().release(mAtlas);
	}

	bool ShadowMapAtlas::addMap(UINT32 size, Rect2I& area, UINT32 border)
	{
		UINT32 sizeWithBorder = size + border * 2;

		UINT32 x, y;
		if (!mLayout.addElement(sizeWithBorder, sizeWithBorder, x, y))
			return false;

		area.width = area.height = size;
		area.x = x + border;
		area.y = y + border;

		mLastUsedCounter = 0;
		return true;
	}

	void ShadowMapAtlas::clear()
	{
		mLayout.clear();
		mLastUsedCounter++;
	}

	bool ShadowMapAtlas::isEmpty() const
	{
		return mLayout.isEmpty();
	}

	SPtr<Texture> ShadowMapAtlas::getTexture() const
	{
		return mAtlas->texture;
	}

	SPtr<RenderTexture> ShadowMapAtlas::getTarget() const
	{
		return mAtlas->renderTexture;
	}

	ShadowMapBase::ShadowMapBase(UINT32 size)
		: mSize(size), mIsUsed(false), mLastUsedCounter (0)
	{ }

	SPtr<Texture> ShadowMapBase::getTexture() const
	{
		return mShadowMap->texture;
	}

	ShadowCubemap::ShadowCubemap(UINT32 size)
		:ShadowMapBase(size)
	{
		mShadowMap = GpuResourcePool::instance().get(
			POOLED_RENDER_TEXTURE_DESC::createCube(PF_D24S8, size, size, TU_DEPTHSTENCIL));

		RENDER_TEXTURE_DESC rtDesc;
		rtDesc.depthStencilSurface.texture = mShadowMap->texture;
		rtDesc.depthStencilSurface.numFaces = 6;
	}

	ShadowCubemap::~ShadowCubemap()
	{
		GpuResourcePool::instance().release(mShadowMap);
	}

	SPtr<RenderTexture> ShadowCubemap::getTarget() const
	{
		return mShadowMap->renderTexture;
	}

	ShadowCascadedMap::ShadowCascadedMap(UINT32 size)
		:ShadowMapBase(size)
	{
		mShadowMap = GpuResourcePool::instance().get(
			POOLED_RENDER_TEXTURE_DESC::create2D(PF_D24S8, size, size, TU_DEPTHSTENCIL, 0, false, NUM_CASCADE_SPLITS));

		RENDER_TEXTURE_DESC rtDesc;
		rtDesc.depthStencilSurface.texture = mShadowMap->texture;
		rtDesc.depthStencilSurface.numFaces = 1;

		for (int i = 0; i < NUM_CASCADE_SPLITS; ++i)
		{
			rtDesc.depthStencilSurface.face = i;
			mTargets[i] = RenderTexture::create(rtDesc);
		}
	}

	ShadowCascadedMap::~ShadowCascadedMap()
	{
		GpuResourcePool::instance().release(mShadowMap);
	}

	SPtr<RenderTexture> ShadowCascadedMap::getTarget(UINT32 cascadeIdx) const
	{
		return mTargets[cascadeIdx];
	}

	const UINT32 ShadowRendering::MAX_ATLAS_SIZE = 8192;
	const UINT32 ShadowRendering::MAX_UNUSED_FRAMES = 60;
	const UINT32 ShadowRendering::MIN_SHADOW_MAP_SIZE = 32;
	const UINT32 ShadowRendering::SHADOW_MAP_FADE_SIZE = 64;
	const UINT32 ShadowRendering::SHADOW_MAP_BORDER = 4;

	ShadowRendering::ShadowRendering(UINT32 shadowMapSize)
		: mShadowMapSize(shadowMapSize)
	{ }

	void ShadowRendering::setShadowMapSize(UINT32 size)
	{
		if (mShadowMapSize == size)
			return;

		mCascadedShadowMaps.clear();
		mDynamicShadowMaps.clear();
		mShadowCubemaps.clear();
	}

	void ShadowRendering::renderShadowMaps(RendererScene& scene, const FrameInfo& frameInfo)
	{
		// Note: Currently all shadows are dynamic and are rebuilt every frame. I should later added support for static
		// shadow maps which can be used for immovable lights. Such a light can then maintain a set of shadow maps,
		// one of which is static and only effects the static geometry, while the rest are per-object shadow maps used
		// for dynamic objects. Then only a small subset of geometry needs to be redrawn, instead of everything.

		// Note: Add support for per-object shadows and a way to force a renderable to use per-object shadows. This can be
		// used for adding high quality shadows on specific objects (e.g. important characters during cinematics).

		const SceneInfo& sceneInfo = scene.getSceneInfo();
		
		// Clear all transient data from last frame
		mShadowInfos.clear();

		mSpotLightShadows.resize(sceneInfo.spotLights.size());
		mRadialLightShadows.resize(sceneInfo.radialLights.size());
		mDirectionalLightShadows.resize(sceneInfo.directionalLights.size());

		mSpotLightShadowOptions.clear();
		mRadialLightShadowOptions.clear();

		// Clear all dynamic light atlases
		for (auto& entry : mCascadedShadowMaps)
			entry.clear();

		for (auto& entry : mDynamicShadowMaps)
			entry.clear();

		for (auto& entry : mShadowCubemaps)
			entry.clear();

		// Determine shadow map sizes and sort them
		UINT32 shadowInfoCount = 0;
		for (UINT32 i = 0; i < (UINT32)sceneInfo.spotLights.size(); ++i)
		{
			const RendererLight& light = sceneInfo.spotLights[i];

			// Note: I'm using visibility across all views, while I could be using visibility for every view individually,
			// if I kept that information somewhere
			if (!light.internal->getCastsShadow() || !sceneInfo.spotLightVisibility[i])
				continue;

			ShadowMapOptions options;
			options.lightIdx = i;

			float maxFadePercent;
			calcShadowMapProperties(light, scene, options.mapSize, options.fadePercents, maxFadePercent);

			// Don't render shadow maps that will end up nearly completely faded out
			if (maxFadePercent < 0.005f)
				continue;

			mSpotLightShadowOptions.push_back(options);
			mSpotLightShadows[i].startIdx = shadowInfoCount;
			mSpotLightShadows[i].numShadows = 0;

			shadowInfoCount++; // For now, always a single fully dynamic shadow for a single light, but that may change
		}

		for (UINT32 i = 0; i < (UINT32)sceneInfo.radialLights.size(); ++i)
		{
			const RendererLight& light = sceneInfo.radialLights[i];

			// Note: I'm using visibility across all views, while I could be using visibility for every view individually,
			// if I kept that information somewhere
			if (!light.internal->getCastsShadow() || !sceneInfo.radialLightVisibility[i])
				continue;

			ShadowMapOptions options;
			options.lightIdx = i;

			float maxFadePercent;
			calcShadowMapProperties(light, scene, options.mapSize, options.fadePercents, maxFadePercent);

			// Don't render shadow maps that will end up nearly completely faded out
			if (maxFadePercent < 0.005f)
				continue;

			mRadialLightShadowOptions.push_back(options);
			mRadialLightShadows[i].startIdx = shadowInfoCount;
			mRadialLightShadows[i].numShadows = 0;

			shadowInfoCount++; // For now, always a single fully dynamic shadow for a single light, but that may change
		}

		// Sort spot lights by size so they fit neatly in the texture atlas
		std::sort(mSpotLightShadowOptions.begin(), mSpotLightShadowOptions.end(),
			[](const ShadowMapOptions& a, const ShadowMapOptions& b) { return a.mapSize > b.mapSize; } );

		// Reserve space for shadow infos
		mShadowInfos.resize(shadowInfoCount);

		// Render shadow maps
		for (UINT32 i = 0; i < (UINT32)sceneInfo.directionalLights.size(); ++i)
		{
			const RendererLight& light = sceneInfo.directionalLights[i];

			if (!light.internal->getCastsShadow())
				return;

			for (UINT32 j = 0; j < (UINT32)sceneInfo.views.size(); ++j)
				renderCascadedShadowMaps(j, i, scene, frameInfo);
		}

		for(auto& entry : mSpotLightShadowOptions)
		{
			UINT32 lightIdx = entry.lightIdx;
			renderSpotShadowMap(sceneInfo.spotLights[lightIdx], entry, scene, frameInfo);
		}

		for (auto& entry : mRadialLightShadowOptions)
		{
			UINT32 lightIdx = entry.lightIdx;
			renderRadialShadowMap(sceneInfo.radialLights[lightIdx], entry, scene, frameInfo);
		}
		
		// Deallocate unused textures
		for(auto iter = mDynamicShadowMaps.begin(); iter != mDynamicShadowMaps.end(); ++iter)
		{
			if(iter->getLastUsedCounter() >= MAX_UNUSED_FRAMES)
			{
				// These are always populated in order, so we can assume all following atlases are also empty
				mDynamicShadowMaps.erase(iter, mDynamicShadowMaps.end());
				break;
			}
		}

		for(auto iter = mCascadedShadowMaps.begin(); iter != mCascadedShadowMaps.end();)
		{
			if (iter->getLastUsedCounter() >= MAX_UNUSED_FRAMES)
				iter = mCascadedShadowMaps.erase(iter);
			else
				++iter;
		}
		
		for(auto iter = mShadowCubemaps.begin(); iter != mShadowCubemaps.end();)
		{
			if (iter->getLastUsedCounter() >= MAX_UNUSED_FRAMES)
				iter = mShadowCubemaps.erase(iter);
			else
				++iter;
		}
	}

	void ShadowRendering::renderCascadedShadowMaps(UINT32 viewIdx, UINT32 lightIdx, RendererScene& scene, 
		const FrameInfo& frameInfo)
	{
		// Note: Currently I'm using spherical bounds for the cascaded frustum which might result in non-optimal usage
		// of the shadow map. A different approach would be to generate a bounding box and then both adjust the aspect
		// ratio (and therefore dimensions) of the shadow map, as well as rotate the camera so the visible area best fits
		// in the map. It remains to be seen if this is viable.
		const SceneInfo& sceneInfo = scene.getSceneInfo();

		const RendererView* view = sceneInfo.views[viewIdx];
		const RendererLight& rendererLight = sceneInfo.directionalLights[lightIdx];
		Light* light = rendererLight.internal;

		RenderAPI& rapi = RenderAPI::instance();

		Vector3 lightDir = light->getRotation().zAxis();
		SPtr<GpuParamBlockBuffer> shadowParamsBuffer = gShadowParamsDef.createBuffer();

		ShadowInfo shadowInfo;
		shadowInfo.lightIdx = lightIdx;
		shadowInfo.textureIdx = -1;

		UINT32 mapSize = std::min(mShadowMapSize, MAX_ATLAS_SIZE);

		for (UINT32 i = 0; i < (UINT32)mCascadedShadowMaps.size(); i++)
		{
			ShadowCascadedMap& shadowMap = mCascadedShadowMaps[i];

			if (!shadowMap.isUsed() && shadowMap.getSize() == mapSize)
			{
				shadowInfo.textureIdx = i;
				shadowMap.markAsUsed();

				break;
			}
		}

		if (shadowInfo.textureIdx == -1)
		{
			shadowInfo.textureIdx = (UINT32)mCascadedShadowMaps.size();
			mCascadedShadowMaps.push_back(ShadowCascadedMap(mapSize));

			ShadowCascadedMap& shadowMap = mCascadedShadowMaps.back();
			shadowMap.markAsUsed();
		}

		ShadowCascadedMap& shadowMap = mCascadedShadowMaps[shadowInfo.textureIdx];

		Matrix4 viewMat = Matrix4::view(light->getPosition(), light->getRotation());
		for (int i = 0; i < NUM_CASCADE_SPLITS; ++i)
		{
			Sphere frustumBounds;
			ConvexVolume cascadeCullVolume = getCSMSplitFrustum(*view, lightDir, i, NUM_CASCADE_SPLITS, frustumBounds);

			float orthoSize = frustumBounds.getRadius();
			Matrix4 proj = Matrix4::projectionOrthographic(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.0f, 1000.0f);
			RenderAPI::instance().convertProjectionMatrix(proj, proj);

			shadowInfo.shadowVPTransform = proj * viewMat;

			float nearDist = 0.05f;
			float farDist = light->getAttenuationRadius();
			float depthRange = farDist - nearDist;
			float depthBias = getDepthBias(*light, depthRange, mapSize);

			gShadowParamsDef.gDepthBias.set(shadowParamsBuffer, depthBias);
			gShadowParamsDef.gDepthRange.set(shadowParamsBuffer, depthRange);
			gShadowParamsDef.gMatViewProj.set(shadowParamsBuffer, shadowInfo.shadowVPTransform);

			rapi.setRenderTarget(shadowMap.getTarget(i));
			rapi.clearRenderTarget(FBT_DEPTH);

			mDepthDirectionalMat.bind(shadowParamsBuffer);

			for (UINT32 j = 0; j < sceneInfo.renderables.size(); j++)
			{
				if (!cascadeCullVolume.intersects(sceneInfo.renderableCullInfos[j].bounds.getSphere()))
					continue;

				scene.prepareRenderable(j, frameInfo);

				RendererObject* renderable = sceneInfo.renderables[j];
				mDepthDirectionalMat.setPerObjectBuffer(renderable->perObjectParamBuffer);

				for (auto& element : renderable->elements)
				{
					if (element.morphVertexDeclaration == nullptr)
						gRendererUtility().draw(element.mesh, element.subMesh);
					else
						gRendererUtility().drawMorph(element.mesh, element.subMesh, element.morphShapeBuffer,
							element.morphVertexDeclaration);
				}
			}

			shadowMap.setShadowInfo(i, shadowInfo);
		}

		mDirectionalLightShadows[lightIdx] = shadowInfo.textureIdx;
	}

	void ShadowRendering::renderSpotShadowMap(const RendererLight& rendererLight, const ShadowMapOptions& options,
		RendererScene& scene, const FrameInfo& frameInfo)
	{
		Light* light = rendererLight.internal;

		const SceneInfo& sceneInfo = scene.getSceneInfo();
		SPtr<GpuParamBlockBuffer> shadowParamsBuffer = gShadowParamsDef.createBuffer();

		ShadowInfo mapInfo;
		mapInfo.fadePerView = options.fadePercents;
		mapInfo.lightIdx = options.lightIdx;

		bool foundSpace = false;
		for (UINT32 i = 0; i < (UINT32)mDynamicShadowMaps.size(); i++)
		{
			ShadowMapAtlas& atlas = mDynamicShadowMaps[i];

			if (atlas.addMap(options.mapSize, mapInfo.area, SHADOW_MAP_BORDER))
			{
				mapInfo.textureIdx = i;

				foundSpace = true;
				break;
			}
		}

		if (!foundSpace)
		{
			mapInfo.textureIdx = (UINT32)mDynamicShadowMaps.size();
			mDynamicShadowMaps.push_back(ShadowMapAtlas(MAX_ATLAS_SIZE));

			ShadowMapAtlas& atlas = mDynamicShadowMaps.back();
			atlas.addMap(options.mapSize, mapInfo.area, SHADOW_MAP_BORDER);
		}

		mapInfo.updateNormArea(MAX_ATLAS_SIZE);
		ShadowMapAtlas& atlas = mDynamicShadowMaps[mapInfo.textureIdx];

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(atlas.getTarget());
		rapi.setViewport(mapInfo.normArea);
		rapi.clearViewport(FBT_DEPTH);

		float nearDist = 0.05f;
		float farDist = light->getAttenuationRadius();
		float depthRange = farDist - nearDist;
		float depthBias = getDepthBias(*light, depthRange, options.mapSize);

		Matrix4 view = Matrix4::view(light->getPosition(), light->getRotation());
		Matrix4 proj = Matrix4::projectionPerspective(light->getSpotAngle(), 1.0f, 0.05f, light->getAttenuationRadius());
		RenderAPI::instance().convertProjectionMatrix(proj, proj);

		mapInfo.shadowVPTransform = proj * view;

		gShadowParamsDef.gDepthBias.set(shadowParamsBuffer, depthBias);
		gShadowParamsDef.gDepthRange.set(shadowParamsBuffer, depthRange);
		gShadowParamsDef.gMatViewProj.set(shadowParamsBuffer, mapInfo.shadowVPTransform);

		mDepthNormalMat.bind(shadowParamsBuffer);

		ConvexVolume localFrustum = ConvexVolume(proj);

		const Vector<Plane>& frustumPlanes = localFrustum.getPlanes();
		Matrix4 worldMatrix = view.transpose();

		Vector<Plane> worldPlanes(frustumPlanes.size());
		UINT32 j = 0;
		for (auto& plane : frustumPlanes)
		{
			worldPlanes[j] = worldMatrix.multiplyAffine(plane);
			j++;
		}

		ConvexVolume worldFrustum(worldPlanes);
		for (UINT32 i = 0; i < sceneInfo.renderables.size(); i++)
		{
			if (!worldFrustum.intersects(sceneInfo.renderableCullInfos[i].bounds.getSphere()))
				continue;

			scene.prepareRenderable(i, frameInfo);

			RendererObject* renderable = sceneInfo.renderables[i];
			mDepthNormalMat.setPerObjectBuffer(renderable->perObjectParamBuffer);

			for (auto& element : renderable->elements)
			{
				if (element.morphVertexDeclaration == nullptr)
					gRendererUtility().draw(element.mesh, element.subMesh);
				else
					gRendererUtility().drawMorph(element.mesh, element.subMesh, element.morphShapeBuffer,
						element.morphVertexDeclaration);
			}
		}

		// Restore viewport
		rapi.setViewport(Rect2(0.0f, 0.0f, 1.0f, 1.0f));

		LightShadows& lightShadows = mSpotLightShadows[options.lightIdx];

		mShadowInfos[lightShadows.startIdx + lightShadows.numShadows] = mapInfo;
		lightShadows.numShadows++;
	}

	void ShadowRendering::renderRadialShadowMap(const RendererLight& rendererLight, 
		const ShadowMapOptions& options, RendererScene& scene, const FrameInfo& frameInfo)
	{
		Light* light = rendererLight.internal;

		const SceneInfo& sceneInfo = scene.getSceneInfo();
		SPtr<GpuParamBlockBuffer> shadowParamsBuffer = gShadowParamsDef.createBuffer();
		SPtr<GpuParamBlockBuffer> shadowCubeMatricesBuffer = gShadowCubeMatricesDef.createBuffer();
		SPtr<GpuParamBlockBuffer> shadowCubeMasksBuffer = gShadowCubeMasksDef.createBuffer();

		ShadowInfo mapInfo;
		mapInfo.lightIdx = options.lightIdx;
		mapInfo.textureIdx = -1;
		mapInfo.fadePerView = options.fadePercents;

		for (UINT32 i = 0; i < (UINT32)mShadowCubemaps.size(); i++)
		{
			ShadowCubemap& cubemap = mShadowCubemaps[i];

			if (!cubemap.isUsed() && cubemap.getSize() == options.mapSize)
			{
				mapInfo.textureIdx = i;
				cubemap.markAsUsed();

				break;
			}
		}

		if (mapInfo.textureIdx == -1)
		{
			mapInfo.textureIdx = (UINT32)mShadowCubemaps.size();
			mShadowCubemaps.push_back(ShadowCubemap(options.mapSize));

			ShadowCubemap& cubemap = mShadowCubemaps.back();
			cubemap.markAsUsed();
		}

		ShadowCubemap& cubemap = mShadowCubemaps[mapInfo.textureIdx];

		float nearDist = 0.05f;
		float farDist = light->getAttenuationRadius();
		float depthRange = farDist - nearDist;
		float depthBias = getDepthBias(*light, depthRange, options.mapSize);

		Matrix4 proj = Matrix4::projectionPerspective(Degree(90.0f), 1.0f, 0.05f, light->getAttenuationRadius());
		RenderAPI::instance().convertProjectionMatrix(proj, proj);

		ConvexVolume localFrustum(proj);

		gShadowParamsDef.gDepthBias.set(shadowParamsBuffer, depthBias);
		gShadowParamsDef.gDepthRange.set(shadowParamsBuffer, depthRange);
		gShadowParamsDef.gMatViewProj.set(shadowParamsBuffer, Matrix4::IDENTITY);

		Matrix4 viewOffsetMat = Matrix4::translation(-light->getPosition());

		ConvexVolume frustums[6];
		Vector<Plane> boundingPlanes;
		for (UINT32 i = 0; i < 6; i++)
		{
			// Calculate view matrix
			Vector3 forward;
			Vector3 up = Vector3::UNIT_Y;

			switch (i)
			{
			case CF_PositiveX:
				forward = Vector3::UNIT_X;
				break;
			case CF_NegativeX:
				forward = -Vector3::UNIT_X;
				break;
			case CF_PositiveY:
				forward = Vector3::UNIT_Y;
				up = -Vector3::UNIT_Z;
				break;
			case CF_NegativeY:
				forward = Vector3::UNIT_X;
				up = Vector3::UNIT_Z;
				break;
			case CF_PositiveZ:
				forward = Vector3::UNIT_Z;
				break;
			case CF_NegativeZ:
				forward = -Vector3::UNIT_Z;
				break;
			}

			Vector3 right = Vector3::cross(up, forward);
			Matrix3 viewRotationMat = Matrix3(right, up, forward);

			Matrix4 view = Matrix4(viewRotationMat) * viewOffsetMat;
			mapInfo.shadowVPTransforms[i] = proj * view;

			gShadowCubeMatricesDef.gFaceVPMatrices.set(shadowCubeMatricesBuffer, mapInfo.shadowVPTransforms[i], i);

			// Calculate world frustum for culling
			const Vector<Plane>& frustumPlanes = localFrustum.getPlanes();
			Matrix4 worldMatrix = view.transpose();

			Vector<Plane> worldPlanes(frustumPlanes.size());
			UINT32 j = 0;
			for (auto& plane : frustumPlanes)
			{
				worldPlanes[j] = worldMatrix.multiplyAffine(plane);
				j++;
			}

			frustums[i] = ConvexVolume(worldPlanes);

			// Register far plane of all frustums
			boundingPlanes.push_back(worldPlanes.back());
		}

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(cubemap.getTarget());
		rapi.clearRenderTarget(FBT_DEPTH);

		mDepthCubeMat.bind(shadowParamsBuffer, shadowCubeMatricesBuffer);

		// First cull against a global volume
		ConvexVolume boundingVolume(boundingPlanes);
		for (UINT32 i = 0; i < sceneInfo.renderables.size(); i++)
		{
			const Sphere& bounds = sceneInfo.renderableCullInfos[i].bounds.getSphere();
			if (!boundingVolume.intersects(bounds))
				continue;

			scene.prepareRenderable(i, frameInfo);

			for(UINT32 j = 0; j < 6; j++)
			{
				int mask = frustums->intersects(bounds) ? 1 : 0;
				gShadowCubeMasksDef.gFaceMasks.set(shadowCubeMasksBuffer, mask, j);
			}

			RendererObject* renderable = sceneInfo.renderables[i];
			mDepthCubeMat.setPerObjectBuffer(renderable->perObjectParamBuffer, shadowCubeMasksBuffer);

			for (auto& element : renderable->elements)
			{
				if (element.morphVertexDeclaration == nullptr)
					gRendererUtility().draw(element.mesh, element.subMesh);
				else
					gRendererUtility().drawMorph(element.mesh, element.subMesh, element.morphShapeBuffer,
						element.morphVertexDeclaration);
			}
		}

		LightShadows& lightShadows = mRadialLightShadows[options.lightIdx];

		mShadowInfos[lightShadows.startIdx + lightShadows.numShadows] = mapInfo;
		lightShadows.numShadows++;
	}

	void ShadowRendering::calcShadowMapProperties(const RendererLight& light, RendererScene& scene, UINT32& size, 
		SmallVector<float, 4>& fadePercents, float& maxFadePercent) const
	{
		const SceneInfo& sceneInfo = scene.getSceneInfo();

		// Find a view in which the light has the largest radius
		float maxRadiusPercent = 0.0f;
		maxFadePercent = 0.0f;
		for (int i = 0; i < (int)sceneInfo.views.size(); ++i)
		{
			const RendererViewProperties& viewProps = sceneInfo.views[i]->getProperties();

			float viewScaleX = viewProps.projTransform[0][0] * 0.5f;
			float viewScaleY = viewProps.projTransform[1][1] * 0.5f;
			float viewScale = std::max(viewScaleX, viewScaleY);

			const Matrix4& viewVP = viewProps.viewProjTransform;

			Vector4 lightClipPos = viewVP.multiply(Vector4(light.internal->getPosition(), 1.0f));
			float radiusNDC = light.internal->getBounds().getRadius() / std::max(lightClipPos.w, 1.0f);

			// Radius of light bounds in percent of the view surface
			float radiusPercent = radiusNDC * viewScale;
			maxRadiusPercent = std::max(maxRadiusPercent, radiusPercent);

			float optimalMapSize = mShadowMapSize * radiusPercent;
			
			// Determine if the shadow should fade out
			float fadePercent = Math::lerp01(optimalMapSize, (float)MIN_SHADOW_MAP_SIZE, (float)SHADOW_MAP_FADE_SIZE);
			fadePercents.push_back(fadePercent);
			maxFadePercent = std::max(maxFadePercent, fadePercent);
		}

		// If light fully (or nearly fully) covers the screen, use full shadow map resolution, otherwise
		// scale it down to smaller power of two, while clamping to minimal allowed resolution
		float maxOptimalMapSize = mShadowMapSize * maxRadiusPercent;
		UINT32 effectiveMapSize = Bitwise::nextPow2((UINT32)maxOptimalMapSize);
		effectiveMapSize = Math::clamp(effectiveMapSize, MIN_SHADOW_MAP_SIZE, mShadowMapSize);

		// Leave room for border
		size = std::max(effectiveMapSize - 2 * SHADOW_MAP_BORDER, 1u);
	}

	ConvexVolume ShadowRendering::getCSMSplitFrustum(const RendererView& view, const Vector3& lightDir, UINT32 cascade, 
		UINT32 numCascades, Sphere& outBounds)
	{
		// Determine split range
		float splitNear = getCSMSplitDistance(view, cascade, numCascades);
		float splitFar = getCSMSplitDistance(view, cascade + 1, numCascades);

		// Calculate the eight vertices of the split frustum
		auto& viewProps = view.getProperties();

		const Matrix4& projMat = viewProps.projTransform;

		float aspect;
		float nearHalfWidth, nearHalfHeight;
		float farHalfWidth, farHalfHeight;
		if(viewProps.projType == PT_PERSPECTIVE)
		{
			aspect = projMat[0][0] / projMat[1][1];
			float tanHalfFOV = 1.0f / projMat[0][0];

			nearHalfWidth = splitNear * tanHalfFOV;
			nearHalfHeight = nearHalfWidth * aspect;

			farHalfWidth = splitFar * tanHalfFOV;
			farHalfHeight = farHalfWidth * aspect;
		}
		else
		{
			aspect = projMat[0][0] / projMat[1][1];

			nearHalfWidth = farHalfWidth = projMat[0][0] / 4.0f;
			nearHalfHeight = farHalfHeight = projMat[1][1] / 4.0f;
		}

		const Matrix4& viewMat = viewProps.viewTransform;
		Vector3 cameraRight = Vector3(viewMat[0]);
		Vector3 cameraUp = Vector3(viewMat[1]);

		const Vector3& viewOrigin = viewProps.viewOrigin;
		const Vector3& viewDir = viewProps.viewDirection;

		Vector3 frustumVerts[] =
		{
			viewOrigin + viewDir * splitNear - cameraRight * nearHalfWidth + cameraUp * nearHalfHeight, // Near, left, top
			viewOrigin + viewDir * splitNear + cameraRight * nearHalfWidth + cameraUp * nearHalfHeight, // Near, right, top
			viewOrigin + viewDir * splitNear + cameraRight * nearHalfWidth - cameraUp * nearHalfHeight, // Near, right, bottom
			viewOrigin + viewDir * splitNear - cameraRight * nearHalfWidth - cameraUp * nearHalfHeight, // Near, left, bottom
			viewOrigin + viewDir * splitFar - cameraRight * farHalfWidth + cameraUp * farHalfHeight, // Far, left, top
			viewOrigin + viewDir * splitFar + cameraRight * farHalfWidth + cameraUp * farHalfHeight, // Far, right, top
			viewOrigin + viewDir * splitFar + cameraRight * farHalfWidth - cameraUp * farHalfHeight, // Far, right, bottom
			viewOrigin + viewDir * splitFar - cameraRight * farHalfWidth - cameraUp * farHalfHeight, // Far, left, bottom
		};

		// Calculate the bounding sphere of the frustum
		float diagonalNearSq = nearHalfWidth * nearHalfWidth + nearHalfHeight * nearHalfHeight;
		float diagonalFarSq = farHalfWidth * farHalfWidth + farHalfHeight * farHalfHeight;

		float length = splitFar - splitNear;
		float offset = (diagonalNearSq - diagonalFarSq) / 2 * length + length * 0.5f;
		float distToCenter = Math::clamp(splitFar - offset, splitNear, splitFar);

		Vector3 center = viewOrigin + viewDir * distToCenter;

		float radius = 0.0f;
		for (auto& entry : frustumVerts)
			radius = std::max(radius, center.squaredDistance(entry));

		radius = std::max(sqrt(radius), 1.0f);
		outBounds = Sphere(center, radius);

		// Generate light frustum planes
		Plane viewPlanes[6];
		viewPlanes[FRUSTUM_PLANE_NEAR] = Plane(frustumVerts[0], frustumVerts[1], frustumVerts[2]);
		viewPlanes[FRUSTUM_PLANE_FAR] = Plane(frustumVerts[5], frustumVerts[4], frustumVerts[7]);
		viewPlanes[FRUSTUM_PLANE_LEFT] = Plane(frustumVerts[4], frustumVerts[0], frustumVerts[3]);
		viewPlanes[FRUSTUM_PLANE_RIGHT] = Plane(frustumVerts[1], frustumVerts[5], frustumVerts[6]);
		viewPlanes[FRUSTUM_PLANE_TOP] = Plane(frustumVerts[4], frustumVerts[5], frustumVerts[1]);
		viewPlanes[FRUSTUM_PLANE_BOTTOM] = Plane(frustumVerts[3], frustumVerts[2], frustumVerts[6]);

		Vector<Plane> lightVolume;

		//// Add camera's planes facing towards the lights (forming the back of the volume)
		for(auto& entry : viewPlanes)
		{
			if (entry.normal.dot(lightDir) < 0.0f)
				lightVolume.push_back(entry);
		}

		//// Determine edge planes by testing adjacent planes with different facing
		////// Pairs of frustum planes that share an edge
		UINT32 adjacentPlanes[][2] =
		{
			{ FRUSTUM_PLANE_NEAR, FRUSTUM_PLANE_LEFT },
			{ FRUSTUM_PLANE_NEAR, FRUSTUM_PLANE_RIGHT },
			{ FRUSTUM_PLANE_NEAR, FRUSTUM_PLANE_TOP },
			{ FRUSTUM_PLANE_NEAR, FRUSTUM_PLANE_BOTTOM },

			{ FRUSTUM_PLANE_FAR, FRUSTUM_PLANE_LEFT },
			{ FRUSTUM_PLANE_FAR, FRUSTUM_PLANE_RIGHT },
			{ FRUSTUM_PLANE_FAR, FRUSTUM_PLANE_TOP },
			{ FRUSTUM_PLANE_FAR, FRUSTUM_PLANE_BOTTOM },

			{ FRUSTUM_PLANE_LEFT, FRUSTUM_PLANE_TOP },
			{ FRUSTUM_PLANE_TOP, FRUSTUM_PLANE_RIGHT },
			{ FRUSTUM_PLANE_RIGHT, FRUSTUM_PLANE_BOTTOM },
			{ FRUSTUM_PLANE_BOTTOM, FRUSTUM_PLANE_LEFT },
		};

		////// Vertex indices of edges on the boundary between two planes
		UINT32 sharedEdges[][2] =
		{
			{ 3, 0 },{ 1, 2 },{ 0, 1 },{ 2, 3 },
			{ 4, 7 },{ 6, 5 },{ 5, 4 },{ 7, 6 },
			{ 4, 0 },{ 5, 1 },{ 6, 2 },{ 7, 3 }
		};

		for(UINT32 i = 0; i < 12; i++)
		{
			const Plane& planeA = viewPlanes[adjacentPlanes[i][0]];
			const Plane& planeB = viewPlanes[adjacentPlanes[i][1]];

			float dotA = planeA.normal.dot(lightDir);
			float dotB = planeB.normal.dot(lightDir);

			if((dotA * dotB) < 0.0f)
			{
				const Vector3& vertA = frustumVerts[sharedEdges[i][0]];
				const Vector3& vertB = frustumVerts[sharedEdges[i][1]];
				Vector3 vertC = vertA + lightDir;

				if (dotA >= 0.0f)
					lightVolume.push_back(Plane(vertA, vertB, vertC));
				else
					lightVolume.push_back(Plane(vertB, vertA, vertC));
			}
		}

		return ConvexVolume(lightVolume);
	}

	float ShadowRendering::getCSMSplitDistance(const RendererView& view, UINT32 index, UINT32 numCascades)
	{
		// Determines the size of each subsequent cascade split. Value of 1 means the cascades will be linearly split.
		// Value of 2 means each subsequent split will be twice the size of the previous one. Valid range is roughly
		// [1, 4].
		// Note: Make this an adjustable property?
		const static float DISTRIBUTON_EXPONENT = 1.0f;

		// First determine the scale of the split, relative to the entire range
		float scaleModifier = 1.0f;
		float scale = 0.0f;
		float totalScale = 0.0f;

		//// Split 0 corresponds to near plane
		if (index > 0)
		{
			for (UINT32 i = 0; i < numCascades; i++)
			{
				if (i < index)
					scale += scaleModifier;

				totalScale += scaleModifier;
				scaleModifier *= DISTRIBUTON_EXPONENT;
			}
		}

		scale = scale / totalScale;

		// Calculate split distance in Z
		auto& viewProps = view.getProperties();
		float near = viewProps.nearPlane;
		float far = viewProps.farPlane;

		return near + (far - near) * scale;
	}

	float ShadowRendering::getDepthBias(const Light& light, float depthRange, UINT32 mapSize)
	{
		const static float RADIAL_LIGHT_BIAS = 0.05f;
		const static float SPOT_DEPTH_BIAS = 1.0f;
		const static float DIR_DEPTH_BIAS = 5.0f;
		const static float DEFAULT_RESOLUTION = 512.0f;
		
		// Increase bias if map size smaller than some resolution
		float resolutionScale;
		
		if (light.getType() == LightType::Directional)
			resolutionScale = light.getBounds().getRadius() / (float)mapSize;
		else
			resolutionScale = DEFAULT_RESOLUTION / (float)mapSize;

		// Decrease bias with larger depth range
		float rangeScale = 1.0f / depthRange;
		
		float defaultBias = 1.0f;
		switch(light.getType())
		{
		case LightType::Directional: 
			defaultBias = DIR_DEPTH_BIAS;
			break;
		case LightType::Radial: 
			defaultBias = RADIAL_LIGHT_BIAS;
			break;
		case LightType::Spot: 
			defaultBias = SPOT_DEPTH_BIAS;
			break;
		}
		
		return defaultBias * light.getShadowBias() *resolutionScale * rangeScale;
	}
}}
