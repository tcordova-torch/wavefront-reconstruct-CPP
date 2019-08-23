//-----------------------------------------------------------------------------
//
// CXPSDKCommon.cpp : Common helper functions for using the Silicon Software
//                    siso_genicam library  
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
#include "CXPSDKCommon.h"


bool manualExit(){
#ifdef _WIN32
	return (kbhit() != 0);
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
#ifdef ADIMEC_OPAL
	//-----------------------------------------------------------------------
	// Adimec OPAL
	//-----------------------------------------------------------------------
	WaitPowerOn = 3000;
	const char cGenicamXMLFile[]="OpalCXP.xml";
	strcpy(GenicamXMLFile,cGenicamXMLFile);
	// Connection properties
	LinkSpeed = LINK_SPEED_2500;
	StreamPaketDataSize = 1024*4;
	// Link Topology
	NrOfLinks = 1;
	PortMask = 0x01; // grabber port A
	// Image Properties
	Width = 1920; 
	Height = 1080; 
	#endif

	#ifdef ADIMEC_QUARTZ
	//-----------------------------------------------------------------------
	// QUARTZ
	//-----------------------------------------------------------------------
	WaitPowerOn = 10000;
	const char cGenicamXMLFile[]="CXP_Quartz.xml";
	strcpy(GenicamXMLFile,cGenicamXMLFile);
	// Connection properties
	LinkSpeed = LINK_SPEED_3125;
	StreamPaketDataSize = 1024*4;
	// Link Topology
	NrOfLinks = 2; 
	PortMask = 0x03; // Quartz uses Port A..B
	// Image Properties 
	Width  = 2048; 
	Height = 1100; 
	#endif
	#ifdef MICROTRON
	WaitPowerOn = 1000;
	strcpy(GenicamXMLFile,"");
	// Link Topology
	PortMask = 0x0F; // Microtron uses Port A .. D
	NrOfLinks = 4;
	StreamPaketDataSize = 1024*4;
	// Image properties
	Width  = 2336; 
	Height = 1728; 
	#endif
	#ifdef OPTRONIS
	//-----------------------------------------------------------------------
	// OPTRONIS
	//-----------------------------------------------------------------------
	WaitPowerOn = 5000;
	const char cGenicamXMLFile[]="CL4000CXP_V0.3.xml";
	strcpy(GenicamXMLFile,cGenicamXMLFile);
	// Connection properties
	LinkSpeed = LINK_SPEED_6250;
	StreamPaketDataSize = 1024*4;
	// Link Topology
	PortMask = 0x0F; // Optronis need Port A .. D
	NrOfLinks = 4;
	// Image properties
	Width  = 2304; 
	Height = 1720; 
	#endif


	#ifdef E2V
	//-----------------------------------------------------------------------
	// E2V Elixa
	//-----------------------------------------------------------------------
	WaitPowerOn = 40000; // measured: ca. 40 sec. unitl the LED is blinking green
	const char cGenicamXMLFile[]="e2v_ELIIXAC4MCP1605_1.0.0.xml";
	// Connection properties
	LinkSpeed = LINK_SPEED_6250;
	StreamPaketDataSize = 1024*4;
	// Link Topology
	NrOfLinks = 4;
	PortMask = 0x0F; // E2V need Port A .. D
	#endif // E2V

	#ifdef TOSHIBA_TELI
	//-----------------------------------------------------------------------
	// Toshiba Teli
	//-----------------------------------------------------------------------
	WaitPowerOn = 1000;
	const char cGenicamXMLFile[]="ED_200.xml";
	strcpy(GenicamXMLFile,cGenicamXMLFile);
	// Connection properties
	LinkSpeed = LINK_SPEED_5000;
	StreamPaketDataSize = 1024*4;
	// Link Topology
	NrOfLinks = 1;
	PortMask = 0x01; // grabber port A
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
void DoConnectToCamera( struct SgcBoardHandle* bh,  struct SgcCameraHandle** chOut, unsigned int portIndex, DiscoveryStyle discoveryStyle)
{
	//----------------------------------------------
	//----------------------------------------------
	*chOut = NULL;
	SgcCameraHandle* ch = 0;
	int rc = 0;
	const unsigned int DiscoveryTimeout = 5000; // 5 sec.

	if (discoveryStyle == Automatic){
		rc = Sgc_scanPorts(bh, PortMask, DiscoveryTimeout, LinkSpeed );
		if (rc != 0){
			printf("Error during camera discovery: %s", Sgc_getErrorDescription(rc));
			printf("Press any key to continue");

			getchar();
			return;
		}
		printf("Camera discovery done");
		rc = Sgc_getCamera(bh,portIndex,&ch);
		if (rc != 0){
			printf("Cannot get camera handle: %s\n", Sgc_getErrorDescription(rc));
			printf("Press any key to continue\n");
			getchar();
			return;
		}
		// connect by using the discovered connection parameters and topology
		rc = Sgc_LinkConnect(bh, ch);
		if (rc != 0){
			printf("unable to connect to camera: %s\n", Sgc_getErrorDescription(rc));
			printf("Press any key to continue\n");
			getchar();
			return;
		}
	} else if (discoveryStyle == ConfigFile) {
		rc = Sgc_LoadBoardConfiguration(bh, LINKCONFIGFILE);
		if (rc != 0){
			printf("Cannot load configuration file '%s': %s\n", LINKCONFIGFILE, Sgc_getErrorDescription(rc));
			printf("Press any key to continue\n");
			getchar();
			return;
		}
		if ((rc = Sgc_SaveBoardConfiguration(bh, LINKCONFIGFILE)) == SGC_OK) {
			printf("Wrote discovery results to file '%s'\n", LINKCONFIGFILE);
		} else {
			printf("Error writing scan results to file '%s': %s\n", LINKCONFIGFILE, Sgc_getErrorDescription(rc));
		}
		int linkGroupCount = Sgc_getCameraCount(bh);
		bool found = false;
		for (unsigned int linkGroup = 0; linkGroup < static_cast<unsigned int>(linkGroupCount); ++linkGroup)
		{
			Sgc_getCameraByIndex(bh, linkGroup, &ch);
			if ((rc = Sgc_LinkConnect(bh, ch)) == SGC_OK){
				found = true;
				break;
			}
		}
		if (!found) {
			printf("unable to connect to camera: %s\n", Sgc_getErrorDescription(rc));
			printf("Press any key to continue\n");
			getchar();
			return;
		}
	}
	else{
		//--------------------------------------------------------
		// manual configuration
		//--------------------------------------------------------
		rc = Sgc_getCamera(bh,portIndex,&ch);
		if (rc != 0){
			printf("Cannot get camera handle: %s\n", Sgc_getErrorDescription(rc));
			printf("Press any key to continue\n");
			getchar();
			return;
		}

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

		// connect by using the predefined connection parameters and topology
		rc = Sgc_LinkConnect(bh, ch);
		if (rc != 0){
			printf("unable to connect to camera: %s\n", Sgc_getErrorDescription(rc));
			printf("Press any key to continue\n");
			getchar();
			return;
		}

		//---------------------------------------------------------------------------
		// read LinkConfig register for testing of connection
		uint32_t linkConfig = 0;
		rc = Sgc_registerReadFromCamera(ch, &linkConfig, 0x00004014);
		if (rc != 0){
			printf("Reading LinkConfig failed: %s\n", Sgc_getErrorDescription(rc));
			printf("Press any key to continue\n");
			getchar();
		}
		else{
			printf("LinkConfig at port x is: %x\n", linkConfig);
		}

		uint32_t StreamPacketSize = 0;
		rc = Sgc_registerReadFromCamera(ch, &StreamPacketSize, 0x00004010);
		if (rc != 0){
			printf("Reading StreamPacketSize failed: %s\n", Sgc_getErrorDescription(rc));
			printf("Press any key to continue\n");
			getchar();
		}
		else{
			printf("StreamPacketSize at port x is: %x\n", StreamPacketSize);
		}
	}

	if ((rc = Sgc_SaveBoardConfiguration(bh, LINKCONFIGFILE)) == SGC_OK) {
		printf("Wrote discovery results to file '%s'\n", LINKCONFIGFILE);
	} else {
		printf("Error writing scan results to file '%s': %s\n", LINKCONFIGFILE, Sgc_getErrorDescription(rc));
	}
	*chOut = ch;
}


//----------------------------------------------------------------------
// displays some camera properties
//----------------------------------------------------------------------
void getAndShowCameraProperty( SgcCameraHandle* ch, const char* propertyStringIn, unsigned int& value, unsigned int addValue, const int linkIndex = -1)
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
	int rc = 0;
	int64_t width  = aWidth;
	int64_t height = aHeight;
	int64_t pixelFormat = aPixelFormat; // format 0x101 Mono8, 0x103 Mono12
	rc = Sgc_setIntegerValue(ch,"Width",width);
	if (rc != 0){
		printf("Error writing: %s: %s\n", "Width", Sgc_getErrorDescription(rc));
	}
	else{
		int64_t widthRead = 0; 
		rc = Sgc_getIntegerValue(ch,"Width",&widthRead);
		if (rc != 0){
			printf("Error reading: %s: %s\n", "Width", Sgc_getErrorDescription(rc));
		}
		else{
			if (widthRead != width){
				printf("Error Setting Width: %ld\n", width);
			}
			else{
				printf("Width set to: %ld\n", width);
			}
		}
	}

	rc = Sgc_setIntegerValue(ch,"Height",height);
	if (rc != 0){
		printf("Error writing: %s: %s\n", "Height", Sgc_getErrorDescription(rc));

	}
	else{
		int64_t heightRead = 0; 
		rc = Sgc_getIntegerValue(ch,"Height",&heightRead);
		if (rc != 0){
			printf("Error reading: %s: %s\n", "Height", Sgc_getErrorDescription(rc));
		}
		else{
			if (heightRead != height){
				printf("Error Setting Height: %ld\n", height);
			}
			else{
				printf("Height set to: %ld\n", height);
			}
		}
	};
	rc = Sgc_setEnumerationValue(ch,"PixelFormat","Mono8");
	if (rc != 0){
		printf("Error reading: %s\n", "PixelFormat");
	}
	else{
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
	// -> GrabberControlled Modus
#ifdef OPTRONIS
	if (TriggerMode == 1){
		// Enum "Acquisition Mode" -> "CoaxPress"
		Sgc_setEnumerationValue(ch,"AcquisitionMode", "CoaxPress");
	}
	else{
		Sgc_setEnumerationValue(ch,"AcquisitionMode", "Continuous");
	}
#endif

#ifdef E2V
	if (TriggerMode == 1){
		// Enum "SynchroMode Mode" -> 2 == Max. Exposure Mode
		int rc = 0;
		int64_t valueReadback = -1;
		valueReadback = -1;
		rc = Sgc_setEnumerationValue(ch,"SynchroMode", "ExtTrigExpMax");
		rc = Sgc_getEnumerationValue(ch,"SynchroMode",&valueReadback);
		if (rc != 0){
			printf("SynchroMode failed: %d\n", rc);
		}

		valueReadback = -1;
		rc = Sgc_setEnumerationValue(ch,"AcquisitionMode", "Standard");
		rc = Sgc_getEnumerationValue(ch,"AcquisitionMode",&valueReadback);
		if (rc != 0){
			printf("AcquisitionMode failed: %d\n", rc);
		}


		valueReadback = -1;
		rc = Sgc_setEnumerationValue(ch,"TriggerSelector", "ExposureActive");
		Sgc_getEnumerationValue(ch,"TriggerSelector",&valueReadback);

		rc = Sgc_setBooleanValue(ch,"TriggerMode", 1);

		valueReadback = -1;
		rc = Sgc_setEnumerationValue(ch,"TriggerSource", "Line2");
		Sgc_getEnumerationValue(ch,"TriggerSource",&valueReadback);

		valueReadback = -1;
		rc = Sgc_setEnumerationValue(ch,"TriggerActivation", "RisingEdge");
		Sgc_getEnumerationValue(ch,"TriggerActivation",&valueReadback);
	

	}
	else{
		Sgc_setEnumerationValue(ch,"SynchroMode", "FreeRun");
	}
#endif
}


//----------------------------------------------------------------------
// Starts the acquisition at the camera by using the GenICam interface
//----------------------------------------------------------------------
void DoStartAcquisition(struct SgcBoardHandle* bh, struct SgcCameraHandle* ch)
{
	// use GenICam command to start acquisition
	Sgc_startAcquisition(ch, true);
}


//----------------------------------------------------------------------
// Stops the acquisition at the camera by using the GenICam interface
//----------------------------------------------------------------------
void DoStopAcquisition(struct SgcBoardHandle* bh, struct SgcCameraHandle* ch)
{

	int rc = 0;
	const char regName[] = "AcquisitionStop";
	rc = Sgc_executeCommand(ch,  regName);
	if (rc != 0){
		printf("Error stopping Acquisition\n");
	}
	else{
		printf("Acquisition stopped\n");
	}
}


