---
title: Exposing code to script API (automated)
---

When you've added a new feature, system or just extended an existing one you might want to expose that functionality to the scripting API. The framework makes this process easier through its automated script binding generator tool. All you need to do is to decorate the C++ types and methods you wish to export and run the tool.

The script binding generator tool (BansheeCodeGenerator) is a standalone Clang-based tool that parses the decorated C++ headers and generates the corresponding C# script code and C++ interop wrappers.

Entirety of automated script export is handled through the **B3D_SCRIPT_EXPORT** macro. The macro supports a variety of parameters used for customizing how will the type/method be exported. Parameters use the `ParameterName(Value)` syntax, separated by commas.

# Exporting classes
In order to export a class to script code you need to decorate the class and one or more methods with **B3D_SCRIPT_EXPORT** macro. 

~~~~~~~~~~~~~{.cpp}
// Decorate the class and the methods with B3D_SCRIPT_EXPORT modifier
class B3D_SCRIPT_EXPORT() MyClass
{
public:
	B3D_SCRIPT_EXPORT()
	MyClass(); // Constructor

	B3D_SCRIPT_EXPORT()
	u32 GetSomeData();
};
~~~~~~~~~~~~~

The example above results in the following script interface (C#):
~~~~~~~~~~~~~{.cs}
public partial class MyClass : ScriptObject
{
	public MyClass() { ... }

	public int GetSomeData() { ... }
}
~~~~~~~~~~~~~

There are a few important rules when using **B3D_SCRIPT_EXPORT**:
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
class B3D_SCRIPT_EXPORT() MyOtherClass
{
   ...
};

// Examples of some valid exported methods
class B3D_SCRIPT_EXPORT() MyClass
{
public:
	B3D_SCRIPT_EXPORT()
	MyClass(); // Constructor

	B3D_SCRIPT_EXPORT()
	u32 GetPlainType();
	
	B3D_SCRIPT_EXPORT()
	void GetPlainTypeAsParam(u32& output);	
	
	B3D_SCRIPT_EXPORT()
	void SetPlainType(u32 value);	
	
	B3D_SCRIPT_EXPORT()
	String GetString();
	
	B3D_SCRIPT_EXPORT()
	void SetString(const String& value);	
	
	B3D_SCRIPT_EXPORT()
	HMesh GetResource();
	
	B3D_SCRIPT_EXPORT()
	void SetResource(const HMesh& mesh);
	
	B3D_SCRIPT_EXPORT()
	HRenderable GetComponent();
	
	B3D_SCRIPT_EXPORT()
	void SetComponent(const HRenderable& renderable);
	
	B3D_SCRIPT_EXPORT()
	SPtr<MyOtherClass> GetNormalObject();
	
	B3D_SCRIPT_EXPORT()
	void SetNormalObject(const SPtr<MyOtherClass>& value);
	
	B3D_SCRIPT_EXPORT()
	Vector<HMesh> GetArray(); // HMesh could have also been a normal class or a component
	
	B3D_SCRIPT_EXPORT()
	void SetArray(const Vector<HMesh>& value); // HMesh could have also been a normal class or a component
};
~~~~~~~~~~~~~ 

## Renaming
**B3D_SCRIPT_EXPORT** accepts a variety of comma separated parameters in the `ParameterName(Value)` format. The `ExportName` parameter allows you to specify a different name for a type or a method, so when exported it uses the specified name rather than the same name as in C++.

~~~~~~~~~~~~~{.cpp}
class B3D_SCRIPT_EXPORT(ExportName(MyRenamedClass)) MyClass
{
public:
	B3D_SCRIPT_EXPORT(ExportName(GetSomeData))
	u32 GetSomeData();
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
You can make a type or a method *public*, *internal* or *private* by specifying the `Visibility` parameter. Accepted values are `Public`, `Internal` and `Private`. By default all types and methods are public.

~~~~~~~~~~~~~{.cpp}
class B3D_SCRIPT_EXPORT() MyClass
{
public:
	// Exported as a private method
	B3D_SCRIPT_EXPORT(ExportName(GetSomeData), Visibility(Private))
	u32 GetSomeData();
};
~~~~~~~~~~~~~

The example above results in the following script interface (C#):
~~~~~~~~~~~~~{.cs}
public partial class MyClass : ScriptObject
{
	private int GetSomeData() { ... }
}
~~~~~~~~~~~~~

## Exporting as properties
The `Property` parameter allows you to specify that a method should be exported as a property. The supported values are `Getter` or `Setter`. When exposing a method as a property the `ExportName` parameter is required and should be the name of the property.

~~~~~~~~~~~~~{.cpp}
// Decorate the class and the methods with B3D_SCRIPT_EXPORT modifier
class B3D_SCRIPT_EXPORT() MyClass
{
public:
	B3D_SCRIPT_EXPORT(Property(Getter), ExportName(SomeData))
	u32 GetSomeData();
	
	B3D_SCRIPT_EXPORT(Property(Setter), ExportName(SomeData))
	void SetSomeData(u32 value);
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
A data type can be exported as a C# *struct* by using the `ExportAsStruct` parameter, accepting values `true` or `false` (default being false). When exported all of the fields of the data type will be exported as a C# *struct*. Any constructors will also be exported, but no other methods. This is meant to be used on simple types that will be used for passing data around. Such types are passed by value and will be copied when crossing the C++/C# boundary.

~~~~~~~~~~~~~{.cpp}
struct B3D_SCRIPT_EXPORT(ExportAsStruct(true)) Volume
{
	Volume()
		: left(0), top(0), right(1), bottom(1), front(0), back(1)
	{ }

	Volume(u32 left, u32 top, u32 right, u32 bottom):
		left(left), top(top), right(right), bottom(bottom), front(0), back(1)
	{ }

	Volume(u32 left, u32 top, u32 front, u32 right, u32 bottom, u32 back):
		left(left), top(top), right(right), bottom(bottom), front(front), back(back)
	{ }
		
	u32 left, top, right, bottom, front, back;
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

Structs support `ExportName` & `Visibility` parameters same as normal class export.

# Exporting enums
Enums can be exported with no additional parameters, just by specifying **B3D_SCRIPT_EXPORT**.

~~~~~~~~~~~~~{.cpp}
enum B3D_SCRIPT_EXPORT() class MyEnum
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

Enums support `ExportName` & `Visibility` parameters same as normal class and struct export.

## Excluding enum entries
By default when exporting enums all of their entries will be exported. You can ignore a certain enum entry by using the `Exclude` parameter. Supported values are `true` or `false`.

~~~~~~~~~~~~~{.cpp}
// Exclude the third enum entry from script code
enum B3D_SCRIPT_EXPORT() class MyEnum
{
	Value1 										= 1,
	Value2 										= 10,
	Value3 		B3D_SCRIPT_EXPORT(Exclude(true))	= 100
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
Individual enum entries can also be renamed using the `ExportName` parameter.

~~~~~~~~~~~~~{.cpp}
enum B3D_SCRIPT_EXPORT() MyEnum
{
	ME_VAL1 	B3D_SCRIPT_EXPORT(ExportName(Value1)) 		= 1,
	ME_VAL2 	B3D_SCRIPT_EXPORT(ExportName(Value2)) 		= 10,
	ME_VAL3 	B3D_SCRIPT_EXPORT(ExportName(Value3)) 		= 100
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

Use the `ExtensionMethodForType` parameter to mark a method as external. Value of the parameter should be the name of the class it is extending.
Use the `ExtensionConstructorForType` parameter to mark a constructor as external. Value of the parameter should be the name of the class it is extending.
The class containing external methods must be marked with the `ExtensionClassForType` parameter.

~~~~~~~~~~~~~{.cpp}
struct MY_CLASS_DESC
{
	int val1;
	float val2;
}

// Some class we're exporting normally
class B3D_SCRIPT_EXPORT() MyClass
{
public:
	MyClass(const MY_CLASS_DESC& desc);

	u32* GetArrayData(u32& count) const;
};

// Extension class for MyClass
class B3D_SCRIPT_EXPORT(ExtensionClassForType(MyClass)) MyClassEx
{
public:
	// External constructor because we don't want to expose MY_CLASS_DESC to script code
	B3D_SCRIPT_EXPORT(ExtensionConstructorForType(MyClass))
	static SPtr<MyClass> Create(int val1, float val2)
	{
		MY_CLASS_DESC desc;
		desc.val1 = val1;
		desc.val2 = val2;
		
		return B3DMakeShared<MyClass>(desc);
	}

	// External method because MyClass returns an array in raw form, but we need it in a Vector
	B3D_SCRIPT_EXPORT(ExtensionMethodForType(MyClass))
	static Vector<u32> GetArrayData(const SPtr<MyClass>& thisPtr)
	{
		u32 numEntries;
		u32* entries = thisPtr->GetArrayData(numEntries);
		
		Vector<u32> output;
		for(u32 i = 0; i < numEntries; i++)
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
 - They must be part of a class that is also exported and marked with the `ExtensionClassForType` parameter
 - They must be static
 - External constructors must return a value of the type they're external to
 - External methods must accept the type they're external to as the first parameter 

# Additional options

## Assembly targeting
The `API` parameter specifies which assembly a type should be exported to. Supported values are `Framework`, `Engine` and `Editor`. Multiple `API` parameters can be specified to export to multiple assemblies.

~~~~~~~~~~~~~{.cpp}
// Export to both Framework and Editor assemblies
class B3D_SCRIPT_EXPORT(API(Framework), API(Editor)) ImportOptions
{
	...
};
~~~~~~~~~~~~~

## Documentation group
The `DocumentationGroup` parameter specifies which module a type belongs to. This determines the documentation group and may also affect the placement in the generated code.

~~~~~~~~~~~~~{.cpp}
class B3D_SCRIPT_EXPORT(DocumentationGroup(Animation)) AnimationClipState
{
	...
};
~~~~~~~~~~~~~

## Output file
The `ExportFile` parameter overrides the name of the output file(s) for the generated script object and its wrappers. If not specified the name of the type will be used.

## Interop-only methods
The `InteropOnly` parameter causes only the internal interop method to be generated, without a public wrapper method. This is useful when you need to manually implement the public-facing method in C# (e.g. for custom marshalling or additional logic). Set to `true` to enable.

~~~~~~~~~~~~~{.cpp}
class B3D_SCRIPT_EXPORT() MyClass
{
public:
	// Only the interop stub is generated; you must write the public C# method manually
	B3D_SCRIPT_EXPORT(InteropOnly(true))
	Vector<HMesh> GetComplexData();
};
~~~~~~~~~~~~~

## Singletons
The `Singleton` parameter marks a class as a singleton. The value should be the name of the getter function used to retrieve the singleton instance.

~~~~~~~~~~~~~{.cpp}
class B3D_SCRIPT_EXPORT(Singleton(GetDebug)) Debug
{
	...
};
~~~~~~~~~~~~~

## Pass by copy
The `PassByCopy` parameter causes a struct to be passed by copy instead of by reference when crossing the C++/C# boundary. Set to `true` to enable.

~~~~~~~~~~~~~{.cpp}
B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Settings), PassByCopy(true))
const ParticleSystemSettings& GetSettings() const;
~~~~~~~~~~~~~

## Inspector UI hints
Several parameters control how exported fields and properties appear in the editor inspector:

 - `UI(Hide)` - Hide the field from the inspector.
 - `UI(Show)` - Force show the field in the inspector.
 - `UI(AsSlider)` - Render as a slider control.
 - `UI(AsLayerMask)` - Render as a layer mask selector.
 - `UI(IsHDRColor)` - Mark as an HDR color field.
 - `UI(AsQuaternion)` - Display rotation as Euler angles.
 - `UI(Inline)` - Inline the struct fields directly in the parent inspector.
 - `UIValueRange([min, max])` - Clamp the value to a range.
 - `UIIncrementStep(value)` - Set the increment step for the inspector control.
 - `UIOrder(value)` - Control the display order in the inspector.
 - `UICategory(name)` - Group the field under a collapsible category.
 - `ApplyOnDirty(true)` - Apply changes when the property is marked dirty.
 - `LoadOnAssign(true)` - Automatically load a resource when it is assigned.
 - `NotNullable(true)` - Mark a parameter or field as non-nullable.

~~~~~~~~~~~~~{.cpp}
class B3D_SCRIPT_EXPORT() MySettings
{
public:
	B3D_SCRIPT_EXPORT(UIValueRange([0, 1]), UI(AsSlider))
	float Intensity = 0.5f;

	B3D_SCRIPT_EXPORT(UICategory(Advanced), UIOrder(1))
	bool UseHighQuality = false;
};
~~~~~~~~~~~~~

# Running the code generator
The code generator (BansheeCodeGenerator) is a standalone tool located in `Framework/Tools/BansheeCodeGenerator`. Once you have decorated the C++ classes with necessary export parameters, build and run the code generator. It will parse all the framework headers and generate the corresponding C# script code and C++ interop wrappers into the `Generated` folders.
