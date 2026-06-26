//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

/** @addtogroup Plugins
 *  @{
 */

/** @defgroup MetalGpuBackend MetalGpuBackend
 *	Metal render API implementation (macOS / iOS).
 */

/** @} */

// Metal framework headers are only available to Objective-C++ translation units. Include them here
// guarded so that plain C++ .cpp consumers of the Metal backend headers still compile while .mm
// sources get full access to the Metal types through our backend headers.
#ifdef __OBJC__
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#endif
