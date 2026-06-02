if(NOT DEFINED DATA_TARGET)
    set(DATA_TARGET Data)
endif()
string(TOLOWER ${DATA_TARGET} DATA_TARGET_TOLOWER)

project(${DATA_TARGET})
add_custom_target(${DATA_TARGET})

set(SKINS
    skins/ti82.skn
    skins/ti83pfr.skn
    skins/ti83.skn
    skins/ti84p.skn
    skins/ti76.skn
    skins/ti82stats.skn
    skins/ti83pse.skn
    skins/ti84p2.skn
    skins/ti85.skn
    skins/ti81.skn
    skins/ti83pfr2.skn
    skins/ti83p.skn
    skins/ti84pse.skn
    skins/ti86.skn)

set("${DATA_TARGET}_FILES" ${SKINS} keybindings.ini)
set("${DATA_TARGET}_FILES" ${${DATA_TARGET}_FILES} PARENT_SCOPE)

qtcreator_add_project_resources(${${DATA_TARGET}_FILES})

install(FILES keybindings.ini DESTINATION ${CMAKE_INSTALL_DATADIR})
install(FILES ${SKINS} DESTINATION ${CMAKE_INSTALL_DATADIR}/skins)
