//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIWidget.h"
#include "GUI/BsGUIManager.h"
#include "GUI/BsGUILabel.h"
#include "GUI/BsGUIPanel.h"
#include "GUI/BsGUINavGroup.h"
#include "Math/BsVector2I.h"
#include "Mesh/BsMesh.h"
#include "Components/BsCCamera.h"
#include "RenderAPI/BsViewport.h"
#include "Scene/BsSceneObject.h"
#include "Resources/BsBuiltinResources.h"
#include "RenderAPI/BsRenderTexture.h"

using namespace bs;

GUIWidget::GUIWidget(const SPtr<Camera>& camera)
	: mCamera(camera), mBatches(this)
{
	Construct(camera);
}

GUIWidget::GUIWidget(const HCamera& camera)
	: mCamera(camera->GetCameraInternal()), mBatches(this)
{
	Construct(mCamera);
}

void GUIWidget::Construct(const SPtr<Camera>& camera)
{
	mStyleSheet = GetBuiltinResources().GetDefaultGUIStyleSheet();

	if(mCamera != nullptr)
	{
		SPtr<RenderTarget> target = mCamera->GetViewport()->GetTarget();

		if(target != nullptr)
			mCachedRTId = target->GetInternalId();
	}

	mDefaultNavGroup = GUINavGroup::Create();

	GUIManager::Instance().RegisterWidget(this);

	mPanel = GUIPanel::Create();
	mPanel->ChangeParentWidget(this);
	UpdateRootPanel();
}

GUIWidget::~GUIWidget()
{
	DestroyInternal();
}

SPtr<GUIWidget> GUIWidget::Create(const SPtr<Camera>& camera)
{
	return B3DMakeSharedFromExisting(new(B3DAllocate<GUIWidget>()) GUIWidget(camera));
}

SPtr<GUIWidget> GUIWidget::Create(const HCamera& camera)
{
	return B3DMakeSharedFromExisting(new(B3DAllocate<GUIWidget>()) GUIWidget(camera));
}

void GUIWidget::DestroyInternal()
{
	if(mPanel != nullptr)
	{
		GUILayout::Destroy(mPanel);
		mPanel = nullptr;
	}

	if(mCamera != nullptr)
	{
		GUIManager::Instance().UnregisterWidget(this);
		mCamera = nullptr;
	}

	mElements.clear();
	mDirtyContents.clear();
}

void GUIWidget::SetDepth(u8 depth)
{
	mDepth = depth;
	mWidgetIsDirty = true;

	UpdateRootPanel();
}

Viewport* GUIWidget::GetTarget() const
{
	if(mCamera != nullptr)
		return mCamera->GetViewport().get();

	return nullptr;
}

void GUIWidget::UpdateTransformInternal(const HSceneObject& parent)
{
	// If the widgets parent scene object moved, we need to mark it as dirty
	// as the GUIManager batching relies on object positions, so it needs to be updated.
	const float diffEpsilon = 0.0001f;

	const Transform& tfrm = parent->GetTransform();
	Vector3 position = tfrm.GetPosition();
	Quaternion rotation = tfrm.GetRotation();
	Vector3 scale = tfrm.GetScale();

	if(!mWidgetIsDirty)
	{
		if(!Math::ApproxEquals(mPosition, position, diffEpsilon))
			mWidgetIsDirty = true;
		else
		{
			if(!Math::ApproxEquals(mRotation, rotation, diffEpsilon))
				mWidgetIsDirty = true;
			else
			{
				if(Math::ApproxEquals(mScale, scale))
					mWidgetIsDirty = true;
			}
		}
	}

	mPosition = position;
	mRotation = rotation;
	mScale = scale;
	mTransform = parent->GetWorldMatrix();
}

void GUIWidget::UpdateRTInternal()
{
	SPtr<RenderTarget> rt;
	u64 newRTId = 0;
	if(mCamera != nullptr)
	{
		rt = mCamera->GetViewport()->GetTarget();
		if(rt != nullptr)
			newRTId = rt->GetInternalId();
	}

	if(mCachedRTId != newRTId)
	{
		mCachedRTId = newRTId;
		UpdateRootPanel();
	}
}

