/*
 * PROJECT:     ReactOS VirtIO GPU Display video driver
 * LICENSE:     GPL-2.0-or-later (https://spdx.org/licenses/GPL-2.0-or-later)
 * PURPOSE:     Pointer related functions for Virtio Virgl GPU Display Driver
 * COPYRIGHT:   Copyright 2023 Jesús Sanz del Rey (jesussanz2003@gmail.com)
 */

#include "virtio_gpu_disp.h"

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
   EngMovePointer(pso, x, y, prcl);
}