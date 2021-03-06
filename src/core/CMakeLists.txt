cmake_minimum_required(VERSION 2.6)

project(core)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth=1024 -std=c++14 -Wextra -Wall -pedantic -Werror -Wno-unused-parameter")

set(WIN_LIBS "")

if(EMSCRIPTEN)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_SDL=2 -s TOTAL_MEMORY=512000000 -s ALLOW_MEMORY_GROWTH=1 -Oz -s DEMANGLE_SUPPORT=1")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -s USE_SDL=2 -s TOTAL_MEMORY=512000000 -s ALLOW_MEMORY_GROWTH=1 -s DEMANGLE_SUPPORT=1")
	set(ZLIB_LIBRARY "")
	set(GLEW_LIBRARIES "")
	set(OPENGL_LIBRARIES "")
	set(SDL2_LIBRARY "")
	add_definitions(-DSLOW_SYSTEM)
	
elseif(ANDROID)
#	find_package(SDL2 REQUIRED)
#	include_directories(${SDL2_INCLUDE_DIR})
#	find_package(SDL2_MIXER REQUIRED)
	
	set(SDL2_LIBRARY "-lsdl2main -lSDL2")
	set(SDLMIXER_LIBRARY "-lSDL2_mixer")
	
	find_package(ZLIB)
	include_directories(${ZLIB_INCLUDE_DIR})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
	set(GLEW_LIBRARIES "")
	set(OPENGL_LIBRARIES "")
	add_definitions(-DSLOW_SYSTEM)

else()
	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE "Debug")
	endif()

	option(SAN "Build with sanitizers" OFF)

	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		MESSAGE("Debug build")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

		if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND SAN)
			MESSAGE("Building with sanitizers")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=integer ")

		elseif(WIN32)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")

		endif()

	else()
		if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -gline-tables-only")
		else()
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
		endif()
	endif()

	add_definitions(-DSTACKTRACE)
	find_package(GLEW REQUIRED)

	find_package(OpenGL REQUIRED)
	include_directories(${OpenGL_INCLUDE_DIRS})
	link_directories(${OpenGL_LIBRARY_DIRS})
	add_definitions(${OpenGL_DEFINITIONS})
	find_package(SDL2 REQUIRED)
	include_directories(${SDL2_INCLUDE_DIR})
	find_package(SDL2_MIXER REQUIRED)
	
	
	if(WIN32)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lmingw32 -lSDL2main -lSDL2_mixer -lglew32 -lws2_32 -lwsock32") # why???
		set(WIN_LIBS "-limagehlp -lws2_32 -lwsock32")
		add_definitions(-DWIN)

	else()
		FIND_PACKAGE(ZLIB)
		INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-pie")
	endif()
endif()


file(GLOB_RECURSE CORE_SRCS
         RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
		 ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
		 ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
		 ${CMAKE_CURRENT_SOURCE_DIR}/*.hxx)

ADD_LIBRARY(core STATIC ${CORE_SRCS})
SET_TARGET_PROPERTIES(core PROPERTIES OUTPUT_NAME "core")
target_link_libraries(core ${WIN_LIBS} ${SDL2_LIBRARY} ${SDLMIXER_LIBRARY} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${ZLIB_LIBRARY} physfs-static soil Box2D)

