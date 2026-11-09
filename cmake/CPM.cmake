# SPDX-License-Identifier: MIT
#
# SPDX-FileCopyrightText: Copyright (c) 2019-2023 Lars Melchior and contributors
#
# Local change: bounded retries around the CPM bootstrap download so transient
# GitHub release CDN failures (HTTP 5xx / network blips) do not fail configure
# on the first attempt. Partial downloads go to a .download temp file and are
# never promoted to the final include path. Hash is verified manually so a
# failed HTTP response does not abort via EXPECTED_HASH before retries run.

set(CPM_DOWNLOAD_VERSION 0.38.7)
set(CPM_HASH_SUM "83e5eb71b2bbb8b1f2ad38f1950287a057624e385c238f6087f94cdfc44af9c5")

if(NOT DEFINED CPM_DOWNLOAD_URL)
  set(CPM_DOWNLOAD_URL
      "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake")
endif()

if(NOT DEFINED CPM_DOWNLOAD_MAX_RETRIES)
  set(CPM_DOWNLOAD_MAX_RETRIES 5)
endif()

if(CPM_SOURCE_CACHE)
  set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
  set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else()
  set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

# Expand relative path. This is important if the provided path contains a tilde (~)
get_filename_component(CPM_DOWNLOAD_LOCATION ${CPM_DOWNLOAD_LOCATION} ABSOLUTE)

set(_cpm_need_download TRUE)
if(EXISTS "${CPM_DOWNLOAD_LOCATION}")
  file(SHA256 "${CPM_DOWNLOAD_LOCATION}" _cpm_existing_hash)
  if(_cpm_existing_hash STREQUAL "${CPM_HASH_SUM}")
    set(_cpm_need_download FALSE)
  else()
    message(STATUS
      "CPM.cmake at ${CPM_DOWNLOAD_LOCATION} has unexpected SHA256 "
      "(${_cpm_existing_hash}); re-downloading")
    file(REMOVE "${CPM_DOWNLOAD_LOCATION}")
  endif()
endif()

if(_cpm_need_download)
  get_filename_component(_cpm_download_dir "${CPM_DOWNLOAD_LOCATION}" DIRECTORY)
  file(MAKE_DIRECTORY "${_cpm_download_dir}")

  # Never write a failed/partial body onto the final include path.
  set(_cpm_download_tmp "${CPM_DOWNLOAD_LOCATION}.download")
  set(_cpm_download_ok FALSE)
  set(_cpm_last_status "not attempted")

  foreach(_cpm_attempt RANGE 1 ${CPM_DOWNLOAD_MAX_RETRIES})
    file(REMOVE "${_cpm_download_tmp}")
    # Do not pass EXPECTED_HASH here: on HTTP failure CMake fatals before
    # STATUS can be inspected, which prevents retries. Verify SHA256 below.
    file(DOWNLOAD
         "${CPM_DOWNLOAD_URL}"
         "${_cpm_download_tmp}"
         STATUS _cpm_status
         TIMEOUT 60
         TLS_VERIFY ON
    )
    list(GET _cpm_status 0 _cpm_status_code)
    set(_cpm_last_status "${_cpm_status}")

    set(_cpm_attempt_ok FALSE)
    if(_cpm_status_code EQUAL 0 AND EXISTS "${_cpm_download_tmp}")
      file(SHA256 "${_cpm_download_tmp}" _cpm_got_hash)
      if(_cpm_got_hash STREQUAL "${CPM_HASH_SUM}")
        set(_cpm_attempt_ok TRUE)
      else()
        set(_cpm_last_status
            "hash mismatch: got ${_cpm_got_hash}, expected ${CPM_HASH_SUM}")
        file(REMOVE "${_cpm_download_tmp}")
      endif()
    else()
      file(REMOVE "${_cpm_download_tmp}")
    endif()

    if(_cpm_attempt_ok)
      # Replace destination only after a hash-verified download.
      file(REMOVE "${CPM_DOWNLOAD_LOCATION}")
      file(RENAME "${_cpm_download_tmp}" "${CPM_DOWNLOAD_LOCATION}")
      set(_cpm_download_ok TRUE)
      if(_cpm_attempt GREATER 1)
        message(STATUS "CPM.cmake download succeeded on attempt ${_cpm_attempt}")
      endif()
      break()
    endif()

    message(WARNING
      "CPM.cmake download attempt ${_cpm_attempt}/${CPM_DOWNLOAD_MAX_RETRIES} "
      "failed (status=${_cpm_last_status}) from ${CPM_DOWNLOAD_URL}")
    if(_cpm_attempt LESS ${CPM_DOWNLOAD_MAX_RETRIES})
      execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep ${_cpm_attempt})
    endif()
  endforeach()

  if(NOT _cpm_download_ok)
    file(REMOVE "${_cpm_download_tmp}")
    message(FATAL_ERROR
      "Failed to download CPM.cmake v${CPM_DOWNLOAD_VERSION} after "
      "${CPM_DOWNLOAD_MAX_RETRIES} attempts.\n"
      "  URL: ${CPM_DOWNLOAD_URL}\n"
      "  Expected SHA256: ${CPM_HASH_SUM}\n"
      "  Destination: ${CPM_DOWNLOAD_LOCATION}\n"
      "  Last status: ${_cpm_last_status}\n"
      "Check network access to GitHub releases, or set CPM_SOURCE_CACHE / "
      "pre-populate the destination file with the expected hash.")
  endif()
endif()

include("${CPM_DOWNLOAD_LOCATION}")
