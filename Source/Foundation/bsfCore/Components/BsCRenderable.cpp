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

	void CRenderable::SetMesh(HMesh mesh)
	{
		mInternal->setMesh(mesh);

		if (mAnimation != nullptr)
			mAnimation->_updateBounds(false);
	}

	void CRenderable::OnInitialized()
	{
		// If mInternal already exists this means this object was deserialized,
		// so all we need to do is initialize it.
		if (mInternal != nullptr)
			mInternal->initialize();
		else
			mInternal = Renderable::create();

		gSceneManager()._bindActor(mInternal, sceneObject());

		mAnimation = SO()->getComponent<CAnimation>();
		if (mAnimation != nullptr)
		{
			_registerAnimation(mAnimation);
			mAnimation->_registerRenderable(static_object_cast<CRenderable>(mThisHandle));
		}
	}

	Bounds CRenderable::GetBounds() const
	{
		mInternal->_updateState(*SO());
		return mInternal->getBounds();
	}

	bool CRenderable::CalculateBounds(Bounds& bounds)
	{
		bounds = getBounds();

		return true;
	}

	void CRenderable::_registerAnimation(const HAnimation& animation)
	{
		mAnimation = animation;

		if (mInternal != nullptr)
		{
			mInternal->setAnimation(animation->_getInternal());

			// Need to update transform because animated renderables handle local transforms through bones, so it
			// shouldn't be included in the renderable's transform.
			mInternal->_updateState(*SO(), true);
		}
	}

	void CRenderable::_unregisterAnimation()
	{
		mAnimation = nullptr;

		if(mInternal != nullptr)
		{
			mInternal->setAnimation(nullptr);

			// Need to update transform because animated renderables handle local transforms through bones, so it
			// shouldn't be included in the renderable's transform.
			mInternal->_updateState(*SO(), true);
		}
	}

	void CRenderable::Update()
	{

	}

	void CRenderable::OnDestroyed()
	{
		if (mAnimation != nullptr)
			mAnimation->_unregisterRenderable();

		gSceneManager()._unbindActor(mInternal);
		mInternal->destroy();
	}

	RTTITypeBase* CRenderable::getRTTIStatic()
	{
		return CRenderableRTTI::Instance();
	}

	RTTITypeBase* CRenderable::getRTTI() const
	{
		return CRenderable::GetRTTIStatic();
	}
}
