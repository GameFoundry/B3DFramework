GUI styles									{#guiStyles}
===============
[TOC]

So far all the GUI elements we have been creating have used the default built-in GUI style. However it is possible to fully customize an element's look by defining a @ref bs::GUIElementStyle "GUIElementStyle" object. This object allows you to specify which textures to use, style of text (if any), default dimensions, margins, padding and similar.

Each style must be given a unique name and then registered with a @ref bs::GUISkin "GUISkin" object which serves as a collection of styles. Create an empty skin by calling @ref bs::GUISkin::create "GUISkin::create()". Register the style by calling @ref bs::GUISkin "GUISkin::setStyle()".

~~~~~~~~~~~~~{.cpp}
HGUISkin skin = GUISkin::create();

HTexture myButtonTex = gImporter().import<Texture>("MyButton.png");
HSpriteTexture myButtonSpriteTex = SpriteTexture::create(myButtonTex);

// Create a style that displayes a fixed size 20x50 image
GUIElementStyle myButtonStyle;
myButtonStyle.normal.texture = myButtonSpriteTex;
myButtonStyle.fixedHeight = true;
myButtonStyle.fixedWidth = true;
myButtonStyle.height = 20;
myButtonStyle.width = 50;

skin->setStyle("MyButtonStyle", myButtonStyle);
~~~~~~~~~~~~~

Once a skin has been created you must assign it to a **GUIWidget** by calling @ref bs::GUIWidget::setSkin "GUIWidget::setSkin()". Each **GUIWidget** can only have a single skin assigned, and all GUI elements parented to that widget will use the styles from the provided skin.

~~~~~~~~~~~~~{.cpp}
HSceneObject guiSO = SceneObject::create("GUI");
HGUIWidget gui = guiSO->addComponent<CGUIWidget>(camera);

gui->setSkin(skin);
~~~~~~~~~~~~~

Finally, you must tell the GUI element which style to use by providing its name as a parameter to their **create()** method.

~~~~~~~~~~~~~{.cpp}
GUIButton* customStyleButton = GUIButton::create(HString("Click me"), "MyButtonStyle");
~~~~~~~~~~~~~

![Button with a custom style](customButton.png)

All GUI elements also have a default style name they will use if you don't provide one in a call to **create()** (as was the case for all GUI elements we created in previous chapters). You can find out that name by calling a static **getGUITypeName()** method present on all GUI elements (e.g. **GUIButton::getGUITypeName()**). You can use this name to override GUI element's default styles in **GUISkin**.

~~~~~~~~~~~~~{.cpp}
// Overrides the default style for a GUIButton
skin->setStyle(GUIButton::getGUITypeName(), myButtonStyle);
~~~~~~~~~~~~~

# GUI style properties {#guiStyles_a}
**GUIElementStyle** allows you to set a variety of different properties to customize its look. They can be categorized into *states*, *textual*, *dimensions* and *offsets*.

## States {#guiStyles_a_a}
States are used to change how an element looks as the user interacts with the element. There are four different states:
 - Normal - Default state when no interaction is happening
 - Hover - State when the user is hovering the pointer over the element
 - Active - State when the user is interacting with the element
 - Focused - State when the element has input focus
 
All elements must have a *normal* state. This is the state that defines their default look. Elements that can be interacted with (like buttons) can optionally also provide the remaining states.

The states can be set in **GUIElementStyle** by populating the following fields:
 - @ref bs::GUIElementStyle::normal "GUIElementStyle::normal"
 - @ref bs::GUIElementStyle::hover "GUIElementStyle::hover"
 - @ref bs::GUIElementStyle::active "GUIElementStyle::active"
 - @ref bs::GUIElementStyle::focused "GUIElementStyle::focused"

Additionally, certain elements can be permanently toggled on or off (i.e. toggle buttons). In that case it is useful to have a separate set of states for on and off modes. Therefore there are also states specific to the "on" mode (while above states default to "off"):
 - @ref bs::GUIElementStyle::normalOn "GUIElementStyle::normalOn"
 - @ref bs::GUIElementStyle::hoverOn "GUIElementStyle::hoverOn"
 - @ref bs::GUIElementStyle::activeOn "GUIElementStyle::activeOn"
 - @ref bs::GUIElementStyle::focusedOn "GUIElementStyle::focusedOn"
 
Each of these eight states is a @ref bs::GUIElementStyle::GUIElementStateStyle "GUIElementStateStyle" object, which contains a single sprite texture and a text color tint. When the style is active the element will render the provided texture covering its available area, and render any text using the provided color.

An example that sets up a button style that has different textures depending if the button is in normal, hovered or active state:
~~~~~~~~~~~~~{.cpp}
HTexture normalTex = gImporter().import<Texture>("normal.png");
HTexture hoverTex = gImporter().import<Texture>("hover.png");
HTexture activeTex = gImporter().import<Texture>("active.png");

GUIElementStyle myButtonStyle;
myButtonStyle.normal.texture = SpriteTexture::create(normalTex);
myButtonStyle.hover.texture = SpriteTexture::create(hoverTex);
myButtonStyle.active.texture = SpriteTexture::create(activeTex);
~~~~~~~~~~~~~

## Textual {#guiStyles_a_b}
As the name implies, this set of properties is relevant for GUI elements that display text. The properties allow you to control text font, size and alignment using the following fields:
 - @ref bs::GUIElementStyle::font "GUIElementStyle::font"
 - @ref bs::GUIElementStyle::fontSize "GUIElementStyle::fontSize"
 - @ref bs::GUIElementStyle::textHorzAlign "GUIElementStyle::textHorzAlign"
 - @ref bs::GUIElementStyle::textVertAlign "GUIElementStyle::textVertAlign"
 - @ref bs::GUIElementStyle::wordWrap "GUIElementStyle::wordWrap"
 
An example customizing text display on a GUI element:
~~~~~~~~~~~~~{.cpp}
HFont font = gResources().load<Font>("MyFont.asset");

// Style with some previously imported font of size 11, with vertically & horizontally centered and no word wrap (overflowing text will be clipped)
GUIElementStyle myButtonStyle;
myButtonStyle.font = font;
myButtonStyle.fontSize = 11;
myButtonStyle.textHorzAlign = THA_Center;
myButtonStyle.textVertAlign = TVA_Center;
~~~~~~~~~~~~~
 
> We'll show how to import fonts in a later chapter.

## Dimensions {#guiStyles_a_c}
Dimensions provided on **GUIElementStyle** allow you to choose what's the default size of the GUI element. The user can override this on a per-element basis by calling **GUIElement::setSize()**, **GUIElement::setFlexibleWidth()**, **GUIElement::setFlexibleHeight()** or similar methods (as we have done in previous chapters).

Dimensions are controlled by the following properties:
 - @ref bs::GUIElementStyle::width "GUIElementStyle::width"
 - @ref bs::GUIElementStyle::minWidth "GUIElementStyle::minWidth"
 - @ref bs::GUIElementStyle::maxWidth "GUIElementStyle::maxWidth"
 - @ref bs::GUIElementStyle::fixedWidth "GUIElementStyle::fixedWidth"
 - @ref bs::GUIElementStyle::height "GUIElementStyle::height"
 - @ref bs::GUIElementStyle::minHeight "GUIElementStyle::minHeight"
 - @ref bs::GUIElementStyle::maxHeight "GUIElementStyle::maxHeight"
 - @ref bs::GUIElementStyle::fixedHeight "GUIElementStyle::fixedHeight"
   
When **GUIElementStyle::fixedWidth** or **GUIElementStyle::fixedHeight** is set to true, the system will use size values provided by **GUIElementStyle::width** and **GUIElementStyle::height** respectively. This is the same as calling **GUIElement::setSize()**.

~~~~~~~~~~~~~{.cpp}
// Style that uses a fixed 50x20 size (e.g. a fixed size button)

GUIElementStyle style;
style.fixedHeight = true;
style.fixedWidth = true;
style.height = 50;
style.width = 20;
~~~~~~~~~~~~~
		
When **GUIElementStyle::fixedWidth** or **GUIElementStyle::fixedHeight** are false, the system will instead use the min/max values provided by **GUIElementStyle::minWidth** / **GUIElementStyle::maxWidth** and **GUIElementStyle::minHeight** / **GUIElementStyle::maxHeight**, respectively. This is the same as calling **GUIElement::setFlexibleWidth()** or **GUIElement::setFlexibleHeight()**.

~~~~~~~~~~~~~{.cpp}
// Style that uses a fixed 20px height and flexible width with minimum 30 pixels, and no limit on maximum size (e.g. a button that expands horizontally to fit its contents)

GUIElementStyle style;
style.fixedHeight = true;
style.fixedWidth = false;
style.height = 50;
style.minWidth = 20;
style.maxWidth = 0; // 0 = infinite
~~~~~~~~~~~~~
   
## Offsets {#guiStyles_a_d}
Finally, styles allow you to provide offsets that control how are GUI elements positioned relative to other elements, and how are GUI element images & text positioned relative to GUI element borders.

### Padding {#guiStyles_a_d_a}
@ref bs::GUIElementStyle::padding "GUIElementStyle::padding" allows you to specify minimum distance between this and the next element in a **GUILayout**. This field is only relevant for elements that are in automatically positioning layouts (i.e. **GUILayoutX** or **GUILayoutY**). 

~~~~~~~~~~~~~{.cpp}
GUIElementStyle style;
style.padding.right = 10; // 10 pixels padding between this and next element in a horizontal layout

// No padding in other directions
style.padding.left = 0;
style.padding.top = 0;
style.padding.bottom = 0;
~~~~~~~~~~~~~

### Margins {#guiStyles_a_d_b}
@ref bs::GUIElementStyle::margins "GUIElementStyle::margins" work similarly to padding, except they work inwards. Margins will offset visible GUI element contents (e.g. text in a button) by the specified amount. Normally the contents are centered but you can use margins to more precisely position them.

Note that margins will modify the bounds for the GUI element used for input. This means that pointer interaction will not be possible with areas outside of the zone defined by margins. This can be useful if you want to make a certain part of a GUI element visible, yet not interactable.

~~~~~~~~~~~~~{.cpp}
GUIElementStyle style;
style.margins.bottom = 5; // Ignore bottom 5 pixels of the visible image (e.g. it could be a baked-in shadow effect we don't want to consider a part of the actual GUI element)

// No margins in other directions
style.margins.left = 0;
style.margins.right = 0;
style.margins.top = 0;
~~~~~~~~~~~~~

### Content offset {#guiStyles_a_d_c}
@ref bs::GUIElementStyle::contentOffset "GUIElementStyle::contentOffset" has the same visual effect as margins, but it doesn't affect the input bounds.

~~~~~~~~~~~~~{.cpp}
GUIElementStyle style;
style.margins.left = 5; // Move the displayed button text (for example) to the right a bit, so it looks better

// No offset in other directions
style.margins.right = 0;
style.margins.top = 0;
style.margins.bottom = 0;
~~~~~~~~~~~~~

### Borders {#guiStyles_a_d_d}
Normally when you set a texture for a GUI element state, the texture will stretch to fill out the area of the GUI element. Unless the texture is a uniform color or a repeatable pattern, that stretching will often look bad.

For this reason bs::f allows you to specify borders though @ref bs::GUIElementStyle::border "GUIElementStyle::border". By setting borders you split the image into 9 sections.

Center section will be resized uniformly as normal. Four corner sections will never be resized. Top and bottom sections will be resized only horizontally (fixed height), and left and right sections will be resized only vertically (fixed width). This allows you to use more complex images that still look good when resized.

Note that this property applies equally to textures in all GUI style states.

~~~~~~~~~~~~~{.cpp}
GUIElementStyle style;
style.border.left = 4;
style.border.right = 4;
style.border.top = 4;
style.border.bottom = 4;
~~~~~~~~~~~~~

![Border sections](Scale9Grid.png) 