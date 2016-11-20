//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsMonoPrerequisites.h"

namespace bs
{
	/** @addtogroup Mono
	 *  @{
	 */

	/**	Contains information about a single Mono (managed) assembly. */
	class BS_MONO_EXPORT MonoAssembly
	{
	public:
		/** @cond INTERNAL */
		/**	Used for uniquely identifying a managed class, normally for use in containers. */
		struct ClassId
		{
			struct Hash
			{
				size_t operator()(const ClassId& v) const;
			};

			struct Equals
			{
				bool operator()(const ClassId &a, const ClassId &b) const;
			};

			ClassId(const String& namespaceName, String name, ::MonoClass* genericInstance = nullptr);

			String namespaceName;
			String name;
			::MonoClass* genericInstance;
		};
		/** @endcond */

	public:
		virtual ~MonoAssembly();

		/**	Returns the name of this assembly. */
		const String& getName() const { return mName; }

		/**
	     * Attempts to find a managed class with the specified namespace and name in this assembly. Returns null if one
		 * cannot be found.
	     */
		MonoClass* getClass(const String& namespaceName, const String& name) const;

		/**	Converts an internal mono representation of a class into engine class. */
		MonoClass* getClass(::MonoClass* rawMonoClass) const;

		/**	Returns a list of all classes in the assembly. */
		const Vector<MonoClass*>& getAllClasses() const;

		/**
	     * Invokes a zero-parameter static method in the format "Class::Method". Used primarily for invoking an assembly
		 * entry point.
	     */
		void invoke(const String& functionName);

	private:
		friend class MonoManager;

		MonoAssembly(const WString& path, const String& name);

		/**
	     * Attempts to find a managed class with the specified namespace and name in this assembly. Registers a new class
		 * using the provided raw class if one cannot be found. Returns null provided raw class is null.
	     */
		MonoClass* getClass(const String& namespaceName, const String& name, ::MonoClass* rawMonoClass) const;

		/**	Loads an assembly into the specified domain. */
		void load(MonoDomain* domain);

		/**
		 * Initializes an assembly from an internal mono image.
		 *
		 * @note	
		 * Normally used for assemblies that were already loaded by the managed runtime as dependencies.
		 */
		void loadFromImage(MonoImage* image);

		/**
		 * Unloads the assembly and all the types associated with it. Caller must ensure not to use any types from this
		 * assembly after it has been unloaded.
		 */
		void unload();

		/**
	     * Returns true if the provided name represents a generic class.
		 *
		 * @note	This method only analyzes the name to determine if it is in generic class format.
	     */
		bool isGenericClass(const String& name) const;

		String mName;
		WString mPath;
		MonoImage* mMonoImage;
		::MonoAssembly* mMonoAssembly;
		UINT8* mDebugData;
		bool mIsLoaded;
		bool mIsDependency;
		
		mutable UnorderedMap<ClassId, MonoClass*, ClassId::Hash, ClassId::Equals> mClasses;
		mutable UnorderedMap<::MonoClass*, MonoClass*> mClassesByRaw;

		mutable bool mHaveCachedClassList;
		mutable Vector<MonoClass*> mCachedClassList;
	};

	/** @} */
}