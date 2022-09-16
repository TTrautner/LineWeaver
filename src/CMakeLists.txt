file(GLOB_RECURSE lineweaver_sources *.cpp *.h)
add_executable(lineweaver ${lineweaver_sources})
set_target_properties(lineweaver PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

find_package(glm CONFIG REQUIRED)
target_link_libraries(lineweaver PRIVATE glm::glm)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(lineweaver PRIVATE glfw)

find_package(glbinding CONFIG REQUIRED)
target_link_libraries(lineweaver PRIVATE glbinding::glbinding glbinding::glbinding-aux)

find_package(globjects CONFIG REQUIRED)
target_link_libraries(lineweaver PRIVATE globjects::globjects)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(lineweaver PRIVATE imgui::imgui)

find_package(tinyfiledialogs CONFIG REQUIRED)
target_link_libraries(lineweaver PRIVATE tinyfiledialogs::tinyfiledialogs)

find_path(STB_INCLUDE_DIRS "stb_c_lexer.h")
target_include_directories(lineweaver PRIVATE ${STB_INCLUDE_DIRS})
