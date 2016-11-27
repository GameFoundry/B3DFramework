Cameras 						{#cameras}
===============

Cameras represent the user's view into the scene, and any graphical application will require at least one camera for the user to be able to see anything. 

They have parameters like position and orientation which define what part of the scene will the user see. Additionally, their parameters like field of view, projection mode and aspect ratio define how is the application scene transformed into the 2D output visible to the user. 

Finally, everything that the camera sees is output to what we call a render target. Render targets can be windows, which we explained in the previous chapter, or off-screen surfaces, as we'll explain later.

Cameras are represented by the @ref bs::CCamera "Camera" component, and they can be created as any other component. At minimum their constructor requires a render target onto which they will output their contents.

Lets create a camera that renders to the primary render window:

~~~~~~~~~~~~~{.cpp}
SPtr<RenderWindow> primaryWindow = gApplication().getPrimaryWindow();

HSceneObject cameraSO = SceneObject::create("Camera");
HCamera camera = cameraSO->addComponent<CCamera>(primaryWindow);
~~~~~~~~~~~~~

Once the camera has been created we can move and orient it using the **SceneObject** transform, as explained earlier. For example:
~~~~~~~~~~~~~{.cpp}
// Move camera to 100 units high, and 500 units away from the center
cameraSO->setPosition(Vector3(0f, 100f, 500f));

// Orient the camera so it is looking at the center
cameraSO->lookAt(Vector3(0f, 0f, 0f));
~~~~~~~~~~~~~

Once set up, any rendered objects in the camera's view will be displayed on the selected render target, which is in this case the primary application window.

However you might want to customize camera's parameters to get the exact feel you need. Lets cover some of the available parameters.

# Projection type
All cameras can be in two projection modes: *Perspective* and *Ortographic*. They can be changed by calling @ref bs::CCamera::setProjectionType "CCamera::setProjectionType()".

## Perspective cameras
This mode simulates human vision, where objects farther away appear smaller. This is what you will need for most 3D applications.

~~~~~~~~~~~~~{.cpp}
camera->setProjectionType(PT_PERSPECTIVE);
~~~~~~~~~~~~~

![Dragon drawn using the perspective camera](PerspectiveCamera.png)  

## Ortographic
Renders the image without perspective distortion, ensuring objects remain the same size regardless of the distance from camera, essentially "flattening" the image. Useful for 2D applications.

~~~~~~~~~~~~~{.cpp}
camera->setProjectionType(PT_ORTHOGRAPHIC);
~~~~~~~~~~~~~

![Dragon drawn using the ortographic camera](OrtographicCamera.png)  

# Field of view
This is a parameter only relevant for perspective cameras. It controls the horizontal angle of vision - increasing it means the camera essentially has a wider lens. Modify it by calling @ref bs::CCamera::setHorzFOV "CCamera::setHorzFOV()".

Example of setting the FOV to 90 degrees:
~~~~~~~~~~~~~{.cpp}
camera->setHorzFOV(Degree(90));
~~~~~~~~~~~~~

Vertical FOV is automatically determined from the aspect ratio.

# Aspect ratio
Aspect ratio allows you to control the ratio of the camera's width and height. It can be set by calling @ref bs::CCamera::setAspectRatio "CCamera::setAspectRatio()". 

Normally you want to set it to the ratio of the render target's width and height, as shown below.

~~~~~~~~~~~~~{.cpp}
SPtr<RenderWindow> primaryWindow = gApplication().getPrimaryWindow();
auto& windowProps = newWindow->getProperties();

float aspectRatio = windowProps.getWidth() / (float)windowProps.getHeight();
camera->setAspectRatio(aspectRatio);
~~~~~~~~~~~~~

But you are also allowed to freely adjust it for different effects.

# Ortographic size
This parameter has a similar purpose as field of view, but is used for ortographic cameras instead. It controls the width and height (in world space) of the area covered by the camera. It is set by calling @ref bs::CCamera::setOrthoWindow "CCamera::setOrthoWindow()".

Set up ortographic view that shows 500x500 units of space, along the current orientation axis.
~~~~~~~~~~~~~{.cpp}
camera->setOrthoWindow(500, 500);
~~~~~~~~~~~~~

For example, if you are making a 2D game, your world units are most likely pixels. In which case you will want to set the ortographic size to the same size as the render target size (resolution):

~~~~~~~~~~~~~{.cpp}
SPtr<RenderWindow> primaryWindow = gApplication().getPrimaryWindow();
auto& windowProps = newWindow->getProperties();

camera->setOrthoWindow(windowProps.getWidth(), windowProps.getHeight());
~~~~~~~~~~~~~