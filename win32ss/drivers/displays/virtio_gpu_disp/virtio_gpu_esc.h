/*
 * PROJECT:     ReactOS VirtIO GPU Display video driver
 * LICENSE:     GPL-2.0-or-later (https://spdx.org/licenses/GPL-2.0-or-later)
 * PURPOSE:     Escape codes for display driver
 * COPYRIGHT:   Copyright 2023 Jesús Sanz del Rey (jesussanz2003@gmail.com)
 */

#pragma once

#include <windef.h>

typedef struct
{
    ULONG NameLength;
    UCHAR Name[64];
} VIRTIO_GPU_CREATE_CONTEXT, *PVIRTIO_GPU_CREATE_CONTEXT;

typedef struct
{
    ULONG ResourceID;
} VIRTIO_GPU_RESOURCE_ID, *PVIRTIO_GPU_RESOURCE_ID;

#define VIRTIO_GPU_ESC_CREATE_CONTEXT   0x20000
#define VIRTIO_GPU_ESC_SET_CONTEXT      0x20001