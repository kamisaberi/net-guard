#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "NetGuard::net_guard" for configuration "Release"
set_property(TARGET NetGuard::net_guard APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(NetGuard::net_guard PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libnet_guard.so"
  IMPORTED_SONAME_RELEASE "libnet_guard.so"
  )

list(APPEND _cmake_import_check_targets NetGuard::net_guard )
list(APPEND _cmake_import_check_files_for_NetGuard::net_guard "${_IMPORT_PREFIX}/lib/libnet_guard.so" )

# Import target "NetGuard::net_guard_static" for configuration "Release"
set_property(TARGET NetGuard::net_guard_static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(NetGuard::net_guard_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libnet_guard.a"
  )

list(APPEND _cmake_import_check_targets NetGuard::net_guard_static )
list(APPEND _cmake_import_check_files_for_NetGuard::net_guard_static "${_IMPORT_PREFIX}/lib/libnet_guard.a" )

# Import target "NetGuard::net-guard-ctl" for configuration "Release"
set_property(TARGET NetGuard::net-guard-ctl APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(NetGuard::net-guard-ctl PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/net-guard-ctl"
  )

list(APPEND _cmake_import_check_targets NetGuard::net-guard-ctl )
list(APPEND _cmake_import_check_files_for_NetGuard::net-guard-ctl "${_IMPORT_PREFIX}/bin/net-guard-ctl" )

# Import target "NetGuard::aryo-net" for configuration "Release"
set_property(TARGET NetGuard::aryo-net APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(NetGuard::aryo-net PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/aryo-net"
  )

list(APPEND _cmake_import_check_targets NetGuard::aryo-net )
list(APPEND _cmake_import_check_files_for_NetGuard::aryo-net "${_IMPORT_PREFIX}/bin/aryo-net" )

# Import target "NetGuard::net_guard_dissector_plugin" for configuration "Release"
set_property(TARGET NetGuard::net_guard_dissector_plugin APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(NetGuard::net_guard_dissector_plugin PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "NetGuard::net_guard"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/net_guard_dissector_plugin.so"
  IMPORTED_SONAME_RELEASE "net_guard_dissector_plugin.so"
  )

list(APPEND _cmake_import_check_targets NetGuard::net_guard_dissector_plugin )
list(APPEND _cmake_import_check_files_for_NetGuard::net_guard_dissector_plugin "${_IMPORT_PREFIX}/lib/net_guard_dissector_plugin.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
