cmake_minimum_required(VERSION 3.20)
project(ShadowForge3D VERSION 0.1.0 LANGUAGES C CXX)

# Use C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# Dependency Resolution
# -----------------------------------------------------------------------------

if(NOT EMSCRIPTEN)
    # 1. GLFW (Window management & input handling)
    find_package(glfw3 CONFIG QUIET)
    if(NOT glfw3_FOUND)
        message(STATUS "GLFW3 package not found locally, fetching via FetchContent...")
        include(FetchContent)
        FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.4
        )
        set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(glfw)
    else()
        message(STATUS "Found installed GLFW3: ${glfw3_DIR}")
    endif()

    # 3. GLAD (OpenGL 3.3 Core Function Pointer Loader)
    add_library(glad STATIC
        external/glad/src/gl.c
    )
    target_include_directories(glad PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
    )
endif()

# 2. GLM (OpenGL Mathematics: vectors, matrices, transforms)
find_package(glm CONFIG QUIET)
if(NOT glm_FOUND)
    message(STATUS "GLM package not found locally, fetching via FetchContent...")
    include(FetchContent)
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1
    )
    FetchContent_MakeAvailable(glm)
else()
    message(STATUS "Found installed GLM: ${glm_DIR}")
endif()

# -----------------------------------------------------------------------------
# Main Executable Target
# -----------------------------------------------------------------------------
add_executable(ShadowForge3D
    src/main.cpp
    src/Matrix4.cpp
    src/Mesh.cpp
    src/Sculpture.cpp
    src/ShadowProjection.cpp
    src/ShadowMatcher.cpp
    src/Level.cpp
    src/HUD.cpp
)

target_include_directories(ShadowForge3D PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ShadowForge3D PRIVATE
    glm::glm
)

if(EMSCRIPTEN)
    set_target_properties(ShadowForge3D PROPERTIES
        OUTPUT_NAME "index"
        SUFFIX ".html"
    )
    target_link_options(ShadowForge3D PRIVATE
        "-sUSE_GLFW=3"
        "-sFULL_ES3=1"
        "-sMAX_WEBGL_VERSION=2"
        "-sMIN_WEBGL_VERSION=2"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sINITIAL_MEMORY=64MB"
        "--preload-file" "${CMAKE_CURRENT_SOURCE_DIR}/shaders@shaders"
        "--shell-file" "${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html"
    )
else()
    target_link_libraries(ShadowForge3D PRIVATE
        glad
        glfw
    )
    if(WIN32)
        target_link_libraries(ShadowForge3D PRIVATE opengl32)
    endif()

    # Copy shaders directory next to executable on build
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
        add_custom_command(TARGET ShadowForge3D POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
                "$<TARGET_FILE_DIR:ShadowForge3D>/shaders"
            COMMENT "Syncing shaders to output directory"
        )
    endif()
endif()

# -----------------------------------------------------------------------------
# Desktop-Only Test & Audit Targets
# -----------------------------------------------------------------------------
if(NOT EMSCRIPTEN)
    add_executable(audit_levels
        tests/audit_levels.cpp
        src/Matrix4.cpp
        src/Mesh.cpp
        src/Sculpture.cpp
        src/ShadowProjection.cpp
        src/ShadowMatcher.cpp
        src/Level.cpp
    )
    target_include_directories(audit_levels PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(audit_levels PRIVATE
        glad
        glfw
        glm::glm
    )
    if(WIN32)
        target_link_libraries(audit_levels PRIVATE opengl32)
    endif()

    add_executable(diagnose_matcher
        tests/diagnose_matcher.cpp
        src/Matrix4.cpp
        src/Mesh.cpp
        src/Sculpture.cpp
        src/ShadowProjection.cpp
        src/ShadowMatcher.cpp
        src/Level.cpp
    )
    target_include_directories(diagnose_matcher PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(diagnose_matcher PRIVATE
        glad
        glfw
        glm::glm
    )
    if(WIN32)
        target_link_libraries(diagnose_matcher PRIVATE opengl32)
    endif()
endif()



