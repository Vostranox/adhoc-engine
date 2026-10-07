cmake_minimum_required(VERSION 3.25)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Use a separate build directory: cmake --workflow --preset debug")
endif()

if(CMAKE_HOST_APPLE)
    set(CMAKE_OSX_ARCHITECTURES x86_64 CACHE STRING "macOS architectures (bundled PhysX supports x86_64)")
endif()
project(AdHoc VERSION 1.0 LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build configuration" FORCE)
endif()

include(cmake/ProjectOptions.cmake)
set(LIB_PATH Lib)
if(APPLE)
    set(EXE_PATH App)
else()
    set(EXE_PATH Exe)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${LIB_PATH}/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${LIB_PATH}/$<CONFIG>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${EXE_PATH}/$<CONFIG>")

if(WIN32)
    add_executable(${PROJECT_NAME} WIN32)
elseif(APPLE)
    if(NOT CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
        message(FATAL_ERROR "Bundled PhysX requires -DCMAKE_OSX_ARCHITECTURES=x86_64 on macOS.")
    endif()
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER io.${PROJECT_NAME}
        MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
else()
    add_executable(${PROJECT_NAME})
endif()
adhoc_project_options(${PROJECT_NAME})

set(ADH_CONFIGURATIONS "${CMAKE_CONFIGURATION_TYPES}")
add_subdirectory(External SYSTEM)
if(ADH_CONFIGURATIONS)
    set(CMAKE_CONFIGURATION_TYPES "${ADH_CONFIGURATIONS}" CACHE STRING "Available configurations" FORCE)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/Source)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/Source/Api)
set(ADH_CORE_SRC ${CMAKE_SOURCE_DIR}/Source/Core)
target_include_directories(${PROJECT_NAME} PRIVATE ${ADH_CORE_SRC})
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/External)

set(PHYSX_LIBRARIES PhysXExtensions PhysX PhysXCooking PhysXCommon PhysXFoundation)
target_compile_definitions(${PROJECT_NAME} PRIVATE PX_GENERATE_META_DATA)
target_link_libraries(${PROJECT_NAME} PRIVATE Lua ImGui ImGuizmo assimp yaml-cpp OpenAL
    ${PHYSX_LIBRARIES} adhoc-entity adhoc-event adhoc-math adhoc-script)
if(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
endif()

if(APPLE)
    set(ADH_COPY_DATA "${PROJECT_BINARY_DIR}/${EXE_PATH}/$<CONFIG>/${PROJECT_NAME}.app/Contents/Resources/Data")
    set(ADH_DATA_RELATIVE "../Resources/Data/")
    set_target_properties(${PROJECT_NAME} PROPERTIES BUILD_RPATH "@loader_path")
    set_target_properties(OpenAL PROPERTIES MACOSX_RPATH TRUE INSTALL_NAME_DIR "@rpath" BUILD_WITH_INSTALL_NAME_DIR TRUE)
else()
    set(ADH_COPY_DATA "${PROJECT_BINARY_DIR}/${EXE_PATH}/$<CONFIG>/Data")
    set(ADH_DATA_RELATIVE "Data/")
    if(UNIX)
        set_target_properties(${PROJECT_NAME} PROPERTIES BUILD_RPATH "$ORIGIN")
    endif()
endif()

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/Source/Data" "${ADH_COPY_DATA}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/Source/Api/Vulkan/Shaders" "${ADH_COPY_DATA}/Resources/Shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ADH_CORE_SRC}/Scripting/Scripts" "${ADH_COPY_DATA}/Assets/Scripts"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ADH_CORE_SRC}/Scripting/Scripts" "${ADH_COPY_DATA}/Resources/Scripts"
    VERBATIM)
add_dependencies(${PROJECT_NAME} copy_assets)

if(WIN32)
    set(ADH_OPENAL_RUNTIME_NAME "$<TARGET_FILE_NAME:OpenAL>")
else()
    set(ADH_OPENAL_RUNTIME_NAME "$<TARGET_SONAME_FILE_NAME:OpenAL>")
endif()
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:OpenAL>" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${ADH_OPENAL_RUNTIME_NAME}"
    VERBATIM)
