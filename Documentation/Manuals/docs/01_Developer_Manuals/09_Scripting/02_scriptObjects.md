---
title: Script objects
---

What we have shown in the previous manual is enough to expose an object to C# and communicate with it. However the framework provides another API built on top of that functionality in the form of script objects. This API handles some of the boilerplate code required for exposing an object to C#, provides a common interface all script objects need to implement, handles assembly refresh (due to script hot-swap) and gracefully handles managed object lifetime and destruction.

To implement the script object interface for a particular type you need two classes:
 - A native interop class (C++)
 - Managed wrapper class for the type (C#)

# Architecture overview
The script binding system has a layered architecture:

 - @b3d::IScriptExportable - Interface implemented by native types that are exported to script. Provides the link from the native object to its script wrapper, and participates in script reload.
 - @b3d::IScriptObjectWrapper - Base interface for all script object wrappers. Tracks the bidirectional link between a native object and its script object, and handles lifetime notifications.
 - @b3d::ScriptObjectWrapper - Extends **IScriptObjectWrapper** with GC handle management, script reload support, and the factory method for creating wrappers.
 - @b3d::TScriptTypeDefinition - Template that provides per-type meta-data (`sInteropMetaData`), auto-registration with **MonoManager**, and detection of optional `SetupScriptBindings()` and `InitializeAdditionalMetaData()` methods.
 - @b3d::TScriptObjectWrapper - Template combining **ScriptObjectWrapper** (or a custom base) with **TScriptTypeDefinition**. This is the primary base class for most script interop objects.

## Specialized wrappers
Several specialized wrapper templates are provided for common native object categories. These handle type-specific concerns like reference counting, handle management and RTTI-based type lookup:

 - @b3d::TScriptReflectableWrapper - For **IReflectable**-derived types passed as shared pointers (e.g. render settings, collider shapes). Inherits from **ScriptReflectableWrapper**.
 - @b3d::TScriptResourceWrapper - For **Resource**-derived types passed as resource handles (e.g. **Mesh**, **Texture**). Inherits from **ScriptResourceWrapper**.
 - @b3d::TScriptGameObjectWrapper - For **GameObject**-derived types passed as game object handles (e.g. **Component**, **SceneObject**). Inherits from **ScriptGameObjectWrapper**.
 - @b3d::TScriptNonReflectableWrapper - For non-**IReflectable** types passed as shared pointers. Inherits from **TScriptNonReflectableWrapperBase**.
 - @b3d::TScriptValueTypeWrapper - For value types that are copied when crossing the C++/C# boundary. Uses weak handle lifetime tracking.

Each specialized wrapper provides:
 - A constructor accepting the appropriate native handle/pointer type
 - `CreateScriptObjectAndWrapper()` - Creates both the managed object and the wrapper, and associates them with a native object
 - `GetOrCreateScriptObject()` - Returns the existing managed object for a native object, or creates a new one if none exists
 - `GetNativeObjectAsShared()` or `GetNativeObjectAsHandle()` - Retrieves the wrapped native object

# Native interop class
This class is intended as a wrapper for the C++ class you're exposing to the scripting API. It will contain all the code needed for C++/C# interop. Which base class to use depends on the type of native object being wrapped:

| Native object type | Base class template | Constructor parameter |
|---|---|---|
| **IReflectable** subclass (shared ptr) | `TScriptReflectableWrapper<NativeType, SelfType>` | `const TShared<NativeType>&` |
| **Resource** subclass (handle) | `TScriptResourceWrapper<NativeType, SelfType>` | `const TResourceHandle<NativeType>&` |
| **GameObject** subclass (handle) | `TScriptGameObjectWrapper<NativeType, SelfType>` | `const TGameObjectHandle<NativeType>&` |
| Non-reflectable (shared ptr) | `TScriptNonReflectableWrapper<NativeType, SelfType>` | `const TShared<NativeType>&` |
| Value type (by value) | `TScriptValueTypeWrapper<NativeType, SelfType>` | `const NativeType&` |
| Other / manual | `TScriptObjectWrapper<SelfType>` | `IScriptExportable*` |

The implementation of the class must include the @B3D_SCRIPT_TYPE_DEFINITION macro. The macro accepts (in order):
 - the assembly constant for the managed wrapper class, usually `kEngineAssembly` or `kEditorAssembly`
 - the namespace constant for the managed wrapper class, usually `kEngineNs` or `kEditorNs`
 - the name of the managed wrapper class

Each wrapper must also provide a static `CreateScriptObject(bool construct)` method that creates the managed object instance.

~~~~~~~~~~~~~{.cpp}
// Example: wrapping an IReflectable-derived type
class ScriptMyObject : public TScriptReflectableWrapper<MyObject, ScriptMyObject>
{
public:
	B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "MyObject")

	ScriptMyObject(const TShared<MyObject>& nativeObject)
		: TScriptReflectableWrapper(nativeObject)
	{ }

	static MonoObject* CreateScriptObject(bool construct);
	static void SetupScriptBindings();
};
~~~~~~~~~~~~~

