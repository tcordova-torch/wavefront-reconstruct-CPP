//-----------------------------------------------------------------------------
// CLHSFullExample.cpp : Defines the entry point for the console application.
// 
// Sample for using the Silicon Software SiSo_Genicam library in combination
// with the Silicon Software frame grabber SDK in order to grab images from
// a frame grabber connected to a GenICam compliant camera
//
// Arguments 
//  %1: Activate PowerOverCxp:  0 - no Power over framegrabber
//                              1 - Power over framegrabber
//  %2: type of Discovery:      0 - Manual link configuration, 1: Automatic discovery, 
//                              2 - Predefined ConfigFile
//  %3: type of trigger:        0 - free run
//                              1 - grabber controlled
//  %4: board index:            select board in multi board systems
//  %5: GenICam file:           filename of camera's GenICam file
//
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
#include <CLHSSDKCommon.h>


void displayUsage()
{
	using namespace std;
	cout << "---------------------------------" << endl;
	cout << "Silicon Software CLHS sample     " << endl;
	cout << "---------------------------------" << endl;
	cout << "usage:" << endl;
	cout << "<Program> [powerOn] [discovery] [triggerMode] [boardIndex] [GenICam file]" << endl;
	cout << "powerOn:      Do Power switch at startup. 0 power switch off, 1 power switch on" << endl;
	cout << "discovery:    Do Link discovery for link configuration" << endl;
	cout << "0 no discovery (manual configuration 1:1), 1 CoaxPress discover on" << endl;
	cout << "trigger mode: Type of image trigger: 0 free-run mode, 1: grabber controlled mode" << endl;
	cout << "boardIndex:   numerical index of the board to use" << endl;
	cout << "GenICam file: filename of the camera's GenICam XML file (full path)" << endl;
}




//----------------------------------------------------------------------
// sets the image dimesions etc. at the image processing applet
// by using the FGLIB interface 
//----------------------------------------------------------------------
void DoAppletSettings( Fg_Struct* FrameGrabber , uint32_t  width, uint32_t height, int triggerMode, bool useCameraSimulation)
{
	
	// TODO: Adjust the applet parameter accorind to your needs

	int port = 0;

	int Timeout(10000);// 10 sec. to wait for first frame
	Fg_setParameterWithType(FrameGrabber, FG_TIMEOUT, &Timeout, 0, FG_PARAM_TYPE_UINT32_T );

	// this examples uses at both ports same dimensions
	port = 0;
	uint32_t appletWidth = width;
	Fg_setParameterWithType(FrameGrabber, FG_WIDTH, &appletWidth, port, FG_PARAM_TYPE_UINT32_T );

	uint32_t appletHeight = height;
	Fg_setParameterWithType(FrameGrabber, FG_HEIGHT, &appletHeight, port, FG_PARAM_TYPE_UINT32_T );

	port = 1;
	Fg_setParameterWithType(FrameGrabber, FG_WIDTH, &appletWidth, port, FG_PARAM_TYPE_UINT32_T );

	Fg_setParameterWithType(FrameGrabber, FG_HEIGHT, &appletHeight, port, FG_PARAM_TYPE_UINT32_T );


	//.....

	if (useCameraSimulation){
		// enable frame generator / simulator on frame grabber
		int rc = 0;
		uint32_t value;
		int parameterId = 0;
		
		// Port 0
		parameterId = Fg_getParameterIdByName(FrameGrabber, "Device1_Process0_module25_SelectSource");
		if (parameterId > 0){
			value = 1; //
			rc = Fg_setParameterWithType(FrameGrabber, parameterId, &value, 0, FG_PARAM_TYPE_UINT32_T);
			if (rc != 0){
				const char* errorMsg = Fg_getLastErrorDescription(FrameGrabber);
				printf("Error(%d): %s", rc, errorMsg);
			}
		}

		// Port 1
		parameterId = Fg_getParameterIdByName(FrameGrabber, "Device1_Process0_module25_1_SelectSource");
		if (parameterId > 0){
			value = 1; //
			rc = Fg_setParameterWithType(FrameGrabber, parameterId, &value, 1, FG_PARAM_TYPE_UINT32_T);
			if (rc != 0){
				const char* errorMsg = Fg_getLastErrorDescription(FrameGrabber);
				printf("Error(%d): %s", rc, errorMsg);
			}
		}

		// set to 10 bit
		// enable access
		value = 7; // enable rwc-access
		rc = Fg_setParameterWithType(FrameGrabber, FG_GLOBAL_ACCESS, &value, 0, FG_PARAM_TYPE_UINT32_T);
		if (rc != 0){
			const char* errorMsg = Fg_getLastErrorDescription(FrameGrabber);
			printf("Error(%d): %s", rc, errorMsg);
		}

		// image format: 10 bit at Port 0
		port = 0;
		value = 21; // 10 bit
		rc = Fg_setParameterWithType(FrameGrabber, FG_FORMAT, &value, port, FG_PARAM_TYPE_UINT32_T);
		if (rc != 0){
			const char* errorMsg = Fg_getLastErrorDescription(FrameGrabber);
			printf("Error(%d): %s", rc, errorMsg);
		}
		// image format: 10 bit at Port 1
		port= 1;
		value = 21; // 10 bit
		rc = Fg_setParameterWithType(FrameGrabber, FG_FORMAT, &value, port, FG_PARAM_TYPE_UINT32_T);
		if (rc != 0){
			const char* errorMsg = Fg_getLastErrorDescription(FrameGrabber);
			printf("Error(%d): %s", rc, errorMsg);
		}

	}
	else{
		// nothing to do
	}
}



