//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScenePicking.h"
#include "BsSceneManager.h"
#include "BsColor.h"
#include "BsMatrix4.h"
#include "BsDebug.h"
#include "BsMath.h"
#include "BsCRenderable.h"
#include "BsSceneObject.h"
#include "BsMesh.h"
#include "BsConvexVolume.h"
#include "BsCCamera.h"
#include "BsCoreThread.h"
#include "BsRenderAPI.h"
#include "BsMaterial.h"
#include "BsPass.h"
#include "BsRasterizerState.h"
#include "BsRenderTexture.h"
#include "BsPixelData.h"
#include "BsGpuParams.h"
#include "BsGpuParamsSet.h"
#include "BsBuiltinEditorResources.h"
#include "BsShader.h"
#include "BsRenderer.h"
#include "BsGizmoManager.h"
#include "BsRendererUtility.h"

using namespace std::placeholders;

namespace bs
{
	ScenePicking::ScenePicking()
	{
		mCore = bs_new<ct::ScenePicking>();

		for (UINT32 i = 0; i < 3; i++)
		{
			HMaterial matPicking = BuiltinEditorResources::instance().createPicking((CullingMode)i);
			HMaterial matPickingAlpha = BuiltinEditorResources::instance().createPickingAlpha((CullingMode)i);

			mCore->mMaterials[i] = matPicking->getCore();
			mCore->mMaterials[3 + i] = matPickingAlpha->getCore();
		}

		gCoreThread().queueCommand(std::bind(&ct::ScenePicking::initialize, mCore));
	}

	ScenePicking::~ScenePicking()
	{
		gCoreThread().queueCommand(std::bind(&ct::ScenePicking::destroy, mCore));
	}

	HSceneObject ScenePicking::pickClosestObject(const SPtr<Camera>& cam, const Vector2I& position, const Vector2I& area, 
		Vector<HSceneObject>& ignoreRenderables, SnapData* data)
	{
		Vector<HSceneObject> selectedObjects = pickObjects(cam, position, area, ignoreRenderables, data);
		if (selectedObjects.size() == 0)
			return HSceneObject();
			
		if (data != nullptr)
		{
			Matrix3 rotation;
			selectedObjects[0]->getWorldRotation().toRotationMatrix(rotation);
			data->normal = rotation.inverse().transpose().transform(data->normal);
		}
		
		return selectedObjects[0];
	}

