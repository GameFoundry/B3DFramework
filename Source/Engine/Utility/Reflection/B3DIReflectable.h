//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Utility/B3DAny.h"

namespace b3d
{
	/** @addtogroup RTTI
	 *  @{
	 */

	/** Flags that may be set on IReflectable objects. */
	enum class ReflectableObjectFlag
	{
		IsDefaultObject = 1 << 0,
	};

	using ReflectableObjectFlags = Flags<ReflectableObjectFlag>;
	B3D_FLAGS_OPERATORS(ReflectableObjectFlag)

	/**
	 * Interface implemented by classes that provide run time type information.
	 *
	 * @note
	 * Any class implementing this interface must implement the GetRtti() method, as well as a static GetRttiStatic()
	 * method, returning the same value as GetRtti(). Object returned by those methods is used for retrieving actual RTTI
	 * data about the class.
	 */
	class B3D_EXPORT IReflectable
	{
	public:
		virtual ~IReflectable() = default;

		/**
		 * Returns an interface you can use to access class' Run Time Type Information.
		 *
		 * @note
		 * You must derive your own version of RTTITypeBase, in which you may encapsulate all reflection specific operations.
		 */
		virtual RTTIType* GetRtti() const = 0;

		/** Returns true if current RTTI class is derived from @p base (Or if it is the same type as base). */
		bool IsDerivedFrom(const RTTIType* base) const;

		/** Returns an unique type identifier of the class. */
		u32 GetTypeId() const;

		/**
		 * Returns the type name of the class.
		 *
		 * @note	Name is not necessarily unique.
		 */
		const String& GetTypeName() const;

		/** Returns mutable flags specific to reflectable objects. */
		ReflectableObjectFlags& GetReflectableObjectFlags() { return mReflectableObjectFlags; }

		/** Returns flags specific to reflectable objects. */
		const ReflectableObjectFlags& GetReflectableObjectFlags() const { return mReflectableObjectFlags; }

		/** Casts this to T. Returns null if unable to cast. */
		template<class T>
		T* As()
		{
			if(IsDerivedFrom(T::GetRttiStatic()))
				return (T*)this;

			return nullptr;
		}

		/** Casts this to T. Returns null if unable to cast. */
		template<class T>
		const T* As() const
		{
			if(IsDerivedFrom(T::GetRttiStatic()))
				return (const T*)this;

			return nullptr;
		}

		/** Checks if the type can be cast to T. */
		template<class T>
		bool Is() const
		{
			return IsDerivedFrom(T::GetRttiStatic());
		}

		/** Creates an empty instance of a class from a type identifier. */
		static SPtr<IReflectable> CreateInstanceFromTypeId(u32 rttiTypeId);

		/** Returns all available RTTI types. */
		static UnorderedMap<u32, RTTIType*>& GetAllRttiTypes()
		{
			static UnorderedMap<u32, RTTIType*> mAllRTTITypes;
			return mAllRTTITypes;
		}

		/** Returns class' RTTI type from type id. */
		static RTTIType* GetRTTITypeFromTypeId(u32 rttiTypeId);

		/** @name Internal
		 *  @{
		 */

		/** Called by each type implementing RTTITypeBase, on program load. */
		static void RegisterRTTITypeInternal(RTTIType* rttiType);

		/** Checks if the provided type id is unique. */
		static bool IsTypeIdDuplicateInternal(u32 typeId);

		/** Returns an interface you can use to access class' Run Time Type Information. */
		static RTTIType* GetRttiStatic();

		/** @} */

	private:
		B3D_SCRIPT_EXPORT(Exclude(true))
		ReflectableObjectFlags mReflectableObjectFlags;

	};

	/** @} */
} // namespace b3d
