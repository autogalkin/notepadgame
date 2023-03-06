
project(notepadgame
        DESCRIPTION ""
        VERSION 0.0.1
        )

# building scintilla
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
                   COMMENT "Original scintilla makefile target"
)

find_package(range-v3 CONFIG REQUIRED)
find_package(Eigen3   CONFIG REQUIRED)
find_package(utf8cpp  CONFIG REQUIRED)
find_package(EnTT     CONFIG REQUIRED)
find_package(Boost)

file(GLOB_RECURSE SRCS *.cpp)
add_library(${PROJECT_NAME} SHARED ${SRCS})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries  (${PROJECT_NAME} PRIVATE  
                                                utf8::cpp
                                                range-v3::range-v3
                                                Eigen3::Eigen
                                                EnTT::EnTT)

target_include_directories(${PROJECT_NAME} PRIVATE "${NPGAME_SCINTILLA_DIR}/include")


install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Debug
        RUNTIME DESTINATION Debug/)

install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Release
        RUNTIME DESTINATION Release/)

install(FILES "${NPGAME_SCINTILLA_DIR}/bin/Scintilla.dll"
        CONFIGURATIONS Debug
        DESTINATION Debug/)

install(FILES "${NPGAME_SCINTILLA_DIR}/bin/Scintilla.dll"
        CONFIGURATIONS Release
        DESTINATION Release/)