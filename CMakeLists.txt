cmake_minimum_required(VERSION 3.20)
project(PhantomBotMusicPlayer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(DISCORD_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/discord_social_sdk")

add_executable(PhantomBotMusicPlayer
    src/main.cpp
)

target_include_directories(PhantomBotMusicPlayer PRIVATE
    "${DISCORD_SDK_DIR}/include"
)

target_compile_definitions(PhantomBotMusicPlayer PRIVATE
    QT_NO_KEYWORDS
)

if(WIN32)
    if(NOT MSVC)
        message(FATAL_ERROR "Windows builds should use Visual Studio/MSVC.")
    endif()

    target_link_directories(PhantomBotMusicPlayer PRIVATE
        "${DISCORD_SDK_DIR}/lib/release"
    )
    target_link_libraries(PhantomBotMusicPlayer PRIVATE
        Qt6::Widgets
        discord_partner_sdk.lib
    )

    add_custom_command(TARGET PhantomBotMusicPlayer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DISCORD_SDK_DIR}/bin/release/discord_partner_sdk.dll"
            "$<TARGET_FILE_DIR:PhantomBotMusicPlayer>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DISCORD_SDK_DIR}/bin/release/discord_krisp.dll"
            "$<TARGET_FILE_DIR:PhantomBotMusicPlayer>"
    )
else()
    target_link_directories(PhantomBotMusicPlayer PRIVATE
        "${DISCORD_SDK_DIR}/lib/release"
    )
    target_link_libraries(PhantomBotMusicPlayer PRIVATE
        Qt6::Widgets
        discord_partner_sdk
    )
    set_target_properties(PhantomBotMusicPlayer PROPERTIES
        BUILD_RPATH "${DISCORD_SDK_DIR}/lib/release"
        INSTALL_RPATH "$ORIGIN/../lib"
    )
endif()

install(TARGETS PhantomBotMusicPlayer RUNTIME DESTINATION bin)

if(WIN32)
    install(FILES
        "${DISCORD_SDK_DIR}/bin/release/discord_partner_sdk.dll"
        "${DISCORD_SDK_DIR}/bin/release/discord_krisp.dll"
        DESTINATION bin
    )
else()
    install(FILES
        "${DISCORD_SDK_DIR}/lib/release/libdiscord_partner_sdk.so"
        DESTINATION lib
    )
endif()
