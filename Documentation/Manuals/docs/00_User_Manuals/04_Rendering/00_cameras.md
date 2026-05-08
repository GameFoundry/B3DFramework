---
title: Cameras
---

Cameras represent the user's view into the scene, and any graphical application will require at least one camera for the user to be able to see anything.

They have parameters like position and orientation which define what part of the scene will the user see. Additionally, their parameters like field of view, projection mode and aspect ratio define how is the application scene transformed into the 2D output visible to the user.

Finally, everything that the camera sees is output to what we call a render target. Render targets can be windows, like the one that was created when the application was started, or an off-screen surface, as we'll explain later.

Cameras are represented by the @b3d::Camera component, and they can be created as any other component.

~~~~~~~~~~~~~{.cpp}
HSceneObject cameraSceneObject = SceneObject::Create("Camera");
HCamera camera = cameraSceneObject->AddComponent<Camera>();
~~~~~~~~~~~~~

Before the camera can render anything, you need to assign the render target to which the camera will output its contents to. Lets create a camera that renders to the primary render window. The primary application window can be retrieved through @b3d::Application::GetPrimaryWindow.

To assign the window, retrieve the @b3d::Viewport object from the camera, and set its target using the @b3d::Viewport::SetTarget method.

~~~~~~~~~~~~~{.cpp}
TShared<RenderWindow> primaryWindow = GetApplication().GetPrimaryWindow();
camera->GetViewport()->SetTarget(primaryWindow);
~~~~~~~~~~~~~

> **Application** is a singleton and its instance can be accessed through @b3d::Application::Instance(), or the helper method @b3d::GetApplication(). All other singletons in the framework follow the same design.

Or alternatively, you can just mark the camera as 'main', which will render to the default render target (in this case, the primary window).

~~~~~~~~~~~~~{.cpp}
camera->SetMain(true);
~~~~~~~~~~~~~

Once the camera has been created we can move and orient it using the **SceneObject** transform, as explained earlier. For example:
~~~~~~~~~~~~~{.cpp}
// Move camera to 10 meters height, and 50 meters away from the center
cameraSceneObject->SetPosition(Vector3(0.0f, 10.0f, 50.0f));

// Orient the camera so it is looking at the center
cameraSceneObject->LookAt(Vector3(0.0f, 0.0f, 0.0f));
~~~~~~~~~~~~~

Once set up, any rendered objects in the camera's view will be displayed on the selected render target, which is in this case the primary application window.

You can also customize a variety of parameters that control how will the camera render the objects.

# Projection type
All cameras can be in two projection modes: *Perspective* and *Orthographic*. They can be changed by calling @b3d::Camera::SetProjectionType.

## Perspective cameras
This mode simulates human vision, where objects farther away appear smaller. This is what you will need for most 3D applications.

~~~~~~~~~~~~~{.cpp}
camera->SetProjectionType(PT_PERSPECTIVE);
~~~~~~~~~~~~~

![Model drawn using the perspective camera](../../Images/PerspectiveCamera.png)

## Orthographic
Renders the image without perspective distortion, ensuring objects remain the same size regardless of the distance from camera, essentially "flattening" the image. Useful for 2D applications.

~~~~~~~~~~~~~{.cpp}
camera->SetProjectionType(PT_ORTHOGRAPHIC);
~~~~~~~~~~~~~

![Model drawn using the orthographic camera](../../Images/OrtographicCamera.png)

# Field of view
This is a parameter only relevant for perspective cameras. It controls the horizontal angle of vision - increasing it means the camera essentially has a wider lens. Modify it by calling @b3d::Camera::SetHorizontalFOV.

Example of setting the FOV to 90 degrees:
~~~~~~~~~~~~~{.cpp}
camera->SetHorizontalFOV(Degree(90));
~~~~~~~~~~~~~

Vertical FOV is automatically determined from the aspect ratio.

