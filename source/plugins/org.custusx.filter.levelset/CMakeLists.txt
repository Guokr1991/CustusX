project(org_custusx_filter_levelset)

###############################################################################
# Initialize Level-Set-Segmentation library
# Find the package and run the include USE file.
###############################################################################
macro(cx_initialize_level_set_segmentation)
    find_package( Level-Set-Segmentation REQUIRED)
    include(${Level-Set-Segmentation_USE_FILE})
endmacro()

cx_initialize_level_set_segmentation()

set(PLUGIN_export_directive "${PROJECT_NAME}_EXPORT")

set(PLUGIN_SRCS
    cxFilterLevelSetPluginActivator.cpp
    cxLevelSetFilterService.cpp
)

# Files which should be processed by Qts moc
set(PLUGIN_MOC_SRCS
    cxFilterLevelSetPluginActivator.h
    cxLevelSetFilterService.h
)

set(PLUGIN_UI_FORMS
)

# QRC Files which should be compiled into the plugin
set(PLUGIN_resources
)

#Compute the plugin dependencies
ctkFunctionGetTargetLibraries(PLUGIN_target_libraries)
set(PLUGIN_target_libraries 
	PUBLIC
	cxResource
	cxResourceFilter
	cxResourceVisualization
	PRIVATE
    ${PLUGIN_target_libraries}   
    levelSetSegLib
	cxPluginUtilities
	${ITK_LIBRARIES}
	)

if (CX_USE_OPENCL_UTILITY)
    set(PLUGIN_target_libraries ${PLUGIN_target_libraries} OpenCLUtilityLibrary ${OPENCL_LIBRARIES})
    include_directories(${OpenCLUtilityLibrary_INCLUDE_DIRS})
endif(CX_USE_OPENCL_UTILITY)

set(PLUGIN_OUTPUT_DIR "")
if(CX_WINDOWS)
    #on windows we want dlls to be placed with the executables
    set(PLUGIN_OUTPUT_DIR "../")
endif(CX_WINDOWS)

ctkMacroBuildPlugin(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${PLUGIN_export_directive}
  SRCS ${PLUGIN_SRCS}
  MOC_SRCS ${PLUGIN_MOC_SRCS}
  UI_FORMS ${PLUGIN_UI_FORMS}
  RESOURCES ${PLUGIN_resources}
  TARGET_LIBRARIES ${PLUGIN_target_libraries}
  OUTPUT_DIR ${PLUGIN_OUTPUT_DIR}
)

install(FILES
        ${Level-Set-Segmentation_KERNELS_DIR}/kernels.cl
        DESTINATION ${CX_INSTALL_ROOT_DIR}/config/lss/)

target_include_directories(org_custusx_filter_levelset
    PUBLIC
    .
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_subdirectory(testing)

