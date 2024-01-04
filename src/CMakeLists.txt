cmake_minimum_required(VERSION 3.22)

add_library(bsplugins SHARED)

set_property(TARGET bsplugins PROPERTY CXX_STANDARD 20)
target_include_directories(bsplugins PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

mo2_configure_plugin(
	bsplugins
	WARNINGS OFF
	PRIVATE_DEPENDS
		lootcli uibase
		boost fmt zlib
		Qt::WebEngineWidgets
)
mo2_install_target(bsplugins)
