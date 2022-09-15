//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/BsShapeMeshes2D.h"
#include "Math/BsRect2.h"
#include "Mesh/BsMesh.h"
#include "Math/BsVector2.h"
#include "Math/BsLine2.h"
#include "Material/BsPass.h"
#include "RenderAPI/BsVertexDataDesc.h"

namespace bs
{
	const UINT32 ShapeMeshes2D::NUM_VERTICES_AA_LINE = 4;
	const UINT32 ShapeMeshes2D::NUM_INDICES_AA_LINE = 6;

	void ShapeMeshes2D::SolidQuad(const Rect2& area, const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset)
	{
		UINT32* indexData = meshData->GetIndices32();
		UINT8* positionData = meshData->GetElementData(VES_POSITION);

		assert((vertexOffset + 4) <= meshData->GetNumVertices());
		assert((indexOffset + 6) <= meshData->GetNumIndices());

		Vector<Vector2> points;
		points.push_back(Vector2(area.x, area.y));
		points.push_back(Vector2(area.x + area.width, area.y));
		points.push_back(Vector2(area.x + area.width, area.y + area.height));
		points.push_back(Vector2(area.x, area.y + area.height));	

		PixelSolidPolygon(points, positionData, vertexOffset, meshData->GetVertexDesc()->GetVertexStride(), indexData, indexOffset);
	}

	void ShapeMeshes2D::PixelLine(const Vector2& a, const Vector2& b, const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset)
	{
		UINT32* indexData = meshData->GetIndices32();
		UINT8* positionData = meshData->GetElementData(VES_POSITION);

		assert((vertexOffset + 2) <= meshData->GetNumVertices());
		assert((indexOffset + 2) <= meshData->GetNumIndices());

		PixelLine(a, b, positionData, vertexOffset, meshData->GetVertexDesc()->GetVertexStride(), indexData, indexOffset);
	}

	void ShapeMeshes2D::QuadLine(const Vector2& a, const Vector2& b, float width, float border, const Color& color,
		const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset)
	{
		Vector<Vector2> linePoints = { a, b };
		QuadLineList(linePoints, width, border, color, meshData, vertexOffset, indexOffset);
	}

	void ShapeMeshes2D::PixelLineList(const Vector<Vector2>& linePoints, const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset)
	{
		assert(linePoints.size() % 2 == 0);

		assert((vertexOffset + linePoints.size() * 2) <= meshData->GetNumVertices());
		assert((indexOffset + linePoints.size() * 2) <= meshData->GetNumIndices());

		UINT32 curVertOffset = vertexOffset;
		UINT32 curIdxOffset = indexOffset;

		UINT32* indexData = meshData->GetIndices32();
		UINT8* positionData = meshData->GetElementData(VES_POSITION);

		UINT32 numPoints = (UINT32)linePoints.size();
		for(UINT32 i = 0; i < numPoints; i += 2)
		{
			PixelLine(linePoints[i], linePoints[i + 1], positionData, curVertOffset, meshData->GetVertexDesc()->GetVertexStride(), indexData, curIdxOffset);

			curVertOffset += 2;
			curIdxOffset += 2;
		}
	}

	void ShapeMeshes2D::QuadLineList(const Vector<Vector2>& linePoints, float width, float border, const Color& color,
		const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset)
	{
		UINT32 numPoints = (UINT32)linePoints.size();
		assert(numPoints >= 2);

		UINT32 numLines = (UINT32)linePoints.size() - 1;
		assert((vertexOffset + (numLines * 2 + 2)) <= meshData->GetNumVertices());
		assert((indexOffset + (numLines * 6)) <= meshData->GetNumIndices());

		UINT32* outIndices = indexOffset + meshData->GetIndices32();
		UINT8* outVertices = vertexOffset + meshData->GetElementData(VES_POSITION);
		UINT8* outColors = vertexOffset + meshData->GetElementData(VES_COLOR);

		UINT32 vertexStride = meshData->GetVertexDesc()->GetVertexStride();
		QuadLineList(&linePoints[0], numPoints, width, border, outVertices, vertexStride, true);

		RGBA colorValue = color.GetAsRgba();

		// Colors and indices
		for(UINT32 i = 0; i < numLines; i++)
		{
			memcpy(outColors, &colorValue, sizeof(colorValue));
			outColors += vertexStride;

			memcpy(outColors, &colorValue, sizeof(colorValue));
			outColors += vertexStride;

			UINT32 idxStart = i * 6;
			outIndices[idxStart + 0] = vertexOffset + idxStart + 0;
			outIndices[idxStart + 1] = vertexOffset + idxStart + 1;
			outIndices[idxStart + 2] = vertexOffset + idxStart + 2;

			outIndices[idxStart + 3] = vertexOffset + idxStart + 1;
			outIndices[idxStart + 4] = vertexOffset + idxStart + 3;
			outIndices[idxStart + 5] = vertexOffset + idxStart + 2;
		}

		memcpy(outColors, &colorValue, sizeof(colorValue));
		outColors += vertexStride;

		memcpy(outColors, &colorValue, sizeof(colorValue));
		outColors += vertexStride;
	}

