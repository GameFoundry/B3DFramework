---
title: Creating components
---

So far we have talked about using built-in components like @bs::CCamera and @bs::CRenderable, but another major way you'll be using components is to create your own. Components serve as the main place to put your gameplay logic in, and this is where you'll be adding a majority of your custom code when creating a game.

# Creation
To create your own component simply implement the @bs::Component interface by deriving from it. Component's constructor must always accept a handle to a **SceneObject**, which represents the scene object the component is part of.

~~~~~~~~~~~~~{.cpp}
// A simple component that does nothing
class CCameraFlyer : public Component
{
public:
	CCameraFlyer(const HSceneObject& parent)
		:Component(parent)
	{}
};
~~~~~~~~~~~~~

# Run-time type information (RTTI)
It's important your component provides some meta-data about itself. This allows features such as dynamic casting and serialization to work properly. This information is provided by the run-time type information (RTTI) system. We'll talk about run-time type information in detail in the following manual, and for now we just provide a minimal working example.

RTTI requires you to create a specialization of the @bs::RTTIType<Type, BaseType, MyRTTIType> interface, which contains information about your class, including its name, id and a helper creation method (at minimum).

~~~~~~~~~~~~~{.cpp}
class CCameraFlyerRTTI : public RTTIType<CCameraFlyer, Component, CCameraFlyerRTTI>
{
public:
	const String& getRTTIName() override { return "CCameraFlyer"; }

	// Unique integer for the type. Use numbers higher than 200000 to avoid conflicts with built-in types
	UINT32 getRTTIId() override { return 200001; }

	SPtr<IReflectable> newRTTIObject() override { return SceneObject::createEmptyComponent<CCameraFlyer>(); }
};
~~~~~~~~~~~~~

The component itself then needs to implement a **getRTTI()** and **getRTTIStatic()** methods which return an instance of the **RTTIType** class you created.

~~~~~~~~~~~~~{.cpp}
class CCameraFlyer : public Component
{
public:
	CCameraFlyer(const HSceneObject& parent)
		:Component(parent)
	{}

	static RTTITypeBase* getRTTIStatic()
	{ return CCameraFlyerRTTI::instance(); }

	RTTITypeBase* getRTTI() const override
	{ return CCameraFlyer::getRTTIStatic(); }
};
~~~~~~~~~~~~~

This is the base template you can use for all components, but in the following chapter we'll explain RTTI in more detail.

# Logic
Each component implementation can override any of the three primary methods for introducing gameplay logic:
 - @bs::Component::onInitialized - Called once when the component is first instantiated. You should use this instead of the constructor for initialization.
 - @bs::Component::update - Called every frame while the game is running and the component is enabled.
 - @bs::Component::fixedUpdate - Similar to `update()` except it gets called at a fixed time interval (e.g. 60 times per second, instead of every frame). Normally you want to use this method for physics-related functionality, which requires fixed time increments in order to ensure stability of calculations.
 - @bs::Component::onDestroyed - Called just before the component is destroyed. Use this instead of the destructor for cleanup.
		
Here is a simple implementation of a component using a few of these methods to implement basic camera movement.
~~~~~~~~~~~~~{.cpp}
// A simple component that moves a Camera component attached to the same SceneObject
class CCameraFlyer : public Component
{
public:
	// Basic constructor same as above
	CCameraFlyer(const HSceneObject& parent)
		:Component(parent)
	{}
	
private:
	// Called once when component is first added to a scene object
	void onInitialized() override
	{
		// Find the camera component we'll be influencing (assumed to be on the same scene object)
		mCamera = SO()->getComponent<CCamera>();
		
		// Create virtual buttons we'll be using for movement (assuming we registered them previously)
		mMoveForward = VirtualButton("Forward");
		mMoveBack = VirtualButton("Back");
		mMoveLeft = VirtualButton("Left");
		mMoveRight = VirtualButton("Right");
	}
	