	Vector<HSceneObject> ScenePicking::pickObjects(const SPtr<Camera>& cam, const Vector2I& position, const Vector2I& area, 
		Vector<HSceneObject>& ignoreRenderables, SnapData* data)
	{
		auto comparePickElement = [&] (const ScenePicking::RenderablePickData& a, const ScenePicking::RenderablePickData& b)
		{
			// Sort by alpha setting first, then by cull mode, then by index
			if (a.alpha == b.alpha)
			{
				if (a.cullMode == b.cullMode)
					return a.index > b.index;
				else
					return (UINT32)a.cullMode > (UINT32)b.cullMode;
			}
			else
				return (UINT32)a.alpha > (UINT32)b.alpha;
		};

		Matrix4 viewProjMatrix = cam->getProjectionMatrixRS() * cam->getViewMatrix();

		const Map<Renderable*, SceneRenderableData>& renderables = SceneManager::instance().getAllRenderables();
		RenderableSet pickData(comparePickElement);
		Map<UINT32, HSceneObject> idxToRenderable;

		for (auto& renderableData : renderables)
		{
			SPtr<Renderable> renderable = renderableData.second.renderable;
			HSceneObject so = renderableData.second.sceneObject;

			if (!so->getActive())
				continue;

			HMesh mesh = renderable->getMesh();
			if (!mesh.isLoaded())
				continue;

			bool found = false;
			for (int i = 0; i < ignoreRenderables.size(); i++)
			{
				if (ignoreRenderables[i] == so)
				{
					found = true;
					break;
				}
			}
			
			if (found)
				continue;

			Bounds worldBounds = mesh->getProperties().getBounds();
			Matrix4 worldTransform = so->getWorldTfrm();
			worldBounds.transformAffine(worldTransform);

			const ConvexVolume& frustum = cam->getWorldFrustum();
			if (frustum.intersects(worldBounds.getSphere()))
			{
				// More precise with the box
				if (frustum.intersects(worldBounds.getBox()))
				{
					for (UINT32 i = 0; i < mesh->getProperties().getNumSubMeshes(); i++)
					{
						UINT32 idx = (UINT32)pickData.size();

						bool useAlphaShader = false;
						SPtr<RasterizerState> rasterizerState;

						HMaterial originalMat = renderable->getMaterial(i);
						if (originalMat != nullptr && originalMat->getNumPasses() > 0)
						{
							SPtr<Pass> firstPass = originalMat->getPass(0); // Note: We only ever check the first pass, problem?
							useAlphaShader = firstPass->hasBlending();

							if (firstPass->getRasterizerState() == nullptr)
								rasterizerState = RasterizerState::getDefault();
							else
								rasterizerState = firstPass->getRasterizerState();
						}
						else
							rasterizerState = RasterizerState::getDefault();

						CullingMode cullMode = rasterizerState->getProperties().getCullMode();

						HTexture mainTexture;
						if (useAlphaShader)
							mainTexture = originalMat->getTexture("gAlbedoTex");

						idxToRenderable[idx] = so;

						Matrix4 wvpTransform = viewProjMatrix * worldTransform;
						pickData.insert({ mesh->getCore(), idx, wvpTransform, useAlphaShader, cullMode, mainTexture });
					}
				}
			}
		}

		UINT32 firstGizmoIdx = (UINT32)pickData.size();

		SPtr<ct::RenderTarget> target = cam->getViewport()->getTarget()->getCore();
		gCoreThread().queueCommand(std::bind(&ct::ScenePicking::corePickingBegin, mCore, target,
			cam->getViewport()->getNormArea(), std::cref(pickData), position, area));

		GizmoManager::instance().renderForPicking(cam, [&](UINT32 inputIdx) { return encodeIndex(firstGizmoIdx + inputIdx); });

		AsyncOp op = gCoreThread().queueReturnCommand(std::bind(&ct::ScenePicking::corePickingEnd, mCore, target,
			cam->getViewport()->getNormArea(), position, area, data != nullptr, _1));
		gCoreThread().submit(true);

		assert(op.hasCompleted());

		PickResults pickResults = op.getReturnValue<PickResults>();
		if (data != nullptr)
		{
			data->pickPosition = cam->screenToWorldPointDeviceDepth(position, pickResults.depth);
			data->normal = pickResults.normal;
		}

		Vector<UINT32> selectedObjects = pickResults.objects;
		Vector<HSceneObject> results;

		for (auto& selectedObjectIdx : selectedObjects)
		{
			if (selectedObjectIdx < firstGizmoIdx)
			{
				auto iterFind = idxToRenderable.find(selectedObjectIdx);

				if (iterFind != idxToRenderable.end())
					results.push_back(iterFind->second);
			}
			else
			{
				UINT32 gizmoIdx = selectedObjectIdx - firstGizmoIdx;

				HSceneObject so = GizmoManager::instance().getSceneObject(gizmoIdx);
				if (so)
					results.push_back(so);
			}
		}

		return results;
	}

	Color ScenePicking::encodeIndex(UINT32 index)
	{
		Color encoded;
		encoded.r = (index & 0xFF) / 255.0f;
		encoded.g = ((index >> 8) & 0xFF) / 255.0f;
		encoded.b = ((index >> 16) & 0xFF) / 255.0f;
		encoded.a = 1.0f;

		if (((index >> 24) & 0xFF))
			LOGERR("Index when picking out of valid range.");

		return encoded;
	}

	UINT32 ScenePicking::decodeIndex(Color color)
	{
		UINT32 r = Math::roundToInt(color.r * 255.0f);
		UINT32 g = Math::roundToInt(color.g * 255.0f);
		UINT32 b = Math::roundToInt(color.b * 255.0f);

		return (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16);
	}

	namespace ct
	{
	const float ScenePicking::ALPHA_CUTOFF = 0.5f;

	PickingParamBlockDef gPickingParamBlockDef;

	void ScenePicking::initialize()
	{
		// Do nothing
	}

	void ScenePicking::destroy()
	{
		bs_delete(this);
	}

