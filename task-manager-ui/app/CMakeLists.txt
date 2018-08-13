set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Quick WebSockets REQUIRED)

PROJECT_TARGET_ADD(taskmanager)

option(NATIVE_BUILD "Build for native environment, without homescreen and window manager" OFF)
if(NATIVE_BUILD)
    message(STATUS "Native build is ON")
    add_definitions(-DNATIVE_BUILD)
endif()

add_executable(taskmanager
	"main.cpp"
	"taskmanager.cpp"
	"procinfo.cpp"
	"qml.qrc"
)
#qt5_use_modules(taskmanager Core Quick WebSockets)
set_target_properties(taskmanager PROPERTIES
	LABELS "EXECUTABLE"
	PREFIX ""
	COMPILE_FLAGS "${EXTRAS_CFLAGS} -DFOR_AFB_BINDING"
	LINK_FLAGS "${BINDINGS_LINK_FLAG}"
	LINK_LIBRARIES "${EXTRAS_LIBRARIES}"
	OUTPUT_NAME "${TARGET_NAME}"
)

target_link_libraries(taskmanager
    Qt5::Core
	Qt5::Quick
	Qt5::WebSockets
	json-c
	afb-helpers
)

#if(NOT NATIVE_BUILD)
    #target_link_libraries(taskmanager
        #homescreen
		#qtwindowmanager
    #)
#endif()
