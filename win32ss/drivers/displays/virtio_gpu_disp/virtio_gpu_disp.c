/*
 * PROJECT:     ReactOS VirtIO GPU Display video driver
 * LICENSE:     GPL-2.0-or-later (https://spdx.org/licenses/GPL-2.0-or-later)
 * PURPOSE:     Simple Display driver for Virtio Virgl GPU
 * COPYRIGHT:   Copyright 2023 Jesús Sanz del Rey (jesussanz2003@gmail.com)
 */

#include "virtio_gpu_disp.h"
#include "../../miniport/virtiogpump/virtio_gpu_ioctl.h"
#include "virtio_gpu_esc.h"
#include <wchar.h>

static DRVFN DrvFunctionTable[] =
{
    {INDEX_DrvEnablePDEV, (PFN)DrvEnablePDEV},
    {INDEX_DrvCompletePDEV, (PFN)DrvCompletePDEV},
    {INDEX_DrvDisablePDEV, (PFN)DrvDisablePDEV},
    {INDEX_DrvGetModes, (PFN)DrvGetModes},
    {INDEX_DrvEnableSurface, (PFN)DrvEnableSurface},
    {INDEX_DrvDisableSurface, (PFN)DrvDisableSurface},
    {INDEX_DrvAssertMode, (PFN)DrvAssertMode},
    {INDEX_DrvSetPalette, (PFN)DrvSetPalette},
    {INDEX_DrvSetPointerShape, (PFN)DrvSetPointerShape},
    {INDEX_DrvMovePointer, (PFN)DrvMovePointer},
    {INDEX_DrvEscape, (PFN)DrvEscape}
};

/*
 * DrvEnableDriver
 *
 * Initial driver entry point exported by the driver DLL. It fills in a
 * DRVENABLEDATA structure with the driver's DDI version number and the
 * calling addresses of all DDI functions supported by the driver.
 *
 * Status
 *    @implemented
 */

BOOL APIENTRY
DrvEnableDriver(
   ULONG iEngineVersion,
   ULONG cj,
   PDRVENABLEDATA pded)
{
    if(cj >= sizeof(DRVENABLEDATA))
    {
        pded->c = sizeof(DrvFunctionTable) / sizeof(DRVFN);
        pded->pdrvfn = DrvFunctionTable;
        pded->iDriverVersion = DDI_DRIVER_VERSION_NT5;
        return TRUE;
    }
    else
        return FALSE;
}

DHPDEV APIENTRY
DrvEnablePDEV(
   IN DEVMODEW *pdm,
   IN LPWSTR pwszLogAddress,
   IN ULONG cPat,
   OUT HSURF *phsurfPatterns,
   IN ULONG cjCaps,
   OUT ULONG *pdevcaps,
   IN ULONG cjDevInfo,
   OUT DEVINFO *pdi,
   IN HDEV hdev,
   IN LPWSTR pwszDeviceName,
   IN HANDLE hDriver)
{
    PPDEV PPDev = EngAllocMem(FL_ZERO_MEMORY, sizeof(PDEV), ALLOC_TAG);
    if(PPDev == NULL)
        return NULL;

    PPDev->hDriver = hDriver;

    GDIINFO GdiInfo;
    DEVINFO DevInfo;
    if(!IntInitScreenInfo(PPDev, pdm, &GdiInfo, &DevInfo))
    {
        EngFreeMem(PPDev);
        return NULL;
    }

    PPDev->DefaultPalette = DevInfo.hpalDefault =
        EngCreatePalette(PAL_BITFIELDS, 0, NULL,
            PPDev->RedMask, PPDev->GreenMask, PPDev->BlueMask);
    if(PPDev->DefaultPalette == NULL)
    {
        EngFreeMem(PPDev);
        return NULL;
    }

    memcpy(pdi, &DevInfo, min(sizeof(DEVINFO), cjDevInfo));
    memcpy(pdevcaps, &GdiInfo, min(sizeof(GDIINFO), cjCaps));

    return (DHPDEV)PPDev;
}

/*
 * DrvCompletePDEV
 *
 * Stores the GDI handle (hdev) of the physical device in dhpdev. The driver
 * should retain this handle for use when calling GDI services.
 *
 * Status
 *    @implemented
 */

