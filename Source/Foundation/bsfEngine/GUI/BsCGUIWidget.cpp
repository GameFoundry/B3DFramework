//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsCGUIWidget.h"
#include "Private/RTTI/BsCGUIWidgetRTTI.h"
#include "GUI/BsGUIWidget.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCCamera.h"

using namespace bs;

CGUIWidget::CGUIWidget()
{
	SetFlag(ComponentFlag::AlwaysRun, true);
}

CGUIWidget::CGUIWidget(const HSceneObject& parent, const SPtr<Camera>& camera)
	: Component(parent), mCamera(camera), mParentHash((u32)-1)
{
	SetFlag(ComponentFlag::AlwaysRun, true);

	mInternal = GUIWidget::Create(camera);
	mOwnerTargetResizedConn = mInternal->OnOwnerTargetResized.Connect(
		std::bind(&CGUIWidget::OwnerTargetResized, this));
	mOwnerWindowFocusChangedConn = mInternal->OnOwnerWindowFocusChanged.Connect(
		std::bind(&CGUIWidget::OwnerWindowFocusChanged, this));
}

CGUIWidget::CGUIWidget(const HSceneObject& parent, const HCamera& camera)
	: CGUIWidget(parent, camera->GetCameraInternal())
{}

const GUIStyleSheetCascade& CGUIWidget::GetStyleSheetCascade() const
{
	return mInternal->GetStyleSheetCascade();
}

void CGUIWidget::SetStyleSheetCascade(const SPtr<const GUIStyleSheetCascade>& styleSheetCascade)
{
	mInternal->SetStyleSheetCascade(styleSheetCascade);
}

GUIPanel* CGUIWidget::GetPanel() const
{
	return mInternal->GetPanel();
}

u8 CGUIWidget::GetDepth() const
{
	return mInternal->GetDepth();
}

void CGUIWidget::SetDepth(u8 depth)
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

Viewport* CGUIWidget::GetTarget() const
{
	return mInternal->GetTarget();
}

SPtr<Camera> CGUIWidget::GetCamera() const
{
	return mInternal->GetCamera();
}

const Vector<GUIRenderable*>& CGUIWidget::GetElements() const
{
	return mInternal->GetElements();
}

void CGUIWidget::Update()
{
	HSceneObject parent = SO();

	u32 curHash = parent->GetTransformHash();
	if(curHash != mParentHash)
	{
		mInternal->UpdateTransformInternal(parent);
		mParentHash = curHash;
	}

	if(parent->GetActive() != mInternal->GetIsActive())
		mInternal->SetIsActive(parent->GetActive());

	mInternal->UpdateRTInternal();
}

void CGUIWidget::OnDestroyed()
{
	mOwnerTargetResizedConn.Disconnect();
	mOwnerWindowFocusChangedConn.Disconnect();
	mInternal = nullptr;
}

RTTITypeBase* CGUIWidget::GetRttiStatic()
{
	return CGUIWidgetRTTI::Instance();
}

RTTITypeBase* CGUIWidget::GetRtti() const
{
	return CGUIWidget::GetRttiStatic();
}
