//-----------------------------------------------------------------------------
//
// CLHSSDKCommon.cpp : Common helper functions for using the Silicon Software
//                     siso_genicam library  
//
//-----------------------------------------------------------------------------

#include <iostream>

#ifdef _WIN32
#include <conio.h>
#else
#include <stdio.h>
#endif
#include <string.h>
#include <stdlib.h>


#include <siso_genicam.h>
#include <siso_genicam_error.h>

#include "CLHSSDKCommon.h"


bool manualExit(){
#ifdef _WIN32
	return (bool) kbhit();
#else
	return false;
#endif
}

unsigned int getTimeStamp()
{
	// GetTickCount();
#ifdef _WIN32
	return GetTickCount();
#else
	return 0;
#endif
}

//-----------------------------------------------------------------------------
//
// Camera specific parameters
//
//-----------------------------------------------------------------------------

void DoSetCameraControlParameters()
{
#ifdef PCO
	//-----------------------------------------------------------------------
	// PCO Flow
	//-----------------------------------------------------------------------
	WaitPowerOn = 10;
	const char cGenicamXMLFile[]="undefined";
	strcpy(GenicamXMLFile,cGenicamXMLFile);
	// Connection properties
	LinkSpeed = 0;
	StreamPaketDataSize = 1024*4;
	// Link Topology
	NrOfLinks = 1;
	PortMask = 0x03; // grabber port A
	// Image Properties
	Width = 1024; 
	Height = 1024; 
	#endif
}

//-----------------------------------------------------------------
// activates power control over CoaxPress and waits a certain
// period of time for booting up the camera
//-----------------------------------------------------------------
void DoPowerOnForVariousCams( Fg_Struct* fg, struct SgcBoardHandle* bh,  unsigned int portIndex)
{
	printf("Idle time for booting camera: %d\n", WaitPowerOn);
	Sgc_powerSwitch(bh, 0xff);
	Sleep(WaitPowerOn); // Wait for boot up of the camera
}


