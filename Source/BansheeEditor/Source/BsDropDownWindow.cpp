//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsDropDownWindow.h"
#include "BsSceneObject.h"
#include "BsCGUIWidget.h"
#include "BsGUIPanel.h"
#include "BsGUITexture.h"
#include "BsGUIWindowFrame.h"
#include "BsDropDownAreaPlacement.h"
#include "BsBuiltinEditorResources.h"
#include "BsGUIDropDownHitBox.h"
#include "BsDropDownWindowManager.h"
#include "BsCamera.h"

namespace bs
{
	DropDownWindow::DropDownWindow(const SPtr<RenderWindow>& parent, const SPtr<Camera>& camera,
		const Vector2I& position, UINT32 width, UINT32 height)
		: mRenderWindow(parent), mFrontHitBox(nullptr), mBackHitBox(nullptr), mRootPanel(nullptr), mPosition(position)
		, mWidth(width), mHeight(height)
	{
		mSceneObject = SceneObject::create("EditorWindow", SOF_Internal | SOF_Persistent | SOF_DontSave);

		mGUI = mSceneObject->addComponent<CGUIWidget>(camera);

		mGUI->setDepth(1); // Needs to be in front of everything (except other drop down and tooltip elements)
		mGUI->setSkin(BuiltinEditorResources::instance().getSkin());

		mRootPanel = mGUI->getPanel()->addNewElement<GUIPanel>();
		
		GUIPanel* frontHitBoxPanel = mRootPanel->addNewElement<GUIPanel>(std::numeric_limits<INT16>::min());
		mFrontHitBox = GUIDropDownHitBox::create(false, false);
		mFrontHitBox->onFocusGained.connect(std::bind(&DropDownWindow::dropDownFocusGained, this));
		frontHitBoxPanel->addElement(mFrontHitBox);

		GUIPanel* backHitBoxPanel = mRootPanel->addNewElement<GUIPanel>(std::numeric_limits<INT16>::max());
		mBackHitBox = GUIDropDownHitBox::create(false, true);
		backHitBoxPanel->addElement(mBackHitBox);
		
		SPtr<Viewport> viewport = camera->getViewport();
		GUIPanel* captureHitBoxPanel = mGUI->getPanel()->addNewElement<GUIPanel>(std::numeric_limits<INT16>::max());
		GUIDropDownHitBox* captureHitBox = GUIDropDownHitBox::create(true, false);
		captureHitBox->setBounds(Rect2I(0, 0, viewport->getWidth(), viewport->getHeight()));
		captureHitBoxPanel->addElement(captureHitBox);

		setSize(width, height);

		GUIPanel* backgroundPanel = mRootPanel->addNewElement<GUIPanel>(500);
		backgroundPanel->addElement(GUITexture::create(TextureScaleMode::RepeatToFit,
			GUIOptions(GUIOption::flexibleWidth(), GUIOption::flexibleHeight()), "WindowBackground"));

		GUIPanel* windowFramePanel = mRootPanel->addNewElement<GUIPanel>(499);

		GUIWindowFrame* windowFrame = GUIWindowFrame::create("WindowFrame");
		windowFramePanel->addElement(windowFrame);

		mContents = mRootPanel->addNewElement<GUIPanel>();
		mContents->setPosition(1, 1);
		mContents->setWidth(width - 2);
		mContents->setHeight(height - 2);
	}

	DropDownWindow::~DropDownWindow()
	{
		mSceneObject->destroy();
	}

	Vector2I DropDownWindow::screenToWindowPos(const Vector2I& screenPos) const
	{
		Vector2I renderWindowPos = mRenderWindow->screenToWindowPos(screenPos);

		Vector2I contentsPos = renderWindowPos;
		Rect2I contentArea = mContents->getGlobalBounds();

		contentsPos.x -= contentArea.x;
		contentsPos.y -= contentArea.y;

		return contentsPos;
	}

	Vector2I DropDownWindow::windowToScreenPos(const Vector2I& windowPos) const
	{
		Vector2I contentsPos = windowPos;
		Rect2I contentArea = mContents->getGlobalBounds();

		contentsPos.x += contentArea.x;
		contentsPos.y += contentArea.y;

		return mRenderWindow->windowToScreenPos(contentsPos);
	}

	void DropDownWindow::setSize(UINT32 width, UINT32 height)
	{
		Rect2I availableBounds(0, 0, mGUI->getTarget()->getWidth(), mGUI->getTarget()->getHeight());
		DropDownAreaPlacement dropDownPlacement = DropDownAreaPlacement::aroundPosition(mPosition);

		DropDownAreaPlacement::HorzDir horzDir;
		DropDownAreaPlacement::VertDir vertDir;
		Rect2I placementBounds = dropDownPlacement.getOptimalBounds(width, height, availableBounds, horzDir, vertDir);
		placementBounds.width = width;
		placementBounds.height = height;

		mRootPanel->setPosition(placementBounds.x, placementBounds.y);
		mRootPanel->setWidth(width);
		mRootPanel->setHeight(height);

		Vector<Rect2I> nonDropDownBounds;
		availableBounds.cut(placementBounds, nonDropDownBounds);

		mFrontHitBox->setBounds(nonDropDownBounds);
		mBackHitBox->setBounds(placementBounds);

		mWidth = width;
		mHeight = height;
	}

	void DropDownWindow::close()
	{
		DropDownWindowManager::instance().close();
	}

	void DropDownWindow::dropDownFocusGained()
	{
		close();
	}
}