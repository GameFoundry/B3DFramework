---
title: Extending the GUI system
---

Even though bsf provides fully skinnable and very customizable GUI elements, sometimes the built-in ones are just not enough if you need some very specialized functionality or look. The framework allows you to create brand new elements and fully customize the way how they are rendered and how the user interacts with such elements.

You are expected to have read the user-facing GUI manuals before proceeding, and as such familiarized yourself with the basics.

All GUI elements derive from the base @bs::GUIElementBase type. The elements can be categorized into two major groups:
 - *Layouts* - They derive from @bs::GUILayout and do not have any graphics, but instead they control the placement of all elements attached to them.
 - *Elements* - They derive from @bs::GUIElement, and they're standard GUI elements like buttons, input boxes, sliders and such.
 
It's unlikely you will have the need to create **GUILayout** types, so we will instead focus on creating custom GUI elements. 
 
# GUIElement implementation
When creating a custom GUI element you will need to override the **GUIElement** class. In this section we'll go over the *minimum* set of functionality every GUI element must implement, split into relevant sections. In later sections we'll focus on optional functionality.

~~~~~~~~~~~~~{.cpp}
// GUI element that displays a single texture
class GUITexture : public GUIElement
{
	// ... implementation
};
~~~~~~~~~~~~~ 

## Construction
In its constructor **GUIElement** expects two parameters:
 - `styleName` - Name of the style to use for rendering the element. This will be looked up in the currently active **GUISkin** and relevant @bs::GUIElementStyle objects will be used for determining object's textures, fonts, colors, borders, margins and similar. Each GUI element type should have a unique default style name, but you also might want to allow the user to override the default style name if he desires. 
 - `dimensions` - Initial set of dimensions determining GUI element's position and size, represented by @bs::GUIDimensions. You can create an empty set of dimensions by calling @bs::GUIDimensions::create, in which case the system will use whichever dimensions are provided by the **GUIElementStyle**. Other overload of **GUIDimensions::create()** accepts a @bs::GUIOptions object, in which you can specify an arbitrary sized array of various dimension related options.

