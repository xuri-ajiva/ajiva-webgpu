set(targetFile ${CMAKE_SOURCE_DIR}/Engine/src/Resource/FilesNames.hpp)
file(WRITE ${targetFile} "#pragma once\n\n")
file(APPEND ${targetFile} "#include <string>\n\n")
file(APPEND ${targetFile} "namespace Ajiva::Resource\n{\n")
file(APPEND ${targetFile} "    class Files\n    {\n    public:\n")

message(STATUS "Target file: ${targetFile}")
message(STATUS "Source dir: ${CMAKE_SOURCE_DIR}/resources")
function(createFileStruct dir)
    message(STATUS "Create File Struct for: ${dir}")
    #get all files and directories in the current directory
    file(GLOB entries ${dir}/*)
    #iterate over all entries
    foreach (entry ${entries})
        #2. call itself for each subdirectory
        if (IS_DIRECTORY ${entry})
            #1. add a struct with the directory name
            #get a name for the struct
            get_filename_component(structName ${entry} NAME)
            #add the struct
            file(APPEND ${targetFile} "        struct ${structName}\n        {\n")
            message(STATUS "Create File Struct for: ${structName}")
            createFileStruct(${entry})
            #close the struct
            file(APPEND ${targetFile} "        };\n")
        else ()
            #3. add a static const std::string for each file
            #get a name for the file, replace . with _
            get_filename_component(fileName ${entry} NAME)
            string(REPLACE "." "_" fileName ${fileName})
            string(REPLACE "-" "_" fileName ${fileName})
            string(REPLACE " " "_" fileName ${fileName})
            #get value for string, the relative path from the resources folder
            file(RELATIVE_PATH fileValue ${CMAKE_SOURCE_DIR}/resources ${entry})
            message(STATUS "Add File: ${fileValue}")
            #add the static const std::string
            file(APPEND ${targetFile} "            static constexpr std::string_view ${fileName} = \"${fileValue}\";\n")
        endif ()
    endforeach ()
endfunction()

createFileStruct(${CMAKE_SOURCE_DIR}/resources)

file(APPEND ${targetFile} "    };\n}\n")
