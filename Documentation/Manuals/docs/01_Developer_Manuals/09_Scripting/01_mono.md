---
title: Interacting with the script runtime
---

In the previous chapter we talked about how to expose a C++ type to the scripting API by using the script binding generator tool. This tool ensures you can generate script bindings easily, but in some cases it is not robust enough and you must interact with the scripting API manually. This manual will explain how to interact with the scripting API and expose C++ code to it manually (without the script binding generator tool). 

You can use manual generation to achieve everything as with the script binding generator tool (in fact the tool uses the same API as we'll describe in this manual), plus a lot more. It is preferred you use automatic generation whenever possible, but when working with lower level systems that closely interact with the scripting system (like serialization, script compilation, etc.) you need more direct access.

All C# script code is executed using the Mono runtime. Mono runtime allows you to communicate with C# code and vice-versa by invoking methods and querying class/method/field information. In this section we'll focus on how to interact with Mono (and therefore the C# runtime).

# MonoManager
bsfMono is a plugin that wraps the functionality of the Mono runtime. The main entry point of the scripting system is the @bs::MonoManager class which allows you to start the runtime and load managed (script) assemblies. The most important method here is @bs::MonoManager::loadAssembly. It loads all the script code from the managed assembly (.dll) at the provided path, and provides meta-data for the entire assembly through the returned @bs::MonoAssembly object. 

~~~~~~~~~~~~~{.cpp}
// Loads the MyManagedAssembly.dll
MonoAssembly& assembly = MonoManager::instance().loadAssembly("Path/To/Assembly", "MyManagedAssembly");
~~~~~~~~~~~~~ 

You can retrieve a previously loaded assembly by calling @bs::MonoManager::getAssembly.

~~~~~~~~~~~~~{.cpp}
// Retrieve the MonoAssembly to a previously loaded assembly
MonoAssembly* assembly = MonoManager::instance().getAssembly("MyManagedAssembly");
~~~~~~~~~~~~~ 

# MonoAssembly
**MonoAssembly** gives you access to all the script classes in an assembly. You can retrieve all clases using @bs::MonoAssembly::getAllClasses, or retrieve a specific one by calling @ref bs::MonoAssembly::getClass(const String&, const String&) const "MonoAssembly::getClass(const String& namespace, const String& typename)". Both of these methods return a @bs::MonoClass object.

~~~~~~~~~~~~~{.cpp}
// Retrieve information about a C# class MyNamespace::MyClass
MonoClass* klass = assembly->getClass("MyNamespace", "MyClass");
~~~~~~~~~~~~~ 

# MonoClass
**MonoClass** gives you access to all methods, fields, properties and attributes of a specific class. It also allows you to register "internal" methods. These methods allow the managed code to call C++ code, and we'll go into them later.

Classes also allow you to create object instances of their type. Use @bs::MonoClass::createInstance to create a new object instance. This method returns a **MonoObject** instance, which is a C++ representation of the C# object, but more on them later. When creating an instance you may choose whether to construct it or not, and to provide constructor signature if you need a specific one.

~~~~~~~~~~~~~{.cpp}
// Create a new instance of the managed class using the parameterless constructor
MonoObject* instance = klass->createInstance();

// Create a new instance of the managed class using parameter count to differentiate between different constructors
UINT32 paramA = 3;
UINT32 paramB = 10;
bool paramC = false;

void* params[3] = { &paramA, &paramB, &paramC };

//// Invoke a constructor accepting three parameters (Behaviour undefined if there are multiple)
MonoObject* instance2 = klass->createInstance(params, 3);

// Create a new instance of the managed class using a specific constructor signature
MonoObject* instance3 = klass->createInstance("int,int,bool", params);
~~~~~~~~~~~~~

When passing arguments to constructors (and methods in general) you need to place the correct number of parameters in a array of *void* pointers, which you then pass to the invoking method. We'll talk more about how to pass arguments to methods later.

To retrieve a method from a class call @bs::MonoClass::getMethod(), accepting a name (without parameter types) and a number of parameters. If your method is overloaded you can use @bs::MonoClass::getMethodExact which accepts a method name, and a comma separated list of parameter types. You may also use @bs::MonoClass::getAllMethods to retrieve all methods in a class. All three of these methods return a @bs::MonoMethod object.

~~~~~~~~~~~~~{.cpp}
// Get method on the class named "MyMethod", accepting zero parameters
MonoMethod* myMethod = klass->getMethod("MyMethod", 0);

// Get method on the class named "MyMethod" with a specific signature
MonoMethod* myMethod2 = klass->getMethodExact("MyMethod", "single,int");
~~~~~~~~~~~~~

# MonoMethod
**MonoMethod** class provides information about about a managed method, as well as giving you multiple ways of invoking it (i.e. calling C# methods from C++).

To invoke a method you may use multiple approaches:
 - @bs::MonoMethod::invoke - Invokes the exact method to exact type it was retrieved from.
 - @bs::MonoMethod::invokeVirtual - Invokes the method polymorphically, meaning it determines the actual type of the provided managed object instance and calls an overriden method if available.
 - @bs::MonoMethod::getThunk - Returns a C++ function pointer that can be used for invoking the method, same as you would any C++ function. This is equivalent to **MonoMethod::invoke()** but is significantly faster. A helper method @ref bs::MonoUtil::invokeThunk<T, Args> "MonoUtil::invokeThunk()" is provided - it is suggested you use it instead of calling thunks manually  because it handles exceptions internally.

All method invocation types follow a similar format:
 - First parameter is always a **MonoObject** which corresponds to the instance of the class to execute the method on. If a method is static this should be null.
 - A list of parameters in the form of an array of *void* pointers, except for thunks which accept parameters as a normal function call.
 - A return value of the type MonoObject*. Non-class types like *int* or *float* will be boxed into objects and must be unboxed in order to retrieve their values. More about boxing/unboxing later.
 - Thunks always output a pointer to **MonoException** as their last parameter. You do not need to handle this manually, but you need to be aware it exists when defining the function pointer signature.

~~~~~~~~~~~~~{.cpp}
// Invoke MyMethod overload with no parameters, on the class instance we created earlier
myMethod->invoke(instance, nullptr);

// Invoke MyMethod overload with float and int parameters, on the class instance we created earlier
float paramA = 1.5f;
UINT32 paramB = 10;

void* params[2] = { &paramA, &paramB };
myMethod2->invoke(instance, params);

// Invoke some static method with no parameters
MonoMethod* someStaticMethod = ...;
someStaticMethod->invoke(nullptr, nullptr);

// Get a method thunk for MyMethod overload with two parameters, and invoke it
typedef void(__stdcall *MyMethodThunkDef)(float, UINT32, MonoException**);
MyMethodThunkDef myMethodThunk = (MyMethodThunkDef)myMethod2->getThunk();

MonoUtil::invokeThunk(myMethodThunk, 1.5f, 10);
~~~~~~~~~~~~~
 
# MonoField
Similar to methods, field information can be retrieved from a **MonoClass** object by calling @bs::MonoClass::getField or @bs::MonoClass::getAllFields. The returned value is a @bs::MonoField which provides information about the field and allows you to retrieve and set values in the field using @bs::MonoField::get / @bs::MonoField::set. 

~~~~~~~~~~~~~{.cpp}
// Read field value from a specific object instance
MonoField* myField = klass->getField("myField");
int myFieldValue;
myField->get(instance, &myFieldValue);

// Set a static field value
MonoField* myStaticField = klass->getField("myStaticField");
int newStaticFieldValue = 10;
myStaticField->set(nullptr, &newStaticFieldValue)
~~~~~~~~~~~~~

Field values are represented by their raw types if they are value types (*int*, *float* or *struct* types in C#), and as **MonoObject** pointer for reference types.

# MonoProperty
Properties are very similar to fields, retrieved from a **MonoClass** object by calling @bs::MonoClass::getProperty. The returned value is a @bs::MonoProperty which provides information about the property and allows you to retrieve and set values on it. The main difference is that properties in C# can be indexed (like arrays) and therefore two sets of set/get methods are provided, one accepting an index and other one not. It's up to the user to know which one to call. The methods are @bs::MonoProperty::get / @bs::MonoProperty::set and @bs::MonoProperty::getIndexed / @bs::MonoProperty::setIndexed.

~~~~~~~~~~~~~{.cpp}
// Read property value from a specific object instance
MonoProperty* myProperty = klass->getProperty("myProperty");
int myPropertyValue;
myProperty->get(instance, &myPropertyValue);

// Set a static property value
MonoProperty* myStaticProperty = klass->getProperty("myStaticProperty");
int newStaticPropertyValue = 10;
myStaticProperty->set(nullptr, &newStaticPropertyValue)

// Read property value from an indexed property
MonoProperty* myIndexedProperty = klass->getProperty("myIndexedProperty");
MonoObject* returnVal = myProperty->getIndexed(instance, 5);
~~~~~~~~~~~~~

Note that indexed properties always return a boxed object in the form of **MonoObject**, whether the object is a value or reference type.

# Attributes
Attributes provide data about a class, method or field provided at runtime, which usually allows such objects to be specialized in some regard. Attributes don't have their own wrapper, because they are esentially normal managed objects and you can work with them as such.

To retrieve a list of attributes from a class use @bs::MonoClass::getAllAttributes(), which returns a list of **MonoClass** objects that identify the attribute types. To get the actual object instance of the attribute you may call @bs::MonoClass::getAttribute with the wanted attribute's **MonoClass**. After that you can call methods, work with field values and similar, same as you would with a normal managed object.

Attributes can also be retrieved from a **MonoMethod** by using @bs::MonoMethod::getAttribute, or from **MonoField** by using @bs::MonoField::getAttribute.

~~~~~~~~~~~~~{.cpp}
// Retrieve class of the attribute, same as for normal classes
MonoClass* attributeClass = ...;

// Check if our class has this attribute
MonoObject* attributeObj = klass->getAttribute(attributeClass);
if(attributeObj != nullptr)
{
	// Class has the attribute. This can be enough information or we can choose to read attribute fields same as described above.
}
~~~~~~~~~~~~~

# Managed objects
All objects (more specifically *class*%es) in C# are represented as **MonoObject** in C++, as we already mentioned. There are also two more specialized types of managed objects: **MonoArray** for managed arrays, and **MonoString** for managed strings.

Be aware that all managed objects are garbage collected. This means you should not keep a reference to them in C++ code unless you are sure they are alive. Just having a pointer to a **MonoObject** will not keep the object alive and it may go out of scope as soon as the control returns to managed code. A good way to deal with this issue is:
 - Call a C++ method in the object's finalizer (`~MyObject()`) which will notify you when the object is no longer valid. Be aware that finalizer may be called after the object is unusable.
 - Require the user to manually destroy the object by calling a custom **Destroy** method or similar. At which point you would notify the C++ code that the object is destroyed.
 - Force the garbage collector to keep the object alive by calling @bs::MonoUtil::newGCHandle which will return a handle to the object. The handle will keep the object alive until you release it by calling @bs::MonoUtil::freeGCHandle. Be aware if an assembly the object belongs to is unloaded all objects will be destroyed regardless of kept handles.
 
~~~~~~~~~~~~~{.cpp}
// Create to retrieve instance to some managed object
MonoObject* instance = ...;
 
UINT32 handle = MonoUtil::newGCHandle(instance);
// We can now safely return control to managed code, without having to worry about the object being garbage collected
 
// At some point you must release the handle
MonoUtil::freeGCHandle(handle);
~~~~~~~~~~~~~ 
 
# Marshalling data
We have shown the basics of how to call methods and send them arguments, as well as receive return values. However there is more to it, as there is a specific set of rules of how values must be passed between C++/C#. These rules depend on the type of the value passed:
 - All primitive types are passed as is. e.g. an *int* in C# will be a 4 byte integer in C++, a *float* will be a float, a *bool* will be a bool.
 - All reference types (*class* in C#) are passed as a pointer to a **MonoObject**. Strings and arrays are handled specially, where strings are passed as pointers to **MonoString**, and arrays as pointers to **MonoArray**.
   - If a reference type parameter in a method in managed code is prefixed with an *out* modifier, then the received parameters are a double pointer to **MonoObject**, **MonoString** and **MonoArray**.
 - Structs (non-primitive value types, **struct** in C#) are provided as raw memory. Make sure that all structs in C# that require marshalling have a `[StructLayout(LayoutKind.Sequential)]` attribute, which ensures they have the same memory layout as C++ structs. This way you can just accept the raw C++ structure and read it with no additional conversion.
  - It is suggested you never pass structures by value, it is known to cause problems in Mono. Instead pass all structures by prefixing them with *ref* which will give you a pointer to the structure (e.g. `MyStruct*`). If you need to output a struct use the *out* modifier which you will give you a double pointer (e.g. `MyStruct**`).
  - In cases where it is not possible to avoid passing structures by value (e.g. when retrieving them from a field, use the @bs::MonoField::getBoxed method instead **MonoField::get**, which will return a struct in the form of a **MonoObject**.
  - Everything above applies only when managed code is calling C++. When doing the opposite, i.e. calling into managed code from C++, all structs need to be boxed (i.e. converted to **MonoObject**). 

## Boxing / Unboxing
Boxing is the process of "wrapping" a value-type (e.g. *int*, *float*, custom *struct* type) in a reference, so it can be represented by a **MonoObject**. Normally value types are passed around directly by value or pointer, but sometimes it is necessary to box them. When you receive a boxed value you can unbox it to its original state by calling @bs::MonoUtil::unbox.

~~~~~~~~~~~~~{.cpp}
// Retrieve some property, assume its static, indexed and is of type int
MonoProperty* property = ...;

// Read a value from the property
MonoObject* boxedValue = property->getIndexed(nullptr, 0);

// Unbox the object back into an integer
UINT32 value = *(UINT32*)MonoUtil::unbox(boxedValue);
~~~~~~~~~~~~~ 

Sometimes you have an unboxed value but C# requires an object. In that case you can box a value by calling @bs::MonoUtil::box.
~~~~~~~~~~~~~{.cpp}
int value = 5;
MonoObject* boxedValue = MonoUtil::box(MonoUtil::getUINT32Class(), &value);
~~~~~~~~~~~~~ 

Note that the first parameter of **MonoUtil::box()** is a **MonoClass** of the type you want to box. If this is a custom type you can retrieve it as we described above. If it is a builtin type you can check @bs::MonoUtil methods like @bs::MonoUtil::getUINT32Class or @bs::MonoUtil::getFloatClass().
  
## Strings
Banshee provides a helper code to assist with marshalling strings:
 - @bs::MonoUtil::monoToWString / @bs::MonoUtil::monoToString - Converts a **MonoString** to a native string
 - @bs::MonoUtil::wstringToMono / @bs::MonoUtil::stringToMono - Converts a native string into a **MonoString**

~~~~~~~~~~~~~{.cpp}
// Convert a native string to managed and back
MonoString* monoString = MonoUtil::stringToMono("My string");
String nativeString = MonoUtil::monoToString(monoString);
~~~~~~~~~~~~~ 

## Arrays
@bs::ScriptArray is a helper class that allows you to construct new arrays and read managed arrays easily. 

To create a new arrays call @bs::ScriptArray::create. Type can be a primitive type like *int*, *float*, a string or a `Script*` object (more about `Script*` objects later). You can then fill the array by calling @bs::ScriptArray::set and retrieve the managed **MonoArray** by calling @bs::ScriptArray::getInternal.

~~~~~~~~~~~~~{.cpp}
// Create a managed array of integers, with 10 elements
ScriptArray outArray = ScriptArray::create<UINT32>(10);
for(UINT32 i = 0; i < 10; i++)
	outArray.set(i, 0);

MonoArray* monoArray = outArray.getInternal();
~~~~~~~~~~~~~ 

You can easily read a **MonoArray** by creating a new **ScriptArray**, using the **MonoArray** in its constructor. Then you can retrieve the size of the array using @bs::ScriptArray::size(), and the value of its elements by calling @bs::ScriptArray::get. 

~~~~~~~~~~~~~{.cpp}
// Read-back the array above into a Vector
ScriptArray inArray(monoArray);

Vector<UINT32> output(inArray.size());
for(UINT32 i = 0; i < inArray.size(); i++)
	output[i] = inArray.get<UINT32>(i);
~~~~~~~~~~~~~ 

## MonoObject assignment
As we discussed earlier, **MonoObject** is garbage collected by the Mono's garbage collector. In order for the GC to be able to track **MonoObject**'s in C++, there are some rules that need to be followed when passing these objects around. 

By default **MonoObject** will not be garabage collected if it is stored on the stack or registers (e.g. a local variable in a function). If that is your only use case then no further action is required. But if you copy it to some heap allocated memory you need to inform the GC of that copy, otherwise it might get collected while you are still using it. Often this is the case when outputting a **MonoObject** through a method parameter (marked with `ref` or `out` in C#).

In these cases instead of assignment (`=` operator), use @bs::MonoUtil::referenceCopy();

~~~~~~~~~~~~~{.cpp}
// One common example for the use of MonoUtil::referenceCopy is outputting a MonoObject by parameter
// This is an internal method that gets called from managed code
void getObject(MonoObject** output)
{
	MonoObject* newObj = ...; // Create a new MonoObject (or retrieve one from a stored GC handle)
	
	*output = newObj; // WRONG - GC isn't aware of this new reference to the MonoObject*
	MonoUtil::referenceCopy(output, newObj); // CORRECT - GC is properly notified
}
~~~~~~~~~~~~~ 

If a **MonoObject** is a member of a struct which you are copying to some heap allocated memory, you need to use @bs::MonoUtil::valueCopy() instead of normal assignment. 

~~~~~~~~~~~~~{.cpp}
struct SomeStruct
{
	int a;
	MonoObject* b;
};

// This is an internal method that gets called from managed code
void getObject(SomeStruct* output)
{
	SomeStruct obj;
	obj.a = 5;
	obj.b = ...; // Create a new MonoObject (or retrieve one from a stored GC handle)
	
	*output = obj; // WRONG
	
	MonoClass* structClass = ...; // Retrieve the MonoClass of the struct's type
	MonoUtil::valueCopy(output, &obj, structClass); // CORRECT - GC is properly notified
}
~~~~~~~~~~~~~ 

When writing to arrays using **ScriptArray** you do not need to follow these rules as **ScriptArray** will handle this internally.

# Internal methods
So far we have talked about calling managed code, and retrieving information about managed types, but we have yet to show how managed code calls C++ code. This is accomplished using internal methods.

The first step is to define a stub method in managed code, like so:
~~~~~~~~~~~~~{.cs}
[MethodImpl(MethodImplOptions.InternalCall)]
private static extern float Internal_GetSomeValue(MyObject obj);
~~~~~~~~~~~~~
	
You then hook up this method with managed code by calling @bs::MonoClass::addInternalCall. In this specific case it would be:
~~~~~~~~~~~~~{.cpp}
float myNativeFunction(MonoObject* obj)
{
	// Do something
}

klass->addInternalCall("Internal_GetSomeValue", &myNativeFunction);
~~~~~~~~~~~~~

After this is done any call to the managed stub method will call the provided native function. You should take care to properly handle parameter passing as described above.

> Note that internal methods cannot be overloaded and each must have a unique name.