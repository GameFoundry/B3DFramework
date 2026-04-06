---
title: GUI styles
---

All GUI elements in the framework are styled using CSS-like style sheets. Style sheets define how elements are rendered, including colors, fonts, sizes, borders, and more. The framework uses the @b3d::GUIStyleSheet class to manage these styles through a cascading system similar to web CSS.

# Style sheets

Style sheets are CSS files that define the visual appearance of GUI elements. The framework provides a default style sheet that can be loaded from the built-in resources:

~~~~~~~~~~~~~{.cpp}
HSceneObject guiSceneObject = SceneObject::Create("GUI");
HGUIWidget guiWidget = guiSceneObject->AddComponent<GUIWidget>(camera);

// Assign the default style sheet
guiWidget->SetStyleSheetCascade(GetBuiltinResources().GetDefaultGUIStyleSheetCascade());
~~~~~~~~~~~~~

Style sheets use standard CSS syntax with selectors, properties, and values. Here is an example of a simple button style:

~~~~~~~~~~~~~{.css}
button
{
    min-width: 20px;
    height: 22px;
    margin-bottom: 4px;
    text-align: center;
    vertical-align: middle;
    color: #B2B2B2;
    background-color: #212121;
    border-color: #1B1B1B;
    border-width: 2px;
    font-family: "Arial";
    font-size: 8;
}
~~~~~~~~~~~~~

# Selectors

Style sheets support several types of selectors to target specific GUI elements:

## Element selectors

Element selectors target all instances of a particular GUI element type:

~~~~~~~~~~~~~{.css}
button { /* Applies to all buttons */ }
label { /* Applies to all labels */ }
inputbox { /* Applies to all input boxes */ }
~~~~~~~~~~~~~

## Class selectors

Class selectors target elements with a specific class name, prefixed with a dot:

~~~~~~~~~~~~~{.css}
button.PrimaryButton
{
    background-color: #0E82FF;
}

button.DangerButton
{
    background-color: #C40000;
}
~~~~~~~~~~~~~

Assign a class to a GUI element by calling @b3d::GUIRenderable::SetStyleSheetClass:

~~~~~~~~~~~~~{.cpp}
GUIButton* primaryButton = GUIButton::Create(HString("Save"));
primaryButton->SetStyleSheetClass("PrimaryButton");
~~~~~~~~~~~~~

## ID selectors

ID selectors target a single element with a specific ID, prefixed with a hash:

~~~~~~~~~~~~~{.css}
#MainMenuButton
{
    width: 200px;
    height: 50px;
}
~~~~~~~~~~~~~

Assign an ID to a GUI element by calling @b3d::GUIElement::SetElementId:

~~~~~~~~~~~~~{.cpp}
GUIButton* mainMenuButton = GUIButton::Create(HString("Main Menu"));
mainMenuButton->SetElementId("MainMenuButton");
~~~~~~~~~~~~~

## Pseudo-class selectors

Pseudo-class selectors target elements in specific states, using a colon:

~~~~~~~~~~~~~{.css}
button:hover
{
    background-color: #2D2D2D;
    border-color: #1B1B1B;
}

button:focus
{
    border-color: #FFA800;
}

button:active
{
    background-color: #FFA800;
    color: #000000;
}

button:checked
{
    background-color: #FFA800;
}
~~~~~~~~~~~~~

Supported pseudo-classes include:
- `:hover` - When the pointer is over the element
- `:focus` - When the element has input focus
- `:active` - When the element is being actively interacted with
- `:checked` - For toggle elements in the checked state
- `:disabled` - When the element is disabled

## Pseudo-element selectors

Pseudo-element selectors target sub-parts of GUI elements, using double colons:

~~~~~~~~~~~~~{.css}
button.Toggle::checkmark
{
    color: #FFA800;
    width: 9px;
    height: 11px;
}
~~~~~~~~~~~~~

# CSS variables

Style sheets support CSS variables for consistent theming. Variables are defined in the `:root` selector and referenced using `var()`:

~~~~~~~~~~~~~{.css}
:root
{
    --PrimaryContentColor: #B2B2B2;
    --PrimaryBackgroundColor: #212121;
    --FocusColor: #FFA800;
    --PrimaryFontFamily: "Arial";
    --PrimaryFontSize: 8;
}

button
{
    color: var(--PrimaryContentColor);
    background-color: var(--PrimaryBackgroundColor);
    font-family: var(--PrimaryFontFamily);
    font-size: var(--PrimaryFontSize);
}

button:focus
{
    border-color: var(--FocusColor);
}
~~~~~~~~~~~~~

# Style properties

Style sheets support a wide range of properties for controlling element appearance and layout.

## Layout properties

Control element size and spacing:

