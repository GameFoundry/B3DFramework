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

	namespace detail
	{
		template <class T>
		MonoObject* async_op_create(const TAsyncOp<T>& op, const std::function<MonoObject*(const Any&)>& convertCallback);

		template <>
		MonoObject* async_op_create(const TAsyncOp<Any>& op, const std::function<MonoObject*(const Any&)>& convertCallback);

		template <class T>
		MonoObject* async_op_create(const TAsyncOp<T>& op, const std::function<MonoObject*(const Any&)>& convertCallback, MonoClass* returnTypeClass);
	} // namespace detail

	/** @} */

	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for AsyncOpBase and AsyncOp<T>. */
	class BS_SCR_BE_EXPORT ScriptAsyncOpBase : public ScriptObject<ScriptAsyncOpBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "AsyncOpBase")

		/** Creates a new managed AsyncOp<T> from a native TAsyncOp. */
		template <class T>
		static MonoObject* Create(const TAsyncOp<T>& op, const std::function<MonoObject*(const Any&)>& convertCallback)
		{
			return detail::async_op_create(op, convertCallback);
		}

		/**
		 * Creates a new managed AsyncOp<T> from a native TAsyncOp and a managed class representing the return type.
		 * To be used when return type T does not implement IReflectable.
		 */
		template <class T>
		static MonoObject* Create(const TAsyncOp<T>& op, const std::function<MonoObject*(const Any&)>& convertCallback, MonoClass* returnTypeClass)
		{
			return detail::async_op_create(op, convertCallback, returnTypeClass);
		}

		/** Creates a AsyncOp type with the provided class bound as its template parameter. */
		static ::MonoClass* BindGenericParam(::MonoClass* param);

		/**
		 * @name Internal
		 * @{
		 */

		/** @copydoc create() */
		static MonoObject* CreateInternal(const AsyncOpBase& op, const std::function<MonoObject*(const Any&)>& convertCallback, u32 rttiId);

		/** @copydoc create() */
		static MonoObject* CreateInternal(const AsyncOpBase& op, const std::function<MonoObject*(const Any&)>& convertCallback);

		/** @copydoc create() */
		static MonoObject* CreateInternal(const AsyncOpBase& op, const std::function<MonoObject*(const Any&)>& convertCallback, MonoClass* returnTypeClass);

		/** @} */
	private:
		ScriptAsyncOpBase(MonoObject* instance, const AsyncOpBase& op, const std::function<MonoObject*(const Any&)>& convertCallback);

		AsyncOpBase mOp;
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

	namespace detail
	{
		template <class T>
		inline MonoObject* async_op_create(const TAsyncOp<T>& op, const std::function<MonoObject*(const Any&)>& convertCallback)
		{
			return ScriptAsyncOpBase::_create(op, convertCallback, TAsyncOp<T>::ReturnValueType::GetRttiStatic()->GetRttiId());
		}

		template <>
		inline MonoObject* async_op_create(const TAsyncOp<Any>& op, const std::function<MonoObject*(const Any&)>& convertCallback)
		{
			return ScriptAsyncOpBase::CreateInternal(op, convertCallback);
		}

		template <class T>
		inline MonoObject* async_op_create(const TAsyncOp<T>& op, const std::function<MonoObject*(const Any&)>& convertCallback, MonoClass* returnTypeClass)
		{
			return ScriptAsyncOpBase::CreateInternal(op, convertCallback, returnTypeClass);
		}
	} // namespace detail

	/** @} */
} // namespace bs
