//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIWidget.h"
#include "GUI/BsGUIManager.h"
#include "GUI/BsGUISkin.h"
#include "GUI/BsGUILabel.h"
#include "GUI/BsGUIPanel.h"
#include "Math/BsVector2I.h"
#include "Components/BsCCamera.h"
#include "RenderAPI/BsViewport.h"
#include "Scene/BsSceneObject.h"
#include "Resources/BsBuiltinResources.h"

namespace bs
{
	GUIWidget::GUIWidget(const SPtr<Camera>& camera)
		: mCamera(camera), mPanel(nullptr), mDepth(128), mIsActive(true), mPosition(BsZero), mRotation(BsIdentity)
		, mScale(Vector3::ONE), mTransform(BsIdentity), mCachedRTId(0), mWidgetIsDirty(false)
	{
		construct(camera);
	}

	GUIWidget::GUIWidget(const HCamera& camera)
		: mCamera(camera->_getCamera()), mPanel(nullptr), mDepth(128), mIsActive(true), mPosition(BsZero)
		, mRotation(BsIdentity), mScale(Vector3::ONE), mTransform(BsIdentity), mCachedRTId(0), mWidgetIsDirty(false)
	{
		construct(mCamera);
	}

	void GUIWidget::construct(const SPtr<Camera>& camera)
	{
		if (mCamera != nullptr)
		{
			SPtr<RenderTarget> target = mCamera->getViewport()->getTarget();

			if (target != nullptr)
			{
				mOwnerTargetResizedConn = target->onResized.connect(std::bind(&GUIWidget::ownerTargetResized, this));
				mCachedRTId = target->getInternalID();
			}
		}

		GUIManager::instance().registerWidget(this);

		mPanel = GUIPanel::create();
		mPanel->_changeParentWidget(this);
		updateRootPanel();
	}

	GUIWidget::~GUIWidget()
	{
		_destroy();
	}

	SPtr<GUIWidget> GUIWidget::create(const SPtr<Camera>& camera)
	{
		return bs_shared_ptr(new (bs_alloc<GUIWidget>()) GUIWidget(camera));
	}

	SPtr<GUIWidget> GUIWidget::create(const HCamera& camera)
	{
		return bs_shared_ptr(new (bs_alloc<GUIWidget>()) GUIWidget(camera));
	}

	void GUIWidget::_destroy()
	{
		if (mPanel != nullptr)
		{
			GUILayout::destroy(mPanel);
			mPanel = nullptr;
		}

		if (mCamera != nullptr)
		{
			GUIManager::instance().unregisterWidget(this);
			mOwnerTargetResizedConn.disconnect();

			mCamera = nullptr;
		}

		mElements.clear();
		mDirtyContents.clear();
	}

	void GUIWidget::setDepth(UINT8 depth)
	{
		mDepth = depth; 
		mWidgetIsDirty = true;

		updateRootPanel();
	}

	Viewport* GUIWidget::getTarget() const
	{
		if(mCamera != nullptr)
			return mCamera->getViewport().get();

		return nullptr;
	}

	void GUIWidget::_updateTransform(const HSceneObject& parent)
	{
		// If the widgets parent scene object moved, we need to mark it as dirty
		// as the GUIManager batching relies on object positions, so it needs to be updated.
		const float diffEpsilon = 0.0001f;

		const Transform& tfrm = parent->getTransform();
		Vector3 position = tfrm.getPosition();
		Quaternion rotation = tfrm.getRotation();
		Vector3 scale = tfrm.getScale();

		if(!mWidgetIsDirty)
		{
			if(!Math::approxEquals(mPosition, position, diffEpsilon))
				mWidgetIsDirty = true;
			else
			{
				if(!Math::approxEquals(mRotation, rotation, diffEpsilon))
					mWidgetIsDirty = true;
				else
				{
					if(Math::approxEquals(mScale, scale))
						mWidgetIsDirty = true;
				}
			}
		}

		mPosition = position;
		mRotation = rotation;
		mScale = scale;
		mTransform = parent->getWorldMatrix();
	}

