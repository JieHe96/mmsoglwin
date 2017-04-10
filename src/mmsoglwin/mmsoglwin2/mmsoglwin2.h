/*
Copyright (c) 2016, Intel Corporation All Rights Reserved.
 
The source code, information and material ("Material") contained herein is owned by Intel Corporation 
or its suppliers or licensors, and title to such Material remains with Intel Corporation or its 
suppliers or licensors. The Material contains proprietary information of Intel or its suppliers 
and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of 
the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, 
distributed or disclosed in any way without Intel's prior express written permission. 
No license under any patent, copyright or other intellectual property rights in the Material is granted 
to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any 
license under such intellectual property rights must be express and approved by Intel in writing.
 
Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other 
notice embedded in Materials by Intel or Intels suppliers or licensors in any way.*/
#pragma once

#include "resource.h"
//#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


#define MAX_LOADSTRING 100
#define MAXPOSSIBLEVIDEOS 25


#define MIX4K_GP 0
#define MIX1280 0
#define MIX1080 1
#define MIX2560 0


#define DEC1280 0
#define DEC1920 1
#define DEC2560 0
#define DEC4k 0


#if (DEC1280==1)
#define WINDOWWIDTH 1280
#define WINDOWHEIGHT 720
#else
#define WINDOWWIDTH 1920
#define WINDOWHEIGHT 1080
#endif


//! Video clip test content array.
//! Global array to hold test clips for the purposes of the sample application. See FULLHD_GP, UHD_GP, MIX4K_GP in stdafx.h.  These defines control while list of clips is used at runtime.  Uncomment the one you want to try and make sure the rest are commented out.  You will notice that the texture sizes in simpleDevice::CreateDevice() are hard coded to the video resolution size.  I did not have a convenient way to get the width / height at runtime.  By hard coding them I was able to be sure that the actual texture being displayed contained the true number of pixels in the video.  If you were to make the textures smaller than the video size, drivers / hw will resize the image and quality will be lost. To examine the image quality, try using the mouse wheel to zoom on the textures, use the arrow keys to pan as needed.*/
wchar_t *videoclips[MAXPOSSIBLEVIDEOS] = {
#if MIX4K_GP
	L"c:/MyProject/MMSF/testcontent/test5.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test0.264", /**< Clip 2 */
	L"c:/MyProject/MMSF/testcontent/test1.264", /**< Clip 3 */
	L"c:/MyProject/MMSF/testcontent/test3.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test4.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test4.264", /**< Clip 4 */
	L"c:/MyProject/MMSF/testcontent/test0.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test1.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test2.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test3.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test4.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test5.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test0.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test1.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test2.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test3.264", /**< Clip 5 */
												//L"../../testcontent/test0.264", /**< Clip 2 */
												//L"../../testcontent/test1.264", /**< Clip 3 */
												//L"../../testcontent/test3.264", /**< Clip 5 */
												//L"../../testcontent/test4.264", /**< Clip 5 */
												//L"../../testcontent/test4.264", /**< Clip 4 */
												//L"../../testcontent/test5.264", /**< Clip 5 */
												//L"../../testcontent/test0.264", /**< Clip 5 */
												//L"../../testcontent/test1.264", /**< Clip 5 */
												//L"../../testcontent/test2.264", /**< Clip 5 */
												//L"../../testcontent/test3.264", /**< Clip 5 */
												//L"../../testcontent/test4.264", /**< Clip 5 */
												//L"../../testcontent/test5.264", /**< Clip 5 */
												//L"../../testcontent/test0.264", /**< Clip 5 */
												//L"../../testcontent/test1.264", /**< Clip 5 */
												//L"../../testcontent/test2.264", /**< Clip 5 */
#endif
#if MIX1280
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
#endif
#if MIX1080
#if 1
												L"c:/share/bbb-1080-1min-01.mp4.h264",
												L"c:/share/sintel-1080-1min-01.mp4.h264",
												L"c:/share/sintel-1080-1min-02.mp4.h264",
												L"c:/share/sintel-1080-1min-03.mp4.h264",
												L"c:/MyProject/VideoClip/test-1080M.mp4.h264",//4
												L"c:/share/sintel-1080-1min-04.mp4.h264",
												L"c:/share/sintel-1080-1min-05.mp4.h264",
												L"c:/share/sintel-1080-1min-06.mp4.h264",
												L"c:/MyProject/VideoClip/test-1080M.mp4.h264",//8
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/test-1080M.mp4.h264",//12
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
#else
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",//
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",//
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",//
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",//
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",//
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",//
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
#endif
#endif
#if MIX2560
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
#endif
};
