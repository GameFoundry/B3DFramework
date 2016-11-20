//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsBinaryDiff.h"

namespace bs
{
	/** @addtogroup SBansheeEngine
	 *  @{
	 */

	/**
	 * Diff handler that performs RTTI object diff for managed objects. Managed objects require special diff handling since
	 * their serialization works differently.
	 */
	class BS_SCR_BE_EXPORT ManagedDiff : public IDiff
	{
	protected:
		/** @copydoc IDiff::generateDiff(const SPtr<SerializedObject>&, const SPtr<SerializedObject>&, ObjectMap&) */
		SPtr<SerializedObject> generateDiff(const SPtr<SerializedObject>& orgSerzObj,
			const SPtr<SerializedObject>& newSerzObj, ObjectMap& objectMap) override;

		/**
		 * @copydoc	IDiff::applyDiff(const SPtr<IReflectable>&, const SPtr<SerializedObject>&, 
		 *			DiffObjectMap&, Vector<DiffCommand>&)
		 */
		void applyDiff(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& serzDiff,
			DiffObjectMap& objectMap, Vector<DiffCommand>& diffCommands) override;
	};

	/** @} */
}