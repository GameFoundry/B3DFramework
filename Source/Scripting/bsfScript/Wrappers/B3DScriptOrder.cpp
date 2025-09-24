//********************************* B3D Framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptOrder.h"

#include "BsMonoClass.h"

using namespace b3d;
MonoField* ScriptOrder::sIndexField = nullptr;

ScriptOrder::ScriptOrder()
{}

void ScriptOrder::SetupScriptBindings()
{
	sIndexField = sInteropMetaData.ScriptClass->GetField("index");
}
