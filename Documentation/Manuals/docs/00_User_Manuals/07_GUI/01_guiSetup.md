---
title: GUI setup
---

All GUI elements in the framework are managed by a @b3d::GUIWidget component. Each widget must have an attached **Camera** component, which determines where the rendered GUI elements will be output.

The camera is created in the same way as shown in previous chapters, and you can use the same camera for both GUI and normal scene rendering. GUI elements will not be affected by the camera's position, orientation, or projection properties, though they may be affected by the size of the camera's render target.

~~~~~~~~~~~~~{.cpp}
SPtr<RenderWindow> primaryWindow = GetApplication().GetPrimaryWindow();

HSceneObject cameraSceneObject = SceneObject::Create("Camera");
HCamera camera = cameraSceneObject->AddComponent<Camera>(primaryWindow);
~~~~~~~~~~~~~

Create a **GUIWidget** the same as with any other component:

~~~~~~~~~~~~~{.cpp}
HSceneObject guiSceneObject = SceneObject::Create("GUI");
HGUIWidget guiWidget = guiSceneObject->AddComponent<GUIWidget>(camera);
~~~~~~~~~~~~~

Before a widget is usable, you must assign it a style sheet cascade. The cascade defines how each element on this widget will be styled (colors, fonts, sizes, etc.). For now, assign the built-in style sheet available from the resources:

~~~~~~~~~~~~~{.cpp}
guiWidget->SetStyleSheetCascade(GetBuiltinResources().GetDefaultGUIStyleSheetCascade());
~~~~~~~~~~~~~

You can now use the GUI widget to add GUI elements to it:

~~~~~~~~~~~~~{.cpp}
// Shows the text "Hello!" on the screen
GUIPanel* mainPanel = guiWidget->GetPanel();
GUILabel* label = mainPanel->AddElement(GUILabel::Create(GUIContent(HString("Hello!"))));

// ... add more elements ...
~~~~~~~~~~~~~

# Transforming GUI

Once you have set up a **GUIWidget** component, you can transform it using its scene object as normal. This allows you to apply 3D transformations to GUI elements, which can be used for various effects, including rendering GUI to in-game surfaces such as screens or monitors.

~~~~~~~~~~~~~{.cpp}
// Rotate 30 degrees around the Z axis
Quaternion rotation(Vector3::kUnitZ, Degree(30.0f));
guiSceneObject->SetRotation(rotation);
~~~~~~~~~~~~~

# Using a separate GUI camera

In the example above, we assumed you would use the same camera for both GUI and scene rendering. However, sometimes it is better to have a separate camera for GUI, or even multiple separate cameras. In such cases, camera creation is mostly the same, but with some additional options that need to be configured.

Initial creation of the camera is identical:

~~~~~~~~~~~~~{.cpp}
SPtr<RenderWindow> primaryWindow = GetApplication().GetPrimaryWindow();

HSceneObject guiCameraSceneObject = SceneObject::Create("GUI camera");
HCamera guiCamera = guiCameraSceneObject->AddComponent<Camera>(primaryWindow);
~~~~~~~~~~~~~

To prevent the camera from rendering scene objects, enable the **RenderSettings::OverlayOnly** property on the camera's **RenderSettings** object:

~~~~~~~~~~~~~{.cpp}
SPtr<RenderSettings> renderSettings = guiCamera->GetRenderSettings();
renderSettings->OverlayOnly = true;

guiCamera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

Now the camera will render only overlay objects (GUI and sprites), and nothing else.

Next, prevent the camera from clearing the render target. By default, cameras will set all pixels in the render target to a default value before they start rendering each frame. Since we want the GUI camera to render on top of content already rendered by the scene camera, we need to disable this functionality. Retrieve the @b3d::Viewport from the camera and configure its clear behavior:

**Viewport** is retrieved by calling @b3d::Camera::GetViewport. Set whether the render target is cleared through @b3d::Viewport::SetClearFlags by providing the @b3d::ClearFlagBits::Empty flag:

~~~~~~~~~~~~~{.cpp}
SPtr<Viewport> viewport = guiCamera->GetViewport();

// Disable clear for color, depth and stencil buffers
viewport->SetClearFlags(ClearFlagBits::Empty);
~~~~~~~~~~~~~

You can also use the viewport to control which portion of the render target the camera renders to. By default, it will output to the entire render target, but you can change the area by calling @b3d::Viewport::SetArea:

~~~~~~~~~~~~~{.cpp}
// Render to the top-left quadrant of the render target
Area2 viewportArea(0.0f, 0.0f, 0.5f, 0.5f);
viewport->SetArea(viewportArea);
~~~~~~~~~~~~~

At this point, you can use the camera to create a **GUIWidget** and use the GUI as normal.
