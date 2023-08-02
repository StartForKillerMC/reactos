/*
 * PROJECT:     ReactOS VirtIO GPU miniport video driver
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

#pragma pack(push, 1)
typedef struct {
    VIRTIOGPUMP_CONTROL_HEADER Header;
    VIRTIOGPUMP_RECT Rect;
    ULONG ScanoutID;
    ULONG ResourceID;
} VIRTIOGPUMP_SET_SCANOUT, *PVIRTIOGPUMP_SET_SCANOUT;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    VIRTIOGPUMP_CONTROL_HEADER Header;
    ULONG ResourceID;
    ULONG Format;
    ULONG Width;
    ULONG Height;
} VIRTIOGPUMP_RESOURCE_CREATE_2D, *PVIRTIOGPUMP_RESOURCE_CREATE_2D;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    VIRTIOGPUMP_CONTROL_HEADER Header;
    ULONG ResourceID;
    ULONG Padding;
} VIRTIOGPUMP_RESOURCE_UNREF, *PVIRTIOGPUMP_RESOURCE_UNREF;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    ULONGLONG Address;
    ULONG Length;
    ULONG Padding;

} VIRTIOGPUMP_MEM_ENTRY, *PVIRTIOGPUMP_MEM_ENTRY;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    VIRTIOGPUMP_CONTROL_HEADER Header;
    ULONG ResourceID;
    ULONG EntryCount;
} VIRTIOGPUMP_RESOURCE_ATTACH_BACKING, *PVIRTIOGPUMP_RESOURCE_ATTACH_BACKING;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    VIRTIOGPUMP_CONTROL_HEADER Header;
    VIRTIOGPUMP_RECT Rect;
    ULONGLONG Offset;
    ULONG ResourceID;
    ULONG Padding;
} VIRTIOGPUMP_TRANSFER_TO_HOST_2D, *PVIRTIOGPUMP_TRANSFER_TO_HOST_2D;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    VIRTIOGPUMP_CONTROL_HEADER Header;
    VIRTIOGPUMP_RECT Rect;
    ULONG ResourceID;
    ULONG Padding;
} VIRTIOGPUMP_RESOURCE_FLUSH, *PVIRTIOGPUMP_RESOURCE_FLUSH;
#pragma pack(pop)

enum virtio_gpu_formats {
    VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM  = 1,
    VIRTIO_GPU_FORMAT_B8G8R8X8_UNORM  = 2,
    VIRTIO_GPU_FORMAT_A8R8G8B8_UNORM  = 3,
    VIRTIO_GPU_FORMAT_X8R8G8B8_UNORM  = 4,

    VIRTIO_GPU_FORMAT_R8G8B8A8_UNORM  = 67,
    VIRTIO_GPU_FORMAT_X8B8G8R8_UNORM  = 68,

    VIRTIO_GPU_FORMAT_A8B8G8R8_UNORM  = 121,
    VIRTIO_GPU_FORMAT_R8G8B8X8_UNORM  = 134,
};

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

    PHYSICAL_ADDRESS PhysFramebuffer;
    ULONG PhysFramebufferLength;

    PVP_DMA_ADAPTER DmaAdapater;

    ULONG ResourceCurrentAllocatorIdx;

    BOOLEAN ModesSaved;
    VIRTIOGPUMP_MODE SavedModes[16];
    UINT EnabledModeCount;
    UINT CurrentMode;

    KTIMER FlushTimer;
    KDPC FlushDpc;
    PSPIN_LOCK  Lock;

    PVOID Framebuffer;
    ULONG ScanoutResourceID;
} VIRTIOGPUMP_DEVICE_EXTENSION, *PVIRTIOGPUMP_DEVICE_EXTENSION;

#define VGPU_TAG ((ULONG)'mgvD')

typedef struct _VIDEO_CREATE_2D_RESOURCE {
  ULONG Width;
  ULONG Height;
  ULONG Format;
} VIDEO_CREATE_2D_RESOURCE, *PVIDEO_CREATE_2D_RESOURCE;

typedef struct _VIDEO_CREATE_2D_RESOURCE {
  ULONG ResourceID;
} VIDEO_RESOURCE_ID, *PVIDEO_RESOURCE_ID;

#define IOCTL_VIDEO_CREATE_2D_RESOURCE \
  CTL_CODE(FILE_DEVICE_VIDEO, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)