//----------------------------------------------------------------------
// connects to CXP cameras either by automatic discovery of
// by manual link topology setup
//----------------------------------------------------------------------
int DoConnectToCamera( struct SgcBoardHandle* bh,  struct SgcCameraHandle** chOut, unsigned int portIndex, DiscoveryStyle discoveryStyle)
{
	//----------------------------------------------
	//----------------------------------------------
	*chOut = NULL;
	SgcCameraHandle* ch = 0;
	int rc = 0;
	const unsigned int discoveryTimeout = 5000; // 5 sec.
	// automatic camera discovery
	if (discoveryStyle == Automatic){

		unsigned int LocalPortMask(PortMask);
		switch(portIndex){
			case 1: LocalPortMask = PortMask | 0x100; break; // nur initiieren
			case 2: LocalPortMask = 1 | 0x300; break; // Master
			case 3: LocalPortMask = 1 | 0x500; break; // Slave
		}
		rc = Sgc_scanPorts(bh, LocalPortMask, discoveryTimeout, LinkSpeed );
		if (rc != 0){
			printf("Error during camera discovery: %s\n", Sgc_getErrorDescription(rc));
//			printf("Press any key to continue");
			//getchar();
			return rc;
		}
		printf("Camera discovery done\n");
		rc = Sgc_getCamera(bh,portIndex,&ch);
		if (rc != 0){
			printf("Cannot get camera handle: %s\n", Sgc_getErrorDescription(rc));
			printf("Press any key to continue\n");
			getchar();
			return rc;
		}
	} else if (discoveryStyle == ConfigFile) {
		// CLHS: not supported yet
	}
	else{
		// Manual

		//--------------------------------------------------------
		// manual configuration
		//--------------------------------------------------------
		rc = Sgc_getCamera(bh,portIndex,&ch);
		if (rc != 0){
			printf("Cannot get camera handle: %s\n", Sgc_getErrorDescription(rc));
			printf("Press any key to continue\n");
			getchar();
			return rc;
		}

		NrOfLinks = 1;

		//--------------------------------------------------------
		// configure the link - topology
		//--------------------------------------------------------
		rc = Sgc_setCameraPropertyWithType(ch, CAM_PROP_NROFLINKS_S, &NrOfLinks, SGC_PROPERTY_TYPE_UINT, NULL);
		if (rc == 0){
			// configure ports


			// set 1. link as Master
			unsigned int link2Set = 0;
			Sgc_setCameraPropertyWithType(ch, CAM_PROP_MASTERLINK_S, &link2Set, SGC_PROPERTY_TYPE_UINT, NULL);

			// set 1. Link connected to frame grabber port A
			int fgPort = FG_PORT_A; // FG-Port A
			Sgc_setCameraPropertyWithType(ch, CAM_PROP_FGPORT_S, &fgPort, SGC_PROPERTY_TYPE_UINT, &link2Set);

			// assign this link to 1st applet applet port(= 0)
			int appletPort = FG_APPLETPORT_A; // applet port A
			Sgc_setCameraPropertyWithType(ch, CAM_PROP_APPLETPORT_S, &appletPort, SGC_PROPERTY_TYPE_UINT, &link2Set);


			if (NrOfLinks > 1){
				// set 2. link as Extension Link
				link2Set = 1;
				Sgc_setCameraPropertyWithType(ch, CAM_PROP_EXTENSIONLINK_S, &link2Set, SGC_PROPERTY_TYPE_UINT, NULL);

				// set 2. Link connected to frame grabber port D
				fgPort = FG_PORT_B; // FG-Port B
				Sgc_setCameraPropertyWithType(ch, CAM_PROP_FGPORT_S, &fgPort, SGC_PROPERTY_TYPE_UINT, &link2Set);

				// set 2. Link connected to 2nd applet port (=1)
				appletPort = FG_APPLETPORT_B; // Applet port B
				Sgc_setCameraPropertyWithType(ch, CAM_PROP_APPLETPORT_S, &appletPort, SGC_PROPERTY_TYPE_UINT, &link2Set);
			}

			if (NrOfLinks > 2){
				// set 3. link as Extension Link
				link2Set = 2;
				Sgc_setCameraPropertyWithType(ch, CAM_PROP_EXTENSIONLINK_S, &link2Set, SGC_PROPERTY_TYPE_UINT, NULL);

				// set 3. Link connected to frame grabber port C
				fgPort = FG_PORT_C; // FG-Port C
				Sgc_setCameraPropertyWithType(ch, CAM_PROP_FGPORT_S, &fgPort, SGC_PROPERTY_TYPE_UINT, &link2Set);

				// set 3. Link connected to 2nd applet port (=1)
				appletPort = FG_APPLETPORT_C; // Applet port C
				Sgc_setCameraPropertyWithType(ch, CAM_PROP_APPLETPORT_S, &appletPort, SGC_PROPERTY_TYPE_UINT, &link2Set);
			}

			if (NrOfLinks > 3){
				// set 3. link as Extension Link
				link2Set = 3;
				Sgc_setCameraPropertyWithType(ch, CAM_PROP_EXTENSIONLINK_S, &link2Set, SGC_PROPERTY_TYPE_UINT, NULL);

				// set 3. Link connected to frame grabber port D
				fgPort = FG_PORT_D; // FG-Port D
				Sgc_setCameraPropertyWithType(ch, CAM_PROP_FGPORT_S, &fgPort, SGC_PROPERTY_TYPE_UINT, &link2Set);

				// set 3. Link connected to 2nd applet port (=1)
				appletPort = FG_APPLETPORT_D; // Applet port D
				Sgc_setCameraPropertyWithType(ch, CAM_PROP_APPLETPORT_S, &appletPort, SGC_PROPERTY_TYPE_UINT, &link2Set);
			}

		}


		//---------------------------------------------------------------------------
		// set Link - speed
/*
		rc = Sgc_setCameraPropertyWithType(ch, CAM_PROP_LINKSPEED_S, &LinkSpeed, SGC_PROPERTY_TYPE_UINT, NULL);
		if (rc != 0){
			printf("Cannot Set CAM_PROP_STREAMPACKET_SIZE_S: %s", Sgc_getErrorDescription(rc));
			printf("Press any key to continue");
			getchar();
		}
		// StreamPacketSize
		rc = Sgc_setCameraPropertyWithType(ch, CAM_PROP_STREAMPACKET_SIZE_S, &StreamPaketDataSize, SGC_PROPERTY_TYPE_UINT, NULL);
		if (rc != 0){
			printf("Cannot Set CAM_PROP_STREAMPACKET_SIZE_S: %s", Sgc_getErrorDescription(rc));
			printf("Press any key to continue");
			getchar();
		}
*/
		// connect by using the predefined connection parameters and topology
		rc = Sgc_LinkConnect(bh, ch);
		if (rc != 0){
			printf("unable to connect to camera: %s\n", Sgc_getErrorDescription(rc));
			printf("Press any key to continue\n");
			getchar();
			return rc;
		}


		// CLHS: not supported yet
	}

	// query the detected cameras
	int cameraCount = Sgc_getCameraCount(bh);
	bool found = false;
	for (unsigned int cameraIndex = 0; cameraIndex < static_cast<unsigned int>(cameraCount); ++cameraIndex)
	{
		Sgc_getCameraByIndex(bh, cameraIndex, &ch);
		if ((rc = Sgc_LinkConnect(bh, ch)) == SGC_OK){
			found = true;
			break;
		}
	}
	if (!found) {
		printf("unable to connect to camera: %s\n", Sgc_getErrorDescription(rc));
		printf("Press any key to continue\n");
		getchar();
		return rc;
	}

	*chOut = ch;
	return rc;
}


