//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2026 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "B3DFolderMonitor.h"

using namespace b3d;

void FolderMonitorManager::Update()
{
	for(auto& monitor : mMonitors)
		monitor->Update();
}

void FolderMonitorManager::RegisterMonitor(FolderMonitor* monitor)
{
	mMonitors.insert(monitor);
}

void FolderMonitorManager::UnregisterMonitor(FolderMonitor* monitor)
{
	mMonitors.erase(monitor);
}
