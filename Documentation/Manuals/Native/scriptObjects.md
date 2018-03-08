Script objects									{#scriptObjects}
===============
[TOC]

What we have shown in the previous manual is enough to expose an object to C# and communicate with it. However bs::f provides another API built on top of that functionality in the form of script objects. This API handles some of the boilerplate code required for exposing an object to C#, provides a common interface all script objects need to implement, handles assembly refresh (due to script hot-swap) and gracefully handles managed object lifetime and destruction.

To implement the script object interface for a particular type you need two classes:
 - A native interop class (C++)
 - Managed wrapper class for the type (C#)

# Native interop class {#scriptObjects_a}
This class is intended as a wrapper for the C++ class you're exposing to the scripting API. It will contain all the code needed for C++/C# interop. All such native interop objects must implement the @ref bs::ScriptObject<T> "ScriptObject<T>" interface. The template parameter should be the type of the interop class itself. The implementation of the class must begin with @ref SCRIPT_OBJ macro. The macro accepts (in order): 
 - the name of the assembly (.dll) the managed wrapper class is in, this is usually `ENGINE_ASSEMBLY` or `EDITOR_ASSEMBLY`
 - the namespace the managed wrapper class
 - the name of the managed wrapper class
 
**ScriptObject** constructor accepts a **MonoObject** which must contain the managed instance of the object and should be provided when the script object is created. If a script object is wrapping a static class, then the constructor is of no relevance as the script object will never be constructed (but still needs to be present).

~~~~~~~~~~~~~{.cpp}
class ScriptMyObject : public ScriptObject <ScriptMyObject>
{
public:
	SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "MyObject")

	ScriptMyObject(MonoObject* instance)
		:ScriptObject(instance)
	{ }
};
~~~~~~~~~~~~~	

**SCRIPT_OBJ** macro defines a static **initRuntimeData()** method you need to implement. In this method you want to take care of hooking up managed internal methods to C++ functions. It gets called automatically on startup and whenever the assembly containing the related managed class is loaded. 

Every **ScriptObject** provides a static @ref bs::ScriptObject<Type, Base>::getMetaData "metaData" structure you can use for retrieving the **MonoClass** of the related managed class. You can use that **MonoClass** to register internal methods to it (as described earlier). 
~~~~~~~~~~~~~{.cpp}
class ScriptMyObject : public ScriptObject <ScriptMyObject>
{
	// Other code ...
	
	static void internal_CreateInstance(MonoObject* obj);
	static float internal_GetSomeValue(MonoObject* obj);
	static void internal_SetSomeObject(MonoObject* obj);
};

void ScriptMyObject::initRuntimeData()
{
	metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptMyObject::internal_CreateInstance);
	metaData.scriptClass->addInternalCall("Internal_GetSomeValue", &ScriptMyObject::internal_GetSomeValue);
	metaData.scriptClass->addInternalCall("Internal_SetSomeObject", &ScriptMyObject::internal_SetSomeObject);
}
~~~~~~~~~~~~~

**initRuntimeData()** is also a good spot to retrieve **MonoMethod**%s (or thunks) for managed methods that needed to be called by the script interop object, if any.

## Creating script object instances {#scriptObjects_a_a}	
If your class is not static you will need to eventually create an instance of the script object. This can be done either from C# or C++, depending on what is needed. For example script interop objects for GUI will be created from managed code because user can add GUI elements himself, but a resource like **Font** will have its script interop object (and managed instance) created purely from C++ because such an object cannot be created directly in managed code.

For the first case you should set up an internal method that accepts the managed object instance, and is called in the managed constructor. This way the method gets called whenever the managed object gets created and you can create the related script interop object.
~~~~~~~~~~~~~{.cpp}
// This method should be called from the managed object's constructor, with "this" object as the parameter
void ScriptMyObject::internal_CreateInstance(MonoObject* obj)
{
	// No need to store the created value anywhere, the system will clean it up automatically when the managed object is destroyed
	bs_new<ScriptMyObject>(obj);
}
~~~~~~~~~~~~~
	
