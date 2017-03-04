//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

/** @addtogroup Plugins
 *  @{
 */

/** @defgroup FBX BansheeFBXImporter
 *	%Mesh importer for the FBX file format.
 */

/** @} */

#define FBXSDK_NEW_API
#include <fbxsdk.h>

#define FBX_IMPORT_MAX_UV_LAYERS 2
#define FBX_IMPORT_MAX_BONE_INFLUENCES 4