	// Called every frame while the component is active
	void update() override
	{
		// Check if any movement or rotation keys are being held
		bool goingForward = GetVirtualInput().isButtonHeld(mMoveForward);
		bool goingBack = GetVirtualInput().isButtonHeld(mMoveBack);
		bool goingLeft = GetVirtualInput().isButtonHeld(mMoveLeft);
		bool goingRight = GetVirtualInput().isButtonHeld(mMoveRight);	
	
		// If the movement button is pressed, determine direction to move in
		Vector3 direction = Vector3::ZERO;
		if (goingForward) direction += SO()->getForward();
		if (goingBack) direction -= SO()->getForward();
		if (goingRight) direction += SO()->getRight();
		if (goingLeft) direction -= SO()->getRight();
		
		// Multiply direction with speed and move in the direction
		float frameDelta = GetTime().getFrameDelta();
		float speed = 10.0f;
		
		Vector3 velocity = direction * speed;
		SO()->move(velocity * frameDelta);
	}
	
	HCamera mCamera; // Camera component that is influenced by this component.

	// Virtual keys we will use for movement
	VirtualButton mMoveForward;
	VirtualButton mMoveBack;
	VirtualButton mMoveLeft;
	VirtualButton mMoveRight;
	
	// RTTI
	static RTTITypeBase* getRTTIStatic()
	{ return CCameraFlyerRTTI::instance(); }

	RTTITypeBase* getRTTI() const override
	{ return CCameraFlyer::getRTTIStatic(); }
};
~~~~~~~~~~~~~

> Use @bs::Component::SO() to access the scene object the component is attached to.
		
> **GetTime()** method provides access to a variety of timing related functionality, and is explained later in the [timing manual](../Utilities/time).
		
# Component handle
You will also likely want to declare a handle you can use to easily access the component, same as **HCamera** or **HRenderable**. This is done by simply creating a *typedef* of a @bs::GameObjectHandle<T>.

~~~~~~~~~~~~~{.cpp}
typedef GameObjectHandle<CCameraFlyer> HCameraFlyer;
~~~~~~~~~~~~~
		
# Using the component
We now have everything ready to use the component. You can create the component as any other by adding it to the scene object.

~~~~~~~~~~~~~{.cpp}
// Create a scene object to add our component to
HSceneObject cameraSO = SceneObject::create("Camera");

// We create a Camera component since our component relies on it
HCamera camera = cameraSO->addComponent<CCamera>(primaryWindow);

// And finally we add our component
HCameraFlyer = cameraSO->addComponent<CCameraFlyer>();
~~~~~~~~~~~~~

# Activating/deactivating a scene object
Any scene object can be temporarily de-activated and reactivated by calling @bs::SceneObject::setActive. When a scene object is deactivated its components will not have **Component::update()** called.

Your component can also be notified at the exact moment when activation/deactivation happens. This way you can perform additional functionality if needed. Override @bs::Component::onEnabled and @bs::Component::onDisabled to get notified every time a component is activated and deactivated, respectively.
		
~~~~~~~~~~~~~{.cpp}
// We're just extending the component we defined above
class CCameraFlyer : public Component
{
	...
	
	void onDisabled() override
	{
		GetDebug().log("Component disabled.");
	}	
	
	void onEnabled() override
	{
		GetDebug().log("Component enabled.");
	}
	
	...
};
~~~~~~~~~~~~~
		
# Getting notified on scene object change
Sometimes you want to get notified when the scene object the component is attached to moves or changes parents. You can do this by overriding the @bs::Component::onTransformChanged method.

~~~~~~~~~~~~~{.cpp}
// We're just extending the component we defined above
class CCameraFlyer : public Component
{
	...
	
	void onTransformChanged(TransformChangedFlags flags) override
	{
		if((flags & TCF_Transform) != 0)
			GetDebug().logDebug("Parent SO moved.");
		
		if((flags & TCF_Parent) != 0)
			GetDebug().logDebug("Scene object parent changed.");
	}	
	
	...
};
~~~~~~~~~~~~~

@bs::TransformChangedFlags parameter will notify you whether the scene object moved, has changed parents, or both.

Note that **Component::onTransformChanged** will never trigger by default. You must first enable it by calling @bs::Component::setNotifyFlags. It accepts the same **TransformChangedFlags** parameter which tells the system in which cases should it trigger **Component::onTransformChanged**.

~~~~~~~~~~~~~{.cpp}
// We're just extending the component we defined above
class CCameraFlyer : public Component
{
	...

	void onInitialized() override
	{
		// Get notified when the scene object moves
		setNotifyFlags(TCF_Transform);
	}	
	
	...
};
~~~~~~~~~~~~~