//----------------------------------------------------------------------
// displays some camera properties
//----------------------------------------------------------------------
void getAndShowCameraProperty( SgcCameraHandle* ch, char* propertyStringIn, unsigned int& value, unsigned int addValue, const int linkIndex = -1)
{
	int rc = 0;
	char propertyString[255];
	strcpy(propertyString, propertyStringIn);
	unsigned int propertyType = SGC_PROPERTY_TYPE_UINT;
	rc = Sgc_getCameraPropertyWithType(ch,propertyString, &value, &propertyType, &addValue);
	if (rc != 0){
		printf("Error reading: %s : %s \n", propertyString, Sgc_getErrorDescription(rc));
	}
	else{
		if (linkIndex >= 0){
			printf("%s[%d]: %d\n", propertyString, linkIndex, value);
		}
		else{
			printf("%s: %d\n", propertyString, value);
		}
	}
}



//----------------------------------------------------------------------
// queries the available cameras and displays some camera properties
//----------------------------------------------------------------------
void DoLookForCameras(struct SgcBoardHandle* bh)
{
	int rc = 0;
	const unsigned int nrOfCams = Sgc_getCameraCount(bh);
	printf("No. of Cameras : %d\n", nrOfCams);

	for (unsigned int cameraIndex = 0; cameraIndex < nrOfCams; cameraIndex++){
		SgcCameraHandle* ch = NULL;
		rc = Sgc_getCameraByIndex(bh,cameraIndex, &ch);
		if (rc == 0 ){
			//---------------------------------------------------------------
			// Show Camera Connection Properties
			//---------------------------------------------------------------
			unsigned int value;
			unsigned int addValue;
			unsigned int nrOfLinks = 0;

			addValue = 0;
			value = 0;
			getAndShowCameraProperty(ch, CAM_PROP_LINKSPEED_S, value, addValue);

			value = 0;
			getAndShowCameraProperty(ch, CAM_PROP_DISCOVERYSPEED_S, value, addValue);

			value = 0;
			getAndShowCameraProperty(ch, CAM_PROP_CONTROLPACKETSIZE_S, value, addValue);

			value = 0;
			getAndShowCameraProperty(ch, CAM_PROP_STREAMPACKET_SIZE_S, value, addValue);

			value = 0;
			getAndShowCameraProperty(ch, CAM_PROP_MASTERID_S, value, addValue);

			nrOfLinks = 0;
			getAndShowCameraProperty(ch, CAM_PROP_NROFLINKS_S, nrOfLinks, addValue);

			// Iteration over all links
			for (unsigned int linkIndex = 0; linkIndex < nrOfLinks; linkIndex++){
				addValue = linkIndex;
				value = 0;
				getAndShowCameraProperty(ch, CAM_PROP_MASTERLINK_S, value, addValue, linkIndex);

				value = 0;
				getAndShowCameraProperty(ch, CAM_PROP_EXTENSIONLINK_S, value, addValue, linkIndex);

				value = 0;
				getAndShowCameraProperty(ch, CAM_PROP_FGPORT_S, value, addValue, linkIndex);

				value = 0;
				getAndShowCameraProperty(ch, CAM_PROP_APPLETPORT_S, value, addValue, linkIndex);

				value = 0;
				getAndShowCameraProperty(ch, CAM_PROP_EXTENSIONID_S, value, addValue, linkIndex);
			}
		}
		else{
			const char* errorMsg = Sgc_getErrorDescription(rc);
			printf("Error while looking for camera %i : Error : %s\n", cameraIndex, errorMsg);
		}
	}
}




