
project(notepadgame
        DESCRIPTION ""
        VERSION 0.0.1
        )

# building scintilla
# patch scintilla source code
set(NPGAME_SCI_PATCH_FILE "${CMAKE_SOURCE_DIR}/extern/patch/scintilla/src/Editor.cxx" )
set(NPGAME_SCI_ORIG_FILE "${NPGAME_SCINTILLA_DIR}/src/Editor.cxx" )
set(NPGAME_SCI_TEMP_FILE "${CMAKE_SOURCE_DIR}/extern/patch/scintilla/src/Editor_temp.cxx")
add_custom_target(scintilla_patch
        COMMAND ${CMAKE_COMMAND} -E copy
                 "${NPGAME_SCI_ORIG_FILE}" "${NPGAME_SCI_TEMP_FILE}" 
        COMMAND ${CMAKE_COMMAND} -E copy
                 "${NPGAME_SCI_PATCH_FILE}" "${NPGAME_SCI_ORIG_FILE}"       
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# find make
find_program(NPGAME_MAKE_EXISTS make mingw32-make)
if(NPGAME_MAKE_EXISTS)
        set(NPGAME_SCIBUILD_COMMAND make)
else()
        find_program(NPGAME_NMAKE_EXISTS  nmake)
        if(NPGAME_NMAKE_EXISTS )
                set(NPGAME_SCIBUILD_COMMAND nmake -f scintilla.mak)
       endif()
endif()

# run the scintilla makefile
add_custom_target(scintilla ALL COMMAND ${NPGAME_SCIBUILD_COMMAND}
                   WORKING_DIRECTORY  "${NPGAME_SCINTILLA_DIR}/win32"
                   DEPENDS scintilla_patch
                   COMMENT "Original scintilla makefile target"
)
# revert patch
add_custom_command(
        TARGET scintilla POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                "${NPGAME_SCI_TEMP_FILE}" "${NPGAME_SCI_ORIG_FILE}"
        COMMAND ${CMAKE_COMMAND} -E remove "${NPGAME_SCI_TEMP_FILE}")




# build notepadgame.dll

find_package(range-v3 CONFIG REQUIRED)
find_package(Eigen3   CONFIG REQUIRED)
find_package(EnTT     CONFIG REQUIRED)
find_package(Boost COMPONENTS container) # and signals2
find_package(utf8cpp  CONFIG REQUIRED)

include("${CMAKE_CURRENT_SOURCE_DIR}/engine/CMakeLists.txt")
include("${CMAKE_CURRENT_SOURCE_DIR}/game/CMakeLists.txt")



add_library(${PROJECT_NAME} SHARED ./dllmain.cpp ${ENGINESRCS} ${GAMESRCS})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/libs/easing/")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/engine")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/game")
target_include_directories(${PROJECT_NAME} PUBLIC "${NPGAME_SCINTILLA_DIR}/include/")

target_link_libraries  (${PROJECT_NAME} PUBLIC  
                                                Boost::container
                                                utf8::cpp
                                                range-v3::range-v3
                                                Eigen3::Eigen
                                                EnTT::EnTT)

target_compile_definitions(${PROJECT_NAME} PUBLIC
                                                NOMINMAX            # windows.h ...
                                                WIN32_LEAN_AND_MEAN # windows.h ...
                                        )

install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Debug
        RUNTIME DESTINATION ./)

install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Release
        RUNTIME DESTINATION ./)

install(FILES "${NPGAME_SCINTILLA_DIR}/bin/Scintilla.dll"
        CONFIGURATIONS Debug
        DESTINATION ./)

install(FILES "${NPGAME_SCINTILLA_DIR}/bin/Scintilla.dll"
        CONFIGURATIONS Release
        DESTINATION ./)