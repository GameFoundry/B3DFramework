---
title: Math
---

General purpose math functionality in the framework is provided through the @b3d::Math class. It provides a variety of familiar methods, such as @b3d::Math::Floor, @b3d::Math::Clamp, @b3d::Math::Cos and many others. Check the API reference for a full list.

All other math functionality is provided through specific types, as listed below.

# Templated Types
Most math types in the framework are actually templated classes that can work with different numeric types. The commonly used type aliases are:

**Vectors:**
- @b3d::Vector2, @b3d::Vector3, @b3d::Vector4 - Float-based vectors (aliases for **TVector2<float>**, **TVector3<float>**, **TVector4<float>**)
- @b3d::Vector2D, @b3d::Vector3D, @b3d::Vector4D - Double-precision vectors (aliases for **TVector2<double>**, **TVector3<double>**, **TVector4<double>**)

**Matrices:**
- @b3d::Matrix3, @b3d::Matrix4 - Float-based matrices (aliases for **TMatrix3<float>**, **TMatrix4<float>**)
- @b3d::Matrix3D, @b3d::Matrix4D - Double-precision matrices (aliases for **TMatrix3<double>**, **TMatrix4<double>**)

**Rotations:**
- @b3d::Quaternion - Float-based quaternion (alias for **TQuaternion<float>**)
- @b3d::QuaternionD - Double-precision quaternion (alias for **TQuaternion<double>**)

**Shapes:**
- @b3d::Sphere, @b3d::SphereD - Sphere (aliases for **TSphere<float>**, **TSphere<double>**)
- @b3d::Plane, @b3d::PlaneD - Plane (aliases for **TPlane<float>**, **TPlane<double>**)
- @b3d::AABox, @b3d::AABoxD - Axis-aligned box (aliases for **TAABox<float>**, **TAABox<double>**)
- @b3d::Ray, @b3d::RayD - Ray (aliases for **TRay<float>**, **TRay<double>**)

The templated forms (@b3d::TVector3, @b3d::TMatrix4, @b3d::TQuaternion, @b3d::TSphere, @b3d::TPlane, @b3d::TAABox, @b3d::TRay, etc.) can be instantiated with either **float** or **double** as the template parameter, allowing you to choose the precision level appropriate for your use case.

~~~~~~~~~~~~~{.cpp}
// Using float-based vectors (default)
Vector3 floatVector(1.0f, 2.0f, 3.0f);

// Using double-precision vectors for high-precision calculations
Vector3D doubleVector(1.0, 2.0, 3.0);
TVector3<double> explicitDoubleVector(1.0, 2.0, 3.0); // Equivalent

// Double-precision matrices
Matrix4D highPrecisionMatrix = Matrix4D::TRS(Vector3D(0.0, 50.0, 0.0),
	Quaternion(Degree(90), Degree(0), Degree(0)),
	Vector3D(1.0, 1.0, 2.0));

// Double-precision quaternion for high-accuracy rotations
QuaternionD preciseRotation(Degree(45.0), Degree(30.0), Degree(60.0));

// Double-precision shapes for physics simulations
SphereD preciseSphere(Vector3D(0.0, 0.0, 0.0), 10.0);
RayD preciseRay(Vector3D(0.0, 0.0, 0.0), Vector3D(0.0, 1.0, 0.0));
AABoxD preciseBox(Vector3D(-5.0, -5.0, -5.0), Vector3D(5.0, 5.0, 5.0));

B3D_LOG(Info, LogGeneric, "Float vector: {0}, Double vector: {1}",
	floatVector, doubleVector);
~~~~~~~~~~~~~

Throughout this manual, examples use the float-based aliases (**Vector3**, **Matrix4**, **Quaternion**, etc.), but all operations and methods work identically with the double-precision variants.

# Vectors
Vectors are represented by @b3d::Vector2, @b3d::Vector3 and @b3d::Vector4 classes. All classes come with a full range of operators so manipulating vectors is easy.

~~~~~~~~~~~~~{.cpp}
Vector3 firstVector(0.0f, 10.0f, 20.0f);
Vector3 secondVector(-1.0f, -2.0f, -3.0f);

