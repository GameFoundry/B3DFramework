//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsReflectionProbeSampling.h"
#include "BsMaterial.h"
#include "BsShader.h"
#include "BsRenderBeast.h"
#include "BsGpuBuffer.h"
#include "BsReflectionProbe.h"
#include "BsLightProbeCache.h"

namespace bs { namespace ct
{
	static const UINT32 BUFFER_INCREMENT = 16 * sizeof(ReflProbeData);

	ReflProbeParamsParamDef gReflProbeParamsParamDef;

	GPUReflProbeData::GPUReflProbeData()
		:mNumProbes(0)
	{ }

	void GPUReflProbeData::setProbes(const Vector<ReflProbeData>& probeData, UINT32 numProbes)
	{
		mNumProbes = numProbes;

		UINT32 size = numProbes * sizeof(ReflProbeData);
		UINT32 curBufferSize;

		if (mProbeBuffer != nullptr)
			curBufferSize = mProbeBuffer->getSize();
		else
			curBufferSize = 0;

		if (size > curBufferSize || curBufferSize == 0)
		{
			// Allocate at least one block even if no probes, to avoid issues with null buffers
			UINT32 bufferSize = std::max(1, Math::ceilToInt(size / (float)BUFFER_INCREMENT)) * BUFFER_INCREMENT;

			GPU_BUFFER_DESC bufferDesc;
			bufferDesc.type = GBT_STRUCTURED;
			bufferDesc.elementCount = bufferSize / sizeof(ReflProbeData);
			bufferDesc.elementSize = sizeof(ReflProbeData);
			bufferDesc.format = BF_UNKNOWN;

			mProbeBuffer = GpuBuffer::create(bufferDesc);
		}

		if (size > 0)
			mProbeBuffer->writeData(0, size, probeData.data(), BWT_DISCARD);
	}

	RendererReflectionProbe::RendererReflectionProbe(ReflectionProbe* probe)
		:probe(probe)
	{
		arrayIdx = -1;
		texture = nullptr;
		customTexture = probe->getCustomTexture() != nullptr;
		textureDirty = LightProbeCache::instance().isRadianceDirty(probe->getUUID());
		arrayDirty = true;
		errorFlagged = false;
	}

	void RendererReflectionProbe::getParameters(ReflProbeData& output) const
	{
		output.type = probe->getType() == ReflectionProbeType::Sphere ? 0 
			: probe->getType() == ReflectionProbeType::Box ? 1 : 2;
		
		output.position = probe->getPosition();
		output.boxExtents = probe->getExtents();

		if (probe->getType() == ReflectionProbeType::Sphere)
			output.radius = probe->getRadius();
		else
			output.radius = output.boxExtents.length();

		output.transitionDistance = probe->getTransitionDistance();
		output.cubemapIdx = arrayIdx;
		output.invBoxTransform.setInverseTRS(output.position, probe->getRotation(), output.boxExtents);
	}
}}