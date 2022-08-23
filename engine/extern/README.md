see assimp/BUILDBINARIES_MSVC2019.bat for instructions
see physx/compiler for build project
see soloud/build/BUILDBINARIES..., build SoLoudStatic 

Example to move submodules (in this case, make sure that extern/physx exists): 
 git mv third_party/physx/physx.submodule/ extern/physx/physx.submodule
 
To update submodules:
git submodule update --init --recursive --remote