void DoGenicamConnect(SgcBoardHandle* bh, SgcCameraHandle* ch)
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

	if (sisodir){
		strcpy(sisoGenApiDir, sisodir);
		// Add (SISODIR5)\genicam\bin\Win32_i86 to path
		strcat(sisoGenApiDir, "/genicam/bin/Win32_i86");
		strcat(pathVarNew, sisoGenApiDir);
	}

	if (sisodir){
		// Add (SISODIR5)\genicam\bin\Win32_i86\GenApi to path
		strcpy(sisoGenApiDir, sisodir);
		strcat(sisoGenApiDir, "/genicam/bin/Win32_i86/GenApi");
		strcat(pathVarNew, ";");
		strcat(pathVarNew, sisoGenApiDir);
		putenv(pathVarNew);
	}

	//----------------------------------------------------------------------
	// Connect to camera by using GenICam Standard 
	// - either by using the XML file stored at camera
	// - or located on a hard disk
	//----------------------------------------------------------------------
	bool useCameraInternalXML;
	if (strcmp(GenicamXMLFile, "") == 0){
		useCameraInternalXML = true;
	}
	else{
		useCameraInternalXML = false;
	}
	if (useCameraInternalXML){
		printf("Sgc_connectCamera\n");
		rc = Sgc_connectCamera(ch);
		if (rc != 0){
			printf("Sgc_connectCamera(%s) failed\n",GenicamXMLFile);
			printf("Error Code: %d\n",rc);
			printf("Error: %s\n",Sgc_getErrorDescription(rc));
			printf("hit any key to exit");
			getchar();
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
		}
	}


	//----------------------------------------------------------------------
	// Check GenICam Access
	//----------------------------------------------------------------------

	int64_t Width = 0;
	Sgc_getIntegerValue(ch,"Width",&Width);
	printf("\nWidth %d\n",Width);


}




