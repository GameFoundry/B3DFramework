GUI setup									{#guiSetup}
===============

All GUI elements in Banshee are handled by a @ref bs::CGUIWidget "GUIWidget" component. Each such component must have an attached **Camera** component, which determines where will the rendered GUI elements be output. 

The camera is created in the same way as we shown before, and you can in-fact use the same camera you use for normal scene rendering. GUI elements will not be affected by camera's position, orientation or projection properties - they might however be affected by the size of the camera's render target.

~~~~~~~~~~~~~{.cpp}
SPtr<RenderWindow> primaryWindow = gApplication().getPrimaryWindow();

HSceneObject cameraSO = SceneObject::create("Camera");
HCamera camera = cameraSO->addComponent<CCamera>(primaryWindow);
~~~~~~~~~~~~~

And we create a **GUIWidget** the same as with any other component.

~~~~~~~~~~~~~{.cpp}
HSceneObject guiSO = SceneObject::create("GUI");
HGUIWidget gui = guiSO->addComponent<CGUIWidget>(camera);
~~~~~~~~~~~~~

Before a widget is usable we must first assign it a **GUISkin**. A skin defines how is every element on this particular widget rendered (its texture(s), fonts, size, etc.). We'll talk about skins and styles in detail later, but for now it's enough to assign the built-in skin available from **BuiltinResources**.

~~~~~~~~~~~~~{.cpp}
gui->setSkin(BuiltinResources::instance().getGUISkin());
~~~~~~~~~~~~~

You can now use the GUI widget to add GUI elements to it
~~~~~~~~~~~~~{.cpp}
// Shows the text "Hello!" on the screen
GUIPanel* mainPanel = gui->getPanel();
mainPanel->addElement(GUILabel::create(HString(L"Hello!")));

// ... add more elements ...
~~~~~~~~~~~~~

Don't worry about what **GUIPanel** or **GUILabel** mean at this time, we'll talk about GUI panels, elements and layouts in later chapters. 

# Using a separate GUI camera
In the example above we have asssumed you will use the same camera for GUI as you use for scene rendering. However sometimes it is useful to have a separate camera for GUI, or even multiple separate cameras. In such case camera creation is mostly the same, but with some additional options that need to be enabled. 

Lets see how to create a camera that can be used for GUI rendering along with scene rendering. Initial creation of the camera is identical, we just choose a render target:

~~~~~~~~~~~~~{.cpp}
SPtr<RenderWindow> primaryWindow = gApplication().getPrimaryWindow();

HSceneObject guiCameraSO = SceneObject::create("GUI camera");
HCamera guiCamera = guiCameraSO->addComponent<CCamera>(primaryWindow);
~~~~~~~~~~~~~

In order to prevent the camera from rendering scene objects, we enable the @ref bs::CameraFlag::Overlay "Overlay" option.
~~~~~~~~~~~~~{.cpp}
guiCamera->setFlag(CameraFlag::Overlay, true);
~~~~~~~~~~~~~

Now our camera will render just overlay objects (GUI and sprites), and nothing else. 

Next, we need to make sure the camera renders after the main scene camera - if we don't then the scene rendering might render on top of the GUI. Using the @ref bs::CCamera::setPriority "CCamera::setPriority()" method we can control the order in which cameras sharing the same render target are rendered. By default all cameras have a priority of 0, and since we want our GUI camera to render after the scene camera, we set its priority to -1.

~~~~~~~~~~~~~{.cpp}
guiCamera->setPriority(-1);
~~~~~~~~~~~~~

And finally, we want to prevent the camera from clearing the render target. By default cameras will set all the pixels in the render target to some default value before they start rendering, every frame. We want our GUI camera to just render on top of anything rendered by the scene camera, so we disable that functionality by retrieving a @ref bs::Viewport "Viewport" from the camera. 

**Viewport** is retrieved by calling @ref bs::CCamera::getViewport "CCamera::getViewport()". It allows you to set how and if the render target is cleared through @ref bs::Viewport::setRequiresClear "Viewport::setRequiresClear()" and @ref bs::Viewport::setClearValues "Viewport::setClearValues()". Clear options can be set separately for color, depth and stencil buffers.

~~~~~~~~~~~~~{.cpp}
SPtr<Viewport> viewport = guiCamera->getViewport();

// Disable clear for color, depth and stencil buffers
viewport->setRequiresClear(false, false, false);
~~~~~~~~~~~~~

> You can also use the viewport to control onto which portion of the render target should the camera render to. By default it will output to the entire render target but you can change the area by calling @ref bs::Viewport::setArea "Viewport::setArea()".

At this point you can use the camera to create a **GUIWidget** and use the GUI as normal.

# Transforming GUI

Once you have set up a **GUIWidget** component, you can transform it using its scene object as normal. This allows you to apply 3D transformations to GUI elements, which can be useful for various interesting effects, including rendering GUI to in-game surfaces (like on a screen of an in-game 3D monitor).

~~~~~~~~~~~~~{.cpp}
// Rotate 30 degrees around the Z axis
Quaternion rotate(Vector3::UNIT_Z, Degree(30.0f));
guiSO->setRotation(rotate);
~~~~~~~~~~~~~

@ref TODO_IMAGE