add_custom_target(run
    COMMAND "$<TARGET_FILE:${PROJECT_NAME}>"
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    DEPENDS ${PROJECT_NAME}
    USES_TERMINAL VERBATIM)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:ADH_DEBUG;_DEBUG>"
    "$<$<NOT:$<CONFIG:Debug>>:ADH_RELEASE>")
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS UNICODE
        "ADH_API=__declspec(dllexport)" ADH_WINDOWS)
elseif(APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE "ADH_API=" ADH_APPLE)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE "ADH_API=" ADH_LINUX)
endif()

option(INSTALL "Install" OFF)
if(INSTALL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE "DATA_DIRECTORY=\"${ADH_COPY_DATA}/\"")
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE "DATA_DIRECTORY=\"${ADH_DATA_RELATIVE}\"")
endif()

#**********************************************
# Add source
#**********************************************
target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Source/Main.cpp
    ${CMAKE_SOURCE_DIR}/Source/Window.hpp
    ${CMAKE_SOURCE_DIR}/Source/Window.cpp
    ${CMAKE_SOURCE_DIR}/Source/Utility.hpp
    ${CMAKE_SOURCE_DIR}/Source/Utf8.hpp
    ${CMAKE_SOURCE_DIR}/Source/Vertex.hpp)

if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE Source/Platform/Windows/Window.cpp Source/Utility.cpp)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_WIN32_KHR)
    target_link_libraries(${PROJECT_NAME} PRIVATE user32 shell32 gdi32 xinput)
elseif(APPLE)
    enable_language(OBJCXX)
    set_target_properties(${PROJECT_NAME} PROPERTIES OBJCXX_STANDARD 23 OBJCXX_STANDARD_REQUIRED YES)
    target_sources(${PROJECT_NAME} PRIVATE Source/Platform/macOS/Window.mm Source/Utility.mm)
    target_compile_options(${PROJECT_NAME} PRIVATE "$<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc>")
    target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_MACOS_MVK)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa" "-framework QuartzCore" "-framework GameController")
else()
    target_sources(${PROJECT_NAME} PRIVATE Source/Platform/Linux/Window.cpp Source/Utility.cpp)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_XCB_KHR)
    target_link_libraries(${PROJECT_NAME} PRIVATE X11 xcb X11-xcb)
endif()

#**********************************************
# Graphics api
#**********************************************
find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})
target_compile_definitions(${PROJECT_NAME} PRIVATE "ADH_VULKAN= ")