# Aspect ratio
Aspect ratio allows you to control the ratio of the camera's width and height. It can be set by calling @b3d::Camera::SetAspectRatio.

Normally you want to set it to the ratio of the render target's width and height, as shown below.

~~~~~~~~~~~~~{.cpp}
TShared<RenderWindow> primaryWindow = GetApplication().GetPrimaryWindow();
const auto& windowProperties = primaryWindow->GetRenderWindowProperties();

float aspectRatio = windowProperties.Width / (float)windowProperties.Height;
camera->SetAspectRatio(aspectRatio);
~~~~~~~~~~~~~

But you are also allowed to freely adjust it for different effects.

# Orthographic size
This parameter has a similar purpose as field of view, but is used for orthographic cameras instead. It controls the width and height (in world space) of the area covered by the camera. It is set by calling @b3d::Camera::SetOrthographicSize.

Set up orthographic view that shows 500x500 units of space, along the current orientation axis.
~~~~~~~~~~~~~{.cpp}
camera->SetOrthographicSize(500.0f, 500.0f);
~~~~~~~~~~~~~

For example, if you are making a 2D game, your world units are most likely pixels. In which case you will want to set the orthographic size to the same size as the render target size (resolution):

~~~~~~~~~~~~~{.cpp}
TShared<RenderWindow> primaryWindow = GetApplication().GetPrimaryWindow();
const auto& windowProps = primaryWindow->GetRenderWindowProperties();

camera->SetOrthographicSize((float)windowProps.Width, (float)windowProps.Height);
~~~~~~~~~~~~~

Alternatively, you can set just the orthographic height or width, and the other dimension will be calculated from the aspect ratio:

~~~~~~~~~~~~~{.cpp}
// Set height, width calculated from aspect ratio
camera->SetOrthographicHeight(600.0f);

// Set width, height calculated from aspect ratio
camera->SetOrthographicWidth(800.0f);
~~~~~~~~~~~~~

# Clipping planes
Cameras have near and far clipping planes that determine the range of distances that will be rendered. Objects closer than the near plane or farther than the far plane will not be visible.

~~~~~~~~~~~~~{.cpp}
// Set near clipping plane (default is 0.05)
camera->SetNearClipDistance(0.1f);

// Set far clipping plane (default is 500.0)
camera->SetFarClipDistance(1000.0f);
~~~~~~~~~~~~~

**Important considerations:**
- Smaller near clip values decrease depth precision at larger distances
- Larger far clip values decrease depth precision at smaller distances
- Keep the ratio between near and far planes as small as possible for best depth precision

# Multi-sample anti-aliasing
To achieve higher rendering quality you may enable MSAA per camera. This will ensure that each rendered pixel receives multiple samples which are then averaged to produce the final pixel color. This process reduced aliasing on pixels that have discontinuities, like pixels that are on a boundary between two surfaces. This reduces what are often called "jaggies".

MSAA can be enabled by providing a values of 1, 2, 4 or 8 to @b3d::Camera::SetSampleCount(). The value determines number of samples per pixel, where 1 means no MSAA. MSAA can be quite performance heavy, and larger MSAA values require proportionally more performance.

~~~~~~~~~~~~~{.cpp}
// Enable 4X MSAA
camera->SetSampleCount(4);
~~~~~~~~~~~~~

![MSAA comparison](../../Images/MSAA.png)

# Render priority
When multiple cameras render to the same target, you can control the order in which they render using priority:

~~~~~~~~~~~~~{.cpp}
// Higher priority cameras render first
camera->SetPriority(10);
~~~~~~~~~~~~~

# Layer masks
Cameras can selectively render only objects on specific layers using a layer bitfield:

~~~~~~~~~~~~~{.cpp}
// Render only objects on layers 0, 1, and 3
camera->SetLayers((1 << 0) | (1 << 1) | (1 << 3));

// Render all layers (default)
camera->SetLayers(0xFFFFFFFFFFFFFFFF);
~~~~~~~~~~~~~

