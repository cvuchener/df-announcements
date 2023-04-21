if(GIT_EXECUTABLE)
	execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --dirty
		WORKING_DIRECTORY ${PROJECT_DIR}
		OUTPUT_VARIABLE VERSION_NAME
		RESULT_VARIABLE GIT_STATUS
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_QUIET
	)
	if(GIT_STATUS)
		unset(VERSION_NAME)
	else()
		message(STATUS "Using version from git ${VERSION_NAME}")
	endif()
endif()
if(NOT DEFINED VERSION_NAME)
	message(STATUS "Using fallback version ${FALLBACK_VERSION}")
	set(VERSION_NAME ${FALLBACK_VERSION})
endif()
configure_file(${VERSION_IN} ${VERSION_OUT} @ONLY)
