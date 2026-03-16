//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRenderableTestSuite.h"

#include "Math/B3DMath.h"
#include "Scene/B3DSceneObject.h"
#include "Scene/B3DSceneInstance.h"
#include "Scene/B3DPrefab.h"
#include "Components/B3DRenderable.h"

using namespace b3d;

static constexpr float kEpsilon = 0.001f;

RenderableTestSuite::RenderableTestSuite()
	: TestSuite("RenderableTestSuite")
{
	B3D_ADD_TEST(RenderableTestSuite::TestRenderableSerialization)
}

void RenderableTestSuite::TestRenderableSerialization()
{
	// Build a hierarchy with multiple renderables, each with distinct property values
	SPtr<SceneInstance> scene = SceneInstance::Create("TestRenderableSerialization");

	HSceneObject root = scene->CreateSceneObject("Root");

	HSceneObject staticMesh = scene->CreateSceneObject("StaticMesh");
	staticMesh->SetParent(root);
	HRenderable staticRenderable = staticMesh->AddComponent<Renderable>();
	// Leave defaults: Layer=1, CullDistanceFactor=1.0, WriteVelocity=true

	HSceneObject overlayMesh = scene->CreateSceneObject("OverlayMesh");
	overlayMesh->SetParent(root);
	HRenderable overlayRenderable = overlayMesh->AddComponent<Renderable>();
	overlayRenderable->SetLayer(64);
	overlayRenderable->SetCullDistanceFactor(0.5f);
	overlayRenderable->SetWriteVelocity(false);

	HSceneObject parent = scene->CreateSceneObject("Parent");
	parent->SetParent(root);

	HSceneObject childMesh = scene->CreateSceneObject("ChildMesh");
	childMesh->SetParent(parent);
	HRenderable childRenderable = childMesh->AddComponent<Renderable>();
	childRenderable->SetLayer(32);
	childRenderable->SetCullDistanceFactor(3.0f);
	childRenderable->SetWriteVelocity(true);

	// Create prefab and instantiate — this exercises binary serialization + deserialization of ECS fields
	HPrefab prefab = Prefab::Create(root);
	SPtr<SceneInstance> cloneScene = prefab->InstantiateAsScene();
	HSceneObject cloneRoot = cloneScene->GetRoot();

	// Verify hierarchy structure
	B3D_TEST_ASSERT(cloneRoot->GetChildCount() == 3)

	HSceneObject cloneStaticMesh = cloneRoot->FindChild("StaticMesh", false);
	HSceneObject cloneOverlayMesh = cloneRoot->FindChild("OverlayMesh", false);
	HSceneObject cloneParent = cloneRoot->FindChild("Parent", false);

	B3D_TEST_ASSERT(cloneStaticMesh != nullptr)
	B3D_TEST_ASSERT(cloneOverlayMesh != nullptr)
	B3D_TEST_ASSERT(cloneParent != nullptr)
	B3D_TEST_ASSERT(cloneParent->GetChildCount() == 1)

	HSceneObject cloneChildMesh = cloneParent->GetChild(0);
	B3D_TEST_ASSERT(cloneChildMesh->GetName() == "ChildMesh")

	// Verify each renderable's ECS-backed properties survived serialization
	HRenderable cloneStaticRenderable = cloneStaticMesh->GetComponent<Renderable>();
	B3D_TEST_ASSERT(cloneStaticRenderable.IsValid())
	B3D_TEST_ASSERT(cloneStaticRenderable->GetLayer() == 1)
	B3D_TEST_ASSERT(Math::ApproxEquals(cloneStaticRenderable->GetCullDistanceFactor(), 1.0f, kEpsilon))
	B3D_TEST_ASSERT(cloneStaticRenderable->GetWriteVelocity() == true)

	HRenderable cloneOverlayRenderable = cloneOverlayMesh->GetComponent<Renderable>();
	B3D_TEST_ASSERT(cloneOverlayRenderable.IsValid())
	B3D_TEST_ASSERT(cloneOverlayRenderable->GetLayer() == 64)
	B3D_TEST_ASSERT(Math::ApproxEquals(cloneOverlayRenderable->GetCullDistanceFactor(), 0.5f, kEpsilon))
	B3D_TEST_ASSERT(cloneOverlayRenderable->GetWriteVelocity() == false)

	HRenderable cloneChildRenderable = cloneChildMesh->GetComponent<Renderable>();
	B3D_TEST_ASSERT(cloneChildRenderable.IsValid())
	B3D_TEST_ASSERT(cloneChildRenderable->GetLayer() == 32)
	B3D_TEST_ASSERT(Math::ApproxEquals(cloneChildRenderable->GetCullDistanceFactor(), 3.0f, kEpsilon))
	B3D_TEST_ASSERT(cloneChildRenderable->GetWriteVelocity() == true)

	// SceneObject without a renderable should return invalid handle
	HRenderable noRenderable = cloneParent->GetComponent<Renderable>();
	B3D_TEST_ASSERT(!noRenderable.IsValid())

	cloneScene->Destroy();
	scene->Destroy();
}
