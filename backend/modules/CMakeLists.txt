add_subdirectory(TilEm)
add_subdirectory(Utils)

set("${TILEM_TARGET}_FILES" ${${TILEM_TARGET}_FILES} PARENT_SCOPE)
set("${UTILS_TARGET}_FILES" ${${UTILS_TARGET}_FILES} PARENT_SCOPE)
