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
	class B3D_SCRIPT_INTEROP_EXPORT ManagedDiff : public IDiff
	{
	protected:
		SPtr<SerializedObject> GenerateDeltaRecursive(IReflectable* orgObj, IReflectable* newObj, ObjectMap& objectMap, bool replicableOnly) override;
		void GenerateDeltaApplyCommands(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& serzDiff, FrameAllocator& alloc, DeltaObjectMap& objectMap, FrameVector<DeltaCommand>& diffCommands, SerializationContext* context) override;
	};

	/** @} */
} // namespace bs
