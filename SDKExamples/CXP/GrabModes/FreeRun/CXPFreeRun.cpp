//-----------------------------------------------------------------------------
// CXPFreeRun.cpp : Defines the entry point for the console application.
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



//----------------------------------------------------------------------
// sets the image dimesions etc. at the image processing applet
// by using the FGLIB interface 
//----------------------------------------------------------------------
void DoAppletSettings( Fg_Struct* FrameGrabber , uint32_t  width, uint32_t height, int triggerMode)
{
	int Timeout(100000);// 10 sec. to wait for first frame
	Fg_setParameterWithType(FrameGrabber, FG_TIMEOUT, &Timeout, 0, FG_PARAM_TYPE_UINT32_T );

	uint32_t appletWidth = width;
	Fg_setParameterWithType(FrameGrabber, FG_WIDTH, &appletWidth, 0, FG_PARAM_TYPE_UINT32_T );

	uint32_t appletHeight = height;
	Fg_setParameterWithType(FrameGrabber, FG_HEIGHT, &appletHeight, 0, FG_PARAM_TYPE_UINT32_T );

	//.....

	// Select type of trigger
	if (triggerMode == 1){
		// settings for grabber controlled mode

		int fg_rc = 0;
		double linePeriod = 100;
		fg_rc = Fg_setParameterWithType(FrameGrabber, FG_LINEPERIOD, &linePeriod, 0, FG_PARAM_TYPE_DOUBLE );
		if (fg_rc != 0){

		}

		unsigned int triggerMode = GRABBER_CONTROLLED;
		fg_rc = Fg_setParameterWithType(FrameGrabber, FG_LINETRIGGERMODE, &triggerMode, 0, FG_PARAM_TYPE_UINT32_T );
		if (fg_rc != 0){

		}

		unsigned int exsyncPolarity = FG_ONE;
		fg_rc = Fg_setParameterWithType(FrameGrabber, FG_EXSYNCPOLARITY, &exsyncPolarity, 0, FG_PARAM_TYPE_UINT32_T );
		if (fg_rc != 0){

		}



	}
	else{
		// settings for grabber free run mode
		int TriggerState(2);// TS_SYNC_STOP
		Fg_setParameterWithType(FrameGrabber, FG_TRIGGERSTATE, &TriggerState, 0, FG_PARAM_TYPE_UINT32_T );
	}

}





