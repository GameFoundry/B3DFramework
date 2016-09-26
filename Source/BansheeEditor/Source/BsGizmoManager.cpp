//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGizmoManager.h"
#include "BsMesh.h"
#include "BsAABox.h"
#include "BsSphere.h"
#include "BsVertexDataDesc.h"
#include "BsShapeMeshes3D.h"
#include "BsMeshHeap.h"
#include "BsCCamera.h"
#include "BsSpriteTexture.h"
#include "BsCoreThread.h"
#include "BsBuiltinEditorResources.h"
#include "BsMaterial.h"
#include "BsGpuParams.h"
#include "BsGpuParamsSet.h"
#include "BsRenderAPI.h"
#include "BsCoreRenderer.h"
#include "BsRendererUtility.h"
#include "BsTransientMesh.h"
#include "BsRendererManager.h"
#include "BsDrawHelper.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	const UINT32 GizmoManager::VERTEX_BUFFER_GROWTH = 4096;
	const UINT32 GizmoManager::INDEX_BUFFER_GROWTH = 4096 * 2;
	const UINT32 GizmoManager::SPHERE_QUALITY = 1;
	const UINT32 GizmoManager::WIRE_SPHERE_QUALITY = 10;
	const float GizmoManager::MAX_ICON_RANGE = 500.0f;
	const UINT32 GizmoManager::OPTIMAL_ICON_SIZE = 64;
	const float GizmoManager::ICON_TEXEL_WORLD_SIZE = 0.05f;

	GizmoManager::GizmoManager()
		: mPickable(false), mCurrentIdx(0), mTransformDirty(false), mColorDirty(false), mDrawHelper(nullptr)
		, mPickingDrawHelper(nullptr), mCore(nullptr)
		
	{
		mTransform = Matrix4::IDENTITY;
		mDrawHelper = bs_new<DrawHelper>();
		mPickingDrawHelper = bs_new<DrawHelper>();

		mIconVertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		mIconVertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);
		mIconVertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);
		mIconVertexDesc->addVertElem(VET_COLOR, VES_COLOR, 0);
		mIconVertexDesc->addVertElem(VET_COLOR, VES_COLOR, 1);

		mIconMeshHeap = MeshHeap::create(VERTEX_BUFFER_GROWTH, INDEX_BUFFER_GROWTH, mIconVertexDesc);

		HMaterial solidMaterial = BuiltinEditorResources::instance().createSolidGizmoMat();
		HMaterial wireMaterial = BuiltinEditorResources::instance().createWireGizmoMat();
		HMaterial lineMaterial = BuiltinEditorResources::instance().createLineGizmoMat();
		HMaterial iconMaterial = BuiltinEditorResources::instance().createIconGizmoMat();
		HMaterial textMaterial = BuiltinEditorResources::instance().createTextGizmoMat();
		HMaterial pickingMaterial = BuiltinEditorResources::instance().createGizmoPickingMat();
		HMaterial alphaPickingMaterial = BuiltinEditorResources::instance().createAlphaGizmoPickingMat();

		CoreInitData initData;

		initData.solidMat = solidMaterial->getCore();
		initData.wireMat = wireMaterial->getCore();
		initData.lineMat = lineMaterial->getCore();
		initData.iconMat = iconMaterial->getCore();
		initData.textMat = textMaterial->getCore();
		initData.pickingMat = pickingMaterial->getCore();
		initData.alphaPickingMat = alphaPickingMaterial->getCore();

		mCore.store(bs_new<GizmoManagerCore>(GizmoManagerCore::PrivatelyConstuct()), std::memory_order_release);

		gCoreAccessor().queueCommand(std::bind(&GizmoManager::initializeCore, this, initData));
	}

	GizmoManager::~GizmoManager()
	{
		mDrawHelper->clearMeshes(mActiveMeshes);
		mActiveMeshes.clear();

		if (mIconMesh != nullptr)
			mIconMeshHeap->dealloc(mIconMesh);

		bs_delete(mDrawHelper);
		bs_delete(mPickingDrawHelper);

		gCoreAccessor().queueCommand(std::bind(&GizmoManager::destroyCore, this, mCore.load(std::memory_order_relaxed)));
	}

	void GizmoManager::initializeCore(const CoreInitData& initData)
	{
		mCore.load(std::memory_order_acquire)->initialize(initData);
	}

	void GizmoManager::destroyCore(GizmoManagerCore* core)
	{
		bs_delete(core);
	}

	void GizmoManager::startGizmo(const HSceneObject& gizmoParent)
	{
		mActiveSO = gizmoParent;

		if(mTransformDirty)
		{
			mTransform = Matrix4::IDENTITY;
			mDrawHelper->setTransform(Matrix4::IDENTITY);
			mTransformDirty = false;
		}

		if(mColorDirty)
		{
			mColor = Color();
			mColorDirty = false;
		}
	}

	void GizmoManager::endGizmo()
	{
		mActiveSO = nullptr;
	}

	void GizmoManager::setColor(const Color& color)
	{
		mDrawHelper->setColor(color);
		mColor = color;

		mColorDirty = true;
	}

	void GizmoManager::setTransform(const Matrix4& transform)
	{
		mDrawHelper->setTransform(transform);
		mTransform = transform;

		mTransformDirty = true;
	}

	void GizmoManager::drawCube(const Vector3& position, const Vector3& extents)
	{
		mSolidCubeData.push_back(CubeData());
		CubeData& cubeData = mSolidCubeData.back();

		cubeData.idx = mCurrentIdx++;
		cubeData.position = position;
		cubeData.extents = extents;
		cubeData.color = mColor;
		cubeData.transform = mTransform;
		cubeData.sceneObject = mActiveSO;
		cubeData.pickable = mPickable;

		mDrawHelper->cube(position, extents);
		mIdxToSceneObjectMap[cubeData.idx] = mActiveSO;
	}

	void GizmoManager::drawSphere(const Vector3& position, float radius)
	{
		mSolidSphereData.push_back(SphereData());
		SphereData& sphereData = mSolidSphereData.back();

		sphereData.idx = mCurrentIdx++;
		sphereData.position = position;
		sphereData.radius = radius;
		sphereData.color = mColor;
		sphereData.transform = mTransform;
		sphereData.sceneObject = mActiveSO;
		sphereData.pickable = mPickable;

		mDrawHelper->sphere(position, radius);
		mIdxToSceneObjectMap[sphereData.idx] = mActiveSO;
	}

	void GizmoManager::drawCone(const Vector3& base, const Vector3& normal, float height, float radius, const Vector2& scale)
	{
		mSolidConeData.push_back(ConeData());
		ConeData& coneData = mSolidConeData.back();

		coneData.idx = mCurrentIdx++;
		coneData.base = base;
		coneData.radius = radius;
		coneData.color = mColor;
		coneData.transform = mTransform;
		coneData.sceneObject = mActiveSO;
		coneData.pickable = mPickable;
		coneData.scale = scale;

		mDrawHelper->cone(base, normal, height, radius, scale);
		mIdxToSceneObjectMap[coneData.idx] = mActiveSO;
	}

	void GizmoManager::drawWireCube(const Vector3& position, const Vector3& extents)
	{
		mWireCubeData.push_back(CubeData());
		CubeData& cubeData = mWireCubeData.back();

		cubeData.idx = mCurrentIdx++;
		cubeData.position = position;
		cubeData.extents = extents;
		cubeData.color = mColor;
		cubeData.transform = mTransform;
		cubeData.sceneObject = mActiveSO;
		cubeData.pickable = mPickable;

		mDrawHelper->wireCube(position, extents);
		mIdxToSceneObjectMap[cubeData.idx] = mActiveSO;
	}

	void GizmoManager::drawWireSphere(const Vector3& position, float radius)
	{
		mWireSphereData.push_back(SphereData());
		SphereData& sphereData = mWireSphereData.back();

		sphereData.idx = mCurrentIdx++;
		sphereData.position = position;
		sphereData.radius = radius;
		sphereData.color = mColor;
		sphereData.transform = mTransform;
		sphereData.sceneObject = mActiveSO;
		sphereData.pickable = mPickable;

		mDrawHelper->wireSphere(position, radius);
		mIdxToSceneObjectMap[sphereData.idx] = mActiveSO;
	}

	void GizmoManager::drawWireCapsule(const Vector3& position, float height, float radius)
	{
		float halfHeight = height * 0.5f;

		// Draw capsule sides
		Vector3 sideNegXBottom = position + Vector3(-radius, -halfHeight, 0.0f);
		Vector3 sideNegXTop = position + Vector3(-radius, halfHeight, 0.0f);

		Vector3 sidePosXBottom = position + Vector3(radius, -halfHeight, 0.0f);
		Vector3 sidePosXTop = position + Vector3(radius, halfHeight, 0.0f);

		Vector3 sideNegZBottom = position + Vector3(0.0f, -halfHeight, -radius);
		Vector3 sideNegZTop = position + Vector3(0.0f, halfHeight, -radius);

		Vector3 sidePosZBottom = position + Vector3(0.0f, -halfHeight, radius);
		Vector3 sidePosZTop = position + Vector3(0.0f, halfHeight, radius);

		drawLine(sideNegXBottom, sideNegXTop);
		drawLine(sidePosXBottom, sidePosXTop);
		drawLine(sideNegZBottom, sideNegZTop);
		drawLine(sidePosZBottom, sidePosZTop);

		// Draw capsule caps
		Vector3 topHemisphere = position + Vector3(0.0f, halfHeight, 0.0f);
		Vector3 botHemisphere = position + Vector3(0.0f, -halfHeight, 0.0f);

		drawWireArc(topHemisphere, Vector3::UNIT_X, radius, Degree(270.0f), -Degree(180.0f));
		drawWireArc(topHemisphere, Vector3::UNIT_Z, radius, Degree(0.0f), -Degree(180.0f));

		drawWireArc(botHemisphere, Vector3::UNIT_X, radius, Degree(90.0f), -Degree(180.0f));
		drawWireArc(botHemisphere, Vector3::UNIT_Z, radius, Degree(180.0f), -Degree(180.0f));

		drawWireDisc(topHemisphere, Vector3::UNIT_Y, radius);
		drawWireDisc(botHemisphere, Vector3::UNIT_Y, radius);
	}

	void GizmoManager::drawWireCone(const Vector3& base, const Vector3& normal, float height, float radius, const Vector2& scale)
	{
		mWireConeData.push_back(ConeData());
		ConeData& coneData = mWireConeData.back();

		coneData.idx = mCurrentIdx++;
		coneData.base = base;
		coneData.radius = radius;
		coneData.color = mColor;
		coneData.transform = mTransform;
		coneData.sceneObject = mActiveSO;
		coneData.pickable = mPickable;
		coneData.scale = scale;

		mDrawHelper->wireCone(base, normal, height, radius, scale);
		mIdxToSceneObjectMap[coneData.idx] = mActiveSO;
	}

	void GizmoManager::drawLine(const Vector3& start, const Vector3& end)
	{
		mLineData.push_back(LineData());
		LineData& lineData = mLineData.back();

		lineData.idx = mCurrentIdx++;
		lineData.start = start;
		lineData.end = end;
		lineData.color = mColor;
		lineData.transform = mTransform;
		lineData.sceneObject = mActiveSO;
		lineData.pickable = mPickable;

		mDrawHelper->line(start, end);
		mIdxToSceneObjectMap[lineData.idx] = mActiveSO;
	}

	void GizmoManager::drawLineList(const Vector<Vector3>& linePoints)
	{
		mLineListData.push_back(LineListData());
		LineListData& lineListData = mLineListData.back();

		lineListData.idx = mCurrentIdx++;
		lineListData.linePoints = linePoints;
		lineListData.color = mColor;
		lineListData.transform = mTransform;
		lineListData.sceneObject = mActiveSO;
		lineListData.pickable = mPickable;

		mDrawHelper->lineList(linePoints);
		mIdxToSceneObjectMap[lineListData.idx] = mActiveSO;
	}

	void GizmoManager::drawWireDisc(const Vector3& position, const Vector3& normal, float radius)
	{
		mWireDiscData.push_back(WireDiscData());
		WireDiscData& wireDiscData = mWireDiscData.back();

		wireDiscData.idx = mCurrentIdx++;
		wireDiscData.position = position;
		wireDiscData.normal = normal;
		wireDiscData.radius = radius;
		wireDiscData.color = mColor;
		wireDiscData.transform = mTransform;
		wireDiscData.sceneObject = mActiveSO;
		wireDiscData.pickable = mPickable;

		mDrawHelper->wireDisc(position, normal, radius);
		mIdxToSceneObjectMap[wireDiscData.idx] = mActiveSO;
	}

	void GizmoManager::drawWireArc(const Vector3& position, const Vector3& normal, float radius, 
		Degree startAngle, Degree amountAngle)
	{
		mWireArcData.push_back(WireArcData());
		WireArcData& wireArcData = mWireArcData.back();

		wireArcData.idx = mCurrentIdx++;
		wireArcData.position = position;
		wireArcData.normal = normal;
		wireArcData.radius = radius;
		wireArcData.startAngle = startAngle;
		wireArcData.amountAngle = amountAngle;
		wireArcData.color = mColor;
		wireArcData.transform = mTransform;
		wireArcData.sceneObject = mActiveSO;
		wireArcData.pickable = mPickable;

		mDrawHelper->wireArc(position, normal, radius, startAngle, amountAngle);
		mIdxToSceneObjectMap[wireArcData.idx] = mActiveSO;
	}

	void GizmoManager::drawWireMesh(const SPtr<MeshData>& meshData)
	{
		mWireMeshData.push_back(WireMeshData());
		WireMeshData& wireMeshData = mWireMeshData.back();

		wireMeshData.idx = mCurrentIdx++;
		wireMeshData.meshData = meshData;
		wireMeshData.color = mColor;
		wireMeshData.transform = mTransform;
		wireMeshData.sceneObject = mActiveSO;
		wireMeshData.pickable = mPickable;

		mDrawHelper->wireMesh(meshData);
		mIdxToSceneObjectMap[wireMeshData.idx] = mActiveSO;
	}

	void GizmoManager::drawFrustum(const Vector3& position, float aspect, Degree FOV, float near, float far)
	{
		mFrustumData.push_back(FrustumData());
		FrustumData& frustumData = mFrustumData.back();

		frustumData.idx = mCurrentIdx++;
		frustumData.position = position;
		frustumData.aspect = aspect;
		frustumData.FOV = FOV;
		frustumData.near = near;
		frustumData.far = far;
		frustumData.color = mColor;
		frustumData.transform = mTransform;
		frustumData.sceneObject = mActiveSO;
		frustumData.pickable = mPickable;

		mDrawHelper->frustum(position, aspect, FOV, near, far);
		mIdxToSceneObjectMap[frustumData.idx] = mActiveSO;
	}

	void GizmoManager::drawIcon(Vector3 position, HSpriteTexture image, bool fixedScale)
	{
		mIconData.push_back(IconData());
		IconData& iconData = mIconData.back();

		iconData.idx = mCurrentIdx++;
		iconData.position = position;
		iconData.texture = image;
		iconData.fixedScale = fixedScale;
		iconData.color = mColor;
		iconData.transform = mTransform;
		iconData.sceneObject = mActiveSO;
		iconData.pickable = mPickable;

		mIdxToSceneObjectMap[iconData.idx] = mActiveSO;
	}

	void GizmoManager::drawText(const Vector3& position, const WString& text, const HFont& font, UINT32 fontSize)
	{
		HFont myFont = font;
		if (myFont == nullptr)
			myFont = BuiltinEditorResources::instance().getDefaultAAFont();

		mTextData.push_back(TextData());
		TextData& textData = mTextData.back();

		textData.idx = mCurrentIdx++;
		textData.position = position;
		textData.text = text;
		textData.font = myFont;
		textData.fontSize = fontSize;
		textData.color = mColor;
		textData.transform = mTransform;
		textData.sceneObject = mActiveSO;
		textData.pickable = mPickable;

		mDrawHelper->text(position, text, myFont, fontSize);
		mIdxToSceneObjectMap[textData.idx] = mActiveSO;
	}

	void GizmoManager::update(const SPtr<Camera>& camera)
	{
		mDrawHelper->clearMeshes(mActiveMeshes);
		mActiveMeshes.clear();

		if (mIconMesh != nullptr)
			mIconMeshHeap->dealloc(mIconMesh);

		IconRenderDataVecPtr iconRenderData;

		mDrawHelper->buildMeshes(DrawHelper::SortType::BackToFront, camera->getPosition());
		mActiveMeshes = mDrawHelper->getMeshes();

		Vector<GizmoManagerCore::MeshData> proxyData;
		for (auto& meshData : mActiveMeshes)
		{
			SPtr<TextureCore> tex;
			if (meshData.texture.isLoaded())
				tex = meshData.texture->getCore();

			if (meshData.type == DrawHelper::MeshType::Solid)
			{
				proxyData.push_back(GizmoManagerCore::MeshData(
					meshData.mesh->getCore(), tex, GizmoManagerCore::MeshType::Solid));
			}
			else if(meshData.type == DrawHelper::MeshType::Wire)
			{
				proxyData.push_back(GizmoManagerCore::MeshData(
					meshData.mesh->getCore(), tex, GizmoManagerCore::MeshType::Wire));
			}
			else if (meshData.type == DrawHelper::MeshType::Line)
			{
				proxyData.push_back(GizmoManagerCore::MeshData(
					meshData.mesh->getCore(), tex, GizmoManagerCore::MeshType::Line));
			}
			else // Text
			{
				proxyData.push_back(GizmoManagerCore::MeshData(
					meshData.mesh->getCore(), tex, GizmoManagerCore::MeshType::Text));
			}
		}

		mIconMesh = buildIconMesh(camera, mIconData, false, iconRenderData);
		SPtr<MeshCoreBase> iconMesh = mIconMesh->getCore();

		GizmoManagerCore* core = mCore.load(std::memory_order_relaxed);

		gCoreAccessor().queueCommand(std::bind(&GizmoManagerCore::updateData, core, camera->getCore(),
			proxyData, iconMesh, iconRenderData));
	}

	void GizmoManager::renderForPicking(const SPtr<Camera>& camera, std::function<Color(UINT32)> idxToColorCallback)
	{
		Vector<IconData> iconData;
		IconRenderDataVecPtr iconRenderData;

		mPickingDrawHelper->clear();

		for (auto& cubeDataEntry : mSolidCubeData)
		{
			if (!cubeDataEntry.pickable)
				continue;

			mPickingDrawHelper->setColor(idxToColorCallback(cubeDataEntry.idx));
			mPickingDrawHelper->setTransform(cubeDataEntry.transform);

			mPickingDrawHelper->cube(cubeDataEntry.position, cubeDataEntry.extents);
		}

		for (auto& cubeDataEntry : mWireCubeData)
		{
			if (!cubeDataEntry.pickable)
				continue;

			mPickingDrawHelper->setColor(idxToColorCallback(cubeDataEntry.idx));
			mPickingDrawHelper->setTransform(cubeDataEntry.transform);

			mPickingDrawHelper->wireCube(cubeDataEntry.position, cubeDataEntry.extents);
		}

		for (auto& sphereDataEntry : mSolidSphereData)
		{
			if (!sphereDataEntry.pickable)
				continue;

			mPickingDrawHelper->setColor(idxToColorCallback(sphereDataEntry.idx));
			mPickingDrawHelper->setTransform(sphereDataEntry.transform);

			mPickingDrawHelper->sphere(sphereDataEntry.position, sphereDataEntry.radius);
		}

		for (auto& sphereDataEntry : mWireSphereData)
		{
			if (!sphereDataEntry.pickable)
				continue;

			mPickingDrawHelper->setColor(idxToColorCallback(sphereDataEntry.idx));
			mPickingDrawHelper->setTransform(sphereDataEntry.transform);

			mPickingDrawHelper->wireSphere(sphereDataEntry.position, sphereDataEntry.radius);
		}

		for (auto& coneDataEntry : mSolidConeData)
		{
			if (!coneDataEntry.pickable)
				continue;

			mPickingDrawHelper->setColor(idxToColorCallback(coneDataEntry.idx));
			mPickingDrawHelper->setTransform(coneDataEntry.transform);

			mPickingDrawHelper->cone(coneDataEntry.base, coneDataEntry.normal, coneDataEntry.radius, coneDataEntry.radius, 
				coneDataEntry.scale);
		}

		for (auto& coneDataEntry : mWireConeData)
		{
			if (!coneDataEntry.pickable)
				continue;

			mPickingDrawHelper->setColor(idxToColorCallback(coneDataEntry.idx));
			mPickingDrawHelper->setTransform(coneDataEntry.transform);

			mPickingDrawHelper->wireCone(coneDataEntry.base, coneDataEntry.normal, coneDataEntry.radius, coneDataEntry.radius,
				coneDataEntry.scale);
		}

		for (auto& lineDataEntry : mLineData)
		{
			if (!lineDataEntry.pickable)
				continue;

			mPickingDrawHelper->setColor(idxToColorCallback(lineDataEntry.idx));
			mPickingDrawHelper->setTransform(lineDataEntry.transform);

			mPickingDrawHelper->line(lineDataEntry.start, lineDataEntry.end);
		}

		for (auto& lineListDataEntry : mLineListData)
		{
			if (!lineListDataEntry.pickable)
				continue;

			mPickingDrawHelper->setColor(idxToColorCallback(lineListDataEntry.idx));
			mPickingDrawHelper->setTransform(lineListDataEntry.transform);

			mPickingDrawHelper->lineList(lineListDataEntry.linePoints);
		}

		for (auto& wireDiscDataEntry : mWireDiscData)
		{
			if (!wireDiscDataEntry.pickable)
				continue;

			mPickingDrawHelper->setColor(idxToColorCallback(wireDiscDataEntry.idx));
			mPickingDrawHelper->setTransform(wireDiscDataEntry.transform);

			mPickingDrawHelper->wireDisc(wireDiscDataEntry.position, wireDiscDataEntry.normal, wireDiscDataEntry.radius);
		}

		for (auto& wireArcDataEntry : mWireArcData)
		{
			if (!wireArcDataEntry.pickable)
				continue;

			mPickingDrawHelper->setColor(idxToColorCallback(wireArcDataEntry.idx));
			mPickingDrawHelper->setTransform(wireArcDataEntry.transform);

			mPickingDrawHelper->wireArc(wireArcDataEntry.position, wireArcDataEntry.normal, wireArcDataEntry.radius, 
				wireArcDataEntry.startAngle, wireArcDataEntry.amountAngle);
		}

		for (auto& wireMeshData : mWireMeshData)
		{
			if (!wireMeshData.pickable)
				continue;

			mPickingDrawHelper->setColor(idxToColorCallback(wireMeshData.idx));
			mPickingDrawHelper->setTransform(wireMeshData.transform);

			mPickingDrawHelper->wireMesh(wireMeshData.meshData);
		}

		for (auto& frustumDataEntry : mFrustumData)
		{
			if (!frustumDataEntry.pickable)
				continue;

			mPickingDrawHelper->setColor(idxToColorCallback(frustumDataEntry.idx));
			mPickingDrawHelper->setTransform(frustumDataEntry.transform);

			mPickingDrawHelper->frustum(frustumDataEntry.position, frustumDataEntry.aspect, frustumDataEntry.FOV, 
				frustumDataEntry.near, frustumDataEntry.far);
		}

		for (auto& textDataEntry : mTextData)
		{
			if (!textDataEntry.pickable)
				continue;

			mPickingDrawHelper->setColor(idxToColorCallback(textDataEntry.idx));
			mPickingDrawHelper->setTransform(textDataEntry.transform);

			mPickingDrawHelper->text(textDataEntry.position, textDataEntry.text, textDataEntry.font,
				textDataEntry.fontSize);
		}

		for (auto& iconDataEntry : mIconData)
		{
			if (!iconDataEntry.pickable)
				continue;

			iconData.push_back(iconDataEntry);
			iconData.back().color = idxToColorCallback(iconDataEntry.idx);
		}

		mPickingDrawHelper->buildMeshes(DrawHelper::SortType::BackToFront, camera->getPosition());
		const Vector<DrawHelper::ShapeMeshData>& meshes = mPickingDrawHelper->getMeshes();

		SPtr<TransientMesh> iconMesh = buildIconMesh(camera, iconData, true, iconRenderData);

		// Note: This must be rendered while Scene view is being rendered
		Matrix4 viewMat = camera->getViewMatrix();
		Matrix4 projMat = camera->getProjectionMatrixRS();
		SPtr<Viewport> viewport = camera->getViewport();

		GizmoManagerCore* core = mCore.load(std::memory_order_relaxed);

		for (auto& meshData : meshes)
		{
			SPtr<TextureCore> tex;
			if (meshData.texture.isLoaded())
				tex = meshData.texture->getCore();

			if(meshData.type == DrawHelper::MeshType::Text)
			{
				gCoreAccessor().queueCommand(std::bind(&GizmoManagerCore::renderGizmos, core, viewMat, projMat,
					camera->getForward(), meshData.mesh->getCore(), tex, GizmoMaterial::PickingAlpha));
			}
			else
			{
				gCoreAccessor().queueCommand(std::bind(&GizmoManagerCore::renderGizmos, core, viewMat, projMat,
					camera->getForward(), meshData.mesh->getCore(), tex, GizmoMaterial::Picking));
			}
		}

		Rect2I screenArea = camera->getViewport()->getArea();

		gCoreAccessor().queueCommand(std::bind(&GizmoManagerCore::renderIconGizmos,
			core, screenArea, iconMesh->getCore(), iconRenderData, true));

		mPickingDrawHelper->clearMeshes(meshes);
		mIconMeshHeap->dealloc(iconMesh);
	}

	void GizmoManager::clearGizmos()
	{
		mSolidCubeData.clear();
		mWireCubeData.clear();
		mSolidSphereData.clear();
		mWireSphereData.clear();
		mSolidConeData.clear();
		mWireConeData.clear();
		mLineData.clear();
		mLineListData.clear();
		mWireDiscData.clear();
		mWireArcData.clear();
		mWireMeshData.clear();
		mFrustumData.clear();
		mTextData.clear();
		mIconData.clear();
		mIdxToSceneObjectMap.clear();

		mDrawHelper->clear();

		mCurrentIdx = 0;
	}

	void GizmoManager::clearRenderData()
	{
		mDrawHelper->clearMeshes(mActiveMeshes);
		mActiveMeshes.clear();

		if (mIconMesh != nullptr)
			mIconMeshHeap->dealloc(mIconMesh);

		mIconMesh = nullptr;

		GizmoManagerCore* core = mCore.load(std::memory_order_relaxed);
		IconRenderDataVecPtr iconRenderData = bs_shared_ptr_new<IconRenderDataVec>();
		
		gCoreAccessor().queueCommand(std::bind(&GizmoManagerCore::updateData, core, 
			nullptr, Vector<GizmoManagerCore::MeshData>(), nullptr, iconRenderData));
	}

	SPtr<TransientMesh> GizmoManager::buildIconMesh(const SPtr<Camera>& camera, const Vector<IconData>& iconData,
		bool forPicking, GizmoManager::IconRenderDataVecPtr& iconRenderData)
	{
		mSortedIconData.clear();
		
		if (iconData.size() > mSortedIconData.size())
			mSortedIconData.resize(iconData.size());

		UINT32 i = 0;
		for (auto& iconEntry : iconData)
		{
			Vector3 viewPoint = camera->worldToViewPoint(iconEntry.position);

			float distance = -viewPoint.z;
			if (distance < camera->getNearClipDistance()) // Ignore behind clip plane
				continue;

			if (distance > MAX_ICON_RANGE) // Ignore too far away
				continue;

			if (!iconEntry.texture.isLoaded()) // Ignore missing texture
				continue;

			if (forPicking && !iconEntry.pickable)
				continue;

			SortedIconData& sortedIconData = mSortedIconData[i];
			sortedIconData.iconIdx = i;
			sortedIconData.distance = distance;
			sortedIconData.screenPosition = camera->viewToScreenPoint(viewPoint);

			i++;
		}

		UINT32 actualNumIcons = i;

		// Sort back to front first, then by texture
		std::sort(mSortedIconData.begin(), mSortedIconData.begin() + actualNumIcons, 
			[&](const SortedIconData& a, const SortedIconData& b)
		{
			if (a.distance == b.distance)
			{
				HTexture texA = iconData[a.iconIdx].texture->getTexture();
				HTexture texB = iconData[b.iconIdx].texture->getTexture();

				if (texA == texB)
					return a.iconIdx < b.iconIdx;

				return texA->getInternalID() < texB->getInternalID();
			}
			else
				return a.distance > b.distance;
		});

		SPtr<MeshData> meshData = bs_shared_ptr_new<MeshData>(actualNumIcons * 4, actualNumIcons * 6, mIconVertexDesc);

		auto positionIter = meshData->getVec3DataIter(VES_POSITION);
		auto texcoordIter = meshData->getVec2DataIter(VES_TEXCOORD);
		auto normalColorIter = meshData->getDWORDDataIter(VES_COLOR, 0);
		auto fadedColorIter = meshData->getDWORDDataIter(VES_COLOR, 1);

		UINT32* indices = meshData->getIndices32();

		float cameraScale = 1.0f;
		if (camera->getProjectionType() == PT_ORTHOGRAPHIC)
			cameraScale = camera->getViewport()->getHeight() / camera->getOrthoWindowHeight();
		else
		{
			Radian vertFOV(Math::tan(camera->getHorzFOV() * 0.5f));
			cameraScale = (camera->getViewport()->getHeight() * 0.5f) / vertFOV.valueRadians();
		}

		iconRenderData = bs_shared_ptr_new<IconRenderDataVec>();
		UINT32 lastTextureIdx = std::numeric_limits<UINT32>::max();
		HTexture curTexture;

		// Note: This assumes the meshes will be rendered using the same camera
		// properties as when they are created
		for (i = 0; i < actualNumIcons; i++)
		{
			SortedIconData& sortedIconData = mSortedIconData[i];
			const IconData& curIconData = iconData[sortedIconData.iconIdx];

			HTexture atlasTexture = curIconData.texture->getTexture();

			if (curTexture != atlasTexture)
			{
				UINT32 numIconsPerTexture = i - lastTextureIdx;
				if (numIconsPerTexture > 0)
				{
					iconRenderData->push_back(IconRenderData());
					IconRenderData& renderData = iconRenderData->back();
					renderData.count = numIconsPerTexture;
					renderData.texture = atlasTexture->getCore();
				}

				lastTextureIdx = i;
				curTexture = atlasTexture;
			}

			UINT32 iconWidth = curIconData.texture->getWidth();
			UINT32 iconHeight = curIconData.texture->getHeight();

			limitIconSize(iconWidth, iconHeight);

			Vector3 position((float)sortedIconData.screenPosition.x, (float)sortedIconData.screenPosition.y, -sortedIconData.distance);
			Vector3 projPosition = camera->projectPoint(position);
			position.z = projPosition.z;

			float halfWidth = iconWidth * 0.5f;
			float halfHeight = iconHeight * 0.5f;

			if (!curIconData.fixedScale)
			{
				float iconScale = 1.0f;
				if (camera->getProjectionType() == PT_ORTHOGRAPHIC)
					iconScale = cameraScale * ICON_TEXEL_WORLD_SIZE;
				else
					iconScale = (cameraScale * ICON_TEXEL_WORLD_SIZE) / sortedIconData.distance;

				halfWidth *= iconScale;
				halfHeight *= iconScale;
			}

			Color normalColor, fadedColor;
			calculateIconColors(curIconData.color, camera, (UINT32)(halfHeight * 2.0f), curIconData.fixedScale, normalColor, fadedColor);

			if (forPicking)
			{
				normalColor = curIconData.color;
				fadedColor = curIconData.color;
			}

			Vector3 positions[4];
			positions[0] = position + Vector3(-halfWidth, -halfHeight, 0.0f);
			positions[1] = position + Vector3(halfWidth, -halfHeight, 0.0f);
			positions[2] = position + Vector3(halfWidth, halfHeight, 0.0f);
			positions[3] = position + Vector3(-halfWidth, halfHeight, 0.0f);

			Vector2 uvs[4];
			uvs[0] = curIconData.texture->transformUV(Vector2(0.0f, 0.0f));
			uvs[1] = curIconData.texture->transformUV(Vector2(1.0f, 0.0f));
			uvs[2] = curIconData.texture->transformUV(Vector2(1.0f, 1.0f));
			uvs[3] = curIconData.texture->transformUV(Vector2(0.0f, 1.0f));

			for (UINT32 j = 0; j < 4; j++)
			{
				positionIter.addValue(positions[j]);
				texcoordIter.addValue(uvs[j]);
				normalColorIter.addValue(normalColor.getAsRGBA());
				fadedColorIter.addValue(fadedColor.getAsRGBA());
			}

			UINT32 vertOffset = i * 4;

			indices[0] = vertOffset + 0;
			indices[1] = vertOffset + 1;
			indices[2] = vertOffset + 2;
			indices[3] = vertOffset + 0;
			indices[4] = vertOffset + 2;
			indices[5] = vertOffset + 3;

			indices += 6;
		}

		return mIconMeshHeap->alloc(meshData, DOT_TRIANGLE_LIST);
	}

	void GizmoManager::limitIconSize(UINT32& width, UINT32& height)
	{
		if (width <= OPTIMAL_ICON_SIZE && height <= OPTIMAL_ICON_SIZE)
			return;

		float relWidth = OPTIMAL_ICON_SIZE / (float)width;
		float relHeight = OPTIMAL_ICON_SIZE / (float)height;

		float scale = std::min(relWidth, relHeight);

		width = Math::roundToInt(width * scale);
		height = Math::roundToInt(height * scale);
	}

	void GizmoManager::calculateIconColors(const Color& tint, const SPtr<Camera>& camera,
		UINT32 iconHeight, bool fixedScale, Color& normalColor, Color& fadedColor)
	{
		normalColor = tint;

		if (!fixedScale)
		{
			float iconToScreenRatio = iconHeight / (float)camera->getViewport()->getHeight();

			if (iconToScreenRatio > 0.3f)
			{
				float alpha = 1.0f - Math::lerp01(iconToScreenRatio, 0.3f, 1.0f);
				normalColor.a *= alpha;
			}
			else if (iconToScreenRatio < 0.1f)
			{
				float alpha = Math::lerp01(iconToScreenRatio, 0.0f, 0.1f);
				normalColor.a *= alpha;
			}
		}

		fadedColor = normalColor;
		fadedColor.a *= 0.2f;
	}

	HSceneObject GizmoManager::getSceneObject(UINT32 gizmoIdx)
	{
		auto iterFind = mIdxToSceneObjectMap.find(gizmoIdx);

		if (iterFind != mIdxToSceneObjectMap.end())
			return iterFind->second;

		return HSceneObject();
	}

	const float GizmoManagerCore::PICKING_ALPHA_CUTOFF = 0.5f;

	GizmoManagerCore::GizmoManagerCore(const PrivatelyConstuct& dummy)
	{
	}

	GizmoManagerCore::~GizmoManagerCore()
	{
		SPtr<CoreRenderer> activeRenderer = RendererManager::instance().getActive();
		if (mCamera != nullptr)
			activeRenderer->unregisterRenderCallback(mCamera.get(), 20);
	}

	void GizmoManagerCore::initialize(const GizmoManager::CoreInitData& initData)
	{
		THROW_IF_NOT_CORE_THREAD;

		mSolidMaterial.mat = initData.solidMat;
		mWireMaterial.mat = initData.wireMat;
		mLineMaterial.mat = initData.lineMat;
		mTextMaterial.mat = initData.textMat;
		mIconMaterial.mat = initData.iconMat;
		mPickingMaterial.mat = initData.pickingMat;
		mAlphaPickingMaterial.mat = initData.alphaPickingMat;

		{
			mLineMaterial.params = mLineMaterial.mat->createParamsSet();
			SPtr<GpuParamsCore> vertParams = mLineMaterial.params->getGpuParams(GPT_VERTEX_PROGRAM);

			vertParams->getParam("matViewProj", mLineMaterial.viewProj);
		}

		{
			mSolidMaterial.params = mSolidMaterial.mat->createParamsSet();
			SPtr<GpuParamsCore> vertParams = mSolidMaterial.params->getGpuParams(GPT_VERTEX_PROGRAM);
			SPtr<GpuParamsCore> fragParams = mSolidMaterial.params->getGpuParams(GPT_FRAGMENT_PROGRAM);

			vertParams->getParam("matViewProj", mSolidMaterial.viewProj);
			fragParams->getParam("viewDir", mSolidMaterial.viewDir);
		}

		{
			mWireMaterial.params = mWireMaterial.mat->createParamsSet();
			SPtr<GpuParamsCore> vertParams = mWireMaterial.params->getGpuParams(GPT_VERTEX_PROGRAM);
			SPtr<GpuParamsCore> fragParams = mWireMaterial.params->getGpuParams(GPT_FRAGMENT_PROGRAM);

			vertParams->getParam("matViewProj", mWireMaterial.viewProj);
		}

		{
			mIconMaterial.params = mIconMaterial.mat->createParamsSet();
			SPtr<GpuParamsCore> vertParams0 = mIconMaterial.params->getGpuParams(GPT_VERTEX_PROGRAM, 0);
			SPtr<GpuParamsCore> vertParams1 = mIconMaterial.params->getGpuParams(GPT_VERTEX_PROGRAM, 1);

			SPtr<GpuParamsCore> fragParams0 = mIconMaterial.params->getGpuParams(GPT_FRAGMENT_PROGRAM, 0);
			SPtr<GpuParamsCore> fragParams1 = mIconMaterial.params->getGpuParams(GPT_FRAGMENT_PROGRAM, 1);

			vertParams0->getParam("matViewProj", mIconMaterial.viewProj[0]);
			vertParams1->getParam("matViewProj", mIconMaterial.viewProj[1]);

			fragParams0->getTextureParam("mainTexture", mIconMaterial.texture[0]);
			fragParams1->getTextureParam("mainTexture", mIconMaterial.texture[1]);
		}

		{
			mPickingMaterial.params = mPickingMaterial.mat->createParamsSet();
			SPtr<GpuParamsCore> vertParams = mPickingMaterial.params->getGpuParams(GPT_VERTEX_PROGRAM);

			vertParams->getParam("matViewProj", mPickingMaterial.viewProj);
		}

		{
			mAlphaPickingMaterial.params = mAlphaPickingMaterial.mat->createParamsSet();
			SPtr<GpuParamsCore> vertParams = mAlphaPickingMaterial.params->getGpuParams(GPT_VERTEX_PROGRAM);
			SPtr<GpuParamsCore> fragParams = mAlphaPickingMaterial.params->getGpuParams(GPT_FRAGMENT_PROGRAM);

			vertParams->getParam("matViewProj", mAlphaPickingMaterial.viewProj);
			fragParams->getTextureParam("mainTexture", mAlphaPickingMaterial.texture);

			GpuParamFloatCore alphaCutoffParam;
			fragParams->getParam("alphaCutoff", alphaCutoffParam);
			alphaCutoffParam.set(PICKING_ALPHA_CUTOFF);
		}

		{
			mTextMaterial.params = mTextMaterial.mat->createParamsSet();

			SPtr<GpuParamsCore> vertParams = mTextMaterial.params->getGpuParams(GPT_VERTEX_PROGRAM);
			SPtr<GpuParamsCore> fragParams = mTextMaterial.params->getGpuParams(GPT_FRAGMENT_PROGRAM);

			vertParams->getParam("matViewProj", mTextMaterial.viewProj);
			fragParams->getTextureParam("mainTexture", mTextMaterial.texture);
		}
	}

	void GizmoManagerCore::updateData(const SPtr<CameraCore>& camera, const Vector<MeshData>& meshes, 
		const SPtr<MeshCoreBase>& iconMesh, const GizmoManager::IconRenderDataVecPtr& iconRenderData)
	{
		if (mCamera != camera)
		{
			SPtr<CoreRenderer> activeRenderer = RendererManager::instance().getActive();
			if (mCamera != nullptr)
				activeRenderer->unregisterRenderCallback(mCamera.get(), 0);

			if (camera != nullptr)
				activeRenderer->registerRenderCallback(camera.get(), 0, std::bind(&GizmoManagerCore::render, this));
		}

		mCamera = camera;
		mMeshes = meshes;
		mIconMesh = iconMesh;
		mIconRenderData = iconRenderData;
	}

	void GizmoManagerCore::render()
	{
		if (mCamera == nullptr)
			return;

		SPtr<RenderTargetCore> renderTarget = mCamera->getViewport()->getTarget();

		float width = (float)renderTarget->getProperties().getWidth();
		float height = (float)renderTarget->getProperties().getHeight();

		Rect2 normArea = mCamera->getViewport()->getNormArea();

		Rect2I screenArea;
		screenArea.x = (int)(normArea.x * width);
		screenArea.y = (int)(normArea.y * height);
		screenArea.width = (int)(normArea.width * width);
		screenArea.height = (int)(normArea.height * height);

		for (auto& meshData : mMeshes)
		{
			GizmoManager::GizmoMaterial material = GizmoManager::GizmoMaterial::Solid;
			switch(meshData.type)
			{
			case MeshType::Solid:
				material = GizmoManager::GizmoMaterial::Solid;
				break;
			case MeshType::Wire:
				material = GizmoManager::GizmoMaterial::Wire;
				break;
			case MeshType::Line:
				material = GizmoManager::GizmoMaterial::Line;
				break;
			case MeshType::Text:
				material = GizmoManager::GizmoMaterial::Text;
				break;
			}

			renderGizmos(mCamera->getViewMatrix(), mCamera->getProjectionMatrixRS(), mCamera->getForward(), 
				meshData.mesh, meshData.texture, material);
		}

		if (mIconMesh != nullptr)
			renderIconGizmos(screenArea, mIconMesh, mIconRenderData, false);
	}

	void GizmoManagerCore::renderGizmos(const Matrix4& viewMatrix, const Matrix4& projMatrix, const Vector3& viewDir, 
		const SPtr<MeshCoreBase>& mesh, const SPtr<TextureCore>& texture, GizmoManager::GizmoMaterial material)
	{
		THROW_IF_NOT_CORE_THREAD;

		Matrix4 viewProjMat = projMatrix * viewMatrix;

		switch (material)
		{
		case GizmoManager::GizmoMaterial::Solid:
			mSolidMaterial.viewProj.set(viewProjMat);
			mSolidMaterial.viewDir.set((Vector4)viewDir);
			gRendererUtility().setPass(mSolidMaterial.mat);
			gRendererUtility().setPassParams(mSolidMaterial.params);
			break;
		case GizmoManager::GizmoMaterial::Wire:
			mWireMaterial.viewProj.set(viewProjMat);
			gRendererUtility().setPass(mWireMaterial.mat);
			gRendererUtility().setPassParams(mWireMaterial.params);
			break;
		case GizmoManager::GizmoMaterial::Line:
			mLineMaterial.viewProj.set(viewProjMat);
			gRendererUtility().setPass(mLineMaterial.mat);
			gRendererUtility().setPassParams(mLineMaterial.params);
			break;
		case GizmoManager::GizmoMaterial::Picking:
			mPickingMaterial.viewProj.set(viewProjMat);
			gRendererUtility().setPass(mPickingMaterial.mat);
			gRendererUtility().setPassParams(mPickingMaterial.params);
			break;
		case GizmoManager::GizmoMaterial::PickingAlpha:
			mAlphaPickingMaterial.viewProj.set(viewProjMat);
			mAlphaPickingMaterial.texture.set(texture);
			gRendererUtility().setPass(mAlphaPickingMaterial.mat);
			gRendererUtility().setPassParams(mAlphaPickingMaterial.params);
			break;
		case GizmoManager::GizmoMaterial::Text:
			mTextMaterial.viewProj.set(viewProjMat);
			mTextMaterial.texture.set(texture);
			gRendererUtility().setPass(mTextMaterial.mat);
			gRendererUtility().setPassParams(mTextMaterial.params);
			break;
		}
		gRendererUtility().draw(mesh, mesh->getProperties().getSubMesh(0));
	}

	void GizmoManagerCore::renderIconGizmos(Rect2I screenArea, SPtr<MeshCoreBase> mesh, GizmoManager::IconRenderDataVecPtr renderData, bool usePickingMaterial)
	{
		RenderAPICore& rapi = RenderAPICore::instance();
		SPtr<VertexData> vertexData = mesh->getVertexData();

		rapi.setVertexDeclaration(vertexData->vertexDeclaration);
		auto vertexBuffers = vertexData->getBuffers();

		SPtr<VertexBufferCore> vertBuffers[1] = { vertexBuffers.begin()->second };
		rapi.setVertexBuffers(0, vertBuffers, 1);

		SPtr<IndexBufferCore> indexBuffer = mesh->getIndexBuffer();
		rapi.setIndexBuffer(indexBuffer);

		rapi.setDrawOperation(DOT_TRIANGLE_LIST);

		// Set up ortho matrix
		Matrix4 projMat;

		const RenderAPIInfo& rapiInfo = rapi.getAPIInfo();
		float left = screenArea.x + rapiInfo.getHorizontalTexelOffset();
		float right = screenArea.x + screenArea.width + rapiInfo.getHorizontalTexelOffset();
		float top = screenArea.y + rapiInfo.getVerticalTexelOffset();
		float bottom = screenArea.y + screenArea.height + rapiInfo.getVerticalTexelOffset();
		float near = rapiInfo.getMinimumDepthInputValue();
		float far = rapiInfo.getMaximumDepthInputValue();

		// Top/bottom have been swapped because we're moving from window coordinates (origin top left)
		// to normalized device coordinates (origin bottom left)
		// Negative near/far because Z is flipped for normalized device coordinates 
		// (positive Z goes into screen as opposed to view space here we're looking along negative Z)
		projMat.makeProjectionOrtho(left, right, top, bottom, -near, -far);

		if (!usePickingMaterial)
		{
			mIconMaterial.viewProj[0].set(projMat);
			mIconMaterial.viewProj[1].set(projMat);

			for (UINT32 passIdx = 0; passIdx < 2; passIdx++)
			{
				gRendererUtility().setPass(mIconMaterial.mat, passIdx);
				gRendererUtility().setPassParams(mIconMaterial.params, passIdx);

				UINT32 curIndexOffset = mesh->getIndexOffset();
				for (auto curRenderData : *renderData)
				{
					mIconMaterial.texture[passIdx].set(curRenderData.texture);

					SPtr<GpuParamsCore> fragParams = mIconMaterial.params->getGpuParams(GPT_FRAGMENT_PROGRAM, passIdx);
					gRendererUtility().setGpuParams(GPT_FRAGMENT_PROGRAM, fragParams);

					rapi.drawIndexed(curIndexOffset, curRenderData.count * 6, mesh->getVertexOffset(), curRenderData.count * 4);
					curIndexOffset += curRenderData.count * 6;
				}
			}
		}
		else
		{
			mAlphaPickingMaterial.viewProj.set(projMat);

			gRendererUtility().setPass(mAlphaPickingMaterial.mat);
			gRendererUtility().setPassParams(mAlphaPickingMaterial.params);

			UINT32 curIndexOffset = 0;
			for (auto curRenderData : *renderData)
			{
				mAlphaPickingMaterial.texture.set(curRenderData.texture);

				SPtr<GpuParamsCore> fragParams = mAlphaPickingMaterial.params->getGpuParams(GPT_FRAGMENT_PROGRAM);
				gRendererUtility().setGpuParams(GPT_FRAGMENT_PROGRAM, fragParams);

				rapi.drawIndexed(curIndexOffset, curRenderData.count * 6, mesh->getVertexOffset(), curRenderData.count * 4);
				curIndexOffset += curRenderData.count * 6;

			}
		}

		mesh->_notifyUsedOnGPU();
	}
}