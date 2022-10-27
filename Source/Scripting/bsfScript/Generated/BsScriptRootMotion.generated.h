//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"

namespace bs
{
	struct RootMotion;
}

namespace bs
{
	class RootMotionEx;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptRootMotion : public ScriptObject<ScriptRootMotion>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "RootMotion")

		ScriptRootMotion(MonoObject* managedInstance, const SPtr<RootMotion>& value);

		SPtr<RootMotion> GetInternal() const { return mInternal; }

		static MonoObject* Create(const SPtr<RootMotion>& value);

	private:
		SPtr<RootMotion> mInternal;

		static MonoObject* InternalGetPositionCurves(ScriptRootMotion* thisPtr);
		static MonoObject* InternalGetRotationCurves(ScriptRootMotion* thisPtr);
	};
} // namespace bs