~~~~~~~~~~~~~{.css}
button
{
    width: 100px;
    height: 30px;
    min-width: 50px;
    max-width: 200px;

    margin: 4px;
    margin-top: 8px;
    margin-bottom: 4px;

    padding: 5px;
    padding-left: 10px;
}
~~~~~~~~~~~~~

Properties:
- `width`, `height` - Fixed dimensions
- `min-width`, `min-height` - Minimum dimensions
- `max-width`, `max-height` - Maximum dimensions
- `margin` - Outer spacing (shorthand or individual sides)
- `padding` - Inner spacing (shorthand or individual sides)

## Visual properties

Control colors, backgrounds, and visibility:

~~~~~~~~~~~~~{.css}
label
{
    color: #B2B2B2;
    background-color: #212121;
    background-image: url("background.png");
    opacity: 0.8;
    visibility: visible; /* or hidden */
}
~~~~~~~~~~~~~

Properties:
- `color` - Text and content color
- `background-color` - Background fill color
- `background-image` - Background image (use `url()` to reference)
- `opacity` - Transparency (0.0 to 1.0)
- `visibility` - Show or hide element

## Text properties

Control font and text formatting:

~~~~~~~~~~~~~{.css}
label
{
    font-family: "Arial";
    font-size: 10;
    text-align: center; /* left, center, right */
    vertical-align: middle; /* top, middle, bottom */
    word-wrap: wrap-word; /* or none */
}
~~~~~~~~~~~~~

Properties:
- `font-family` - Font name (must be imported)
- `font-size` - Font size in points
- `text-align` - Horizontal text alignment
- `vertical-align` - Vertical text alignment
- `word-wrap` - Text wrapping behavior

## Border properties

Control element borders:

~~~~~~~~~~~~~{.css}
button
{
    border-width: 2px;
    border-color: #1B1B1B;
    border-style: solid;

    border-radius: 5px;
    border-top-left-radius: 10px;
    border-top-right-radius: 10px;
}
~~~~~~~~~~~~~

You can also set individual border sides:

~~~~~~~~~~~~~{.css}
button
{
    border-top-width: 1px;
    border-top-color: #FFFFFF;
    border-top-style: solid;

    border-bottom-width: 2px;
    border-bottom-color: #000000;
}
~~~~~~~~~~~~~

Properties:
- `border` - Shorthand for all borders
- `border-width` - Border thickness
- `border-color` - Border color
- `border-style` - Border style (currently only `solid` is supported)
- `border-radius` - Rounded corners
- Individual border sides: `border-top`, `border-right`, `border-bottom`, `border-left`

# Loading custom style sheets

Create custom style sheets by writing CSS files and loading them as resources:

~~~~~~~~~~~~~{.cpp}
// Import the CSS file
HGUIStyleSheet customStyleSheet = GetImporter().Import<GUIStyleSheet>("MyStyles.css");

// Create a cascade with the custom style sheet
SPtr<GUIStyleSheetCascade> styleSheetCascade = GUIStyleSheetCascade::Create();
styleSheetCascade->RegisterStyleSheet(customStyleSheet, 0);

// Assign to widget
guiWidget->SetStyleSheetCascade(styleSheetCascade);
~~~~~~~~~~~~~

# Style sheet cascades

The @b3d::GUIStyleSheetCascade class manages multiple style sheets with importance levels. Style sheets with higher importance values override rules from those with lower importance. This allows for layered styling:

~~~~~~~~~~~~~{.cpp}
// Create a cascade with multiple style sheets
SPtr<GUIStyleSheetCascade> cascade = GUIStyleSheetCascade::Create();

// Base styles (lowest importance)
cascade->RegisterStyleSheet(baseStyleSheet, 0);

// Theme-specific styles (higher importance)
cascade->RegisterStyleSheet(darkThemeStyleSheet, 10);

// Component-specific overrides (highest importance)
cascade->RegisterStyleSheet(customComponentStyleSheet, 20);

guiWidget->SetStyleSheetCascade(cascade);
~~~~~~~~~~~~~

# Querying styles at runtime

You can query the computed styles for a GUI element using @b3d::GUIStyleSheet::BuildRules:

~~~~~~~~~~~~~{.cpp}
HGUIStyleSheet styleSheet = GetBuiltinResources().GetDefaultGUIStyleSheet();

// Build rules for a button element
GUIStyleSheetRules rules = styleSheet->BuildRules("button");

// Access specific properties
Color backgroundColor = rules.BackgroundColor;
float fontSize = rules.FontSize;
HFont font = rules.Font;
~~~~~~~~~~~~~

Query rules for specific element states:

~~~~~~~~~~~~~{.cpp}
// Build rules for a button in hover state
GUIStyleSheetRules hoverRules = styleSheet->BuildRules("button", "", "", "", "hover");

// Build rules for a button with a specific class
GUIStyleSheetRules classRules = styleSheet->BuildRules("button", "PrimaryButton");
~~~~~~~~~~~~~
