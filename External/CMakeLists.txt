foreach(sentinel
        Assimp/CMakeLists.txt
        PhysX/physx/compiler/public/CMakeLists.txt
        openal-soft/CMakeLists.txt
        yaml-cpp/CMakeLists.txt
        ImGui/imgui.h
        ImGuizmo/src/ImGuizmo.h
        stb/stb_image.h
        lua/src/lua.h
        adhoc-entity/include/adh/entity.hpp
        adhoc-event/include/adh/event.hpp
        adhoc-math/include/adh/math.hpp
        adhoc-script/include/adh/script.hpp)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${sentinel}")
        message(FATAL_ERROR "External/${sentinel} is missing, fetch the submodules with:\n"
                            "  git submodule update --init --recursive")
    endif()
endforeach()

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
if(COMMAND cmake_diagnostic)
    cmake_diagnostic(SET CMD_POLICY IGNORE)
endif()
set(BUILD_SHARED_LIBS OFF)
set(BUILD_TESTING OFF)
set(CMAKE_DEBUG_POSTFIX "")

set(ASSIMP_BUILD_TESTS          OFF)
set(ASSIMP_BUILD_ASSIMP_TOOLS   OFF)
set(ASSIMP_BUILD_SAMPLES        OFF)
set(ASSIMP_BUILD_DOCS           OFF)
set(ASSIMP_INSTALL              OFF)
set(ASSIMP_NO_EXPORT            ON)
set(ASSIMP_WARNINGS_AS_ERRORS   OFF)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF)
set(ASSIMP_BUILD_MINIZIP        ON)
add_subdirectory(Assimp)
target_compile_definitions(assimp PRIVATE ASSIMP_USE_STB_IMAGE_STATIC)
target_compile_options(assimp PRIVATE
    $<$<CXX_COMPILER_ID:Clang,AppleClang>:-Wno-unsafe-buffer-usage -Wno-unsafe-buffer-usage-in-libc-call>)

if(APPLE)
    set(TARGET_BUILD_PLATFORM mac)
else()
    string(TOLOWER "${CMAKE_SYSTEM_NAME}" TARGET_BUILD_PLATFORM)
endif()
set(PHYSX_ROOT_DIR                ${CMAKE_CURRENT_SOURCE_DIR}/PhysX/physx)
set(PXSHARED_PATH                 ${PHYSX_ROOT_DIR}/../pxshared)
set(PXSHARED_INSTALL_PREFIX       ${CMAKE_INSTALL_PREFIX})
set(CMAKEMODULES_VERSION          "1.27")
set(CMAKEMODULES_PATH             ${PHYSX_ROOT_DIR}/../externals/cmakemodules)
set(PX_OUTPUT_BIN_DIR             ${CMAKE_CURRENT_BINARY_DIR}/PhysX)
set(PX_OUTPUT_LIB_DIR             ${CMAKE_BINARY_DIR}/${LIB_PATH})
set(PX_GENERATE_STATIC_LIBRARIES  ON)
set(PX_BUILDSNIPPETS              OFF)
set(PX_BUILDPUBLICSAMPLES         OFF)
set(NV_USE_DEBUG_WINCRT           ON)
add_subdirectory(${PHYSX_ROOT_DIR}/compiler/public PhysX)

get_property(physx_targets DIRECTORY ${PHYSX_ROOT_DIR}/source/compiler/cmake PROPERTY BUILDSYSTEM_TARGETS)
foreach(target IN LISTS physx_targets)
    get_target_property(type ${target} TYPE)
    if(type MATCHES "^(STATIC|SHARED|MODULE|OBJECT)_LIBRARY$")
        target_compile_options(${target} PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w> $<$<CXX_COMPILER_ID:MSVC>:/WX->)
    endif()
endforeach()
target_compile_definitions(PhysXFoundation INTERFACE PX_PHYSX_STATIC_LIB)

set(LIBTYPE                       SHARED)
set(ALSOFT_UTILS                  OFF)
set(ALSOFT_EXAMPLES               OFF)
set(ALSOFT_TESTS                  OFF)
set(ALSOFT_INSTALL                OFF)
set(ALSOFT_INSTALL_CONFIG         OFF)
set(ALSOFT_INSTALL_HRTF_DATA      OFF)
set(ALSOFT_INSTALL_AMBDEC_PRESETS OFF)
set(ALSOFT_INSTALL_EXAMPLES       OFF)
set(ALSOFT_INSTALL_UTILS          OFF)
set(ALSOFT_ENABLE_MODULES         OFF)
set(ALSOFT_UPDATE_BUILD_VERSION   OFF)
set(ALSOFT_DLOPEN_NOTES           OFF)
add_subdirectory(openal-soft)

set(YAML_BUILD_SHARED_LIBS OFF)
set(YAML_CPP_BUILD_TOOLS   OFF)
set(YAML_CPP_BUILD_TESTS   OFF)
set(YAML_CPP_BUILD_CONTRIB OFF)
set(YAML_CPP_INSTALL       OFF)
set(YAML_CPP_FORMAT_SOURCE OFF)
add_subdirectory(yaml-cpp)

add_library(ImGui STATIC
    ImGui/imgui.cpp
    ImGui/imgui_demo.cpp
    ImGui/imgui_draw.cpp
    ImGui/imgui_tables.cpp
    ImGui/imgui_widgets.cpp)
target_include_directories(ImGui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ImGui)
target_compile_features(ImGui PUBLIC cxx_std_11)

add_library(ImGuizmo STATIC ImGuizmo/src/ImGuizmo.cpp)
target_include_directories(ImGuizmo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo/src)
target_link_libraries(ImGuizmo PUBLIC ImGui)
target_compile_features(ImGuizmo PRIVATE cxx_std_17)

set(LUA_SRC
    lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c
    lopcodes.c lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c
    lauxlib.c lbaselib.c lcorolib.c ldblib.c liolib.c lmathlib.c loadlib.c loslib.c
    lstrlib.c ltablib.c lutf8lib.c linit.c)
list(TRANSFORM LUA_SRC PREPEND lua/src/)
add_library(Lua STATIC ${LUA_SRC})
target_include_directories(Lua PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lua/src)
set_target_properties(Lua PROPERTIES C_STANDARD 99 C_EXTENSIONS ON)
if(WIN32)
    target_compile_definitions(Lua PRIVATE _CRT_SECURE_NO_WARNINGS)
elseif(APPLE)
    target_compile_definitions(Lua PRIVATE LUA_USE_MACOSX)
elseif(UNIX)
    target_compile_definitions(Lua PRIVATE LUA_USE_LINUX)
    target_link_libraries(Lua PRIVATE m ${CMAKE_DL_LIBS})
endif()

add_library(adhoc-entity INTERFACE)
target_include_directories(adhoc-entity SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/adhoc-entity/include)
target_compile_features(adhoc-entity INTERFACE cxx_std_23)

add_library(adhoc-event INTERFACE)
target_include_directories(adhoc-event SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/adhoc-event/include)
target_compile_features(adhoc-event INTERFACE cxx_std_23)

add_library(adhoc-math INTERFACE)
target_include_directories(adhoc-math SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/adhoc-math/include)
target_compile_features(adhoc-math INTERFACE cxx_std_20)
if(NOT APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64|i[3-6]86|x86)$")
    target_compile_options(adhoc-math INTERFACE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-msse4.1>)
endif()

add_library(adhoc-script INTERFACE)
target_include_directories(adhoc-script SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/adhoc-script/include)
target_compile_features(adhoc-script INTERFACE cxx_std_23)
target_link_libraries(adhoc-script INTERFACE Lua)
