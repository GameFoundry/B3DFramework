//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/B3DShapeMeshes2D.h"
#include "Math/B3DArea2.h"
#include "Mesh/B3DMesh.h"
#include "Math/B3DVector2.h"
#include "Math/B3DLine2.h"
#include "Material/B3DPass.h"
#include "GpuBackend/B3DVertexDescription.h"

using namespace b3d;

const u32 ShapeMeshes2D::kNumVerticesAaLine = 4;
const u32 ShapeMeshes2D::kNumIndicesAaLine = 6;

void ShapeMeshes2D::SolidQuad(const Area2& area, const TShared<MeshData>& meshData, u32 vertexOffset, u32 indexOffset)
{
	u32* indexData = meshData->GetIndices32();
	u8* positionData = meshData->GetElementData(VES_POSITION);

	B3D_ASSERT((vertexOffset + 4) <= meshData->GetVertexCount());
	B3D_ASSERT((indexOffset + 6) <= meshData->GetIndexCount());

	Vector<Vector2> points;
	points.push_back(Vector2(area.X, area.Y));
	points.push_back(Vector2(area.X + area.Width, area.Y));
	points.push_back(Vector2(area.X + area.Width, area.Y + area.Height));
	points.push_back(Vector2(area.X, area.Y + area.Height));

	PixelSolidPolygon(points, positionData, vertexOffset, meshData->GetVertexDescription()->GetVertexStride(), indexData, indexOffset);
}

void ShapeMeshes2D::PixelLine(const Vector2& a, const Vector2& b, const TShared<MeshData>& meshData, u32 vertexOffset, u32 indexOffset)
{
	u32* indexData = meshData->GetIndices32();
	u8* positionData = meshData->GetElementData(VES_POSITION);

	B3D_ASSERT((vertexOffset + 2) <= meshData->GetVertexCount());
	B3D_ASSERT((indexOffset + 2) <= meshData->GetIndexCount());

	PixelLine(a, b, positionData, vertexOffset, meshData->GetVertexDescription()->GetVertexStride(), indexData, indexOffset);
}

void ShapeMeshes2D::QuadLine(const Vector2& a, const Vector2& b, float width, float border, const Color& color, const TShared<MeshData>& meshData, u32 vertexOffset, u32 indexOffset)
{
	Vector<Vector2> linePoints = { a, b };
	QuadLineList(linePoints, width, border, color, meshData, vertexOffset, indexOffset);
}

void ShapeMeshes2D::PixelLineList(const Vector<Vector2>& linePoints, const TShared<MeshData>& meshData, u32 vertexOffset, u32 indexOffset)
{
	B3D_ASSERT(linePoints.size() % 2 == 0);

	B3D_ASSERT((vertexOffset + linePoints.size() * 2) <= meshData->GetVertexCount());
	B3D_ASSERT((indexOffset + linePoints.size() * 2) <= meshData->GetIndexCount());

	u32 curVertOffset = vertexOffset;
	u32 curIdxOffset = indexOffset;

	u32* indexData = meshData->GetIndices32();
	u8* positionData = meshData->GetElementData(VES_POSITION);

	u32 numPoints = (u32)linePoints.size();
	for(u32 i = 0; i < numPoints; i += 2)
	{
		PixelLine(linePoints[i], linePoints[i + 1], positionData, curVertOffset, meshData->GetVertexDescription()->GetVertexStride(), indexData, curIdxOffset);

		curVertOffset += 2;
		curIdxOffset += 2;
	}
}

