//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/B3DScriptOrder.h"

#include "B3DMonoClass.h"

using namespace b3d;
MonoField* ScriptOrder::sIndexField = nullptr;

ScriptOrder::ScriptOrder()
{}

void ScriptOrder::SetupScriptBindings()
{
	sIndexField = sInteropMetaData.ScriptClass->GetField("index");
}
