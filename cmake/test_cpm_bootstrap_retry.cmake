# Reproducible validation for cmake/CPM.cmake bootstrap retry / final failure.
# Usage (from repo root):
#   cmake -P cmake/test_cpm_bootstrap_retry.cmake
#
# Case A: bad URL must fail after bounded retries with clear diagnostics and
#         must not leave a partial final file.
# Case B: real URL must succeed (hash verified) when network is available.

cmake_minimum_required(VERSION 3.14)

set(_cpm_test_root "${CMAKE_CURRENT_LIST_DIR}/../build-cpm-retry-test")
file(REMOVE_RECURSE "${_cpm_test_root}")
file(MAKE_DIRECTORY "${_cpm_test_root}")

set(_cpm_script "${CMAKE_CURRENT_LIST_DIR}/CPM.cmake")
set(_cpm_hash "83e5eb71b2bbb8b1f2ad38f1950287a057624e385c238f6087f94cdfc44af9c5")

# --- Case A: final failure ---
set(_fail_bin "${_cpm_test_root}/fail")
file(MAKE_DIRECTORY "${_fail_bin}")
set(_fail_dest "${_fail_bin}/cmake/CPM_0.38.7.cmake")
execute_process(
  COMMAND "${CMAKE_COMMAND}"
          "-DCPM_DOWNLOAD_VERSION=0.38.7"
          "-DCPM_HASH_SUM=${_cpm_hash}"
          "-DCPM_DOWNLOAD_MAX_RETRIES=2"
          "-DCPM_DOWNLOAD_URL=https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.0.0/DOES_NOT_EXIST.cmake"
          "-DCMAKE_BINARY_DIR=${_fail_bin}"
          -P "${_cpm_script}"
  WORKING_DIRECTORY "${_fail_bin}"
  RESULT_VARIABLE _fail_rc
  OUTPUT_VARIABLE _fail_out
  ERROR_VARIABLE _fail_err
)
if(_fail_rc EQUAL 0)
  message(FATAL_ERROR "Case A: expected CPM bootstrap failure, but cmake exited 0.\n${_fail_out}\n${_fail_err}")
endif()
set(_fail_combined "${_fail_out}\n${_fail_err}")
if(NOT _fail_combined MATCHES "Failed to download CPM.cmake")
  message(FATAL_ERROR "Case A: missing final-failure diagnostics.\n${_fail_combined}")
endif()
if(NOT _fail_combined MATCHES "attempt 1/2")
  message(FATAL_ERROR "Case A: expected retry warning for attempt 1/2.\n${_fail_combined}")
endif()
if(EXISTS "${_fail_dest}")
  message(FATAL_ERROR "Case A: partial/final file must not remain at ${_fail_dest}")
endif()
if(EXISTS "${_fail_dest}.download")
  message(FATAL_ERROR "Case A: temp download file must not remain at ${_fail_dest}.download")
endif()
message(STATUS "Case A OK: bounded retries + clear final failure, no partial file")

# --- Case B: success path (requires network) ---
set(_ok_bin "${_cpm_test_root}/ok")
file(MAKE_DIRECTORY "${_ok_bin}")
set(_ok_dest "${_ok_bin}/cmake/CPM_0.38.7.cmake")
execute_process(
  COMMAND "${CMAKE_COMMAND}"
          "-DCPM_DOWNLOAD_VERSION=0.38.7"
          "-DCPM_HASH_SUM=${_cpm_hash}"
          "-DCPM_DOWNLOAD_MAX_RETRIES=5"
          "-DCMAKE_BINARY_DIR=${_ok_bin}"
          -P "${_cpm_script}"
  WORKING_DIRECTORY "${_ok_bin}"
  RESULT_VARIABLE _ok_rc
  OUTPUT_VARIABLE _ok_out
  ERROR_VARIABLE _ok_err
)
if(NOT _ok_rc EQUAL 0)
  message(FATAL_ERROR "Case B: expected CPM bootstrap success.\n${_ok_out}\n${_ok_err}")
endif()
if(NOT EXISTS "${_ok_dest}")
  message(FATAL_ERROR "Case B: missing downloaded file ${_ok_dest}")
endif()
file(SHA256 "${_ok_dest}" _ok_got_hash)
if(NOT _ok_got_hash STREQUAL "${_cpm_hash}")
  message(FATAL_ERROR "Case B: hash mismatch got=${_ok_got_hash}")
endif()
if(EXISTS "${_ok_dest}.download")
  message(FATAL_ERROR "Case B: temp download file must not remain")
endif()
message(STATUS "Case B OK: download + SHA256 verification succeeded")

file(REMOVE_RECURSE "${_cpm_test_root}")
message(STATUS "All CPM bootstrap retry checks passed")
