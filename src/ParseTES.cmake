cmake_minimum_required(VERSION 3.27)

include(FetchContent)

FetchContent_Declare(
	esp_json
	GIT_REPOSITORY https://github.com/matortheeternal/esp.json.git
	GIT_TAG master
)

FetchContent_MakeAvailable(esp_json)

set(Python_FIND_VIRTUALENV STANDARD)

# find Python before include mo2-cmake, otherwise this will trigger a bunch of CMP0111
# due to the imported configuration mapping variables defined in mo2.cmake
find_package(Python ${MO2_PYTHON_VERSION} COMPONENTS Interpreter Development REQUIRED)
get_filename_component(Python_HOME ${Python_EXECUTABLE} PATH)
set(Python_DLL_DIR "${Python_HOME}/DLLs")
set(Python_LIB_DIR "${Python_HOME}/Lib")

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
