---
title: Extending the GUI system
---

Even though bsf provides fully skinnable and very customizable GUI elements, sometimes the built-in ones are just not enough if you need some very specialized functionality or look. The framework allows you to create brand new elements and fully customize the way how they are rendered and how the user interacts with such elements.

You are expected to have read the user-facing GUI manuals before proceeding, and as such familiarized yourself with the basics.

All GUI elements derive from the base @b3d::GUIElement type. The elements can be categorized into two major groups:
 - *Layouts* - They derive from @b3d::GUILayout and do not have any graphics, but instead they control the placement of all elements attached to them.
 - *Renderable* - They derive from @b3d::GUIRenderable, and provide graphics rendering capabilities.
 - *Interactable* - They derive from @b3d::GUIInteractable, and they're standard GUI elements like buttons, input boxes, sliders and such that support user interaction.
 
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
In its constructor **GUIRenderable** expects two parameters:
 - `styleClass` - Name of the style class to use for rendering the element. This will be looked up in the currently active style sheet and relevant style rules will be used for determining object's textures, fonts, colors, borders, margins and similar. Each GUI element type should have a unique default style class name, but you also might want to allow the user to override it if desired. 
 - `sizeConstraints` - Initial set of size constraints determining GUI element's size constraints, represented by @b3d::GUISizeConstraints. You can create empty constraints by calling @b3d::GUISizeConstraints::Create(), in which case the system will use whichever constraints are provided by the style sheet. Another overload of **GUISizeConstraints::Create()** accepts a @b3d::GUIOptions object, in which you can specify an arbitrary sized array of various size constraint related options.