For the second case where you want to create the interop object from C++ you can create a static *create()* method like so:
~~~~~~~~~~~~~{.cpp}
MonoObject* ScriptMyObject::create()
{
	MonoObject* managedObj = metaData.scriptClass->createInstance();
	bs_new<ScriptMyObject>(managedObj);
	
	return managedObj;
}
~~~~~~~~~~~~~
	
If you ever receive a **MonoObject** of the type you know that has a **ScriptObject** implemented, you can retrieve it by calling @ref bs::ScriptObject::toNative(MonoObject*) "ScriptObject::toNative(MonoObject*)" static method. 
~~~~~~~~~~~~~{.cpp}
void ScriptMyObject::internal_SetSomeObject(MonoObject* obj)
{
	ScriptSomeObject* someObj = ScriptSomeObject::toNative(obj);
}
~~~~~~~~~~~~~

## Destroying script object instances ### {#scripting_a_b}
When the managed object is destroyed (e.g. goes out of scope and gets garbage collected) the system will automatically take care of freeing the related **ScriptObject**. If you need to add onto or replace that functionality you can override @ref bs::ScriptObjectBase::_onManagedInstanceDeleted "ScriptObject::_onManagedInstanceDeleted()" method. This is useful if you need to perform some additional cleanup.

# Managed wrapper object ## {#scripting_b}
Creating the script interop class is one half of the job done. You also need to create the managed counterpart, however that is significantly simpler.

Every managed script object must implement the **ScriptObject** interface. For example a C# version of the class we're using in this example would look like:
~~~~~~~~~~~~~{.cs}
namespace BansheeEngine
{
	public class MyObject : ScriptObject
	{
		public MyObject()
		{
			Internal_CreateInstance(this)
		}
		
		public float SomeValue
		{
			get { return Internal_GetSomeValue(this); }
		}
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_CreateInstance(MyObject obj);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetSomeValue(MyObject obj);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void internal_SetSomeObject(SomeObject obj);
	}
}
~~~~~~~~~~~~~

That's all that needs to be done. You can now create the object in C# and use its property to retrieve the value from C++ code.

All managed **ScriptObject**%s provide a **ScriptObject::GetCachedPtr()** method which returns an **IntPtr** which points to the script interop object described in previous sections. This can be used for passing a reference to the **ScriptObject** directly, rather than passing a **MonoObject** which then needs to be converted to **ScriptObject**.

For example, if we change the *Internal_GetSomeValue()* method in the example above to use this approach, it would look like so:
~~~~~~~~~~~~~{.cs}
namespace BansheeEngine
{
	public float SomeValue
	{
		get { return Internal_GetSomeValue(GetCachedPtr()); }
	}

	public class MyObject : ScriptObject
	{
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetSomeValue(IntPtr obj);
	}
}
~~~~~~~~~~~~~

And the corresponding C++ function would then accept the script object directly:
~~~~~~~~~~~~~{.cpp}
float ScriptMyObject::internal_GetSomeValue(ScriptMyObject* obj)
{
	// No need to convert from MonoObject to ScriptMyObject
}
~~~~~~~~~~~~~

# Assembly refresh {#scripting_c}
Assembly refresh is the process that happens when managed code is recompiled and scripts need to be reloaded. This is primarily used in Banshee 3D editor to hot-reload the scripts while the editor is still running. When assembly refresh happens all managed objects are effectively destroyed.

By default any script objects of such managed objects are destroyed as well. In many cases this is okay, for example GUI elements don't persist refresh, because they're just rebuilt from the managed code every time the refresh happens. However objects like resources, scene objects and components are persistent - we don't wish to reload the entire scene and all resources every time assembly refresh happens.

