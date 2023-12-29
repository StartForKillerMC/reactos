/*
 * PROJECT:     ReactOS VirtIO GPU OpenGL ICD driver
 * LICENSE:     GPL-2.0-or-later (https://spdx.org/licenses/GPL-2.0-or-later)
 * PURPOSE:     OpenGL ICD Driver for Virtio Virgl GPU
 * COPYRIGHT:   Copyright 2023 Jesús Sanz del Rey (jesussanz2003@gmail.com)
 */

#pragma once

#include <windows.h>
#include "debug.h"

typedef struct virgl_context {
    HDC hDC;
    ULONG ResourceID;
} VIRGL_CONTEXT, *PVIRGL_CONTEXT;
