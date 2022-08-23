Nice visual studio extensions:
https://www.syncfusion.com/blogs/post/15-must-have-visual-studio-extensions-for-developers.aspx#visual-studio-spell-checker

For deployment info, see:
https://doc.qt.io/qt-5/windows-deployment.html

For PDBs (very annoying to find), see: https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_5152/qt.qt5.5152.debug_info.win64_msvc2019_64/

Resolutions to known issues:

1) Missing dlls: https://stackoverflow.com/questions/28732602/qt-example-executables-wont-run-missing-qt5cored-dll
	Can 1) Add the missing dlls using the windows deployment tool, 2) move them manually, or 3) move the dll directory to the system PATH environmental variable
	

GPU doesn't support open GL ES 3.0:
https://community.khronos.org/t/open-gl-es-3-1-installation-on-windows-7-64-bit-with-intel-hd-5500/76499/2
	Critical  Aug 24, 2019 @ 15:35:39.889  Gb::GL::Shader
	Error, could not compile Vertex shader
	Critical  Aug 24, 2019 @ 15:35:39.914  Gb::GL::Shader
	ERROR: 0:2: '#extension' :  'GL_OES_sample_shading' is not supported
	ERROR: 0:2: '#extension' :  'GL_OES_sample_variables' is not supported
	ERROR: 0:2: '#extension' :  'GL_OES_shader_image_atomic' is not supported
	ERROR: 0:2: '#extension' :  'GL_OES_shader_multisample_interpolation' is not supported
	ERROR: 0:2: '#extension' :  'GL_OES_texture_storage_multisample_2d_array' is not supported
	ERROR: 0:2: '#extension' :  'GL_OES_geometry_shader' is not supported
	ERROR: 0:2: '#extension' :  'GL_OES_gpu_shader5' is not supported
	ERROR: 0:2: '#extension' :  'GL_OES_primitive_bounding_box' is not supported
	ERROR: 0:2: '#extension' :  'GL_OES_shader_io_blocks' is not supported
	ERROR: 0:2: '#extension' :  'GL_OES_tessellation_shader' is not supported
	ERROR: 0:2: '#extension' :  'GL_OES_texture_buffer' is not supported
	ERROR: 0:2: '#extension' :  'GL_OES_texture_cube_map_array' is not supported