Vector3 sumVector = firstVector + secondVector;
Vector3 differenceVector = secondVector - firstVector;
Vector3 scaledVector = firstVector * 3.0f;
~~~~~~~~~~~~~

They also come with a variety of helpful methods, and some of the more useful ones are:
 - @b3d::Vector3::Dot - Dot product
 - @b3d::Vector3::Cross - Cross product
 - @b3d::Vector3::Normalize - Normalizes the vector
 - @b3d::Vector3::Length - Returns the length of the vector
 - @b3d::Vector3::Distance - Returns the distance between two vectors

And many more as listed on their API reference pages.

~~~~~~~~~~~~~{.cpp}
Vector3 crossProduct = firstVector.Cross(secondVector);
float dotProduct = firstVector.Dot(secondVector);
float vectorLength = firstVector.Length();
~~~~~~~~~~~~~

## Integer vectors
Integer 2D vector type is also provided as @b3d::Vector2I. It also supports a full range of operators and comes with a few helper methods. Higher level integer vector types can also be created in the form of @b3d::Vector3I and @b3d::Vector4I.

~~~~~~~~~~~~~{.cpp}
Vector2I integerVector(0, 10);
~~~~~~~~~~~~~

# Angles
Angles are represented using either @b3d::Degree or @b3d::Radian classes. They accept a raw floating point value, which they expect to be in degrees or radians, respectively. Once created the system automatically converts between the two.

~~~~~~~~~~~~~{.cpp}
Degree firstAngle(90.0f);
Radian secondAngle = firstAngle;

void PrintAngle(Degree angle)
{
	float angleValue = angle.ValueDegrees();
	B3D_LOG(Info, LogGeneric, "Angle is {0}", angleValue);
}

// Caller doesn't need to care if the method accepts radians or degrees
PrintAngle(secondAngle);
~~~~~~~~~~~~~

# Quaternions
@b3d::Quaternion%s are the primary way of representing rotations in the framework. They can be created using Euler angles, axis/angle combination, or from a rotation matrix (talked about later).

~~~~~~~~~~~~~{.cpp}
// Quaternion that rotates 30 degrees around X axis, followed by 50 degrees around Z axis (Euler angle representation)
Quaternion firstRotation(Degree(30), 0, Degree(50));

// Quaternion that rotates 40 degrees around Y axis (axis/angle representation)
Quaternion secondRotation(Vector3::kUnitY, Degree(40));

// Quaternion initialized from a rotation matrix
Matrix3 rotationMatrix = ...;
Quaternion thirdRotation(rotationMatrix);
~~~~~~~~~~~~~

Once created quaternion can be used to apply rotation to a 3D vector by calling @b3d::Quaternion::Rotate.

~~~~~~~~~~~~~{.cpp}
Vector3 originalVector(1, 0, 0);
Vector3 rotatedVector = secondRotation.Rotate(originalVector);
~~~~~~~~~~~~~

Rotations can be combined by multiplying the quaternions.

~~~~~~~~~~~~~{.cpp}
// Quaternion that applies secondRotation, followed by firstRotation
Quaternion combinedRotation = firstRotation * secondRotation;
~~~~~~~~~~~~~

Inverse of a rotation can be obtained by calling @b3d::Quaternion::Inverse.
~~~~~~~~~~~~~{.cpp}
// Quaternion that rotates 40 degrees around Y axis
Quaternion rotation(Degree(40), Vector3::kUnitY);

// Quaternion that rotates 40 degrees around Y axis, in opposite direction
Quaternion inverseRotation = rotation.Inverse();
~~~~~~~~~~~~~

Quaternions only make sense for rotations if they are normalized. If you're doing many operations with them, due to precision issues they might become un-normalized. In such cases you can call @b3d::Quaternion::Normalize to re-normalize them.

~~~~~~~~~~~~~{.cpp}
rotation.Normalize();
~~~~~~~~~~~~~

You can transform a quaternion back to a more familiar form using one of these methods:
 - @b3d::Quaternion::ToEulerAngles
 - @b3d::Quaternion::ToAxisAngle
 - @b3d::Quaternion::ToRotationMatrix

~~~~~~~~~~~~~{.cpp}
Radian xAngle, yAngle, zAngle;
rotation.ToEulerAngles(xAngle, yAngle, zAngle);