// -----------------------------------------------------------------------------
int main(int argc, char *argv[] ){
	using namespace std;

	WaitPowerOn = 1000;
	LinkSpeed = 0;
	NrOfLinks = 1;
	PortMask = 0x01;
	StreamPaketDataSize = 1024;
	Width  = 1024; 
	Height = 1024;
	TriggerMode = 0; // freeRunning mode



	//-----------------------------------------------------------------------
	// master frame grabber
	//-----------------------------------------------------------------------
	// frame grabber control
	Fg_Struct* MasterFrameGrabber;

	//-----------------------------------------------------------------------
	// TODO:  for customer:
	// Adjust to your local board index depending on link topology
	//-----------------------------------------------------------------------
//	uint32_t MasterBoardIndex = 0;
	uint32_t MasterBoardIndex = 3;
	// camera control
	SgcCameraHandle* MasterCameraHandle = NULL;
	struct SgcBoardHandle* MasterSGCBoardHandle = NULL;
	unsigned int MasterCameraPortIndex = 0;
	dma_mem* MasterDmaMemory = NULL;



	//---------------------------------------------------------------------------
	// Check command line args
	//---------------------------------------------------------------------------
	if (argc > 1){
		if ((strcmp(argv[1],"-h") == 0) || (strcmp(argv[1],"-help") == 0)){
			displayUsage();
			getchar();
			printf("press any key to exit\n", AppletFileName);
			return 0;
		}
	}

	//----------------------------------------------------------------------
	// Auto- detect or configure the cameras in use
	// Automatic:  runs camera detection based on discovery logic
	// Manual:     configure link topology manually
	// ConfigFile: configure link topology based on configuration file 
	//----------------------------------------------------------------------
	DiscoveryStyle discoveryStyle = Automatic;

	//----------------------------------------------------------------------
	// Board index: select the frame grabber to use
	//----------------------------------------------------------------------
	bool doAskForBoardIndex = true;

  //----------------------------------------------------------------------
	// GenICam file: preset the camera GenICam file
	// Use GenICam file from camera or load from hard disk
	//----------------------------------------------------------------------
	if (argc > 5){
		// let it be given from command line 
		strcpy(GenicamXMLFile, argv[5]);
	}
	printf("using GenICam from: %s \n", GenicamXMLFile);


	//---------------------------------------------------------------------------
	// set all camera specific control parameters
	// depending on camera's behaviour
	//---------------------------------------------------------------------------
	DoSetCameraControlParameters();



	//---------------------------------------------------------------------------
	// Find board index and load applet onto the frame grabber
	//---------------------------------------------------------------------------
	// find 1st board
	// boards are fixed in case of 2 frame grabber example
/*
	MasterBoardIndex = 0;
	if (doAskForBoardIndex){
		MasterBoardIndex = GetBoardIndex(PN_MICROENABLE5A2CLHSF2); // search for me5-CLHS board A2 = Prototype
	}
*/
	// Load applet
	// to be placed at the SISODIR5\DLL\<Plattform> for Acquisition applets (DLL based applets)
	// to be placed at the SISODIR5\Hardware Applets\<Plattform> for VisualApplets (HAP based applets)
	// CLHS: <Plattform> = "me5A1-CLHS" of "me5AD8-CLHS-F2"
	
	//-----------------------------------------------------------------------
	// TODO for customer: 
	// adjust applet filename according to the the flashed file
	//-----------------------------------------------------------------------
	if (argc > 1){
		strcpy(AppletFileName, argv[1]);
	}
	else{
    int rc = Fg_findApplet(3,AppletFileName, 255);
//		strcat(AppletFileName, "Standard_Design_CLHS_2x1_RowByRow10_AMD64.hap"); // Standard
	}
	



	//---------------------------------------------------------------------------
	// create board handler for frame grabber control control
	//---------------------------------------------------------------------------
	printf("Fg_Init %d MasterBoardIndex %s\n",MasterBoardIndex ,AppletFileName);
	MasterFrameGrabber = Fg_Init(AppletFileName, MasterBoardIndex);
	if(MasterFrameGrabber == 0){
		cout << "Error in Fg_Init at board: " << MasterBoardIndex << " (" << AppletFileName << "): " << Fg_getLastErrorDescription(0) << endl;
		getchar();
		exit(-1);
	}
	//---------------------------------------------------------------------------
	// create board handler for (GenICam based) camera control
	//---------------------------------------------------------------------------
	int rc = Sgc_initBoardEx(MasterFrameGrabber,0,&MasterSGCBoardHandle,0x02, 0);
	if (rc != 0){
		printf("Sgc_initBoard returned %d\n",rc);
		getchar();
		exit(-1);
	}


//----------------------------------------------------------------------
	//Auto- detect or configure the cameras in use
	//----------------------------------------------------------------------
  bool useFulldiscovery = false;
  if (useFulldiscovery){	
	  rc = 0;
	  int ErrorCount = 0;
	  int NrOrRuns = 1;
	  for (int i = 0; i < NrOrRuns; i++){
		  discoveryStyle = Automatic;//Manual;
  	  int rcMaster = DoConnectToCamera(MasterSGCBoardHandle,&MasterCameraHandle, MasterCameraPortIndex, discoveryStyle);
		  rc = 0;
		  if (rcMaster != 0){
			  ErrorCount++;
		  }
		  printf("NrOfErrors: %d\n", ErrorCount);
		  printf("Run: %d\n", i+1);
	  }
  	
	  printf("Final No Of Discovery Errors: %d\n", ErrorCount);
	  printf("Final No Runs: %d\n", NrOrRuns);

	  int NrOfCamerasAtMasterFG = Sgc_getCameraCount(MasterSGCBoardHandle);
	  printf("No of cameras found at Master FG: %d\n", NrOfCamerasAtMasterFG);


    //----------------------------------------------------------------------
    // Display camera information
    //----------------------------------------------------------------------
    ShowCameraInfo(MasterCameraHandle);

	  //----------------------------------------------------------------------
	  // only for: Display available / configured cameras
	  //----------------------------------------------------------------------
	  DoLookForCameras( MasterSGCBoardHandle);

    //----------------------------------------------------------------------
	  // Create a GenICam handling
	  //----------------------------------------------------------------------
  //	DoGenicamConnect( MasterSGCBoardHandle, MasterCameraHandle);
    rc = Sgc_connectCamera(MasterCameraHandle);
  }
  else{

  }

	//----------------------------------------------------------------------
	// configure camera: image dimension and image format
	//----------------------------------------------------------------------
	//DoSetImageFormat(board_handle, camera_handle, Width, Height, PixelFormat);

	//----------------------------------------------------------------------
	// set applet properties / parameters accordingly
	//----------------------------------------------------------------------
	const bool CameraSimulator = false;
	DoAppletSettings(MasterFrameGrabber, Width, Height, TriggerMode, CameraSimulator);


	//----------------------------------------------------------------------
	// Acquisition start at Camera 
	// 1. step: manual synchronization with microDisplay
	//----------------------------------------------------------------------

	int BitDepth = 10; // 8 * ByteWidth
  int BitsPerPixel = 1;
	uint32_t ImageFormat = 0;
	rc = Fg_getParameterWithType(MasterFrameGrabber,FG_FORMAT, &ImageFormat, 0);
	if (rc == FG_OK){
		switch(ImageFormat) 
		{
    case FG_GRAY:
				BitDepth = 8; 
        BitsPerPixel = 1;
				break;
    case FG_GRAY16:
          BitDepth = 8; 
          BitsPerPixel = 2;
          break;
		case FG_GRAY10: 
				BitDepth = 10;
        BitsPerPixel = 2;
				break;
		default : 
				BitDepth = 10; 
        BitsPerPixel = 2;
				break;
		}
	}
	else{
		BitDepth = 8; 
	}


  // allocate frame memory for 2 DMA-RAM transfers
  // TODO: Berechnung für 10 Bit Packed Width * Height * 16/10 (aufgerundet)
  size_t BufSize((size_t) Width * Height * BitsPerPixel);
  const unsigned int c_MemoryBuffers(16);
  MasterDmaMemory = Fg_AllocMemEx(MasterFrameGrabber, BufSize * c_MemoryBuffers, c_MemoryBuffers);



	// Display for Master and slave frames
	int MasterDisplayId = CreateDisplay(BitDepth, Width, Height);
	SetBufferWidth(MasterDisplayId, Width, Height);



	// Master
	rc = Fg_AcquireEx(MasterFrameGrabber, 0, GRAB_INFINITE, ACQ_STANDARD, MasterDmaMemory);
	if(rc != 0){
		cout << "Error in Fg_AcquireEx for channel 0: " << Fg_getErrorDescription(MasterFrameGrabber, rc) << endl;
		Fg_FreeMemEx(MasterFrameGrabber, MasterDmaMemory);
		CloseDisplay(MasterDisplayId);
		Fg_FreeGrabber(MasterFrameGrabber);
		exit(-1);
	}
	Sleep(100);
	int Status(0);



	// 2. step: Start Acquisition at camera
	// Start Master channel only
  if (useFulldiscovery){
	  DoStartAcquisition(MasterSGCBoardHandle, MasterCameraHandle);
  }


	printf("Acquisition at camera started\n");
	printf("Press any key to continue\n");
  getchar();

	//----------------------------------------------------------------------
	// grabbing loop
	//----------------------------------------------------------------------
#ifdef _WIN32
	unsigned long tc0,tc1;
	tc0 = GetTickCount();
	frameindex_t FrameNo = 0;
	for(int i = 1; i <= 10000 && !_kbhit(); i++){
#else
	for(i = 1; i <= 1000; i++){
#endif
		FrameNo = Fg_getLastPicNumberBlockingEx(MasterFrameGrabber, FrameNo + 1, 0, 10, MasterDmaMemory);
		if(FrameNo < 0){
			cout << "Error: " << Status << endl;
		}else{
		}


		void* MasterImageBuffer = Fg_getImagePtrEx(MasterFrameGrabber, FrameNo, 0, MasterDmaMemory);
		if(MasterImageBuffer == 0){
			break;
		}else{
			if(FrameNo % 100 != 0){
#ifdef _WIN32
				tc1 = GetTickCount();
#endif
				DrawBuffer(MasterDisplayId, MasterImageBuffer, FrameNo, 0);
#ifdef _WIN32
				printf("FPS = %d\n",(Status*1000)/(tc1-tc0+1));
#endif
				int Value(Status);
        Value = FrameNo;
				Status = Fg_getParameterWithType(MasterFrameGrabber, FG_TRANSFER_LEN, &Value, 0, FG_PARAM_TYPE_UINT32_T );
  		  cout << "\rGrabbed image " << FrameNo << " len = " << Value << endl;
        if(Status < 0){
          cout << "\rGrabbed image " << FrameNo << " len = " << Value << endl;
        }

			}
		}
	}
	cout << endl;


	//----------------------------------------------------------------------
	// terminate acquisition
	//----------------------------------------------------------------------
	// Stop camera at master channel
	DoStopAcquisition(MasterSGCBoardHandle, MasterCameraHandle);

	// Stop DMA on both grabber
	Fg_stopAcquireEx(MasterFrameGrabber, 0, MasterDmaMemory, 0);


	//----------------------------------------------------------------------
	// free resources
	//----------------------------------------------------------------------
	// Additional CXP resources
	Sgc_freeBoard(MasterSGCBoardHandle);

	// standard SDK

	Fg_FreeMemEx(MasterFrameGrabber, MasterDmaMemory);
	CloseDisplay(MasterDisplayId);
	Fg_FreeGrabber(MasterFrameGrabber);

	printf("Press any key to exit\n");
	getchar();


	return 0;
}