	void GUIWidget::_updateRT()
	{
		SPtr<RenderTarget> rt;
		UINT64 newRTId = 0;
		if(mCamera != nullptr)
		{
			rt = mCamera->getViewport()->getTarget();
			if (rt != nullptr)
				newRTId = rt->getInternalID();
		}

		if(mCachedRTId != newRTId)
		{
			mCachedRTId = newRTId;

			mOwnerTargetResizedConn.disconnect();
			if(rt != nullptr)
				mOwnerTargetResizedConn = rt->onResized.connect(std::bind(&GUIWidget::ownerTargetResized, this));

			updateRootPanel();
		}
	}

	void GUIWidget::_updateLayout()
	{
		bs_frame_mark();

		// Determine dirty contents and layouts
		FrameStack<GUIElementBase*> todo;
		todo.push(mPanel);

		while (!todo.empty())
		{
			GUIElementBase* currentElem = todo.top();
			todo.pop();

			if (currentElem->_isDirty())
			{
				GUIElementBase* updateParent = currentElem->_getUpdateParent();
				assert(updateParent != nullptr || currentElem == mPanel);

				if (updateParent != nullptr)
					_updateLayout(updateParent);
				else // Must be root panel
					_updateLayout(mPanel);
			}
			else
			{
				UINT32 numChildren = currentElem->_getNumChildren();
				for (UINT32 i = 0; i < numChildren; i++)
					todo.push(currentElem->_getChild(i));
			}
		}

		bs_frame_clear();
	}

	void GUIWidget::_updateLayout(GUIElementBase* elem)
	{
		GUIElementBase* parent = elem->_getParent();
		bool isPanelOptimized = parent != nullptr && parent->_getType() == GUIElementBase::Type::Panel;

		GUIElementBase* updateParent = nullptr;

		if (isPanelOptimized)
			updateParent = parent;
		else
			updateParent = elem;

		// For GUIPanel we can do a an optimization and update only the element in question instead
		// of all the children
		if (isPanelOptimized)
		{
			GUIPanel* panel = static_cast<GUIPanel*>(updateParent);

			GUIElementBase* dirtyElement = elem;
			dirtyElement->_updateOptimalLayoutSizes();

			LayoutSizeRange elementSizeRange = panel->_getElementSizeRange(dirtyElement);
			Rect2I elementArea = panel->_getElementArea(panel->_getLayoutData().area, dirtyElement, elementSizeRange);

			GUILayoutData childLayoutData = panel->_getLayoutData();
			panel->_updateDepthRange(childLayoutData);
			childLayoutData.area = elementArea;

			panel->_updateChildLayout(dirtyElement, childLayoutData);
		}
		else
		{
			GUILayoutData childLayoutData = updateParent->_getLayoutData();
			updateParent->_updateLayout(childLayoutData);
		}
		
		// Mark dirty contents
		bs_frame_mark();
		{
			FrameStack<GUIElementBase*> todo;
			todo.push(elem);

			while (!todo.empty())
			{
				GUIElementBase* currentElem = todo.top();
				todo.pop();

				if (currentElem->_getType() == GUIElementBase::Type::Element)
					mDirtyContents.insert(static_cast<GUIElement*>(currentElem));

				currentElem->_markAsClean();

				UINT32 numChildren = currentElem->_getNumChildren();
				for (UINT32 i = 0; i < numChildren; i++)
					todo.push(currentElem->_getChild(i));
			}
		}
		bs_frame_clear();
	}

	void GUIWidget::_registerElement(GUIElementBase* elem)
	{
		assert(elem != nullptr && !elem->_isDestroyed());

		if (elem->_getType() == GUIElementBase::Type::Element)
		{
			mElements.push_back(static_cast<GUIElement*>(elem));
			mWidgetIsDirty = true;
		}
	}

	void GUIWidget::_unregisterElement(GUIElementBase* elem)
	{
		assert(elem != nullptr);

		auto iterFind = std::find(begin(mElements), end(mElements), elem);

		if (iterFind != mElements.end())
		{
			mElements.erase(iterFind);
			mWidgetIsDirty = true;
		}

		if (elem->_getType() == GUIElementBase::Type::Element)
			mDirtyContents.erase(static_cast<GUIElement*>(elem));
	}

