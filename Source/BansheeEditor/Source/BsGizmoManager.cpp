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

namespace bs
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
		, mPickingDrawHelper(nullptr)
		
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

		mGizmoRenderer = RendererExtension::create<GizmoRenderer>(initData);
	}

	GizmoManager::~GizmoManager()
	{
		mDrawHelper->clearMeshes(mActiveMeshes);
		mActiveMeshes.clear();

		if (mIconMesh != nullptr)
			mIconMeshHeap->dealloc(mIconMesh);

		bs_delete(mDrawHelper);
		bs_delete(mPickingDrawHelper);
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

	Vector<GizmoManager::MeshRenderData> GizmoManager::createMeshProxyData(const Vector<DrawHelper::ShapeMeshData>& meshData)
	{
		Vector<MeshRenderData> proxyData;
		for (auto& entry : meshData)
		{
			SPtr<TextureCore> tex;
			if (entry.texture.isLoaded())
				tex = entry.texture->getCore();

			if (entry.type == DrawHelper::MeshType::Solid)
				proxyData.push_back(MeshRenderData(entry.mesh->getCore(), tex, GizmoMeshType::Solid));
			else if (entry.type == DrawHelper::MeshType::Wire)
				proxyData.push_back(MeshRenderData(entry.mesh->getCore(), tex, GizmoMeshType::Wire));
			else if (entry.type == DrawHelper::MeshType::Line)
				proxyData.push_back(MeshRenderData(entry.mesh->getCore(), tex, GizmoMeshType::Line));
			else // Text
				proxyData.push_back(MeshRenderData(entry.mesh->getCore(), tex, GizmoMeshType::Text));
		}

		return proxyData;
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

		Vector<MeshRenderData> proxyData = createMeshProxyData(mActiveMeshes);

		mIconMesh = buildIconMesh(camera, mIconData, false, iconRenderData);

		SPtr<MeshCoreBase> iconMesh;
		if(mIconMesh != nullptr)
			iconMesh = mIconMesh->getCore();

		GizmoRenderer* renderer = mGizmoRenderer.get();

		gCoreThread().queueCommand(std::bind(&GizmoRenderer::updateData, renderer, camera->getCore(),
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
		
		SPtr<TransientMeshCore> iconMeshCore;
		if (iconMesh != nullptr)
			iconMeshCore = iconMesh->getCore();

		// Note: This must be rendered while Scene view is being rendered
		GizmoRenderer* renderer = mGizmoRenderer.get();

		Vector<MeshRenderData> proxyData = createMeshProxyData(meshes);
		gCoreThread().queueCommand(std::bind(&GizmoRenderer::renderData, renderer, camera->getCore(),
											 proxyData, iconMeshCore, iconRenderData, true));

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

		GizmoRenderer* renderer = mGizmoRenderer.get();
		IconRenderDataVecPtr iconRenderData = bs_shared_ptr_new<IconRenderDataVec>();
		
		gCoreThread().queueCommand(std::bind(&GizmoRenderer::updateData, renderer,
			nullptr, Vector<MeshRenderData>(), nullptr, iconRenderData));
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

		if(actualNumIcons > 0)
			return mIconMeshHeap->alloc(meshData, DOT_TRIANGLE_LIST);

		return nullptr;
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

	GizmoParamBlockDef gHandleParamBlockDef;
	GizmoPickingParamBlockDef gGizmoPickingParamBlockDef;

	const float GizmoRenderer::PICKING_ALPHA_CUTOFF = 0.5f;

	GizmoRenderer::GizmoRenderer()
		:RendererExtension(RenderLocation::PostLightPass, 0)
	{
	}

	void GizmoRenderer::initialize(const Any& data)
	{
		THROW_IF_NOT_CORE_THREAD;

		const GizmoManager::CoreInitData& initData = any_cast_ref<GizmoManager::CoreInitData>(data);

		mMeshMaterials[(UINT32)GizmoMeshType::Solid] = initData.solidMat;
		mMeshMaterials[(UINT32)GizmoMeshType::Wire] = initData.wireMat;
		mMeshMaterials[(UINT32)GizmoMeshType::Line] = initData.lineMat;
		mMeshMaterials[(UINT32)GizmoMeshType::Text] = initData.textMat;
		mIconMaterial = initData.iconMat;
		mPickingMaterials[0] = initData.pickingMat;
		mPickingMaterials[1] = initData.alphaPickingMat;

		mMeshGizmoBuffer = gHandleParamBlockDef.createBuffer();
		mIconGizmoBuffer = gHandleParamBlockDef.createBuffer();
		mMeshPickingParamBuffer = gGizmoPickingParamBlockDef.createBuffer();
		mIconPickingParamBuffer = gGizmoPickingParamBlockDef.createBuffer();
	}

	void GizmoRenderer::updateData(const SPtr<CameraCore>& camera, const Vector<GizmoManager::MeshRenderData>& meshes,
		const SPtr<MeshCoreBase>& iconMesh, const GizmoManager::IconRenderDataVecPtr& iconRenderData)
	{
		mCamera = camera;
		mMeshes = meshes;
		mIconMesh = iconMesh;
		mIconRenderData = iconRenderData;

		// Allocate and assign GPU program parameter objects
		UINT32 meshCounters[(UINT32)GizmoMeshType::Count];
		bs_zero_out(meshCounters);

		for (auto& meshData : mMeshes)
		{
			UINT32 typeIdx = (UINT32)meshData.type;
			UINT32 paramsIdx = meshCounters[typeIdx];

			meshData.paramsIdx = paramsIdx;

			if (paramsIdx >= mMeshParamSets[typeIdx].size())
			{
				SPtr<GpuParamsSetCore> paramsSet = mMeshMaterials[typeIdx]->createParamsSet();
				paramsSet->setParamBlockBuffer("Uniforms", mMeshGizmoBuffer, true);

				mMeshParamSets[typeIdx].push_back(paramsSet);
			}

			meshCounters[typeIdx]++;
		}

		UINT32 iconMeshIdx = 0;
		for(auto& iconData : *mIconRenderData)
		{
			iconData.paramsIdx = iconMeshIdx;

			SPtr<GpuParamsSetCore> paramsSet;
			if (iconMeshIdx >= mIconParamSets.size())
			{
				mIconMaterial->createParamsSet();
				paramsSet->setParamBlockBuffer("Uniforms", mIconGizmoBuffer, true);

				mIconParamSets.push_back(paramsSet);
			}
			else
				paramsSet = mIconParamSets[iconMeshIdx];

			SPtr<GpuParamsCore> params0 = paramsSet->getGpuParams(0);
			SPtr<GpuParamsCore> params1 = paramsSet->getGpuParams(1);

			GpuParamTextureCore textureParam0;
			GpuParamTextureCore textureParam1;

			params0->getTextureParam(GPT_FRAGMENT_PROGRAM, "gMainTexture", textureParam0);
			params1->getTextureParam(GPT_FRAGMENT_PROGRAM, "gMainTexture", textureParam1);

			textureParam0.set(iconData.texture);
			textureParam1.set(iconData.texture);

			iconMeshIdx++;
		}
	}

	bool GizmoRenderer::check(const CameraCore& camera)
	{
		return &camera == mCamera.get();
	}

	void GizmoRenderer::render(const CameraCore& camera)
	{
		renderData(mCamera, mMeshes, mIconMesh, mIconRenderData, false);
	}

	void GizmoRenderer::renderData(const SPtr<CameraCore>& camera, Vector<GizmoManager::MeshRenderData>& meshes,
		const SPtr<MeshCoreBase>& iconMesh, const GizmoManager::IconRenderDataVecPtr& iconRenderData, bool usePickingMaterial)
	{
		if (camera == nullptr)
			return;

		SPtr<RenderTargetCore> renderTarget = camera->getViewport()->getTarget();

		float width = (float)renderTarget->getProperties().getWidth();
		float height = (float)renderTarget->getProperties().getHeight();

		Rect2I screenArea = camera->getViewport()->getArea();
		Matrix4 viewMatrix = camera->getViewMatrix();
		Matrix4 projMatrix = camera->getProjectionMatrixRS();
		Matrix4 viewProjMat = projMatrix * viewMatrix;

		if (!usePickingMaterial)
		{
			gHandleParamBlockDef.gMatViewProj.set(mMeshGizmoBuffer, viewProjMat);
			gHandleParamBlockDef.gViewDir.set(mMeshGizmoBuffer, (Vector4)camera->getForward());

			for (auto& entry : meshes)
			{
				UINT32 typeIdx = (UINT32)entry.type;

				gRendererUtility().setPass(mMeshMaterials[typeIdx]);
				gRendererUtility().setPassParams(mMeshParamSets[typeIdx][entry.paramsIdx]);

				gRendererUtility().draw(entry.mesh, entry.mesh->getProperties().getSubMesh(0));
			}
		}
		else
		{
			// Allocate and assign GPU program parameter objects
			UINT32 pickingCounters[2];
			bs_zero_out(pickingCounters);

			for (auto& entry : meshes)
			{
				UINT32 typeIdx = entry.type == GizmoMeshType::Text ? 1 : 0;
				UINT32 paramsIdx = pickingCounters[typeIdx];

				entry.paramsIdx = paramsIdx;

				if (paramsIdx >= mPickingParamSets[typeIdx].size())
				{
					SPtr<GpuParamsSetCore> paramsSet = mPickingMaterials[typeIdx]->createParamsSet();
					paramsSet->setParamBlockBuffer("Uniforms", mMeshPickingParamBuffer, true);

					mPickingParamSets[typeIdx].push_back(paramsSet);
				}

				pickingCounters[typeIdx]++;
			}

			for (auto& iconData : *iconRenderData)
			{
				iconData.paramsIdx = pickingCounters[1];

				if (iconData.paramsIdx >= mPickingParamSets[1].size())
				{
					SPtr<GpuParamsSetCore> paramsSet = mPickingMaterials[1]->createParamsSet();
					paramsSet->setParamBlockBuffer("Uniforms", mIconPickingParamBuffer, true);

					mPickingParamSets[1].push_back(paramsSet);
				}

				pickingCounters[1]++;
			}

			gGizmoPickingParamBlockDef.gMatViewProj.set(mMeshPickingParamBuffer, viewProjMat);
			gGizmoPickingParamBlockDef.gAlphaCutoff.set(mMeshPickingParamBuffer, PICKING_ALPHA_CUTOFF);

			for (auto& entry : meshes)
			{
				UINT32 typeIdx = entry.type == GizmoMeshType::Text ? 1 : 0;

				gRendererUtility().setPass(mPickingMaterials[typeIdx]);
				gRendererUtility().setPassParams(mPickingParamSets[typeIdx][entry.paramsIdx]);

				gRendererUtility().draw(entry.mesh, entry.mesh->getProperties().getSubMesh(0));
			}
		}

		if (iconMesh != nullptr)
			renderIconGizmos(screenArea, iconMesh, iconRenderData, usePickingMaterial);
	}

	void GizmoRenderer::renderIconGizmos(Rect2I screenArea, SPtr<MeshCoreBase> mesh, 
		GizmoManager::IconRenderDataVecPtr renderData, bool usePickingMaterial)
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
			gHandleParamBlockDef.gMatViewProj.set(mIconGizmoBuffer, projMat);
			gHandleParamBlockDef.gViewDir.set(mIconGizmoBuffer, Vector4::ZERO);

			for (UINT32 passIdx = 0; passIdx < 2; passIdx++)
			{
				gRendererUtility().setPass(mIconMaterial, passIdx);
				
				UINT32 curIndexOffset = mesh->getIndexOffset();
				for (auto curRenderData : *renderData)
				{
					gRendererUtility().setPassParams(mIconParamSets[curRenderData.paramsIdx], passIdx);

					rapi.drawIndexed(curIndexOffset, curRenderData.count * 6, mesh->getVertexOffset(), curRenderData.count * 4);
					curIndexOffset += curRenderData.count * 6;
				}
			}
		}
		else
		{
			gGizmoPickingParamBlockDef.gMatViewProj.set(mIconPickingParamBuffer, projMat);
			gGizmoPickingParamBlockDef.gAlphaCutoff.set(mIconPickingParamBuffer, PICKING_ALPHA_CUTOFF);

			for (auto& iconData : *renderData)
			{
				SPtr<GpuParamsSetCore> paramsSet = mPickingParamSets[1][iconData.paramsIdx];
				SPtr<GpuParamsCore> params = paramsSet->getGpuParams();

				GpuParamTextureCore textureParam;
				params->getTextureParam(GPT_FRAGMENT_PROGRAM, "gMainTexture", textureParam);

				textureParam.set(iconData.texture);
			}

			gRendererUtility().setPass(mPickingMaterials[1]);

			UINT32 curIndexOffset = mesh->getIndexOffset();
			for (auto curRenderData : *renderData)
			{
				gRendererUtility().setPassParams(mPickingParamSets[1][curRenderData.paramsIdx]);

				rapi.drawIndexed(curIndexOffset, curRenderData.count * 6, mesh->getVertexOffset(), curRenderData.count * 4);
				curIndexOffset += curRenderData.count * 6;
			}
		}

		mesh->_notifyUsedOnGPU();
	}
}