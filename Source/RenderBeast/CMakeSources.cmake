set(BS_RENDERBEAST_INC_NOFILTER
	"Include/BsGpuResourcePool.h"
	"Include/BsRenderBeastOptions.h"
	"Include/BsSamplerOverrides.h"
	"Include/BsRenderBeast.h"
	"Include/BsRenderBeastFactory.h"
	"Include/BsRenderBeastPrerequisites.h"
	"Include/BsObjectRendering.h"
	"Include/BsLightRendering.h"
	"Include/BsPostProcessing.h"
	"Include/BsRendererView.h"
	"Include/BsRendererObject.h"
	"Include/BsLightGrid.h"
	"Include/BsImageBasedLighting.h"
	"Include/BsShadowRendering.h"
	"Include/BsRendererScene.h"
	"Include/BsStandardDeferredLighting.h"
	"Include/BsLightProbes.h"
	"Include/BsRenderCompositor.h"
	"Include/BsRendererTextures.h"
)

set(BS_RENDERBEAST_SRC_NOFILTER
	"Source/BsGpuResourcePool.cpp"
	"Source/BsSamplerOverrides.cpp"
	"Source/BsRenderBeast.cpp"
	"Source/BsRenderBeastFactory.cpp"
	"Source/BsRenderBeastPlugin.cpp"
	"Source/BsObjectRendering.cpp"
	"Source/BsLightRendering.cpp"
	"Source/BsPostProcessing.cpp"
	"Source/BsRendererView.cpp"
	"Source/BsRendererObject.cpp"
	"Source/BsLightGrid.cpp"
	"Source/BsImageBasedLighting.cpp"
	"Source/BsShadowRendering.cpp"
	"Source/BsRendererScene.cpp"
	"Source/BsStandardDeferredLighting.cpp"
	"Source/BsLightProbes.cpp"
	"Source/BsRenderCompositor.cpp"
	"Source/BsRendererTextures.cpp"
)

source_group("Header Files" FILES ${BS_RENDERBEAST_INC_NOFILTER})
source_group("Source Files" FILES ${BS_RENDERBEAST_SRC_NOFILTER})

set(BS_RENDERBEAST_SRC
	${BS_RENDERBEAST_INC_NOFILTER}
	${BS_RENDERBEAST_SRC_NOFILTER}
)