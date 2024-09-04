//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptResourceManager.h"
#include "BsMonoManager.h"
#include "BsMonoAssembly.h"
#include "BsMonoClass.h"
#include "Resources/BsResources.h"
#include "Reflection/BsRTTIType.h"
#include "Resources/BsResource.h"
#include "Wrappers/BsScriptManagedResource.h"
#include "Wrappers/BsScriptRRefBase.h"

using namespace std::placeholders;

using namespace bs;
ScriptResourceManager::ScriptResourceManager()
{
	mResourceDestroyedConn = GetResources().OnResourceDestroyed.Connect(std::bind(&ScriptResourceManager::OnResourceDestroyed, this, _1));
	mDomainUnloadedConn = MonoManager::Instance().OnDomainUnload.Connect(std::bind(&ScriptResourceManager::ClearRRefs, this));
}

ScriptResourceManager::~ScriptResourceManager()
{
	mDomainUnloadedConn.Disconnect();
	mResourceDestroyedConn.Disconnect();
}

ScriptRRefBase* ScriptResourceManager::GetScriptRRef(const HResource& resource, ::MonoClass* rrefClass)
{
	UnorderedMap<UUID, ScriptRRefBase*>& rrefs = mScriptRRefsPerType[rrefClass];
	const auto iterFind = rrefs.find(resource.GetId());
	if(iterFind != rrefs.end())
		return iterFind->second;

	MonoObject* const referenceScriptObject = ScriptRRefBase::CreateScriptObject(resource, rrefClass);
	ScriptRRefBase* const referenceScriptWrapper = ScriptRRefBase::GetScriptObjectWrapper(referenceScriptObject);

	rrefs[resource.GetId()] = referenceScriptWrapper;

	return referenceScriptWrapper;
}

void ScriptResourceManager::OnResourceDestroyed(const UUID& uuid)
{
	for(auto& entry : mScriptRRefsPerType)
	{
		UnorderedMap<UUID, ScriptRRefBase*>& resourceReferencesById = entry.second;

		const auto found = resourceReferencesById.find(uuid);
		if(found != resourceReferencesById.end())
		{
			ScriptRRefBase* const scriptReferenceWrapper = found->second;
			scriptReferenceWrapper->NotifyNativeObjectDestroyed();

			resourceReferencesById.erase(found);
		}
	}
}

void ScriptResourceManager::ClearRRefs()
{
	mScriptRRefsPerType.clear();
}