As a rule of thumb in order to aid construction GUI elements should provide a set of static `create()` methods (but aren't required to). You usually want to provide a few overloads of `create()` that allow the user to use custom style/dimensions or stick with the default one.

~~~~~~~~~~~~~{.cpp}
class GUITexture : public GUIElement
{
public:
	static const String& getGUITypeName()
	{
		// Returns the name of this GUI element, used for looking up the default style
		static String name = "Texture";
		return name;
	}
	
	GUITexture(const String& styleName, const GUIDimensions& dimensions)
		:GUIElement(styleName, dimensions)
	{ }

	// create() overload accepting custom dimensions in the form of GUIOptions
	static GUITexture* create(const GUIOptions& options, const String& styleName = StringUtil::BLANK)
	{
		// If user didn't provide a style name, use the default
		String finalStyleName = styleName;
		if(styleName == StringUtil::BLANK)
			finalStyleName = getGUITypeName();

		// Pass style name, and construct dimensions from the provided list of options
		return bs_new<GUITexture>(finalStyleName, GUIDimensions::create(options));
	}
	
	// create() overload using default dimensions
	static GUITexture* create(const String& styleName = StringUtil::BLANK)
	{
		// Same as above
		String finalStyleName = styleName;
		if(styleName == StringUtil::BLANK)
			finalStyleName = getGUITypeName();

		// Pass style name, and construct default dimensions object
		return bs_new<GUITexture>(finalStyleName, GUIDimensions::create());
	}

	// ... remaining GUITexture implementation
};
~~~~~~~~~~~~~ 

## Graphics
In order for your GUI element to be visible you need to define its graphics. This involves:
 - Creating a set of vertices and indices that represent your GUI element
 - Create the material to render the GUI element with, and bind required textures and other properties to it

This is done by implementing @bs::GUIElement::updateRenderElementsInternal() and @bs::GUIElement::_fillBuffer() methods.

### GUIElement::updateRenderElementsInternal()

Called whenever the element's size or style changes. This is the method where you should rebuild the GUI element's mesh. Your main goal is to populate the `mRenderElements` array with information about your GUI element and its renderable elements. 

Your GUI element can output multiple render elements. Each element corresponds to a mesh and a material to render the mesh with. For most GUI elements there will be only one element, but you will need multiple elements in case your GUI element uses multiple textures or materials.

Each entry in the `mRenderElements` array is of @bs::GUIRenderElement type and it contains:
 - Information about the mesh. This includes number of mesh vertices and indices, as well as the mesh type (triangle or line).
 - Material used, as well as material specific data such as texture and tint. You can grab built-in materials used by the GUI system from @bs::SpriteManager. We'll talk more about materials below.
 - Depth of the render element. You can overlay different render elements and this controls which element gets shown on top/bottom. Elements with higher depth are shown below elements with lower depth.

~~~~~~~~~~~~~{.cpp}
class GUITexture : public GUIElement
{
	void updateRenderElementsInternal()
	{		
		if(mRenderElements.size() < 1)
			mRenderElements.resize(1);
	
		GUIRenderElement& element = mRenderElements[0];
		
		// Render element's mesh will use 4 vertices and 6 indices (a quad)
		element.numVertices = 4;
		element.numIndices = 6;
		
		// We'll use the built-in transparent image material
		element.material = SpriteManager::instance().getImageMaterial(true);
		
		// Populate the material parameters (normally you want to populate this when properties change and just return it here)
		element.matInfo = &mMatInfo;
		
		//// Use the normal texture as provided in GUIElementStyle
		mMatInfo.texture = _getStyle().normal.texture;
		
		//// No tint
		mMatInfo.tint = Color::White;
		
		GUIElement::updateRenderElementsInternal();
	}
	
	private:
		SpriteMaterialInfo mMatInfo;
};
~~~~~~~~~~~~~

#### Materials
GUI materials are represented by the @bs::SpriteMaterial type and you can use @bs::SpriteManager to retrieve the built-in materials. You can also create your own sprite materials by implementing the **SpriteMaterial** class and registering them with the **SpriteManager** by calling @bs::SpriteManager::registerMaterial. 

Material parameters are represented by the @bs::SpriteMaterialInfo structure, which contains the texture and color to render the element with. It also contains a `groupId` identifier which tells the external systems with which other GUI elements is this element allowed to be grouped with. Grouped elements are rendered together (batched) for better performance, but sometimes this is not wanted. Generally you want to provide the memory address of the parent **GUIWidget** for the `groupId` field, and don't need to worry about it further. You can retrieve the element's **GUIWidget** by calling @bs::GUIElement::_getParentWidget().

Textures used by the material parameter can be retrieved from the current **GUIElementStyle**, or provided directly to the GUI element by user-specified means. It is preferred to use the **GUIElementStyle** approach as this allows the user to customize the element look through **GUISkin**s, or by providing a custom style name. To retrieve the currently active style call @bs::GUIElement::_getStyle(). 

### GUIElement::_fillBuffer()

This method allows you to provide the actual vertex and index data that will be used for rendering the GUI element. Note that number of vertices and indices provided must match the number you specified in **updateRenderElementsInternal()**. The method will be called once for each present render element, with the render element index as one of the parameters.

The method receives pointers to the index and vertex memory that must be populated by the function, offsets at which memory location should the function output its data at, offset to apply to vertex positions and external buffer sizes (used for overflow checking if needed).

The vertices are always in a specific format depending on @bs::GUIMeshType set on **GUIRenderElement**:

**bs::GUIMeshType::Triangle**
 - Each vertex contains a 2D position, followed by 2D texture coordinates
 - Vertex size is `sizeof(Vector2) * 2`
 
**bs::GUIMeshType::Line**
 - Each vertex contains only a 2D position
 - Vertex size is `sizeof(Vector2)`
 
~~~~~~~~~~~~~{.cpp}
class GUITexture : public GUIElement
{
	// ... remaining GUITexture implementation

	void _fillBuffer(UINT8* vertices, UINT32* indices, UINT32 vertexOffset, UINT32 indexOffset,
		const Vector2I& offset, UINT32 maxNumVerts, UINT32 maxNumIndices, UINT32 renderElementIdx) const
	{
		UINT32 vertexStride = sizeof(Vector2) * 2;
		UINT32 indexStride = sizeof(UINT32);

		// Get location of this element in the provided buffers
		vertices += vertexStride * vertexOffset;
		UINT8* uvs = vertices + sizeof(Vector2);
		
		indices += indexStride * indexOffset;
		
		// Populate the vertices, uv and indices buffer
		// ... generate vertices, uvs and indices as you would for any mesh
	}
};
~~~~~~~~~~~~~

The framework also provides a set of helper classes for generating required geometry in the form of @bs::ImageSprite and @bs::TextSprite classes. **ImageSprite** can easily generate image geometry of specified size, whether a simple quad or a scale-9-grid image (scalable image with fixed borders). And **TextSprite** will take a text string, font and additional options as input, and output a set of quads required for text rendering. It is suggested that you use these unless you require non-quad meshes.

Note that it is suggested that you generate the actual mesh geometry in **updateRenderElementsInternal()**, and then just return the generated data in **_fillBuffer()**. This is beneficial as the geometry only needs to change when render elements are updated, and **_fillBuffer()** will be called more often than **updateRenderElementsInternal()**.

## Layout
In order for the GUI element to work well with the automatic positioning performed by GUI layouts, you must override the @bs::GUIElement::_getOptimalSize method. As the name implies the method should return the optimal size of your GUI element. For example if your element was displaying a texture 64x64 in size, then the optimal size should probably return 64x64. If you element is displaying text you can use @bs::GUIHelper to help you calculate the bounds of the text. If displaying something else then it's up to you to determine what constitutes an optimal size.

~~~~~~~~~~~~~{.cpp}
class GUITexture : public GUIElement
{
	// ... remaining GUITexture implementation

	Vector2I _getOptimalSize() const
	{
		Vector2I optimalSize;
		
		// Retrieve the size of the main texture, if one is provided
		HSpriteTexture tex = _getStyle().normal.texture;
		if(tex != nullptr)
		{
			optimalSize.x = tex->getWidth();
			optimalSize.y = tex->getHeight();
		}
		
		return optimalSize;
	}
};
~~~~~~~~~~~~~

This concludes the minimal set of functionality required for implementing a **GUIElement**. At this point you will have a static GUI element that allows no user interaction. In next sections we expand on this by adding support for user input and dynamic graphics.

# Redrawing
If your element allows the user to update certain contents of it after creation (i.e. it's not static), you need to notify the GUI system so it knows when to rebuild the element. For example if your GUI element displays a text string you might provide a `setText()` method that allows the user to change what is displayed. When text is changed the GUI system will need to update the batched GUI mesh, update the GUI element's mesh and/or update the layout position/size of the element.

There are three ways to notify the GUI system the element is dirty, each more expensive in performance than the previous:
 - @bs::GUIElement::_markMeshAsDirty() - Causes the batched GUI mesh to be re-assembled. Will cause a call to **GUIElement::_fillBuffer()**. Call this if element's contents and size didn't change (e.g. when its position, or depth changes). Batched GUI mesh is a mesh managed by the GUI system that consists of multiple GUI elements sharing the same material properties (used to improve rendering performance).
 - @bs::GUIElement::_markContentAsDirty() - Has the same effect as **GUIElement::_markMeshDirty()**, but will also trigger a **GUIElement::updateRenderElementsInternal()** call so the GUI element mesh will be fully rebuilt. Call this when contents of the GUI element change, but not its size (e.g. changing a texture but it is the same dimensions as the old one.)
 - @bs::GUIElement::_markLayoutAsDirty() - Has the same effect as **GUIElement::_markContentDirty()**, but will also cause the layout system to recalculate the element's position and size. This can be expensive as this will generally trigger layout updates for all siblings and children, potentially even parent GUI elements. Call this when element changes and no other dirty calls are appropriate. Normally you have to call this whenever the element's size changes.
 
~~~~~~~~~~~~~{.cpp}
class GUITexture : public GUIElement
{
	// ... remaining GUITexture implementation

	// Changes the look of the GUI element by fading it out when enabled
	void setFadeOut(bool fadeOut)
	{
		if(fadeOut)
			mMatParams.tint = Color(1.0f, 1.0f, 1.0f, 0.5f);
		else
			mMatParams.tint = Color::White;
		
		// Ensure the system redraws the mesh. Since neither the size or contents changed we can call the cheapest redraw method, '_markMeshAsDirty()'
		_markMeshAsDirty();
	}
	
	// Updated version of _getMaterial()
	const SpriteMaterialInfo& _getMaterial(UINT32 renderElementIdx, SpriteMaterial** material) const
	{
		// Get material for rendering opaque images
		*material = SpriteManager::instance().getImageOpaqueMaterial();
		
		// Assuming remaining mMatParams contents are populated elsewhere
		return mMatParams;
	}
	
	SpriteMaterialInfo mMatParams;
};
~~~~~~~~~~~~~
 
# Input
In order for the GUI element to accept input from the user you can implement any or all of the following methods:
 - @bs::GUIElement::_mouseEvent - Triggered when the mouse moves and/or is clicked. See @bs::GUIMouseEventType for a list of events. Receives @bs::GUIMouseEvent structure as input.
 - @bs::GUIElement::_textInputEvent - Triggered when the user inputs some text on the keyboard, while the GUI element is in focus. Receives @bs::GUITextInputEvent structure as input.
 - @bs::GUIElement::_virtualButtonEvent - Triggered when a certain virtual button is pressed, while the GUI element is in focus. Receives @bs::GUIVirtualButtonEvent structure as input.
 - @bs::GUIElement::_commandEvent - Triggers when some kind of a specialized event happens, like losing/gaining focus, deleting text, selecting text, forcing redraw and similar. See @bs::GUICommandEventType for an explanation of what each event type does. Receives @bs::GUICommandEvent structure as input.
 
Each of the methods should return false if other GUI elements are allowed to receive the same event. If true is returned the GUI element will consume the event and it wont be visible to others.
 
~~~~~~~~~~~~~{.cpp}
class GUITexture : public GUIElement
{
	// ... remaining GUITexture implementation

	bool _mouseEvent(const GUIMouseEvent& ev)
	{
		// Fade out the texture when user mouses over the element
		if(ev.getType() == GUIMouseEventType::MouseOver)
			setFadeOut(true);
		else if(ev.getType() == GUIMouseEventType::MouseOut)
			setFadeOut(false);

		return false;
	}
};
~~~~~~~~~~~~~
