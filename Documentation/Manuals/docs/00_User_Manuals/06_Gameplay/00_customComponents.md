---
title: Creating components
---

So far we have talked about using built-in components like @b3d::Camera and @b3d::Renderable, but another major way you'll be using components is to create your own. Components serve as the main place to put your gameplay logic in, and this is where you'll be adding a majority of your custom code when creating a game.

# Creation
To create your own component simply implement the @b3d::Component interface by deriving from it. Component's constructor must always accept a handle to a **SceneObject**, which represents the scene object the component is part of.

~~~~~~~~~~~~~{.cpp}
// A simple component that does nothing
class CameraFlyer : public Component
{
public:
	CameraFlyer(const HSceneObject& parent)
		: Component(parent)
	{}
};
~~~~~~~~~~~~~

# Run-time type information (RTTI)
It's important your component provides some meta-data about itself. This allows features such as dynamic casting and serialization to work properly. This information is provided by the run-time type information (RTTI) system. We'll talk about run-time type information in detail in the following manual, and for now we just provide a minimal working example.

RTTI requires you to create a specialization of the @b3d::RTTIType<Type, BaseType, MyRTTIType> interface, which contains information about your class, including its name, id and a helper creation method (at minimum).

~~~~~~~~~~~~~{.cpp}
class CameraFlyerRTTI : public RTTIType<CameraFlyer, Component, CameraFlyerRTTI>
{
public:
	const String& GetRTTIName() override
	{
		static String name = "CameraFlyer";
		return name;
	}

	// Unique integer for the type. Use numbers higher than 200000 to avoid conflicts with built-in types
	u32 GetRTTIId() override { return 200001; }

	TShared<IReflectable> NewRTTIObject() override
	{
		return SceneObject::CreateEmptyComponent<CameraFlyer>();
	}
};
~~~~~~~~~~~~~

The component itself then needs to implement a **GetRtti()** and **GetRttiStatic()** methods which return an instance of the **RTTIType** class you created.

~~~~~~~~~~~~~{.cpp}
class CameraFlyer : public Component
{
public:
	CameraFlyer(const HSceneObject& parent)
		: Component(parent)
	{}

	static RTTIType* GetRttiStatic()
	{
		return CameraFlyerRTTI::Instance();
	}

	RTTIType* GetRtti() const override
	{
		return CameraFlyer::GetRttiStatic();
	}
};
~~~~~~~~~~~~~

This is the base template you can use for all components, but in the following chapter we'll explain RTTI in more detail.

# Logic
Each component implementation can override any of the primary methods for introducing gameplay logic:
 - @b3d::Component::OnCreated - Called once when the component is first created, before it's initialized. Called regardless of the state the component is in.
 - @b3d::Component::OnBeginPlay - Called once when the component first becomes active (leaves the Stopped state). This includes component creation if the component is immediately active.
 - @b3d::Component::Update - Called every frame while the component is in Running state and enabled.
 - @b3d::Component::FixedUpdate - Similar to `Update()` except it gets called at a fixed time interval (e.g. 60 times per second, instead of every frame). Use this for physics-related functionality that requires stable time increments.
 - @b3d::Component::OnEnabled - Called every time a component leaves the Stopped state and is enabled.
 - @b3d::Component::OnDisabled - Called every time a component enters the Stopped state.
 - @b3d::Component::OnDestroyed - Called just before the component is destroyed. Use this instead of the destructor for cleanup.

## Component states
Components can be in different states that control which events trigger:
- **Running** - Scene manager is sending out all events including per-frame Update().
- **Paused** - Scene manager sends all events except Update().
- **Stopped** - Scene manager only sends OnCreated/OnDestroyed events.

Here is a simple implementation of a component using a few of these methods to implement basic camera movement:

~~~~~~~~~~~~~{.cpp}
// A simple component that moves a Camera component attached to the same SceneObject
class CameraFlyer : public Component
{
public:
	CameraFlyer(const HSceneObject& parent)
		: Component(parent)
	{}

private:
	// Called once when component is first created
	void OnCreated() override
	{
		// Find the camera component we'll be influencing (assumed to be on the same scene object)
		mCamera = SO()->GetComponent<Camera>();

		// Create virtual buttons we'll be using for movement (assuming we registered them previously)
		mMoveForward = VirtualInput::GetOrCreateVirtualButton("Forward");
		mMoveBack = VirtualInput::GetOrCreateVirtualButton("Back");
		mMoveLeft = VirtualInput::GetOrCreateVirtualButton("Left");
		mMoveRight = VirtualInput::GetOrCreateVirtualButton("Right");
	}

	// Called every frame while the component is active and in Running state
	void Update() override
	{
		// Check if any movement keys are being held
		bool goingForward = GetVirtualInput().IsButtonHeld(mMoveForward);
		bool goingBack = GetVirtualInput().IsButtonHeld(mMoveBack);
		bool goingLeft = GetVirtualInput().IsButtonHeld(mMoveLeft);
		bool goingRight = GetVirtualInput().IsButtonHeld(mMoveRight);

		// If the movement button is pressed, determine direction to move in
		Vector3 direction = Vector3::kZero;
		if (goingForward) direction += SceneObject()->GetTransform().GetForward();
		if (goingBack) direction -= SceneObject()->GetTransform().GetForward();
		if (goingRight) direction += SceneObject()->GetTransform().GetRight();
		if (goingLeft) direction -= SceneObject()->GetTransform().GetRight();

		// Multiply direction with speed and move in the direction
		float frameDelta = GetTime().GetFrameDelta();
		float speed = 10.0f;

		Vector3 velocity = direction * speed;
		SceneObject()->Move(velocity * frameDelta);
	}

