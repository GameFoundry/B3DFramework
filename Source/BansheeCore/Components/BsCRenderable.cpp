//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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
			mAnimation->_updateBounds(false);
	}

	void CRenderable::onInitialized()
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
			mAnimation->_registerRenderable(mThisHandle);
		}
	}

	Bounds CRenderable::getBounds() const
	{
		mInternal->_updateState(*SO());
		return mInternal->getBounds();
	}

	bool CRenderable::calculateBounds(Bounds& bounds)
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

	void CRenderable::update()
	{

	}

	void CRenderable::onDestroyed()
	{
		if (mAnimation != nullptr)
			mAnimation->_unregisterRenderable();

		gSceneManager()._unbindActor(mInternal);
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