## SetupScriptBindings
**B3D_SCRIPT_TYPE_DEFINITION** works together with @b3d::TScriptTypeDefinition to auto-detect an optional static `SetupScriptBindings()` method on your class. If present, it gets called automatically on startup and whenever the assembly containing the related managed class is loaded.

Every **TScriptTypeDefinition** provides a static `sInteropMetaData` field (of type @b3d::ScriptTypeMetaData) you can use for retrieving the **MonoClass** of the related managed class. You can use that **MonoClass** to register internal methods to it (as described in the previous manual).

~~~~~~~~~~~~~{.cpp}
class ScriptMyObject : public TScriptReflectableWrapper<MyObject, ScriptMyObject>
{
	// Other code ...

	static float internal_GetSomeValue(ScriptMyObject* self);
	static void internal_SetSomeValue(ScriptMyObject* self, float value);
};

void ScriptMyObject::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSomeValue", &ScriptMyObject::internal_GetSomeValue);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSomeValue", &ScriptMyObject::internal_SetSomeValue);
}
~~~~~~~~~~~~~

**SetupScriptBindings()** is also a good spot to retrieve **MonoMethod**%s (or thunks) for managed methods that need to be called by the script interop object, if any.

## Creating script object instances
If your class is not static you will need to eventually create an instance of the script object. The specialized wrappers provide `CreateScriptObjectAndWrapper()` and `GetOrCreateScriptObject()` static methods for this purpose.

`GetOrCreateScriptObject()` is the preferred entry point — it first checks if the native object already has an associated script object (via @b3d::IScriptExportable::GetScriptObjectWrapper), and only creates a new one if needed. For **IReflectable** and **GameObject** types, it also performs RTTI-based lookup to find the correct wrapper type for derived classes.

~~~~~~~~~~~~~{.cpp}
// From C++: get or create the managed object for a native object
TShared<MyObject> nativeObject = ...;
MonoObject* managedObj = ScriptMyObject::GetOrCreateScriptObject(nativeObject);
~~~~~~~~~~~~~

If the managed object is created from C# (e.g. via a constructor), you should set up an internal method that accepts the managed object instance and creates the wrapper:
~~~~~~~~~~~~~{.cpp}
void ScriptMyObject::internal_CreateInstance(MonoObject* scriptObject)
{
	TShared<MyObject> nativeObject = B3DMakeShared<MyObject>();
	ScriptObjectWrapper::Create<ScriptMyObject>(nativeObject, scriptObject);
}
~~~~~~~~~~~~~

@b3d::ScriptObjectWrapper::Create is the factory method for creating wrappers. It takes the native object and the managed object as parameters:
~~~~~~~~~~~~~{.cpp}
template<typename ScriptWrapperType, typename NativeType>
static ScriptWrapperType* Create(NativeType&& nativeObject, MonoObject* scriptObject);
~~~~~~~~~~~~~

If you ever receive a **MonoObject** of the type you know that has a **TScriptObjectWrapper** implemented, you can retrieve it by calling the static `GetScriptObjectWrapper(MonoObject*)` method.
~~~~~~~~~~~~~{.cpp}
void ScriptMyObject::internal_SetSomeObject(MonoObject* obj)
{
	ScriptSomeObject* someObj = ScriptSomeObject::GetScriptObjectWrapper(obj);
}
~~~~~~~~~~~~~

