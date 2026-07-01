if(NOT DEFINED TILEM_TARGET)
    set(TILEM_TARGET TilEm)
endif()
string(TOLOWER ${TILEM_TARGET} TILEM_PROJECT_TOLOWER)

project(${TILEM_TARGET})

find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)

if(NOT DEFINED TILEM_VERSION_MAJOR)
    set(TILEM_VERSION_MAJOR 1)
endif()
if(NOT DEFINED TILEM_VERSION_MINOR)
    set(TILEM_VERSION_MINOR 0)
endif()
if(NOT DEFINED TILEM_URI)
    set(TILEM_URI ${TILEM_TARGET})
endif()

option(QMLDUMP "Dump qml info so it's loadable by the QtCreator editor" ON)

qtcreator_add_project_resources(qmldir)

find_package(Glib REQUIRED)
find_package(GObject REQUIRED)
find_package(LIBC REQUIRED)
find_package(GDK-PixBuf REQUIRED)
find_package(TiCalcs2 REQUIRED)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/backend.h.in ${CMAKE_CURRENT_BINARY_DIR}/backend.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/backend.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/backend.cpp)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
if(NOT DEFINED EMU_TARGET)
    set(EMU_TARGET "Emu")
endif()
include_directories(${${EMU_TARGET}_SOURCE_DIR})

set(LIBS ${TiCalcs2_LIBRARIES} ${Glib_LIBRARIES} ${GObject_LIBRARIES} ${LIBC_LIBRARIES} ${GDK-PixBuf_LIBRARIES})
if(CLICK_MODE)
    deploy_libs(${LIBS})
endif(CLICK_MODE)

if(NOT DEFINED EMU_TARGET)
    set(EMU_TARGET Emu)
endif()

set(LIBS ${LIBS} ${EMU_TARGET} )

include_directories(
    ${CMAKE_BINARY_DIR}
    ${Glib_INCLUDE_DIRS}
    ${GObject_INCLUDE_DIRS}
    ${GDK-PixBuf_INCLUDE_DIRS}
    ${TiCalcs2_INCLUDE_DIRS}
    ${emu_SOURCE_DIR}
)

set(tilem_HDRS
    ${CMAKE_CURRENT_BINARY_DIR}/backend.h
    ${CMAKE_CURRENT_SOURCE_DIR}/calcscreen.h
    ${CMAKE_CURRENT_SOURCE_DIR}/linkbuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/calc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/calclink.h
    ${CMAKE_CURRENT_SOURCE_DIR}/calcthread.h
    ${CMAKE_CURRENT_SOURCE_DIR}/settings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/skin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/skinimage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/skinops.h)

set(tilem_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/backend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/calcscreen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/calcthread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/calc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/calclink.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/settings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/skin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/skinimage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/skinops.c)

add_library(${TILEM_TARGET} SHARED
    ${tilem_MOC_SRCS}
    ${tilem_SRCS}
)

qt5_use_modules(${TILEM_TARGET} Gui Core Qml Quick)

target_link_libraries(${TILEM_TARGET}
    ${LIBS}
)

set(TILEM_LIB_NAME $<TARGET_FILE:${TILEM_TARGET}>)

set("${TILEM_TARGET}_FILES" ${TILEM_LIB_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/qmldir)
set("${TILEM_TARGET}_FILES" ${${TILEM_TARGET}_FILES} PARENT_SCOPE)

set(TILEM_PLUGIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/${TILEM_TARGET})
if(QMLDUMP)
add_custom_command(TARGET ${TILEM_TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${TILEM_LIB_NAME} ${TILEM_PLUGIN_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy qmldir ${TILEM_PLUGIN_DIR}
    COMMAND ${CMAKE_COMMAND} -E echo "Run qmlplugindump"
    COMMAND qmlplugindump -notrelocatable ${TILEM_TARGET} 1.0 ${CMAKE_CURRENT_BINARY_DIR} > ${TILEM_PLUGIN_DIR}/${TILEM_PROJECT_TOLOWER}.qmltypes)
endif(QMLDUMP)

install(TARGETS ${TILEM_TARGET}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/${TILEM_TARGET} )
install(FILES "qmldir"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/${TILEM_TARGET})
