//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsShapeMeshes2D.h"
#include "BsRect2.h"
#include "BsMesh.h"
#include "BsVector2.h"
#include "BsLine2.h"
#include "BsPass.h"
#include "BsVertexDataDesc.h"

namespace BansheeEngine
{
	const UINT32 ShapeMeshes2D::NUM_VERTICES_AA_LINE = 4;
	const UINT32 ShapeMeshes2D::NUM_INDICES_AA_LINE = 6;

	void ShapeMeshes2D::solidQuad(const Rect2& area, const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset)
	{
		UINT32* indexData = meshData->getIndices32();
		UINT8* positionData = meshData->getElementData(VES_POSITION);

		assert((vertexOffset + 4) <= meshData->getNumVertices());
		assert((indexOffset + 6) <= meshData->getNumIndices());

		Vector<Vector2> points;
		points.push_back(Vector2(area.x, area.y));
		points.push_back(Vector2(area.x + area.width, area.y));
		points.push_back(Vector2(area.x + area.width, area.y + area.height));
		points.push_back(Vector2(area.x, area.y + area.height));	

		pixelSolidPolygon(points, positionData, vertexOffset, meshData->getVertexDesc()->getVertexStride(), indexData, indexOffset);
	}

	void ShapeMeshes2D::pixelLine(const Vector2& a, const Vector2& b, const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset)
	{
		UINT32* indexData = meshData->getIndices32();
		UINT8* positionData = meshData->getElementData(VES_POSITION);

		assert((vertexOffset + 2) <= meshData->getNumVertices());
		assert((indexOffset + 2) <= meshData->getNumIndices());

		pixelLine(a, b, positionData, vertexOffset, meshData->getVertexDesc()->getVertexStride(), indexData, indexOffset);
	}

	void ShapeMeshes2D::quadLine(const Vector2& a, const Vector2& b, float width, const Color& color, 
		const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset)
	{
		Vector<Vector2> linePoints = { a, b };
		quadLineList(linePoints, width, color, meshData, vertexOffset, indexOffset);
	}

	void ShapeMeshes2D::pixelLineList(const Vector<Vector2>& linePoints, const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset)
	{
		assert(linePoints.size() % 2 == 0);

		assert((vertexOffset + linePoints.size() * 2) <= meshData->getNumVertices());
		assert((indexOffset + linePoints.size() * 2) <= meshData->getNumIndices());

		UINT32 curVertOffset = vertexOffset;
		UINT32 curIdxOffset = indexOffset;

		UINT32* indexData = meshData->getIndices32();
		UINT8* positionData = meshData->getElementData(VES_POSITION);

		UINT32 numPoints = (UINT32)linePoints.size();
		for(UINT32 i = 0; i < numPoints; i += 2)
		{
			pixelLine(linePoints[i], linePoints[i + 1], positionData, curVertOffset, meshData->getVertexDesc()->getVertexStride(), indexData, curIdxOffset);

			curVertOffset += 2;
			curIdxOffset += 2;
		}
	}

