---
title: Renderer extensions
---

Renderer is a system that processes all renderable objects in the scene, renders them, applies lighting and shadows, renders overlay elements such as GUI and applies post processing effects. It is the system that determines how your game looks (together with custom materials you might specify). In bs::f the renderer is implemented as a plugin, so you may create your own and fully customize the look of your game. bs::f also comes with a default renderer called "RenderBeast".

In this chapter we'll show how to create extensions to the renderer, which are primarily useful when adding systems that need to perform rendering, but you do not wish to completely replace existing renderer functionality, but rather add to it. Such systems might perform particle effect rendering, GUI overlays, custom 2D rendering and similar.

# Creating extensions

To create a renderer extensions implement your own class deriving from @bs::RendererExtension. Note it is a core thread class, as all rendering is executed on the core thread.

~~~~~~~~~~~~~{.cpp}
class MyRendererExtension : public RendererExtension
{
public:
	MyRendererExtension();
}
~~~~~~~~~~~~~

Your implementation needs to pass two parameters to the base **RendererExtension** class. The first parameter is of type @bs::RenderLocation which determines at which point during rendering will your extension be triggered. It can be any of the following values, which are executed in the order specified:
 - @bs::RenderLocation::PreBasePass - Triggered before any scene objects are rendered. The renderer guarantees the render targets used for rendering scene objects will be bound (e.g. GBuffer).
 - @bs::RenderLocation::PostBasePass - Triggered after scene objects are rendered, but before they are lit. The renderer guarantees the render targets used for rendering scene objects will be bound (e.g. GBuffer).
 - @bs::RenderLocation::PostLightPass - Triggered after all scene objects have been rendered and their final information has been written to the final scene color buffer, without any post-processing. The renderer guarantees the final scene color render target will be bound.
 - @bs::RenderLocation::Overlay - Triggered after all scene objects have been rendered and their final information has been written to the final scene color buffer, with post-processing. The renderer guarantees the final scene color render target will be bound.
 
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
 - @bs::RendererExtension::check() - Called every frame for every camera in the scene. The methods accepts a camera as a parameter and returns @bs::RendererExtensionRequest that signals the renderer if and under which circumstances should **RendererExtension::render()** be called.
 - @bs::RendererExtension::render() - Called every frame for every camera that the **ct::RendererExtension::check()** method returned true for. This is the method where you place the bulk of extension code and perform actual rendering. The rendering is performed using the low level rendering API as described previously. Note that this is the only method in the extension that you should be rendering from.
 
~~~~~~~~~~~~~{.cpp}
// Renderer extension with check() and render() methods
class MyRendererExtension : public RendererExtension
{
public:
	MyRendererExtension()
		: RendererExtension(RenderLocation::PostLightPass, 0)
	{ }
	
	bool check(const Camera& camera, bool& canSkip) override
	{
		// Render on any camera
		return true;
	}

	void render(const Camera& camera, const RendererViewContext& viewContext)) override
	{
		RenderAPI& rapi = RenderAPI::instance();
		
		// bind pipeline state, vertex/index buffers, etc.
		rapi.drawIndexed(0, numIndices, 0, numVertices);
	}
}
~~~~~~~~~~~~~

@bs::RendererViewContext is an additional parameter provided to the **render()** method, that gives you insight in the current state of the renderer. In particular it contains the current render target set by the renderer. If you ever change the render target inside the extension (through a call to **RenderAPI::setRenderTarget()**) you must ensure to restore the original render target before exiting the method.

# Registering an extension
Once extension is implemented you need to register it with the renderer by calling @bs::RendererExtension::create<T>, where the template parameter is the type of your extension. You must also provide initialization data that will be passed to the extension - this can be null.

Note that while extensions are executed on the core thread, they are started from the sim (main) thread.

~~~~~~~~~~~~~{.cpp}
// Calling from sim thread
SPtr<ct::MyRendererExtension> rendererExt = RendererExtension::create<ct::MyRendererExtension>(nullptr);
~~~~~~~~~~~~~

# Initialization
When implementing your extension you may optionally override the @bs::RendererExtension::initialize() method. This method will be called on the core thread once the extension is created.

Note that you shouldn't use the constructor for initialization, since the constructor will trigger on the simulation thread, which is not a valid thread for rendering operations.

**RendererExtension::initialize()** additionally also accepts the data passed to the **RendererExtension::create<T>()** method. The data is passed as **Any** type, meaning you can bind whatever you wish to it. Normally it is some kind of a *struct* containing the necessary initialization parameters.

~~~~~~~~~~~~~{.cpp}
struct MyInitData
{
	int a;
	float b;
	SPtr<ct::Texture> c;
}

// Core thread
class MyRendererExtension : public RendererExtension
{
public:
	// ... other extension code
	
	void initialize(const Any& data) override
	{
		const MyInitData& initData = any_cast_ref<MyInitData>(data);
		// initialize whatever is required
	}
}

// Sim thread
HTexture tex = ...;

MyInitData initData;
initData.a = 5;
initData.b = 30.0f;
initData.c = tex->getCore(); // Get version of texture usable on core thread

SPtr<ct::MyRendererExtension> rendererExt = RendererExtension::create<ct::MyRendererExtension>(initData);
~~~~~~~~~~~~~

# Destruction
Similar to how you shouldn't use the constructor for initializing the extension, neither should you use the destructor for destruction. Instead if you need to perform cleanup before the extension is destroyed, override the @bs::RendererExtension::destroy() method.

~~~~~~~~~~~~~{.cpp}
class MyRendererExtension : public RendererExtension
{
public:
	// ... other extension code
	
	void destroy() override
	{
		// clean up
	}
}
~~~~~~~~~~~~~

# Communicating with the extension
If you need further communication with your extension from the sim thread, you should use the command queue as described in the [core thread](../Low_Level_rendering/coreThread) manual. If not using the command queue then you must ensure to use some other form of thread primitives to ensure safe communication between the two threads.

~~~~~~~~~~~~~{.cpp}
class MyRendererExtension : public RendererExtension
{
public:
	void myCustomUpdateMethod(float newValue)
	{
		// respond to change in value
	}
}

// Sim thread
ct::MyRendererExtension myExtension = ...;
float newValue = 15.0f;

auto executeOnCore = [&]()
{
	myExtension->myCustomUpdateMethod(newValue);
};

gCoreThread().queueCommand(executeOnCore);
~~~~~~~~~~~~~
