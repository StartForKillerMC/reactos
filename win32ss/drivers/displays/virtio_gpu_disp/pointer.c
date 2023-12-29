/*
 * PROJECT:     ReactOS VirtIO GPU Display video driver
 * LICENSE:     GPL-2.0-or-later (https://spdx.org/licenses/GPL-2.0-or-later)
 * PURPOSE:     Pointer related functions for Virtio Virgl GPU Display Driver
 * COPYRIGHT:   Copyright 2023 Jesús Sanz del Rey (jesussanz2003@gmail.com)
 */

#include "virtio_gpu_disp.h"
#include "../../miniport/virtiogpump/virtio_gpu_ioctl.h"

/*
 * DrvSetPointerShape
 *
 * Sets the new pointer shape.
 *
 * Status
 *    @unimplemented
 */

ULONG APIENTRY
DrvSetPointerShape(
    IN SURFOBJ *pso,
    IN SURFOBJ *psoMask,
    IN SURFOBJ *psoColor,
    IN XLATEOBJ *pxlo,
    IN LONG xHot,
    IN LONG yHot,
    IN LONG x,
    IN LONG y,
    IN RECTL *prcl,
    IN FLONG fl)
{
    PPDEV ppdev = (PPDEV)pso->dhpdev;

    VirtioPrintDebug("DrvSetPointerShape:\n");

    if(psoMask != NULL && x != -1) {
        if(psoMask->sizlBitmap.cx > 64 || psoMask->sizlBitmap.cy > 128)
            return SPS_DECLINE;

        if(!VirtioGpuDispRegenaretPointerResource(ppdev,psoMask->sizlBitmap.cx, psoMask->sizlBitmap.cy / 2))
            return SPS_ERROR;
        VirtioPrintDebug("width: %d, height: %d, 0x%lx\n", psoMask->sizlBitmap.cx, psoMask->sizlBitmap.cy / 2, psoMask->iBitmapFormat);

        LONG StartX = x - xHot;
        LONG StartY = y - yHot;
        ULONG SizeX = (ULONG)min((LONG)ppdev->PointerWidth, (LONG)ppdev->ScreenWidth - StartX);
        ULONG SizeY = (ULONG)min((LONG)ppdev->PointerHeight, (LONG)ppdev->ScreenHeight - StartY);

        PULONG DrawZone = (PULONG)ppdev->PointerVirtAddress;
        if(psoMask->iBitmapFormat == BMF_1BPP) {
            for(INT x = 0; x < 64; x++) {
                for(INT y = 0; y < 64; y++) {
                    if(x < SizeX && y < SizeY) {
                        INT UsedX = ((x >> 3) << 3) + (7 - (x % 8));
                        INT UsedY = y;
                        UCHAR SubX = UsedX & 0x7;
                        UCHAR ColorAnd = ((PUCHAR)psoMask->pvScan0)[(UsedX >> 3) + UsedY * psoMask->lDelta];
                        UCHAR ColorXor = ((PUCHAR)psoMask->pvScan0)[(UsedX >> 3) + (UsedY + ppdev->PointerHeight) * psoMask->lDelta];
                        DrawZone[x + y * 64] = (ColorAnd & (1 << SubX)) ? 0x0000000 : ((ColorXor & (1 << SubX)) ? 0xFFFFFFFF : 0xFF000000);
                    } else {
                        DrawZone[x + y * 64] = 0;
                    }
                }
            }
        } else {
            for(INT i = 0; i < 64*64; i++) {
                DrawZone[i] = 0xFF00FFFF;
            }
        }
    }

    if(ppdev) {
        VirtioGpuDispTransferResource(ppdev, ppdev->PointerResourceID, 64, 64, ppdev->PointerVirtAddress, 64 * 64 * 4);

        VIDEO_CURSOR VideoCursor;
        VideoCursor.ResourceID = ppdev->PointerResourceID;
        VideoCursor.X = x;
        VideoCursor.Y = y;
        VideoCursor.HotX = xHot;
        VideoCursor.HotY = yHot;

        DWORD ulTemp;
        if (EngDeviceIoControl(ppdev->hDriver, IOCTL_VIRTIOGPU_UPDATE_CURSOR, &VideoCursor,
                                    sizeof(VIDEO_CURSOR), NULL, 0, &ulTemp))
            return FALSE;
    }

    //return SPS_ACCEPT_NOEXCLUDE;
    return EngSetPointerShape(pso, psoMask, psoColor, pxlo, xHot, yHot, x, y, prcl, fl);
}

