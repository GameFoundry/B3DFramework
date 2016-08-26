//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCRenderable.h"
#include "BsCRenderableRTTI.h"
#include "BsSceneObject.h"
#include "BsMesh.h"
#include "BsMaterial.h"
#include "BsCAnimation.h"
#include "BsBounds.h"
#include "BsSceneManager.h"

namespace BansheeEngine
{
	CRenderable::CRenderable(const HSceneObject& parent)
		:Component(parent)
	{
		setName("Renderable");
	}

	void CRenderable::onInitialized()
	{
		// If mInternal already exists this means this object was deserialized,
		// so all we need to do is initialize it.
		if (mInternal != nullptr)
			mInternal->initialize();
		else
			mInternal = Renderable::create();

		gSceneManager()._registerRenderable(mInternal, sceneObject());

		HAnimation animationComponent = SO()->getComponent<CAnimation>();
		if (animationComponent != nullptr)
			_setAnimation(animationComponent->_getInternal());
	}

	Bounds CRenderable::getBounds() const
	{
		mInternal->_updateTransform(mThisHandle);
		return mInternal->getBounds();
	}

	bool CRenderable::calculateBounds(Bounds& bounds)
	{
		bounds = getBounds();

		return true;
	}

	void CRenderable::_setAnimation(const SPtr<Animation>& animation)
	{
		if (mInternal != nullptr)
		{
			mInternal->setAnimation(animation);

			// Need to update transform because animated renderables handle local transforms through bones, so it shouldn't
			// be included in the renderable's transform.
			mInternal->_updateTransform(mThisHandle, true);
		}
	}

	void CRenderable::update()
	{

	}

	void CRenderable::onDestroyed()
	{
		gSceneManager()._unregisterRenderable(mInternal);
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