//----------------------------------------------------------------------
// Create a GenICam handling
//----------------------------------------------------------------------
void DoGenicamConnection( struct SgcCameraHandle* ch, const char* GenicamXMLFile, bool UseCameraInternalXML)
{
	int rc = 0;

	// Add genapi to temp. to path variable in oder to load the DLLs	
	const char* sisodir = getenv("SISODIR5");
	const char* pathvar = getenv("PATH");
	char pathVarNew[2048];
	char sisoGenApiDir[512];

	strcpy(pathVarNew, "PATH=");
	strcat(pathVarNew, pathvar);
	strcat(pathVarNew, ";");

	strcpy(sisoGenApiDir, sisodir);
	// Add (SISODIR5)\genicam\bin\Win32_i86 to path
	strcat(sisoGenApiDir, "/genicam/bin/Win32_i86");
	strcat(pathVarNew, sisoGenApiDir);

	// Add (SISODIR5)\genicam\bin\Win32_i86\GenApi to path
	strcpy(sisoGenApiDir, sisodir);
	strcat(sisoGenApiDir, "/genicam/bin/Win32_i86/GenApi");
	strcat(pathVarNew, ";");
	strcat(pathVarNew, sisoGenApiDir);
	putenv(pathVarNew);

	if (UseCameraInternalXML){
		printf("Sgc_connectCamera\n");
		rc = Sgc_connectCamera(ch);
		if (rc != 0){
			printf("Sgc_connectCamera() failed\n");
			printf("Error Code: %d\n",rc);
			printf("Error: %s\n",Sgc_getErrorDescription(rc));
			printf("hit any key to exit");
			getchar();
			exit(rc);
		}
	}
	else{
		printf("Sgc_connectCameraWithExternalXml\n");
		rc = Sgc_connectCameraWithExternalXml(ch, GenicamXMLFile);
		if (rc != 0){
			printf("Sgc_connectCameraWithExternalXml(%s) failed\n",GenicamXMLFile);
			printf("Error Code: %d\n",rc);
			printf("Error: %s\n",Sgc_getErrorDescription(rc));
			printf("hit any key to exit");
			getchar();
			exit(rc);
		}
	}
}

