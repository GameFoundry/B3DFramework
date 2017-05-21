//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsShadowRendering.h"
#include "BsRendererView.h"

namespace bs { namespace ct
{
	ConvexVolume ShadowRendering::getCSMSplitFrustum(const RendererView& view, const Vector3& lightDir, UINT32 cascade, 
		UINT32 numCascades, Sphere& outBounds)
	{
		// Determine split range
		float splitNear = getCSMSplitDistance(view, cascade, numCascades);
		float splitFar = getCSMSplitDistance(view, cascade + 1, numCascades);

		// Calculate the eight vertices of the split frustum
		auto& viewProps = view.getProperties();

		const Matrix4& projMat = viewProps.projTransform;

		float aspect;
		float nearHalfWidth, nearHalfHeight;
		float farHalfWidth, farHalfHeight;
		if(viewProps.projType == PT_PERSPECTIVE)
		{
			aspect = projMat[0][0] / projMat[1][1];
			float tanHalfFOV = 1.0f / projMat[0][0];

			nearHalfWidth = splitNear * tanHalfFOV;
			nearHalfHeight = nearHalfWidth * aspect;

			farHalfWidth = splitFar * tanHalfFOV;
			farHalfHeight = farHalfWidth * aspect;
		}
		else
		{
			aspect = projMat[0][0] / projMat[1][1];

			nearHalfWidth = farHalfWidth = projMat[0][0] / 4.0f;
			nearHalfHeight = farHalfHeight = projMat[1][1] / 4.0f;
		}

		const Matrix4& viewMat = viewProps.viewTransform;
		Vector3 cameraRight = Vector3(viewMat[0]);
		Vector3 cameraUp = Vector3(viewMat[1]);

		const Vector3& viewOrigin = viewProps.viewOrigin;
		const Vector3& viewDir = viewProps.viewDirection;

		Vector3 frustumVerts[] =
		{
			viewOrigin + viewDir * splitNear - cameraRight * nearHalfWidth + cameraUp * nearHalfHeight, // Near, left, top
			viewOrigin + viewDir * splitNear + cameraRight * nearHalfWidth + cameraUp * nearHalfHeight, // Near, right, top
			viewOrigin + viewDir * splitNear + cameraRight * nearHalfWidth - cameraUp * nearHalfHeight, // Near, right, bottom
			viewOrigin + viewDir * splitNear - cameraRight * nearHalfWidth - cameraUp * nearHalfHeight, // Near, left, bottom
			viewOrigin + viewDir * splitFar - cameraRight * farHalfWidth + cameraUp * farHalfHeight, // Far, left, top
			viewOrigin + viewDir * splitFar + cameraRight * farHalfWidth + cameraUp * farHalfHeight, // Far, right, top
			viewOrigin + viewDir * splitFar + cameraRight * farHalfWidth - cameraUp * farHalfHeight, // Far, right, bottom
			viewOrigin + viewDir * splitFar - cameraRight * farHalfWidth - cameraUp * farHalfHeight, // Far, left, bottom
		};

		// Calculate the bounding sphere of the frustum
		float diagonalNearSq = nearHalfWidth * nearHalfWidth + nearHalfHeight * nearHalfHeight;
		float diagonalFarSq = farHalfWidth * farHalfWidth + farHalfHeight * farHalfHeight;

		float length = splitFar - splitNear;
		float offset = (diagonalNearSq - diagonalFarSq) / 2 * length + length * 0.5f;
		float distToCenter = Math::clamp(splitFar - offset, splitNear, splitFar);

		Vector3 center = viewOrigin + viewDir * distToCenter;

		float radius = 0.0f;
		for (auto& entry : frustumVerts)
			radius = std::max(radius, center.squaredDistance(entry));

		radius = std::max(sqrt(radius), 1.0f);
		outBounds = Sphere(center, radius);

		// Generate light frustum planes
		Plane viewPlanes[6];
		viewPlanes[FRUSTUM_PLANE_NEAR] = Plane(frustumVerts[0], frustumVerts[1], frustumVerts[2]);
		viewPlanes[FRUSTUM_PLANE_FAR] = Plane(frustumVerts[5], frustumVerts[4], frustumVerts[7]);
		viewPlanes[FRUSTUM_PLANE_LEFT] = Plane(frustumVerts[4], frustumVerts[0], frustumVerts[3]);
		viewPlanes[FRUSTUM_PLANE_RIGHT] = Plane(frustumVerts[1], frustumVerts[5], frustumVerts[6]);
		viewPlanes[FRUSTUM_PLANE_TOP] = Plane(frustumVerts[4], frustumVerts[5], frustumVerts[1]);
		viewPlanes[FRUSTUM_PLANE_BOTTOM] = Plane(frustumVerts[3], frustumVerts[2], frustumVerts[6]);

		Vector<Plane> lightVolume;

		//// Add camera's planes facing towards the lights (forming the back of the volume)
		for(auto& entry : viewPlanes)
		{
			if (entry.normal.dot(lightDir) < 0.0f)
				lightVolume.push_back(entry);
		}

		//// Determine edge planes by testing adjacent planes with different facing
		////// Pairs of frustum planes that share an edge
		UINT32 adjacentPlanes[][2] =
		{
			{ FRUSTUM_PLANE_NEAR, FRUSTUM_PLANE_LEFT },
			{ FRUSTUM_PLANE_NEAR, FRUSTUM_PLANE_RIGHT },
			{ FRUSTUM_PLANE_NEAR, FRUSTUM_PLANE_TOP },
			{ FRUSTUM_PLANE_NEAR, FRUSTUM_PLANE_BOTTOM },

			{ FRUSTUM_PLANE_FAR, FRUSTUM_PLANE_LEFT },
			{ FRUSTUM_PLANE_FAR, FRUSTUM_PLANE_RIGHT },
			{ FRUSTUM_PLANE_FAR, FRUSTUM_PLANE_TOP },
			{ FRUSTUM_PLANE_FAR, FRUSTUM_PLANE_BOTTOM },

			{ FRUSTUM_PLANE_LEFT, FRUSTUM_PLANE_TOP },
			{ FRUSTUM_PLANE_TOP, FRUSTUM_PLANE_RIGHT },
			{ FRUSTUM_PLANE_RIGHT, FRUSTUM_PLANE_BOTTOM },
			{ FRUSTUM_PLANE_BOTTOM, FRUSTUM_PLANE_LEFT },
		};

		////// Vertex indices of edges on the boundary between two planes
		UINT32 sharedEdges[][2] =
		{
			{ 3, 0 },{ 1, 2 },{ 0, 1 },{ 2, 3 },
			{ 4, 7 },{ 6, 5 },{ 5, 4 },{ 7, 6 },
			{ 4, 0 },{ 5, 1 },{ 6, 2 },{ 7, 3 }
		};

		for(UINT32 i = 0; i < 12; i++)
		{
			const Plane& planeA = viewPlanes[adjacentPlanes[i][0]];
			const Plane& planeB = viewPlanes[adjacentPlanes[i][1]];

			float dotA = planeA.normal.dot(lightDir);
			float dotB = planeB.normal.dot(lightDir);

			if((dotA * dotB) < 0.0f)
			{
				const Vector3& vertA = frustumVerts[sharedEdges[i][0]];
				const Vector3& vertB = frustumVerts[sharedEdges[i][1]];
				Vector3 vertC = vertA + lightDir;

				if (dotA >= 0.0f)
					lightVolume.push_back(Plane(vertA, vertB, vertC));
				else
					lightVolume.push_back(Plane(vertB, vertA, vertC));
			}
		}

		return ConvexVolume(lightVolume);
	}

	float ShadowRendering::getCSMSplitDistance(const RendererView& view, UINT32 index, UINT32 numCascades)
	{
		// Determines the size of each subsequent cascade split. Value of 1 means the cascades will be linearly split.
		// Value of 2 means each subsequent split will be twice the size of the previous one. Valid range is roughly
		// [1, 4].
		// Note: Make this an adjustable property?
		const static float DISTRIBUTON_EXPONENT = 1.0f;

		// First determine the scale of the split, relative to the entire range
		float scaleModifier = 1.0f;
		float scale = 0.0f;
		float totalScale = 0.0f;

		//// Split 0 corresponds to near plane
		if (index > 0)
		{
			for (UINT32 i = 0; i < numCascades; i++)
			{
				if (i < index)
					scale += scaleModifier;

				totalScale += scaleModifier;
				scaleModifier *= DISTRIBUTON_EXPONENT;
			}
		}

		scale = scale / totalScale;

		// Calculate split distance in Z
		auto& viewProps = view.getProperties();
		float near = viewProps.nearPlane;
		float far = viewProps.farPlane;

		return near + (far - near) * scale;
	}
}}