VOID APIENTRY
DrvCompletePDEV(
   IN DHPDEV dhpdev,
   IN HDEV hdev)
{
    ((PPDEV)dhpdev)->hDevEng = hdev;
}

/*
 * DrvDisablePDEV
 *
 * Release the resources allocated in DrvEnablePDEV.  If a surface has been
 * enabled DrvDisableSurface will have already been called.
 *
 * Status
 *    @implemented
 */

VOID APIENTRY
DrvDisablePDEV(
   IN DHPDEV dhpdev)
{
    if (((PPDEV)dhpdev)->DefaultPalette)
    {
        EngDeletePalette(((PPDEV)dhpdev)->DefaultPalette);
    }

    EngFreeMem(dhpdev);
}

typedef struct
{
    DWORD dwVersion;
    DWORD dwDriverVersion;
    WCHAR szDriverName[256];
} OPENGL_INFO, *POPENGL_INFO;

ULONG APIENTRY
DrvEscape(
    IN SURFOBJ *pso,
    IN ULONG iEsc,
    IN ULONG cjIn,
    IN PVOID pvIn,
    IN ULONG cjOut,
    OUT PVOID pvOut)
{
    PPDEV ppdev = (PPDEV)pso->dhpdev;
    ULONG ulTemp;

    switch(iEsc) {
        case QUERYESCSUPPORT:
        {
            if(cjIn < sizeof(DWORD)) return FALSE;
            DWORD escCode = *(PDWORD)pvIn;

            switch(escCode) {
                case OPENGL_GETINFO:
                    return sizeof(DWORD);
                default:
                    return FALSE;
            }
        }

        case OPENGL_GETINFO:
        {
            if(cjOut < sizeof(OPENGL_INFO)) return FALSE;
            POPENGL_INFO info = (POPENGL_INFO)pvOut;

            info->dwVersion = 1;
            info->dwDriverVersion = 1;
            wcscpy(info->szDriverName, L"VIRTIOGPUICD");

            return cjOut;
        }

        case VIRTIO_GPU_ESC_CREATE_CONTEXT:
        {
            if(cjIn < sizeof(VIRTIO_GPU_CREATE_CONTEXT)) return FALSE;
            if(cjOut < sizeof(VIRTIO_GPU_RESOURCE_ID)) return FALSE;
            PVIRTIO_GPU_CREATE_CONTEXT contextRequest = (PVIRTIO_GPU_CREATE_CONTEXT)pvIn;
            PVIRTIO_GPU_RESOURCE_ID resourceIdResponse = (PVIRTIO_GPU_RESOURCE_ID)pvOut;

            VIDEO_CREATE_CONTEXT createContext;
            createContext.NameLength = contextRequest->NameLength;
            strncpy((char *)createContext.Name, (char *)contextRequest->Name, sizeof(createContext.Name));

            VIDEO_RESOURCE_ID resultResourceId;
            if (EngDeviceIoControl(ppdev->hDriver, IOCTL_VIRTIOGPU_CREATE_CONTEXT, &createContext, sizeof(createContext), &resultResourceId, sizeof(resultResourceId), &ulTemp))
            {
                return FALSE;
            }

            resourceIdResponse->ResourceID = resultResourceId.ResourceID;

            return sizeof(VIRTIO_GPU_RESOURCE_ID);
        }

        case VIRTIO_GPU_ESC_SET_CONTEXT:
        {
            if(cjIn < sizeof(VIRTIO_GPU_RESOURCE_ID)) return FALSE;
            PVIRTIO_GPU_RESOURCE_ID resourceIdRequest = (PVIRTIO_GPU_RESOURCE_ID)pvIn;

            VIDEO_RESOURCE_ID resourceId;
            resourceId.ResourceID = resourceIdRequest->ResourceID;
            if (EngDeviceIoControl(ppdev->hDriver, IOCTL_VIRTIOGPU_SET_CONTEXT, &resourceId, sizeof(resourceId), NULL, 0, &ulTemp))
            {
                return FALSE;
            }

            return TRUE;
        }
    }

    return FALSE; //Not supported
}

VOID
VirtioPrintDebug(
    IN PCHAR DebugMessage, ...)
{
    va_list ap;

    va_start(ap, DebugMessage);
    EngDebugPrint(STANDARD_DEBUG_PREFIX, DebugMessage, ap);
    va_end(ap);
}
