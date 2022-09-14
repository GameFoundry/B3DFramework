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

namespace bs
{
	CRenderable::CRenderable()
	{
		setName("Renderable");
		setFlag(ComponentFlag::AlwaysRun, true);
	}

	CRenderable::CRenderable(const HSceneObject& parent)
		:Component(parent)
	{
		setName("Renderable");
		setFlag(ComponentFlag::AlwaysRun, true);
	}

	void CRenderable::setMesh(HMesh mesh)
	{
		mInternal->setMesh(mesh);

		if (mAnimation != nullptr)
			mAnimation->UpdateBoundsInternal(false);
	}

	void CRenderable::onInitialized()
	{
		// If mInternal already exists this means this object was deserialized,
		// so all we need to do is initialize it.
		if (mInternal != nullptr)
			mInternal->initialize();
		else
			mInternal = Renderable::create();

		gSceneManager().BindActorInternal(mInternal, sceneObject());

		mAnimation = SO()->getComponent<CAnimation>();
		if (mAnimation != nullptr)
		{
			RegisterAnimationInternal(mAnimation);
			mAnimation->RegisterRenderableInternal(static_object_cast<CRenderable>(mThisHandle));
		}
	}

	Bounds CRenderable::getBounds() const
	{
		mInternal->UpdateStateInternal(*SO());
		return mInternal->getBounds();
	}

	bool CRenderable::calculateBounds(Bounds& bounds)
	{
		bounds = getBounds();

		return true;
	}

	void CRenderable::RegisterAnimationInternal(const HAnimation& animation)
	{
		mAnimation = animation;

		if (mInternal != nullptr)
		{
			mInternal->setAnimation(animation->GetInternalInternal());

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
			mInternal->setAnimation(nullptr);

			// Need to update transform because animated renderables handle local transforms through bones, so it
			// shouldn't be included in the renderable's transform.
			mInternal->UpdateStateInternal(*SO(), true);
		}
	}

	void CRenderable::update()
	{

	}

	void CRenderable::onDestroyed()
	{
		if (mAnimation != nullptr)
			mAnimation->UnregisterRenderableInternal();

		gSceneManager().UnbindActorInternal(mInternal);
		mInternal->destroy();
	}

	RTTITypeBase* CRenderable::getRTTIStatic()
	{
		return CRenderableRTTI::instance();
	}

	RTTITypeBase* CRenderable::getRTTI() const
	{
		return CRenderable::getRTTIStatic();
	}
}
