/*
 * PROJECT:     ReactOS VirtIO Gpu miniport video driver
 * LICENSE:     GPL-2.0-or-later (https://spdx.org/licenses/GPL-2.0-or-later)
 * PURPOSE:     Simple framebuffer driver for Virtio Virgl GPU
 * COPYRIGHT:   Copyright 2023 Jesús Sanz del Rey (jesussanz2003@gmail.com)
 */

#pragma once

#include <ntifs.h>

#define __BROKEN__
#include <miniport.h>
#include <video.h>
#include <dderror.h>
#include <windef.h>
#include <wdmguid.h>

#pragma pack(push, 1)
typedef struct
{
    UCHAR Index;
    UCHAR Id;
    UCHAR Padding[2];
    ULONG Offset;
} VIRTIOGPUMP_PCI_CAP_HEADER, *PVIRTIOGPUMP_PCI_CAP_HEADER;
#pragma pack(pop)

typedef struct {
    volatile ULONG deviceFeaturesSel;
    volatile ULONG deviceFeatures;
    volatile ULONG guestFeaturesSel;
    volatile ULONG guestFeatures;
    volatile USHORT msixConfig;
    volatile USHORT numQueues;
    volatile UCHAR deviceStatus;
    volatile UCHAR configGeneration;

    volatile USHORT queueSelect;
    volatile USHORT queueSize;
    volatile USHORT queueMsixVector;
    volatile USHORT queueEnable;
    volatile USHORT queueNotify;
    volatile ULONGLONG queueDesc;
    volatile ULONGLONG queueGuest;
    volatile ULONGLONG queueDevice;
    volatile USHORT queueNotifyData;
    volatile USHORT queueReset;
} VIRTIOGPUMP_REGISTERS, *PVIRTIOGPUMP_REGISTERS;

typedef struct {
    ULONG events_read;
	ULONG events_clear;
	ULONG num_scanouts;
	ULONG num_caspsets;
} VIRTIOGPUMP_CONFIG, *PVIRTIOGPUMP_CONFIG;

#define VIRTIO_DEVICE_ACKNOWLEDGED 0x1
#define VIRTIO_DRIVER_LOADED 0x2
#define VIRTIO_DRIVER_READY 0x4
#define VIRTIO_FEATURES_OK 0x8
#define VIRTIO_DEVICE_ERROR 0x40
#define VIRTIO_DRIVER_FAILED 0x80

#define VIRTQ_DESC_F_NEXT 1
#define VIRTQ_DESC_F_WRITE 2
#define VIRTQ_DESC_F_INDIRECT 4

#define VIRTQ_USED_F_NO_NOTIFY 1

#pragma pack(push, 1)
typedef struct {
    volatile ULONGLONG Address;
    volatile ULONG Length;
    volatile USHORT Flags;
    volatile USHORT Next;
} VIRTIOGPUMP_QUEUE_DESCRIPTOR, *PVIRTIOGPUMP_QUEUE_DESCRIPTOR;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    volatile USHORT Flags;
    volatile USHORT Index;
    volatile USHORT Rings[];
} VIRTIOGPUMP_QUEUE_AVAILABLE, *PVIRTIOGPUMP_QUEUE_AVAILABLE;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    volatile ULONG Index;
    volatile ULONG Length;
} VIRTIOGPUMP_QUEUE_USED_ELEMENT, *PVIRTIOGPUMP_QUEUE_USED_ELEMENT;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    volatile USHORT Flags;
    volatile USHORT Index;
    volatile VIRTIOGPUMP_QUEUE_USED_ELEMENT Rings[];
} VIRTIOGPUMP_QUEUE_USED, *PVIRTIOGPUMP_QUEUE_USED;
#pragma pack(pop)

typedef struct {
    volatile PVIRTIOGPUMP_QUEUE_DESCRIPTOR Descriptors;
    volatile PVIRTIOGPUMP_QUEUE_AVAILABLE Available;
    volatile PVIRTIOGPUMP_QUEUE_USED Used;
    volatile USHORT FreeList;
    volatile USHORT FreeCount;
    USHORT QueueSize;
    USHORT QueueMask;
    volatile USHORT LastUsed;

    volatile PUSHORT Notify;

    volatile PUCHAR DescriptorsEndStatus;
} VIRTIOGPUMP_QUEUE_VIRTQUEUE, *PVIRTIOGPUMP_QUEUE_VIRTQUEUE;

