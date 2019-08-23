// ============================================================================
// === This example shows the usage of applet 'QuadAreaGray8' w/ HW action commands
// === for board 'mE4-VQ4GE'
// ===
// === (c) 2016 Silicon Software GmbH
// ============================================================================

#include <fgrab_prototyp.h>
#include <gbe.h>

#include <cstring>
#include <iostream>

// ### PARAMETERS

// --- global parameters

const char* c_applet = "QuadAreaGray8";

const int c_bpp = 1;

// --- include some helper functions

#include "error-handling-common.h"
#include "error-handling-gbe.h"
#include "helper-common.h"

// ### camera event handler

static int notifyCameraEvent(CameraHandle*, const GbeEventInfo * const eventInfo, const void* self)
{
    switch (eventInfo->type) {
    case GBE_EVENT_ACTIONCOMMAND_TIMEOUT:
        std::cout << "[EVENT] action command timeout: " << eventInfo->data.actioncommand_timeout.actCount << std::endl;
        break;
    default:
        break;
    }

    return 0;
}

// ### MAIN

int main(int argc, char** argv)
{
    // === print header

    std::cout << "============================================================================" << std::endl;
    std::cout << "=== This example shows the usage of applet 'QuadAreaGray8' w/ HW action commands" << std::endl;
    std::cout << "=== for board 'mE4-VQ4GE'" << std::endl;
    std::cout << "===" << std::endl;
    std::cout << "=== (c) 2016 Silicon Software GmbH" << std::endl;
    std::cout << "============================================================================" << std::endl << std::endl;

    // === print usage

    if (argc == 2 && strcmp(argv[1], "help") == 0) {
        std::cout << "Usage: " << argv[0] << " [board] [port] [width] [height] [buffer] [loops]" << std::endl << std::endl;
        return 0;
    }

    // === default values

    unsigned int board = 0;
    unsigned int port = 0;
    unsigned int width = 512;
    unsigned int height = 512;
    unsigned int buffers = 8;
    unsigned int loops = 1000;

    // === read command line arguments

    if (argc >= 2)
        readCommandLineArgument(argv[1], &board);
    if (argc >= 3)
        readCommandLineArgument(argv[2], &port);
    if (argc >= 4)
        readCommandLineArgument(argv[3], &width);
    if (argc >= 5)
        readCommandLineArgument(argv[4], &height);
    if (argc >= 6)
        readCommandLineArgument(argv[5], &buffers);
    if (argc >= 7)
        readCommandLineArgument(argv[6], &loops);

    // === print used values

    std::cout << "Board Id  : " << board << std::endl;
    std::cout << "Port Id   : " << port << std::endl;
    std::cout << "Width     : " << width << std::endl;
    std::cout << "Height    : " << height << std::endl;
    std::cout << "# Buffers : " << buffers << std::endl;
    std::cout << "# Loops   : " << loops << std::endl << std::endl;

    // === local variables

    Fg_Struct* fg = 0;

    BoardHandle* gige_board_handle = 0;
    CameraHandle* gige_camera_handle = 0;

    int errcode = 0;

    const unsigned int c_dma = port;
    bool is_acquisition_started = false;
    
    dma_mem* memory = 0;
    
    int display_id = -1;
    
    uint8_t* display_memory = 0;

    const unsigned int c_width = width;
    const unsigned int c_height = height;

    try {

        Fg_InitLibraries(0);

        // === initialize board and applet

        fg = Fg_Init(c_applet, board);

        if (fg == 0)
            throwFgError(fg, "Fg_Init(...)");

        // === set applet parameters

        unsigned int param_FG_WIDTH = c_width;
        if ((errcode = Fg_setParameterWithType(fg, FG_WIDTH, &param_FG_WIDTH, c_dma, FG_PARAM_TYPE_UINT32_T)) < 0)
            throwFgError(fg, errcode, "Fg_setParameterWithType(FG_WIDTH)");
        
        unsigned int param_FG_HEIGHT = c_height;
        if ((errcode = Fg_setParameterWithType(fg, FG_HEIGHT, &param_FG_HEIGHT, c_dma, FG_PARAM_TYPE_UINT32_T)) < 0)
            throwFgError(fg, errcode, "Fg_setParameterWithType(FG_HEIGHT)");
        
        // === gige initialization sequence

        if ((errcode = Gbe_initBoard(board, 0, &gige_board_handle)) < 0)
            throwGbeError(errcode, "Gbe_initBoard(...)");

        if ((errcode = Gbe_scanNetwork(gige_board_handle, -1, 2000)) < 0)
            throwGbeError(errcode, "Gbe_scanNetwork(...)");

        if ((errcode = Gbe_getFirstCamera(gige_board_handle, port, &gige_camera_handle)) < 0)
            throwGbeError(errcode, "Gbe_getFirstCamera(...)");

        if ((errcode = Gbe_connectCamera(gige_camera_handle)) < 0)
            throwGbeError(errcode, "Gbe_connectCamera(...)");

        if ((errcode = Gbe_setIntegerValue(gige_camera_handle, "Width", c_width)) < 0)
            throwGbeError(errcode, "Gbe_setIntegerValue(\"Width\")");

        if ((errcode = Gbe_setIntegerValue(gige_camera_handle, "Height", c_height)) < 0)
            throwGbeError(errcode, "Gbe_setIntegerValue(\"Height\")");

        if ((errcode = Gbe_setEnumerationValue(gige_camera_handle, "PixelFormat", "Mono8")) < 0)
            throwGbeError(errcode, "Gbe_setEnumerationValue(\"PixelFormat\")");

        if ((errcode = Gbe_setEnumerationValue(gige_camera_handle, "AcquisitionMode", "Continuous")) < 0)
            throwGbeError(errcode, "Gbe_setEnumerationValue(\"AcquisitionMode\")");

        // === initialize action commands (firmware part)

        uint32_t fwacDisable = 0;

        if ((errcode = Gbe_setCameraPropertyWithType(gige_camera_handle, GBE_CAMPROP_FWAC_ENABLE, &fwacDisable, GBE_PROPERTY_TYPE_UINT32)) < 0)
            throwGbeError(errcode, "Gbe_setCameraPropertyWithType(\"GBE_CAMPROP_FWAC_ENABLE\")");

        uint32_t actionDeviceKey = 2;
        uint32_t actionGroupKey = 1;
        uint32_t actionGroupMask = 1;

        if ((errcode = Gbe_setCameraPropertyWithType(gige_camera_handle, GBE_CAMPROP_FWAC_DEVICEKEY, &actionDeviceKey, GBE_PROPERTY_TYPE_UINT32)) < 0)
            throwGbeError(errcode, "Gbe_setCameraPropertyWithType(\"GBE_CAMPROP_FWAC_DEVICEKEY\")");

        if ((errcode = Gbe_setCameraPropertyWithType(gige_camera_handle, GBE_CAMPROP_FWAC_GROUPKEY, &actionGroupKey, GBE_PROPERTY_TYPE_UINT32)) < 0)
            throwGbeError(errcode, "Gbe_setCameraPropertyWithType(\"GBE_CAMPROP_FWAC_GROUPKEY\")");

        if ((errcode = Gbe_setCameraPropertyWithType(gige_camera_handle, GBE_CAMPROP_FWAC_GROUPMASK, &actionGroupMask, GBE_PROPERTY_TYPE_UINT32)) < 0)
            throwGbeError(errcode, "Gbe_setCameraPropertyWithType(\"GBE_CAMPROP_FWAC_GROUPMASK\")");

        uint32_t fwacTimeout = 250000000;

        if ((errcode = Gbe_setCameraPropertyWithType(gige_camera_handle, GBE_CAMPROP_FWAC_TIMEOUT, &fwacTimeout, GBE_PROPERTY_TYPE_UINT32)) < 0)
            throwGbeError(errcode, "Gbe_setCameraPropertyWithType(\"GBE_CAMPROP_FWAC_TIMEOUT\")");

        uint32_t fwacEnable = 1;

        if ((errcode = Gbe_setCameraPropertyWithType(gige_camera_handle, GBE_CAMPROP_FWAC_ENABLE, &fwacEnable, GBE_PROPERTY_TYPE_UINT32)) < 0)
            throwGbeError(errcode, "Gbe_setCameraPropertyWithType(\"GBE_CAMPROP_FWAC_ENABLE\")");

        // === initialize action commands (camera part)

        if ((errcode = Gbe_setIntegerValue(gige_camera_handle, "ActionDeviceKey", actionDeviceKey)) < 0)
            throwGbeError(errcode, "Gbe_setIntegerValue(\"ActionDeviceKey\")");

        if ((errcode = Gbe_setIntegerValue(gige_camera_handle, "ActionSelector", 1)) < 0)
            throwGbeError(errcode, "Gbe_setIntegerValue(\"ActionSelector\")");

        if ((errcode = Gbe_setIntegerValue(gige_camera_handle, "ActionGroupKey", actionGroupKey)) < 0)
            throwGbeError(errcode, "Gbe_setIntegerValue(\"ActionGroupKey\")");

        if ((errcode = Gbe_setIntegerValue(gige_camera_handle, "ActionGroupMask", actionGroupMask)) < 0)
            throwGbeError(errcode, "Gbe_setIntegerValue(\"ActionGroupMask\")");

        // === set camera trigger source to action commands

        if ((errcode = Gbe_setEnumerationValue(gige_camera_handle, "TriggerSelector", "FrameStart")) < 0)
            throwGbeError(errcode, "Gbe_setEnumerationValue(\"TriggerSelector\")");

        if ((errcode = Gbe_setEnumerationValue(gige_camera_handle, "TriggerMode", "On")) < 0)
            throwGbeError(errcode, "Gbe_setEnumerationValue(\"TriggerMode\")");

        if ((errcode = Gbe_setEnumerationValue(gige_camera_handle, "TriggerSource", "Action1")) < 0)
            throwGbeError(errcode, "Gbe_setEnumerationValue(\"TriggerSource\")");

        // === register camera event callback

        if ((errcode = Gbe_registerCameraEventCallback(gige_camera_handle, notifyCameraEvent, 0)) < 0)
            throwGbeError(errcode, "Gbe_registerCameraEventCallback()");

        // === allocate acquisition memory for dma transfers

        memory = Fg_AllocMemEx(fg, (c_width * c_height * c_bpp) * buffers, buffers);

        if (memory == 0)
            throwFgError(fg, "Fg_AllocMemEx(...)");
        
        display_memory = new uint8_t[c_width * c_height];

        if (display_memory == 0)
            throwError("Error allocating display memory");

        // === create display widgets

        display_id = CreateDisplay(8, c_width, c_height);
        SetBufferWidth(display_id, c_width, c_height);

        // === start image acquisition and enable trigger

        if (Fg_AcquireEx(fg, c_dma, GRAB_INFINITE, ACQ_STANDARD, memory) < 0)
            throwFgError(fg, "Fg_AcquireEx(...)");

        if ((errcode = Gbe_startAcquisition(gige_camera_handle)) < 0)
            throwGbeError(errcode, "Gbe_startAcquisition(...)");

        unsigned int param_FG_TRIGGERSTATE = TS_ACTIVE;
        if ((errcode = Fg_setParameterWithType(fg, FG_TRIGGERSTATE, &param_FG_TRIGGERSTATE, c_dma, FG_PARAM_TYPE_UINT32_T)) < 0)
            throwFgError(fg, errcode, "Fg_setParameterWithType(FG_TRIGGERSTATE)");

        is_acquisition_started = true;

        // === enter the image acquisition loop

        frameindex_t next_expected_image = 1;

        for (unsigned int i = 0; i < loops; ++i)
        {
            // --- wait for available images

            frameindex_t last_completely_transferred_image = Fg_getLastPicNumberBlockingEx(fg, next_expected_image, c_dma, 10, memory);

            if (last_completely_transferred_image < 0)
                    throwFgError(fg, "Fg_getLastPicNumberBlockingEx(...)");

            // --- get data pointers for the last available images

            void* dma_data_pointer = Fg_getImagePtrEx(fg, last_completely_transferred_image, c_dma, memory);

            if (dma_data_pointer == 0)
                throwFgError(fg, "Fg_getImagePtrEx(...)");

            uint8_t* image_memory = static_cast<uint8_t*>(dma_data_pointer);

            // --- paint the last image into the display widget

            DrawBuffer(display_id, image_memory, static_cast<int>(last_completely_transferred_image), getDmaString(c_dma).c_str());

            // --- get pending action commands counter and log to console

            uint32_t fwacPending = 0;

            if ((errcode = Gbe_getCameraPropertyWithType(gige_camera_handle, GBE_CAMPROP_FWAC_PENDING, &fwacPending, GBE_PROPERTY_TYPE_UINT32)) < 0)
                throwGbeError(errcode, "Gbe_getCameraPropertyWithType(\"GBE_CAMPROP_FWAC_PENDING\")");

            std::cout << "Frame#: " << last_completely_transferred_image <<  " - Pending Action commands: "<< fwacPending << std::endl;

            // --- update next expected image

            next_expected_image = last_completely_transferred_image + 1;
        }

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    // === cleanup

    if (display_id != -1)
        CloseDisplay(display_id);

    if (is_acquisition_started) {
        unsigned int param_FG_TRIGGERSTATE = TS_ASYNC_STOP;
        Fg_setParameterWithType(fg, FG_TRIGGERSTATE, &param_FG_TRIGGERSTATE, c_dma, FG_PARAM_TYPE_UINT32_T);
        Fg_stopAcquireEx(fg, c_dma, memory, 0);
    }

    if (gige_camera_handle != 0) {
        if (is_acquisition_started)
            Gbe_stopAcquisition(gige_camera_handle);
        Gbe_registerCameraEventCallback(gige_camera_handle, 0, 0);
        Gbe_disconnectCamera(gige_camera_handle);
        Gbe_freeCamera(gige_camera_handle);
    }

    if (gige_board_handle != 0)
        Gbe_freeBoard(gige_board_handle);

    if (memory != 0)
        Fg_FreeMemEx(fg, memory);

    if (display_memory != 0)
        delete [] display_memory;

    if (fg != 0)
        Fg_FreeGrabber(fg);

    Fg_FreeLibraries();

    return 0;
}
