//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	class ParticleEmitterShape;

	class BS_SCR_BE_EXPORT ScriptParticleEmitterShapeBase : public ScriptObjectBase
	{
	public:
		ScriptParticleEmitterShapeBase(MonoObject* instance);
		virtual ~ScriptParticleEmitterShapeBase() {}

		SPtr<ParticleEmitterShape> getInternal() const { return mInternal; }
	protected:
		SPtr<ParticleEmitterShape> mInternal;
	};

	class BS_SCR_BE_EXPORT ScriptParticleEmitterShape : public ScriptObject<ScriptParticleEmitterShape, ScriptParticleEmitterShapeBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleEmitterShape")

		ScriptParticleEmitterShape(MonoObject* managedInstance, const SPtr<ParticleEmitterShape>& value);

		SPtr<ParticleEmitterShape> getInternal() const;
		static MonoObject* create(const SPtr<ParticleEmitterShape>& value);

	private:
	};
}