	void ShapeMeshes2D::QuadLineList(const Vector2* linePoints, UINT32 numPoints, float width, float border, UINT8* outVertices,
		UINT32 vertexStride, bool indexed)
	{
		assert(numPoints >= 2);
		UINT32 numLines = numPoints - 1;

		width += border;

		Vector2 prevPoints[2];

		// Start segment
		{
			Vector2 a = linePoints[0];
			Vector2 b = linePoints[1];

			Vector2 diff = b - a;
			diff.Normalize();

			// Flip 90 degrees
			Vector2 normal(diff.y, -diff.x);

			prevPoints[0] = a - normal * width - diff * border;
			prevPoints[1] = a + normal * width - diff * border;

			memcpy(outVertices, &prevPoints[0], sizeof(prevPoints[0]));
			outVertices += vertexStride;

			memcpy(outVertices, &prevPoints[1], sizeof(prevPoints[1]));
			outVertices += vertexStride;
		}

		// Middle segments
		{
			for (UINT32 i = 1; i < numLines; i++)
			{
				Vector2 a = linePoints[i - 1];
				Vector2 b = linePoints[i];
				Vector2 c = linePoints[i + 1];

				Vector2 diffPrev = b - a;
				diffPrev.Normalize();

				Vector2 diffNext = c - b;
				diffNext.Normalize();

				// Flip 90 degrees
				Vector2 normalPrev(diffPrev.y, -diffPrev.x);
				Vector2 normalNext(diffNext.y, -diffNext.x);

				Vector2 curPoints[2];

				const float sign[] = { -1.0f, 1.0f };
				for (UINT32 j = 0; j < 2; j++)
				{
					Vector2 linePrevPoint = a + normalPrev * width * sign[j];
					Line2 linePrev(linePrevPoint, diffPrev);

					Vector2 lineNextPoint = b + normalNext * width * sign[j];
					Line2 lineNext(lineNextPoint, diffNext);

					auto intersect = linePrev.Intersects(lineNext);
					if (intersect.second != 0.0f) // Not parallel
						curPoints[j] = linePrev.GetPoint(intersect.second);
					else
						curPoints[j] = lineNextPoint;

					memcpy(outVertices, &curPoints[j], sizeof(curPoints[j]));
					outVertices += vertexStride;
				}

				if (!indexed)
				{
					memcpy(outVertices, &curPoints[0], sizeof(curPoints[0]));
					outVertices += vertexStride;

					memcpy(outVertices, &prevPoints[1], sizeof(prevPoints[1]));
					outVertices += vertexStride;

					memcpy(outVertices, &curPoints[0], sizeof(curPoints[0]));
					outVertices += vertexStride;

					memcpy(outVertices, &curPoints[1], sizeof(curPoints[1]));
					outVertices += vertexStride;

					prevPoints[0] = curPoints[0];
					prevPoints[1] = curPoints[1];
				}
			}
		}

		// End segment
		{
			Vector2 a = linePoints[numPoints - 2];
			Vector2 b = linePoints[numPoints - 1];

			Vector2 diff = b - a;
			diff.Normalize();

			// Flip 90 degrees
			Vector2 normal(diff.y, -diff.x);

			Vector2 curPoints[2];
			curPoints[0] = b - normal * width + diff * border;
			curPoints[1] = b + normal * width + diff * border;

			memcpy(outVertices, &curPoints[0], sizeof(curPoints[0]));
			outVertices += vertexStride;

			memcpy(outVertices, &curPoints[1], sizeof(curPoints[1]));
			outVertices += vertexStride;

			if (!indexed)
			{
				memcpy(outVertices, &curPoints[0], sizeof(curPoints[0]));
				outVertices += vertexStride;

				memcpy(outVertices, &prevPoints[1], sizeof(prevPoints[1]));
				outVertices += vertexStride;
			}
		}
	}

	void ShapeMeshes2D::PixelSolidPolygon(const Vector<Vector2>& points, UINT8* outVertices,
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

	void ShapeMeshes2D::PixelLine(const Vector2& a, const Vector2& b, UINT8* outVertices,
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
