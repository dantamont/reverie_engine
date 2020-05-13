IF(NOT EXISTS "C:/Users/dante/Documents/Projects/grand-blue-engine/third_party/assimp/assimp-5.0.0/build/install_manifest.txt")
  MESSAGE(FATAL_ERROR "Cannot find install manifest: \"C:/Users/dante/Documents/Projects/grand-blue-engine/third_party/assimp/assimp-5.0.0/build/install_manifest.txt\"")
ENDIF(NOT EXISTS "C:/Users/dante/Documents/Projects/grand-blue-engine/third_party/assimp/assimp-5.0.0/build/install_manifest.txt")

FILE(READ "C:/Users/dante/Documents/Projects/grand-blue-engine/third_party/assimp/assimp-5.0.0/build/install_manifest.txt" files)
STRING(REGEX REPLACE "\n" ";" files "${files}")
FOREACH(file ${files})
  MESSAGE(STATUS "Uninstalling \"$ENV{DESTDIR}${file}\"")
  EXEC_PROGRAM(
    "C:/Users/dante/Documents/Projects/grand-blue-engine/third_party/cmake-3.16.3-win64-x64/bin/cmake.exe" ARGS "-E remove \"$ENV{DESTDIR}${file}\""
    OUTPUT_VARIABLE rm_out
    RETURN_VALUE rm_retval
    )
  IF(NOT "${rm_retval}" STREQUAL 0)
    MESSAGE(FATAL_ERROR "Problem when removing \"$ENV{DESTDIR}${file}\"")
  ENDIF(NOT "${rm_retval}" STREQUAL 0)
ENDFOREACH(file)
