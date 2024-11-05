//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Threading/BsAsyncOp.h"

namespace bs
{
	/** @addtogroup Implementation
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
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAsyncOpBase : public ScriptObject<ScriptAsyncOpBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "AsyncOpBase")

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
		ScriptAsyncOpBase(MonoObject* instance, const AsyncOp& op, const std::function<MonoObject*(const Any&)>& convertCallback);

		AsyncOp mOp;
		std::function<MonoObject*(const Any&)> mConvertCallback;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static bool InternalIsComplete(ScriptAsyncOpBase* thisPtr);
		static void InternalBlockUntilComplete(ScriptAsyncOpBase* thisPtr);
		static MonoObject* InternalGetValue(ScriptAsyncOpBase* thisPtr);
	};

	/** @} */

	/** @addtogroup Implementation
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
} // namespace bs
