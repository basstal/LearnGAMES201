cmake_minimum_required(VERSION 3.10)

set(ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../..")

if (MSVC)
    set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT.LIB")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG
        "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT.LIB")
    set(CMAKE_CXX_FLAGS_RELEASE "/MT")
    set(CMAKE_CXX_FLAGS_DEBUG "/MDd /DEBUG")
endif()

get_filename_component(ROOT_PATH ${ROOT_PATH} REALPATH)

# lib source glad
add_library(glad "${ROOT_PATH}/Plugins/glad/glad.c")

target_include_directories(glad PUBLIC "${ROOT_PATH}/Plugins")

# lib source imgui
add_library(imgui
    "${ROOT_PATH}/Plugins/imgui/imgui.cpp"
    "${ROOT_PATH}/Plugins/imgui/imgui_widgets.cpp"
    "${ROOT_PATH}/Plugins/imgui/imgui_draw.cpp"
    "${ROOT_PATH}/Plugins/imgui/imgui_demo.cpp"
    "${ROOT_PATH}/Plugins/imgui/imgui_impl_glfw.cpp"
    "${ROOT_PATH}/Plugins/imgui/imgui_impl_opengl3.cpp")

target_include_directories(imgui PUBLIC
    "${ROOT_PATH}/Plugins/imgui/"
    "${ROOT_PATH}/Plugins/glfw-3.3.4/include"
    "${ROOT_PATH}/Plugins")

# lib source Icosphere
add_library(lcosphere
    "${ROOT_PATH}/Plugins/others/Icosphere.cpp")

target_include_directories(lcosphere PUBLIC
    "${ROOT_PATH}/Plugins/others"
    "${ROOT_PATH}/Plugins/")

set(PROJECT_NAME "ParticleKinematics")

# Project name
project(${PROJECT_NAME})

# project source
add_executable(${PROJECT_NAME} 
    ParticleKinematics.cpp
    "${ROOT_PATH}/Scripts/Shader.cpp"
    "${ROOT_PATH}/Scripts/Utils.cpp"
    "${ROOT_PATH}/Scripts/STBImage.cpp")

target_link_libraries(${PROJECT_NAME} 
    glad
    imgui
    lcosphere)

if(MSVC)
    target_link_libraries(${PROJECT_NAME}
        "${ROOT_PATH}/Plugins/glfw-3.3.4/lib-vc2019/glfw3_mt.lib")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC 
    "${ROOT_PATH}/Plugins"
    "${ROOT_PATH}/Plugins/imgui"
    "${ROOT_PATH}/Plugins/glfw-3.3.4/include"
    "${ROOT_PATH}/Scripts/include")



# Particle Explosion
set(PROJECT_NAME "ParticleExplosion")


# project source
add_executable(${PROJECT_NAME} 
    ParticleExplosion.cpp
    "${ROOT_PATH}/Scripts/Shader.cpp"
    "${ROOT_PATH}/Scripts/Utils.cpp"
    "${ROOT_PATH}/Scripts/STBImage.cpp")

target_link_libraries(${PROJECT_NAME} 
    glad
    imgui
    lcosphere)
    
if(MSVC)
    target_link_libraries(${PROJECT_NAME}
        "${ROOT_PATH}/Plugins/glfw-3.3.4/lib-vc2019/glfw3_mt.lib")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC 
    "${ROOT_PATH}/Plugins"
    "${ROOT_PATH}/Plugins/imgui"
    "${ROOT_PATH}/Plugins/glfw-3.3.4/include"
    "${ROOT_PATH}/Scripts/include")