	void GUIWidget::_markMeshDirty(GUIElementBase* elem)
	{
		mWidgetIsDirty = true;
	}

	void GUIWidget::_markContentDirty(GUIElementBase* elem)
	{
		if (elem->_getType() == GUIElementBase::Type::Element)
			mDirtyContents.insert(static_cast<GUIElement*>(elem));
	}

	void GUIWidget::setSkin(const HGUISkin& skin)
	{
		mSkin = skin;

		for(auto& element : mElements)
			element->_refreshStyle();
	}

	const GUISkin& GUIWidget::getSkin() const
	{
		if(mSkin.isLoaded())
			return *mSkin;
		else
			return *BuiltinResources::instance().getGUISkin();
	}

	void GUIWidget::setCamera(const SPtr<Camera>& camera)
	{
		SPtr<Camera> newCamera = camera;
		if(newCamera != nullptr)
		{
			if (newCamera->getViewport()->getTarget() == nullptr)
				newCamera = nullptr;
		}

		if (mCamera == newCamera)
			return;

		GUIManager::instance().unregisterWidget(this);

		mOwnerTargetResizedConn.disconnect();

		mCamera = newCamera;

		Viewport* viewport = getTarget();
		if (viewport != nullptr && viewport->getTarget() != nullptr)
			mOwnerTargetResizedConn = viewport->getTarget()->onResized.connect(std::bind(&GUIWidget::ownerTargetResized, this));

		GUIManager::instance().registerWidget(this);

		updateRootPanel();
	}

	void GUIWidget::setIsActive(bool active)
	{
		mIsActive = active;
	}

	bool GUIWidget::isDirty(bool cleanIfDirty)
	{
		if (!mIsActive)
			return false;

		bool dirty = mWidgetIsDirty || mDirtyContents.size() > 0;

		if(cleanIfDirty && dirty)
		{
			mWidgetIsDirty = false;

			for (auto& dirtyElement : mDirtyContents)
				dirtyElement->_updateRenderElements();

			mDirtyContents.clear();
			updateBounds();
		}
		
		return dirty;
	}

	bool GUIWidget::inBounds(const Vector2I& position) const
	{
		Viewport* target = getTarget();
		if (target == nullptr)
			return false;

		// Technically GUI widget bounds can be larger than the viewport, so make sure we clip to viewport first
		if(!target->getPixelArea().contains(position))
			return false;

		Vector3 vecPos((float)position.x, (float)position.y, 0.0f);
		vecPos = mTransform.inverse().multiplyAffine(vecPos);

		Vector2I localPos(Math::roundToInt(vecPos.x), Math::roundToInt(vecPos.y));
		return mBounds.contains(localPos);
	}

	void GUIWidget::updateBounds() const
	{
		if(mElements.size() > 0)
			mBounds = mElements[0]->_getClippedBounds();

		for(auto& elem : mElements)
		{
			Rect2I elemBounds = elem->_getClippedBounds();
			mBounds.encapsulate(elemBounds);
		}
	}

	void GUIWidget::ownerTargetResized()
	{
		updateRootPanel();

		onOwnerTargetResized();
	}

	void GUIWidget::ownerWindowFocusChanged()
	{
		onOwnerWindowFocusChanged();
	}

	void GUIWidget::updateRootPanel()
	{
		Viewport* target = getTarget();
		if (target == nullptr)
			return;

		Rect2I area = target->getPixelArea();
		UINT32 width = area.width;
		UINT32 height = area.height;

		GUILayoutData layoutData;
		layoutData.area.width = width;
		layoutData.area.height = height;
		layoutData.clipRect = Rect2I(0, 0, width, height);
		layoutData.setWidgetDepth(mDepth);

		mPanel->setWidth(width);
		mPanel->setHeight(height);

		mPanel->_setLayoutData(layoutData);
		mPanel->_markLayoutAsDirty();
	}
}