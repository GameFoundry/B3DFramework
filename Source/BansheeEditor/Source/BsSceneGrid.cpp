//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSceneGrid.h"
#include "BsMath.h"
#include "BsShapeMeshes3D.h"
#include "BsVertexDataDesc.h"
#include "BsMaterial.h"
#include "BsMesh.h"
#include "BsBuiltinEditorResources.h"
#include "BsCCamera.h"
#include "BsRect3.h"
#include "BsCoreThread.h"
#include "BsEditorSettings.h"
#include "BsRendererManager.h"
#include "BsRenderer.h"
#include "BsRendererUtility.h"

namespace BansheeEngine
{
	const Color SceneGridCore::GRID_LINE_COLOR = Color(0.5f, 0.5f, 0.5f);
	const float SceneGridCore::LINE_WIDTH = 0.025f;
	const float SceneGridCore::LINE_BORDER_WIDTH = 0.00075f;
	const float SceneGridCore::FADE_OUT_START = 5.0f;
	const float SceneGridCore::FADE_OUT_END = 40.0f;

	SceneGrid::SceneGrid(const SPtr<Camera>& camera)
		:mCoreDirty(true), mCore(nullptr)
	{
		mVertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		mVertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);
		mVertexDesc->addVertElem(VET_FLOAT3, VES_NORMAL);

		mCore.store(bs_new<SceneGridCore>(), std::memory_order_release);

		HMaterial gridMaterial = BuiltinEditorResources::instance().createSceneGridMaterial();
		SPtr<MaterialCore> materialCore = gridMaterial->getCore();
		gCoreAccessor().queueCommand(std::bind(&SceneGrid::initializeCore, this, camera->getCore(), materialCore));

