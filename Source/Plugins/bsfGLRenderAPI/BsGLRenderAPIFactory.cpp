//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLRenderAPIFactory.h"
#include "BsGLRenderAPI.h"

namespace bs
{
	namespace ct
	{
		constexpr const char* GLRenderAPIFactory::SystemName;

		void GLRenderAPIFactory::Create()
		{
			RenderAPI::StartUp<GLRenderAPI>();
		}

		GLRenderAPIFactory::InitOnStart GLRenderAPIFactory::initOnStart;
	} // namespace ct
} // namespace bs
