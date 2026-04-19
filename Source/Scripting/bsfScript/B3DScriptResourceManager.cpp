//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptResourceManager.h"
#include "B3DMonoManager.h"
#include "B3DMonoAssembly.h"
#include "B3DMonoClass.h"
#include "B3DScriptObjectManager.h"
#include "Resources/B3DResources.h"
#include "Reflection/B3DRTTIType.h"
#include "Resources/B3DResource.h"
#include "Wrappers/B3DScriptManagedResource.h"
#include "Wrappers/B3DScriptRRefBase.h"

using namespace b3d;
ScriptResourceManager::ScriptResourceManager()
{
	mResourceDestroyedConn = GetResources().OnResourceDestroyed.Connect([this](const UUID& uuid) { OnResourceDestroyed(uuid); });
	mOnWillUnloadAssembliesConnection = ScriptObjectManager::Instance().OnWillUnloadAssemblies.Connect([this]() { ClearRRefs(); });
}

ScriptResourceManager::~ScriptResourceManager()
{
	mOnWillUnloadAssembliesConnection.Disconnect();
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

void ScriptResourceManager::NotifyScriptRRefScriptObjectDestroyed(ScriptRRefBase* scriptRRef)
{
	if(!B3D_ENSURE(scriptRRef != nullptr))
		return;

	const UUID& resourceId = scriptRRef->GetNativeObject().GetId();

	// Note: This can be faster if I determine the resource reference type first
	for(auto& entry : mScriptRRefsPerType)
	{
		UnorderedMap<UUID, ScriptRRefBase*>& resourceReferencesById = entry.second;

		const auto found = resourceReferencesById.find(resourceId);
		if(found != resourceReferencesById.end())
			resourceReferencesById.erase(found);
	}
}

void ScriptResourceManager::ClearRRefs()
{
	mScriptRRefsPerType.clear();
}
