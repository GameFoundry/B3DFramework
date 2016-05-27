//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsFMODAudioClip.h"
#include "BsDataStream.h"

namespace BansheeEngine
{
	FMODAudioClip::FMODAudioClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples, const AUDIO_CLIP_DESC& desc)
		:AudioClip(samples, streamSize, numSamples, desc)
	{ }

	FMODAudioClip::~FMODAudioClip()
	{
		// TODO
	}

	void FMODAudioClip::initialize()
	{
		// TODO

		AudioClip::initialize();
	}

	void FMODAudioClip::getSamples(UINT8* samples, UINT32 offset, UINT32 count) const
	{
		// TODO
	}

	SPtr<DataStream> FMODAudioClip::getSourceStream(UINT32& size)
	{
		// TODO
		return nullptr;
	}
}