As a rule of thumb in order to aid construction GUI elements should provide a set of static `create()` methods (but aren't required to). You usually want to provide a few overloads of `create()` that allow the user to use custom style/dimensions or stick with the default one.

~~~~~~~~~~~~~{.cpp}
class GUITexture : public GUIRenderable
{
public:
	static const String& GetGuiTypeName()
	{
		// Returns the name of this GUI element, used for looking up the style class
		static String name = "Texture";
		return name;
	}
	
	GUITexture(PrivatelyConstruct, const String& styleClass, const GUISizeConstraints& sizeConstraints)
		:GUIRenderable(styleClass, sizeConstraints)
	{ }

	// Create() overload accepting custom size constraints in the form of GUIOptions
	static GUITexture* Create(const GUIOptions& options, const String& styleClass = StringUtility::kBlank)
	{
		// If user didn't provide a style class, use the default
		String finalStyleClass = GetStyleClass<GUITexture>(styleClass);

		// Pass style class, and construct size constraints from the provided list of options
		return B3DNew<GUITexture>(PrivatelyConstruct(), finalStyleClass, GUISizeConstraints::Create(options));
	}
	
	// Create() overload using default size constraints
	static GUITexture* Create(const String& styleClass = StringUtility::kBlank)
	{
		// Same as above
		String finalStyleClass = GetStyleClass<GUITexture>(styleClass);

		// Pass style class, and construct default size constraints object
		return B3DNew<GUITexture>(PrivatelyConstruct(), finalStyleClass, GUISizeConstraints::Create());
	}

	// ... remaining GUITexture implementation
};
~~~~~~~~~~~~~ 

## Graphics
In order for your GUI element to be visible you need to define its graphics. This involves:
 - Creating a set of vertices and indices that represent your GUI element
 - Create the material to render the GUI element with, and bind required textures and other properties to it

This is done by implementing @b3d::GUIRenderable::UpdateRenderElements() and @b3d::GUIRenderable::FillBuffer() methods.

### GUIRenderable::UpdateRenderElements()

Called whenever the element's size or style changes. This is the method where you should rebuild the GUI element's mesh. Your main goal is to populate the internal render elements with information about your GUI element and its renderable elements. 

Your GUI element can output multiple render elements. Each element corresponds to a mesh and a material to render the mesh with. For most GUI elements there will be only one element, but you will need multiple elements in case your GUI element uses multiple textures or materials.

Each render element is of @b3d::GUIRenderElement type and it contains:
 - Information about the mesh. This includes number of mesh vertices and indices, as well as the mesh type (triangle or line).
 - Material used, as well as material specific data such as texture and tint. You can grab built-in materials used by the GUI system from @b3d::SpriteManager. We'll talk more about materials below.
 - Depth of the render element. You can overlay different render elements and this controls which element gets shown on top/bottom. Elements with higher depth are shown below elements with lower depth.

~~~~~~~~~~~~~{.cpp}
class GUITexture : public GUIRenderable
{
	void UpdateRenderElements() override
	{		
		// Create render element helper with sprite information
		Sprite* sprite = mSprite.GetPtr();
		if(sprite)
		{
			GUIRenderElementHelper::SpriteInfo spriteInfo(sprite, 0); // depth 0
			GUIRenderElementHelper::Populate(spriteInfo, mRenderElements);
		}
	}
	
	private:
		TShared<Sprite> mSprite;
};
~~~~~~~~~~~~~

#### Materials
GUI materials are represented by the @b3d::SpriteMaterial type and you can use @b3d::SpriteManager to retrieve the built-in materials. You can also create your own sprite materials by implementing the **SpriteMaterial** class and registering them with the **SpriteManager** by calling @b3d::SpriteManager::RegisterMaterial(). 

Material parameters are represented by the @b3d::SpriteMaterialInfo structure, which contains the texture and color to render the element with. It also contains a `groupId` identifier which tells the external systems with which other GUI elements is this element allowed to be grouped with. Grouped elements are rendered together (batched) for better performance, but sometimes this is not wanted. Generally you want to provide the memory address of the parent **GUIWidget** for the `groupId` field, and don't need to worry about it further. You can retrieve the element's **GUIWidget** by calling @b3d::GUIElement::GetParentWidget().

Textures used by the material parameter can be retrieved from the current style, or provided directly to the GUI element by user-specified means. It is preferred to use the style sheet approach as this allows the user to customize the element look through custom style sheets, or by providing a custom style class name.

### GUIRenderable::FillBuffer()

This method allows you to provide the actual vertex and index data that will be used for rendering the GUI element. Note that number of vertices and indices provided must match the number you specified in **UpdateRenderElements()**. The method will be called once for each present render element, with the render element index as one of the parameters.

The method receives pointers to the index and vertex memory that must be populated by the function, offsets at which memory location should the function output its data at, offset to apply to vertex positions and external buffer sizes (used for overflow checking if needed).

The vertices are always in a specific format depending on @b3d::GUIMeshType set on **GUIRenderElement**:

**b3d::GUIMeshType::Triangle**
 - Each vertex contains a 2D position, followed by 2D texture coordinates
 - Vertex size is `sizeof(Vector2) * 2`
 
**b3d::GUIMeshType::Line**
 - Each vertex contains only a 2D position
 - Vertex size is `sizeof(Vector2)`
 
~~~~~~~~~~~~~{.cpp}
class GUITexture : public GUIRenderable
{
	// ... remaining GUITexture implementation

	void FillBuffer(u8* vertices, u32* indices, u32 vertexOffset, u32 indexOffset,
		const Vector2I& offset, u32 maxVertexCount, u32 maxIndexCount, u32 renderElementIndex) const override
	{
		u32 vertexStride = sizeof(Vector2) * 2;
		u32 indexStride = sizeof(u32);

		// Get location of this element in the provided buffers
		vertices += vertexStride * vertexOffset;
		u8* uvs = vertices + sizeof(Vector2);
		
		indices += indexStride * indexOffset;
		
		// Populate the vertices, uv and indices buffer
		// ... generate vertices, uvs and indices as you would for any mesh
	}
};
~~~~~~~~~~~~~

The framework also provides a set of helper classes for generating required geometry in the form of @b3d::ImageSprite and @b3d::TextSprite classes. **ImageSprite** can easily generate image geometry of specified size, whether a simple quad or a scale-9-grid image (scalable image with fixed borders). And **TextSprite** will take a text string, font and additional options as input, and output a set of quads required for text rendering. It is suggested that you use these unless you require non-quad meshes.

Note that it is suggested that you generate the actual mesh geometry in **UpdateRenderElements()**, and then just return the generated data in **FillBuffer()**. This is beneficial as the geometry only needs to change when render elements are updated, and **FillBuffer()** will be called more often than **UpdateRenderElements()**.

## Layout
In order for the GUI element to work well with the automatic positioning performed by GUI layouts, you must override the @b3d::GUIElement::CalculateUnconstrainedOptimalSize method. As the name implies the method should return the optimal size of your GUI element. For example if your element was displaying a texture 64x64 in size, then the optimal size should probably return 64x64. If your element is displaying text you can use @b3d::GUIUtility to help you calculate the bounds of the text. If displaying something else then it's up to you to determine what constitutes an optimal size.

~~~~~~~~~~~~~{.cpp}
class GUITexture : public GUIRenderable
{
	// ... remaining GUITexture implementation

	GUILogicalSize CalculateUnconstrainedOptimalSize() const override
	{
		GUILogicalSize optimalSize{kZeroTag};
		
		// Retrieve the size of the main texture, if one is provided
		const HSpriteTexture& tex = GetTextureFromStyle();
		if(tex != nullptr)
		{
			// SpriteTexture has width/height accessible via its content bounds
			optimalSize.Width = GUILogicalUnit(tex->GetWidth());
			optimalSize.Height = GUILogicalUnit(tex->GetHeight());
		}
		
		return optimalSize;
	}
};
~~~~~~~~~~~~~

This concludes the minimal set of functionality required for implementing a **GUIElement**. At this point you will have a static GUI element that allows no user interaction. In next sections we expand on this by adding support for user input and dynamic graphics.

# Redrawing
If your element allows the user to update certain contents of it after creation (i.e. it's not static), you need to notify the GUI system so it knows when to rebuild the element. For example if your GUI element displays a text string you might provide a `SetText()` method that allows the user to change what is displayed. When text is changed the GUI system will need to update the batched GUI mesh, update the GUI element's mesh and/or update the layout position/size of the element.

There are three ways to notify the GUI system the element is dirty, each more expensive in performance than the previous:
 - @b3d::GUIElement::MarkMeshAsDirty() - Causes the batched GUI mesh to be re-assembled. Will cause a call to **GUIRenderable::FillBuffer()**. Call this if element's contents and size didn't change (e.g. when its position, or depth changes). Batched GUI mesh is a mesh managed by the GUI system that consists of multiple GUI elements sharing the same material properties (used to improve rendering performance).
 - @b3d::GUIElement::MarkContentAsDirty() - Has the same effect as **GUIElement::MarkMeshAsDirty()**, but will also trigger a **GUIRenderable::UpdateRenderElements()** call so the GUI element mesh will be fully rebuilt. Call this when contents of the GUI element change, but not its size (e.g. changing a texture but it is the same dimensions as the old one.)
 - @b3d::GUIElement::MarkLayoutAsDirty() - Has the same effect as **GUIElement::MarkContentAsDirty()**, but will also cause the layout system to recalculate the element's position and size. This can be expensive as this will generally trigger layout updates for all siblings and children, potentially even parent GUI elements. Call this when element changes and no other dirty calls are appropriate. Normally you have to call this whenever the element's size changes.
 
~~~~~~~~~~~~~{.cpp}
class GUITexture : public GUIRenderable
{
	// ... remaining GUITexture implementation

	// Changes the look of the GUI element by fading it out when enabled
	void SetFadeOut(bool fadeOut)
	{
		if(fadeOut)
			mTintColor = Color(1.0f, 1.0f, 1.0f, 0.5f);
		else
			mTintColor = Color::kWhite;
		
		// Ensure the system redraws the mesh. Since neither the size or contents changed we can call the cheapest redraw method
		MarkMeshAsDirty();
	}
	
	Color mTintColor = Color::kWhite;
};
~~~~~~~~~~~~~
 
# Input
In order for the GUI element to accept input from the user you can implement any or all of the following methods:
 - @b3d::GUIInteractable::DoOnMouseEvent - Triggered when the mouse moves and/or is clicked. See @b3d::GUIMouseEventType for a list of events. Receives @b3d::GUIMouseEvent structure as input.
 - @b3d::GUIInteractable::DoOnTextInputEvent - Triggered when the user inputs some text on the keyboard, while the GUI element is in focus. Receives @b3d::GUITextInputEvent structure as input.
 - @b3d::GUIInteractable::DoOnVirtualButtonEvent - Triggered when a certain virtual button is pressed, while the GUI element is in focus. Receives @b3d::GUIVirtualButtonEvent structure as input.
 - @b3d::GUIInteractable::DoOnCommandEvent - Triggers when some kind of a specialized event happens, like losing/gaining focus, deleting text, selecting text, forcing redraw and similar. See @b3d::GUICommandEventType for an explanation of what each event type does. Receives @b3d::GUICommandEvent structure as input.
 
Each of the methods should return false if other GUI elements are allowed to receive the same event. If true is returned the GUI element will consume the event and it won't be visible to others.
 
~~~~~~~~~~~~~{.cpp}
class GUITexture : public GUIInteractable
{
	// ... remaining GUITexture implementation

	bool DoOnMouseEvent(const GUIMouseEvent& ev) override
	{
		// Fade out the texture when user mouses over the element
		if(ev.GetType() == GUIMouseEventType::MouseOver)
			SetFadeOut(true);
		else if(ev.GetType() == GUIMouseEventType::MouseOut)
			SetFadeOut(false);

		return false;
	}
};
~~~~~~~~~~~~~
