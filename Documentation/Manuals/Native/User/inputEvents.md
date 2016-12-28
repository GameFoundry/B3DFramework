Input events						{#inputEvents}
===============

Events represent another way of handling user input. They are an alternative to input polling, and it's up to the developer to choose which way of handling input he prefers. These approaches aren't identical though, and events can provide more information than polling. Same as polling, events are also handled by the **Input** class.

# Button presses
You can subscribe to the following events that report when the user interacted with a button:
 - @ref bs::Input::onButtonDown "Input::onButtonDown" - Triggered whenever a button has been pressed.
 - @ref bs::Input::onButtonUp "Input::onButtonUp" - Triggered whenever a button has been released.
 
Both of these events supply the @ref bs::ButtonEvent "ButtonEvent" structure, containing the code of the button that was pressed, along with some other information.

~~~~~~~~~~~~~{.cpp}
Vector3 position(BsZero);

// Callback method that triggers when any button is pressed
auto handleButtonDown = [&](const ButtonEvent& event)
{
	// If user presses space, "jump"
	if (event.buttonCode == BC_SPACE)
		position.y += 5.0f;
};

// Connect the callback to the event
gInput().onButtonDown.connect(handleButtonDown);
~~~~~~~~~~~~~

# Mouse/touch input
Use @ref bs::Input::onPointerMoved "Input::onPointerMoved" to track whenever the user moves the mouse or his finger on a touch device. The event supplies the @ref bs::PointerEvent "PointerEvent" structure, containing information like screen position of the event, delta from the last frame, state of all the mouse buttons, scroll wheel movement and more.

~~~~~~~~~~~~~{.cpp}
Vector3 position(BsZero);

// Callback method that triggers whenever the pointer moves
auto handlePointerMove = [&](const PointerEvent& event)
{
	// Move the object in 2D plane together with the pointer, if left mouse button is pressed
	if(event.buttonStates[(int)PointerEventButton::Left])
	{
		position.x = (float)event.screenPos.x;
		position.y = (float)event.screenPos.y;
	}
	
	// Change object depth depending on mouse scroll wheel
	position.z += event.mouseWheelScrollAmount;
};

// Connect the callback to the event
gInput().onPointerMoved.connect(handlePointerMove);
~~~~~~~~~~~~~

Pointers may also receive specialized button down/up events, similar to *Input::onButtonDown* and *Input::onButtonUp*. They trigger at the same time, but provide *PointerEvent* structure instead of *ButtonEvent* - which may be more useful in certain situations. These methods are:
 - @ref bs::Input::onPointerPressed "Input::onPointerPressed" - Triggered whenever a pointer button has been pressed or screen touch began.
 - @ref bs::Input::onPointerReleased "Input::onPointerReleased" - Triggered whenever a pointer button has been released or screen touch ended.
 - @ref bs::Input::onPointerDoubleClick "Input::onPointerDoubleClick" - Triggered when the user quickly clicks the pointer buttons or taps the screen in succession.
 
~~~~~~~~~~~~~{.cpp}
Vector3 position(BsZero);

// Callback method that triggers when any button is pressed
auto handleDoubleClick = [&](const PointerEvent& event)
{
	// Jump on double click
	position.y += 5.0f;
};

// Connect the callback to the event
gInput().onPointerDoubleClick.connect(handleDoubleClick);
~~~~~~~~~~~~~

# Text input
If user is typing text (using a physical or a touch keyboard) you may subscribe to @ref bs::Input::onCharInput "Input::onCharInput" to receive individual characters as the user inputs them. 

~~~~~~~~~~~~~{.cpp}
StringStream inputString;

// Callback method that appends a character to the inputString stream
auto handleDoubleClick = [&](const TextInputEvent& event)
{
	inputString<<(char)textChar;
};

// Connect the callback to the event
gInput().onCharInput.connect(handleCharInput);
~~~~~~~~~~~~~

Note that the system will register keyboard buttons as both text input and as normal button presses - it's up to the caller to decide which to process when. If keyboard is used for gameplay then button presses should be used, but if user is actually typing text, then character input is better suited. This is because button events report button codes as physical keyboard keys, yet character input will actually translate those physical key presses into character codes depending on the user's keyboard settings, which ensures non-english keyboard layouts work as intended.