# Camera flags
Cameras support various flags that control their behavior:

~~~~~~~~~~~~~{.cpp}
// On-demand rendering: camera only renders when requested
camera->SetFlags(CameraFlag::OnDemand);

// Request a redraw for an on-demand camera
camera->NotifyNeedsRedraw();
~~~~~~~~~~~~~

On-demand rendering is useful for UI overlays that only update on user interaction.

# Coordinate transformations
Cameras provide many methods for converting between different coordinate spaces:

~~~~~~~~~~~~~{.cpp}
// World to screen
Vector3 worldPoint(10.0f, 5.0f, 0.0f);
Vector2I screenPoint = camera->WorldToScreenPoint(worldPoint);

// Screen to world
Vector3 worldPoint = camera->ScreenToWorldPoint(screenPoint, 10.0f); // 10.0f is depth

// Screen to ray (useful for picking)
Ray ray = camera->ScreenPointToRay(screenPoint);

// World to view space
Vector3 viewPoint = camera->WorldToViewPoint(worldPoint);

// Normalized device coordinates (NDC)
Vector2 ndcPoint = camera->WorldToNDCPoint(worldPoint);
Vector3 worldFromNDC = camera->NDCToWorldPoint(ndcPoint, 10.0f);
~~~~~~~~~~~~~

# Custom projection and view matrices
For advanced use cases, you can provide custom projection and view matrices:

~~~~~~~~~~~~~{.cpp}
// Custom projection matrix (e.g., for VR or oblique projections)
Matrix4 customProj = ...; // Calculate custom projection
camera->SetCustomProjectionMatrix(true, customProj);

// Custom view matrix (e.g., for portal rendering)
Matrix4 customView = ...; // Calculate custom view
camera->SetCustomViewMatrix(true, customView);

// Disable custom matrices to return to automatic calculation
camera->SetCustomProjectionMatrix(false);
camera->SetCustomViewMatrix(false);
~~~~~~~~~~~~~

# Accessing camera matrices
You can retrieve the camera's view and projection matrices for use in custom rendering:

~~~~~~~~~~~~~{.cpp}
// Get view matrix (camera position and orientation)
const Matrix4& viewMatrix = camera->GetViewMatrix();

// Get projection matrix (how 3D points project to 2D)
const Matrix4& projMatrix = camera->GetProjectionMatrix();

// Get unadjusted projection (standard right-hand, depth [-1,1])
const Matrix4& standardProj = camera->GetUnadjustedProjectionMatrix();
~~~~~~~~~~~~~

# Frustum and culling
The camera's frustum defines the visible volume. You can access it for custom culling:

~~~~~~~~~~~~~{.cpp}
// Get frustum in local space
const ConvexVolume& frustum = camera->GetFrustum();

// Get frustum in world space
ConvexVolume worldFrustum = camera->GetWorldFrustum();

// Get bounding box of the frustum
const AABox& frustumBounds = camera->GetBoundingBox();
~~~~~~~~~~~~~

You can also manually set frustum extents for asymmetric projections:

~~~~~~~~~~~~~{.cpp}
// Set custom frustum extents (for oblique projections, off-center cameras, etc.)
camera->SetFrustumExtents(-1.0f, 1.5f, -1.0f, 1.0f); // left, right, top, bottom

// Reset to automatic calculation
camera->ResetFrustumExtents();

// Query current extents
float left, right, top, bottom;
camera->GetFrustumExtents(left, right, top, bottom);
~~~~~~~~~~~~~

# Render settings
Each camera can have custom render settings that control post-processing and visual effects:

~~~~~~~~~~~~~{.cpp}
// Create custom render settings
TShared<RenderSettings> settings = B3DMakeShared<RenderSettings>();
// ... configure settings (covered in rendering manuals)

camera->SetRenderSettings(settings);
~~~~~~~~~~~~~

See the [Render Settings](../12_Advanced_Rendering/00_renderSettings.md) manual for details.