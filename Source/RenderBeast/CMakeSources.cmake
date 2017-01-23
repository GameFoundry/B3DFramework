set(BS_RENDERBEAST_INC_NOFILTER
	"Include/BsRenderTexturePool.h"
	"Include/BsRenderBeastOptions.h"
	"Include/BsSamplerOverrides.h"
	"Include/BsRenderBeast.h"
	"Include/BsRenderBeastFactory.h"
	"Include/BsRenderBeastPrerequisites.h"
	"Include/BsRenderTargets.h"
	"Include/BsObjectRendering.h"
	"Include/BsLightRendering.h"
	"Include/BsPostProcessing.h"
	"Include/BsRendererCamera.h"
	"Include/BsRendererObject.h"
	"Include/BsReflectionCubemap.h"
)

set(BS_RENDERBEAST_SRC_NOFILTER
	"Source/BsRenderTexturePool.cpp"
	"Source/BsSamplerOverrides.cpp"
	"Source/BsRenderBeast.cpp"
	"Source/BsRenderBeastFactory.cpp"
	"Source/BsRenderBeastPlugin.cpp"
	"Source/BsRenderTargets.cpp"
	"Source/BsObjectRendering.cpp"
	"Source/BsLightRendering.cpp"
	"Source/BsPostProcessing.cpp"
	"Source/BsRendererCamera.cpp"
	"Source/BsRendererObject.cpp"
	"Source/BsReflectionCubemap.cpp"
)

source_group("Header Files" FILES ${BS_RENDERBEAST_INC_NOFILTER})
source_group("Source Files" FILES ${BS_RENDERBEAST_SRC_NOFILTER})

set(BS_RENDERBEAST_SRC
	${BS_RENDERBEAST_INC_NOFILTER}
	${BS_RENDERBEAST_SRC_NOFILTER}
)