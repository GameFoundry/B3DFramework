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
#include "Serialization/BsScriptAssemblyManager.h"
#include "BsManagedResource.h"
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

ScriptManagedResource* ScriptResourceManager::CreateManagedScriptResource(const HManagedResource& resource, MonoObject* instance)
{
	const UUID& uuid = resource.GetId();
#if B3D_DEBUG
	ThrowExceptionIfInvalidOrDuplicateInternal(uuid);
#endif

	ScriptManagedResource* scriptResource = new(B3DAllocate<ScriptManagedResource>()) ScriptManagedResource(instance, resource);
	mScriptResources[uuid] = scriptResource;

	return scriptResource;
}

ScriptResourceBase* ScriptResourceManager::CreateBuiltinScriptResource(const HResource& resource, MonoObject* instance)
{
	const UUID& uuid = resource.GetId();
#if B3D_DEBUG
	ThrowExceptionIfInvalidOrDuplicateInternal(uuid);
#endif

	if(!resource.IsLoaded(false))
		return nullptr;

	u32 rttiId = resource->GetRtti()->GetRttiId();
	BuiltinResourceInfo* info = ScriptAssemblyManager::Instance().GetBuiltinResourceInfo(rttiId);

	if(info == nullptr)
		return nullptr;

	ScriptResourceBase* scriptResource = info->CreateCallback(resource, instance);
	mScriptResources[uuid] = scriptResource;

	return scriptResource;
}

ScriptResourceBase* ScriptResourceManager::GetScriptResource(const HResource& resource, bool create)
{
	const UUID& uuid = resource.GetId();

	if(uuid.Empty())
		return nullptr;

	ScriptResourceBase* output = GetScriptResource(uuid);

	if(output == nullptr && create)
		return CreateBuiltinScriptResource(resource);

	return output;
}

ScriptResourceBase* ScriptResourceManager::GetScriptResource(const UUID& uuid)
{
	if(uuid.Empty())
		return nullptr;

	auto findIter = mScriptResources.find(uuid);
	if(findIter != mScriptResources.end())
		return findIter->second;

	return nullptr;
}

ScriptRRefBase* ScriptResourceManager::GetScriptRRef(const HResource& resource, ::MonoClass* rrefClass)
{
	UnorderedMap<UUID, ScriptRRefBase*>& rrefs = mScriptRRefsPerType[rrefClass];
	const auto iterFind = rrefs.find(resource.GetId());
	if(iterFind != rrefs.end())
		return iterFind->second;

	ScriptRRefBase* newRRef = ScriptRRefBase::Create(resource, rrefClass);
	rrefs[resource.GetId()] = newRRef;

	return newRRef;
}

void ScriptResourceManager::DestroyScriptResource(ScriptResourceBase* resource)
{
	HResource resourceHandle = resource->GetGenericHandle();
	const UUID& uuid = resourceHandle.GetId();

	if(uuid.Empty())
		B3D_EXCEPT(InvalidParametersException, "Provided resource handle has an undefined resource UUID.");

#if B3D_DEBUG
	for(auto& kvp : mScriptRRefsPerType)
	{
		UnorderedMap<UUID, ScriptRRefBase*>& rrefs = kvp.second;

		// No handles should exist at this point because we only manually free the ScriptResourceBase object if the
		// native resource is destroyed, which we handle in onResourceDestroyed. And only other destruction should
		// happen during assembly refresh, which we handled in clearRRefs().
		const auto iterFind = rrefs.find(uuid);
		B3D_ASSERT(iterFind == rrefs.end());
	}
#endif

	(resource)->~ScriptResourceBase();
	MemoryAllocator<DefaultAllocatorTag>::Free(resource);

	mScriptResources.erase(uuid);
}

void ScriptResourceManager::OnResourceDestroyed(const UUID& uuid)
{
	for(auto& kvp : mScriptRRefsPerType)
	{
		UnorderedMap<UUID, ScriptRRefBase*>& rrefs = kvp.second;

		const auto iterFind = rrefs.find(uuid);
		if(iterFind != rrefs.end())
			iterFind->second->ClearResource();
	}

	auto findIter = mScriptResources.find(uuid);
	if(findIter != mScriptResources.end())
	{
		findIter->second->NotifyResourceDestroyed();
		mScriptResources.erase(findIter);
	}
}

void ScriptResourceManager::ClearRRefs()
{
	mScriptRRefsPerType.clear();
}

void ScriptResourceManager::ThrowExceptionIfInvalidOrDuplicateInternal(const UUID& uuid) const
{
	if(uuid.Empty())
		B3D_EXCEPT(InvalidParametersException, "Provided resource handle has an undefined resource UUID.");

	auto findIter = mScriptResources.find(uuid);
	if(findIter != mScriptResources.end())
	{
		B3D_EXCEPT(InvalidStateException, "Provided resource handle already has a script resource. \
											 Retrieve the existing instance instead of creating a new one.");
	}
}
