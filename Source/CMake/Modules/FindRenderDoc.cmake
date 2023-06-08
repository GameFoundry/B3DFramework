# Find RenderDoc
#
# This module defines
#  RenderDoc_INCLUDE_DIRS
#  RenderDoc_LIBRARIES
#  RenderDoc_FOUND

start_find_package(RenderDoc)

set(RenderDoc_INSTALL_DIR ${BSF_DEPENDENCY_DIRECTORY}/RenderDoc CACHE PATH "")
gen_default_lib_search_dirs(RenderDoc)

find_imported_includes(RenderDoc RenderDoc/renderdoc_app.h)

add_library(RenderDoc::renderdoc INTERFACE IMPORTED)
set(RenderDoc_LIBRARIES RenderDoc::renderdoc)

if(WIN32)
    list(APPEND RenderDoc_LIBRARY_RELEASE_SEARCH_DIRS "${RenderDoc_INSTALL_DIR}/bin")
    list(APPEND RenderDoc_LIBRARY_DEBUG_SEARCH_DIRS "${RenderDoc_INSTALL_DIR}/bin")
    install_dependency_binary(RenderDoc renderdoc)
endif()

end_find_package(RenderDoc renderdoc)