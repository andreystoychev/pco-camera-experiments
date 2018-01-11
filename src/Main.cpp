#include <iostream>
#include <string>
#include <windows.h>
#include <chrono>
#include <thread>

#include "PCO_err.h"
#include "sc2_SDKStructures.h"
#include "SC2_SDKAddendum.h"
#include "SC2_CamExport.h"
#include "SC2_Defs.h"

#include "TiffFunctions.h"


int main() {
	int iRet;
	HANDLE cam_red;
	HANDLE cam_green;
	HANDLE BufEvent = NULL;
	short BufNum = -1;
	WORD *BufAdr = NULL;
	PCO_Description strDescription;
	WORD RecordingState;
	std::string file_name = "";

	std::cout << "Getting a HANDLE to the second (green) camera" << std::endl << std::endl;
	iRet = PCO_OpenCamera(&cam_red, 0);
	iRet = PCO_OpenCamera(&cam_green, 0);
	iRet = PCO_CloseCamera(cam_red);

	if (iRet != PCO_NOERROR) {
		std::cout << "No camera found" << std::endl;
		std::cout << "Press <Enter> to exit" << std::endl;
		std::string temp;
		std::getline(std::cin, temp);
		return -1;
	}

	strDescription.wSize = sizeof(PCO_Description);
	//dwAcquModeExNumberImages
	//PCO_SetRecordStopEvent
	iRet = PCO_GetCameraDescription(cam_green, &strDescription);

	int binning = 4;
	for (int i = 1; i <= 1000; i++) {
		iRet = PCO_GetRecordingState(cam_green, &RecordingState);
		if (RecordingState) {
			iRet = PCO_SetRecordingState(cam_green, 0);
		}

		// set camera to default state
		iRet = PCO_ResetSettingsToDefault(cam_green);

		iRet = PCO_SetTimestampMode(cam_green, 0);

		iRet = PCO_SetBinning(cam_green, binning, binning);
		if (iRet) {
			std::cout << "ERROR: Setting the binning returned an error." << std::endl;
		}

		iRet = PCO_EnableSoftROI(cam_green, 1, NULL, 0);
		if (iRet) {
			std::cout << "ERROR: Setting the SoftROI mode returned an error." << std::endl;
		}

		if (binning == 1) {
			iRet = PCO_SetROI(cam_green, 1, 1, 2560, 2160);
		} else if (binning == 2) {
			iRet = PCO_SetROI(cam_green, 1, 1, 1024, 1024);
		} else if (binning == 4) {
			iRet = PCO_SetROI(cam_green, 1, 1, 512, 512);
		} else {
			std::cout << "ERROR: Wrong binning detected." << std::endl;
			std::cout << "Press <Enter> to exit" << std::endl;
			std::string temp;
			std::getline(std::cin, temp);
			iRet = PCO_CloseCamera(cam_green);
			return -1;
		}
		if (iRet) {
			std::cout << "ERROR: Setting the ROI returned an error." << std::endl;
		}

		iRet = PCO_SetDelayExposureTime(cam_green, 0, 8, 0x0002, 0x0002);
		if (iRet) {
			std::cout << "ERROR: Setting the exposure time returned an error." << std::endl;
		}

		iRet = PCO_SetNoiseFilterMode(cam_green, 0x0001);
		if (iRet) {
			std::cout << "ERROR: Setting the noise filtering returned an error." << std::endl;
		}

		iRet = PCO_ArmCamera(cam_green);

		DWORD CameraWarning, CameraError, CameraStatus;
		iRet = PCO_GetCameraHealthStatus(cam_green, &CameraWarning, &CameraError, &CameraStatus);
		if (CameraError != 0) {
			std::cout << "Camera has ErrorStatus" << std::endl;
			std::cout << "Press <Enter> to exit" << std::endl;
			std::string temp;
			std::getline(std::cin, temp);
			iRet = PCO_CloseCamera(cam_green);
			return -1;
		}

		WORD XResAct, YResAct, XResMax, YResMax;
		DWORD bufsize;
		iRet = PCO_GetSizes(cam_green, &XResAct, &YResAct, &XResMax, &YResMax);
		std::cout << "Size is: " << XResAct << "x" << YResAct << std::endl;

		DWORD currentDelay, currentExposure;
		WORD currentDelayTimeBase, currentExposureTimeBase;
		iRet = PCO_GetDelayExposureTime(cam_green, &currentDelay, &currentExposure, &currentDelayTimeBase, &currentExposureTimeBase);
		std::cout << "Exposure is: " << currentExposure << std::endl;

		bufsize = XResAct*YResAct * sizeof(WORD);
		//BufEvent = NULL;
		//BufNum = -1;
		//BufAdr = NULL;
		iRet = PCO_AllocateBuffer(cam_green, &BufNum, bufsize, &BufAdr, &BufEvent);
		iRet = PCO_SetImageParameters(cam_green, XResAct, YResAct, IMAGEPARAMETERS_READ_WHILE_RECORDING, NULL, 0);

		std::cout << "Starting camera" << std::endl;
		iRet = PCO_SetRecordingState(cam_green, 1);

		std::cout << "Grabbing image from running camera...";
		iRet = PCO_GetImageEx(cam_green, 1, 0, 0, BufNum, XResAct, YResAct, 16);
		if (iRet != PCO_NOERROR) {
			std::cout << "failed" << std::endl;
			break;
		} else {
			std::cout << "done" << std::endl;
		}

		file_name = "img\\green__" + std::to_string(binning) + "x" + std::to_string(binning) + "__" + std::to_string(XResAct) + "x" + std::to_string(YResAct) + "__" + std::to_string(8) + "ms__none__" + std::to_string(i) + ".tiff";
		store_tiff(file_name.c_str(), XResAct, YResAct, 0, BufAdr);
		std::cout << "Storing image to " << file_name << std::endl << std::endl;

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	std::cout << "Stopping camera and closing connection" << std::endl;
	iRet = PCO_SetRecordingState(cam_green, 0);
	iRet = PCO_FreeBuffer(cam_green, BufNum);
	iRet = PCO_CloseCamera(cam_green);

	std::cout << "Press <Enter> to end" << std::endl;
	std::string temp;
	std::getline(std::cin, temp);
	return 0;
}