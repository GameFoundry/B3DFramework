//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Serialization/BsBinaryDiff.h"

namespace bs
{
	/** @addtogroup bsfScript
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
		 * @copydoc	IDiff::applyDiff(const SPtr<IReflectable>&, const SPtr<SerializedObject>&, FrameAlloc&, DiffObjectMap&, FrameVector<DiffCommand>&, SerializationContext*)
		 */
		void applyDiff(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& serzDiff,
			FrameAlloc& alloc, DiffObjectMap& objectMap, FrameVector<DiffCommand>& diffCommands,
			SerializationContext* context) override;
	};

	/** @} */
}