## Lifetime tracking
Script object wrappers manage the link between the native object and the managed object using GC handles. The @b3d::ScriptObjectLifetimeTrackingMode enum controls the behavior:

 - `StrongHandleWithGarbageCollection` (default) - The wrapper holds a strong GC handle. Periodically, the system checks if the native object's reference count has dropped to 1 (only the wrapper holds it). If so, the handle transitions to weak, allowing garbage collection to destroy both the managed and native objects.
 - `StrongHandleWithExplicitDestroy` - The wrapper holds a strong GC handle that is released when the native object is explicitly destroyed. Used by **GameObject** and **Resource** types.
 - `WeakHandle` - The wrapper holds a weak GC handle. The native object is freed when the managed object is garbage collected. Used by value types.

## Destroying script object instances
When the managed object is destroyed (e.g. goes out of scope and gets garbage collected) the system will automatically take care of freeing the related **TScriptObjectWrapper**. You can override `NotifyScriptObjectDestroyed(bool isDestroyedDueToScriptReload)` if you need to perform additional cleanup. Similarly, `NotifyNativeObjectDestroyed()` is called when the native object is destroyed first.

# IScriptExportable
Native types that are exported to script should implement the @b3d::IScriptExportable interface. This provides the native-side link to the script world:

 - `GetScriptObjectWrapper()` - Returns the associated **IScriptObjectWrapper**, or null if no script object exists yet.
 - Script reload callbacks (mirroring those on **ScriptObjectWrapper**) - `ShouldPersistScriptReload()`, `BackupDataBeforeScriptReload()`, `RecreateScriptObjectAfterScriptReload()`, `RestoreDataAfterScriptReload()`, `NotifyScriptReloadFinished()`.

The association between a native object and its wrapper is set up automatically when `ScriptObjectWrapper::Create()` is called.

# Managed wrapper object
Creating the script interop class is one half of the job done. You also need to create the managed counterpart, however that is significantly simpler.

Every managed script object must implement the **ScriptObject** interface. For example a C# version of the class we're using in this example would look like:
~~~~~~~~~~~~~{.cs}
namespace b3d
{
	public class MyObject : ScriptObject
	{
		public MyObject()
		{
			Internal_CreateInstance(this)
		}