	HCamera mCamera; // Camera component that is influenced by this component.

	// Virtual keys we will use for movement
	VirtualButton mMoveForward;
	VirtualButton mMoveBack;
	VirtualButton mMoveLeft;
	VirtualButton mMoveRight;

	// RTTI
	static RTTIType* GetRttiStatic()
	{
		return CameraFlyerRTTI::Instance();
	}

	RTTIType* GetRtti() const override
	{
		return CameraFlyer::GetRttiStatic();
	}
};
~~~~~~~~~~~~~

> Use @b3d::Component::SceneObject() to access the scene object the component is attached to.

> **GetTime()** method provides access to a variety of timing related functionality, and is explained later in the [timing manual](../15_Utilities/08_time).

# Component handle
You will also likely want to declare a handle you can use to easily access the component, same as **HCamera** or **HRenderable**. This is done by simply creating a *typedef* of a @b3d::TGameObjectHandle<T>.

~~~~~~~~~~~~~{.cpp}
typedef TGameObjectHandle<CameraFlyer> HCameraFlyer;
~~~~~~~~~~~~~

# Using the component
We now have everything ready to use the component. You can create the component as any other by adding it to the scene object.

~~~~~~~~~~~~~{.cpp}
// Create a scene object to add our component to
HSceneObject cameraObject = SceneObject::Create("Camera");

// We create a Camera component since our component relies on it
HCamera camera = cameraObject->AddComponent<Camera>(primaryWindow);

// And finally we add our component
HCameraFlyer cameraFlyer = cameraObject->AddComponent<CameraFlyer>();
~~~~~~~~~~~~~

# Enabling/disabling components
Any component can be temporarily disabled and re-enabled by calling @b3d::Component::SetEnabled. When a component is disabled, its **Component::Update()** and **Component::FixedUpdate()** methods will not be called, and it will be in the Stopped state.

~~~~~~~~~~~~~{.cpp}
// Disable the camera flyer
cameraFlyer->SetEnabled(false);

// Re-enable it
cameraFlyer->SetEnabled(true);
~~~~~~~~~~~~~

Your component can also be notified at the exact moment when activation/deactivation happens. Override @b3d::Component::OnEnabled and @b3d::Component::OnDisabled to get notified every time a component is enabled and disabled, respectively.

~~~~~~~~~~~~~{.cpp}
// We're just extending the component we defined above
class CameraFlyer : public Component
{
	...

	void OnDisabled() override
	{
		B3D_LOG(Info, LogGeneric, "Component disabled.");
	}

	void OnEnabled() override
	{
		B3D_LOG(Info, LogGeneric, "Component enabled.");
	}

	...
};
~~~~~~~~~~~~~

> Note that disabling a scene object via @b3d::SceneObject::SetActive also disables all its components and will trigger the OnDisabled event.

# Getting notified on scene object change
Sometimes you want to get notified when the scene object the component is attached to moves or changes parents. You can do this by overriding the @b3d::Component::OnTransformChanged method.

~~~~~~~~~~~~~{.cpp}
// We're just extending the component we defined above
class CameraFlyer : public Component
{
	...

	void OnTransformChanged(TransformChangedFlags flags) override
	{
		if ((flags & TCF_Transform) != 0)
			B3D_LOG(Debug, LogGeneric, "Parent SO moved.");

		if ((flags & TCF_Parent) != 0)
			B3D_LOG(Debug, LogGeneric, "Scene object parent changed.");
	}

	...
};
~~~~~~~~~~~~~

@b3d::TransformChangedFlags parameter will notify you whether the scene object moved, has changed parents, or both.

Note that **Component::OnTransformChanged** will never trigger by default. You must first enable it by calling @b3d::Component::SetNotifyFlags. It accepts the same **TransformChangedFlags** parameter which tells the system in which cases should it trigger **Component::OnTransformChanged**.

~~~~~~~~~~~~~{.cpp}
// We're just extending the component we defined above
class CameraFlyer : public Component
{
	...

	void OnCreated() override
	{
		// Get notified when the scene object moves
		SetNotifyFlags(TCF_Transform);
	}

	...
};
~~~~~~~~~~~~~

# Component flags
Components can be configured with special flags that control their behavior. The most important flag is @b3d::ComponentFlag::AlwaysRun:

~~~~~~~~~~~~~{.cpp}
class CameraFlyer : public Component
{
public:
	CameraFlyer(const HSceneObject& parent)
		: Component(parent)
	{
		// This component will always run, even when the scene is paused
		mFlags = ComponentFlag::AlwaysRun;
	}

	...
};
~~~~~~~~~~~~~

When **ComponentFlag::AlwaysRun** is set, the component will remain in the Running state regardless of the global scene manager state. This is useful for components that need to run even when the game is paused, like UI components or debug tools.

> The AlwaysRun flag must be set in the constructor and should not be changed during the component's lifetime.
