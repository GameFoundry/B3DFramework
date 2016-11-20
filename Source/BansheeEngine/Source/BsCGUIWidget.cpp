//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCGUIWidget.h"
#include "BsCGUIWidgetRTTI.h"
#include "BsGUIWidget.h"
#include "BsSceneObject.h"
#include "BsCCamera.h"

namespace bs
{
	CGUIWidget::CGUIWidget(const HSceneObject& parent, const SPtr<Camera>& camera)
		:Component(parent), mCamera(camera), mParentHash((UINT32)-1)
	{
		mInternal = GUIWidget::create(camera);
		mOwnerTargetResizedConn = mInternal->onOwnerTargetResized.connect(
			std::bind(&CGUIWidget::ownerTargetResized, this));
		mOwnerWindowFocusChangedConn = mInternal->onOwnerWindowFocusChanged.connect(
			std::bind(&CGUIWidget::ownerWindowFocusChanged, this));
	}

	CGUIWidget::CGUIWidget(const HSceneObject& parent, const HCamera& camera)
		:CGUIWidget(parent, camera->_getCamera())
	{ }

	CGUIWidget::~CGUIWidget()
	{ }

	void CGUIWidget::setSkin(const HGUISkin& skin)
	{
		mInternal->setSkin(skin);
	}

	const GUISkin& CGUIWidget::getSkin() const
	{
		return mInternal->getSkin();
	}

	const HGUISkin& CGUIWidget::getSkinResource() const
	{
		return mInternal->getSkinResource();
	}

	GUIPanel* CGUIWidget::getPanel() const
	{
		return mInternal->getPanel();
	}

	UINT8 CGUIWidget::getDepth() const
	{
		return mInternal->getDepth();
	}

	void CGUIWidget::setDepth(UINT8 depth)
	{
		mInternal->setDepth(depth);
	}

	bool CGUIWidget::inBounds(const Vector2I& position) const
	{
		return mInternal->inBounds(position);
	}

	const Rect2I& CGUIWidget::getBounds() const
	{
		return mInternal->getBounds();
	}

	bool CGUIWidget::isDirty(bool cleanIfDirty)
	{
		return mInternal->isDirty(cleanIfDirty);
	}

	Viewport* CGUIWidget::getTarget() const
	{
		return mInternal->getTarget();
	}

	SPtr<Camera> CGUIWidget::getCamera() const
	{
		return mInternal->getCamera();
	}

	const Vector<GUIElement*>& CGUIWidget::getElements() const
	{
		return mInternal->getElements();
	}

	void CGUIWidget::update()
	{
		HSceneObject parent = SO();

		UINT32 curHash = parent->getTransformHash();
		if (curHash != mParentHash)
		{
			mInternal->_updateTransform(parent);
			mParentHash = curHash;
		}

		if (parent->getActive() != mInternal->getIsActive())
			mInternal->setIsActive(parent->getActive());

		mInternal->_updateRT();
	}

	void CGUIWidget::onDestroyed()
	{
		mOwnerTargetResizedConn.disconnect();
		mOwnerWindowFocusChangedConn.disconnect();
		mInternal = nullptr;
	}

	RTTITypeBase* CGUIWidget::getRTTIStatic()
	{
		return CGUIWidgetRTTI::instance();
	}

	RTTITypeBase* CGUIWidget::getRTTI() const
	{
		return CGUIWidget::getRTTIStatic();
	}
}