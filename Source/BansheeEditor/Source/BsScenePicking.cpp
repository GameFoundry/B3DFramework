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
#include "BsBlendState.h"
#include "BsDepthStencilState.h"
#include "BsRasterizerState.h"
#include "BsRenderTarget.h"
#include "BsPixelData.h"
#include "BsGpuParams.h"
#include "BsBuiltinEditorResources.h"
#include "BsShader.h"
#include "BsCoreRenderer.h"
#include "BsGizmoManager.h"
#include "BsRendererUtility.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	const float ScenePickingCore::ALPHA_CUTOFF = 0.5f;

	ScenePicking::ScenePicking()
	{
		mCore = bs_new<ScenePickingCore>();

		for (UINT32 i = 0; i < 3; i++)
		{
			HMaterial matPicking = BuiltinEditorResources::instance().createPicking((CullingMode)i);
			HMaterial matPickingAlpha = BuiltinEditorResources::instance().createPickingAlpha((CullingMode)i);

			mCore->mMaterialData[i].mMatPickingCore = matPicking->getCore();
			mCore->mMaterialData[i].mMatPickingAlphaCore = matPickingAlpha->getCore();
		}

		gCoreAccessor().queueCommand(std::bind(&ScenePickingCore::initialize, mCore));
	}

	ScenePicking::~ScenePicking()
	{
		gCoreAccessor().queueCommand(std::bind(&ScenePickingCore::destroy, mCore));
	}

	HSceneObject ScenePicking::pickClosestObject(const SPtr<Camera>& cam, const Vector2I& position, const Vector2I& area)
	{
		Vector<HSceneObject> selectedObjects = pickObjects(cam, position, area);
		if (selectedObjects.size() == 0)
			return HSceneObject();

		return selectedObjects[0];
	}

	Vector<HSceneObject> ScenePicking::pickObjects(const SPtr<Camera>& cam, const Vector2I& position, const Vector2I& area)
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

			Bounds worldBounds = mesh->getProperties().getBounds();
			Matrix4 worldTransform = so->getWorldTfrm();
			worldBounds.transformAffine(worldTransform);

			// TODO - I could limit the frustum to the visible area we're rendering for a speed boost
			// but this is unlikely to be a performance bottleneck
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
						{
							const Map<String, SHADER_OBJECT_PARAM_DESC>& textureParams = originalMat->getShader()->getTextureParams();
							for (auto& objectParam : textureParams)
							{
								if (objectParam.second.rendererSemantic == RPS_Diffuse)
								{
									mainTexture = originalMat->getTexture(objectParam.first);
									break;
								}
							}
						}

						idxToRenderable[idx] = so;

						Matrix4 wvpTransform = viewProjMatrix * worldTransform;
						pickData.insert({ mesh->getCore(), idx, wvpTransform, useAlphaShader, cullMode, mainTexture });
					}
				}
			}
		}

		UINT32 firstGizmoIdx = (UINT32)pickData.size();

		SPtr<RenderTargetCore> target = cam->getViewport()->getTarget()->getCore();
		gCoreAccessor().queueCommand(std::bind(&ScenePickingCore::corePickingBegin, mCore, target,
			cam->getViewport()->getNormArea(), std::cref(pickData), position, area));

		GizmoManager::instance().renderForPicking(cam, [&](UINT32 inputIdx) { return encodeIndex(firstGizmoIdx + inputIdx); });

		AsyncOp op = gCoreAccessor().queueReturnCommand(std::bind(&ScenePickingCore::corePickingEnd, mCore, target, 
			cam->getViewport()->getNormArea(), position, area, _1));
		gCoreAccessor().submitToCoreThread(true);

		assert(op.hasCompleted());

		Vector<UINT32> selectedObjects = op.getReturnValue<Vector<UINT32>>();
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

	void ScenePickingCore::initialize()
	{
		for (UINT32 i = 0; i < 3; i++)
		{
			MaterialData& md = mMaterialData[i];

			{
				SPtr<PassParametersCore> passParams = md.mMatPickingCore->getPassParameters(0);

				md.mParamPickingVertParams = passParams->mVertParams;
				md.mParamPickingVertParams->getParam("matWorldViewProj", md.mParamPickingWVP);

				md.mParamPickingFragParams = passParams->mFragParams;
				md.mParamPickingFragParams->getParam("colorIndex", md.mParamPickingColor);
			}

			{
				SPtr<PassParametersCore> passParams = md.mMatPickingAlphaCore->getPassParameters(0);

				md.mParamPickingAlphaVertParams = passParams->mVertParams;
				md.mParamPickingAlphaVertParams->getParam("matWorldViewProj", md.mParamPickingAlphaWVP);

				md.mParamPickingAlphaFragParams = passParams->mFragParams;

				md.mParamPickingAlphaFragParams->getParam("colorIndex", md.mParamPickingAlphaColor);
				md.mParamPickingAlphaFragParams->getTextureParam("mainTexture", md.mParamPickingAlphaTexture);

				GpuParamFloatCore alphaCutoffParam;
				md.mParamPickingAlphaFragParams->getParam("alphaCutoff", alphaCutoffParam);
				alphaCutoffParam.set(ALPHA_CUTOFF);
			}
		}
	}

	void ScenePickingCore::destroy()
	{
		bs_delete(this);
	}

	void ScenePickingCore::corePickingBegin(const SPtr<RenderTargetCore>& target, const Rect2& viewportArea,
		const ScenePicking::RenderableSet& renderables, const Vector2I& position, const Vector2I& area)
	{
		RenderAPICore& rs = RenderAPICore::instance();

		rs.beginFrame();
		rs.setRenderTarget(target);
		rs.setViewport(viewportArea);
		rs.clearRenderTarget(FBT_COLOR | FBT_DEPTH | FBT_STENCIL, Color::White);
		rs.setScissorRect(position.x, position.y, position.x + area.x, position.y + area.y);

		gRendererUtility().setPass(mMaterialData[0].mMatPickingCore, 0);
		bool activeMaterialIsAlpha = false;
		CullingMode activeMaterialCull = (CullingMode)0;

		for (auto& renderable : renderables)
		{
			if (activeMaterialIsAlpha != renderable.alpha || activeMaterialCull != renderable.cullMode)
			{
				activeMaterialIsAlpha = renderable.alpha;
				activeMaterialCull = renderable.cullMode;

				if (activeMaterialIsAlpha)
					gRendererUtility().setPass(mMaterialData[(UINT32)activeMaterialCull].mMatPickingAlphaCore, 0);
				else
					gRendererUtility().setPass(mMaterialData[(UINT32)activeMaterialCull].mMatPickingCore, 0);
			}

			Color color = ScenePicking::encodeIndex(renderable.index);
			MaterialData& md = mMaterialData[(UINT32)activeMaterialCull];

			if (activeMaterialIsAlpha)
			{
				md.mParamPickingAlphaWVP.set(renderable.wvpTransform);
				md.mParamPickingAlphaColor.set(color);
				md.mParamPickingAlphaTexture.set(renderable.mainTexture->getCore());

				gRendererUtility().setGpuParams(GPT_VERTEX_PROGRAM, md.mParamPickingAlphaVertParams);
				gRendererUtility().setGpuParams(GPT_FRAGMENT_PROGRAM, md.mParamPickingAlphaFragParams);
			}
			else
			{
				md.mParamPickingWVP.set(renderable.wvpTransform);
				md.mParamPickingColor.set(color);

				gRendererUtility().setGpuParams(GPT_VERTEX_PROGRAM, md.mParamPickingVertParams);
				gRendererUtility().setGpuParams(GPT_FRAGMENT_PROGRAM, md.mParamPickingFragParams);
			}

			UINT32 numSubmeshes = renderable.mesh->getProperties().getNumSubMeshes();

			for (UINT32 i = 0; i < numSubmeshes; i++)
				gRendererUtility().draw(renderable.mesh, renderable.mesh->getProperties().getSubMesh(i));
		}
	}

	void ScenePickingCore::corePickingEnd(const SPtr<RenderTargetCore>& target, const Rect2& viewportArea, const Vector2I& position,
		const Vector2I& area, AsyncOp& asyncOp)
	{
		const RenderTargetProperties& rtProps = target->getProperties();

		if (rtProps.isWindow())
		{
			// TODO: When I do implement this then I will likely want a method in RenderTarget that unifies both render window and render texture readback
			BS_EXCEPT(NotImplementedException, "Picking is not supported on render windows as framebuffer readback methods aren't implemented");
		}

		SPtr<RenderTextureCore> rtt = std::static_pointer_cast<RenderTextureCore>(target);
		SPtr<TextureCore> outputTexture = rtt->getBindableColorTexture();

		if (position.x < 0 || position.x >= (INT32)outputTexture->getProperties().getWidth() ||
			position.y < 0 || position.y >= (INT32)outputTexture->getProperties().getHeight())
		{
			asyncOp._completeOperation(Vector<UINT32>());
			return;
		}

		SPtr<PixelData> outputPixelData = outputTexture->getProperties().allocateSubresourceBuffer(0);
		AsyncOp unused;

		RenderAPICore& rs = RenderAPICore::instance();

		rs.endFrame();
		outputTexture->readSubresource(0, *outputPixelData);

		Map<UINT32, UINT32> selectionScores;
		UINT32 maxWidth = std::min((UINT32)(position.x + area.x), outputPixelData->getWidth());
		UINT32 maxHeight = std::min((UINT32)(position.y + area.y), outputPixelData->getHeight());

		if (rtProps.requiresTextureFlipping())
		{
			UINT32 vertOffset = outputPixelData->getHeight() - 1;

			for (UINT32 y = maxHeight; y > (UINT32)position.y; y--)
			{
				for (UINT32 x = (UINT32)position.x; x < maxWidth; x++)
				{
					Color color = outputPixelData->getColorAt(x, vertOffset - y);
					UINT32 index = ScenePicking::decodeIndex(color);

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
					UINT32 index = ScenePicking::decodeIndex(color);

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

		Vector<UINT32> results;
		for (auto& selectedObject : selectedObjects)
			results.push_back(selectedObject.index);

		asyncOp._completeOperation(results);
	}
}