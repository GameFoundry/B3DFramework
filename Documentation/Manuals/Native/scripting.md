Scripting									{#scripting}
===============
[TOC]

Often when you extend the native portion of the engine in some way you might want to expose that functionality to the managed (i.e. scripting) code. This guide will show you how to create C++ objects that communicate with C# code and vice versa. 

Before we delve into the specifics of Banshee's scripting you should understand how the scripting system works in general. All C# script code is ran from the C++ part of the engine using the Mono runtime. Mono runtime allows you to communicate with C# code (and for the C# code to communicate with C++), query class/method/field information and pass data between the two languages.

# Scripting system #	{#scripting_a}

Because using Mono directly is complex (mostly due to its lack of documentation), Banshee provides a set of easy to use wrappers for almost all of Mono related functionality. 

BansheeMono is a plugin that wraps the functionality of the Mono runtime. The main entry point of the scripting system is the @ref BansheeEngine::MonoManager "MonoManager" class which allows you to start the runtime and load managed (script) assemblies. The most important method here is @ref BansheeEngine::MonoManager::loadAssembly "MonoManager::loadAssembly". It loads all the script code from the managed assembly at the provided path, and provides meta-data for the entire assembly through the returned @ref BansheeEngine::MonoAssembly "MonoAssembly" object. 

## MonoAssembly 	{#scripting_a_a}
@ref BansheeEngine::MonoAssembly "MonoAssembly" gives you access to all the script classes in an assembly. You can retrieve all clases using @ref BansheeEngine::MonoAssembly::getAllClasses "MonoAssembly::getAllClasses", or retrieve a specific one by calling @ref BansheeEngine::MonoAssembly::getClass(const String&, const String&) const "MonoAssembly::getClass(namespace, typename)". Both of these methods return a @ref BansheeEngine::MonoClass "MonoClass" object.

## MonoClass 	{#scripting_a_b}
@ref BansheeEngine::MonoClass "MonoClass" gives you access to all methods, fields, properties and attributes of a specific class. It also allows you to register "internal" methods. These methods allow the managed code to call C++ code, and we'll go into them later.

Classes also allow you to create object instances of their type. Use @ref BansheeEngine::MonoClass::createInstance "MonoClass::createInstance" to create a new object instance. All managed objects are referenced using a `MonoObject` type (more on that later), which is returned from the @ref BansheeEngine::MonoClass::createInstance "MonoClass:createInstance" call. When creating an instance you may choose whether to construct it or not, and to provide constructor signature if you need a specific one.

To retrieve a method from a class call @ref BansheeEngine::MonoClass::getMethod() "MonoClass::getMethod()", accepting a name (without parameter types) and a number of parameters. If your method is overloaded you can use @ref BansheeEngine::MonoClass::getMethodExact "MonoClass:getMethodExact()" which accepts a method name, and a comma separated list of parameter types. You may also use @ref BansheeEngine::MonoClass::getAllMethods "MonoClass::getAllMethods" to retrieve all methods in a class.

All the above methods return a @ref BansheeEngine::MonoMethod "MonoMethod" object.

## MonoMethod {#scripting_a_c}
This class provides information about about a managed method, as well as giving you multiple ways of invoking it (it allows you to call C# methods from C++).

To invoke a method you may use multiple approaches:
 - @ref BansheeEngine::MonoMethod::invoke "MonoMethod::invoke" - Calls the exact method on a specific managed object, with the provided parameters. We'll see how are managed objects referenced in native code later, as well as how passing data between C++ and managed code works.
 - @ref BansheeEngine::MonoMethod::invokeVirtual "MonoMethod::invokeVirtual" - Calls the method polymorphically. Determines the actual type of the provided managed object instance and calls an overriden method if available.
 - @ref BansheeEngine::MonoMethod::getThunk "MonoMethod::getThunk" - Returns a C++ function pointer accepting a managed object, zero or more parameters and an exception object. You can then call the function pointer like you would a C++ function. This is equivalent to @ref BansheeEngine::MonoMethod::invoke "MonoMethod::invoke" but is significantly faster. A helper method @ref BansheeEngine::MonoUtil::invokeThunk<T, Args> "MonoUtil::invokeThunk" is provided - it is suggested you use it instead of calling thunks manually (it handles exceptions internally).

When calling static methods you should provide a null value for the managed object instance.

## MonoField {#scripting_a_d}
Similar to methods, field information can be retrieved from a @ref BansheeEngine::MonoClass "MonoClass" object by calling @ref BansheeEngine::MonoClass::getField "MonoClass::getField" or @ref BansheeEngine::MonoClass::getAllFields "MonoClass::getAllFields". The returned value is a @ref BansheeEngine::MonoField "MonoField" which provides information about the field and allows you to retrieve and set values in the field using @ref BansheeEngine::MonoField::getValue "MonoField::getValue" / @ref BansheeEngine::MonoField::setValue "MonoField::setValue". This works similar to how methods are invoked and is explained in more detail later.

## MonoProperty {#scripting_a_e}
Properties are very similar to fields, retrieved from a @ref BansheeEngine::MonoClass "MonoClass" object by calling @ref BansheeEngine::MonoClass::getProperty "MonoClass::getProperty". The returned value is a @ref BansheeEngine::MonoProperty "MonoProperty" which provides information about the property and allows you to retrieve and set values on it. The main difference is that properties in C# can be indexed (like arrays) and therefore a two set of set/get methods are provided, one accepting an index and other one not. It's up to the user to know which one to call. The methods are @ref BansheeEngine::MonoProperty::get "MonoProperty::get" / @ref BansheeEngine::MonoProperty::set "MonoProperty::set" and @ref BansheeEngine::MonoProperty::getIndexed "MonoProperty::getIndexed" / @ref BansheeEngine::MonoProperty::setIndexed "MonoProperty::setIndexed".

## Attributes {#scripting_a_f}
Attributes provide data about a class, method or field provided at runtime, which usually allows such objects to be specialized in some regard. Attributes don't have their own wrapper, because they are esentially normal managed objects and you can work with them as such.

To retrieve a list of attributes from a class use @ref BansheeEngine::MonoClass::getAllAttributes() "MonoClass::getAllAttributes", which returns a list of @ref BansheeEngine::MonoClass "MonoClass" objects that identify the attribute types. To get the actual object instance of the attribute you may call @ref BansheeEngine::MonoClass::getAttribute "MonoClass::getAttribute" with the wanted attribute's @ref BansheeEngine::MonoClass "MonoClass". After that you can call methods, work with field values and other, same as you would with a normal managed object (described below).

Attributes can also be retrieved from a @ref BansheeEngine::MonoMethod "MonoMethod" using @ref BansheeEngine::MonoMethod::getAttribute "MonoMethod::getAttribute", or from @ref BansheeEngine::MonoField "MonoField" using @ref BansheeEngine::MonoField::getAttribute "MonoField::getAttribute".

## Managed objects {#scripting_a_g}
So far we have talked about invoking methods and retrieving field values, but we haven't yet explained how to access managed object instances in C++ code. All managed objects in Mono are represented by a `MonoObject`.

For example, when calling a non-static method the first parameter provided to @ref BansheeEngine::MonoMethod::invoke "MonoMethod::invoke" is a `MonoObject` pointer. Same goes for retrieving or setting values on fields, properties. Attributes are also a `MonoObject`.

Mono also provides two more specialized types of managed objects: `MonoArray` for managed arrays, and `MonoString` for managed strings, but they are both still a `MonoObject`.

Be aware that all managed objects are garbage collected. This means you should not keep a reference to them unless you are sure they are alive. Just having a pointer to a `MonoObject` will not keep the object alive and it may go out of scope as soon as the control returns to managed code. A good way to deal with this issue is:
 - Call a native method in the object's finalizer (`~MyObject()`) which will notify you when the object is no longer valid. Be aware that finalizer may be called after the object is unusable.
 - Require the user to manually destroy the object by calling a custom `Destroy` method or similar.
 - Force the garbage collector to keep the object alive by calling @ref BansheeEngine::MonoUtil::newGCHandle "MonoUtil::newGCHandle" which will return a handle to the object. The handle will keep the object alive until you release it by calling @ref BansheeEngine::MonoUtil::freeGCHandle "MonoUtil::freeGCHandle". Be aware if an assembly the object belongs to is unloaded all objects will be destroyed regardless of kept handles.
 
## Marshalling data {#scripting_a_h}
Mono does not perform automatic marshalling of data when calling managed code from C++ (or vice versa). This is important when calling methods, retrieving/setting field/property values, and responding to calls from managed code, because you need to know in what format to expect the data.

The rules are:
 - All primitive types are passed as is. e.g. an `int` in C# will be a 4 byte integer in C++, a `float` will be a float, a `bool` will be a bool.
 - All reference types (`class` in C#) are passed as a `MonoObject*`. Strings and arrays are handled specially, where strings are passed as `MonoString*`, and arrays as `MonoArray*`.
   - If a reference type parameter in a method in managed code is prefixed with an `out` modifier, then the received parameters are `MonoObject**`, `MonoString**`, `MonoArray**` and your method is expected to populate those values.
 - Structs (non-primitive value types, `struct` in C#) are provided as raw memory. Make sure that all structs in C# that require marshalling have a `[StructLayout(LayoutKind.Sequential)]` attribute, which ensures they have the same memory layout as C++ structs. This way you can just accept the raw C++ structure and read it with no additional conversion.
  - It is suggested you never pass structures by value, it is known to cause problems in Mono. Instead pass all structures by prefixing them with `ref` which will give you a pointer to the structure (e.g. `MyStruct*`). If you need to output a struct use the `out` modifier which you will give you a double pointer (e.g. `MyStruct**`).
  - In cases where it is not possible to avoid passing structures by value (e.g. when retrieving them from a field, use the @ref BansheeEngine::MonoField::getValueBoxed "MonoField::getValueBoxed" method instead @ref BansheeEngine::MonoField::getValue "MonoField::getValue", which will return a struct in the form of a `MonoObject`. You can then retrieve the raw struct value by calling @ref BansheeEngine::MonoUtil::unbox "MonoUtil::unbox".
  - Everything above applies only when managed code is calling C++. When calling into managed code from C++, all structs need to be boxed (i.e. converted to `MonoObject`). Use @ref BansheeEngine::MonoUtil::box "MonoUtil::box" to convert a C++ struct into a `MonoObject*`. See @ref BansheeEngine::ScriptVector3 "ScriptVector3" for an example implementation.
  
Banshee provides a helper code to assist with marshalling strings:
 - @ref BansheeEngine::MonoUtil::monoToWString "MonoUtil::monoToWString" / @ref BansheeEngine::MonoUtil::monoToString "MonoUtil::monoToString" - Converts a `MonoString*` to a native string
 - @ref BansheeEngine::MonoUtil::wstringToMono "MonoUtil::wstringToMono" / @ref BansheeEngine::MonoUtil::stringToMono "MonoUtil::stringToMono" - Converts a native string into a `MonoString*`

@ref BansheeEngine::ScriptArray "ScriptArray" is a helper class that allows you to construct new arrays and read managed arrays, easily. 

To create a new arrays call @ref BansheeEngine::ScriptArray::create "ScriptArray<Type>::create". Type can be a primitive type like `int`, `float`, a native string or a `Script*` object (more about `Script*` objects later). You can then fill the array by calling @ref BansheeEngine::ScriptArray::set "ScriptArray::set" and retrieve the managed `MonoArray*` by calling @ref BansheeEngine::ScriptArray::getInternal "ScriptArray::getInternal".

To more easily read existing arrays create a new @ref BansheeEngine::ScriptArray "ScriptArray" by providing it with a `MonoArray*` in the constructor. Then you can easily retrieve the size of the array using @ref BansheeEngine::ScriptArray::size() "ScriptArray::size()", and the value of its elements by calling @ref BansheeEngine::ScriptArray::get "ScriptArray::get<Type>"". 

## Internal methods {#scripting_a_i}
So far we have talked about calling managed code, and retrieving information about managed types, but we have yet to show how managed code calls C++ code. This is accomplished using native methods.

The first step is to define a stub method in managed code, like so:
~~~~~~~~~~~~~{.cs}
[MethodImpl(MethodImplOptions.InternalCall)]
private static extern float Internal_GetSomeValue(MyObject obj);
~~~~~~~~~~~~~
	
You then hook up this method with managed code by calling @ref BansheeEngine::MonoClass::addInternalCall "MonoClass::addInternalCall". In this specific case it would be:
~~~~~~~~~~~~~{.cpp}
myClass->addInternalCall("Internal_GetSomeValue", &myNativeFunction);
~~~~~~~~~~~~~

Assuming `myClass` is a @ref BansheeEngine::MonoClass "MonoClass" of the type that contains the stub method. After this call any call to the managed stub method will call the provided native function `myNativeFunction`. You should take care to properly handle parameter passing as described above.

Take a look at @ref BansheeEngine::ScriptGUISkin "ScriptGUISkin" implementation for a simple example of how exactly does this work. 

# Script objects {#scripting_b}
As you can see interaction between the two languages can get a bit cumbersome. For that reason Banshee implements a higher level system built on the functionality shown so far. It provides an universal interface all script objects must implement. It primarily ensures that native and managed code is always linked by keeping a pointer to each other's objects, as well as gracefully handling managed object destruction and handling assembly refresh (due to script hot-swap).

When exposing a class to the scripting interface you need to add two things:
 - A native interop object (C++)
 - Managed wrapper for the class (C#)

## Native interop object ## {#scripting_b_a}

All native interop objects implement the @ref BansheeEngine::ScriptObject "ScriptObject" interface. A basic implementation of such an interface can be:
~~~~~~~~~~~~~{.cpp}
class ScriptMyObject : public ScriptObject <ScriptMyObject>
{
public:
	SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "MyObject")

private:
	ScriptMyObject(MonoObject* instance);
	
	static void internal_CreateInstance(MonoObject* obj);
	static float internal_GetSomeValue(MonoObject* obj);
};
~~~~~~~~~~~~~	
	
All @ref BansheeEngine::ScriptObject "ScriptObjects" must begin with a @ref SCRIPT_OBJ macro. The macro accepts (in order): 
 - the name of the assembly (.dll) the managed script object is in, this is either `ENGINE_ASSEMBLY` or `EDITOR_ASSEMBLY`
 - the namespace the type is in
 - the name of the managed type

@ref SCRIPT_OBJ macro also defines a static `initRuntimeData()` method you need to implement (more on that later).

When constructing a @ref BansheeEngine::ScriptObject "ScriptObject" you must also provide a pointer to the managed object that wraps it (note the constructor). If the @ref BansheeEngine::ScriptObject "ScriptObject" is used for a class that is static then the constructor is of no consequence as the @ref BansheeEngine::ScriptObject "ScriptObject" itself never needs to be instantiated (all of its methods will be static). 
 
The two last method definitions in the example are called from C# (via an internal call, see the section about internal methods earlier).

### initRuntimeData ### {#scripting_b_a_a}
`initRuntimeData` is a static method that every @ref BansheeEngine::ScriptObject "ScriptObject" needs to implement. It takes care of hooking up managed internal methods to C++ functions. It gets called automatically whenever the assembly containing the related managed class is loaded. 

Every @ref BansheeEngine::ScriptObject "ScriptObject" provides a static @ref BansheeEngine::ScriptObject<Type, Base>::getMetaData "metaData" structure you can use for retrieving the @ref BansheeEngine::MonoClass "MonoClass" of the related managed class. You can use that @ref BansheeEngine::MonoClass "MonoClass" to register internal methods to it (as described earlier). For example a basic `initRuntimeData()` might look like so:
~~~~~~~~~~~~~{.cpp}
void ScriptMyObject::initRuntimeData()
{
	metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptFont::internal_CreateInstance);
	metaData.scriptClass->addInternalCall("Internal_GetSomeValue", &ScriptFont::internal_GetSomeValue);
}
~~~~~~~~~~~~~

`initRuntimeData` is also a good spot to retrieve @ref BansheeEngine::MonoMethod "MonoMethods" (or thunks) for managed methods that needed to be called by the script interop object, if any.

### Creating script object instances ### {#scripting_b_a_b}	
If your class is not static you will need to eventually create an instance of the script object. This can be done either from C# or C++, depending on what is needed. For example script interop objects for GUI will be created from managed code because user can add GUI elements himself, but a resource like @ref BansheeEngine::Font "Font" will have its script interop object (and managed instance) created purely from C++ because such an object cannot be created directly in managed code.

For the first case you should set up an internal method that accepts the managed object instance, and is called in the managed constructor (`internal_CreateInstance` in the above example). This way the method gets called whenever the managed object gets created and you can create the related script interop object. A simple implementation would look like so:
~~~~~~~~~~~~~{.cpp}
void ScriptMyObject::internal_createInstance(MonoObject* obj)
{
	bs_new<ScriptMyObject>(obj);
}
~~~~~~~~~~~~~
	
Note that you don't actually need to store the created object anywhere. The @ref BansheeEngine::ScriptObject "ScriptObject" constructor ensures that the pointer to the script interop object is stored in the managed object.

For the second case where you want to create the interop object from C++ you can create a static `create()` method like so:
~~~~~~~~~~~~~{.cpp}
MonoObject* ScriptMyObject::create()
{
	MonoObject* managedObj = metaData.scriptClass->createInstance();
	bs_new<ScriptMyObject>(managedObj);
	
	return managedObj;
}
~~~~~~~~~~~~~
	
In this case the method calls a parameterless constructor but you may specify parameters as needed.

If you have a `MonoObject*` but need to retrieve its @ref BansheeEngine::ScriptObject "ScriptObject" use @ref BansheeEngine::ScriptObject::toNative(MonoObject*) "toNative(MonoObject*)" static method. Within the interop object instance you can use @ref BansheeEngine::ScriptObjectBase::getManagedInstance() "ScriptObject::getManagedInstance()" to retrieve the managed object.

### Destroying script object instances ### {#scripting_b_a_c}
When the managed object is destroyed (e.g. goes out of scope and gets garbage collected) the system will automatically take care of freeing the related ScriptObject. If you need to add onto or replace that functionality you can override @ref BansheeEngine::ScriptObjectBase::_onManagedInstanceDeleted "ScriptObject::_onManagedInstanceDeleted" method.

## Managed wrapper object ## {#scripting_b_b}
Creating the script interop object is one half of the job done. You also need to create the managed counterpart, however that is significantly simpler.

Every managed script object must implement the ScriptObject interface. For example a C# version of the class we're using in this example would look like:
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
	}
}
~~~~~~~~~~~~~

That's all that needs to be done. You can now create the object in C# and use its property to retrieve the value from C++ code. All managed `ScriptObjects` provide a `GetCachedPtr` method which returns an `IntPtr` which points to the script interop object described in previous sections.

## Assembly refresh ##
What has been shown so far is enough to create a class exposed to script, however object of such a class will not survive assembly refresh. Assembly refresh happens when scripts are recompiled and managed assemblies are unloaded and reloaded. This is something that generally happens only in the editor.

If you don't care about your object surviving the refresh, you do not need to implement what is described here. For example GUI elements don't persist refresh, because they're just rebuilt from the managed code every time the refresh happens. However objects like resources, scene objects and components are persistent - we don't wish to reload the entire scene and all resources every time assembly refresh happens.

A persistent script object need to inherit a variation of @ref BansheeEngine::ScriptObject "ScriptObject" like so:
~~~~~~~~~~~~~{.cpp}
class MyScriptObject : public ScriptObject<MyScriptObject, PersistentScriptObjectBase>
~~~~~~~~~~~~~	
	
This ensures that your object is treated properly during assembly refresh. Persistent object then needs to handle four different actions, represented by overrideable methods. These methods are called in order specified, during assembly refresh.
 - @ref BansheeEngine::ScriptObjectBase::beginRefresh() "ScriptObject::beginRefresh()" - Called just before the refresh starts. The object is still alive here and you can perform needed actions (e.g. saving managed object's contents).
 - @ref BansheeEngine::ScriptObjectBase::_onManagedInstanceDeleted "ScriptObject::_onManagedInstanceDeleted()" - Called after assembly unload happened and the managed object was destroyed. You should override this to prevent the @ref BansheeEngine::ScriptObject "ScriptObject" itself from being deleted if the assembly refresh is in progress. If assembly refresh is not in progress this method should delete the @ref BansheeEngine::ScriptObject "ScriptObject" as normal because it likely got called due to standard reasons (managed object went out of scope).
 - @ref BansheeEngine::ScriptObject::_createManagedInstance "ScriptObject::_createManagedInstance()" - Creates the managed instance after new assemblies are loaded. You should override this if your managed class is constructed using a constructor with parameters. By default this will call @ref BansheeEngine::MonoClass::createInstance "MonoClass::createInstance()" using the parameterless constructor.
 - @ref BansheeEngine::ScriptObjectBase::endRefresh() "ScriptObject::endRefresh()" - Called after all assemblies are loaded, and after all script interop objects were either destroyed (non-persistent) or had their managed instances created (persistent). If you stored any data during @ref BansheeEngine::ScriptObjectBase::beginRefresh() "ScriptObject::beginRefresh()", you should restore it here.
 
See @ref BansheeEngine::ScriptSceneObject "ScriptSceneObject" and its base class @ref BansheeEngine::ScriptGameObjectBase "ScriptGameObjectBase" for example implementations of these methods.

## Deriving from ScriptObject ##
Sometimes script objects are polymorphic. For example a `GUIElement` is derived from `ScriptObject` in managed code, and `GUIButton` is derived from `GUIElement`, however they both have script interop objects of their own.

Due to the nature of how our script interop objects are defined we cannot follow the same simple chain of inheritance in C++ code. For example class definition script interop object for `GUIElement` would be:
~~~~~~~~~~~~~{.cpp}
class ScriptGUIElement : public ScriptObject<ScriptGUIElement>
{
public:
	SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUIElement")
...
}
~~~~~~~~~~~~~	
	
But what would it be for `GUIButton`? It also needs to implement @ref BansheeEngine::ScriptObject "ScriptObject" with its own @ref SCRIPT_OBJ macro so we cannot just inherit from `ScriptGUIElement` directly as it would clash.

The solution is to create a third class that will serve as a base for both. This third class will be a base class for @ref BansheeEngine::ScriptObject "ScriptObject" (its second template parameter allows us to override its default `ScriptObjectBase` base class). The third class will need to inherit @ref BansheeEngine::ScriptObjectBase "ScriptObjectBase" and can implement any functionality common to all GUI elements (e.g. it might store a pointer to a native `GUIElement*`).

Then we can define script interop object for GUI element as:
~~~~~~~~~~~~~{.cpp}
class ScriptGUIElement : public ScriptObject<ScriptGUIElement, ScriptGUIElementBase>
~~~~~~~~~~~~~

Where `ScriptGUIElementBase` is our third (common) class. Interop object for `GUIButton` would then be:
~~~~~~~~~~~~~{.cpp}
class ScriptGUIButton : public ScriptObject<ScriptGUIButton, ScriptGUIElementBase>
~~~~~~~~~~~~~	
	
This ensures that all GUI elements can now be accessed through the common `ScriptGUIElementBase` interface. Which is important if `GUIElement` provides some internal method calls shared between all GUI element types, otherwise we wouldn't know what to cast the interop object held by its managed object to.

See @ref BansheeEngine::ScriptGUIElement "ScriptGUIElement" and @ref BansheeEngine::ScriptGUIButton "ScriptGUIButton" for an example.
