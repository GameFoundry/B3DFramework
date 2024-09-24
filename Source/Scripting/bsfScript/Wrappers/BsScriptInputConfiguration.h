//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Input/BsInputConfiguration.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for VirtualAxis. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptVirtualAxis : public ScriptObject<ScriptVirtualAxis>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "VirtualAxis")

	private:
		ScriptVirtualAxis(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static u32 InternalInitVirtualAxis(MonoString* name);
	};

	/** @} */
} // namespace bs
