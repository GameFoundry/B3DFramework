set(BS_RENDERBEAST_INC_NOFILTER
	"BsRenderBeastOptions.h"
	"BsRenderBeast.h"
	"BsRenderBeastFactory.h"
	"BsRenderBeastPrerequisites.h"
	"BsRendererLight.h"
	"BsRendererView.h"
	"BsRendererRenderable.h"
	"BsRendererParticles.h"
	"BsRendererReflectionProbe.h"
	"BsRendererScene.h"
	"BsRenderCompositor.h"
	"BsRenderBeastIBLUtility.h"
)

set(BS_RENDERBEAST_SRC_NOFILTER
	"BsRenderBeast.cpp"
	"BsRenderBeastFactory.cpp"
	"BsRenderBeastTestSuite.cpp"
	"BsRenderBeastPlugin.cpp"
	"BsRendererLight.cpp"
	"BsRendererView.cpp"
	"BsRendererRenderable.cpp"
	"BsRendererParticles.cpp"
	"BsRendererReflectionProbe.cpp"
	"BsRendererScene.cpp"
	"BsRenderCompositor.cpp"
	"BsRenderBeastIBLUtility.cpp"
)

set(BS_RENDERBEAST_INC_SHADING
	"Shading/BsTiledDeferred.h"
	"Shading/BsStandardDeferred.h"
	"Shading/BsLightGrid.h"
	"Shading/BsLightProbes.h"
	"Shading/BsShadowRendering.h"
	"Shading/BsPostProcessing.h"
	"Shading/BsGpuParticleSimulation.h"
)

set(BS_RENDERBEAST_SRC_SHADING
	"Shading/BsTiledDeferred.cpp"
	"Shading/BsStandardDeferred.cpp"
	"Shading/BsLightGrid.cpp"
	"Shading/BsLightProbes.cpp"
	"Shading/BsShadowRendering.cpp"
	"Shading/BsPostProcessing.cpp"
	"Shading/BsGpuParticleSimulation.cpp"
)

set(BS_RENDERBEAST_INC_UTILITY
	"Utility/BsGpuSort.h"
	"Utility/BsSamplerOverrides.h"
	"Utility/BsRendererTextures.h"
	"Utility/BsTextureRowAllocator.h"
)

set(BS_RENDERBEAST_SRC_UTILITY
	"Utility/BsGpuSort.cpp"
	"Utility/BsSamplerOverrides.cpp"
	"Utility/BsRendererTextures.cpp"
)

if(WIN32)
	set(BS_RENDERBEAST_WIN32RES
	"BsRenderBeastWin32Resource.rc"
	)
else()
	set(BS_RENDERBEAST_WIN32RES )
endif()

source_group("" FILES ${BS_RENDERBEAST_INC_NOFILTER} ${BS_RENDERBEAST_SRC_NOFILTER} ${BS_RENDERBEAST_WIN32RES})
source_group("Shading" FILES ${BS_RENDERBEAST_INC_SHADING} ${BS_RENDERBEAST_SRC_SHADING})
source_group("Utility" FILES ${BS_RENDERBEAST_INC_UTILITY} ${BS_RENDERBEAST_SRC_UTILITY})

set(BS_RENDERBEAST_SRC
	${BS_RENDERBEAST_INC_NOFILTER}
	${BS_RENDERBEAST_SRC_NOFILTER}
	${BS_RENDERBEAST_INC_SHADING}
	${BS_RENDERBEAST_SRC_SHADING}
	${BS_RENDERBEAST_INC_UTILITY}
	${BS_RENDERBEAST_SRC_UTILITY}
	${BS_RENDERBEAST_WIN32RES}
)