void ShapeMeshes2D::QuadLineList(const Vector<Vector2>& linePoints, float width, float border, const Color& color, const TShared<MeshData>& meshData, u32 vertexOffset, u32 indexOffset)
{
	u32 numPoints = (u32)linePoints.size();
	B3D_ASSERT(numPoints >= 2);

	u32 numLines = (u32)linePoints.size() - 1;
	B3D_ASSERT((vertexOffset + (numLines * 2 + 2)) <= meshData->GetVertexCount());
	B3D_ASSERT((indexOffset + (numLines * 6)) <= meshData->GetIndexCount());

	u32* outIndices = indexOffset + meshData->GetIndices32();
	u8* outVertices = vertexOffset + meshData->GetElementData(VES_POSITION);
	u8* outColors = vertexOffset + meshData->GetElementData(VES_COLOR);

	u32 vertexStride = meshData->GetVertexDescription()->GetVertexStride();
	QuadLineList(&linePoints[0], numPoints, width, border, outVertices, vertexStride, true);

	RGBA colorValue = color.GetAsRgba();

	// Colors and indices
	for(u32 i = 0; i < numLines; i++)
	{
		memcpy(outColors, &colorValue, sizeof(colorValue));
		outColors += vertexStride;

		memcpy(outColors, &colorValue, sizeof(colorValue));
		outColors += vertexStride;

		u32 idxStart = i * 6;
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

void ShapeMeshes2D::QuadLineList(const Vector2* linePoints, u32 numPoints, float width, float border, u8* outVertices, u32 vertexStride, bool indexed)
{
	B3D_ASSERT(numPoints >= 2);
	u32 numLines = numPoints - 1;

	width += border;

	Vector2 prevPoints[2];

	// Start segment
	{
		Vector2 a = linePoints[0];
		Vector2 b = linePoints[1];

		Vector2 diff = b - a;
		diff.Normalize();

		// Flip 90 degrees
		Vector2 normal(diff.Y, -diff.X);

		prevPoints[0] = a - normal * width - diff * border;
		prevPoints[1] = a + normal * width - diff * border;

		memcpy(outVertices, &prevPoints[0], sizeof(prevPoints[0]));
		outVertices += vertexStride;

		memcpy(outVertices, &prevPoints[1], sizeof(prevPoints[1]));
		outVertices += vertexStride;
	}

	// Middle segments
	{
		for(u32 i = 1; i < numLines; i++)
		{
			Vector2 a = linePoints[i - 1];
			Vector2 b = linePoints[i];
			Vector2 c = linePoints[i + 1];

			Vector2 diffPrev = b - a;
			diffPrev.Normalize();

			Vector2 diffNext = c - b;
			diffNext.Normalize();

			// Flip 90 degrees
			Vector2 normalPrev(diffPrev.Y, -diffPrev.X);
			Vector2 normalNext(diffNext.Y, -diffNext.X);

			Vector2 curPoints[2];

			const float sign[] = { -1.0f, 1.0f };
			for(u32 j = 0; j < 2; j++)
			{
				Vector2 linePrevPoint = a + normalPrev * width * sign[j];
				Line2 linePrev(linePrevPoint, diffPrev);

				Vector2 lineNextPoint = b + normalNext * width * sign[j];
				Line2 lineNext(lineNextPoint, diffNext);

				auto intersect = linePrev.Intersects(lineNext);
				if(intersect.second != 0.0f) // Not parallel
					curPoints[j] = linePrev.GetPoint(intersect.second);
				else
					curPoints[j] = lineNextPoint;

				memcpy(outVertices, &curPoints[j], sizeof(curPoints[j]));
				outVertices += vertexStride;
			}

			if(!indexed)
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
		Vector2 normal(diff.Y, -diff.X);

		Vector2 curPoints[2];
		curPoints[0] = b - normal * width + diff * border;
		curPoints[1] = b + normal * width + diff * border;

		memcpy(outVertices, &curPoints[0], sizeof(curPoints[0]));
		outVertices += vertexStride;

		memcpy(outVertices, &curPoints[1], sizeof(curPoints[1]));
		outVertices += vertexStride;

		if(!indexed)
		{
			memcpy(outVertices, &curPoints[0], sizeof(curPoints[0]));
			outVertices += vertexStride;

			memcpy(outVertices, &prevPoints[1], sizeof(prevPoints[1]));
			outVertices += vertexStride;
		}
	}
}

void ShapeMeshes2D::PixelSolidPolygon(const Vector<Vector2>& points, u8* outVertices, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset)
{
	outVertices += (vertexOffset * vertexStride);

	for(auto& point : points)
	{
		Vector2* vertices = (Vector2*)outVertices;
		(*vertices) = point;

		outVertices += vertexStride;
	}

	outIndices += indexOffset;
	i32 numPoints = (i32)points.size();
	u32 idxCnt = 0;
	for(int i = 2; i < numPoints; i++)
	{
		outIndices[idxCnt++] = vertexOffset;
		outIndices[idxCnt++] = vertexOffset + i - 1;
		outIndices[idxCnt++] = vertexOffset + i;
	}
}

void ShapeMeshes2D::PixelLine(const Vector2& a, const Vector2& b, u8* outVertices, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset)
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