typedef struct
{
    PHYSICAL_ADDRESS BarsToMapPhysStart[5];
    ULONG BarsToMapLength[5];
    PVOID BarsToMapVirtStart[5];

    VIRTIOGPUMP_PCI_CAP_HEADER RegistersBarCap;
    PVOID VirtRegistersStart;

    VIRTIOGPUMP_PCI_CAP_HEADER NotifyBarCap;
    PVOID VirtNotifyStart;

    VIRTIOGPUMP_PCI_CAP_HEADER IsrBarCap;
    PVOID VirtIsrStart;

    VIRTIOGPUMP_PCI_CAP_HEADER ConfigBarCap;
    PVOID VirtConfigStart;

    PVIRTIOGPUMP_REGISTERS Registers;
    volatile PVOID Notify;
    volatile PULONG IsrStatus;
    volatile PVIRTIOGPUMP_CONFIG DeviceConfig;

    ULONG NotifyMultiplier;

    PVIRTIOGPUMP_QUEUE_VIRTQUEUE VirtualQueues;
    ULONG ActiveQueues;
} VIRTIOGPUMP_DEVICE_EXTENSION, *PVIRTIOGPUMP_DEVICE_EXTENSION;

#pragma pack(push, 1)
typedef struct {
    ULONG Type;
    ULONG Flags;
    ULONGLONG FenceID;
    ULONG CtxID;
    UCHAR RingIdx;
    UCHAR Padding[3];
} VIRTIOGPUMP_CONTROL_HEADER, *PVIRTIOGPUMP_CONTROL_HEADER;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    ULONG X;
	ULONG Y;
	ULONG Width;
	ULONG Height;

} VIRTIOGPUMP_RECT, *PVIRTIOGPUMP_RECT;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    VIRTIOGPUMP_RECT Rect;
    ULONG Enabled;
    ULONG Flags;
} VIRTIOGPUMP_MODE, *PVIRTIOGPUMP_MODE;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    VIRTIOGPUMP_CONTROL_HEADER Header;
    VIRTIOGPUMP_MODE Modes[16];
} VIRTIOGPUMP_DISPLAY_INFO, *PVIRTIOGPUMP_DISPLAY_INFO;
#pragma pack(pop)

enum virtio_gpu_ctrl_type {
        /* 2d commands */
        VIRTIO_GPU_CMD_GET_DISPLAY_INFO = 0x0100,
        VIRTIO_GPU_CMD_RESOURCE_CREATE_2D,
        VIRTIO_GPU_CMD_RESOURCE_UNREF,
        VIRTIO_GPU_CMD_SET_SCANOUT,
        VIRTIO_GPU_CMD_RESOURCE_FLUSH,
        VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D,
        VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING,
        VIRTIO_GPU_CMD_RESOURCE_DETACH_BACKING,
        VIRTIO_GPU_CMD_GET_CAPSET_INFO,
        VIRTIO_GPU_CMD_GET_CAPSET,
        VIRTIO_GPU_CMD_GET_EDID,
        VIRTIO_GPU_CMD_RESOURCE_ASSIGN_UUID,

        /* 3d commands (OpenGL) */
        VIRTIO_GPU_CMD_CTX_CREATE = 0x0200,
        VIRTIO_GPU_CMD_CTX_DESTROY,
        VIRTIO_GPU_CMD_CTX_ATTACH_RESOURCE,
        VIRTIO_GPU_CMD_CTX_DETACH_RESOURCE,
        VIRTIO_GPU_CMD_RESOURCE_CREATE_3D,
        VIRTIO_GPU_CMD_TRANSFER_TO_HOST_3D,
        VIRTIO_GPU_CMD_TRANSFER_FROM_HOST_3D,
        VIRTIO_GPU_CMD_SUBMIT_3D,

        /* cursor commands */
        VIRTIO_GPU_CMD_UPDATE_CURSOR = 0x0300,
        VIRTIO_GPU_CMD_MOVE_CURSOR,

        /* success responses */
        VIRTIO_GPU_RESP_OK_NODATA = 0x1100,
        VIRTIO_GPU_RESP_OK_DISPLAY_INFO,
        VIRTIO_GPU_RESP_OK_CAPSET_INFO,
        VIRTIO_GPU_RESP_OK_CAPSET,
        VIRTIO_GPU_RESP_OK_EDID,

        /* error responses */
        VIRTIO_GPU_RESP_ERR_UNSPEC = 0x1200,
        VIRTIO_GPU_RESP_ERR_OUT_OF_MEMORY,
        VIRTIO_GPU_RESP_ERR_INVALID_SCANOUT_ID,
        VIRTIO_GPU_RESP_ERR_INVALID_RESOURCE_ID,
        VIRTIO_GPU_RESP_ERR_INVALID_CONTEXT_ID,
        VIRTIO_GPU_RESP_ERR_INVALID_PARAMETER,
};

#define VGPU_TAG ((ULONG)'mgvD')