	void ScenePicking::corePickingBegin(const SPtr<RenderTarget>& target, const Rect2& viewportArea,
		const bs::ScenePicking::RenderableSet& renderables, const Vector2I& position, const Vector2I& area)
	{
		RenderAPI& rs = RenderAPI::instance();

		SPtr<RenderTexture> rtt = std::static_pointer_cast<RenderTexture>(target);

		SPtr<Texture> outputTexture = rtt->getColorTexture(0);
		TextureProperties outputTextureProperties = outputTexture->getProperties();

		TEXTURE_DESC normalTexDesc;
		normalTexDesc.type = TEX_TYPE_2D;
		normalTexDesc.width = outputTextureProperties.getWidth();
		normalTexDesc.height = outputTextureProperties.getHeight();
		normalTexDesc.format = PF_RG11B10F;
		normalTexDesc.usage = TU_RENDERTARGET;

		SPtr<Texture> normalsTexture = Texture::create(normalTexDesc);
		SPtr<Texture> depthTexture = rtt->getDepthStencilTexture();

		RENDER_TEXTURE_DESC pickingMRT;
		pickingMRT.colorSurfaces[0].face = 0;
		pickingMRT.colorSurfaces[0].texture = outputTexture;
		pickingMRT.colorSurfaces[1].face = 0;
		pickingMRT.colorSurfaces[1].texture = normalsTexture;

		pickingMRT.depthStencilSurface.face = 0;
		pickingMRT.depthStencilSurface.texture = depthTexture;
		
		mPickingTexture = RenderTexture::create(pickingMRT);

		rs.setRenderTarget(mPickingTexture);
		rs.setViewport(viewportArea);
		rs.clearRenderTarget(FBT_COLOR | FBT_DEPTH | FBT_STENCIL, Color::White);
		rs.setScissorRect(position.x, position.y, position.x + area.x, position.y + area.y);

		gRendererUtility().setPass(mMaterials[0]);

		UINT32 numEntries = (UINT32)renderables.size();
		UINT32* renderableIndices = bs_stack_alloc<UINT32>(numEntries);

		UINT32 typeCounters[6];
		bs_zero_out(typeCounters);

		UINT32 idx = 0;
		for (auto& renderable : renderables)
		{
			UINT32 typeIdx;
			if (!renderable.alpha)
				typeIdx = 0;
			else
				typeIdx = 3;

			typeIdx += (UINT32)renderable.cullMode;

			UINT32 renderableIdx = typeCounters[typeIdx];
			renderableIndices[idx] = renderableIdx;

			SPtr<GpuParamsSet> paramsSet;
			if (renderableIdx >= mParamSets[typeIdx].size())
			{
				paramsSet = mMaterials[typeIdx]->createParamsSet();
				mParamSets[typeIdx].push_back(paramsSet);
			}
			else
				paramsSet = mParamSets[typeIdx][renderableIdx];

			SPtr<GpuParamBlockBuffer> paramBuffer;
			if (idx >= mParamBuffers.size())
			{
				paramBuffer = gPickingParamBlockDef.createBuffer();
				mParamBuffers.push_back(paramBuffer);
			}
			else
				paramBuffer = mParamBuffers[idx];

			paramsSet->setParamBlockBuffer("Uniforms", paramBuffer, true);

			Color color = bs::ScenePicking::encodeIndex(renderable.index);

			gPickingParamBlockDef.gMatViewProj.set(paramBuffer, renderable.wvpTransform);
			gPickingParamBlockDef.gAlphaCutoff.set(paramBuffer, ALPHA_CUTOFF);
			gPickingParamBlockDef.gColorIndex.set(paramBuffer, color);

			typeCounters[typeIdx]++;
			idx++;
		}

		UINT32 activeMaterialIdx = 0;
		idx = 0;
		for (auto& renderable : renderables)
		{
			UINT32 typeIdx;
			if (!renderable.alpha)
				typeIdx = 0;
			else
				typeIdx = 3;

			typeIdx += (UINT32)renderable.cullMode;

			if (activeMaterialIdx != typeIdx)
			{
				gRendererUtility().setPass(mMaterials[typeIdx]);
				activeMaterialIdx = typeIdx;
			}

			UINT32 renderableIdx = renderableIndices[idx];
			gRendererUtility().setPassParams(mParamSets[typeIdx][renderableIdx]);

			UINT32 numSubmeshes = renderable.mesh->getProperties().getNumSubMeshes();

			for (UINT32 i = 0; i < numSubmeshes; i++)
				gRendererUtility().draw(renderable.mesh, renderable.mesh->getProperties().getSubMesh(i));

			idx++;
		}

		bs_stack_free(renderableIndices);
	}

