# Source files and their filters
include(CMakeSources.cmake)

# Packages
if(RENDER_API_MODULE MATCHES "Vulkan")
	find_package(Vulkan)
	find_package(glslang)
endif()

# Includes
set(BansheeVulkanRenderAPI_INC 
	"Include" 
	"../BansheeUtility/Include" 
	"../BansheeCore/Include"
	${Vulkan_INCLUDE_DIR}
	${glslang_INCLUDE_DIR})

if(WIN32)
	set(BansheeVulkanRenderAPI_INC ${BansheeVulkanRenderAPI_INC} "Source/Win32")
else()
# TODO_OTHER_PLATFORMS_GO_HERE
endif()	
	
include_directories(${BansheeVulkanRenderAPI_INC})	
	
# Target
add_library(BansheeVulkanRenderAPI SHARED ${BS_BANSHEEVULKANRENDERAPI_SRC})

# Libraries	
## External libs: Vulkan, glslang
add_package_library(BansheeVulkanRenderAPI Vulkan)
add_package_library(BansheeVulkanRenderAPI glslang)

## Local libs
target_link_libraries(BansheeVulkanRenderAPI PRIVATE BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET BansheeVulkanRenderAPI PROPERTY FOLDER Plugins)