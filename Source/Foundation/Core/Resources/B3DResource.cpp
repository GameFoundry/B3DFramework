//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/B3DResource.h"
#include "RTTI/B3DResourceRTTI.h"
#include "Resources/B3DResourceMetaData.h"
#include "Script/B3DIScriptObjectWrapper.h"

using namespace b3d;

Resource::Resource(bool createRenderProxy, const String& name)
	: CoreObject(createRenderProxy), mId(UUIDGenerator::GenerateRandom()), mName(name), mKeepSourceData(true)
{
	mMetaData = B3DMakeShared<ResourceMetaData>();
}

void Resource::Destroy()
{
	IScriptObjectWrapper* const scriptObjectWrapper = GetScriptObjectWrapper();
	if(scriptObjectWrapper != nullptr)
		scriptObjectWrapper->NotifyNativeObjectDestroyed();

	ClearAssociatedScriptObjectWrapper();
	CoreObject::Destroy();
}

void Resource::GetResourceDependencies(FrameVector<HResource>& dependencies) const
{
	Lock lock(mDependenciesMutex);

	for(auto& dependency : mDependencies)
	{
		if(dependency != nullptr)
			dependencies.push_back(B3DStaticResourceCast<Resource>(dependency));
	}
}

bool Resource::AreDependenciesLoaded() const
{
	Lock lock(mDependenciesMutex);
	B3DMarkAllocatorFrame();

	bool areLoaded = true;
	{
		for(auto& dependency : mDependencies)
		{
			if(dependency != nullptr && !dependency.IsLoaded())
			{
				areLoaded = false;
				break;
			}
		}
	}

	B3DClearAllocatorFrame();
	return areLoaded;
}

void Resource::AddResourceDependency(const HResource& resource)
{
	if(resource == nullptr)
		return;

	Lock lock(mDependenciesMutex);
	mDependencies.push_back(resource.GetWeak());
}

void Resource::RemoveResourceDependency(const HResource& resource)
{
	Lock lock(mDependenciesMutex);
	mDependencies.erase(std::remove(mDependencies.begin(), mDependencies.end(), resource.GetWeak()), mDependencies.end());
}

RTTIType* Resource::GetRttiStatic()
{
	return ResourceRTTI::Instance();
}

RTTIType* Resource::GetRtti() const
{
	return Resource::GetRttiStatic();
}