		updateGridMesh();
	}

	SceneGrid::~SceneGrid()
	{
		gCoreAccessor().queueCommand(std::bind(&SceneGrid::destroyCore, this, mCore.load(std::memory_order_relaxed)));
	}

	void SceneGrid::initializeCore(const SPtr<CameraCore>& camera, const SPtr<MaterialCore>& material)
	{
		THROW_IF_NOT_CORE_THREAD;

		mCore.load()->initialize(camera, material);
	}

	void SceneGrid::destroyCore(SceneGridCore* core)
	{
		THROW_IF_NOT_CORE_THREAD;

		bs_delete(core);
	}

	void SceneGrid::setSize(UINT32 size)
	{
		if (mSize != size)
		{
			mSize = size;
			updateGridMesh();
		}
	}

	void SceneGrid::setSpacing(float spacing)
	{
		if (mSpacing != spacing)
		{
			mSpacing = spacing;
			mCoreDirty = true;
		}
	}

	void SceneGrid::setMode(GridMode mode)
	{
		if(mMode != mode)
		{
			mMode = mode;
			updateGridMesh();
		}
	}

	void SceneGrid::setSettings(const SPtr<EditorSettings>& settings)
	{
		mSettings = settings;
		updateFromEditorSettings();
	}

	void SceneGrid::_update()
	{
		if (mSettings != nullptr && mSettingsHash != mSettings->getHash())
			updateFromEditorSettings();

		if (mCoreDirty)
		{
			Vector3 gridPlaneNormal = Vector3(0.0f, 1.0f, 0.0f);

			switch (mMode)
			{
			case GridMode::OrthoX:
			case GridMode::OrthoNegX:
				gridPlaneNormal = Vector3(1.0f, 0.0f, 0.0f);
				break;
			case GridMode::OrthoZ:
			case GridMode::OrthoNegZ:
				gridPlaneNormal = Vector3(0.0f, 0.0f, 1.0f);
				break;
			default:
				break;
			}

			SceneGridCore* core = mCore.load(std::memory_order_relaxed);
			gCoreAccessor().queueCommand(
				std::bind(&SceneGridCore::updateData, core, mGridMesh->getCore(), mSpacing, 
				mMode == GridMode::Perspective, gridPlaneNormal));

			mCoreDirty = false;
		}
	}

	void SceneGrid::updateFromEditorSettings()
	{
		setSize(mSettings->getGridSize());
		setSpacing(mSettings->getGridSpacing());

		mSettingsHash = mSettings->getHash();
	}

	void SceneGrid::updateGridMesh()
	{
		std::array<Vector3, 2> axes;
		Vector3 origin;

		switch(mMode)
		{
		case GridMode::Perspective:
			axes[0] = Vector3::UNIT_X;
			axes[1] = Vector3::UNIT_Z;
			origin = Vector3::ZERO;
			break;
		case GridMode::OrthoX:
			axes[0] = Vector3::UNIT_Y;
			axes[1] = Vector3::UNIT_Z;
			origin = Vector3(500.0f, 0.0f, 0.0f);
			break;
		case GridMode::OrthoY:
			axes[0] = Vector3::UNIT_X;
			axes[1] = Vector3::UNIT_Z;
			origin = Vector3(0.0f, 500.0f, 0.0f);
			break;
		case GridMode::OrthoZ:
			axes[0] = Vector3::UNIT_X;
			axes[1] = Vector3::UNIT_Y;
			origin = Vector3(0.0f, 0.0f, 500.0f);
			break;
		case GridMode::OrthoNegX:
			axes[0] = Vector3::UNIT_Y;
			axes[1] = Vector3::UNIT_Z;
			origin = Vector3(-500.0f, 0.0f, 0.0f);
			break;
		case GridMode::OrthoNegY:
			axes[0] = Vector3::UNIT_X;
			axes[1] = Vector3::UNIT_Z;
			origin = Vector3(0.0f, -500.0f, 0.0f);
			break;
		case GridMode::OrthoNegZ:
			axes[0] = Vector3::UNIT_X;
			axes[1] = Vector3::UNIT_Y;
			origin = Vector3(0.0f, 0.0f, -500.0f);
			break;
		}

		std::array<float, 2> extents;
		extents[0] = mSize * 0.5f;
		extents[1] = mSize * 0.5f;

		Rect3 quad(origin, axes, extents);
		SPtr<MeshData> meshData = bs_shared_ptr_new<MeshData>(8, 12, mVertexDesc);

		ShapeMeshes3D::solidQuad(quad, meshData, 0, 0);
		mGridMesh = Mesh::create(meshData);
		mCoreDirty = true;
	}

	SceneGridCore::~SceneGridCore()
	{
		SPtr<CoreRenderer> activeRenderer = RendererManager::instance().getActive();
		activeRenderer->unregisterRenderCallback(mCamera.get(), 5);
	}

	void SceneGridCore::initialize(const SPtr<CameraCore>& camera, const SPtr<MaterialCore>& material)
	{
		mCamera = camera;
		mGridMaterial = material;
		mMaterialParams = material->createParamsSet();

		mViewProjParam = mGridMaterial->getParamMat4("matViewProj");
		mWorldCameraPosParam = mGridMaterial->getParamVec4("worldCameraPos");
		mGridColorParam = mGridMaterial->getParamColor("gridColor");
		mGridSpacingParam = mGridMaterial->getParamFloat("gridSpacing");
		mGridBorderWidthParam = mGridMaterial->getParamFloat("gridBorderWidth");
		mGridFadeOutStartParam = mGridMaterial->getParamFloat("gridFadeOutStart");
		mGridFadeOutEndParam = mGridMaterial->getParamFloat("gridFadeOutEnd");
		mGridMaterial->getParam("gridPlaneNormal", mGridPlaneNormalParam);

		SPtr<CoreRenderer> activeRenderer = RendererManager::instance().getActive();
		activeRenderer->registerRenderCallback(camera.get(), 5, std::bind(&SceneGridCore::render, this));			
	}

	void SceneGridCore::updateData(const SPtr<MeshCore>& mesh, float spacing, bool fadeGrid, const Vector3& gridPlaneNormal)
	{
		mGridMesh = mesh;
		mSpacing = spacing;
		mFadeGrid = fadeGrid;
		mGridPlaneNormal = gridPlaneNormal;
	}

	void SceneGridCore::render()
	{
		THROW_IF_NOT_CORE_THREAD;

		Matrix4 projMatrix = mCamera->getProjectionMatrixRS();
		Matrix4 viewMatrix = mCamera->getViewMatrix();

		Matrix4 viewProjMatrix = projMatrix * viewMatrix;
		mViewProjParam.set(viewProjMatrix);

		mWorldCameraPosParam.set(Vector4(mCamera->getPosition(), 1.0f));
		mGridColorParam.set(GRID_LINE_COLOR);
		mGridSpacingParam.set(mSpacing);
		mGridBorderWidthParam.set(LINE_BORDER_WIDTH);
		mGridPlaneNormalParam.set(mGridPlaneNormal);

		if (mFadeGrid)
		{
			mGridFadeOutStartParam.set(FADE_OUT_START);
			mGridFadeOutEndParam.set(FADE_OUT_END);
		}
		else
		{
			mGridFadeOutStartParam.set(1000.0f);
			mGridFadeOutEndParam.set(1500.0f);
		}

		mGridMaterial->updateParamsSet(mMaterialParams);

		gRendererUtility().setPass(mGridMaterial, 0);
		gRendererUtility().setPassParams(mMaterialParams);
		gRendererUtility().draw(mGridMesh, mGridMesh->getProperties().getSubMesh(0));
	}
}