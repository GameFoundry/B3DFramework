---
title: Exposing code to script API (automated)
---

When you've added a new feature, system or just extended an existing one you might want to expose that functionality to the scripting API. The framework makes this process easier through its automated script binding generator tool. All you need to do is to decorate the C++ types and methods you wish to export and run the tool.

This manual will teach you how to decorate the C++ code in order to expose it to the script API. To learn how to run the script binding generator tool visit the [Generating script bindings](../Build/generatingScriptBindings) manual.

Entirety of automated script export is handled through the **BS_SCRIPT_EXPORT** macro. The macro supports a variety of parameters used for customizing how will the type/method be exported.

# Exporting classes
In order to export a class to script code you need to decorate the class and one or multiple methods with **BS_SCRIPT_EXPORT** macro. 

~~~~~~~~~~~~~{.cpp}
// Decorate the class and the methods with BS_SCRIPT_EXPORT modifier
class BS_SCRIPT_EXPORT() MyClass
{
public:
	BS_SCRIPT_EXPORT()
	MyClass(); // Constructor

	BS_SCRIPT_EXPORT()
	UINT32 getSomeData();
};
~~~~~~~~~~~~~

The example above results in the following script interface (C#):
~~~~~~~~~~~~~{.cs}
public partial class MyClass : ScriptObject
{
	public MyClass() { ... }

	public int getSomeData() { ... }
}
~~~~~~~~~~~~~

There are a few important rules when using **BS_SCRIPT_EXPORT**:
 - It CAN be used on non-templated classes or structs, as well as **Resource**s and **Component**s
 - It CANNOT be used on templated classes or structs
 - It CAN be used on public constructors and non-templated methods
 - It CANNOT be used on destructors, operators, templated methods or private/protected methods of any kind

If you do not respect these rules the code generation will either fail, or the generated code will be invalid and will fail to compile.
 
There are also limitations on the types of parameters and return values the exported methods are allowed to have:
 - **Resource**%s and **Component**%s must be passed as handles (e.g. **HMesh**, **HRenderable**)
 - Other classes must be passed as shared pointers (e.g. **SPtr<MyClass>**)
 - Plain types such as **int**, **float** or types we'll describe below can be passed directly (e.g. **int**)
 - If a parameter is used as input it is beneficial to decorate it as a constant reference or a pointer (e.g. *const HMesh&*)
 - If a parameter is used as output it is beneficial to decorate it as a non-constant reference or a pointer (e.g. *HMesh&*)
 - If a parameter is an array, it must be of type **Vector** (e.g. *Vector<HMesh>* or *Vector<SPtr<MyClass>>*)
 
~~~~~~~~~~~~~{.cpp}
class BS_SCRIPT_EXPORT() MyOtherClass
{
   ...
};

// Examples of some valid exported methods
class BS_SCRIPT_EXPORT() MyClass
{
public:
	BS_SCRIPT_EXPORT()
	MyClass(); // Constructor

	BS_SCRIPT_EXPORT()
	UINT32 getPlainType();
	
	BS_SCRIPT_EXPORT()
	void getPlainTypeAsParam(UINT32& output);	
	
	BS_SCRIPT_EXPORT()
	void setPlainType(UINT32 value);	
	
	BS_SCRIPT_EXPORT()
	String getString();
	
	BS_SCRIPT_EXPORT()
	void setString(const String& value);	
	
	BS_SCRIPT_EXPORT()
	HMesh getResource();
	
	BS_SCRIPT_EXPORT()
	void setResource(const HMesh& mesh);
	
	BS_SCRIPT_EXPORT()
	HRenderable getComponent();
	
	BS_SCRIPT_EXPORT()
	void setComponent(const HRenderable& renderable);
	
	BS_SCRIPT_EXPORT()
	SPtr<MyOtherClass> getNormalObject();
	
	BS_SCRIPT_EXPORT()
	void setNormalObject(const SPtr<MyOtherClass>& value);
	
	BS_SCRIPT_EXPORT()
	Vector<HMesh> getArray(); // HMesh could have also been a normal class or a component
	
	BS_SCRIPT_EXPORT()
	void setArray(const Vector<HMesh>& value); // HMesh could have also been a normal class or a component
};
~~~~~~~~~~~~~ 

## Renaming
**BS_SCRIPT_EXPORT** accepts a variety of comma separated parameters in the format "param1:value1,param2:value2". Parameter named "n" allows you to specify a different name for a type or a method, so when exported it uses the specified name rather than the same name as in C++.

~~~~~~~~~~~~~{.cpp}
class BS_SCRIPT_EXPORT(n:MyRenamedClass) MyClass
{
public:
	BS_SCRIPT_EXPORT(n:GetSomeData)
	UINT32 getSomeData();
};
~~~~~~~~~~~~~

The example above results in the following script interface (C#):
~~~~~~~~~~~~~{.cs}
public partial class MyRenamedClass : ScriptObject
{
	public int GetSomeData() { ... }
}
~~~~~~~~~~~~~

## Visibility
You can make a type or a method *public*, *internal* or *private* by specifying the "v" parameter. Accepted values are "public", "internal" and "private". By default all types and methods are public.

~~~~~~~~~~~~~{.cpp}
class BS_SCRIPT_EXPORT() MyClass
{
public:
	// Exported as a private method
	BS_SCRIPT_EXPORT(n:GetSomeData,v:private)
	UINT32 getSomeData();
};
~~~~~~~~~~~~~

The example above results in the following script interface (C#):
~~~~~~~~~~~~~{.cs}
public partial class MyRenamedClass : ScriptObject
{
	private int GetSomeData() { ... }
}
~~~~~~~~~~~~~

## Exporting as properties
Parameter named "pr" allows you to specify that a method should be exported as a property. The supported values for the parameter are "getter" or "setter". When exposing a method as a property the name ("n") parameter is required and should be the name of the property.

~~~~~~~~~~~~~{.cpp}
// Decorate the class and the methods with BS_SCRIPT_EXPORT modifier
class BS_SCRIPT_EXPORT() MyClass
{
public:
	BS_SCRIPT_EXPORT(pr:getter,n:SomeData)
	UINT32 getSomeData();
	
	BS_SCRIPT_EXPORT(pr:setter,n:SomeData)
	void setSomeData(UINT32 value);
};
~~~~~~~~~~~~~

The example above results in the following script interface (C#):
~~~~~~~~~~~~~{.cs}
public partial class MyClass : ScriptObject
{
	public int SomeData
	{
		get { ... }
		set { ... }
	}
}
~~~~~~~~~~~~~

You are allowed to provide only getter, only setter, or both. Providing multiple getters or setters for the propery with the same name results in undefined behaviour.

Getter/setter methods must follow a specific template otherwise they will be ignored during generation:
 - Getter method must return a non-void value, and have no parameters
 - Setter method must not have a return value and a single parameter
 - Getter return value and setter parameter types must match

## Extending script interface further
Sometimes automatic code generation just isn't good enough. For that reason all exported C# classes are marked with the *partial* keyword, meaning you can extend their interface with manually written code in a separate file, as required. 
 
# Exporting structures
A data type can be exported as a C# *struct* by using the "pl" parameter, accepting values "true" or "false" (default being false). When exported all of the fields of the data type will be exported as a C# *struct*. Any constructors will also be exported, but no other methods. This is meant to be used on simple types that will be used for passing data around. Such types are passed by value and will be copied when crossing the C++/C# boundary.

~~~~~~~~~~~~~{.cpp}
struct BS_SCRIPT_EXPORT(pl:true) Volume
{
	Volume()
		: left(0), top(0), right(1), bottom(1), front(0), back(1)
	{ }

	Volume(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom):
		left(left), top(top), right(right), bottom(bottom), front(0), back(1)
	{ }

	Volume(UINT32 left, UINT32 top, UINT32 front, UINT32 right, UINT32 bottom, UINT32 back):
		left(left), top(top), right(right), bottom(bottom), front(front), back(back)
	{ }
		
	UINT32 left, top, right, bottom, front, back;
};
~~~~~~~~~~~~~

The example above results in the following script code (C#):
~~~~~~~~~~~~~{.cs}
public partial struct Volume
{
	// C# doesn't support parameterless struct constructors, and therefore a static Default() method is generated instead
	public static Volume Default()
	{
		Volume value = new Volume();
		value.left = 0;
		value.top = 0;
		value.right = 1;
		value.bottom = 1;
		value.front = 0;
		value.back = 1;

		return value;
	}

	public Volume(int left, int top, int right, int bottom)
	{
		this.left = left;
		this.top = top;
		this.right = right;
		this.bottom = bottom;
		this.front = 0;
		this.back = 1;
	}

	public Volume(int left, int top, int front, int right, int bottom, int back)
	{
		this.left = left;
		this.top = top;
		this.right = right;
		this.bottom = bottom;
		this.front = front;
		this.back = back;
	}

	public int left;
	public int top;
	public int right;
	public int bottom;
	public int front;
	public int back;
}
~~~~~~~~~~~~~

Note when generating constructors the system is only able to parse class member initializers and constructor initializers defined in the header file and will ignore any in the .cpp file.

Structs support rename & visibility parameters same as normal class export.

# Exporting enums
Enums can be exported with no additional parameters, just by specifying **BS_SCRIPT_EXPORT**.

~~~~~~~~~~~~~{.cpp}
enum BS_SCRIPT_EXPORT() class MyEnum
{
	Value1 = 1,
	Value2 = 10,
	Value3 = 100
};
~~~~~~~~~~~~~

The example above results in the following script code (C#):
~~~~~~~~~~~~~{.cs}
public enum MyEnum
{
	Value1 = 1,
	Value2 = 10,
	Value3 = 100
}
~~~~~~~~~~~~~

Enums support rename & visibility parameters same as normal class and struct export.

## Excluding enum entries
By default when exporting enums all of their entries will be exported. You can ignore a certain enum entry by using the "ex" parameter. Supported values are "true" or "false".

~~~~~~~~~~~~~{.cpp}
// Exclude the third enum entry from script code
enum BS_SCRIPT_EXPORT() class MyEnum
{
	Value1 										= 1,
	Value2 										= 10,
	Value3 		BS_SCRIPT_EXPORT(ex:true)		= 100
};
~~~~~~~~~~~~~

The example above results in the following script code (C#):
~~~~~~~~~~~~~{.cs}
public enum MyEnum
{
	Value1 = 1,
	Value2 = 10,
}
~~~~~~~~~~~~~

## Renaming enum entries
Individual enum entries can also be renamed using the "n" parameter.

~~~~~~~~~~~~~{.cpp}
enum BS_SCRIPT_EXPORT() MyEnum
{
	ME_VAL1 	BS_SCRIPT_EXPORT(n:Value1) 		= 1,
	ME_VAL2 	BS_SCRIPT_EXPORT(n:Value2) 		= 10,
	ME_VAL3 	BS_SCRIPT_EXPORT(n:Value3) 		= 100
};
~~~~~~~~~~~~~

The example above results in the following script code (C#):
~~~~~~~~~~~~~{.cs}
public enum MyEnum
{
	Value1 = 1,
	Value2 = 10,
	Value3 = 100
}
~~~~~~~~~~~~~

# Exporting comments
All Javadoc-type comments on exported types and methods will automatically be parsed, converted to XML documentation format and exported to script code.

# External methods
Sometimes the C++ interface just isn't suitable for export to script code as-is. Sometimes you want to make the script code more streamlined and higher level, without modifying the existing C++ interface. Other times the method parameters or return values don't fit the requirements we stated above.

External methods allow you to extend functionality of some class, **Resource** or a **Component** by defining static methods which are then exported as if they were part of the original class. Note these types of methods are not relevant for struct or enum export.

Use the "e" parameter to mark a method as external. Value of the parameter should be the name of the class it is extending.
Use the "ec" parameter to mark a constructor as external. Value of the parameter should be the name of the class it is extending.

~~~~~~~~~~~~~{.cpp}
struct MY_CLASS_DESC
{
	int val1;
	float val2;
}

// Some class we're exporting normally
class BS_SCRIPT_EXPORT() MyClass
{
public:
	MyClass(const MY_CLASS_DESC& desc);

	UINT32* getArrayData(UINT32& count) const;
};

// Extension class for MyClass
class BS_SCRIPT_EXPORT(e:MyClass) MyClassEx
{
public:
	// External constructor because we don't want to expose MY_CLASS_DESC to script code
	BS_SCRIPT_EXPORT(ec:MyClass)
	static SPtr<MyClass> create(int val1, float val2)
	{
		MY_CLASS_DESC desc;
		desc.val1 = val1;
		desc.val2 = val2;
		
		return bs_shared_ptr_new<MyClass>(desc);
	}

	// External method because MyClass returns an array in raw form, but we need it in a Vector
	BS_SCRIPT_EXPORT(e:MyClass)
	static Vector<UINT32> getArrayData(const SPtr<MyClass>& thisPtr)
	{
		UINT32 numEntries;
		UINT32* entries = thisPtr->getArrayData(numEntries);
		
		Vector<UINT32> output;
		for(UINT32 i = 0; i < numEntries; i++)
			output.push_back(entries[i]);
			
		return output;
	}
};
~~~~~~~~~~~~~

The example above results in the following script interface (C#):
~~~~~~~~~~~~~{.cs}
public partial class MyClass : ScriptObject
{
	public MyClass(int val1, float val2) { ... }

	public int[] getArrayData() { ... }
}
~~~~~~~~~~~~~

External methods must follow these rules:
 - They must be part of a class that is also exported and marked with the "e" parameter
 - They must be static
 - External constructors must return a value of the type they're external to
 - External methods must accept the type they're external to as the first parameter 
 
# Running the code generator
Once you have decorated the C++ classes with necessary export parameters you can run the code generator simply by enabling *GENERATE_SCRIPT_BINDINGS* and regenerating the CMake build. The generator will run automatically over all bs::f code and generate relevant script code.
