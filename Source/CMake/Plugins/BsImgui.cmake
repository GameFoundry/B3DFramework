include(FetchContent)


# if INCLUDE_IMGUI_PLUGIN is true, then git clone the imgui plugin (with
# recursive submodules). it will be cloned into the build directory where it
# will be configured as bsfImgui target. other targets can then set bsfImgui
# as their dependency in their respective cmake files.

set(FETCHCONTENT_QUIET 0)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/pgruenbacher/bsfimgui.git
  GIT_PROGRESS TRUE
)

FetchContent_GetProperties(imgui)
if(NOT googletest_POPULATED)
	FetchContent_Populate(imgui)

	add_subdirectory(${imgui_SOURCE_DIR} ${imgui_BINARY_DIR})

	if (TARGET bsfImgui)
		# Do something.
		message( STATUS "bsfImgui target available")
	else()		
  		message( SEND_ERROR "bsfImgui target not available" )
	endif()
endif()