void GUIWidget::UpdateLayoutInternal()
{
	// Check if render target size changed and update if needed
	// Note: Purposely not relying to the RenderTarget::onResized callback, as it will trigger /before/ Input events.
	// These events might trigger a resize, meaning the size would be delayed one frame, resulting in a visual artifact
	// where the GUI doesn't match the target size.
	Viewport* target = GetTarget();
	if(target != nullptr)
	{
		Rect2I area = target->GetPixelArea();
		u32 width = area.Width;
		u32 height = area.Height;

		const Rect2I& panelArea = mPanel->GetLayoutData().Area;
		if(panelArea.Width != width || panelArea.Height != height)
		{
			UpdateRootPanel();
			OnOwnerTargetResized();
		}
	}

	B3DMarkAllocatorFrame();

	// Determine dirty contents and layouts
	FrameStack<GUIElementBase*> todo;
	todo.push(mPanel);

	while(!todo.empty())
	{
		GUIElementBase* currentElem = todo.top();
		todo.pop();

		if(currentElem->IsDirty())
		{
			GUIElementBase* updateParent = currentElem->GetUpdateParent();
			B3D_ASSERT(updateParent != nullptr || currentElem == mPanel);

			if(updateParent != nullptr)
				UpdateLayoutInternal(updateParent);
			else // Must be root panel
				UpdateLayoutInternal(mPanel);
		}
		else
		{
			u32 numChildren = currentElem->GetChildCount();
			for(u32 i = 0; i < numChildren; i++)
				todo.push(currentElem->GetChild(i));
		}
	}

	B3DClearAllocatorFrame();
}

void GUIWidget::UpdateLayoutInternal(GUIElementBase* elem)
{
	GUIElementBase* parent = elem->GetParent();
	bool isPanelOptimized = parent != nullptr && parent->GetType() == GUIElementBase::Type::Panel;

	GUIElementBase* updateParent = nullptr;

	if(isPanelOptimized)
		updateParent = parent;
	else
		updateParent = elem;

	// For GUIPanel we can do a an optimization and update only the element in question instead
	// of all the children
	if(isPanelOptimized)
	{
		GUIPanel* panel = static_cast<GUIPanel*>(updateParent);

		GUIElementBase* dirtyElement = elem;
		dirtyElement->UpdateOptimalLayoutSizes();

		LayoutSizeRange elementSizeRange = panel->GetElementSizeRangeInternal(dirtyElement);
		Rect2I elementArea = panel->GetElementAreaInternal(panel->GetLayoutData().Area, dirtyElement, elementSizeRange);

		GUILayoutData childLayoutData = panel->GetLayoutData();
		panel->UpdateDepthRangeInternal(childLayoutData);
		childLayoutData.Area = elementArea;

		panel->UpdateChildLayoutInternal(dirtyElement, childLayoutData);
	}
	else
	{
		GUILayoutData childLayoutData = updateParent->GetLayoutData();
		updateParent->UpdateLayout(childLayoutData);
	}

	// Mark dirty contents
	B3DMarkAllocatorFrame();
	{
		FrameStack<GUIElementBase*> todo;
		todo.push(elem);

		while(!todo.empty())
		{
			GUIElementBase* currentElem = todo.top();
			todo.pop();

			MarkContentDirty(currentElem);
			currentElem->MarkAsClean();

			u32 numChildren = currentElem->GetChildCount();
			for(u32 i = 0; i < numChildren; i++)
				todo.push(currentElem->GetChild(i));
		}
	}
	B3DClearAllocatorFrame();
}

void GUIWidget::RegisterElement(GUIElementBase* guiElementBase)
{
	B3D_ASSERT(guiElementBase != nullptr && !guiElementBase->IsDestroyed());

	if(guiElementBase->GetType() == GUIElementBase::Type::Element)
	{
		mElements.push_back(static_cast<GUIElement*>(guiElementBase));
		mWidgetIsDirty = true;

		if(guiElementBase->IsVisible())
		{
			auto guiElement = static_cast<GUIElement*>(guiElementBase);
			mBatches.Add(guiElement);
			mBatches.MarkContentDirty(guiElement);
		}
	}
}

void GUIWidget::UnregisterElement(GUIElementBase* guiElement)
{
	B3D_ASSERT(guiElement != nullptr);

	auto iterFind = std::find(begin(mElements), end(mElements), guiElement);

	if(iterFind != mElements.end())
	{
		mElements.erase(iterFind);
		mWidgetIsDirty = true;
	}

	if(guiElement->GetType() == GUIElementBase::Type::Element)
	{
		mDirtyContents.erase(static_cast<GUIElement*>(guiElement));

		const auto guiElem = static_cast<GUIElement*>(guiElement);
		mBatches.Remove(guiElem);
	}
}

