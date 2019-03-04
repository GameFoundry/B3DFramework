Math					{#mathUtilities}
===============
[TOC]

General purpose math functionality in bs::f is provided through the @ref bs::Math "Math" class. It provides a variety of familiar methods, such as @ref bs::Math::floor "Math::floor()", @ref bs::Math::clamp "Math::clamp()", @ref bs::Math::cos "Math::cos()" and many others. Check the API reference for a full list.

All other math functionality is provided through specific types, as listed below.

# Vectors {#mathUtilities_a}
Vectors are represented by @ref bs::Vector2 "Vector2", @ref bs::Vector3 "Vector3" and @ref bs::Vector4 "Vector4" classes. All classes come with a full range of operators so manipulating vectors is easy.

~~~~~~~~~~~~~{.cpp}
Vector3 a(0.0f, 10.0f, 20.0f);
Vector3 b(-1.0f, -2.0f, -3.0f);

Vector3 c = a + b;
Vector3 d = b - a;
Vector3 scaled = a * 3.0f;
~~~~~~~~~~~~~

They also come with a variety of helpful methods, and some of the more useful ones are:
 - @ref bs::Vector3::dot "Vector3::dot()" - Dot product
 - @ref bs::Vector3::cross "Vector3::cross()" - Cross product
 - @ref bs::Vector3::normalize "Vector3::normalize()" - Normalizes the vector
 - @ref bs::Vector3::length "Vector3::length()" - Returns the length of the vector
 - @ref bs::Vector3::distance "Vector3::distance()" - Returns the distance between two vectors
 
And many more as listed on their API reference pages.

~~~~~~~~~~~~~{.cpp}
Vector3 cross = a.cross(b);
float dotProduct = a.dot(b);
float length = a.length();
~~~~~~~~~~~~~

## Integer vectors {#mathUtilities_a_a}
Integer 2D vector type is also provided as @ref bs::Vector2I "Vector2I". It also supports a full range of operators and comes with a few helper methods. Higher level integer vector types can also be created in the form of @ref bs::Vector3I "Vector3I" and @ref bs::Vector4I "Vector4I".

~~~~~~~~~~~~~{.cpp}
Vector2I intVec(0, 10);
~~~~~~~~~~~~~

# Angles {#mathUtilities_b}
Angles are represented using either @ref bs::Degree "Degree" or @ref bs::Radian "Radian" classes. They accept a raw floating point value, which they expect to be in degrees or radians, respectively. Once created the system automatically converts between the two.

~~~~~~~~~~~~~{.cpp}
Degree myAngle(90.0f);
Radian myAngle2 = myAngle;

void printAngle(Degree angle)
{
	float angleValue = angle.valueDegrees();
	gDebug().logDebug("Angle is " + toString(angleValue));
}

// Caller doesn't need to care if the method accepts radians or degrees
printAngle(myAngle2);
~~~~~~~~~~~~~

# Quaternions {#mathUtilities_c}
@ref bs::Quaternion "Quaternion"%s are the primary way of representing rotations in bs::f. They can be created using Euler angles, axis/angle combination, or from a rotation matrix (talked about later). 

~~~~~~~~~~~~~{.cpp}
// Quaternion that rotates 30 degrees around X axis, followed by 50 degrees around Z axis (Euler angle representation)
Quaternion a(Degree(30), 0, Degree(50));

// Quaternion that rotates 40 degrees around Y axis (axis/angle representation)
Quaternion b(Vector3::UNIT_Y, Degree(40));

// Quaternion initialized from a rotation matrix
Matrix3 someMatrix = ...;
Quaternion c(someMatrix);
~~~~~~~~~~~~~

Once created quaternion can be used to apply rotation to a 3D vector by calling @ref bs::Quaternion::rotate "Quaternion::rotate()".

~~~~~~~~~~~~~{.cpp}
Vector3 myVector(1, 0, 0);
Vector3 rotatedVector = b.rotate(myVector);
~~~~~~~~~~~~~

Rotations can be combined by multiplying the quaternions.

~~~~~~~~~~~~~{.cpp}
// Quaternion that applies rotation b, followed by rotation a
Quaternion combination = a * b;
~~~~~~~~~~~~~

Inverse of a rotation can be obtained by calling @ref bs::Quaternion::inverse "Quaternion::inverse()".
~~~~~~~~~~~~~{.cpp}
// Quaternion that rotates 40 degrees around Y axis
Quaternion quat(Degree(40), Vector3::UNIT_Y);

// Quaternion that rotates 40 degrees around Y axis, in opposite direction
Quaternion invQuat = quat.inverse();
~~~~~~~~~~~~~

Quaternions only make sense for rotations if they are normalized. If you're doing many operations with them, due to precision issues they might become un-normalized. In such cases you can call @ref bs::Quaternion::normalize "Quaternion::normalize()" to re-normalize them.

~~~~~~~~~~~~~{.cpp}
quat.normalize();
~~~~~~~~~~~~~

You can transform a quaternion back to a more familiar form using one of these methods:
 - @ref bs::Quaternion::toEulerAngles "Quaternion::toEulerAngles()"
 - @ref bs::Quaternion::toAxisAngle "Quaternion::toAxisAngle()"
 - @ref bs::Quaternion::toRotationMatrix "Quaternion::toRotationMatrix()"
 
~~~~~~~~~~~~~{.cpp}
Radian xAngle, yAngle, zAngle;
quat.toEulerAngles(xAngle, yAngle, zAngle);

Radian angle;
Vector3 axis;
quat.toAxisAngle(axis, angle);

Matrix3 rotationMat;
quat.toRotationMatrix(rotationMat);
~~~~~~~~~~~~~

## Other useful methods {#mathUtilities_c_a}

Often you want to rotate towards a certain direction, for example making a camera look towards something. You can create such rotation by calling @ref bs::Quaternion::lookRotation "Quaternion::lookRotation()".

~~~~~~~~~~~~~{.cpp}
Quaternion q;

// Look towards negative z axis
q.lookRotation(-Vector3::UNIT_Z);
~~~~~~~~~~~~~

You can interpolate between using two quaternions using @ref bs::Quaternion::lerp "Quaternion::lerp()" and @ref bs::Quaternion::slerp "Quaternion::slerp()". The first method offers normal linear interpolation, and the second method offers a specialized form of interpolation possible only with quaternions, called spherical interpolation. Spherical interpolation is special because it doesn't change the magnitude of the quaternion (i.e. the interpolation happens on a surface of a sphere), while linear interpolation might require a quaternion to be re-normalized after interpolating. On the other hand linear interpolation requires less CPU cycles to execute.

~~~~~~~~~~~~~{.cpp}
Quaternion interpVal0 = Quaternion::lerp(0.5f, a, b);
interpVal0.normalize();

Quaternion interpVal1 = Quaternion::slerp(0.5f, a, b);
~~~~~~~~~~~~~

Sometimes you have two vectors and want to find a rotation that rotates the first vector into the position of the second. You can do that by using @ref bs::Quaternion::getRotationFromTo "Quaternion::getRotationFromTo()".

~~~~~~~~~~~~~{.cpp}
Vector3 from(0.0f, 1.0f, 0.0f);
Vector3 to(1.0f, 0.0f, 0.0f);

// Generates a rotation that rotates from a vector pointing towards positive Y axis, to a vector pointing towards positive X axis. Essentially creates a 90 degree rotation around the Z axis.
Quaternion quat = Quaternion::getRotationFromTo(from, to);
~~~~~~~~~~~~~

You can find out the rotation axes of a quaternion by calling @ref bs::Quaternion::xAxis "Quaternion::xAxis()", @ref bs::Quaternion::yAxis "Quaternion::yAxis()" or @ref bs::Quaternion::zAxis "Quaternion::zAxis()". For example you can use this to find the direction in which a quaternion is facing.

~~~~~~~~~~~~~{.cpp}
// Assuming we consider the Z axis the facing direction
Vector3 dir = quat.zAxis();
~~~~~~~~~~~~~

# Matrices {#mathUtilities_d}

Matrices can be split into two major types: @ref bs::Matrix3 "Matrix3" representing a 3x3 matrix and @ref bs::Matrix4 "Matrix4" representing a 4x4 matrix. 3x3 matrices are used primarily for representing rotations, and are used similarly to quaternions. 4x4 matrices are used to represent a complete set of transformations like scale, translation and rotation, and are the most commonly used matrix type. We also provide a generic @ref bs::MatrixNxM<N, M> "MatrixNxM<N, M>" template for other matrix sizes, but they come with much simpler functionality.

## Matrix3 {#mathUtilities_d_a}
**Matrix3** can be initialized using Euler angles, axis/angle combination, or from a quaternion. It can also accept a scale factor as well as rotation.

~~~~~~~~~~~~~{.cpp}
// Matrix that rotates 30 degrees around X axis, followed by 50 degrees around Z axis (Euler angle representation)
Matrix3 a(Degree(30), 0, Degree(50));

// Matrix that rotates 40 degrees around Y axis (axis/angle representation)
Matrix3 b(Vector3::UNIT_Y, Degree(40));

// Matrix initialized from a quaternion
Quaternion someQuat = ...;
Matrix3 c(someQuat);

// Matrix initialized from a quaternion, but also does scaling along with rotation
Vector3 scale(1.0f, 0.5f, 2.0f);
Matrix3 d(someQuat, scale);
~~~~~~~~~~~~~

To apply a matrix transformation to a 3D vector call @ref bs::Matrix3::multiply() "Matrix3::multiply()".

~~~~~~~~~~~~~{.cpp}
Vector3 myVector(1, 0, 0);
Vector3 transformedVector = b.multiply(myVector);
~~~~~~~~~~~~~

Matrices can be multiplied to combine their transformations.

~~~~~~~~~~~~~{.cpp}
// Creates a matrix that first applies transformation of matrix "a", followed by transformation of matrix "b"
Matrix3 combination = b * a;
~~~~~~~~~~~~~

Matrices can be transposed (switching rows/columns) by calling @ref bs::Matrix3::transpose "Matrix3::transpose()".

~~~~~~~~~~~~~{.cpp}
Matrix3 mat(Vector3::UNIT_Y, Degree(40));
Matrix3 transpose = mat.transpose();
~~~~~~~~~~~~~

Matrices can be inverted by calling @ref bs::Matrix3::inverse "Matrix3::inverse()". Not all matrices have an inverse therefore this method returns a boolean which returns true if an inverse was found.

~~~~~~~~~~~~~{.cpp}
Matrix3 inverseMat;
if(mat.inverse(inverseMat))
	gDebug().logDebug("Inverse found!");
~~~~~~~~~~~~~

You can decompose a matrix back into rotation & scale components by calling @ref bs::Matrix3::decomposition "Matrix3::decomposition()". Note that this is only able to work if the matrix contains rotation and/or uniform scale, without any other transformations. Otherwise returned values will likely not be accurate.

~~~~~~~~~~~~~{.cpp}
Quaternion rotation;
Vector3 scale;
mat.decomposition(rotation, scale);
~~~~~~~~~~~~~

If the matrix contains only rotation you can also use any of the following methods to extract it:
 - @ref bs::Matrix3::toEulerAngles "Matrix3::toEulerAngles()"
 - @ref bs::Matrix3::toAxisAngle "Matrix3::toAxisAngle()"
 - @ref bs::Matrix3::toQuaternion "Matrix3::toQuaternion()"
 
~~~~~~~~~~~~~{.cpp}
Radian xAngle, yAngle, zAngle;
mat.toEulerAngles(xAngle, yAngle, zAngle);

Radian angle;
Vector3 axis;
mat.toAxisAngle(axis, angle);

Quaternion quat;
mat.toQuaternion(quat);
~~~~~~~~~~~~~ 
 
## Matrix4 {#mathUtilities_d_b}

**Matrix4** can be initialized using any of the following static methods:
 - @ref bs::Matrix4::rotation "Matrix4::rotation()" - Creates a matrix containing only rotation, from a quaternion.
 - @ref bs::Matrix4::translation "Matrix4::translation()" - Creates a matrix containing only translation.
 - @ref bs::Matrix4::scaling "Matrix4::scaling()" - Creates a matrix containing only scale.
 - @ref bs::Matrix4::TRS "Matrix4::TRS()" - Creates a matrix containing translation, rotation and scale. Scale is applied first, followed by rotation and finally translation.
 
~~~~~~~~~~~~~{.cpp}
Quaternion rotation(Degree(90), Degree(0), Degree(0));
Matrix4 rotMat = Matrix4::rotation(rotation);

Vector3 translation(0.0f, 50.0f, 0.0f);
Matrix4 transMat = Matrix4::translation(translation);

Vector3 scale(1.0f, 1.0f, 2.0f);
Matrix4 scaleMat = Matrix4::scaling(scale);

Matrix4 combinedMat = Matrix4::TRS(translation, rotation, scale);
~~~~~~~~~~~~~
 
To apply a matrix transformation to a 4D vector you can call @ref bs::Matrix4::multiply(const Vector4&) const "Matrix4::multiply()".

~~~~~~~~~~~~~{.cpp}
Vector4 myVector(1, 0, 0, 1);
Vector4 transformedVector = combinedMat.multiply(myVector);
~~~~~~~~~~~~~

Not that a vector has its 4th component set to 1. This means the vector is treated as a point, and will be translated by the matrix. If the 4th component was 0, the vector would be treated as a direction instead, and translation would not be applied.

You can also use overriden @ref bs::Matrix4::multiply(const Vector3&) const "Matrix4::multiply()" to multiply a **Vector3**, in which case it is assumed to be a point (4th component is equal to 1). If you instead wish to assume a **Vector3** is a direction, use @ref bs::Matrix4::multiplyDirection "Matrix4::multiplyDirection()" instead.

~~~~~~~~~~~~~{.cpp}
Vector3 myVector(1, 0, 0);
Vector3 transformedPoint = combinedMat.multiply(myVector);

// No translation applied (if matrix had any)
Vector3 transformedDirection = combinedMat.multiplyDirection(myVector);
~~~~~~~~~~~~~

Matrices can be multiplied to combine their transformations.

~~~~~~~~~~~~~{.cpp}
// Creates a matrix that first applies transformation of matrix "a", followed by transformation of matrix "b"
Matrix4 combination = b * a;
~~~~~~~~~~~~~

Matrices can be transposed (switching rows/columns) by calling @ref bs::Matrix4::transpose "Matrix4::transpose()".

~~~~~~~~~~~~~{.cpp}
Matrix4 mat(Vector3::UNIT_Y, Degree(40));
Matrix4 transpose = mat.transpose();
~~~~~~~~~~~~~

Matrices can be inverted by calling @ref bs::Matrix4::inverse "Matrix4::inverse()". Not all matrices have an inverse therefore this method returns a boolean which returns true if an inverse was found.

~~~~~~~~~~~~~{.cpp}
Matrix4 inverseMat;
if(mat.inverse(inverseMat))
	gDebug().logDebug("Inverse found!");
~~~~~~~~~~~~~

You can decompose a matrix back into rotation, scale and translation components by calling @ref bs::Matrix4::decomposition "Matrix4::decomposition()". Note that this is only able to work if the matrix contains rotation, translation and uniform scale, without any other transformations. Otherwise returned values will likely not be accurate.

~~~~~~~~~~~~~{.cpp}
Vector3 translation;
Quaternion rotation;
Vector3 scale;
mat.decomposition(translation, rotation, scale);
~~~~~~~~~~~~~

# Rays {#mathUtilities_e}
@ref bs::Ray "Ray"s are represented using an origin point, and a direction. They are often used in physics for intersection tests.

~~~~~~~~~~~~~{.cpp}
// Ray with origin at world origin, looking up
Ray ray(Vector3(0, 0, 0), Vector3(0, 1, 0));
~~~~~~~~~~~~~ 

You can use @ref bs::Ray::getPoint "Ray::getPoint()" to get a point some distance from ray origin, along the direction.

~~~~~~~~~~~~~{.cpp}
Vector3 point = ray.getPoint(10.0f);
~~~~~~~~~~~~~  

Rays can be transformed by matrices by calling @ref bs::Ray::transform "Ray::transform()".

~~~~~~~~~~~~~{.cpp}
Matrix4 mat = ...;
ray.transform(mat);
~~~~~~~~~~~~~  

They also provide a series of *intersects* methods that allow them to test for intersection against axis aligned boxes, spheres, planes and triangles:
 - @ref bs::Ray::intersects(const AABox&) const "Ray::intersects(const AABox&)" - Axis aligned box intersection
 - @ref bs::Ray::intersects(const Sphere&) const "Ray::intersects(const Sphere&)" - Sphere intersection
 - @ref bs::Ray::intersects(const Plane&) const "Ray::intersects(const Plane&)" - Plane intersection
 - @ref bs::Ray::intersects(const Vector3&, const Vector3&, const Vector3&, const Vector3&, bool, bool) const "Ray::intersects(const Vector3&, const Vector3&, const Vector3&)" - Triangle intersection
 
# Rectangles {#mathUtilities_f}

@ref bs::Rect2 "Rect2" and @ref bs::Rect2I "Rect2I" structures can be used for storing rectangles using floating point or integer values, respectively. Check their API reference for the methods they support, but in most scenarios you will be using them for storage and method parameters.

# Shapes {#mathUtilities_g}

bs::f supports a variety of other 3D shapes:
 - @ref bs::AABox "AABox"
 - @ref bs::Sphere "Sphere"
 - @ref bs::Plane "Plane"
 - @ref bs::Capsule "Capsule"
 
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