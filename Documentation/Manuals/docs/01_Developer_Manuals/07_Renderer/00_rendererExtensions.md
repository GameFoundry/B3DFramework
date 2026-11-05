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
 - @b3d::RenderLocation::Prepare - Triggered before the view renders any scene objects. Useful for preparing data or resources before the main render passes. There are no attachments at this location.
 - @b3d::RenderLocation::PreBasePass - Triggered after the G-buffer is cleared, before scene objects are rendered into it. The attachments are the G-buffer and scene depth.
 - @b3d::RenderLocation::PostBasePass - Triggered after scene objects are rendered into the G-buffer, but before they are lit. The attachments are the G-buffer and scene depth, whose depth is read-only.
 - @b3d::RenderLocation::PostLightPass - Triggered after all scene objects have been rendered and lit into scene color, without any post-processing. The attachments are scene color and scene depth.
 - @b3d::RenderLocation::Overlay - Triggered after post-processing, once the final image of the view is in its render target. The attachment is the render target.

The second parameter is the priority, a signed number. It determines in what order will renderer extensions attached to the same render location be executed. Those with higher priority will execute before those with lower priority.

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
 - @b3d::RendererExtension::Render() - Called every frame for every camera that the **RendererExtension::Check()** method returned `RendererExtensionRequest::ForceRender` for. This is the method where you place the bulk of extension code. It is called while the renderer builds the frame graph of the view, and adds the extension's passes to that graph. The passes record their commands with the low level rendering API as described previously, later, when the graph executes. Note that this is the only method in the extension that you should be rendering from.
 
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
		// Draw into the attachments of the location, keeping their contents
		render::FrameGraphRasterPass pass;
		viewContext.SetAttachments(pass);

		// Every parameter set the draws bind is added to the pass. Resources that are not resources of the graph may only be read.
		TShared<render::GpuParameterSet> parameters = ...;
		pass.AddParameterSet(parameters);

		// Records the pass when the graph executes, with the render pass already open
		viewContext.Graph.AddRasterPass("MyExtension", std::move(pass), [parameters, viewport = viewContext.Viewport](render::FrameGraphPassContext& context)
		{
			render::GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
			commandBuffer.SetViewport(viewport);

			// bind pipeline state, vertex/index buffers, etc.
			commandBuffer.SetGpuParameterSet(parameters);
			commandBuffer.DrawIndexed(0, numIndices, 0, numVertices);
		});
	}
}
~~~~~~~~~~~~~

@b3d::RendererViewContext is an additional parameter provided to the **Render()** method. It holds the frame graph of the view (**RendererViewContext::Graph**) and the attachments of the extension's location. **RendererViewContext::SetAttachments()** attaches them to a raster pass, loading their contents, and **RendererViewContext::Viewport** is the area of the attachments the view covers. The graph sets no viewport, so every pass sets one before it draws.

An extension may add any number of passes, including compute passes and passes that render into textures of its own (see the [frame graph](../Renderer/frameGraph) manual). Passes must not open or close render passes or issue barriers; the graph does both. Since the passes record after **Render()** returns, and after every other view of the render target has added its passes, the pass functions must capture the state they need instead of reading the extension's members when they run.

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