void GUIWidget::NotifyElementVisibilityChanged(GUIElementBase* guiElementBase, bool isVisible)
{
	if(guiElementBase->GetType() != GUIElementBase::Type::Element)
		return;

	const auto guiElement = static_cast<GUIElement*>(guiElementBase);
	if(isVisible)
		mBatches.Add(guiElement);
	else
		mBatches.Remove(guiElement);
}

void GUIWidget::MarkMeshDirty(GUIElementBase* elem)
{
	mWidgetIsDirty = true;

	if(elem->GetType() == GUIElementBase::Type::Element)
		mBatches.MarkMeshDirty(static_cast<GUIElement*>(elem));
}

void GUIWidget::MarkContentDirty(GUIElementBase* elem)
{
	if(elem->GetType() == GUIElementBase::Type::Element)
	{
		auto guiElement = static_cast<GUIElement*>(elem);
		if(!guiElement->IsVisible())
			return;

		mDirtyContents.insert(guiElement);
		mBatches.MarkContentDirty(guiElement);
	}
}

void GUIWidget::SetSkin(const HGUISkin& skin)
{
	mSkin = skin;

	for(auto& element : mElements)
		element->NotifyStyleSheetChanged();
}

const GUISkin& GUIWidget::GetSkin() const
{
	if(mSkin.IsLoaded())
		return *mSkin;
	else
		return *BuiltinResources::Instance().GetGuiSkin();
}

void GUIWidget::SetStyleSheet(const HGUIStyleSheet& styleSheet)
{
	mStyleSheet = styleSheet;

	for(auto& element : mElements)
		element->NotifyStyleSheetChanged();
}

void GUIWidget::SetCamera(const SPtr<Camera>& camera)
{
	SPtr<Camera> newCamera = camera;
	if(newCamera != nullptr)
	{
		if(newCamera->GetViewport()->GetTarget() == nullptr)
			newCamera = nullptr;
	}

	if(mCamera == newCamera)
		return;

	GUIManager::Instance().UnregisterWidget(this);
	mCamera = newCamera;
	GUIManager::Instance().RegisterWidget(this);

	UpdateRootPanel();
}

void GUIWidget::SetIsActive(bool active)
{
	mIsActive = active;
}

GUIDrawGroupRenderDataUpdate GUIWidget::RebuildDirtyRenderData()
{
	if(!mIsActive)
		return GUIDrawGroupRenderDataUpdate();

	const bool dirty = mWidgetIsDirty || !mDirtyContents.empty();

	if(dirty)
	{
		// Update render contents recursively because updates can cause child GUI elements to become dirty
		while(!mDirtyContents.empty())
		{
			mDirtyContentsTemp.swap(mDirtyContents);

			for(auto& dirtyElement : mDirtyContentsTemp)
				dirtyElement->UpdateRenderElements();

			mDirtyContentsTemp.clear();
		}

		UpdateBounds();
	}

	mWidgetIsDirty = false;
	return mBatches.RebuildDirty(dirty);
}

bool GUIWidget::InBounds(const Vector2I& position) const
{
	Viewport* target = GetTarget();
	if(target == nullptr)
		return false;

	// Technically GUI widget bounds can be larger than the viewport, so make sure we clip to viewport first
	if(!target->GetPixelArea().Contains(position))
		return false;

	Vector3 vecPos((float)position.X, (float)position.Y, 0.0f);
	vecPos = mTransform.Inverse().MultiplyAffine(vecPos);

	Vector2I localPos(Math::RoundToI32(vecPos.X), Math::RoundToI32(vecPos.Y));
	return mBounds.Contains(localPos);
}

void GUIWidget::UpdateBounds() const
{
	if(!mElements.empty())
		mBounds = mElements[0]->GetClippedBounds();

	for(auto& elem : mElements)
	{
		Rect2I elemBounds = elem->GetClippedBounds();
		mBounds.Encapsulate(elemBounds);
	}
}

void GUIWidget::OwnerWindowFocusChanged()
{
	OnOwnerWindowFocusChanged();
}

void GUIWidget::UpdateRootPanel()
{
	Viewport* target = GetTarget();
	if(target == nullptr)
		return;

	Rect2I area = target->GetPixelArea();
	u32 width = area.Width;
	u32 height = area.Height;

	GUILayoutData layoutData;
	layoutData.Area.Width = width;
	layoutData.Area.Height = height;
	layoutData.ClipRect = Rect2I(0, 0, width, height);
	layoutData.SetWidgetDepth(mDepth);

	mPanel->SetWidth(width);
	mPanel->SetHeight(height);

	mPanel->SetLayoutData(layoutData);
	mPanel->MarkLayoutAsDirty();
}
