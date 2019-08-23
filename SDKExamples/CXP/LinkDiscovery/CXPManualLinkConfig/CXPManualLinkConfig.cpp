//-----------------------------------------------------------------------------
// CXPManualLinkConfig.cpp : Defines the entry point for the console application.
// 
// Sample for using the Silicon Software SiSo_Genicam library in combination
// with the  Silicon Software frame grabber SDK in order to grab images from
// a CoaxPress frame grabber connected to a GenICam compliant camera
//
// Arguments 
//  %1: Activate PowerOverCxp:  0 - no PoCXp 
//                              1 - PoCXP )
//  %2: type of Discovery:      0 - Manual link configuration, 1: Automatic discovery, 
//                              2 - Predefined ConfigFile
//  %3: type of trigger:        0 - free run
//                              1 - grabber controlled
//  %4: board index:            select board in multi board systems
//  %5: GenICam file:           filename of camera's GenICam file
//
// Example:
// 1 1 0 1 C:\\CL4000CXP_V0.3.xml
//
// Please note, that not all arguments might be in use depending on the specific
// example.
//
//-----------------------------------------------------------------------------


#include <iostream>
#include <stdlib.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <stdio.h>
#endif


#include <fgrab_prototyp.h>
#include <fgrab_struct.h>
#include <siso_genicam.h>

// sample codes 
#include <board_and_dll_chooser.h>
#include <CXPSDKCommon.h>


void displayUsage()
{
	using namespace std;
	cout << "---------------------------------" << endl;
	cout << "Silicon Software CoaxPress sample" << endl;
	cout << "---------------------------------" << endl;
	cout << "usage:" << endl;
	cout << "<Program> [powerOn] [discovery] [triggerMode] [boardIndex] [GenICam file]" << endl;
	cout << "powerOn:      Do Power switch at startup. 0 power switch off, 1 power switch on" << endl;
	cout << "discovery:    Do CoaxPress discovery for link configuration" << endl;
	cout << "0 no discovery (manual configuration 1:1), 1 CoaxPress discover on" << endl;
	cout << "trigger mode: Type of image trigger: 0 free-run mode, 1: grabber controlled mode" << endl;
	cout << "boardIndex:   numerical index of the board to use" << endl;
	cout << "GenICam file: filename of the camera's GenICam XML file (full path)" << endl;
}



// -----------------------------------------------------------------------------
int main(int argc, char *argv[] ){
	using namespace std;

	WaitPowerOn = 1000;
	LinkSpeed = LINK_SPEED_1250;
	NrOfLinks = 1;
	PortMask = 0x01;
	StreamPaketDataSize = 1024;



	if (argc > 1){
		if ((strcmp(argv[1],"-h") == 0) || (strcmp(argv[1],"-help") == 0)){
			displayUsage();
			getchar();
			printf("press any key to exit\n");
			return 0;
		}
	}



	//---------------------------------------------------------------------------
	// Check command line args
	//---------------------------------------------------------------------------
	if (argc > 1){
		if ((strcmp(argv[1],"-h") == 0) || (strcmp(argv[1],"-help") == 0)){
			displayUsage();
			getchar();
			printf("press any key to exit\n");
			return 0;
		}
	}

	//----------------------------------------------------------------------
	// Reset Camera / Power switch of camera at start up 
	//----------------------------------------------------------------------
	// Switch off power
	bool doPowerOffOn = false;
	if ((argc > 1) && (strcmp(argv[1],"0") == 0)){
		doPowerOffOn = false;
	}


	//----------------------------------------------------------------------
	// Auto- detect or configure the cameras in use
	//----------------------------------------------------------------------
	DiscoveryStyle discoveryStyle = Manual;
	if ((argc > 2) && (strcmp(argv[2],"1") == 0)) {
		discoveryStyle = Automatic;
	} else if ((argc > 2) && (strcmp(argv[2],"2") == 0)) {
		discoveryStyle = ConfigFile;
	}


	//----------------------------------------------------------------------
	// Trigger mode : Image Acq. controlled by camera(0) or by grabber (1)
	//----------------------------------------------------------------------
	TriggerMode = 0; // Camera free run
	if ((argc > 3) && (strcmp(argv[3],"1") == 0)){
		// grabber controlled mode
		TriggerMode = 1;
	}


	//----------------------------------------------------------------------
	// Board index: select the frame grabber to use
	//----------------------------------------------------------------------
	bool doAskForBoardIndex = true;
	unsigned int boardIndex = 0; // board #0
	if (argc > 4){
		// let it be given by command line, if more than 1 board is present
		boardIndex = atoi(argv[4]);
		doAskForBoardIndex = false;
	}

	//----------------------------------------------------------------------
	// GenICam file: preset the camera xml file
	//----------------------------------------------------------------------
	if (argc > 5){
		// let it be given from command line 
		strcpy(GenicamXMLFile, argv[5]);
		printf("using GenICam from: %s \n", GenicamXMLFile);
	}
	else{
		printf("using GenICam from: camera\n");
	}
	//---------------------------------------------------------------------------
	// set all camera specific control parameters
	//---------------------------------------------------------------------------
	DoSetCameraControlParameters();


	//---------------------------------------------------------------------------
	// Load applet
	//---------------------------------------------------------------------------
	if (doAskForBoardIndex){
		boardIndex = GetBoardIndex(PN_MICROENABLE5AQ8CXP6D); // search for me5-VQ8 board 
	}
	// uses the acquisition applet for grabbing. 
//	strcpy(AppletFileName, "Acq_SingleCXP6x4AreaGray8.dll"); // placed at the SISODIR\DLL\<Plattform>

	int ret = FG_OK;
	ret = Fg_findApplet(boardIndex,AppletFileName,512);
	if (ret != FG_OK){
		printf("Unable to find applet\n");
		getchar();
		exit(-1);
	}
	else{
		printf("Using %s\n", AppletFileName);
	}


	printf("fg_init %s\n", AppletFileName);
	Fg_Struct* FrameGrabber = Fg_Init(AppletFileName, boardIndex);
	if(FrameGrabber == 0){
		cout << "Error in Fg_Init (" << AppletFileName << "): " << Fg_getLastErrorDescription(0) << endl;
		getchar();
		exit(-1);
	}


	//---------------------------------------------------------------------------
	// create CXP-board handler
	//---------------------------------------------------------------------------
	struct SgcBoardHandle* board_handle=NULL;
	SgcCameraHandle *camera_handle=NULL;
	unsigned int portIndex = 0;

	ret = Sgc_initBoard(FrameGrabber,0,&board_handle);
	if (ret != 0){
		printf("Sgc_initBoard returned %d\n",ret);
		getchar();
		exit(-1);
	}

	//----------------------------------------------------------------------
	// power-On PowerOverCoaxPress and wait a camera specific time for boot up
	//----------------------------------------------------------------------
	if (doPowerOffOn){
		DoPowerOnForVariousCams(FrameGrabber, board_handle, portIndex);
	}

	//----------------------------------------------------------------------
	//Auto- detect or configure the cameras in use
	//----------------------------------------------------------------------
	DoConnectToCamera(board_handle,&camera_handle, portIndex, Manual);

	//----------------------------------------------------------------------
	// Display available / configured cameras
	//----------------------------------------------------------------------
	DoLookForCameras( board_handle);

	//----------------------------------------------------------------------
	// free resources
	//----------------------------------------------------------------------
	// Additional CXP resources
	Sgc_freeBoard(board_handle);
	Fg_FreeGrabber(FrameGrabber);

	printf("Press any key to exit\n");
	getchar();

	return 0;
}
