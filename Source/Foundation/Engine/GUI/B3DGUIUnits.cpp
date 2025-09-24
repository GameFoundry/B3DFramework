//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIUnits.h"

using namespace b3d;

template struct B3D_EXPORT TUnitValue<i32, LogicalPixel>;
template struct B3D_EXPORT TUnitValue<float, LogicalPixel>;
template struct B3D_EXPORT TUnitValue<i32, PhysicalPixel>;
template struct B3D_EXPORT TUnitValue<float, PhysicalPixel>;

template struct B3D_EXPORT TVector2<TUnitValue<i32, LogicalPixel>>;
template struct B3D_EXPORT TVector2<TUnitValue<float, LogicalPixel>>;
template struct B3D_EXPORT TVector2<TUnitValue<i32, PhysicalPixel>>;
template struct B3D_EXPORT TVector2<TUnitValue<float, PhysicalPixel>>;

template struct B3D_EXPORT TSize2<TUnitValue<i32, LogicalPixel>>;
template struct B3D_EXPORT TSize2<TUnitValue<float, LogicalPixel>>;
template struct B3D_EXPORT TSize2<TUnitValue<i32, PhysicalPixel>>;
template struct B3D_EXPORT TSize2<TUnitValue<float, PhysicalPixel>>;

template struct B3D_EXPORT TArea2<TUnitValue<i32, LogicalPixel>>;
template struct B3D_EXPORT TArea2<TUnitValue<float, LogicalPixel>>;
template struct B3D_EXPORT TArea2<TUnitValue<i32, PhysicalPixel>>;
template struct B3D_EXPORT TArea2<TUnitValue<float, PhysicalPixel>>;
