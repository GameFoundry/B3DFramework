Creating components						{#customComponents}
===============
[TOC]

So far we have talked about using built-in components like @ref bs::CCamera "Camera" and @ref bs::CRenderable "Renderable", but another major way you'll be using components is to create your own. Components serve as the main place to put your gameplay logic in, and this is where you'll be adding a majority of your custom code when creating a game.

# Creation {#customComponents_a}
To create your own component simply implement the @ref bs::Component "Component" interface by deriving from it. Component's constructor must always accept a handle to a **SceneObject**, which represents the scene object the component is part of.

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

# Logic {#customComponents_b}
Each component implementation can override any of the three primary methods for introducing gameplay logic:
 - @ref bs::Component::onInitialized "Component::onInitialized" - Called once when the component is first instantiated. You should use this instead of the constructor for initialization.
 - @ref bs::Component::update "Component::update" - Called every frame while the game is running and the component is enabled.
 - @ref bs::Component::fixedUpdate "Component::fixedUpdate" - Similar to `update()` except it gets called at a fixed time interval (e.g. 60 times per second, instead of every frame). Normally you want to use this method for physics-related functionality, which requires fixed time increments in order to ensure stability of calculations.
 - @ref bs::Component::onDestroyed "Component::onDestroyed" - Called just before the component is destroyed. Use this instead of the destructor for cleanup.
		
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
		bool goingForward = gVirtualInput().isButtonHeld(mMoveForward);
		bool goingBack = gVirtualInput().isButtonHeld(mMoveBack);
		bool goingLeft = gVirtualInput().isButtonHeld(mMoveLeft);
		bool goingRight = gVirtualInput().isButtonHeld(mMoveRight);	
	
		// If the movement button is pressed, determine direction to move in
		Vector3 direction = Vector3::ZERO;
		if (goingForward) direction += SO()->getForward();
		if (goingBack) direction -= SO()->getForward();
		if (goingRight) direction += SO()->getRight();
		if (goingLeft) direction -= SO()->getRight();
		
		// Multiply direction with speed and move in the direction
		float frameDelta = gTime().getFrameDelta();
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
};
~~~~~~~~~~~~~

> Use @ref bs::Component::SO() "Component::SO()" to access the scene object the component is attached to.
		
> **gTime()** method provides access to a variety of timing related functionality, and is explained later in the [timing manual](@ref time).
		
# Component handle {#customComponents_c}
You will also likely want to declare a handle you can use to easily access the component, same as **HCamera** or **HRenderable**. This is done by simply creating a *typedef* on the @ref bs::GameObjectHandle<T> "GameObjectHandle<T>" object.

~~~~~~~~~~~~~{.cpp}
typedef GameObjectHandle<CCameraFlyer> HCameraFlyer;
~~~~~~~~~~~~~	
		
# Using the component {#customComponents_d}
We now have everything ready to use the component. You can create the component as any other by adding it to the scene object.

~~~~~~~~~~~~~{.cpp}
// Create a scene object to add our component to
HSceneObject cameraSO = SceneObject::create("Camera");

// We create a Camera component since our component relies on it
HCamera camera = cameraSO->addComponent<CCamera>(primaryWindow);

// And finally we add our component
HCameraFlyer = cameraSO->addComponent<CCameraFlyer>();
~~~~~~~~~~~~~

# Data {#customComponents_e}
Often a component will contain some data which you want to persist after the scene is saved, which requires you to implement a special interface on your custom component class. We'll talk more about this in the next chapter.

# Activating/deactivating a scene object {#customComponents_f}
Any scene object can be temporarily de-activated and reactivated by calling @ref bs::SceneObject::setActive "SceneObject::setActive()". When a scene object is deactivated its components will not have **Component::update()** called.

Your component can also be notified at the exact moment when activation/deactivation happens. This way you can perform additional functionality if needed. Override @ref bs::Component::onEnabled "Component::onEnabled" and @ref bs::Component::onDisabled "Component::onDisabled" to get notified every time a component is activated and deactivated, respectively.
		
~~~~~~~~~~~~~{.cpp}
// We're just extending the component we defined above
class CCameraFlyer : public Component
{
	...
	
	void onDisabled() override
	{
		gDebug().log("Component disabled.");
	}	
	
	void onEnabled() override
	{
		gDebug().log("Component enabled.");
	}
	
	...
};
~~~~~~~~~~~~~
		
# Getting notified on scene object change {#customComponents_g}
Sometimes you want to get notified when the scene object the component is attached to moves or changes parents. You can do this by overriding the @ref bs::Component::onTransformChanged "Component::onTransformChanged()" method.

~~~~~~~~~~~~~{.cpp}
// We're just extending the component we defined above
class CCameraFlyer : public Component
{
	...
	
	void onTransformChanged(TransformChangedFlags flags) override
	{
		if((flags & TCF_Transform) != 0)
			gDebug().logDebug("Parent SO moved.");
		
		if((flags & TCF_Parent) != 0)
			gDebug().logDebug("Scene object parent changed.");
	}	
	
	...
};
~~~~~~~~~~~~~

@ref bs::TransformChangedFlags "TransformChangedFlags" parameter will notify you whether the scene object moved, has changed parents, or both.

Note that **Component::onTransformChanged** will never trigger by default. You must first enable it by calling @ref bs::Component::setNotifyFlags "Component::setNotifyFlags()". It accepts the same **TransformChangedFlags** parameter which tells the system in which cases should it trigger **Component::onTransformChanged**.

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