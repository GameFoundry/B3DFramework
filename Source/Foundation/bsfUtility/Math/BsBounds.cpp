//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsBounds.h"
#include "Math/BsRay.h"
#include "Math/BsPlane.h"
#include "Math/BsSphere.h"

using namespace b3d;

template<typename T>
TBounds<T>::TBounds(const TAABox<T>& box, const TSphere<T>& sphere)
	: mBox(box), mSphere(sphere)
{}

template<typename T>
void TBounds<T>::SetBounds(const TAABox<T>& box, const TSphere<T>& sphere)
{
	mBox = box;
	mSphere = sphere;
}

template<typename T>
void TBounds<T>::Merge(const TBounds<T>& rhs)
{
	mBox.Merge(rhs.mBox);
	mSphere.Merge(rhs.mSphere);
}

template<typename T>
void TBounds<T>::Merge(const TVector3<T>& point)
{
	mBox.Merge(point);
	mSphere.Merge(point);
}

template<typename T>
void TBounds<T>::Transform(const TMatrix4<T>& matrix)
{
	mBox.Transform(matrix);
	mSphere.Transform(matrix);
}

template<typename T>
void TBounds<T>::TransformAffine(const TMatrix4<T>& matrix)
{
	mBox.TransformAffine(matrix);
	mSphere.Transform(matrix);
}

template struct B3D_UTILITY_EXPORT TBounds<float>;
template struct B3D_UTILITY_EXPORT TBounds<double>;
