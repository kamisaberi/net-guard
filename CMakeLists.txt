# File: /home/kami/net-guard/CMakeLists.txt

cmake_minimum_required(VERSION 3.20)
project(net_guard VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "/usr/local/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -O3
        -march=native
        -Wall
        -Wextra
        -Werror=return-type
        -fno-omit-frame-pointer
    )
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    /home/kami/blackbox-sentinel/include
    /usr/local/include
)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_library(BLACKBOX_LIB blackbox REQUIRED PATHS /usr/local/lib /usr/lib)
find_library(XINFER_LIB xinfer REQUIRED PATHS /usr/local/lib /usr/lib)

message(STATUS "[net-guard] Linked Blackbox Essential: ${BLACKBOX_LIB}")
message(STATUS "[net-guard] Linked Xinfer Essential:   ${XINFER_LIB}")

set(LIB_SOURCES
    src/core/tsc_clock.cpp
    src/protocols/gtpu_pfcp.cpp
    src/protocols/goose_sv_l2.cpp
    src/protocols/itch_mdp.cpp
    src/protocols/oran_e2.cpp
    src/protocols/pqc_tls.cpp
    src/defense/slice_airgap.cpp
    src/pipeline/guard_engine.cpp
    src/crypto/pqc_kem_engine.cpp
)

add_library(net_guard SHARED ${LIB_SOURCES})
add_library(net_guard_static STATIC ${LIB_SOURCES})
set_target_properties(net_guard_static PROPERTIES OUTPUT_NAME net_guard)

target_include_directories(net_guard PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:/home/kami/blackbox-sentinel/include>
    $<BUILD_INTERFACE:/usr/local/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_include_directories(net_guard_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:/home/kami/blackbox-sentinel/include>
    $<BUILD_INTERFACE:/usr/local/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(net_guard PUBLIC
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
    Threads::Threads
    OpenSSL::Crypto
)

target_link_libraries(net_guard_static PUBLIC
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
    Threads::Threads
    OpenSSL::Crypto
)

add_executable(net-guard-ctl tools/net_guard_ctl/main.cpp)
target_link_libraries(net-guard-ctl PRIVATE
    net_guard
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
)

add_executable(aryo-net tools/aryo_net/main.cpp)
target_link_libraries(aryo-net PRIVATE
    net_guard
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
)

add_library(net_guard_dissector_plugin SHARED
    tools/sentinel_plugin/net_guard_plugin.cpp
)
target_link_libraries(net_guard_dissector_plugin PRIVATE
    net_guard
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
)
set_target_properties(net_guard_dissector_plugin PROPERTIES
    PREFIX ""
    OUTPUT_NAME "net_guard_dissector_plugin"
)

add_executable(bench_latency benchmarks/bench_latency.cpp)
target_link_libraries(bench_latency PRIVATE
    net_guard
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
)

enable_testing()

add_executable(test_gtpu_slice tests/test_gtpu_slice.cpp)
target_link_libraries(test_gtpu_slice PRIVATE net_guard)
add_test(NAME TestGtpuSlice COMMAND test_gtpu_slice)

add_executable(test_goose_l2 tests/test_goose_l2.cpp)
target_link_libraries(test_goose_l2 PRIVATE net_guard)
add_test(NAME TestGooseL2 COMMAND test_goose_l2)

add_executable(test_itch_oran_pqc tests/test_itch_oran_pqc.cpp)
target_link_libraries(test_itch_oran_pqc PRIVATE net_guard)
add_test(NAME TestItchOranPqc COMMAND test_itch_oran_pqc)

add_executable(test_pqc_mlkem tests/test_pqc_mlkem.cpp)
target_link_libraries(test_pqc_mlkem PRIVATE net_guard OpenSSL::Crypto)
add_test(NAME TestPqcMlKem COMMAND test_pqc_mlkem)


add_executable(test_plugin tests/test_sentinel_plugin.cpp)
target_link_libraries(test_plugin PRIVATE
    net_guard
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
    ${CMAKE_DL_LIBS}
)
add_test(NAME TestSentinelPlugin COMMAND test_plugin)

add_executable(test_net_tier1_tier2_pipeline tests/test_net_tier1_tier2_pipeline.cpp)
target_link_libraries(test_net_tier1_tier2_pipeline PRIVATE
    net_guard
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
    ${CMAKE_DL_LIBS}
)
add_test(NAME TestNetTier1Tier2Pipeline COMMAND test_net_tier1_tier2_pipeline)

install(TARGETS net_guard net_guard_static net-guard-ctl aryo-net net_guard_dissector_plugin
    EXPORT NetGuardTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(TARGETS net_guard_dissector_plugin
    DESTINATION /usr/local/lib/blackbox
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

set(CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/NetGuard)
configure_package_config_file(
    cmake/NetGuardConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/NetGuardConfig.cmake
    INSTALL_DESTINATION ${CONFIG_INSTALL_DIR}
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/NetGuardConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/NetGuardConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/NetGuardConfigVersion.cmake
    DESTINATION ${CONFIG_INSTALL_DIR}
)
install(EXPORT NetGuardTargets
    FILE NetGuardTargets.cmake
    NAMESPACE NetGuard::
    DESTINATION ${CONFIG_INSTALL_DIR}
)

configure_file(
    cmake/net_guard.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/net_guard.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/net_guard.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)