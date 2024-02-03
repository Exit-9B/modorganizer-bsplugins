cmake_minimum_required(VERSION 3.27)

include(FetchContent)

FetchContent_Declare(
	esp_json
	GIT_REPOSITORY https://github.com/matortheeternal/esp.json.git
	GIT_TAG master
)

FetchContent_MakeAvailable(esp_json)

set(PYTHON_BUILD_PATH ${PYTHON_ROOT}/PCBuild/amd64)
set(Python_EXECUTABLE ${PYTHON_BUILD_PATH}/python.exe)

find_program(CLANG_FORMAT clang-format)

set(CODEGEN_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/MakeFormParser.py)

foreach(GAME SSE)
	set(INPUT_FILE ${esp_json_SOURCE_DIR}/data/${GAME}.json)
	set(OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/include/FormParser.${GAME}.inl)
	add_custom_command(
		OUTPUT ${OUTPUT_FILE}
	    COMMAND
			${Python_EXECUTABLE}
			${CODEGEN_SCRIPT}
			${INPUT_FILE}
			${OUTPUT_FILE}
		DEPENDS
			${CODEGEN_SCRIPT}
			${INPUT_FILE}
	)

	if(CLANG_FORMAT AND EXISTS ${PROJECT_SOURCE_DIR}/.clang-format)
		add_custom_command(
			OUTPUT ${OUTPUT_FILE}
			COMMAND
				${CLANG_FORMAT} -i
				--style=file:${PROJECT_SOURCE_DIR}/.clang-format
				${OUTPUT_FILE}
			APPEND
		)
	endif()
endforeach()

list(APPEND TES_INCLUDE_FILES ${OUTPUT_FILE})
