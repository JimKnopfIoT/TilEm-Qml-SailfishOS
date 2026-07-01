if(NOT DEFINED UTILS_TARGET)
    set(UTILS_TARGET Utils)
endif()
string(TOLOWER ${UTILS_TARGET} UTILS_TARGET_TOLOWER)

project(${UTILS_TARGET})

include(ResolveLibs)

include(CLICKInstallDirs)
include(CLICKDeployLibs)

find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)

if(NOT DEFINED ${UTILS_VERSION_MAJOR})
    set(UTILS_VERSION_MAJOR 1)
endif()
if(NOT DEFINED ${UTILS_VERSION_MINOR})
    set(UTILS_VERSION_MINOR 0)
endif()
if(NOT DEFINED ${UTILS_URI})
    set(UTILS_URI ${UTILS_TARGET})
endif()

option(QMLDUMP "Dump qml info so it's loadable by the QtCreator editor" ON)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/backend.h.in ${CMAKE_CURRENT_BINARY_DIR}/backend.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/backend.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/backend.cpp)

#set(LIBS "")
#if(CLICK_MODE)
#    deploy_libs(${LIBS})
#endif(CLICK_MODE)

set(utils_HDRS
    ${CMAKE_CURRENT_BINARY_DIR}/backend.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qmlenv.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qmlfile.h
    )

set(utils_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/backend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qmlenv.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qmlfile.cpp
    )

add_library(${UTILS_TARGET} SHARED
    ${utils_MOC_SRCS}
    ${utils_SRCS}
)

qt5_use_modules(${UTILS_TARGET} Gui Core Qml Quick)

target_link_libraries(${UTILS_TARGET}
    ${LIBS}
)

set(UTILS_LIB_NAME $<TARGET_FILE:${UTILS_TARGET}>)

set("${UTILS_TARGET}_FILES" ${UTILS_LIB_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/qmldir)
set("${UTILS_TARGET}_FILES" ${${UTILS_TARGET}_FILES} PARENT_SCOPE)

set(UTILS_PLUGIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/${UTILS_TARGET})
if(QMLDUMP)
add_custom_command(TARGET ${UTILS_TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${UTILS_LIB_NAME} ${UTILS_PLUGIN_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy qmldir ${UTILS_PLUGIN_DIR}
    COMMAND ${CMAKE_COMMAND} -E echo "Run qmlplugindump"
    COMMAND qmlplugindump -notrelocatable ${UTILS_TARGET} 1.0 ${CMAKE_CURRENT_BINARY_DIR} > ${UTILS_PLUGIN_DIR}/${UTILS_PROJECT_TOLOWER}.qmltypes)
endif(QMLDUMP)

install(TARGETS ${UTILS_TARGET}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/${UTILS_TARGET} )
install(FILES "qmldir"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/${UTILS_TARGET} )