// -----------------------------------------------------------------------------
int main(int argc, char *argv[] ){
	using namespace std;

	WaitPowerOn = 1000;
	LinkSpeed = LINK_SPEED_1250;
	NrOfLinks = 1;
	PortMask = 0x01;
	StreamPaketDataSize = 1024;
	Width  = 1024; 
	Height = 1024;

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
//	strcpy(AppletFileName, "Acq_SingleCXP6x4LineGray8.dll"); // placed at the SISODIR\DLL\<Plattform>
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
	if ((argc > 1) && (strcmp(argv[1],"0") == 0)){
		doPowerOffOn = false;
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
	DoConnectToCamera(board_handle,&camera_handle, portIndex, discoveryStyle);

	//----------------------------------------------------------------------
	// Display available / configured cameras
	//----------------------------------------------------------------------
	DoLookForCameras( board_handle);


	//----------------------------------------------------------------------
	// Create a GenICam handling
	//----------------------------------------------------------------------

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


	//----------------------------------------------------------------------
	// Display camera properties defined by GenICam
	//----------------------------------------------------------------------

	// Connect by using camera internal XML
	DoGenicamConnection(camera_handle, "e:\\e2v_ELIIXAC4MCP1605_1.0.0.xml", false);

	const char *DeviceVendorNameStr=NULL;
	Sgc_getStringValue(camera_handle,"DeviceVendorName",&DeviceVendorNameStr);
	printf("\nDeviceVendorName %s\n",DeviceVendorNameStr);

	const char *DeviceModelNameStr=NULL;
	Sgc_getStringValue(camera_handle,"DeviceModelName",&DeviceModelNameStr);
	printf("DeviceModelNameStr %s\n",DeviceModelNameStr);

	const char *DeviceManufacturerInfo=NULL;
	Sgc_getStringValue(camera_handle,"DeviceManufacturerInfo",&DeviceManufacturerInfo);
	printf("\nDeviceManufacturerInfo%s\n",DeviceManufacturerInfo);

	const char *DeviceVersion=NULL;
	Sgc_getStringValue(camera_handle,"DeviceVersion",&DeviceVersion);
	printf("DeviceVersion%s\n",DeviceVersion);

	const char *DeviceFirmwareVersion=NULL;
	Sgc_getStringValue(camera_handle,"DeviceFirmwareVersion",&DeviceFirmwareVersion);
	printf("DeviceFirmwareVersion%s\n",DeviceFirmwareVersion);

	const char *DeviceIdStr=NULL;
	Sgc_getStringValue(camera_handle,"DeviceID",&DeviceIdStr);
	printf("DeviceID %s\n\n",DeviceIdStr);


	int64_t CamWidth=0;
	Sgc_getIntegerValue(camera_handle,"Width",&CamWidth);
	printf("Width of Camera %ld\n",CamWidth);

	int64_t CamHeight=0;
	Sgc_getIntegerValue(camera_handle,"Height",&CamHeight);
	printf("Height of Camera %ld\n",CamHeight);


	int64_t streamPacketSize=0;
	Sgc_getIntegerValue(camera_handle,"StreamPacketSize",&streamPacketSize);
	printf("Stream Packet Size %ld\n",streamPacketSize);
	streamPacketSize = 4096;
	Sgc_setIntegerValue(camera_handle,"StreamPacketSize",streamPacketSize);
	printf("Stream Packet set to %ld\n",streamPacketSize);
	streamPacketSize = 0;
	Sgc_getIntegerValue(camera_handle,"StreamPacketSize",&streamPacketSize);
	printf("Stream Packet Size %ld\n",streamPacketSize);

	//----------------------------------------------------------------------
	// configure image dimension and image format according to the cameras
	//----------------------------------------------------------------------
	DoSetImageFormat(board_handle, camera_handle, Width, Height, PixelFormat);
	DoSetAcquisitionControl(board_handle, camera_handle, TriggerMode /* camera in free running mode*/);


	////////////////////////////////////////////////////////////////
	// Standard SiSo-SDK / sample
	//----------------------------------------------------------------------
	//----------------------------------------------------------------------
	unsigned int ByteWidth(1);
	size_t BufSize((size_t) Width * Height * ByteWidth);
	const unsigned int c_MemoryBuffers(16);
	dma_mem* DmaMemory = Fg_AllocMemEx(FrameGrabber, BufSize * c_MemoryBuffers, c_MemoryBuffers);


	//----------------------------------------------------------------------
	// set applet properties
	//----------------------------------------------------------------------
	DoAppletSettings(FrameGrabber, Width, Height, TriggerMode /* camera in free running mode*/);


	//----------------------------------------------------------------------
	// Acquisition start at Camera 
	// 1. step: manual synchronization with microDisplay
	//----------------------------------------------------------------------

	int DisplayId(CreateDisplay(8 * ByteWidth, Width, Height));
	SetBufferWidth(DisplayId, Width, Height);

	// 1. step ACQ-Start at frame grabber
	int i(Fg_AcquireEx(FrameGrabber, 0, -1, ACQ_STANDARD, DmaMemory));
	if(i != 0){
		cout << "Error in Fg_AcquireEx for channel 0: " << Fg_getErrorDescription(FrameGrabber, i) << endl;
		Fg_FreeMemEx(FrameGrabber, DmaMemory);
		CloseDisplay(DisplayId);
		Fg_FreeGrabber(FrameGrabber);
		exit(-1);
	}
	Sleep(100);
	frameindex_t Status(0);


	// 2. step: Start Acquisition at camera
	DoStartAcquisition(board_handle, camera_handle);


	printf("Acquisition at camera started\n");
	printf("Press any key to continue\n");

	//----------------------------------------------------------------------
	// grabbing loop
	//----------------------------------------------------------------------
	const int NrOfImages2Grab = 100000;
	unsigned long tc0,tc1;
	tc0 = getTimeStamp();
	for(i = 1; i <= NrOfImages2Grab; i++){
		if (manualExit()){
			break;
		}
		Status = Fg_getLastPicNumberBlockingEx(FrameGrabber, Status + 1, 0, 10, DmaMemory);
		if(Status < 0){
			cout << "Error: " << Status << endl;
		}else{
		}

		void* ImageBuffer = Fg_getImagePtrEx(FrameGrabber, Status, 0, DmaMemory);
		if(ImageBuffer == 0){
//			break;
		}else{
			if(Status % 100 != 0){
				tc1 = getTimeStamp();
				DrawBuffer(DisplayId, ImageBuffer, (int) Status, 0);
				printf("FPS = %lu\n",(Status*1000)/(tc1-tc0+1));
				int Value((int)Status);
				Fg_getParameterWithType(FrameGrabber, FG_TRANSFER_LEN, &Value, 0, FG_PARAM_TYPE_UINT32_T );
				cout << "\rGrabbed image " << Status << " len = " << Value;
			}
		}
	}
	cout << endl;


	//----------------------------------------------------------------------
	// terminate acquisition
	//----------------------------------------------------------------------
	DoStopAcquisition(board_handle, camera_handle);

	Fg_stopAcquireEx(FrameGrabber, 0, DmaMemory, 0);


	//----------------------------------------------------------------------
	// free resources
	//----------------------------------------------------------------------
	// Additional CXP resources
	Sgc_freeBoard(board_handle);

	// standard SDK
	Fg_FreeMemEx(FrameGrabber, DmaMemory);
	CloseDisplay(DisplayId);
	Fg_FreeGrabber(FrameGrabber);

	printf("Press any key to exit\n");
	getchar();


	return 0;
}
