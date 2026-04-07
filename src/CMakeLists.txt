cmake_minimum_required(VERSION 3.28)

find_package(mo2-cmake CONFIG REQUIRED)
find_package(mo2-uibase CONFIG REQUIRED)
find_package(mo2-lootcli-header CONFIG REQUIRED)
find_package(mo2-dds-header CONFIG REQUIRED)
find_package(mo2-bsatk CONFIG REQUIRED)
find_package(mo2-libbsarch CONFIG REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS WebEngineWidgets)
find_package(ZLIB REQUIRED)
find_package(lz4 CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED COMPONENTS program_options thread interprocess signals2 uuid accumulators)

include(FetchContent)

FetchContent_Declare(
	ryml
	EXCLUDE_FROM_ALL
	GIT_REPOSITORY https://github.com/biojppm/rapidyaml.git
	GIT_TAG master
)

FetchContent_MakeAvailable(ryml)

include(${CMAKE_CURRENT_SOURCE_DIR}/ParseTES.cmake)

add_library(bsplugins SHARED)

set_property(TARGET bsplugins PROPERTY CXX_STANDARD 20)
target_sources(bsplugins PRIVATE ${TES_INCLUDE_FILES})
target_include_directories(
	bsplugins
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include
)
target_link_libraries(bsplugins PRIVATE ryml)

if(MSVC)
	target_compile_options(
		bsplugins
		PRIVATE
			/bigobj # increase number of sections in .obj file
			/W4     # level 4 warnings
			/WX     # treat all compiler warnings as errors
			/wd4201 # nonstandard extension used: nameless struct/union
			/wd4458 # declaration of 'identifier' hides class member
	)
endif()

mo2_configure_plugin(bsplugins
	WARNINGS OFF
	PRIVATE_DEPENDS
		mo2::uibase mo2::bsatk mo2::lootcli-header
		mo2::libbsarch Boost::program_options Boost::signals2
		Boost::uuid Boost::accumulators ZLIB::ZLIB
		Qt::WebEngineWidgets
)
mo2_default_source_group()
target_link_libraries(bsplugins
	PUBLIC
		mo2::dds-header
	PRIVATE
		mo2::uibase mo2::bsatk mo2::lootcli-header
		mo2::libbsarch Boost::program_options Boost::signals2
		Boost::uuid Boost::accumulators ZLIB::ZLIB
		Qt6::WebEngineWidgets)
mo2_install_plugin(bsplugins)