set(VULKAN_API_SRC ${CMAKE_SOURCE_DIR}/Source/Api/Vulkan)
target_sources(${PROJECT_NAME} PRIVATE
    ${VULKAN_API_SRC}/Initializers.hpp
    ${VULKAN_API_SRC}/Tools.hpp
    ${VULKAN_API_SRC}/Instance.hpp
    ${VULKAN_API_SRC}/Instance.cpp
    ${VULKAN_API_SRC}/PhysicalDevice.hpp
    ${VULKAN_API_SRC}/PhysicalDevice.cpp
    ${VULKAN_API_SRC}/Surface.hpp
    ${VULKAN_API_SRC}/Surface.cpp
    ${VULKAN_API_SRC}/Device.hpp
    ${VULKAN_API_SRC}/Device.cpp
    ${VULKAN_API_SRC}/DeviceQueues.hpp
    ${VULKAN_API_SRC}/DeviceQueues.cpp
    ${VULKAN_API_SRC}/Allocator.hpp
    ${VULKAN_API_SRC}/Allocator.cpp
    ${VULKAN_API_SRC}/Context.hpp
    ${VULKAN_API_SRC}/Context.cpp
    ${VULKAN_API_SRC}/Swapchain.hpp
    ${VULKAN_API_SRC}/Swapchain.cpp
    ${VULKAN_API_SRC}/Buffer.hpp
    ${VULKAN_API_SRC}/Buffer.cpp
    ${VULKAN_API_SRC}/Image.hpp
    ${VULKAN_API_SRC}/Image.cpp
    ${VULKAN_API_SRC}/ImageView.hpp
    ${VULKAN_API_SRC}/ImageView.cpp
    ${VULKAN_API_SRC}/Framebuffer.hpp
    ${VULKAN_API_SRC}/Framebuffer.cpp
    ${VULKAN_API_SRC}/Shader.hpp
    ${VULKAN_API_SRC}/Shader.cpp
    ${VULKAN_API_SRC}/Shader.inl
    ${VULKAN_API_SRC}/VertexLayout.hpp
    ${VULKAN_API_SRC}/VertexLayout.cpp
    ${VULKAN_API_SRC}/PipelineLayout.hpp
    ${VULKAN_API_SRC}/PipelineLayout.cpp
    ${VULKAN_API_SRC}/DescriptorSet.hpp
    ${VULKAN_API_SRC}/DescriptorSet.cpp
    ${VULKAN_API_SRC}/VertexBuffer.hpp
    ${VULKAN_API_SRC}/VertexBuffer.cpp
    ${VULKAN_API_SRC}/UniformBuffer.hpp
    ${VULKAN_API_SRC}/UniformBuffer.cpp
    ${VULKAN_API_SRC}/IndexBuffer.hpp
    ${VULKAN_API_SRC}/IndexBuffer.cpp
    ${VULKAN_API_SRC}/Attachments.hpp
    ${VULKAN_API_SRC}/Attachments.cpp
    ${VULKAN_API_SRC}/ComputePipeline.hpp
    ${VULKAN_API_SRC}/ComputePipeline.cpp
    ${VULKAN_API_SRC}/GraphicsPipeline.hpp
    ${VULKAN_API_SRC}/GraphicsPipeline.cpp
    ${VULKAN_API_SRC}/Subpass.hpp
    ${VULKAN_API_SRC}/Subpass.cpp
    ${VULKAN_API_SRC}/RenderPass.hpp
    ${VULKAN_API_SRC}/RenderPass.cpp
    ${VULKAN_API_SRC}/Sampler.hpp
    ${VULKAN_API_SRC}/Sampler.cpp
    ${VULKAN_API_SRC}/Texture2D.hpp
    ${VULKAN_API_SRC}/Texture2D.cpp
    ${VULKAN_API_SRC}/TextureCube.hpp
    ${VULKAN_API_SRC}/TextureCube.cpp
    ${VULKAN_API_SRC}/CommandPool.hpp
    ${VULKAN_API_SRC}/CommandPool.cpp
    ${VULKAN_API_SRC}/CommandBuffer.hpp
    ${VULKAN_API_SRC}/CommandBuffer.cpp
    ${VULKAN_API_SRC}/Viewport.hpp
    ${VULKAN_API_SRC}/Viewport.cpp
    ${VULKAN_API_SRC}/Scissor.hpp
    ${VULKAN_API_SRC}/Scissor.cpp
    ${VULKAN_API_SRC}/Memory.hpp)

#**********************************************
#**********************************************
target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Source/Renderer/Bloom.hpp
    ${CMAKE_SOURCE_DIR}/Source/Renderer/Bloom.cpp
    ${CMAKE_SOURCE_DIR}/Source/Renderer/EntityPicker.hpp
    ${CMAKE_SOURCE_DIR}/Source/Renderer/EntityPicker.cpp
    ${CMAKE_SOURCE_DIR}/Source/Renderer/FullscreenPass.hpp
    ${CMAKE_SOURCE_DIR}/Source/Renderer/FullscreenPass.cpp
    ${CMAKE_SOURCE_DIR}/Source/Renderer/LightBuffer.hpp
    ${CMAKE_SOURCE_DIR}/Source/Renderer/LightBuffer.cpp
    ${CMAKE_SOURCE_DIR}/Source/Renderer/ParticleRenderer.hpp
    ${CMAKE_SOURCE_DIR}/Source/Renderer/ParticleRenderer.cpp
    ${CMAKE_SOURCE_DIR}/Source/Renderer/RenderSettings.hpp
    ${CMAKE_SOURCE_DIR}/Source/Renderer/SceneRenderer.hpp
    ${CMAKE_SOURCE_DIR}/Source/Renderer/SceneRenderer.cpp
    ${CMAKE_SOURCE_DIR}/Source/Renderer/SkyboxRenderer.hpp
    ${CMAKE_SOURCE_DIR}/Source/Renderer/SkyboxRenderer.cpp
    ${CMAKE_SOURCE_DIR}/Source/Renderer/SunShadows.hpp
    ${CMAKE_SOURCE_DIR}/Source/Renderer/SunShadows.cpp)

