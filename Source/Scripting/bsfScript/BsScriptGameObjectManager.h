//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Utility/BsModule.h"

namespace bs
{
	class ScriptRenderable;

	/** @addtogroup bsfScript
	 *  @{
	 */

	/**
	 * Manages all active GameObject interop objects. GameObjects can be created from native code and used in managed code
	 * therefore we need to keep a dictionary or all the native objects we have mapped to managed objects.
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGameObjectManager : public Module<ScriptGameObjectManager>
		// TODO - This should be refactored to ScriptGameObjectCollection, as currently UUIDs from all native game objects will be referenced here
	{
	public:
		ScriptGameObjectManager();
		~ScriptGameObjectManager();

	private:
		/**
		 * Triggers OnReset methods on all registered managed components.
		 *
		 * @note	Usually this happens after an assembly reload.
		 */
		void SendComponentResetEvents();

		HEvent mOnAssemblyReloadDoneConn;
	};

	/** @} */
} // namespace bs
