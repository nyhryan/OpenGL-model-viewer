# CMakeList.txt : CMake project for Chapter1, include source and define
# project specific logic here.

# visual studio auto generated config
if (CMAKE_VERSION VERSION_GREATER 3.12)
	set_property(TARGET ${dirname} PROPERTY CXX_STANDARD 20)
endif()

# get name of current directory
get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
string(REPLACE " " "_" dirname ${dirname})

# sets output directory to bin/${dirname}/
set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/bin/${dirname})

# make executable's name as directory name
add_executable (${dirname}
	"${dirname}.cpp"
	App.cpp
	Mesh.cpp
	Model.cpp
	Shader.cpp
	Camera.cpp
	${HELPER}
)

target_include_directories(${dirname} PRIVATE ${Stb_INCLUDE_DIR})

# link libraries
target_link_libraries(${dirname} PRIVATE OpenGL::GL)
target_link_libraries(${dirname} PRIVATE glfw)
target_link_libraries(${dirname} PRIVATE gl3w)
target_link_libraries(${dirname} PRIVATE fmt::fmt)
target_link_libraries(${dirname} PRIVATE imgui::imgui)
target_link_libraries(${dirname} PRIVATE assimp::assimp)
target_link_libraries(${dirname} PRIVATE glm::glm-header-only)


# add _d at the end of .exe in debug build
# set executable output directory to bin/${dirname}/
set_target_properties(${dirname} PROPERTIES
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
)

# copy_resources: copy shader scripts to bin/${dirname}/shaders
add_custom_target(
	"copy_${dirname}_resources" ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_CURRENT_LIST_DIR}/resources
	${OUTPUT_DIR}/resources
	COMMENT "Copying resource files to \'${OUTPUT_DIR}\'..."
)
add_dependencies(${dirname} "copy_${dirname}_resources")