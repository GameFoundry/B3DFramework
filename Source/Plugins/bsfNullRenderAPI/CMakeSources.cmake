set(BS_NULLRENDERAPI_INC_NOFILTER
	"BsNullTexture.h"
	"BsNullRenderTargets.h"
	"BsNullRenderStates.h"
	"BsNullPrerequisites.h"
	"BsNullBuffers.h"
	"BsNullRenderAPI.h"
	"BsNullRenderAPIFactory.h"
	"BsNullCommandBuffer.h"
	"BsNullQueries.h"
)

set(BS_NULLRENDERAPI_SRC_NOFILTER
	"BsNullTexture.cpp"
	"BsNullRenderTargets.cpp"
	"BsNullRenderStates.cpp"
	"BsNullPlugin.cpp"
	"BsNullBuffers.cpp"
	"BsNullRenderAPI.cpp"
	"BsNullRenderAPIFactory.cpp"
	"BsNullCommandBuffer.cpp"
	"BsNullQueries.cpp"
	"BsNullWin32Resource.rc"
)

source_group("" FILES ${BS_NULLRENDERAPI_SRC_NOFILTER} ${BS_NULLRENDERAPI_INC_NOFILTER})

set(BS_NULLRENDERAPI_SRC
	${BS_NULLRENDERAPI_INC_NOFILTER}
	${BS_NULLRENDERAPI_SRC_NOFILTER}
)