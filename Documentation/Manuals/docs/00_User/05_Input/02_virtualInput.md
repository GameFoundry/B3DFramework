Virtual input						{#virtualInput}
===============
[TOC]

Virtual input is a high-level input system, built on top of existing **Input** functionality shown earlier. The main difference between the two systems is that virtual input abstracts the concept of buttons and axes into virtual objects, instead of referencing them directly.

This allows the application to use virtual buttons and axes without needing to know actual hardware buttons/axes the user is using. This means same virtual buttons can be used for multiple input devices (e.g. keyboard & gamepad), as well as allow the user to manually re-map keys with no additional gameplay logic.

All virtual input is handled through the @ref bs::VirtualInput "VirtualInput" class, accessible through @ref bs::gVirtualInput() "gVirtualInput()". You'll notice it shares a very similar interface with **Input**, with the only difference being how we represent buttons and axes.

Before we explain individual aspects, lets see a quick working example to give you a rough idea:
~~~~~~~~~~~~~{.cpp}
// Set up input configuration that maps virtual keys to actual hardware keys
SPtr<InputConfiguration> inputConfig = gVirtualInput().getConfiguration();

//// Virtual button named "Forward" maps to W and Up arrow keys
inputConfig->registerButton("Forward", BC_W);
inputConfig->registerButton("Forward", BC_UP);

// (Somewhere else in the app) Use the virtual key
VirtualKey forwardKey("Forward");

if(gVirtualInput().isButtonDown(forwardKey))
	gDebug().logDebug("Moving forward...");
~~~~~~~~~~~~~

# Input configuration {#virtualInput_a}
Before we can use the virtual input system, we must first create a set of virtual buttons and axes, name them, and map them to actual hardware keys. To do this we require an @ref bs::InputConfiguration "InputConfiguration" object, which can be retrieved from **VirtualInput** by calling @ref bs::VirtualInput::getConfiguration() "VirtualInput::getConfiguration()".

~~~~~~~~~~~~~{.cpp}
SPtr<InputConfiguration> inputConfig = gVirtualInput().getConfiguration();
~~~~~~~~~~~~~

# Virtual buttons {#virtualInput_b}
## Registration {#virtualInput_b_a}
Virtual buttons can be registered by giving them a unique name, and a hardware button code they map to. Any time a particular hardware button is pressed, the virtual button will be reported as pressed as well. New buttons are registered with @ref bs::InputConfiguration::registerButton() "InputConfiguration::registerButton()".

~~~~~~~~~~~~~{.cpp}
// Register a virtual button named "Forward" that maps to W and Up arrow keys
inputConfig->registerButton("Forward", BC_W);
inputConfig->registerButton("Forward", BC_UP);
~~~~~~~~~~~~~

You can also unregister an existing button by calling @ref bs::InputConfiguration::unregisterButton() "InputConfiguration::unregisterButton()".

~~~~~~~~~~~~~{.cpp}
inputConfig->unregisterButton("Forward");
~~~~~~~~~~~~~

These mappings can be registered/unregistered during runtime, meaning you should use this functionality to provide input remapping for your users.

## Usage {#virtualInput_b_b}
Once your virtual button has been registered you can use it by creating a @ref bs::VirtualButton "VirtualButton" object. This object expects the button name you provided when registering the button.

~~~~~~~~~~~~~{.cpp}
VirtualButton forwardKey("Forward");
~~~~~~~~~~~~~

> It is preferable you create virtual buttons during start-up and them save them for later use, instead of creating them every time you use them.

Created button can be used in **VirtualInput** with following events, similar to **Input** events:
 - @ref bs::VirtualInput::onButtonDown "VirtualInput::onButtonDown" - Triggered whenever a button has been pressed.
 - @ref bs::VirtualInput::onButtonUp "VirtualInput::onButtonUp" - Triggered whenever a button has been released.
 - @ref bs::VirtualInput::onButtonHeld "VirtualInput::onButtonHeld" - Triggered every frame while a button is being held.
 
~~~~~~~~~~~~~{.cpp}
Vector3 position(BsZero);

// Callback method that triggers when any virtual button is being held down
auto handleButtonHeld = [&](const VirtualButton& btn, UINT32 deviceIdx)
{
	// If user holds down W or Up arrow, move forward
	if (btn == forwardKey)
		position.z += 5.0f;
};

// Connect the callback to the event
gVirtualInput().onButtonHeld.connect(handleButtonHeld);
~~~~~~~~~~~~~

And you can also use the following polling methods. Again, similar to **Input**:
 - @ref bs::VirtualInput::isButtonDown "VirtualInput::isButtonDown" - Checks has the button been pressed this frame. Only valid for one frame.
 - @ref bs::VirtualInput::isButtonHeld "VirtualInput::isButtonHeld" - Checks is the button currently being held. This is valid for the first frame the button is pressed, and for any following frame until it is released.
 - @ref bs::VirtualInput::isButtonUp "VirtualInput::isButtonUp" - Checks has the button been released this frame. Only valid for one frame.

~~~~~~~~~~~~~{.cpp}
Vector3 position(BsZero);

// Move 5 units forward for every frame while W or Up arrow is pressed
if(gVirtualInput().isButtonHeld(forwardKey))
	position.z += 5.0f;
~~~~~~~~~~~~~

# Virtual axes {#virtualInput_c}
## Registration {#virtualInput_c_a}
Virtual axes allow you to map hardware axes (e.g. gamepad analog stick or mouse movement) to virtual axes. They are registered similarily to buttons, though **InputConfiguration** by calling @ref bs::InputConfiguration::registerAxis "InputConfiguration::registerAxis()". 

You are required to give it a unique name, and fill out @ref bs::VIRTUAL_AXIS_DESC "VIRTUAL_AXIS_DESC" structure that describes the axis. The structure allows you to choose which hardware axes to reference, as well as set other properties like sensitivity, inversion or dead zones.

~~~~~~~~~~~~~{.cpp}
// Map gamepad right stick X axis and mouse X axis to a virtual axis for looking left/right
VIRTUAL_AXIS_DESC desc;
desc.type = (int)InputAxis::RightStickX | (int)InputAxis::MouseX;

inputConfig->registerAxis("LookLeftRight", desc);
~~~~~~~~~~~~~

> Note that unlike with buttons you shouldn't call **VirtualInput::registerAxis** multiple times for the same virtual axis. Instead provide all hardware axes in the **VIRTUAL_AXIS_DESC::type** by ORing them together.

Existing virtual axes can be unmapped by calling @ref bs::InputConfiguration::unregisterAxis "InputConfiguration::unregisterAxis()".

~~~~~~~~~~~~~{.cpp}
inputConfig->unregisterAxis("LookLeftRight");
~~~~~~~~~~~~~

## Usage {#virtualInput_c_b}
Once you wish to use the virtual axis you construct a @ref bs::VirtualAxis "VirtualAxis" object by providing it with the name of the axis.
~~~~~~~~~~~~~{.cpp}
VirtualAxis lookLeftRightAxis("LookLeftRight");
~~~~~~~~~~~~~

Then you can use @ref bs::VirtualInput::getAxisValue() "VirtualInput::getAxisValue()" to retrieve the current value of the axis.

~~~~~~~~~~~~~{.cpp}
// Rotate the camera left/right depending on the axis
Degree lookAngle(0.0f);

//...

lookAngle += (Degree)gVirtualInput().getAxisValue(lookLeftRightAxis);
~~~~~~~~~~~~~