//----------------------------------------------------------------------
// sets the image dimension and pixel format at the camera by using the 
// GenICam interface
//----------------------------------------------------------------------
void DoSetImageFormat(struct SgcBoardHandle* bh, struct SgcCameraHandle* ch, int aWidth, int aHeight, int aPixelFormat)
{
	// TODO: Implement GenICam calls in order to set the requested width, height, pixel format at
	// the camera

	int rc = 0;
	int64_t width  = aWidth;
	int64_t height = aHeight;
	int64_t pixelFormat = aPixelFormat; // format 0x101 Mono8, 0x103 Mono12
	rc = Sgc_setIntegerValue(ch,"Width",width);
	if (rc != 0){
		printf("Error writing: %s: %s\n", "Width", Sgc_getErrorDescription(rc));
	}
	else{
		// Readback of the values to check success
		int64_t widthRead = 0; 
		rc = Sgc_getIntegerValue(ch,"Width",&widthRead);
		if (rc != 0){
			printf("Error reading: %s: %s\n", "Width", Sgc_getErrorDescription(rc));
		}
		else{
			if (widthRead != width){
				printf("Error Setting Width: %d\n", width);
			}
			else{
				printf("Width set to: %d\n", width);
			}
		}
	}

	rc = Sgc_setIntegerValue(ch,"Height",height);
	if (rc != 0){
		printf("Error writing: %s: %s\n", "Height", Sgc_getErrorDescription(rc));

	}
	else{
		// Readback of the values to check success
		int64_t heightRead = 0; 
		rc = Sgc_getIntegerValue(ch,"Height",&heightRead);
		if (rc != 0){
			printf("Error reading: %s: %s\n", "Height", Sgc_getErrorDescription(rc));
		}
		else{
			if (heightRead != height){
				printf("Error Setting Height: %d\n", height);
			}
			else{
				printf("Height set to: %d\n", height);
			}
		}
	};
	rc = Sgc_setEnumerationValue(ch,"PixelFormat","Mono8");
	if (rc != 0){
		printf("Error reading: %s\n", "PixelFormat");
	}
	else{
		// Readback of the values to check success
	}
}
//----------------------------------------------------------------------
// enables a certain trigger mode at the camera by using the GenICam 
// interface.
// TriggerMode 0: "FreeRun" mode
// TriggerMode 1: "GrabberControlled" mode
//----------------------------------------------------------------------
void DoSetAcquisitionControl(struct SgcBoardHandle* bh, struct SgcCameraHandle* ch, int TriggerMode)
{
	// TODO: map Trigger modes to GenICam command in order to synchronize
	// applet setup and camera modes
}


//----------------------------------------------------------------------
// Starts the acquisition at the camera by using the GenICam interface
//----------------------------------------------------------------------
void DoStartAcquisition(struct SgcBoardHandle* bh, struct SgcCameraHandle* ch)
{
	// TODO: Write camera register in order to start the data transfer from camera

	// use GenICam command to start acquisition
	printf("Starting acquisition at camera\n");
	int rc = Sgc_startAcquisition(ch, true);
	if (rc != 0){
		printf("Error starting camera\n");
	}
}


//----------------------------------------------------------------------
// Stops the acquisition at the camera by using the GenICam interface
//----------------------------------------------------------------------
void DoStopAcquisition(struct SgcBoardHandle* bh, struct SgcCameraHandle* ch)
{
	// TODO: Write camera register in order to stop the data transfer from camera
	printf("Stopping acquisition at camera\n");
	int rc = Sgc_stopAcquisition(ch, 1);
	if (rc != 0){
		printf("Error stopping camera\n");
	}
/*
	const char regName[] = "AcquisitionStop";
	rc = Sgc_executeCommand(ch,  regName);
	if (rc != 0){
		printf("Error stopping Acquisition\n");
	}
	else{
		printf("Acquisition stopped\n");
	}
*/
}



void ShowCameraInfo(SgcCameraHandle* ch)
{
  SgcCameraInfo* CameraInfo = Sgc_getCameraInfo(ch);
  if (ch){
    printf("Camera vendor:   %s\n", CameraInfo->deviceVendorName);
    printf("Camera model:    %s\n", CameraInfo->deviceModelName);
    printf("Camera family:   %s\n", CameraInfo->deviceFamilyName);
    printf("Camera serial:   %s\n", CameraInfo->deviceSerialNumber);
    printf("Camera firmware: %s\n", CameraInfo->deviceFirmwareVersion);
    printf("Camera version:  %s\n", CameraInfo->deviceVersion);
  }
  else{
    printf("Sgc_getCameraInfo failed\n");
  }
}



