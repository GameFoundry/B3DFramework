//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	class ImportOptions;

#if !BS_IS_BANSHEE3D
	class BS_SCR_BE_EXPORT ScriptImportOptionsBase : public ScriptObjectBase
	{
	public:
		ScriptImportOptionsBase(MonoObject* instance);
		virtual ~ScriptImportOptionsBase() {}

		SPtr<ImportOptions> getInternal() const { return mInternal; }
	protected:
		SPtr<ImportOptions> mInternal;
	};

	class BS_SCR_BE_EXPORT ScriptImportOptions : public ScriptObject<ScriptImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ImportOptions")

		ScriptImportOptions(MonoObject* managedInstance, const SPtr<ImportOptions>& value);

		SPtr<ImportOptions> getInternal() const;
		static MonoObject* create(const SPtr<ImportOptions>& value);

	private:
	};
#endif
}
