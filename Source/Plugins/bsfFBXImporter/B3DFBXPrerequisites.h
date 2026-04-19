//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

/** @addtogroup Plugins
 *  @{
 */

/** @defgroup FBX bsfFBXImporter
 *	%Mesh importer for the FBX file format.
 */

/** @} */

#define FBXSDK_NEW_API
#include <fbxsdk.h>

#define FBX_IMPORT_MAX_UV_LAYERS 2
#define FBX_IMPORT_MAX_BONE_INFLUENCES 4
