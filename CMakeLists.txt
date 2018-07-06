cmake_minimum_required(VERSION 3.9)
project(Orvis)


set(ORV_EXECUTABLES_FOLDER      ${PROJECT_SOURCE_DIR}/exe)
set(ORV_LIBRARIES_FOLDER		${PROJECT_SOURCE_DIR}/lib)
set(ORV_EXTERNAL_FOLDER			${PROJECT_SOURCE_DIR}/ext)
set(ORV_BINARIES_FOLDER			${PROJECT_SOURCE_DIR}/bin)

set(ORV_RESOURCES_PATH			${PROJECT_SOURCE_DIR}/res)
set(ORV_SHADERS_PATH			${PROJECT_SOURCE_DIR}/shd)


#find_package(GLEW REQUIRED)
find_package(assimp REQUIRED)
find_package(glm 0.9.8 REQUIRED)
find_package(glfw3 3.2.1 REQUIRED)
find_package(glbinding REQUIRED)

#list(APPEND ORV_INCLUDE_DIRECTORIES ${GLEW_INCLUDE_PATH})

add_definitions(
    -DGLFW_INCLUDE_NONE
    -DSTBI_MSC_SECURE_CRT
	-D_SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING
	-D_SILENCE_PARALLEL_ALGORITHMS_EXPERIMENTAL_WARNING
    -DGLM_FORCE_RADIANS -DGLM_ENABLE_EXPERIMENTAL
    -DNOMINMAX
    -DSOURCE_DIRECTORY="${CMAKE_SOURCE_DIR}/"
)

if(MSVC)
    add_compile_options(/std:c++latest /MP /openmp /permissive- /Zc:twoPhase- /wd4251)
    set(ORV_LINKER_FLAGS "/NODEFAULTLIB:libcmt /ignore:4098,4099,4221 /MANIFEST:NO")
    list(APPEND ORV_LIBRARIES opengl32)
    file(COPY "${ASSIMP_ROOT_DIR}/bin${ASSIMP_ARCHITECTURE}/assimp-vc140-mt.dll" DESTINATION "${ORV_BINARIES_FOLDER}/Debug")
    file(COPY "${ASSIMP_ROOT_DIR}/bin${ASSIMP_ARCHITECTURE}/assimp-vc140-mt.dll" DESTINATION "${ORV_BINARIES_FOLDER}/Release")
else()
endif()


##### internal libraries
list(APPEND ORV_INCLUDE_DIRECTORIES "${ORV_LIBRARIES_FOLDER}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${ORV_BINARIES_FOLDER}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  "${ORV_BINARIES_FOLDER}")
file(GLOB children RELATIVE ${ORV_LIBRARIES_FOLDER} ${ORV_LIBRARIES_FOLDER}/*)
foreach(subdir ${children})
    if(IS_DIRECTORY ${ORV_LIBRARIES_FOLDER}/${subdir})
        file(GLOB_RECURSE ORV_SOURCE_FILE "${ORV_LIBRARIES_FOLDER}/${subdir}/*.cpp")
        if(NOT ORV_SOURCE_FILE)
            # Internal header only library
            set(success 0)
        else()
            add_library(${subdir} ${ORV_SOURCE_FILE})
            target_link_libraries(${subdir} PRIVATE glfw glbinding::glbinding ${ASSIMP_LIBRARIES} glm)

            list(APPEND ORV_LIBRARIES ${subdir})
            set_target_properties(${subdir} PROPERTIES
											LINKER_LANGUAGE CXX
											CXX_STANDARD 17
											CXX_STANDARD_REQUIRED ON)
            target_include_directories(${subdir} PUBLIC ${ORV_INCLUDE_DIRECTORIES})
        endif()
    endif()
endforeach()
include_directories(${ORV_INCLUDE_DIRECTORIES})

##### external libraries residing inside the project
list(APPEND ORV_INCLUDE_DIRECTORIES ${ORV_EXTERNAL_FOLDER})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${ORV_BINARIES_FOLDER}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  "${ORV_BINARIES_FOLDER}")
file(GLOB children RELATIVE ${ORV_EXTERNAL_FOLDER} ${ORV_EXTERNAL_FOLDER}/*)
foreach(subdir ${children})
    if(IS_DIRECTORY ${ORV_EXTERNAL_FOLDER}/${subdir})
        file(GLOB_RECURSE ORV_SOURCE_FILE "${ORV_EXTERNAL_FOLDER}/${subdir}/*.cpp")

        if(NOT ORV_SOURCE_FILE)
            # Internal header only library
            set(success 0)

        else()
            ### handle structures like: external/<something>/include as proper include paths
            if(IS_DIRECTORY "${ORV_EXTERNAL_FOLDER}/${subdir}/include")
                list(APPEND ORV_INCLUDE_DIRECTORIES "${ORV_EXTERNAL_FOLDER}/${subdir}/include")
            endif()

            add_library(${subdir} ${ORV_SOURCE_FILE})
            list(APPEND ORV_LIBRARIES ${subdir})
            target_link_libraries(${subdir} PRIVATE glbinding::glbinding)
            set_target_properties(${subdir} PROPERTIES
											LINKER_LANGUAGE CXX
											CXX_STANDARD 17
											CXX_STANDARD_REQUIRED ON)
            target_include_directories(${subdir} PUBLIC ${ORV_INCLUDE_DIRECTORIES})
        endif()
    endif()
endforeach()
include_directories(${ORV_INCLUDE_DIRECTORIES})


##### internal executables
file(GLOB children RELATIVE ${ORV_EXECUTABLES_FOLDER} ${ORV_EXECUTABLES_FOLDER}/*)
foreach(subdir ${children})
    if(IS_DIRECTORY ${ORV_EXECUTABLES_FOLDER}/${subdir})
        file(GLOB_RECURSE EXE_SOURCES "${ORV_EXECUTABLES_FOLDER}/${subdir}/*.cpp")

        add_executable(${subdir} ${EXE_SOURCES})
        target_link_libraries(${subdir} PRIVATE ${ORV_LIBRARIES})
        target_link_libraries(${subdir} PRIVATE glfw glbinding::glbinding ${ASSIMP_LIBRARIES} glm)
        #file(COPY ${ORV_DLLS} DESTINATION "${ORV_BINARIES_FOLDER}")
        set_target_properties(${subdir} PROPERTIES
										LINKER_LANGUAGE CXX
										RUNTIME_OUTPUT_DIRECTORY "${ORV_BINARIES_FOLDER}"
										CXX_STANDARD 17
										CXX_STANDARD_REQUIRED ON)
        target_include_directories(${subdir} PUBLIC ${ORV_INCLUDE_DIRECTORIES})
    endif()
endforeach()
