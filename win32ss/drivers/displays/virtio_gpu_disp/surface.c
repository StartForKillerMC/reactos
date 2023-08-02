/*
 * PROJECT:     ReactOS VirtIO GPU Display video driver
 * LICENSE:     GPL-2.0-or-later (https://spdx.org/licenses/GPL-2.0-or-later)
 * PURPOSE:     Surface related functions for Virtio Virgl GPU Display Driver
 * COPYRIGHT:   Copyright 2023 Jesús Sanz del Rey (jesussanz2003@gmail.com)
 */

#include "virtio_gpu_disp.h"

/*
 * DrvEnableSurface
 *
 * Create engine bitmap around frame buffer and set the video mode requested
 * when PDEV was initialized.
 *
 * Status
 *    @implemented
 */

HSURF APIENTRY
DrvEnableSurface(
  DHPDEV dhpdev)
{
    PPDEV ppdev = (PPDEV)dhpdev;

    ULONG ulTemp;
    if (EngDeviceIoControl(ppdev->hDriver, IOCTL_VIDEO_SET_CURRENT_MODE,
                          &(ppdev->ModeIndex), sizeof(ULONG), NULL, 0,
                          &ulTemp))
        return NULL;

    VIDEO_MEMORY VideoMemory;
    VIDEO_MEMORY_INFORMATION VideoMemoryInfo;
    VideoMemory.RequestedVirtualAddress = NULL;
    if (EngDeviceIoControl(ppdev->hDriver, IOCTL_VIDEO_MAP_VIDEO_MEMORY,
                          &VideoMemory, sizeof(VIDEO_MEMORY),
                          &VideoMemoryInfo, sizeof(VIDEO_MEMORY_INFORMATION),
                          &ulTemp))
        return NULL;

    ppdev->ScreenPtr = VideoMemoryInfo.FrameBufferBase;
    ppdev->iDitherFormat = BMF_32BPP;

    SIZEL ScreenSize;
    ScreenSize.cx = ppdev->ScreenWidth;
    ScreenSize.cy = ppdev->ScreenHeight;

    HSURF hSurface = (HSURF)EngCreateBitmap(ScreenSize, ppdev->ScreenDelta, BMF_32BPP,
                        (ppdev->ScreenDelta > 0) ? BMF_TOPDOWN : 0, ppdev->ScreenPtr);
    if(hSurface == NULL)
        return NULL;

    if(!EngAssociateSurface(hSurface, ppdev->hDevEng, 0))
    {
        EngDeleteSurface(hSurface);
        return NULL;
    }

    ppdev->hSurfEng = hSurface;

   return hSurface;
}

/*
 * DrvDisableSurface
 *
 * Used by GDI to notify a driver that the surface created by DrvEnableSurface
 * for the current device is no longer needed.
 *
 * Status
 *    @implemented
 */

VOID APIENTRY
DrvDisableSurface(
   IN DHPDEV dhpdev)
{
   PPDEV ppdev = (PPDEV)dhpdev;

   EngDeleteSurface(ppdev->hSurfEng);
   ppdev->hSurfEng = NULL;

   VIDEO_MEMORY VideoMemory;
   VideoMemory.RequestedVirtualAddress = ((PPDEV)dhpdev)->ScreenPtr;

   DWORD ulTemp;
   EngDeviceIoControl(((PPDEV)dhpdev)->hDriver, IOCTL_VIDEO_UNMAP_VIDEO_MEMORY,
                      &VideoMemory, sizeof(VIDEO_MEMORY), NULL, 0, &ulTemp);
}

/*
 * DrvAssertMode
 *
 * Sets the mode of the specified physical device to either the mode specified
 * when the PDEV was initialized or to the default mode of the hardware.
 *
 * Status
 *    @implemented
 */

BOOL APIENTRY
DrvAssertMode(
   IN DHPDEV dhpdev,
   IN BOOL bEnable)
{
    PPDEV ppdev = (PPDEV)dhpdev;
    ULONG ulTemp;

    if (bEnable)
    {
        if (EngDeviceIoControl(ppdev->hDriver, IOCTL_VIDEO_SET_CURRENT_MODE,
                             &(ppdev->ModeIndex), sizeof(ULONG), NULL, 0,
                             &ulTemp))
            return FALSE;
        return TRUE;
    }
    else
    {
        return !EngDeviceIoControl(ppdev->hDriver, IOCTL_VIDEO_RESET_DEVICE,
                                    NULL, 0, NULL, 0, &ulTemp);
    }
}

BOOL APIENTRY
DrvSetPalette(
   IN DHPDEV dhpdev,
   IN PALOBJ *ppalo,
   IN FLONG fl,
   IN ULONG iStart,
   IN ULONG cColors)
{
    //Unimplemented for this device
    return FALSE;
}