	void ScenePicking::corePickingEnd(const SPtr<RenderTarget>& target, const Rect2& viewportArea, 
		const Vector2I& position, const Vector2I& area, bool gatherSnapData, AsyncOp& asyncOp)
	{
		const RenderTargetProperties& rtProps = target->getProperties();
		RenderAPI& rs = RenderAPI::instance();

		rs.setRenderTarget(nullptr);
		rs.submitCommandBuffer(nullptr);

		if (rtProps.isWindow())
		{
			BS_EXCEPT(NotImplementedException, "Picking is not supported on render windows as framebuffer readback methods aren't implemented");
		}

		SPtr<Texture> outputTexture = mPickingTexture->getColorTexture(0);
		SPtr<Texture> normalsTexture = mPickingTexture->getColorTexture(1);
		SPtr<Texture> depthTexture = mPickingTexture->getDepthStencilTexture();

		if (position.x < 0 || position.x >= (INT32)outputTexture->getProperties().getWidth() ||
			position.y < 0 || position.y >= (INT32)outputTexture->getProperties().getHeight())
		{
			mPickingTexture = nullptr;

			asyncOp._completeOperation(Vector<UINT32>());
			return;
		}

		SPtr<PixelData> outputPixelData = outputTexture->getProperties().allocBuffer(0, 0);
		SPtr<PixelData> normalsPixelData;
		SPtr<PixelData> depthPixelData;
		if (gatherSnapData)
		{
			normalsPixelData = normalsTexture->getProperties().allocBuffer(0, 0);
			depthPixelData = depthTexture->getProperties().allocBuffer(0, 0);
		}

		outputTexture->readData(*outputPixelData);

		Map<UINT32, UINT32> selectionScores;
		UINT32 maxWidth = std::min((UINT32)(position.x + area.x), outputPixelData->getWidth());
		UINT32 maxHeight = std::min((UINT32)(position.y + area.y), outputPixelData->getHeight());

		if (rtProps.requiresTextureFlipping())
		{
			UINT32 vertOffset = outputPixelData->getHeight();

			for (UINT32 y = maxHeight; y > (UINT32)position.y; y--)
			{
				for (UINT32 x = (UINT32)position.x; x < maxWidth; x++)
				{
					Color color = outputPixelData->getColorAt(x, vertOffset - y);
					UINT32 index = bs::ScenePicking::decodeIndex(color);

					if (index == 0x00FFFFFF) // Nothing selected
						continue;

					auto iterFind = selectionScores.find(index);
					if (iterFind == selectionScores.end())
						selectionScores[index] = 1;
					else
						iterFind->second++;
				}
			}
		}
		else
		{
			for (UINT32 y = (UINT32)position.y; y < maxHeight; y++)
			{
				for (UINT32 x = (UINT32)position.x; x < maxWidth; x++)
				{
					Color color = outputPixelData->getColorAt(x, y);
					UINT32 index = bs::ScenePicking::decodeIndex(color);

					if (index == 0x00FFFFFF) // Nothing selected
						continue;

					auto iterFind = selectionScores.find(index);
					if (iterFind == selectionScores.end())
						selectionScores[index] = 1;
					else
						iterFind->second++;
				}
			}
		}

		// Sort by score
		struct SelectedObject { UINT32 index; UINT32 score; };

		Vector<SelectedObject> selectedObjects(selectionScores.size());
		UINT32 idx = 0;
		for (auto& selectionScore : selectionScores)
		{
			selectedObjects[idx++] = { selectionScore.first, selectionScore.second };
		}

		std::sort(selectedObjects.begin(), selectedObjects.end(),
			[&](const SelectedObject& a, const SelectedObject& b)
		{
			return b.score < a.score;
		});

		Vector<UINT32> objects;
		for (auto& selectedObject : selectedObjects)
			objects.push_back(selectedObject.index);
		
		PickResults result;
		if (gatherSnapData)
		{
			depthTexture->readData(*depthPixelData);
			normalsTexture->readData(*normalsPixelData);

			Vector2I samplePixel = position;
			if (rtProps.requiresTextureFlipping())
				samplePixel.y = depthPixelData->getHeight() - samplePixel.y;

			float depth = depthPixelData->getDepthAt(samplePixel.x, samplePixel.y);
			Color normal = normalsPixelData->getColorAt(samplePixel.x, samplePixel.y);

			const RenderAPIInfo& rapiInfo = rs.getAPIInfo();
			float max = rapiInfo.getMaximumDepthInputValue();
			float min = rapiInfo.getMinimumDepthInputValue();
			depth = depth * Math::abs(max - min) + min;

			result.depth = depth;
			result.normal = Vector3((normal.r * 2) - 1, (normal.g * 2) - 1, (normal.b * 2) - 1);
		}

		mPickingTexture = nullptr;
		
		result.objects = objects;
		asyncOp._completeOperation(result);
	}
	}
}