Radian angle;
Vector3 axis;
rotation.ToAxisAngle(axis, angle);

Matrix3 rotationMatrix;
rotation.ToRotationMatrix(rotationMatrix);
~~~~~~~~~~~~~

## Other useful methods

Often you want to rotate towards a certain direction, for example making a camera look towards something. You can create such rotation by calling @b3d::Quaternion::LookRotation.

~~~~~~~~~~~~~{.cpp}
Quaternion lookRotation;

// Look towards negative z axis
lookRotation.LookRotation(-Vector3::kUnitZ);
~~~~~~~~~~~~~

You can interpolate between using two quaternions using @b3d::Quaternion::Lerp and @b3d::Quaternion::Slerp. The first method offers normal linear interpolation, and the second method offers a specialized form of interpolation possible only with quaternions, called spherical interpolation. Spherical interpolation is special because it doesn't change the magnitude of the quaternion (i.e. the interpolation happens on a surface of a sphere), while linear interpolation might require a quaternion to be re-normalized after interpolating. On the other hand linear interpolation requires less CPU cycles to execute.

~~~~~~~~~~~~~{.cpp}
Quaternion linearInterpolation = Quaternion::Lerp(0.5f, firstRotation, secondRotation);
linearInterpolation.Normalize();

Quaternion sphericalInterpolation = Quaternion::Slerp(0.5f, firstRotation, secondRotation);
~~~~~~~~~~~~~

Sometimes you have two vectors and want to find a rotation that rotates the first vector into the position of the second. You can do that by using @b3d::Quaternion::GetRotationFromTo.

~~~~~~~~~~~~~{.cpp}
Vector3 fromVector(0.0f, 1.0f, 0.0f);
Vector3 toVector(1.0f, 0.0f, 0.0f);

// Generates a rotation that rotates from a vector pointing towards positive Y axis, to a vector pointing towards positive X axis. Essentially creates a 90 degree rotation around the Z axis.
Quaternion vectorRotation = Quaternion::GetRotationFromTo(fromVector, toVector);
~~~~~~~~~~~~~

You can find out the rotation axes of a quaternion by calling @b3d::Quaternion::XAxis, @b3d::Quaternion::YAxis or @b3d::Quaternion::ZAxis. For example you can use this to find the direction in which a quaternion is facing.

~~~~~~~~~~~~~{.cpp}
// Assuming we consider the Z axis the facing direction
Vector3 facingDirection = rotation.ZAxis();
~~~~~~~~~~~~~

# Matrices

Matrices can be split into two major types: @b3d::Matrix3 representing a 3x3 matrix and @b3d::Matrix4 representing a 4x4 matrix. 3x3 matrices are used primarily for representing rotations, and are used similarly to quaternions. 4x4 matrices are used to represent a complete set of transformations like scale, translation and rotation, and are the most commonly used matrix type. We also provide a generic @b3d::MatrixNxM template for other matrix sizes, but they come with much simpler functionality.

## Matrix3
**Matrix3** can be initialized using Euler angles, axis/angle combination, or from a quaternion. It can also accept a scale factor as well as rotation.

~~~~~~~~~~~~~{.cpp}
// Matrix that rotates 30 degrees around X axis, followed by 50 degrees around Z axis (Euler angle representation)
Matrix3 firstMatrix(Degree(30), 0, Degree(50));

// Matrix that rotates 40 degrees around Y axis (axis/angle representation)
Matrix3 secondMatrix(Vector3::kUnitY, Degree(40));

// Matrix initialized from a quaternion
Quaternion someQuaternion = ...;
Matrix3 thirdMatrix(someQuaternion);

// Matrix initialized from a quaternion, but also does scaling along with rotation
Vector3 scaleVector(1.0f, 0.5f, 2.0f);
Matrix3 fourthMatrix(someQuaternion, scaleVector);
~~~~~~~~~~~~~

To apply a matrix transformation to a 3D vector call @b3d::Matrix3::Multiply().

~~~~~~~~~~~~~{.cpp}
Vector3 originalVector(1, 0, 0);
Vector3 transformedVector = secondMatrix.Multiply(originalVector);
~~~~~~~~~~~~~

