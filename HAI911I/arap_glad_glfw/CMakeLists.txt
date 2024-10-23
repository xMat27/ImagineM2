# ----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.0)

SET(CMAKE_EXPORT_COMPILE_COMMANDS 1)
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

project(tpArap)

add_executable(
  tpArap
 	gmini.cpp
	src/Camera.h
	src/Camera.cpp
	src/LaplacianWeights.h
	src/linearSystem.h
	src/Mesh.h
	src/Mesh.cpp
	src/RectangleSelectionTool.h
	src/Scene.h
	src/Trackball.cpp
	src/Trackball.h
	src/Vec3.h)

add_subdirectory(extern/glad)
target_link_libraries(${PROJECT_NAME} PRIVATE glad)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
add_subdirectory(extern/glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)


target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_DL_LIBS})

add_custom_command(TARGET tpArap
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:tpArap> ${CMAKE_CURRENT_SOURCE_DIR})



