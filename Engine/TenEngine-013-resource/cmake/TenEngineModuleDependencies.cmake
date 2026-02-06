# TenEngine 013-Resource worktree: module dependency (for reference / TenEngineHelpers).
# Matches specs/_contracts/000-module-dependency-map.md.
set(TENENGINE_013_RESOURCE_DEPS "001-core" "002-object" "028-texture")

# Map module ID -> CMake target name
function(tenengine_module_id_to_target mod_id out_var)
  string(REGEX REPLACE "^[0-9]+-" "" name_part "${mod_id}")
  string(REPLACE "-" "_" target_suffix "${name_part}")
  set(${out_var} "te_${target_suffix}" PARENT_SCOPE)
endfunction()

function(tenengine_get_deps mod_id out_list_var)
  string(REPLACE "-" "_" u "${mod_id}")
  string(TOUPPER "${u}" U)
  set(var_name "TENENGINE_${U}_DEPS")
  if(DEFINED ${var_name})
    set(${out_list_var} ${${var_name}} PARENT_SCOPE)
  else()
    set(${out_list_var} "" PARENT_SCOPE)
  endif()
endfunction()
