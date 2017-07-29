//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsEditorWindowBase.h"
#include "BsEditorApplication.h"
#include "BsCoreThread.h"
#include "BsSceneObject.h"
#include "BsRenderWindow.h"
#include "BsMainEditorWindow.h"
#include "BsEditorWindowManager.h"
#include "BsCCamera.h"
#include "BsGUIWindowFrameWidget.h"
#include "BsBuiltinEditorResources.h"

namespace bs
{
	EditorWindowBase::EditorWindowBase(bool isModal)
		:mOwnsRenderWindow(true), mIsModal(isModal)
	{
		UINT32 width = 200;
		UINT32 height = 200;

		INT32 left = -1;
		INT32 top = -1;

		// If possible open the window in the center of the main editor window
		MainEditorWindow* mainWindow = EditorWindowManager::instance().getMainWindow();
		if(mainWindow != nullptr)
		{
			left = mainWindow->getLeft() + mainWindow->getWidth() / 2 - width / 2;
			top = mainWindow->getTop() + mainWindow->getHeight() / 2 - height / 2;
		}


		RENDER_WINDOW_DESC renderWindowDesc;
		renderWindowDesc.videoMode = VideoMode(width, height);
		renderWindowDesc.title = "EditorWindow";
		renderWindowDesc.fullscreen = false;
		renderWindowDesc.border = WindowBorder::None;
		renderWindowDesc.toolWindow = true;
		renderWindowDesc.modal = isModal;
		renderWindowDesc.hideUntilSwap = true;
		renderWindowDesc.left = left;
		renderWindowDesc.top = top;
		renderWindowDesc.depthBuffer = false;

		mRenderWindow = RenderWindow::create(renderWindowDesc, gCoreApplication().getPrimaryWindow());

		construct(mRenderWindow);
	}

	EditorWindowBase::EditorWindowBase(const SPtr<RenderWindow>& renderWindow)
		:mOwnsRenderWindow(false), mIsModal(false)
	{
		construct(renderWindow);
	}

	EditorWindowBase::~EditorWindowBase()
	{
		mResizedConn.disconnect();

		if(mOwnsRenderWindow)
			mRenderWindow->destroy();

		mSceneObject->destroy();
	}

	void EditorWindowBase::initialize()
	{
		setSize(200, 200);
	}

	void EditorWindowBase::close()
	{
		EditorWindowManager::instance().destroy(this);
	}

	void EditorWindowBase::hide()
	{
		mRenderWindow->hide();
	}

	void EditorWindowBase::construct(const SPtr<RenderWindow>& renderWindow)
	{
		mRenderWindow = renderWindow;
		mSceneObject = SceneObject::create("EditorWindow", SOF_Internal | SOF_Persistent | SOF_DontSave);

		mCamera = mSceneObject->addComponent<CCamera>(renderWindow, 0.0f, 0.0f, 1.0f, 1.0f);
		mCamera->setNearClipDistance(5);
		mCamera->setAspectRatio(1.0f);
		mCamera->setLayers(0);

		SPtr<RenderSettings> settings = mCamera->getRenderSettings();
		settings->overlayOnly = true;

		mGUI = mSceneObject->addComponent<CGUIWidget>(mCamera);
		mGUI->setDepth(128);

		mGUI->setSkin(BuiltinEditorResources::instance().getSkin());

		mWindowFrame = mSceneObject->addComponent<WindowFrameWidget>(!mIsModal, mCamera->_getCamera(), renderWindow.get(), BuiltinEditorResources::instance().getSkin());
		mWindowFrame->setDepth(129);

		mResizedConn = renderWindow->onResized.connect(std::bind(&EditorWindowBase::resized, this));
	}

	void EditorWindowBase::setPosition(INT32 x, INT32 y)
	{
		mRenderWindow->move(x, y);
	}

	void EditorWindowBase::setSize(UINT32 width, UINT32 height)
	{
		mRenderWindow->resize(width, height);
	}

	INT32 EditorWindowBase::getLeft() const
	{
		return mRenderWindow->getProperties().getLeft();
	}

	INT32 EditorWindowBase::getTop() const
	{
		return mRenderWindow->getProperties().getTop();
	}

	UINT32 EditorWindowBase::getWidth() const
	{
		return (UINT32)mRenderWindow->getProperties().getWidth();
	}

	UINT32 EditorWindowBase::getHeight() const
	{
		return (UINT32)mRenderWindow->getProperties().getHeight();
	}

	bool EditorWindowBase::hasFocus() const
	{
		return mRenderWindow->getProperties().hasFocus();
	}
}