#**********************************************
#**********************************************
target_sources(${PROJECT_NAME} PRIVATE
    ${ADH_CORE_SRC}/Event/Event.hpp
    ${ADH_CORE_SRC}/Event/Event.cpp
    ${ADH_CORE_SRC}/Event/EventTypes.hpp)

#**********************************************
# Physics library
#**********************************************
target_sources(${PROJECT_NAME} PRIVATE
    ${ADH_CORE_SRC}/Physics/PhysicsWorld.hpp
    ${ADH_CORE_SRC}/Physics/PhysicsWorld.cpp)

#**********************************************
# Math library
#**********************************************
target_sources(${PROJECT_NAME} PRIVATE
    ${ADH_CORE_SRC}/Math/Math.hpp)

#**********************************************
# Scene
#**********************************************
target_sources(${PROJECT_NAME} PRIVATE
    ${ADH_CORE_SRC}/Scene/Scene.hpp
    ${ADH_CORE_SRC}/Scene/Scene.cpp
    ${ADH_CORE_SRC}/Scene/ParticleSystem.hpp
    ${ADH_CORE_SRC}/Scene/ParticleSystem.cpp
    ${ADH_CORE_SRC}/Scene/Serializer.hpp
    ${ADH_CORE_SRC}/Scene/Serializer.cpp
    ${ADH_CORE_SRC}/Scene/ComponentsSerializer.hpp
    ${ADH_CORE_SRC}/Scene/Components.hpp
    ${ADH_CORE_SRC}/Scene/Components/Camera.hpp
    ${ADH_CORE_SRC}/Scene/Components/RigidBody.hpp
    ${ADH_CORE_SRC}/Scene/Components/RigidBody.cpp
    ${ADH_CORE_SRC}/Scene/Components/Light.hpp
    ${ADH_CORE_SRC}/Scene/Components/Material.hpp
    ${ADH_CORE_SRC}/Scene/Components/MaterialTextures.hpp
    ${ADH_CORE_SRC}/Scene/Components/MaterialTextures.cpp
    ${ADH_CORE_SRC}/Scene/Components/Mesh.hpp
    ${ADH_CORE_SRC}/Scene/Components/Mesh.cpp
    ${ADH_CORE_SRC}/Scene/Components/ParticleEmitter.hpp
    ${ADH_CORE_SRC}/Scene/Components/Skybox.hpp
    ${ADH_CORE_SRC}/Scene/Components/Tag.hpp
    ${ADH_CORE_SRC}/Scene/Components/Transform.hpp)

#**********************************************
# Scripting
#**********************************************
target_sources(${PROJECT_NAME} PRIVATE
    ${ADH_CORE_SRC}/Scripting/Script.hpp
    ${ADH_CORE_SRC}/Scripting/ScriptHandler.hpp
    ${ADH_CORE_SRC}/Scripting/ScriptHandler.cpp)

#**********************************************
# Audio
#**********************************************
target_sources(${PROJECT_NAME} PRIVATE
    ${ADH_CORE_SRC}/Audio/LoadWav.hpp
    ${ADH_CORE_SRC}/Audio/Audio.hpp
    ${ADH_CORE_SRC}/Audio/Audio.cpp)

#**********************************************
# Input
#**********************************************
target_sources(${PROJECT_NAME} PRIVATE
    ${ADH_CORE_SRC}/Input/Input.hpp
    ${ADH_CORE_SRC}/Input/Input.cpp
    ${ADH_CORE_SRC}/Input/Controller.hpp
    ${ADH_CORE_SRC}/Input/Controller.cpp
    ${ADH_CORE_SRC}/Input/Mouse.hpp
    ${ADH_CORE_SRC}/Input/Mouse.cpp
    ${ADH_CORE_SRC}/Input/Keyboard.hpp
    ${ADH_CORE_SRC}/Input/Keyboard.cpp
    ${ADH_CORE_SRC}/Input/Keycodes.hpp
    ${ADH_CORE_SRC}/Input/AppleKeycodes.hpp)

