//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMonoPrerequisites.h"

namespace b3d
{
	/** @addtogroup Mono
	 *  @{
	 */

	/**	Contains information about a single Mono (managed) class. */
	class B3D_MONO_EXPORT MonoClass
	{
		/** @cond INTERNAL */

		/** Used for uniquely identifying a method in a managed class, normally for use in containers. */
		struct MethodId
		{
			struct Hash
			{
				size_t operator()(const MethodId& v) const;
			};

			struct Equals
			{
				bool operator()(const MethodId& a, const MethodId& b) const;
			};

			MethodId(const String& name, u32 numParams);

			String Name;
			u32 NumParams;
		};

		/** @endcond */

	public:
		~MonoClass();

		/**	Returns the namespace of this class. */
		const String& GetNamespace() const { return mNamespace; }

		/**	Returns the type name of this class. */
		const String& GetTypeName() const { return mTypeName; }

		/**	Returns the full name (Namespace::TypeName) of this class. */
		const String& GetFullName() const { return mFullName; }

		/** Returns the assembly this class is a part of. */
		const MonoAssembly* GetAssembly() const { return mParentAssembly; }

		/**
		 * Returns an object referencing a method with the specified name and number of parameters.
		 *
		 * @note
		 * If the method is overloaded then you should use getMethodExact().
		 * Does not query base class methods.
		 * Returns null if method cannot be found.
		 */
		MonoMethod* GetMethod(const String& name, u32 numParams = 0) const;

		/**
		 * Returns an object referencing a field with the specified name.
		 *
		 * @note
		 * Does not query base class fields.
		 * Returns null if field cannot be found.
		 */
		MonoField* GetField(const String& name) const;

		/**
		 * Returns an object referencing a property with the specified name.
		 *
		 * @note
		 * Does not query base class properties.
		 * Returns null if property cannot be found.
		 */
		MonoProperty* GetProperty(const String& name) const;

		/**
		 * Returns an instance of an attribute of the specified @p monoClass that is part of this class. Returns null if
		 * this class type does not have that type of attribute.
		 */
		MonoObject* GetAttribute(MonoClass* monoClass) const;

		/**	Returns the base class of this class. Null if this class has no base. */
		MonoClass* GetBaseClass() const;

		/**
		 * Returns an object referencing a method, expects exact method name with parameters.
		 *
		 * @note
		 * Does not query base class methods.
		 * Returns null if method cannot be found.
		 * Example: name = "CreateInstance", signature = "Vector2,int[]"
		 */
		MonoMethod* GetMethodExact(const String& name, const String& signature) const;

		/**
		 * Returns all fields belonging to this class.
		 *
		 * @note	Be aware this will not include the fields of any base classes.
		 */
		const Vector<MonoField*>& GetAllFields() const;

		/**
		 * Returns all properties belonging to this class.
		 *
		 * @note	Be aware this will not include the properties of any base classes.
		 */
		const Vector<MonoProperty*>& GetAllProperties() const;

		/**
		 * Returns all methods belonging to this class.
		 *
		 * @note	Be aware this will not include the methods of any base classes.
		 */
		const Vector<MonoMethod*>& GetAllMethods() const;

		/**	Gets all attributes applied to this class. */
		Vector<MonoClass*> GetAllAttributes() const;

		/**	Check if this class has an attribute of the type @p monoClass. */
		bool HasAttribute(MonoClass* monoClass) const;

		/**	Check if this class has a field with the specified name. Does not check base classes. */
		bool HasField(const String& name) const;

		/**	Checks if this class is a sub class of the specified class. */
		bool IsSubClassOf(const MonoClass* monoClass) const;

		/**	Checks is the provided object instance of this class' type. */
		bool IsInstanceOfType(MonoObject* object) const;

		/** Returns the size of an instance of this class, in bytes. */
		u32 GetInstanceSize() const;

		/**
		 * Shortcut for invoking a method on a class. Invokes a method with the provided name and number of parameters.
		 *
		 * @param[in]	name		Name of the method to invoke (no parameter list or brackets.
		 * @param[in]	instance	Object instance on invoke the method on. Null if method is static.
		 * @param[in]	params		Array containing pointers to method parameters. Array length must be equal to number of
		 *							parameters. Can be null if method has no parameters. For value types parameters should
		 *							be pointers to the values and for reference types they should be pointers to MonoObject.
		 * @param[in]	numParams	Number of parameters the method accepts.
		 *
		 * @note
		 * You cannot use this to call overloaded methods that have the same number of parameters. Use getMethodExact() and
		 * then invoke the method from the returned method object.
		 */
		MonoObject* InvokeMethod(const String& name, MonoObject* instance = nullptr, void** params = nullptr, u32 numParams = 0);

		/**
		 * Hooks up an internal call that will trigger the provided method callback when the managed method with the
		 * specified name is called. If name is not valid this will silently fail.
		 */
		void AddInternalCall(const String& name, const void* method);

		/**
		 * Creates a new instance of this class and optionally constructs it. If you don't construct the instance then you
		 * should invoke the ".ctor" method manually afterwards.
		 */
		MonoObject* CreateInstance(bool construct = true) const;

		/**
		 * Creates a new instance of this class and then constructs it using the constructor with the specified number of
		 * parameters.
		 *
		 * @param[in]	params		Array containing pointers to constructor parameters. Array length must be equal to
		 *							number of parameters.
		 * @param[in]	numParams	Number of parameters the constructor accepts.
		 *
		 * @note	If the class have multiple constructors with the same number of parameters use the other
		 *			createInstance(const String&, void**) overload that allows you to provide exact signature.
		 */
		MonoObject* CreateInstance(void** params, u32 numParams);

		/**
		 * Creates a new instance of this class and then constructs it using the constructor with the specified signature.
		 *
		 * @param[in]	ctorSignature	Method signature. Example: "Vector2,int[]"
		 * @param[in]	params			Array containing pointers to constructor parameters. Array length must be equal to
		 *								number of parameters.
		 */
		MonoObject* CreateInstance(const String& ctorSignature, void** params);

		/**	Returns the internal mono representation of the class. */
		::MonoClass* GetInternalClass() const { return mClass; }

		/** Invokes the parameterless constructor on the provided object. */
		static void Construct(MonoObject* object);

	private:
		friend class MonoAssembly;

		/**
		 * Constructs a new mono class object.
		 *
		 * @param[in]	ns				Namespace the class belongs to.
		 * @param[in]	type			Type name of the class.
		 * @param[in]	monoClass		Internal mono class.
		 * @param[in]	parentAssembly	Assembly to which this class belongs.
		 */
		MonoClass(const String& ns, const String& type, ::MonoClass* monoClass, const MonoAssembly* parentAssembly);

		const MonoAssembly* mParentAssembly;
		::MonoClass* mClass;
		String mNamespace;
		String mTypeName;
		String mFullName;

		mutable UnorderedMap<MethodId, MonoMethod*, MethodId::Hash, MethodId::Equals> mMethods;
		mutable UnorderedMap<String, MonoField*> mFields;
		mutable UnorderedMap<String, MonoProperty*> mProperties;

		mutable bool mHasCachedFields;
		mutable Vector<MonoField*> mCachedFieldList;

		mutable bool mHasCachedProperties;
		mutable Vector<MonoProperty*> mCachedPropertyList;

		mutable bool mHasCachedMethods;
		mutable Vector<MonoMethod*> mCachedMethodList;
	};

	/** @} */
} // namespace b3d
