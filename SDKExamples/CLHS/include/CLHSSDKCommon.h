
//-----------------------------------------------------------------------------
//
// CXPSDKCommin.cpp : Common helper functions for using the Silicon Software
//                    siso_genicam libarary  
//
//-----------------------------------------------------------------------------

#ifndef CLHSSDKCOMMON_H
#define CLHSSDKCOMMON_H

#include <siso_genicam.h>


//-----------------------------------------------------------------------------
// define the camera to use
//#define OPTRONIS
//#define ADIMEC_OPAL
//#define ADIMEC_QUARTZ
//#define TOSHIBA_TELI
//#define E2V
//#define MICROTRON
#define PCO
//-----------------------------------------------------------------------------



#define FG_PORT_A 0
#define FG_PORT_B 1
#define FG_PORT_C 2
#define FG_PORT_D 3


#define FG_APPLETPORT_A 0
#define FG_APPLETPORT_B 1
#define FG_APPLETPORT_C 2
#define FG_APPLETPORT_D 3


//-----------------------------------------------------------------
// global variables for camera specific control
//-----------------------------------------------------------------
static unsigned int WaitPowerOn = 10000;         // default idle time after PowerOn
static unsigned int PortMask = 0x03;             // all frame grabber ports are activated
static unsigned int NrOfLinks = 4;               // camera specific no. of links
static unsigned int LinkSpeed = 0;               // bit rate
static unsigned int StreamPaketDataSize = 1024*4;// Stream Packet Size
static char GenicamXMLFile[512];                 // Buffer for filename of camera's Genicam file
static char AppletFileName[512];                 // Filename of the applet to use
static int Width = 9*256;                        // Image width
static int Height = 1024;                        // Image Height
static int PixelFormat = 0x101; // mono8         // Pixel format
static int TriggerMode = 0;                      // FreeRun mode as a default   




//-----------------------------------------------------------------------------
enum DiscoveryStyle { Manual, Automatic, ConfigFile };

#define LINKCONFIGFILE "custom.cxpconfig.xml"  // name of the link configuration file


//-----------------------------------------------------------------
// Define Camera specific parameters at global variables
//-----------------------------------------------------------------
void DoSetCameraControlParameters();

//-----------------------------------------------------------------
// activates power control over CoaxPress and waits a certain
// period of time for booting up the camera
//-----------------------------------------------------------------
void DoPowerOnForVariousCams( Fg_Struct* fg, struct SgcBoardHandle* bh,  unsigned int portIndex);

//----------------------------------------------------------------------
// connects to CXP cameras either by automatic discovery of
// by manual link topology setup
//----------------------------------------------------------------------
//void DoConnectToCamera( struct SgcBoardHandle* bh,  struct SgcCameraHandle** chOut, unsigned int portIndex, DiscoveryStyle discoveryStyle);
int DoConnectToCamera( struct SgcBoardHandle* bh,  struct SgcCameraHandle** chOut, unsigned int portIndex, DiscoveryStyle discoveryStyle);

//----------------------------------------------------------------------
// tries to retrieve a valid GenICam XML either from camera or
// from a given file
//----------------------------------------------------------------------
void DoGenicamConnection( struct SgcCameraHandle* ch, const char* GenicamXMLFile, bool UseCameraInternalXML);

//----------------------------------------------------------------------
// queries the available cameras and displays some camera properties
//----------------------------------------------------------------------
void DoLookForCameras(struct SgcBoardHandle* bh);

//----------------------------------------------------------------------
// sets the image dimension and pixel format at the camera by using the 
// GenICam interface
//----------------------------------------------------------------------
void DoSetImageFormat(struct SgcBoardHandle* bh, struct SgcCameraHandle* ch, int aWidth, int aHeight, int aPixelFormat);

//----------------------------------------------------------------------
// enables a certain trigger mode at the camera by using the GenICam 
// interface.
// TriggerMode 0: "FreeRun" mode
// TriggerMode 1: "GrabberControlled" mode
//----------------------------------------------------------------------
void DoSetAcquisitionControl(struct SgcBoardHandle* bh, struct SgcCameraHandle* ch, int TriggerMode);

//----------------------------------------------------------------------
// Starts the acquisition at the camera by using the GenICam interface
//----------------------------------------------------------------------
void DoStartAcquisition(struct SgcBoardHandle* bh, struct SgcCameraHandle* ch);

//----------------------------------------------------------------------
// Stops the acquisition at the camera by using the GenICam interface
//----------------------------------------------------------------------
void DoStopAcquisition(struct SgcBoardHandle* bh, struct SgcCameraHandle* ch);


//----------------------------------------------------------------------
// Show camera information
//----------------------------------------------------------------------
void ShowCameraInfo(SgcCameraHandle* ch);

// helper functions
bool manualExit();
unsigned int getTimeStamp();



#endif // CLHSSDKCOMMON_H