		public float SomeValue
		{
			get { return Internal_GetSomeValue(GetCachedPtr()); }
			set { Internal_SetSomeValue(GetCachedPtr(), value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_CreateInstance(MyObject obj);

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetSomeValue(IntPtr obj);

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSomeValue(IntPtr obj, float value);
	}
}
~~~~~~~~~~~~~

All managed **ScriptObject**%s provide a **ScriptObject::GetCachedPtr()** method which returns an **IntPtr** which points to the script interop object. This is the preferred approach for passing a reference to the native wrapper — the corresponding C++ function then accepts the script object wrapper directly:

~~~~~~~~~~~~~{.cpp}
float ScriptMyObject::internal_GetSomeValue(ScriptMyObject* self)
{
	// Access the native object directly through the wrapper
	return self->GetNativeObjectAsShared()->GetSomeValue();
}
~~~~~~~~~~~~~

# Assembly refresh
Assembly refresh is the process that happens when managed code is recompiled and scripts need to be reloaded. This is primarily used in Banshee 3D editor to hot-reload the scripts while the editor is still running. When assembly refresh happens all managed objects are effectively destroyed.

By default any script objects of such managed objects are destroyed as well. In many cases this is okay, for example GUI elements don't persist refresh, because they're just rebuilt from the managed code every time the refresh happens. However objects like resources, scene objects and components are persistent - we don't wish to reload the entire scene and all resources every time assembly refresh happens.

Specialized wrappers like **TScriptGameObjectWrapper** and **TScriptResourceWrapper** automatically persist through reload (their `ShouldPersistScriptReload()` returns `true`). For other wrapper types, override the method:

~~~~~~~~~~~~~{.cpp}
class ScriptMyObject : public TScriptReflectableWrapper<MyObject, ScriptMyObject>
{
	bool ShouldPersistScriptReload() const override { return true; }
	...
};
~~~~~~~~~~~~~

This ensures that your object is treated properly during assembly refresh. Persistent objects then need to handle four different actions, represented by overrideable methods. These methods are called in the order specified, during assembly refresh.
 - `BackupDataBeforeScriptReload()` - Called just before the refresh starts. The object is still alive here and you can use this time to save the contents of the managed object before it is destroyed. Returns a `TOptional<ScriptObjectReloadPersistentData>` containing any data to carry across the reload.
 - `NotifyScriptObjectDestroyed(bool isDestroyedDueToScriptReload)` - Called after assembly unload happened and the managed object was destroyed. Override this to prevent the **TScriptObjectWrapper** itself from being deleted when `isDestroyedDueToScriptReload` is `true`. If a reload is not in progress, clean up normally.
 - `RecreateScriptObjectAfterScriptReload()` - Creates the managed instance after new assemblies are loaded. By default this calls `CreateScriptObject(true)` and binds it. The native object's **IScriptExportable** interface is also given an opportunity to handle recreation.
 - `RestoreDataAfterScriptReload(const ScriptObjectReloadPersistentData& data)` - Called after all assemblies are loaded, and after all script interop objects were either destroyed (non-persistent) or had their managed instances re-created (persistent). Restore here any data saved during `BackupDataBeforeScriptReload()`.

Note that both **ScriptObjectWrapper** and **IScriptExportable** have reload callbacks. The wrapper-side callbacks manage the script object lifecycle, while the native-side callbacks on **IScriptExportable** allow the native object to participate in the reload process independently of the wrapper.
 
# Deriving from TScriptObjectWrapper
Sometimes script objects are polymorphic. For example a **GUIElement** is derived from **ScriptObject** in managed code, and **GUIButton** is derived from **GUIElement**, however they both have script interop objects of their own.

Due to the nature of how script interop objects are defined we cannot follow the same simple chain of inheritance in C++ code. For example class definition for the script interop object for **GUIElement** would be:
~~~~~~~~~~~~~{.cpp}
class ScriptGUIElement : public TScriptObjectWrapper<ScriptGUIElement>
{
public:
	B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIElement")
...
}
~~~~~~~~~~~~~

But what would it be for **GUIButton**? It also needs to implement **TScriptObjectWrapper** with its own **B3D_SCRIPT_TYPE_DEFINITION** macro so we cannot just inherit from **ScriptGUIElement** directly as it would clash.

The solution is to create a third class that will serve as a base for both. This third class will be a base class for **TScriptObjectWrapper** (its second template parameter allows us to override its default **ScriptObjectWrapper** base class). The third class will need to inherit @b3d::ScriptObjectWrapper and can implement any functionality common to all GUI elements (e.g. it might store a pointer to a native **GUIElement**).

Provided our common base class is defined as such:
~~~~~~~~~~~~~{.cpp}
class ScriptGUIElementBase : public ScriptObjectWrapper
{
	// Functionality common to all GUI elements
};
~~~~~~~~~~~~~

Then we can define the script interop object for **GUIElement** as:
~~~~~~~~~~~~~{.cpp}
class ScriptGUIElement : public TScriptObjectWrapper<ScriptGUIElement, ScriptGUIElementBase>
{
public:
	B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIElement")
...
};
~~~~~~~~~~~~~

And the interop object for **GUIButton** would then be:
~~~~~~~~~~~~~{.cpp}
class ScriptGUIButton : public TScriptObjectWrapper<ScriptGUIButton, ScriptGUIElementBase>
{
public:
	B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIButton")
...
};
~~~~~~~~~~~~~

This ensures that all GUI elements can now be accessed through the common **ScriptGUIElementBase** interface. This is important if **GUIElement** provides some internal method calls shared between all GUI element types, otherwise we wouldn't know what to cast the interop object held by its managed object to.

Note that the specialized wrappers (**TScriptReflectableWrapper**, **TScriptResourceWrapper**, etc.) already follow this pattern — they use their non-template base class (e.g. **ScriptReflectableWrapper**) as the common base, and all derived types share it via the third template parameter.
