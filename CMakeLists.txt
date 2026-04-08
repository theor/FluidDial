cmake_minimum_required(VERSION 3.20)
project(FluidDialSDL CXX C)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# User config
set(DISPLAY_WIDTH  240 CACHE STRING "Display width")
set(DISPLAY_HEIGHT 320 CACHE STRING "Display height")
option(STUB_SERIAL "Stub serial (no hardware)" OFF)

add_definitions(-DLGFX_SDL)

# Python for git-version.py
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Version generation: git-version.py writes src/version.cxx -> src/version.cpp
set(VERSION_CPP "${CMAKE_SOURCE_DIR}/src/version.cpp")
add_custom_command(
    OUTPUT  "${VERSION_CPP}"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/git-version.py"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Generating src/version.cpp"
    DEPENDS "${CMAKE_SOURCE_DIR}/git-version.py"
)
add_custom_target(gen_version DEPENDS "${VERSION_CPP}")

# FetchContent dependencies
include(FetchContent)
FetchContent_Declare(LovyanGFX
    GIT_REPOSITORY https://github.com/lovyan03/LovyanGFX.git
    GIT_TAG develop
    GIT_SHALLOW TRUE
)
FetchContent_Declare(JsonStreamingParser
    GIT_REPOSITORY https://github.com/MitchBradley/json-streaming-parser.git
    GIT_TAG charp-1.0.2
    GIT_SHALLOW TRUE
)
FetchContent_Declare(GrblParser
    GIT_REPOSITORY https://github.com/MitchBradley/GrblParser.git
    GIT_TAG 9108f54
    GIT_SHALLOW FALSE
)
FetchContent_MakeAvailable(LovyanGFX JsonStreamingParser GrblParser)

# SDL2: try cmake/FindSDL2.cmake (finds MSYS2/MinGW installs) first,
# then fall back to building SDL2 from source via FetchContent.
# SDL2_LINK_TARGET holds the CMake target name to link against.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(SDL2)
if (SDL2_FOUND)
    message(STATUS "Packaged version of SDL2 will be used.")
    set(SDL2_LINK_TARGET SDL2::SDL2)
else()
    message(STATUS "Bundled version of SDL2 will be downloaded and built (static).")
    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY  https://github.com/libsdl-org/SDL.git
        GIT_TAG         5773c347d58d07a45b33ac4ecc93fd3e6a86d483
    )
    FetchContent_MakeAvailable(SDL2)
    # Static target avoids a runtime SDL2.dll dependency.
    set(SDL2_LINK_TARGET SDL2-static)
endif()

# LovyanGFX: SDL platform files plus the core implementation files that
# LovyanGFX expects the application to compile (they are not header-only).
add_library(lovyangfx_sdl STATIC
    "${lovyangfx_SOURCE_DIR}/src/lgfx/v1/platforms/sdl/Panel_sdl.cpp"
    "${lovyangfx_SOURCE_DIR}/src/lgfx/v1/platforms/sdl/common.cpp"
    "${lovyangfx_SOURCE_DIR}/src/lgfx/v1/LGFXBase.cpp"
    "${lovyangfx_SOURCE_DIR}/src/lgfx/v1/LGFX_Sprite.cpp"
    "${lovyangfx_SOURCE_DIR}/src/lgfx/v1/lgfx_fonts.cpp"
    "${lovyangfx_SOURCE_DIR}/src/lgfx/v1/misc/pixelcopy.cpp"
    "${lovyangfx_SOURCE_DIR}/src/lgfx/v1/misc/SpriteBuffer.cpp"
    "${lovyangfx_SOURCE_DIR}/src/lgfx/v1/misc/common_function.cpp"
    "${lovyangfx_SOURCE_DIR}/src/lgfx/v1/panel/Panel_Device.cpp"
    "${lovyangfx_SOURCE_DIR}/src/lgfx/v1/panel/Panel_FrameBufferBase.cpp"
    "${lovyangfx_SOURCE_DIR}/src/lgfx/utility/lgfx_pngle.c"
    "${lovyangfx_SOURCE_DIR}/src/lgfx/utility/lgfx_qoi.c"
    "${lovyangfx_SOURCE_DIR}/src/lgfx/utility/lgfx_qrcode.c"
    "${lovyangfx_SOURCE_DIR}/src/lgfx/utility/lgfx_tjpgd.c"
    "${lovyangfx_SOURCE_DIR}/src/lgfx/utility/lgfx_miniz.c"
    "${lovyangfx_SOURCE_DIR}/src/lgfx/Fonts/IPA/lgfx_font_japan.c"
    "${lovyangfx_SOURCE_DIR}/src/lgfx/Fonts/efont/lgfx_efont_cn.c"
    "${lovyangfx_SOURCE_DIR}/src/lgfx/Fonts/efont/lgfx_efont_ja.c"
    "${lovyangfx_SOURCE_DIR}/src/lgfx/Fonts/efont/lgfx_efont_kr.c"
    "${lovyangfx_SOURCE_DIR}/src/lgfx/Fonts/efont/lgfx_efont_tw.c"
)
target_include_directories(lovyangfx_sdl PUBLIC "${lovyangfx_SOURCE_DIR}/src")
target_compile_definitions(lovyangfx_sdl PUBLIC LGFX_USE_V1)
target_link_libraries(lovyangfx_sdl PUBLIC ${SDL2_LINK_TARGET})

