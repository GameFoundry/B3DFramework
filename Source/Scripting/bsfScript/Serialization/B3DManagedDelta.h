//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Serialization/B3DBinaryDelta.h"

namespace b3d
{
	/** @addtogroup bsfScript
	 *  @{
	 */

	/**
	 * Delta handler that performs RTTI object delta for managed objects. Managed objects require special delta handling since their serialization works differently.
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedDeltaHandler : public IDeltaHandler
	{
	protected:
		TShared<SerializedObject> GenerateDeltaRecursive(IReflectable* original, IReflectable* modified, ObjectMap& objectMap, RTTIOperationContext& context, bool replicableOnly) override;
		void GenerateDeltaApplyCommands(const TShared<IReflectable>& object, const TShared<SerializedObject>& delta, FrameAllocator& allocator, DeltaObjectMap& objectMap, FrameVector<DeltaCommand>& inOutDeltaCommands, RTTIOperationContext& context) override;
	};

	/** @} */
} // namespace b3d