In order to make your script objects persist through refresh you need to inherit from a variation of **ScriptObject** that as its second template parameter takes a @ref bs::PersistentScriptObjectBase "PersistentScriptObjectBase".

~~~~~~~~~~~~~{.cpp}
class MyScriptObject : public ScriptObject<MyScriptObject, PersistentScriptObjectBase>
{
	...
};
~~~~~~~~~~~~~	
	
This ensures that your object is treated properly during assembly refresh. Persistent object then needs to handle four different actions, represented by overrideable methods. These methods are called in order specified, during assembly refresh.
 - @ref bs::ScriptObjectBase::beginRefresh() "ScriptObject::beginRefresh()" - Called just before the refresh starts. The object is still alive here and you can use this time to save the contents of the managed object before it is destroyed.
 - @ref bs::ScriptObjectBase::_onManagedInstanceDeleted "ScriptObject::_onManagedInstanceDeleted()" - Called after assembly unload happened and the managed object was destroyed. You should override this to prevent the **ScriptObject** itself from being deleted if the assembly refresh is in progress. If assembly refresh is not in progress this method should delete the **ScriptObject** as it likely got called due to standard reasons (managed object went out of scope).
 - @ref bs::ScriptObject::_createManagedInstance "ScriptObject::_createManagedInstance()" - Creates the managed instance after new assemblies are loaded. You should override this if your managed class is constructed using a constructor with parameters. By default this will call **MonoClass::createInstance()** using the parameterless constructor.
 - @ref bs::ScriptObjectBase::endRefresh() "ScriptObject::endRefresh()" - Called after all assemblies are loaded, and after all script interop objects were either destroyed (non-persistent) or had their managed instances re-created (persistent). If you stored any data during **ScriptObject::beginRefresh()**, you should restore it here.
 
# Deriving from ScriptObject {#scripting_d}
Sometimes script objects are polymorphic. For example a **GUIElement** is derived from **ScriptObject** in managed code, and **GUIButton** is derived from **GUIElement**, however they both have script interop objects of their own.

Due to the nature of how our script interop objects are defined we cannot follow the same simple chain of inheritance in C++ code. For example class definition for script interop object for **GUIElement** would be:
~~~~~~~~~~~~~{.cpp}
class ScriptGUIElement : public ScriptObject<ScriptGUIElement>
{
public:
	SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUIElement")
...
}
~~~~~~~~~~~~~	
	
But what would it be for **GUIButton**? It also needs to implement **ScriptObject** with its own **SCRIPT_OBJ** macro so we cannot just inherit from **ScriptGUIElement** directly as it would clash.

The solution is to create a third class that will serve as a base for both. This third class will be a base class for **ScriptObject** (its second template parameter allows us to override its default **ScriptObjectBase** base class). The third class will need to inherit @ref bs::ScriptObjectBase "ScriptObjectBase" and can implement any functionality common to all GUI elements (e.g. it might store a pointer to a native **GUIElement**).

Provided our common base class is defined as such:
~~~~~~~~~~~~~{.cpp}
class ScriptGUIElementBase : public ScriptObjectBase
{
	// Functionality common to all GUI elements
};
~~~~~~~~~~~~~

Then we can define script interop object for **GUIElement** as:
~~~~~~~~~~~~~{.cpp}
class ScriptGUIElement : public ScriptObject<ScriptGUIElement, ScriptGUIElementBase>
{
public:
	SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUIElement")
...
};
~~~~~~~~~~~~~

And interop object for **GUIButton** would then be:
~~~~~~~~~~~~~{.cpp}
class ScriptGUIButton : public ScriptObject<ScriptGUIButton, ScriptGUIElementBase>
{
public:
	SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUIButton")
...
};
~~~~~~~~~~~~~	
	
This ensures that all GUI elements can now be accessed through the common **ScriptGUIElementBase** interface. Which is important if **GUIElement** provides some internal method calls shared between all GUI element types, otherwise we wouldn't know what to cast the interop object held by its managed object to.