#include "BsGLPrerequisites.h"
#include "BsGLRenderAPIFactory.h"

namespace bs
{
	extern "C" BS_RSGL_EXPORT const char* getPluginName()
	{
		return SystemName;
	}
}