/*
 * DrvMovePointer
 *
 * Moves the pointer to a new position and ensures that GDI does not interfere
 * with the display of the pointer.
 *
 * Status
 *    @unimplemented
 */

VOID APIENTRY
DrvMovePointer(
    IN SURFOBJ *pso,
    IN LONG x,
    IN LONG y,
    IN RECTL *prcl)
{
    //If i enable this the cursor disappears
    //VirtioGpuDispMoveMouse((PPDEV)pso->dhpdev, x, y);
    EngMovePointer(pso, x, y, prcl);
}

BOOLEAN
VirtioGpuDispRegenaretPointerResource(
    PPDEV ppdev,
    ULONG Width,
    ULONG Height)
{
    //Check if we don't need to regenerate it
    if(Width == ppdev->PointerWidth && Height == ppdev->PointerHeight)
        return TRUE;

    if(ppdev->PointerResourceID != 0)
    {
        //TODO: Destry old resource
        return TRUE;
    }

    VIDEO_CREATE_2D_RESOURCE Create2DResource;
    VIDEO_RESOURCE_ID ResourceID;

    Create2DResource.Width = 64; //Cursor resource is obligated to be 64x64
    Create2DResource.Height = 64;
    Create2DResource.Format = VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM;

    DWORD ulTemp;
    if (EngDeviceIoControl(ppdev->hDriver, IOCTL_VIRTIOGPU_CREATE_2D_RESOURCE, &Create2DResource,
                                sizeof(VIDEO_CREATE_2D_RESOURCE), &ResourceID, sizeof(VIDEO_RESOURCE_ID), &ulTemp))
        return FALSE;

    ppdev->PointerWidth = Width;
    ppdev->PointerHeight = Height;
    ppdev->PointerResourceID = ResourceID.ResourceID;
    ppdev->PointerVirtAddress = EngAllocMem(FL_ZERO_MEMORY, 64 * 64 * 4, ALLOC_TAG);

    VirtioPrintDebug("ResourceID: %d\n", ResourceID.ResourceID);

    return TRUE;
}

BOOLEAN
VirtioGpuDispMoveMouse(
   PPDEV ppdev,
   ULONG x,
   ULONG y)
{
    if(ppdev->PointerResourceID == 0)
        return TRUE;

    VIDEO_CURSOR VideoCursor;
    VideoCursor.ResourceID = ppdev->PointerResourceID;
    VideoCursor.X = x;
    VideoCursor.Y = y;

    DWORD ulTemp;
    if (EngDeviceIoControl(ppdev->hDriver, IOCTL_VIRTIOGPU_MOVE_CURSOR, &VideoCursor,
                                sizeof(VIDEO_CURSOR), NULL, 0, &ulTemp))
        return FALSE;

    return TRUE;
}

BOOLEAN
VirtioGpuDispTransferResource(
   PPDEV ppdev,
   ULONG ResourceID,
   ULONG Width,
   ULONG Height,
   PVOID Buffer,
   ULONG BufferLength)
{

    PVIDEO_TRANSFER_2D VideoFlush = (PVIDEO_TRANSFER_2D)EngAllocMem(FL_ZERO_MEMORY, sizeof(VIDEO_TRANSFER_2D) + BufferLength, ALLOC_TAG);
    VideoFlush->ResourceID = ResourceID;
    VideoFlush->X = VideoFlush->Y = 0;
    VideoFlush->Width = Width;
    VideoFlush->Height = Height;
    VideoFlush->BufferLength = BufferLength;
    RtlCopyMemory((PVOID)((ULONG)VideoFlush + sizeof(VIDEO_TRANSFER_2D)), Buffer, BufferLength);

    DWORD ulTemp;
    if (EngDeviceIoControl(ppdev->hDriver, IOCTL_VIRTIOGPU_RESOURCE_TRANSFER_2D, VideoFlush,
                                sizeof(VIDEO_TRANSFER_2D) + BufferLength, NULL, 0, &ulTemp))
        return FALSE;

    return TRUE;
}