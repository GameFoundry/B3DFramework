---
title: Events
---

Events allow objects to expose events that may trigger during execution. External objects interested in those events can then register callbacks with those events and be notified when they happen. They are useful because they allow two objects to communicate without necessarily knowing about each other's types, which can reduce class coupling and improve design.

They're represented with the @Event class.

# Creating events
When creating an event, all you need to do is specify a format of the callback it sends out, for example:
~~~~~~~~~~~~~{.cpp}
class MyPlayerController
{
public:
	Event<void()> OnPlayerJumped; // Event that triggers a callback with no parameters
	Event<void(u32)> OnPlayerCollectedCoins; // Event that triggers a callback with a u32 parameter
};
~~~~~~~~~~~~~

The format of the callback method signature is the same format as followed by C++ *std::function*: *returnType(param1Type, param2Type, ...)*. 

# Triggering events

When an object is ready to trigger an event simply call it like a function:
~~~~~~~~~~~~~{.cpp}
class MyPlayerController
{
public:
	// Assume this is a function called every frame
	void Update()
	{
		bool spacePressed = /*... check input system for button press ...*/;
		u32 grabbedCoins = /*... use physics system to see if player is colliding with any coin objects ...*/;

		if(spacePressed)
			OnPlayerJumped(); // Trigger event

		if(grabbedCoins > 0)
			OnPlayerCollectedCoins(grabbedCoins); // Trigger event
	}

	Event<void()> OnPlayerJumped;
	Event<void(u32)> OnPlayerCollectedCoins;
};
~~~~~~~~~~~~~

# Subscribing to events

An external object can register itself with an event by calling @b3d::Event<RetType(Args...)>::Connect().
~~~~~~~~~~~~~{.cpp}
// Define a couple of methods that trigger when events are triggered
auto playerJumpedCallback = [&]()
{
	B3D_LOG(Info, LogGeneric, "Player jumped!");
};

auto playerCollectedCoinsCallback = [&](u32 numberOfCoins)
{
	B3D_LOG(Info, LogGeneric, "Player collected: {0} coins!", numberOfCoins);
};

MyPlayerController playerController;

// Subscribe to events
playerController.OnPlayerJumped.Connect(&playerJumpedCallback);
playerController.OnPlayerCollectedCoins.Connect(&playerCollectedCoinsCallback);

// ... run player logic every frame ...
~~~~~~~~~~~~~

Subscribing to an event will return an @b3d::HEvent handle. You can use this handle to manually disconnect from the event by calling @b3d::HEvent::Disconnect. For example:

~~~~~~~~~~~~~{.cpp}
HEvent eventHandle = playerController.OnPlayerJumped.Connect(&playerJumpedCallback);

// ... do something and decide event is no longer required ...

eventHandle.Disconnect();
~~~~~~~~~~~~~

## Class methods as event callbacks
In the example above we used lambda functions as event callbacks, and it would have also worked for global functions, but if you wish to use class methods as event callbacks, some additional logic is required. 

The main difference between lambda/global functions and class methods is that class methods have an implicit parameter, the *this* pointer. This is not something you see when you define a function, but it is always there "under the hood".

For example:
~~~~~~~~~~~~~{.cpp}
class MyEventSubscriber
{
	void EventCallback(); // What you see
	// void EventCallback(MyEventSubscriber* thisPointer); // What the compiler sees for the above method
};
~~~~~~~~~~~~~

When setting up event callbacks we must therefore provide the *this* pointer manually. However since the event owner has no idea which class will subscribe to its event, it cannot provide the *this* pointer to the object of that class. The recommended way to handle this is to wrap the class method call in a lambda function while capturing *this*.
~~~~~~~~~~~~~{.cpp}
// Alternative version of the above example, using lambda instead of std::bind
class MyEventSubscriber
{
	void PlayerJumpedCallback() // Has a hidden "this" pointer parameter
	{
		B3D_LOG(Info, LogGeneric, "Player jumped!");
	}

	void SubscribeToEvents(MyPlayerController& playerController)
	{
		// Wrap our class method call in a lambda function
		auto callback = [this]()
		{
			PlayerJumpedCallback();
		};

		// Register the callback without the event needing to know about "this" pointer parameter
		playerController.OnPlayerJumped.Connect(callback);
	}
};
~~~~~~~~~~~~~
