/*
 * PROJECT:     ReactOS VirtIO GPU Display video driver
 * LICENSE:     GPL-2.0-or-later (https://spdx.org/licenses/GPL-2.0-or-later)
 * PURPOSE:     Simple Display driver for Virtio Virgl GPU
 * COPYRIGHT:   Copyright 2023 Jesús Sanz del Rey (jesussanz2003@gmail.com)
 */

#pragma once

#include <stdarg.h>
#include <windef.h>
#include <wingdi.h>
#include <winddi.h>
#include <winioctl.h>
#include <ntddvdeo.h>

typedef struct _PDEV
{
    HANDLE hDriver;
    HDEV hDevEng;
    HSURF hSurfEng;

    ULONG ModeIndex;
    ULONG ScreenWidth;
    ULONG ScreenHeight;
    ULONG ScreenDelta;
    BYTE BitsPerPixel;

    ULONG RedMask;
    ULONG GreenMask;
    ULONG BlueMask;

    PVOID ScreenPtr;

    HPALETTE DefaultPalette;

    DWORD iDitherFormat;
    ULONG MemHeight;
    ULONG MemWidth;
} PDEV, *PPDEV;

#define DEVICE_NAME	L"virtiogpudisp"
#define ALLOC_TAG	'GOIV'

#define STANDARD_DEBUG_PREFIX     "virtiogpudisp: "
VOID
VirtioPrintDebug(IN PCHAR DebugMessage, ...);

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
    IN HANDLE hDriver);

BOOL
IntInitScreenInfo(
    PPDEV ppdev,
    LPDEVMODEW pDevMode,
    PGDIINFO pGdiInfo,
    PDEVINFO pDevInfo);

DWORD
GetAvailableModes(
    HANDLE hDriver,
    PVIDEO_MODE_INFORMATION *ModeInfo,
    DWORD *ModeInfoSize);

ULONG APIENTRY
DrvGetModes(
   IN HANDLE hDriver,
   IN ULONG cjSize,
   OUT DEVMODEW *pdm);

HSURF APIENTRY
DrvEnableSurface(
  DHPDEV dhpdev);

VOID APIENTRY
DrvDisableSurface(
   IN DHPDEV dhpdev);

BOOL APIENTRY
DrvAssertMode(
   IN DHPDEV dhpdev,
   IN BOOL bEnable);

BOOL APIENTRY
DrvSetPalette(
   IN DHPDEV dhpdev,
   IN PALOBJ *ppalo,
   IN FLONG fl,
   IN ULONG iStart,
   IN ULONG cColors);

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
   IN FLONG fl);

VOID APIENTRY
DrvMovePointer(
   IN SURFOBJ *pso,
   IN LONG x,
   IN LONG y,
   IN RECTL *prcl);