# json-streaming-parser (sources live in the repo root, not src/)
add_library(json_parser STATIC
    "${jsonstreamingparser_SOURCE_DIR}/JsonStreamingParser.cpp"
)
target_include_directories(json_parser PUBLIC "${jsonstreamingparser_SOURCE_DIR}")
target_compile_definitions(json_parser PUBLIC JSP_USE_CHARP)

# GrblParser
add_library(grbl_parser STATIC
    "${grblparser_SOURCE_DIR}/src/GrblParserC.c"
    "${grblparser_SOURCE_DIR}/src/alarm.c"
    "${grblparser_SOURCE_DIR}/src/e4math.c"
)
target_include_directories(grbl_parser PUBLIC "${grblparser_SOURCE_DIR}/src")
target_compile_definitions(grbl_parser PUBLIC E4_POS_T)

# Force sdlmain.c to compile as C++ (it #includes a .hpp)
set_source_files_properties("${CMAKE_SOURCE_DIR}/src/sdlmain.c" PROPERTIES LANGUAGE CXX)

set(SRC "${CMAKE_SOURCE_DIR}/src")
set(FLUIDDIAL_SOURCES
    "${SRC}/ardmain.cpp"
    "${SRC}/sdlmain.c"
    "${SRC}/SystemWindowsLGFX.cpp"
    "${SRC}/Touch_Class.cpp"
    "${SRC}/System.cpp"
    "${SRC}/AboutScene.cpp"
    "${SRC}/ConfigItem.cpp"
    "${SRC}/ConfirmScene.cpp"
    "${SRC}/Drawing.cpp"
    "${SRC}/FileMenu.cpp"
    "${SRC}/FileParser.cpp"
    "${SRC}/FileParser2.cpp"
    "${SRC}/FilePreviewScene.cpp"
    "${SRC}/FileSelectScene.cpp"
    "${SRC}/FluidNCModel.cpp"
    "${SRC}/HelpScene.cpp"
    "${SRC}/HomingScene.cpp"
    "${SRC}/MacroMenu.cpp"
    "${SRC}/Menu.cpp"
    "${SRC}/MenuScene.cpp"
    "${SRC}/MultiJogScene.cpp"
    "${SRC}/PieMenu.cpp"
    "${SRC}/Point.cpp"
    "${SRC}/ProbingScene.cpp"
    "${SRC}/Scene.cpp"
    "${SRC}/StatusScene.cpp"
    "${SRC}/Text.cpp"
    "${SRC}/ToolChangeScene.cpp"
    "${SRC}/polar.c"
    "${VERSION_CPP}"
)

add_executable(fluiddial_sdl ${FLUIDDIAL_SOURCES})
add_dependencies(fluiddial_sdl gen_version)

target_include_directories(fluiddial_sdl PRIVATE "${SRC}")
target_compile_definitions(fluiddial_sdl PRIVATE
    USE_LOVYANGFX
    WINDOWS
    LGFX_USE_V1
    JSP_USE_CHARP
    E4_POS_T
    VERBATIM_GCODE_MODES
    DISPLAY_WIDTH=${DISPLAY_WIDTH}
    DISPLAY_HEIGHT=${DISPLAY_HEIGHT}
)
if(STUB_SERIAL)
    target_compile_definitions(fluiddial_sdl PRIVATE STUB_SERIAL)
endif()
target_link_libraries(fluiddial_sdl PRIVATE lovyangfx_sdl json_parser grbl_parser ${SDL2_LINK_TARGET})
if(WIN32)
    target_link_libraries(fluiddial_sdl PRIVATE comctl32)
    set_target_properties(fluiddial_sdl PROPERTIES WIN32_EXECUTABLE FALSE)
    add_custom_command(TARGET fluiddial_sdl POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/data"
            "$<TARGET_FILE_DIR:fluiddial_sdl>/data"
        COMMENT "Copying data/ to build directory"
    )
endif()