#**********************************************
# Editor
#**********************************************
set(ADH_EDITOR_SRC ${CMAKE_SOURCE_DIR}/Source/Editor)
target_sources(${PROJECT_NAME} PRIVATE
    ${ADH_EDITOR_SRC}/EditorInput.hpp
    ${ADH_EDITOR_SRC}/EditorInput.cpp
    ${ADH_EDITOR_SRC}/Editor.hpp
    ${ADH_EDITOR_SRC}/Editor.cpp
    ${ADH_EDITOR_SRC}/Api/Vulkan/VulkanImGui.hpp
    ${ADH_EDITOR_SRC}/Api/Vulkan/VulkanImGui.cpp
    ${ADH_EDITOR_SRC}/IconFontCppHeaders/IconFontAwesome5.hpp
    ${ADH_EDITOR_SRC}/EditorContext.hpp
    ${ADH_EDITOR_SRC}/EditorActions.hpp
    ${ADH_EDITOR_SRC}/EditorActions.cpp
    ${ADH_EDITOR_SRC}/EditorCamera.hpp
    ${ADH_EDITOR_SRC}/EditorCamera.cpp
    ${ADH_EDITOR_SRC}/DragDrop.hpp
    ${ADH_EDITOR_SRC}/DragDrop.cpp
    ${ADH_EDITOR_SRC}/Shell.hpp
    ${ADH_EDITOR_SRC}/Shell.cpp
    ${ADH_EDITOR_SRC}/UndoHistory.hpp
    ${ADH_EDITOR_SRC}/UndoHistory.cpp
    ${ADH_EDITOR_SRC}/Theme.hpp
    ${ADH_EDITOR_SRC}/Theme.cpp
    ${ADH_EDITOR_SRC}/Widgets.hpp
    ${ADH_EDITOR_SRC}/Widgets.cpp
    ${ADH_EDITOR_SRC}/Panels/Panel.hpp
    ${ADH_EDITOR_SRC}/Panels/Panel.cpp
    ${ADH_EDITOR_SRC}/Panels/MainMenuBar.hpp
    ${ADH_EDITOR_SRC}/Panels/MainMenuBar.cpp
    ${ADH_EDITOR_SRC}/Panels/Toolbar.hpp
    ${ADH_EDITOR_SRC}/Panels/Toolbar.cpp
    ${ADH_EDITOR_SRC}/Panels/SaveChangesDialog.hpp
    ${ADH_EDITOR_SRC}/Panels/SaveChangesDialog.cpp
    ${ADH_EDITOR_SRC}/Panels/AssetPanel.hpp
    ${ADH_EDITOR_SRC}/Panels/AssetPanel.cpp
    ${ADH_EDITOR_SRC}/Panels/ConsolePanel.hpp
    ${ADH_EDITOR_SRC}/Panels/ConsolePanel.cpp
    ${ADH_EDITOR_SRC}/Panels/GamePanel.hpp
    ${ADH_EDITOR_SRC}/Panels/GamePanel.cpp
    ${ADH_EDITOR_SRC}/Panels/InspectorPanel.hpp
    ${ADH_EDITOR_SRC}/Panels/InspectorPanel.cpp
    ${ADH_EDITOR_SRC}/Panels/ComponentEditors.hpp
    ${ADH_EDITOR_SRC}/Panels/ComponentEditors.cpp
    ${ADH_EDITOR_SRC}/Panels/SceneHierarchyPanel.hpp
    ${ADH_EDITOR_SRC}/Panels/SceneHierarchyPanel.cpp
    ${ADH_EDITOR_SRC}/Panels/ScenePanel.hpp
    ${ADH_EDITOR_SRC}/Panels/ScenePanel.cpp
    ${ADH_EDITOR_SRC}/Panels/SettingsPanel.hpp
    ${ADH_EDITOR_SRC}/Panels/SettingsPanel.cpp
    ${ADH_EDITOR_SRC}/Panels/ViewportPanel.hpp
    ${ADH_EDITOR_SRC}/Panels/ViewportPanel.cpp
    ${ADH_EDITOR_SRC}/Panels/ViewportRect.hpp)

option(ADH_BUILD_TESTS "Build AdHoc regression tests" OFF)
if(ADH_BUILD_TESTS)
    enable_testing()
    add_subdirectory(Tests)
endif()
