////////////////////////////////////////////////////////////////////////////
// ME4 Frame grabber example
//
//
//
// File:	AcqBlock.cpp
//
// Copyrights by Silicon Software 2002-2010
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
		"MediumAreaGray",
		"MediumAreaRGB",
		"MediumLineGray",
		"MediumLineRGB",
		""
};

const char *dll_list_me3xxl[] = {
		"DualAreaGray12XXL",
		"DualAreaRGB36XXL",
		"DualLineGray12XXL",
		"DualLineRGB36XXL",
		"MediumAreaGray12XXL",
		"MediumAreaRGBXXL",
		"MediumLineGray12XXL",
		"MediumLineRGBXXL",
		""
};

const char *dll_list_me4_dual[] = {
		"DualAreaGray16",
		"DualAreaRGB48",
		"DualLineGray16",
		"DualLineRGB30",
		"MediumAreaGray16",
		"MediumAreaRGB36",
		"MediumLineGray16",
		"MediumLineRGB36",
		""
};

const char *dll_list_me4_single[] = {
	"SingleAreaGray16",
	"SingleAreaRGB48",
	"SingleLineGray16",
	"SingleLineRGB48",
	""
};



const char *dll_list_me4_VD_AD4_dual[] = {
	"DualAreaGray16",
	"DualAreaRGB48",
	"DualLineGray16",
	"DualLineRGB30",
	"MediumAreaGray16",
	"MediumAreaRGB36",
	"MediumLineGray16",
	"MediumLineRGB36",
	"Acq_BaseAreaBayer12",
	"Acq_DualBaseAreaBayer8",
	"Acq_DualBaseAreaGray12",
	"Acq_DualBaseAreaRGB24",
	"Acq_MediumAreaBayer12",
	"Acq_MediumAreaGray12",
	"Acq_MediumAreaRGB36",
	"Acq_FullAreaBayer8",
	"Acq_FullAreaGray8",
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


	int nr_of_buffer	=	8;			// Number of memory buffer
	int nBoard			=	selectBoardDialog();			// Board Number
	int nCamPort		=	PORT_A;		// Port (PORT_A / PORT_B)
	int MaxPics			=	100;		// Number of images to grab
	int status = 0;
	Fg_Struct *fg;


	const char *dllName = NULL;

	int boardType = 0;

	boardType = Fg_getBoardType(nBoard);

	dllName = selectDll(boardType, dll_list_me3, dll_list_me3xxl, dll_list_me4_dual, dll_list_me4_single, dll_list_me4_VD_AD4_dual, NULL);


	// Initialization of the microEnable frame grabber
	if((fg = Fg_Init(dllName,nBoard)) == NULL) {
		status = ErrorMessageWait(fg);
		return status;
	}
	fprintf(stdout,"Init Grabber ok\n");

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
	dma_mem * pMem0;
	if((pMem0 = Fg_AllocMemEx(fg,totalBufSize,nr_of_buffer)) == NULL){
		status = ErrorMessageWait(fg);
		return status;
	} else {
		fprintf(stdout,"%d framebuffer allocated for port %d ok\n",nr_of_buffer,nCamPort);
	}

	// Creating a display window for image output
	int Bits = getNoOfBitsFromImageFormat(format);
	int nId = ::CreateDisplay(Bits,width,height);
	SetBufferWidth(nId,width,height);
	if((Fg_AcquireEx(fg,nCamPort,GRAB_INFINITE,ACQ_BLOCK,pMem0)) < 0){
		status = ErrorMessageWait(fg);
		return status;
	}

	// ====================================================
	// MAIN LOOP
    int Pics=1;
	frameindex_t nbi, bufNr;

	while(Pics <= MaxPics){
		bufNr = Fg_getImageEx(fg,SEL_ACT_IMAGE,0,nCamPort,10,pMem0);
		if ((bufNr <0) || (Pics >= MaxPics)){
			fprintf(stdout,"stop: Pics: %d,bufNr: %d\n",Pics,(int)bufNr);
			Fg_stopAcquireEx(fg,nCamPort,pMem0,0);
			Fg_FreeMemEx(fg, pMem0);
			Fg_FreeGrabber(fg);
			CloseDisplay(nId);
			return (int)bufNr;
		}
		if (bufNr > 0) {
			// Draw the image
			::DrawBuffer(nId,Fg_getImagePtrEx(fg,bufNr,0,pMem0),(int)bufNr,"");

			// Read the number of blocked images
			nbi = Fg_getStatusEx(fg, NUMBER_OF_BLOCKED_IMAGES, 0, nCamPort, pMem0);

			fprintf(stdout,"nbi: %d\n",(int)nbi);

			// Unblock the buffer
			Fg_setStatusEx(fg,FG_UNBLOCK,bufNr,nCamPort,pMem0);
		}
		Pics++;
	}

	// ====================================================


	// Freeing the grabber resource
	Fg_stopAcquireEx(fg,nCamPort,pMem0,0);
	Fg_FreeMemEx(fg, pMem0);
	Fg_FreeGrabber(fg);

	CloseDisplay(nId);

	return FG_OK;
}
