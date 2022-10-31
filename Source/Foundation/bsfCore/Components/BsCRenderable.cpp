//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCRenderable.h"
#include "Private/RTTI/BsCRenderableRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Mesh/BsMesh.h"
#include "Material/BsMaterial.h"
#include "Components/BsCAnimation.h"
#include "Math/BsBounds.h"
#include "Scene/BsSceneManager.h"

using namespace bs;

CRenderable::CRenderable()
{
	SetName("Renderable");
	SetFlag(ComponentFlag::AlwaysRun, true);
}

CRenderable::CRenderable(const HSceneObject& parent)
	: Component(parent)
{
	SetName("Renderable");
	SetFlag(ComponentFlag::AlwaysRun, true);
}

void CRenderable::SetMesh(HMesh mesh)
{
	mInternal->SetMesh(mesh);

	if(mAnimation != nullptr)
		mAnimation->UpdateBoundsInternal(false);
}

void CRenderable::OnInitialized()
{
	// If mInternal already exists this means this object was deserialized,
	// so all we need to do is initialize it.
	if(mInternal != nullptr)
		mInternal->Initialize();
	else
		mInternal = Renderable::Create();

	GetSceneManager().BindActorInternal(mInternal, SceneObject());

	mAnimation = SO()->GetComponent<CAnimation>();
	if(mAnimation != nullptr)
	{
		RegisterAnimationInternal(mAnimation);
		mAnimation->RegisterRenderableInternal(static_object_cast<CRenderable>(mThisHandle));
	}
}

Bounds CRenderable::GetBounds() const
{
	mInternal->UpdateStateInternal(*SO());
	return mInternal->GetBounds();
}

bool CRenderable::CalculateBounds(Bounds& bounds)
{
	bounds = GetBounds();

	return true;
}

void CRenderable::RegisterAnimationInternal(const HAnimation& animation)
{
	mAnimation = animation;

	if(mInternal != nullptr)
	{
		mInternal->SetAnimation(animation->GetInternalInternal());

		// Need to update transform because animated renderables handle local transforms through bones, so it
		// shouldn't be included in the renderable's transform.
		mInternal->UpdateStateInternal(*SO(), true);
	}
}

void CRenderable::UnregisterAnimationInternal()
{
	mAnimation = nullptr;

	if(mInternal != nullptr)
	{
		mInternal->SetAnimation(nullptr);

		// Need to update transform because animated renderables handle local transforms through bones, so it
		// shouldn't be included in the renderable's transform.
		mInternal->UpdateStateInternal(*SO(), true);
	}
}

void CRenderable::Update()
{
}

void CRenderable::OnDestroyed()
{
	if(mAnimation != nullptr)
		mAnimation->UnregisterRenderableInternal();

	GetSceneManager().UnbindActorInternal(mInternal);
	mInternal->Destroy();
}

RTTITypeBase* CRenderable::GetRttiStatic()
{
	return CRenderableRTTI::Instance();
}

RTTITypeBase* CRenderable::GetRtti() const
{
	return CRenderable::GetRttiStatic();
}
