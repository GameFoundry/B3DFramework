//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsMonoUtil.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	
	 * Base class for all interop object that derive from IReflectable (except for those that have their own specialized
	 * base classes, such as resources and components). 
	 */
	class BS_SCR_BE_EXPORT ScriptReflectableBase : public ScriptObjectBase
	{
	public:
		/**	Returns the wrapped reflectable object. */
		SPtr<IReflectable> getReflectable() const { return mInternal; }

	protected:
		ScriptReflectableBase(MonoObject* instance);
		virtual ~ScriptReflectableBase() = default;

		SPtr<IReflectable> mInternal;
	};

	/**	Base class for a specific reflectable's interop object. */
	template<class ScriptClass, class InternalType, class BaseType = ScriptReflectableBase>
	class BS_SCR_BE_EXPORT TScriptReflectable : public ScriptObject <ScriptClass, BaseType>
	{
	public:
		/** Returns the wrapped reflectable object. */
		SPtr<InternalType> getInternal() const { return std::static_pointer_cast<InternalType>(mInternal); }

	protected:
		TScriptReflectable(MonoObject* instance, const SPtr<IReflectable>& reflectable)
			:ScriptObject<ScriptClass, BaseType>(instance)
		{
			mInternal = reflectable;
		}

		virtual ~TScriptReflectable() = default;
	};

	/** @} */
}
