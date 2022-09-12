//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsCGUIWidget.h"
#include "Private/RTTI/BsCGUIWidgetRTTI.h"
#include "GUI/BsGUIWidget.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCCamera.h"

namespace bs
{
	CGUIWidget::CGUIWidget()
	{
		setFlag(ComponentFlag::AlwaysRun, true);
	}

	CGUIWidget::CGUIWidget(const HSceneObject& parent, const SPtr<Camera>& camera)
		:Component(parent), mCamera(camera), mParentHash((UINT32)-1)
	{
		setFlag(ComponentFlag::AlwaysRun, true);

		mInternal = GUIWidget::create(camera);
		mOwnerTargetResizedConn = mInternal->onOwnerTargetResized.connect(
			std::bind(&CGUIWidget::ownerTargetResized, this));
		mOwnerWindowFocusChangedConn = mInternal->onOwnerWindowFocusChanged.connect(
			std::bind(&CGUIWidget::ownerWindowFocusChanged, this));
	}

	CGUIWidget::CGUIWidget(const HSceneObject& parent, const HCamera& camera)
		:CGUIWidget(parent, camera->_getCamera())
	{ }

	void CGUIWidget::SetSkin(const HGUISkin& skin)
	{
		mInternal->SetSkin(skin);
	}

	const GUISkin& CGUIWidget::GetSkin() const
	{
		return mInternal->GetSkin();
	}

	const HGUISkin& CGUIWidget::GetSkinResource() const
	{
		return mInternal->GetSkinResource();
	}

	GUIPanel* CGUIWidget::getPanel() const
	{
		return mInternal->GetPanel();
	}

	UINT8 CGUIWidget::GetDepth() const
	{
		return mInternal->GetDepth();
	}

	void CGUIWidget::SetDepth(UINT8 depth)
	{
		mInternal->SetDepth(depth);
	}

	bool CGUIWidget::InBounds(const Vector2I& position) const
	{
		return mInternal->InBounds(position);
	}

	const Rect2I& CGUIWidget::GetBounds() const
	{
		return mInternal->GetBounds();
	}

	Viewport* CGUIWidget::getTarget() const
	{
		return mInternal->GetTarget();
	}

	SPtr<Camera> CGUIWidget::GetCamera() const
	{
		return mInternal->GetCamera();
	}

	const Vector<GUIElement*>& CGUIWidget::getElements() const
	{
		return mInternal->GetElements();
	}

	void CGUIWidget::Update()
	{
		HSceneObject parent = SO();

		UINT32 curHash = parent->GetTransformHash();
		if (curHash != mParentHash)
		{
			mInternal->_updateTransform(parent);
			mParentHash = curHash;
		}

		if (parent->GetActive() != mInternal->getIsActive())
			mInternal->SetIsActive(parent->getActive());

		mInternal->_updateRT();
	}

	void CGUIWidget::OnDestroyed()
	{
		mOwnerTargetResizedConn.Disconnect();
		mOwnerWindowFocusChangedConn.Disconnect();
		mInternal = nullptr;
	}

	RTTITypeBase* CGUIWidget::getRTTIStatic()
	{
		return CGUIWidgetRTTI::Instance();
	}

	RTTITypeBase* CGUIWidget::getRTTI() const
	{
		return CGUIWidget::GetRTTIStatic();
	}
}
