Multi-Media Samples README

Gettting Started:

Enter the ./mmsoglwin folder and open the solution file.  Rebuild the entire solution to compile all projects and related dependencies.

Licenses:

Source code provided within the ./mmsoglwin folder is provided under the Intel Sample Source Code License Agreement.  A copy of this license is provided in the ./licenses folder.

Source code provided in the ./decode folder is provided under the Intel(R) Media SDK EULA.  Please download the SDK to obtain a copy of the latest EULA from here: https://software.intel.com/en-us/media-sdk

Source code provided in the ./gl folder is provided under the Khronos* License.  A copy of this license is provided in the ./licenses folder.

Sample videos in the ./testcontent folder are provided by Elemental* under the Creative Commons License.  A copy of this license is provided in the ./licenses folder.

Source Tree Structure:

+---decode
|   +---common
|   |   |   sample_common.sln
|   |   |   sample_common.vcproj
|   |   |   sample_common.vcxproj
|   |   |   sample_common.vcxproj.filters
|   |   |   sample_common.vcxproj.user
|   |   |   
|   |   +---include
|   |   |   |   base_allocator.h
|   |   |   |   current_date.h
|   |   |   |   d3d11_allocator.h
|   |   |   |   d3d11_device.h
|   |   |   |   d3d_allocator.h
|   |   |   |   d3d_device.h
|   |   |   |   d3d_device.h.orig
|   |   |   |   hw_device.h
|   |   |   |   sample_defs.h
|   |   |   |   sample_utils.h
|   |   |   |   sysmem_allocator.h
|   |   |   |   vpp_ex.h
|   |   |   |   
|   |   |   \---vm
|   |   |           atomic_defs.h
|   |   |           file_defs.h
|   |   |           so_defs.h
|   |   |           strings_defs.h
|   |   |           thread_defs.h
|   |   |           time_defs.h
|   |   |           
|   |   +---props
|   |   |       winsdk_win32.props
|   |   |       winsdk_win32.vsprops
|   |   |       winsdk_x64.props
|   |   |       winsdk_x64.vsprops
|   |   |       
|   |   \---src
|   |       |   base_allocator.cpp
|   |       |   d3d11_allocator.cpp
|   |       |   d3d11_device.cpp
|   |       |   d3d_allocator.cpp
|   |       |   d3d_device.cpp
|   |       |   d3d_device.cpp.orig
|   |       |   sample_utils.cpp
|   |       |   sysmem_allocator.cpp
|   |       |   vpp_ex.cpp
|   |       |   
|   |       \---vm
|   |               atomic.cpp
|   |               shared_object.cpp
|   |               thread.cpp
|   |               
|   +---include
|   |       decode_render.h
|   |       pipeline_decode.h
|   |       
|   \---src
|           decode_render.cpp
|           pipeline_decode.cpp
|           
+---gl
|       glcorearb.h
|       glext.h
|       glxext.h
|       wglext.h
|       
+---licenses
|       CC-BY-ND-3.0.txt
|       Intel Sample Source Code License Agreement.txt
|       Khronos License (MIT).txt
|       Readme.txt
|       
+---mmsoglwin
|   |   mmsoglwin.sln
|   |   
|   +---mmsoglwin1
|   |       mmsoglwin1.cpp
|   |       mmsoglwin1.h
|   |       mmsoglwin1.ico
|   |       mmsoglwin1.rc
|   |       mmsoglwin1.vcxproj
|   |       mmsoglwin1.vcxproj.filters
|   |       ReadMe.txt
|   |       Resource.h
|   |       simpleDX9Device.cpp
|   |       simpleDX9Device.h
|   |       small.ico
|   |       targetver.h
|   |       
|   +---mmsoglwin2
|   |       mmsoglwin2.cpp
|   |       mmsoglwin2.h
|   |       mmsoglwin2.ico
|   |       mmsoglwin2.rc
|   |       mmsoglwin2.vcxproj
|   |       mmsoglwin2.vcxproj.filters
|   |       ReadMe.txt
|   |       Resource.h
|   |       simpleDX9Device.cpp
|   |       simpleDX9Device.h
|   |       small.ico
|   |       targetver.h
|   |       
|   \---mmsoglwin3
|           mmsoglwin3.cpp
|           mmsoglwin3.h
|           mmsoglwin3.ico
|           mmsoglwin3.rc
|           mmsoglwin3.vcxproj
|           mmsoglwin3.vcxproj.filters
|           ReadMe.txt
|           Resource.h
|           simpleDX9Device.cpp
|           simpleDX9Device.h
|           small.ico
|           targetver.h
|           
\---testcontent
        test0.264
        test1.264
        test2.264
        test3.264
        test4.264
        test5.264

Attribution and thanks:

MMS relies on a number of technologies and test content to allow users to quickly begin experimenting with media applications. The following section provides details on how these technologies are used within MMS. It also provides links to the online resources where developers may go to learn more about them for use in their own projects. 

• Elemental* 4k test content - MMS uses several test clips created by Elemental. For the purposes of MMS the h264 raw bitstreams were extracted from the mp4 container with no modifications. No unique additions or changes were made to the raw bitstreams. Special thanks to the Elemental* team for this very useful test content. Elemental* test clips may be obtained from their website located here: http://www.elementaltechnologies.com/resources/4k-test-sequences . Please note that this content is provided under the Creative Commons Attribution-NoDerivs 3.0 Unported License. All Elemental* test content may be found in the ./testcontent folder. Full license text may be found within the framework licenses folder and at the following online link: http://creativecommons.org/licenses/by-nd/3.0/deed.en_US
 
• Blender* Open Projects Content - MMS leverages video test content available courtesy of the Blender Institute in Amsterdam. Blender* Open Projects content may be found here: http://www.blender.org/features/projects/ . Big Buck Bunny is provided under the Creative Commons Attribution 3.0 Unported (CC BY 3.0) license. More details are available here: https://peach.blender.org/
 
• OpenGL* - MMS uses the OpenGL* API to enable our video texturing samples and includes several OpenGL* header files in the source code bundle to make development environment configuration easier for users. These files include: glcorearb.h, glext.h, glxext.h, and wglext.h. For license information and additional detail please visit https://www.opengl.org/registry/
 
• Microsoft* DirectX* Version 9.x API - MMS uses the DirectX* Version 9.x Direct3D* API to enable our video texturing examples. The full Microsoft* DirectX* version 9.x SDK may be downloaded from the web at the following link: http://www.microsoft.com/en-us/download/details.aspx?id=6812 

• Intel® Media SDK Decode Samples - MMS uses sample source code from the Intel® Media SDK to enable all decode functionality. All code contained within ./decode and ./common is provided under the Intel® Media SDK licensing terms. Many thanks to the Intel® Media SDK team for the very useful sample applications that led to the creation of MMS. 