Matrices can be multiplied to combine their transformations.

~~~~~~~~~~~~~{.cpp}
// Creates a matrix that first applies transformation of firstMatrix, followed by transformation of secondMatrix
Matrix3 combinedMatrix = secondMatrix * firstMatrix;
~~~~~~~~~~~~~

Matrices can be transposed (switching rows/columns) by calling @b3d::Matrix3::Transpose.

~~~~~~~~~~~~~{.cpp}
Matrix3 originalMatrix(Vector3::kUnitY, Degree(40));
Matrix3 transposedMatrix = originalMatrix.Transpose();
~~~~~~~~~~~~~

Matrices can be inverted by calling @b3d::Matrix3::Inverse. Not all matrices have an inverse therefore this method returns a boolean which returns true if an inverse was found.

~~~~~~~~~~~~~{.cpp}
Matrix3 inverseMatrix;
if(originalMatrix.Inverse(inverseMatrix))
	B3D_LOG(Info, LogGeneric, "Inverse found!");
~~~~~~~~~~~~~

You can decompose a matrix back into rotation & scale components by calling @b3d::Matrix3::Decomposition. Note that this is only able to work if the matrix contains rotation and/or uniform scale, without any other transformations. Otherwise returned values will likely not be accurate.

~~~~~~~~~~~~~{.cpp}
Quaternion rotation;
Vector3 scale;
originalMatrix.Decomposition(rotation, scale);
~~~~~~~~~~~~~

If the matrix contains only rotation you can also use any of the following methods to extract it:
 - @b3d::Matrix3::ToEulerAngles
 - @b3d::Matrix3::ToAxisAngle
 - @b3d::Matrix3::ToQuaternion

~~~~~~~~~~~~~{.cpp}
Radian xAngle, yAngle, zAngle;
originalMatrix.ToEulerAngles(xAngle, yAngle, zAngle);

Radian angle;
Vector3 axis;
originalMatrix.ToAxisAngle(axis, angle);

Quaternion quaternion;
originalMatrix.ToQuaternion(quaternion);
~~~~~~~~~~~~~ 
 
## Matrix4

**Matrix4** can be initialized using any of the following static methods:
 - @b3d::Matrix4::Rotation - Creates a matrix containing only rotation, from a quaternion.
 - @b3d::Matrix4::Translation - Creates a matrix containing only translation.
 - @b3d::Matrix4::Scaling - Creates a matrix containing only scale.
 - @b3d::Matrix4::TRS - Creates a matrix containing translation, rotation and scale. Scale is applied first, followed by rotation and finally translation.

~~~~~~~~~~~~~{.cpp}
Quaternion rotation(Degree(90), Degree(0), Degree(0));
Matrix4 rotationMatrix = Matrix4::Rotation(rotation);

Vector3 translationVector(0.0f, 50.0f, 0.0f);
Matrix4 translationMatrix = Matrix4::Translation(translationVector);

Vector3 scaleVector(1.0f, 1.0f, 2.0f);
Matrix4 scaleMatrix = Matrix4::Scaling(scaleVector);

Matrix4 combinedMatrix = Matrix4::TRS(translationVector, rotation, scaleVector);
~~~~~~~~~~~~~

To apply a matrix transformation to a 4D vector you can call @b3d::Matrix4::Multiply(const Vector4&).

~~~~~~~~~~~~~{.cpp}
Vector4 originalVector(1, 0, 0, 1);
Vector4 transformedVector = combinedMatrix.Multiply(originalVector);
~~~~~~~~~~~~~

Not that a vector has its 4th component set to 1. This means the vector is treated as a point, and will be translated by the matrix. If the 4th component was 0, the vector would be treated as a direction instead, and translation would not be applied.

You can also use overriden @b3d::Matrix4::Multiply(const Vector3&) to multiply a **Vector3**, in which case it is assumed to be a point (4th component is equal to 1). If you instead wish to assume a **Vector3** is a direction, use @b3d::Matrix4::MultiplyDirection instead.

~~~~~~~~~~~~~{.cpp}
Vector3 originalVector3(1, 0, 0);
Vector3 transformedPoint = combinedMatrix.Multiply(originalVector3);

