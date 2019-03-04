Events						{#events}
===============
[TOC]

Events allow objects to expose events that may trigger during execution. External objects interested in those events can then register callbacks with those events and be notified when they happen. They are useful because they allow two objects to communicate without necessarily knowing about each other's types, which can reduce class coupling and improve design.

They're represented with the @ref bs::TEvent<RetType, Args> "Event" class.

# Creating events {#events_a}
When creating an event, all you need to do is specify a format of the callback it sends out, for example:
~~~~~~~~~~~~~{.cpp}
class MyPlayerController
{
public:
	Event<void()> onPlayerJumped; // Event that triggers a callback with no parameters
	Event<void(UINT32)> onPlayerCollectedCoins; // Event that triggers a callback with an UINT32 parameter
};
~~~~~~~~~~~~~

The format of the callback method signature is the same format as followed by C++ *std::function*: *returnType(param1Type, param2Type, ...)*. 

# Triggering events {#events_b}

When an object is ready to trigger an event simply call it like a function:
~~~~~~~~~~~~~{.cpp}
class MyPlayerController
{
public:
	// Assume this is a function called every frame
	void update()
	{
		bool spacePressed = /*... check input system for button press ...*/;
		UINT32 grabbedCoins = /*... use physics system to see if player is colliding with any coin objects ...*/;
		
		if(spacePressed)
			onPlayerJumped(); // Trigger event
			
		if(grabbedCoins > 0)
			onPlayerCollectedCoins(grabbedCoins); // Trigger event
	}

	Event<void()> onPlayerJumped;
	Event<void(UINT32)> onPlayerCollectedCoins;
};
~~~~~~~~~~~~~

# Subscribing to events {#events_c}

An external object can register itself with an event by calling @ref bs::TEvent<RetType, Args> "Event::connect()". 
~~~~~~~~~~~~~{.cpp}
// Define a couple of methods that trigger when events are triggered
auto playerJumpedCallback = [&]()
{
	gDebug().logDebug("Player jumped!");
};

auto playerCollectedCoinsCallback = [&](UINT32 numCoins)
{
	gDebug().logDebug("Player collected: " + toString(numCoins) + " coins!");
};

MyPlayerController playerController;

// Subscribe to events
playerController.onPlayerJumped.connect(&playerJumpedCallback);
playerController.onPlayerCollectedCoins.connect(&playerCollectedCoinsCallback);

// ... run player logic every frame ...
~~~~~~~~~~~~~

Subscribing to an event will return an @ref bs::HEvent "HEvent" handle. You can use this handle to manually disconnect from the event by calling @ref bs::HEvent::disconnect "HEvent::disconnect". For example:

~~~~~~~~~~~~~{.cpp}
HEvent eventHandle = playerController.onPlayerJumped.connect(&playerJumpedCallback);

// ... do something and decide event is no longer required ...

eventHandle.disconnect();
~~~~~~~~~~~~~

## Class methods as event callbacks {#events_c_a}
In the example above we used lambda functions as event callbacks, and it would have also worked for global functions, but if you wish to use class methods as event callbacks, some additional logic is required. 

The main difference between lambda/global functions and class methods is that class methods have an implicit parameter, the *this* pointer. This is not something you see when you define a function, but it is always there "under the hood".

For example:
~~~~~~~~~~~~~{.cpp}
class MyEventSubscriber
{
	void eventCallback(); // What you see
	// void eventCallback(MyEventSubscriber* thisPtr); // What the compiler sees for the above method
};
~~~~~~~~~~~~~

When setting up event callbacks we must therefore provide the *this* pointer manually. However since the event owner has no idea which class will subscribe to its event, it cannot provide the *this* pointer to the object of that class. By using **std::bind** the event subscriber can instead permanently bind its *this* pointer when subscribing to an event.

If you are unfamiliar with *std::bind*, it basically transforms a function signature by permanently *binding* one of its parameters. Here's a simple example:
~~~~~~~~~~~~~{.cpp}
auto myFunc = [&](UINT32 param)
{ };

// Permanently binds 5 as the function parameter
auto myFuncNoParams = std::bind(myFunc, 5);

// Call the function with parameter 5
myFuncNoParams();
~~~~~~~~~~~~~

Using this same logic, we can permanently bind the *this* pointer when subscribing to an event:
~~~~~~~~~~~~~{.cpp}
class MyEventSubscriber
{
	void playerJumpedCallback() // Has a hidden "this" pointer parameter
	{
		gDebug().logDebug("Player jumped!");
	}
	
	// Assuming the same player controller class we defined earlier
	void subscribeToEvents(MyPlayerController& playerController)
	{
		// Bind the "this" pointer
		auto callback = std::bind(&MyEventSubscriber::playerJumpedCallback, this);
	
		// Register the callback without the event needing to know about "this" pointer parameter
		playerController.onPlayerJumped.connect(callback);
	}
};
~~~~~~~~~~~~~

Alternatively, you can wrap the class function call in a lambda function while capturing *this*.
~~~~~~~~~~~~~{.cpp}
// Alternative version of the above example, using lambda instead of std::bind
class MyEventSubscriber
{
	void playerJumpedCallback() // Has a hidden "this" pointer parameter
	{
		gDebug().logDebug("Player jumped!");
	}
	
	void subscribeToEvents(MyPlayerController& playerController)
	{
		// Wrap our class method call in a lambda function
		auto callback = [this]()
		{
			playerJumpedCallback();
		};
		
		// Register the callback without the event needing to know about "this" pointer parameter
		playerController.onPlayerJumped.connect(callback);
	}
};
~~~~~~~~~~~~~
