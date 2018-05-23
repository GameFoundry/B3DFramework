Cameras 						{#cameras}
===============
[TOC]

Cameras represent the user's view into the scene, and any graphical application will require at least one camera for the user to be able to see anything. 

They have parameters like position and orientation which define what part of the scene will the user see. Additionally, their parameters like field of view, projection mode and aspect ratio define how is the application scene transformed into the 2D output visible to the user. 

Finally, everything that the camera sees is output to what we call a render target. Render targets can be windows, like the one that was created when the application was started, or an off-screen surface, as we'll explain later.

Cameras are represented by the @ref bs::CCamera "Camera" component, and they can be created as any other component. 

~~~~~~~~~~~~~{.cpp}
HSceneObject cameraSO = SceneObject::create("Camera");
HCamera camera = cameraSO->addComponent<CCamera>();
~~~~~~~~~~~~~

Before the camera can render anything, you need to assign the render target to which the camera will output its contents to. Lets create a camera that renders to the primary render window. The primary application window can be retrieved through @ref bs::Application::getPrimaryWindow "Application::getPrimaryWindow()".

To assign the window, retrieve the @ref bs::Viewport "Viewport" object from the camera, and set its target using the @ref bs::Viewport::setTarget "Viewport::setTarget()" method.

~~~~~~~~~~~~~{.cpp}
SPtr<RenderWindow> primaryWindow = gApplication().getPrimaryWindow();
camera->getViewport()->setTarget(primaryWindow);
~~~~~~~~~~~~~

> **Application** is a singleton and its instance can be accessed through @ref bs::Application::instance() "Application::instance()", or the helper method @ref bs::gApplication() "gApplication()". All other singletons in the framework follow the same design.

Or alternatively, you can just mark the camera as 'main', which will render to the default render target (in this case, the primary window).

~~~~~~~~~~~~~{.cpp}
camera->setMain(true);
~~~~~~~~~~~~~

Once the camera has been created we can move and orient it using the **SceneObject** transform, as explained earlier. For example:
~~~~~~~~~~~~~{.cpp}
// Move camera to 10 meters height, and 50 meters away from the center
cameraSO->setPosition(Vector3(0f, 10f, 50f));

// Orient the camera so it is looking at the center
cameraSO->lookAt(Vector3(0f, 0f, 0f));
~~~~~~~~~~~~~

Once set up, any rendered objects in the camera's view will be displayed on the selected render target, which is in this case the primary application window.

You can also customize a variety of parameters that control how will the camera render the objects.

# Projection type {#cameras_a}
All cameras can be in two projection modes: *Perspective* and *Ortographic*. They can be changed by calling @ref bs::CCamera::setProjectionType "CCamera::setProjectionType()".

## Perspective cameras {#cameras_a_a}
This mode simulates human vision, where objects farther away appear smaller. This is what you will need for most 3D applications.

~~~~~~~~~~~~~{.cpp}
camera->setProjectionType(PT_PERSPECTIVE);
~~~~~~~~~~~~~

![Model drawn using the perspective camera](PerspectiveCamera.png)  

## Ortographic {#cameras_a_b}
Renders the image without perspective distortion, ensuring objects remain the same size regardless of the distance from camera, essentially "flattening" the image. Useful for 2D applications.

~~~~~~~~~~~~~{.cpp}
camera->setProjectionType(PT_ORTHOGRAPHIC);
~~~~~~~~~~~~~

![Model drawn using the ortographic camera](OrtographicCamera.png)  

# Field of view {#cameras_b}
This is a parameter only relevant for perspective cameras. It controls the horizontal angle of vision - increasing it means the camera essentially has a wider lens. Modify it by calling @ref bs::CCamera::setHorzFOV "CCamera::setHorzFOV()".

Example of setting the FOV to 90 degrees:
~~~~~~~~~~~~~{.cpp}
camera->setHorzFOV(Degree(90));
~~~~~~~~~~~~~

Vertical FOV is automatically determined from the aspect ratio.

# Aspect ratio {#cameras_c}
Aspect ratio allows you to control the ratio of the camera's width and height. It can be set by calling @ref bs::CCamera::setAspectRatio "CCamera::setAspectRatio()". 

Normally you want to set it to the ratio of the render target's width and height, as shown below.

~~~~~~~~~~~~~{.cpp}
SPtr<RenderWindow> primaryWindow = gApplication().getPrimaryWindow();
auto& windowProps = primaryWindow->getProperties();

float aspectRatio = windowProps.getWidth() / (float)windowProps.getHeight();
camera->setAspectRatio(aspectRatio);
~~~~~~~~~~~~~

But you are also allowed to freely adjust it for different effects.

# Ortographic size {#cameras_d}
This parameter has a similar purpose as field of view, but is used for ortographic cameras instead. It controls the width and height (in world space) of the area covered by the camera. It is set by calling @ref bs::CCamera::setOrthoWindow "CCamera::setOrthoWindow()".

Set up ortographic view that shows 500x500 units of space, along the current orientation axis.
~~~~~~~~~~~~~{.cpp}
camera->setOrthoWindow(500, 500);
~~~~~~~~~~~~~

For example, if you are making a 2D game, your world units are most likely pixels. In which case you will want to set the ortographic size to the same size as the render target size (resolution):

~~~~~~~~~~~~~{.cpp}
SPtr<RenderWindow> primaryWindow = gApplication().getPrimaryWindow();
auto& windowProps = primaryWindow->getProperties();

camera->setOrthoWindow(windowProps.getWidth(), windowProps.getHeight());
~~~~~~~~~~~~~

# Multi-sample anti-aliasing {#cameras_e}
To achieve higher rendering quality you may enable MSAA per camera. This will ensure that each rendered pixel receives multiple samples which are then averaged to produce the final pixel color. This process reduced aliasing on pixels that have discontinuities, like pixels that are on a boundary between two surfaces. This reduces what are often called "jaggies". 

MSAA can be enabled by providing a values of 1, 2, 4 or 8 to @ref bs::CCamera::setMSAACount() "CCamera::setMSAACount()". The value determines number of samples per pixel, where 1 means no MSAA. MSAA can be quite performance heavy, and larger MSAA values require proportionally more performance. 

~~~~~~~~~~~~~{.cpp}
// Enable 4X MSAA
camera->setMSAACount(4);
~~~~~~~~~~~~~

![MSAA comparison](MSAA.png)  