// No translation applied (if matrix had any)
Vector3 transformedDirection = combinedMatrix.MultiplyDirection(originalVector3);
~~~~~~~~~~~~~

Matrices can be multiplied to combine their transformations.

~~~~~~~~~~~~~{.cpp}
// Creates a matrix that first applies transformation of firstMatrix, followed by transformation of secondMatrix
Matrix4 combinedMatrix = secondMatrix * firstMatrix;
~~~~~~~~~~~~~

Matrices can be transposed (switching rows/columns) by calling @b3d::Matrix4::Transpose.

~~~~~~~~~~~~~{.cpp}
Matrix4 originalMatrix(Vector3::kUnitY, Degree(40));
Matrix4 transposedMatrix = originalMatrix.Transpose();
~~~~~~~~~~~~~

Matrices can be inverted by calling @b3d::Matrix4::Inverse. Not all matrices have an inverse therefore this method returns a boolean which returns true if an inverse was found.

~~~~~~~~~~~~~{.cpp}
Matrix4 inverseMatrix;
if(originalMatrix.Inverse(inverseMatrix))
	B3D_LOG(Info, LogGeneric, "Inverse found!");
~~~~~~~~~~~~~

You can decompose a matrix back into rotation, scale and translation components by calling @b3d::Matrix4::Decomposition. Note that this is only able to work if the matrix contains rotation, translation and uniform scale, without any other transformations. Otherwise returned values will likely not be accurate.

~~~~~~~~~~~~~{.cpp}
Vector3 translation;
Quaternion rotation;
Vector3 scale;
originalMatrix.Decomposition(translation, rotation, scale);
~~~~~~~~~~~~~

# Rays
A @b3d::Ray is represented using an origin point, and a direction. They are often used in physics for intersection tests.

~~~~~~~~~~~~~{.cpp}
// Ray with origin at world origin, looking up
Ray ray(Vector3(0, 0, 0), Vector3(0, 1, 0));
~~~~~~~~~~~~~

You can use @b3d::Ray::GetPoint to get a point some distance from ray origin, along the direction.

~~~~~~~~~~~~~{.cpp}
Vector3 pointOnRay = ray.GetPoint(10.0f);
~~~~~~~~~~~~~

Rays can be transformed by matrices by calling @b3d::Ray::Transform.

~~~~~~~~~~~~~{.cpp}
Matrix4 transformMatrix = ...;
ray.Transform(transformMatrix);
~~~~~~~~~~~~~

They also provide a series of *Intersects* methods that allow them to test for intersection against axis aligned boxes, spheres, planes and triangles:
 - @b3d::Ray::Intersects(const AABox&) - Axis aligned box intersection
 - @b3d::Ray::Intersects(const Sphere&) - Sphere intersection
 - @b3d::Ray::Intersects(const Plane&) - Plane intersection
 - @b3d::Ray::Intersects(const Vector3&, const Vector3&, const Vector3&, const Vector3&, bool, bool) - Triangle intersection
 
# Rectangles

@b3d::Rect2 and @b3d::Rect2I structures can be used for storing rectangles using floating point or integer values, respectively. Check their API reference for the methods they support, but in most scenarios you will be using them for storage and method parameters.

# Shapes

Framework supports a variety of other 3D shapes:
 - @b3d::AABox
 - @b3d::Sphere
 - @b3d::Plane
 - @b3d::Capsule
 
How they work should be self explanatory from their API reference. All of the shapes provide a way be initialized, to be transformed by a world matrix as well a set of intersection tests against rays and other shapes.

~~~~~~~~~~~~~{.cpp}
// Axis aligned box created from minimum and maximum corners
Vector3 min(-1.0f, -1.0f, -1.0f);
Vector3 max(1.0f, 1.0f, 1.0f);

AABox box(min, max);

// Sphere created from center and radius
Vector3 center(0.0f, 10.0f, 0.0f);
float radius = 20.0f;

Sphere sphere(center, radius);

// Plane created from normal, and distance from origin along the normal
Vector3 normal(0.0f, 1.0f, 0.0f);
float dist = 10.0f;

Plane plane(normal, dist);
~~~~~~~~~~~~~  
