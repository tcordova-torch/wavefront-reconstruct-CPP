////////////////////////////////////////////////////////////////////////////
// ME4 Frame grabber example
//
//
// File:	Slave.cpp
//
// Example for using Master/Slave Mode This project demonstrates the slave.
// 
// The example loads an applet onto the frame grabber in master mode and grabs
// images in Standard acquisition mode on port B. The corresponding master
// example loads the applet in slave mode and grabs on port A. To run this
// example, the frame grabber needs to by initialized in master mode first.
// To do so, run the according master example and then start this project.
//
// Copyrights by Silicon Software 2002-2010
//
////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <time.h>

#include "board_and_dll_chooser.h"

#include <fgrab_struct.h>
#include <fgrab_prototyp.h>
#include <fgrab_define.h>
#include <SisoDisplay.h>

int ErrorMessage(Fg_Struct *fg)
{
	int			error	= Fg_getLastErrorNumber(fg);
	const char*	err_str = Fg_getLastErrorDescription(fg);
	fprintf(stderr,"Error: %d : %s\n",error,err_str);
	return error;
}

int ErrorMessageWait(Fg_Struct *fg)
{
	int			error	= ErrorMessage(fg);
	printf (" ... press ENTER to continue\n");
	getchar();
	return error;
}

const char *dll_list_me3[] = {
		"DualAreaGray",
		"DualAreaRGB",
		"DualLineGray",
		"DualLineRGB",
		""
};

const char *dll_list_me3xxl[] = {
		"DualAreaGray12XXL",
		"DualAreaRGB36XXL",
		"DualLineGray12XXL",
		"DualLineRGB36XXL",
		""
};


const char *dll_list_me4_dual[] = {
		"DualAreaGray16",
		"DualAreaRGB48",
		"DualLineGray16",
		"DualLineRGB30",
		"Baumer_DualLineGray16TrgInsert"
};

const char *dll_list_me4_single[] = {
		""
};

const char *dll_list_me4_VD_AD4_dual[] = {
	"DualAreaGray16",
	"DualAreaRGB48",
	"DualLineGray16",
	"DualLineRGB30",
	"Acq_DualBaseAreaBayer8",
	"Acq_DualBaseAreaGray12",
	"Acq_DualBaseAreaRGB24",
	""
};

// returns the bit with according to the selected image format
int getNoOfBitsFromImageFormat(const int format)
{
	int Bits = 8;
	switch(format){
case FG_GRAY:
	Bits = 8;
	break;
case FG_GRAY16:
	Bits = 16;
	break;
case FG_COL24:
case FG_COL48:
	Bits = 24;
	break;
default:
	Bits = 8;
	break;
	};
	return Bits;
}


////////////////////////////////////////////////////////////////////////
// Main function
//

int main(int argc, char* argv[], char* envp[]){

	int nr_of_buffer  = 8;      // Number of memory buffer
	int nBoard        = 0;      // Board Number
	int MaxPics       = 100;    // Number of images to grab
	int boardType     = 0;
	Fg_Struct *fg     = NULL;
	int status = 0;

	// the slave initializes the frame grabber and grabs on port A, while the
	// slave process grabs on Port B. both processes cannot grab on the same
	int nCamPort      = PORT_B; // Port (PORT_A / PORT_B)

	// select frame grabber to be uses
	nBoard = selectBoardDialog();


	// select applet to be used
	const char *dllName = NULL;
	boardType = Fg_getBoardType(nBoard);
	dllName = selectDll(boardType, dll_list_me3, dll_list_me3xxl, dll_list_me4_dual, dll_list_me4_single, dll_list_me4_VD_AD4_dual, NULL);

	// Initialization of the microEnable frame grabber
	// Initialize the frame grabber as a slave
	// to have success, start a different process and initialize the frame grabber
	// in master mode.
	// Pls. note, that not all applet are capable for master / slave mode.
	// Please refer to according applet documentation.
	int slaveMode = 1;
	if((fg = Fg_InitEx(dllName,nBoard, slaveMode)) == NULL) {
		status = ErrorMessageWait(fg);
		return status;
	}
	fprintf(stdout,"Init slave frame grabber ok\n");

	// Setting the image size
	int width	= 512;
	int height	= 512;
	if (Fg_setParameter(fg,FG_WIDTH,&width,nCamPort) < 0) {
		status = ErrorMessageWait(fg);
		return status;
	}
	if (Fg_setParameter(fg,FG_HEIGHT,&height,nCamPort) < 0) {
		status = ErrorMessageWait(fg);
		return status;
	}

	int bitAlignment = FG_LEFT_ALIGNED;
	if (Fg_setParameter(fg,FG_BITALIGNMENT,&bitAlignment,nCamPort) < 0) {
		status = ErrorMessageWait(fg);
		return status;
	}


	fprintf(stdout,"Set Image Size on port %d (w: %d,h: %d) ok\n",nCamPort,width,height);


	// Get bit depth
	int format;
	Fg_getParameter(fg,FG_FORMAT,&format,nCamPort);
	size_t bytesPerPixel = 1;
	switch(format){
	case FG_GRAY:	bytesPerPixel = 1; break;
	case FG_GRAY16:	bytesPerPixel = 2; break;
	case FG_COL24:	bytesPerPixel = 3; break;
	case FG_COL32:	bytesPerPixel = 4; break;
	case FG_COL30:	bytesPerPixel = 5; break;
	case FG_COL48:	bytesPerPixel = 6; break;
	}
    
    // Calculate buffer size (careful to avoid integer arithmetic overflows!) and allocate memory
	size_t totalBufSize = (size_t) width * height * nr_of_buffer * bytesPerPixel;
	dma_mem *pMem0;
	if((pMem0 = Fg_AllocMemEx(fg,totalBufSize,nr_of_buffer)) == NULL){
		status = ErrorMessageWait(fg);
		return status;
	} else {
		fprintf(stdout,"%d frame buffer allocated for port %d ok\n",nr_of_buffer,nCamPort);
	}

	// Creating a display window for image output
	int Bits = getNoOfBitsFromImageFormat(format);

	int nId = ::CreateDisplay(Bits,width,height);
	SetBufferWidth(nId,width,height);
	if((Fg_AcquireEx(fg,nCamPort,GRAB_INFINITE,ACQ_STANDARD,pMem0)) < 0){
		status = ErrorMessageWait(fg);
		return status;
	}
	// ====================================================
	// MAIN LOOP
	frameindex_t lastPicNr = 0;
	while((lastPicNr = Fg_getLastPicNumberBlockingEx(fg,lastPicNr+1,nCamPort,10,pMem0))< MaxPics) {
		if(lastPicNr <0){
			status = ErrorMessageWait(fg);
			Fg_stopAcquireEx(fg,nCamPort,pMem0,0);
			Fg_FreeMemEx(fg, pMem0);
			Fg_FreeGrabber(fg);
			CloseDisplay(nId);
			return status;
		}
		::DrawBuffer(nId,Fg_getImagePtrEx(fg,lastPicNr,0,pMem0),(int)lastPicNr,"");
	}

	// ====================================================
	// Freeing the grabber resource
	Fg_stopAcquireEx(fg,nCamPort,pMem0,0);
	Fg_FreeMemEx(fg, pMem0);
	Fg_FreeGrabber(fg);

	CloseDisplay(nId);

	return FG_OK;
}
