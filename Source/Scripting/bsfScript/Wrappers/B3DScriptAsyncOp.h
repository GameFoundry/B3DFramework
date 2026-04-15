//********************************* B3D Framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptValueTypeWrapper.h"
#include "Threading/B3DAsyncOp.h"

namespace b3d
{
	/** @addtogroup ScriptInteropEngine-Internal 
	 * @{
	 */

	template <class T>
	MonoObject* AsyncOpCreate(const TAsyncOp<T>& op, const std::function<MonoObject*(const Any&)>& convertCallback);

	template <>
	MonoObject* AsyncOpCreate(const TAsyncOp<Any>& op, const std::function<MonoObject*(const Any&)>& convertCallback);

	template <class T>
	MonoObject* AsyncOpCreate(const TAsyncOp<T>& op, const std::function<MonoObject*(const Any&)>& convertCallback, MonoClass* returnTypeClass);

	/** @} */

	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for AsyncOpBase and AsyncOp<T>. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAsyncOpBase : public TScriptValueTypeWrapper<AsyncOp, ScriptAsyncOpBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "AsyncOpBase")

		ScriptAsyncOpBase(const AsyncOp& op, const std::function<MonoObject*(const Any&)>& convertCallback);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct)
		{
			return nullptr;
		}

		/** Creates a new managed AsyncOp<T> from a native TAsyncOp. */
		template <class T>
		static MonoObject* Create(const TAsyncOp<T>& op, const std::function<MonoObject*(const Any&)>& convertCallback)
		{
			return AsyncOpCreate(op, convertCallback);
		}

		/**
		 * Creates a new managed AsyncOp<T> from a native TAsyncOp and a managed class representing the return type.
		 * To be used when return type T does not implement IReflectable.
		 */
		template <class T>
		static MonoObject* Create(const TAsyncOp<T>& op, const std::function<MonoObject*(const Any&)>& convertCallback, MonoClass* returnTypeClass)
		{
			return AsyncOpCreate(op, convertCallback, returnTypeClass);
		}

		/** Creates a AsyncOp type with the provided class bound as its template parameter. */
		static ::MonoClass* BindGenericParam(::MonoClass* param);

		/**
		 * @name Internal
		 * @{
		 */

		/** @copydoc Create() */
		static MonoObject* CreateInternal(const AsyncOp& op, const std::function<MonoObject*(const Any&)>& convertCallback, u32 rttiId);

		/** @copydoc Create() */
		static MonoObject* CreateInternal(const AsyncOp& op, const std::function<MonoObject*(const Any&)>& convertCallback);

		/** @copydoc Create() */
		static MonoObject* CreateInternal(const AsyncOp& op, const std::function<MonoObject*(const Any&)>& convertCallback, MonoClass* returnTypeClass);

		/** @} */
	private:
		std::function<MonoObject*(const Any&)> mConvertCallback;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static bool InternalIsComplete(ScriptAsyncOpBase* thisPtr);
		static void InternalBlockUntilComplete(ScriptAsyncOpBase* thisPtr);
		static MonoObject* InternalGetValue(ScriptAsyncOpBase* thisPtr);
	};

	/** @} */

	/** @addtogroup ScriptInteropEngine-Internal
	 * @{
	 */

	template <class T>
	inline MonoObject* AsyncOpCreate(const TAsyncOp<T>& op, const std::function<MonoObject*(const Any&)>& convertCallback)
	{
		return ScriptAsyncOpBase::CreateInternal(op, convertCallback, TAsyncOp<T>::ReturnValueType::GetRttiStatic()->GetRttiId());
	}

	template <>
	inline MonoObject* AsyncOpCreate(const TAsyncOp<Any>& op, const std::function<MonoObject*(const Any&)>& convertCallback)
	{
		return ScriptAsyncOpBase::CreateInternal(op, convertCallback);
	}

	template <class T>
	inline MonoObject* AsyncOpCreate(const TAsyncOp<T>& op, const std::function<MonoObject*(const Any&)>& convertCallback, MonoClass* returnTypeClass)
	{
		return ScriptAsyncOpBase::CreateInternal(op, convertCallback, returnTypeClass);
	}

	/** @} */
} // namespace b3d