	void ShapeMeshes2D::quadLineList(const Vector<Vector2>& linePoints, float width, const Color& color,
		const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset)
	{
		assert(linePoints.size() >= 2 && linePoints.size() % 2 == 0);

		UINT32 numLines = (UINT32)linePoints.size() / 2;
		assert((vertexOffset + (numLines * 2 + 2)) <= meshData->getNumVertices());
		assert((indexOffset + (numLines * 6)) <= meshData->getNumIndices());

		UINT32* outIndices = indexOffset + meshData->getIndices32();
		UINT8* outVertices = vertexOffset + meshData->getElementData(VES_POSITION);
		UINT8* outColors = vertexOffset + meshData->getElementData(VES_COLOR);

		RGBA colorValue = color.getAsRGBA();

		// Start segment
		{
			Vector2 a = linePoints[0];
			Vector2 b = linePoints[1];

			Vector2 diff = b - a;
			diff.normalize();

			// Flip 90 degrees
			Vector2 normal(diff.y, -diff.x);

			Vector2 lineA = a + normal * width;
			Vector2 lineB = a - normal * width;

			memcpy(outVertices, &lineA, sizeof(lineA));
			outVertices += vertexOffset;

			memcpy(outVertices, &lineB, sizeof(lineB));
			outVertices += vertexOffset;
		}

		// Middle segments
		{
			for(UINT32 i = 1; i < numLines; i++)
			{
				Vector2 a = linePoints[i - 1];
				Vector2 b = linePoints[i];
				Vector2 c = linePoints[i + 1];

				Vector2 diffPrev = b - a;
				diffPrev.normalize();

				Vector2 diffNext = c - b;
				diffNext.normalize();

				// Flip 90 degrees
				Vector2 normalPrev(diffPrev.y, -diffPrev.x);
				Vector2 normalNext(diffNext.y, -diffNext.x);

				const float sign[] = { 1.0f, -1.0f };
				for(UINT32 j = 0; j < 2; j++)
				{
					Vector2 linePrevPoint = a + normalPrev * width * sign[j];
					Line2 linePrev(linePrevPoint, diffPrev);
					
					Vector2 lineNextPoint = b + normalNext * width * sign[j];
					Line2 lineNext(lineNextPoint, diffNext);

					auto intersect = linePrev.intersects(lineNext);
					Vector2 intersectPoint;
					if(intersect.second != 0.0f) // Not parallel
						intersectPoint = linePrev.getPoint(intersect.second);
					else
						intersectPoint = lineNextPoint;

					memcpy(outVertices, &intersectPoint, sizeof(intersectPoint));
					outVertices += vertexOffset;
				}
			}
		}

		// End segment
		{
			Vector2 a = linePoints[linePoints.size() - 2];
			Vector2 b = linePoints[linePoints.size() - 1];

			Vector2 diff = b - a;
			diff.normalize();

			// Flip 90 degrees
			Vector2 normal(diff.y, -diff.x);

			Vector2 lineA = a + normal * width;
			Vector2 lineB = a - normal * width;

			memcpy(outVertices, &lineA, sizeof(lineA));
			outVertices += vertexOffset;

			memcpy(outVertices, &lineB, sizeof(lineB));
			outVertices += vertexOffset;
		}

		// Colors and indices
		for(UINT32 i = 0; i < numLines; i++)
		{
			memcpy(outColors, &colorValue, sizeof(colorValue));
			outColors += vertexOffset;

			memcpy(outColors, &colorValue, sizeof(colorValue));
			outColors += vertexOffset;

			UINT32 idxStart = i * 6;
			outIndices[idxStart + 0] = vertexOffset + idxStart + 0;
			outIndices[idxStart + 1] = vertexOffset + idxStart + 1;
			outIndices[idxStart + 2] = vertexOffset + idxStart + 2;

			outIndices[idxStart + 3] = vertexOffset + idxStart + 0;
			outIndices[idxStart + 4] = vertexOffset + idxStart + 2;
			outIndices[idxStart + 5] = vertexOffset + idxStart + 3;
		}

		memcpy(outColors, &colorValue, sizeof(colorValue));
		outColors += vertexOffset;

		memcpy(outColors, &colorValue, sizeof(colorValue));
		outColors += vertexOffset;
	}

	void ShapeMeshes2D::pixelWirePolygon(const Vector<Vector2>& points, UINT8* outVertices, UINT8* outColors,
		UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset)
	{
		INT32 numPoints = (INT32)points.size();
		UINT32 curVertOffset = vertexOffset;
		UINT32 curIdxOffset = indexOffset;
		for (INT32 i = 0, j = numPoints - 1; i < numPoints; j = i++)
		{
			pixelLine(points[j], points[i], outVertices, curVertOffset, vertexStride, outIndices, curIdxOffset);
			curVertOffset += 2;
			curIdxOffset += 2;
		}
	}

	void ShapeMeshes2D::pixelSolidPolygon(const Vector<Vector2>& points, UINT8* outVertices,
		UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset)
	{
		outVertices += (vertexOffset * vertexStride);

		for (auto& point : points)
		{
			Vector2* vertices = (Vector2*)outVertices;
			(*vertices) = point;

			outVertices += vertexStride;
		}

		outIndices += indexOffset;
		INT32 numPoints = (INT32)points.size();
		UINT32 idxCnt = 0;
		for (int i = 2; i < numPoints; i++)
		{
			outIndices[idxCnt++] = vertexOffset;
			outIndices[idxCnt++] = vertexOffset + i - 1;
			outIndices[idxCnt++] = vertexOffset + i;
		}
	}

	void ShapeMeshes2D::pixelLine(const Vector2& a, const Vector2& b, UINT8* outVertices,
		UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset)
	{
		outVertices += (vertexOffset * vertexStride);

		Vector2* vertices = (Vector2*)outVertices;
		(*vertices) = a;

		vertices = (Vector2*)(outVertices + vertexStride);
		(*vertices) = b;

		outIndices += indexOffset;
		outIndices[0] = vertexOffset + 0;
		outIndices[1] = vertexOffset + 1;
	}
}