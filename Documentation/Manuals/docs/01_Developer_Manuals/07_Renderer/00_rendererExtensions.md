---
title: Renderer extensions
---

Renderer is a system that processes all renderable objects in the scene, renders them, applies lighting and shadows, renders overlay elements such as GUI and applies post processing effects. It is the system that determines how your game looks (together with custom materials you might specify). In the framework the renderer is implemented as a plugin, so you may create your own and fully customize the look of your game. The framework also comes with a default renderer called "RenderBeast".

In this chapter we'll show how to create extensions to the renderer, which are primarily useful when adding systems that need to perform rendering, but you do not wish to completely replace existing renderer functionality, but rather add to it. Such systems might perform particle effect rendering, GUI overlays, custom 2D rendering and similar.

# Creating extensions

To create a renderer extensions implement your own class deriving from @b3d::RendererExtension. Note it is a render thread class, as all rendering is executed on the render thread.

~~~~~~~~~~~~~{.cpp}
class MyRendererExtension : public RendererExtension
{
public:
	MyRendererExtension();
}
~~~~~~~~~~~~~

Your implementation needs to pass two parameters to the base **RendererExtension** class. The first parameter is of type @b3d::RenderLocation which determines at which point during rendering will your extension be triggered. It can be any of the following values, which are executed in the order specified:
 - @b3d::RenderLocation::Prepare - Triggered before any rendering begins. Useful for preparing data or resources before the main render passes.
 - @b3d::RenderLocation::PreBasePass - Triggered before any scene objects are rendered. The renderer guarantees the render targets used for rendering scene objects will be bound (e.g. GBuffer).
 - @b3d::RenderLocation::PostBasePass - Triggered after scene objects are rendered, but before they are lit. The renderer guarantees the render targets used for rendering scene objects will be bound (e.g. GBuffer).
 - @b3d::RenderLocation::PostLightPass - Triggered after all scene objects have been rendered and their final information has been written to the final scene color buffer, without any post-processing. The renderer guarantees the final scene color render target will be bound.
 - @b3d::RenderLocation::Overlay - Triggered after all scene objects have been rendered and their final information has been written to the final scene color buffer, with post-processing. The renderer guarantees the final scene color render target will be bound.
 
The second parameter is the priority. It determines in what order will renderer extensions attached to the same render location be executed. Those with higher priority will execute before those with lower priority.

~~~~~~~~~~~~~{.cpp}
// Renderer extension with necessary constructor arguments
class MyRendererExtension : public RendererExtension
{
public:
	MyRendererExtension()
		: RendererExtension(RenderLocation::PostLightPass, 0)
	{ }
}
~~~~~~~~~~~~~
 
Finally the implementation needs to implement the following methods:
 - @b3d::RendererExtension::Check() - Called every frame for every camera in the scene. The method accepts a camera as a parameter and returns @b3d::RendererExtensionRequest that signals the renderer if and under which circumstances should **RendererExtension::Render()** be called.
 - @b3d::RendererExtension::Render() - Called every frame for every camera that the **RendererExtension::Check()** method returned `RendererExtensionRequest::ForceRender` for. This is the method where you place the bulk of extension code and perform actual rendering. The rendering is performed using the low level rendering API as described previously. Note that this is the only method in the extension that you should be rendering from.
 
~~~~~~~~~~~~~{.cpp}
// Renderer extension with Check() and Render() methods
class MyRendererExtension : public RendererExtension
{
public:
	MyRendererExtension()
		: RendererExtension(RenderLocation::PostLightPass, 0)
	{ }
	
	RendererExtensionRequest Check(const render::Camera& camera) override
	{
		// Render on any camera
		return RendererExtensionRequest::ForceRender;
	}

	void Render(const render::Camera& camera, const render::RendererViewContext& viewContext) override
	{
		GpuCommandBuffer& commandBuffer = ...;
		
		// bind pipeline state, vertex/index buffers, etc.
		commandBuffer.DrawIndexed(0, numIndices, 0, numVertices);
	}
}
~~~~~~~~~~~~~

@b3d::RendererViewContext is an additional parameter provided to the **Render()** method, that gives you insight in the current state of the renderer. In particular it contains the current render target set by the renderer. If you ever change the render target inside the extension (through a call to **GpuCommandBuffer::BeginRenderPass()**) you must ensure to restore the original render target before exiting the method.

# Registering an extension
Once extension is implemented you need to register it with the renderer by calling @b3d::RendererExtension::Create<T>, where the template parameter is the type of your extension. You must also provide initialization data that will be passed to the extension - this can be null.

Note that while extensions are executed on the render thread, they are started from the main thread.

~~~~~~~~~~~~~{.cpp}
// Calling from main thread
TShared<render::MyRendererExtension> rendererExt = RendererExtension::Create<render::MyRendererExtension>(nullptr);
~~~~~~~~~~~~~

# Initialization
When implementing your extension you may optionally override the @b3d::RendererExtension::Initialize() method. This method will be called on the render thread once the extension is created.

Note that you shouldn't use the constructor for initialization, since the constructor will trigger on the main thread, which is not a valid thread for rendering operations.

**RendererExtension::Initialize()** additionally also accepts the data passed to the **RendererExtension::Create<T>()** method. The data is passed as **Any** type, meaning you can bind whatever you wish to it. Normally it is some kind of a *struct* containing the necessary initialization parameters.

~~~~~~~~~~~~~{.cpp}
struct MyInitData
{
	int a;
	float b;
	TShared<render::Texture> c;
}

// Render thread
class MyRendererExtension : public RendererExtension
{
public:
	// ... other extension code
	
	void Initialize(const Any& data) override
	{
		const MyInitData& initData = AnyCastRef<MyInitData>(data);
		// initialize whatever is required
	}
}

// Main thread
HTexture tex = ...;

MyInitData initData;
initData.a = 5;
initData.b = 30.0f;
initData.c = B3DGetRenderProxy(tex); // Get version of texture usable on render thread

TShared<render::MyRendererExtension> rendererExt = RendererExtension::Create<render::MyRendererExtension>(initData);
~~~~~~~~~~~~~

# Destruction
Similar to how you shouldn't use the constructor for initializing the extension, neither should you use the destructor for destruction. Instead if you need to perform cleanup before the extension is destroyed, override the @b3d::RendererExtension::Destroy() method.

~~~~~~~~~~~~~{.cpp}
class MyRendererExtension : public RendererExtension
{
public:
	// ... other extension code
	
	void Destroy() override
	{
		// clean up
	}
}
~~~~~~~~~~~~~

# Communicating with the extension
If you need further communication with your extension from the main thread, you should use the command queue as described in the [render thread](../Low_Level_rendering/renderThread) manual. If not using the command queue then you must ensure to use some other form of thread primitives to ensure safe communication between the two threads.

~~~~~~~~~~~~~{.cpp}
class MyRendererExtension : public RendererExtension
{
public:
	void MyCustomUpdateMethod(float newValue)
	{
		// respond to change in value
	}
}

// Main thread
render::MyRendererExtension myExtension = ...;
float newValue = 15.0f;

auto fnExecuteOnRender = [myExtension, newValue]()
{
	myExtension->MyCustomUpdateMethod(newValue);
};

GetRenderThread().PostCommand(std::move(fnExecuteOnRender));
~~~~~~~~~~~~~
