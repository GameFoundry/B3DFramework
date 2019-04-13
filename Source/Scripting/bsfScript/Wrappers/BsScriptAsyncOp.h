//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Threading/BsAsyncOp.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for AsyncOpBase and AsyncOp<T>. */
	class BS_SCR_BE_EXPORT ScriptAsyncOpBase : public ScriptObject<ScriptAsyncOpBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "AsyncOpBase")

		/** Creates a new managed AsyncOp<T> from a native TAsyncOp. */
		template<class T>
		static MonoObject* create(const TAsyncOp<T>& op, const std::function<MonoObject*(const Any&)>& convertCallback)
		{
			return createInternal(op, convertCallback, TAsyncOp<T>::ReturnValueType::getRTTIStatic()->getRTTIId());
		}

		/** Creates a new managed AsyncOp<T> from a native TAsyncOp. */
		template<>
		static MonoObject* create(const TAsyncOp<Any>& op, const std::function<MonoObject*(const Any&)>& convertCallback)
		{
			return createInternal(op, convertCallback);
		}

		/** Creates a AsyncOp type with the provided class bound as its template parameter. */
		static ::MonoClass* bindGenericParam(::MonoClass* param);

	private:
		ScriptAsyncOpBase(MonoObject* instance, const AsyncOpBase& op, 
			const std::function<MonoObject*(const Any&)>& convertCallback);

		/** @copydoc create() */
		static MonoObject* createInternal(const AsyncOpBase& op, 
			const std::function<MonoObject*(const Any&)>& convertCallback, UINT32 rttiId);

		/** @copydoc create() */
		static MonoObject* createInternal(const AsyncOpBase& op, 
			const std::function<MonoObject*(const Any&)>& convertCallback);

		AsyncOpBase mOp;
		std::function<MonoObject*(const Any&)> mConvertCallback;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static bool internal_IsComplete(ScriptAsyncOpBase* thisPtr);
		static void internal_BlockUntilComplete(ScriptAsyncOpBase* thisPtr);
		static MonoObject* internal_GetValue(ScriptAsyncOpBase* thisPtr);
	};

	/** @} */
}