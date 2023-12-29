/*
 * PROJECT:     ReactOS VirtIO GPU miniport video driver
 * LICENSE:     GPL-2.0-or-later (https://spdx.org/licenses/GPL-2.0-or-later)
 * PURPOSE:     Simple framebuffer driver for Virtio Virgl GPU
 * COPYRIGHT:   Copyright 2023 Jesús Sanz del Rey (jesussanz2003@gmail.com)
 */

#include "virtio_gpu_mp.h"

#include <debug.h>

/*
 * VirtioGpuMpFindAdapter
 *
 * Detects the Virtio Gpu display adapter.
 */

VP_STATUS
NTAPI
VirtioGpuMpFindAdapter(
    IN PVOID HwDeviceExtension,
    IN PVOID HwContext,
    IN PWSTR ArgumentString,
    IN OUT PVIDEO_PORT_CONFIG_INFO ConfigInfo,
    OUT PUCHAR Again)
{
    ERR_(IHVVIDEO, "VirtioGpuMpFindAdapter\n");

    PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension;

    VP_STATUS Status;
    USHORT VendorId = 0x1AF4;
    USHORT DeviceId = 0x1050;
    ULONG Slot = 0;

    VirtioGpuMpDeviceExtension = (PVIRTIOGPUMP_DEVICE_EXTENSION)HwDeviceExtension;

    VIDEO_ACCESS_RANGE AccessRanges[5]; //PCI Don't have more than 5 BAR
    VideoPortZeroMemory(&AccessRanges, sizeof(AccessRanges));
    Status = VideoPortGetAccessRanges(HwDeviceExtension, 0, NULL,
                                      5, AccessRanges,
                                      &VendorId, &DeviceId, &Slot);

    //Enable busmastering
    ULONG Command = 0;
    if(VideoPortGetBusData(HwDeviceExtension, PCIConfiguration, 0, (PVOID) &Command,
                        FIELD_OFFSET(
                            PCI_COMMON_CONFIG,
                            Command
                        ), sizeof(ULONG)) == 0)
    {
        ERR_(IHVVIDEO, "Error reading command config\n");
        return ERROR_DEV_NOT_EXIST;
    }
    Command |= (1 << 2);
    Command &= ~(1 << 10);
    if(VideoPortSetBusData(HwDeviceExtension, PCIConfiguration, 0, (PVOID) &Command,
                        FIELD_OFFSET(
                            PCI_COMMON_CONFIG,
                            Command
                        ), sizeof(ULONG)) == 0)
    {
        ERR_(IHVVIDEO, "Error writting command config\n");
        return ERROR_DEV_NOT_EXIST;
    }

    //TODO: Check if capabilities are enabled

    UCHAR CapabilityOffset = 0;
    if(VideoPortGetBusData(HwDeviceExtension, PCIConfiguration, 0, (PVOID) &CapabilityOffset,
                        FIELD_OFFSET(
                            PCI_COMMON_CONFIG,
                            u.type0.CapabilitiesPtr
                        ), sizeof(UCHAR)) == 0)
    {
        ERR_(IHVVIDEO, "Not exist");
        return ERROR_DEV_NOT_EXIST;
    }

    ERR_(IHVVIDEO, "Capability Offset: 0x%x\n", CapabilityOffset);

    while(CapabilityOffset) {
        UCHAR CapabilityID = 0;
        UCHAR CapabilityNext = 0;
        if(VideoPortGetBusData(HwDeviceExtension, PCIConfiguration,
                            0, (PVOID) &CapabilityID,
                            CapabilityOffset + 0, sizeof(UCHAR)) == 0 ||
           VideoPortGetBusData(HwDeviceExtension, PCIConfiguration,
                            0, (PVOID) &CapabilityNext,
                            CapabilityOffset + 1, sizeof(UCHAR)) == 0)
        {
            ERR_(IHVVIDEO, "Not exist\n");
            return ERROR_DEV_NOT_EXIST;
        }

        ERR_(IHVVIDEO, "Capability ID: 0x%x Offset: 0x%x\n", CapabilityID, CapabilityOffset);

        if(CapabilityID == 0x09) {
            UCHAR CapabilityType = 0;
            if(VideoPortGetBusData(HwDeviceExtension, PCIConfiguration,
                                0, (PVOID) &CapabilityType,
                                CapabilityOffset + 3, sizeof(UCHAR)) == 0)
            {
                ERR_(IHVVIDEO, "Not exist\n");
                return ERROR_DEV_NOT_EXIST;
            }

            ERR_(IHVVIDEO, "Capability Type: 0x%x\n", CapabilityType);

            switch(CapabilityType) {
                case 0x1:
                    if(VideoPortGetBusData(HwDeviceExtension, PCIConfiguration,
                            0, (PVOID) &VirtioGpuMpDeviceExtension->RegistersBarCap, CapabilityOffset + 4,
                            sizeof(VIRTIOGPUMP_PCI_CAP_HEADER)) == 0)
                    {
                        ERR_(IHVVIDEO, "Not exist\n");
                        return ERROR_DEV_NOT_EXIST;
                    }
                    ERR_(IHVVIDEO, "Found Registers Cap Offset: 0x%x\n", CapabilityOffset);
                    break;
                case 0x2:
                    if(VideoPortGetBusData(HwDeviceExtension, PCIConfiguration,
                            0, (PVOID) &VirtioGpuMpDeviceExtension->NotifyBarCap, CapabilityOffset + 4,
                            sizeof(VIRTIOGPUMP_PCI_CAP_HEADER)) == 0 ||
                       VideoPortGetBusData(HwDeviceExtension, PCIConfiguration,
                            0, (PVOID) &VirtioGpuMpDeviceExtension->NotifyMultiplier,
                            CapabilityOffset + 16, sizeof(UINT)) == 0)
                    {
                        ERR_(IHVVIDEO, "Not exist\n");
                        return ERROR_DEV_NOT_EXIST;
                    }
                    ERR_(IHVVIDEO, "Found Notify Cap Offset: 0x%x\n", CapabilityOffset);
                    break;
                case 0x3:
                    if(VideoPortGetBusData(HwDeviceExtension, PCIConfiguration,
                            0, (PVOID) &VirtioGpuMpDeviceExtension->IsrBarCap, CapabilityOffset + 4,
                            sizeof(VIRTIOGPUMP_PCI_CAP_HEADER)) == 0)
                    {
                        ERR_(IHVVIDEO, "Not exist\n");
                        return ERROR_DEV_NOT_EXIST;
                    }
                    ERR_(IHVVIDEO, "Found Isr Cap Offset: 0x%x\n", CapabilityOffset);
                    break;
                case 0x4:
                    if(VideoPortGetBusData(HwDeviceExtension, PCIConfiguration,
                            0, (PVOID) &VirtioGpuMpDeviceExtension->ConfigBarCap, CapabilityOffset + 4,
                            sizeof(VIRTIOGPUMP_PCI_CAP_HEADER)) == 0)
                    {
                        ERR_(IHVVIDEO, "Not exist\n");
                        return ERROR_DEV_NOT_EXIST;
                    }
                    ERR_(IHVVIDEO, "Found Config Cap Offset: 0x%x\n", CapabilityOffset);
                    break;
            }
        }

        CapabilityOffset = CapabilityNext;
    }

    if(Status == NO_ERROR)
    {
        for(INT i = 0; i < 5; i++) {
            if(VirtioGpuMpDeviceExtension->RegistersBarCap.Index == (i+1) || VirtioGpuMpDeviceExtension->NotifyBarCap.Index == (i+1) ||
               VirtioGpuMpDeviceExtension->IsrBarCap.Index == (i+1)       || VirtioGpuMpDeviceExtension->ConfigBarCap.Index == (i+1))
            {
                VirtioGpuMpDeviceExtension->BarsToMapPhysStart[i] = AccessRanges[i].RangeStart;
                VirtioGpuMpDeviceExtension->BarsToMapLength[i] = AccessRanges[i].RangeLength;
            } else {
                //Set to zero to check for this later
                VirtioGpuMpDeviceExtension->BarsToMapLength[i] = 0;
            }
        }

        VirtioGpuMpDeviceExtension->PhysFramebuffer = AccessRanges[0].RangeStart;
        VirtioGpuMpDeviceExtension->PhysFramebufferLength = AccessRanges[0].RangeLength;
    }

    VP_DEVICE_DESCRIPTION DeviceDescription;
    DeviceDescription.ScatterGather = TRUE;
    DeviceDescription.Dma32BitAddresses = TRUE;
    DeviceDescription.Dma64BitAddresses = TRUE;
    DeviceDescription.MaximumLength = (ULONG)-1;
    VirtioGpuMpDeviceExtension->DmaAdapter = VideoPortGetDmaAdapter(HwDeviceExtension, &DeviceDescription);
    if(VirtioGpuMpDeviceExtension->DmaAdapter == NULL)
        return ERROR_DEV_NOT_EXIST;

    ERR_(IHVVIDEO, "Inited Status: %d, ridx(%d:%d), nidx(%d:%d), iidx(%d:%d), cidx(%d:%d)\n", Status,
        VirtioGpuMpDeviceExtension->RegistersBarCap.Index, VirtioGpuMpDeviceExtension->RegistersBarCap.Offset,
        VirtioGpuMpDeviceExtension->NotifyBarCap.Index, VirtioGpuMpDeviceExtension->NotifyBarCap.Offset,
        VirtioGpuMpDeviceExtension->IsrBarCap.Index, VirtioGpuMpDeviceExtension->IsrBarCap.Offset,
        VirtioGpuMpDeviceExtension->ConfigBarCap.Index, VirtioGpuMpDeviceExtension->ConfigBarCap.Offset);

    return Status;
}

VOID
VirtioGpuMpSetupQueue(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension,
    ULONG Index)
{
    PVIRTIOGPUMP_QUEUE_VIRTQUEUE VirtQueue = &VirtioGpuMpDeviceExtension->VirtualQueues[Index];
    VirtioGpuMpDeviceExtension->Registers->queueSelect = Index;
    UINT QueueSize = VirtioGpuMpDeviceExtension->Registers->queueSize;
    VirtioGpuMpDeviceExtension->Registers->queueSize = QueueSize;

    ULONG DescriptorsSize = sizeof(VIRTIOGPUMP_QUEUE_VIRTQUEUE) * QueueSize;
    ULONG AvailableSize = (QueueSize + 3) * sizeof(USHORT);
    ULONG UsedSize = (3 * sizeof(USHORT)) + (QueueSize * sizeof(VIRTIOGPUMP_QUEUE_USED_ELEMENT));

    VirtQueue->Descriptors = (volatile PVIRTIOGPUMP_QUEUE_DESCRIPTOR)VideoPortAllocatePool((PVOID)VirtioGpuMpDeviceExtension, VpNonPagedPoolCacheAligned,
        DescriptorsSize, VGPU_TAG);
    VideoPortZeroMemory((PVOID)VirtQueue->Descriptors, DescriptorsSize);
    VirtQueue->Available = (volatile PVIRTIOGPUMP_QUEUE_AVAILABLE)VideoPortAllocatePool((PVOID)VirtioGpuMpDeviceExtension, VpNonPagedPoolCacheAligned,
        AvailableSize, VGPU_TAG);
    VideoPortZeroMemory((PVOID)VirtQueue->Available, AvailableSize);
    VirtQueue->Used = (volatile PVIRTIOGPUMP_QUEUE_USED)VideoPortAllocatePool((PVOID)VirtioGpuMpDeviceExtension, VpNonPagedPoolCacheAligned,
        UsedSize, VGPU_TAG);
    VideoPortZeroMemory((PVOID)VirtQueue->Used, UsedSize);

    VirtQueue->FreeList = 0;
    for(INT i = 0; i < QueueSize; i++)
    {
        //ERR_(IHVVIDEO, "Free List for queue %d is %d\n", Index, VirtQueue->FreeList);
        VirtQueue->Descriptors[i].Next = VirtQueue->FreeList;
        VirtQueue->FreeList = i;
        VirtQueue->FreeCount++;
    }

    VirtQueue->QueueSize = QueueSize;
    //TODO: Find a proper way to support msvc compiler
    VirtQueue->QueueMask = (1 << ((sizeof(QueueSize) * 8) - 1 - __builtin_clz(QueueSize))) - 1;
    VirtQueue->Notify = (volatile PUSHORT)((ULONG_PTR)VirtioGpuMpDeviceExtension->Notify + (ULONG_PTR)(USHORT)VirtioGpuMpDeviceExtension->Registers->queueNotify * (ULONG_PTR)VirtioGpuMpDeviceExtension->NotifyMultiplier);
    VirtQueue->DescriptorsEndStatus = (PUCHAR)VideoPortAllocatePool((PVOID)VirtioGpuMpDeviceExtension, VpNonPagedPool,
            sizeof(UCHAR) * QueueSize, VGPU_TAG);
    for(INT i = 0; i < QueueSize; i++)
    {
        VirtQueue->DescriptorsEndStatus[i] = 1; //finished
    }

    VirtioGpuMpDeviceExtension->Registers->queueDesc = MmGetPhysicalAddress((PVOID)VirtQueue->Descriptors).QuadPart;
    VirtioGpuMpDeviceExtension->Registers->queueGuest = MmGetPhysicalAddress((PVOID)VirtQueue->Available).QuadPart;
    VirtioGpuMpDeviceExtension->Registers->queueDevice = MmGetPhysicalAddress((PVOID)VirtQueue->Used).QuadPart;
    VirtioGpuMpDeviceExtension->Registers->queueMsixVector = 0xFFFF;
    VirtioGpuMpDeviceExtension->Registers->queueEnable = 0x1;

    VirtioGpuMpDeviceExtension->ActiveQueues |= (1 << Index);
}

PVIRTIOGPUMP_QUEUE_DESCRIPTOR
VirtioGpuMpAllocateDescriptors(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension,
    ULONG QueueIndex,
    UINT Count,
    PUSHORT FirstDescriptorIndex
)
{
    if(QueueIndex >= VirtioGpuMpDeviceExtension->Registers->numQueues) return NULL;
    PVIRTIOGPUMP_QUEUE_VIRTQUEUE Queue = &VirtioGpuMpDeviceExtension->VirtualQueues[QueueIndex];
    if((VirtioGpuMpDeviceExtension->ActiveQueues & (1 << QueueIndex)) == 0 || Queue->FreeCount < Count) return NULL;

    volatile PVIRTIOGPUMP_QUEUE_DESCRIPTOR LastDescriptor = NULL;
    USHORT LastIndex = 0;
    while(Count > 0)
    {
        USHORT i = Queue->FreeList;
        volatile PVIRTIOGPUMP_QUEUE_DESCRIPTOR Descriptor = &Queue->Descriptors[i];

        Queue->FreeList = Descriptor->Next;
        Queue->FreeCount--;

        if(LastDescriptor != NULL)
        {
            Descriptor->Flags = VIRTQ_DESC_F_NEXT;
            Descriptor->Next = LastIndex;
        }
        else
        {
            Descriptor->Flags = 0;
            Descriptor->Next = 0;
        }

        LastDescriptor = Descriptor;
        LastIndex = i;
        Count--;
    }

    if(FirstDescriptorIndex != NULL) *FirstDescriptorIndex = LastIndex;

    return (PVIRTIOGPUMP_QUEUE_DESCRIPTOR)LastDescriptor;
}

PVIRTIOGPUMP_QUEUE_DESCRIPTOR
VirtioGpuMpGetNextDescriptor(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension,
    ULONG QueueIndex,
    UINT DescriptorIndex
)
{
    if(QueueIndex >= VirtioGpuMpDeviceExtension->Registers->numQueues) return NULL;
    PVIRTIOGPUMP_QUEUE_VIRTQUEUE Queue = &VirtioGpuMpDeviceExtension->VirtualQueues[QueueIndex];
    if((VirtioGpuMpDeviceExtension->ActiveQueues & (1 << QueueIndex)) == 0) return NULL;

    if(Queue->Descriptors[DescriptorIndex].Next >= Queue->QueueSize) return NULL;

    return (PVIRTIOGPUMP_QUEUE_DESCRIPTOR)&Queue->Descriptors[Queue->Descriptors[DescriptorIndex].Next];
}

VOID
VirtioGpuMpFreeDescriptor(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension,
    ULONG QueueIndex,
    USHORT DescriptorIndex
)
{
    if(QueueIndex >= VirtioGpuMpDeviceExtension->Registers->numQueues) return;
    PVIRTIOGPUMP_QUEUE_VIRTQUEUE Queue = &VirtioGpuMpDeviceExtension->VirtualQueues[QueueIndex];
    if((VirtioGpuMpDeviceExtension->ActiveQueues & (1 << QueueIndex)) == 0) return;

    Queue->Descriptors[DescriptorIndex].Next = Queue->FreeList;
    Queue->Descriptors[DescriptorIndex].Address = 0;
    Queue->FreeList = DescriptorIndex;
    Queue->FreeCount++;
}

static
VOID
VirtioGpuMpSubmitChain(
    PVIRTIOGPUMP_QUEUE_VIRTQUEUE Queue,
    USHORT DescriptorIndex
)
{
    volatile PVIRTIOGPUMP_QUEUE_AVAILABLE Available = (volatile PVIRTIOGPUMP_QUEUE_AVAILABLE)Queue->Available;

    //TRACE_(IHVVIDEO, "Submitting Descriptor %d to available index %d\n", DescriptorIndex, Available->Index);

    Available->Rings[Available->Index & Queue->QueueMask] = DescriptorIndex;
    KeMemoryBarrier();
    //__asm__ __volatile__ ("" ::: "memory");
    //__atomic_thread_fence(__ATOMIC_SEQ_CST);
    Available->Index++;
}

static
VOID
VirtioGpuMpKickQueue(
    PVIRTIOGPUMP_QUEUE_VIRTQUEUE Queue,
    USHORT QueueIndex
)
{
    *Queue->Notify = QueueIndex;
    KeMemoryBarrier();

    //__asm__ __volatile__ ("" ::: "memory");
    //__atomic_thread_fence(__ATOMIC_SEQ_CST);
}

#define INTERRUPT_NOT_PRESENT_TIMEOUT_MS 60000L
#define INTERRUPT_NOT_PRESENT_TIMEOUT_100NS (INTERRUPT_NOT_PRESENT_TIMEOUT_MS * 10000L)

VOID
VirtioGpuMpSubmitDescriptor(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension,
    ULONG QueueIndex,
    USHORT DescriptorIndex,
    SHORT WaitDescriptorIndex
)
{
    if(QueueIndex >= VirtioGpuMpDeviceExtension->Registers->numQueues) return;

    PVIRTIOGPUMP_QUEUE_VIRTQUEUE Queue = &VirtioGpuMpDeviceExtension->VirtualQueues[QueueIndex];
    if((VirtioGpuMpDeviceExtension->ActiveQueues & (1 << QueueIndex)) == 0) return;

    if(WaitDescriptorIndex >= 0) {
        VideoPortAcquireSpinLockAtDpcLevel((PVOID)VirtioGpuMpDeviceExtension, VirtioGpuMpDeviceExtension->CommandCompleteLock);
        *((volatile PUCHAR)&Queue->DescriptorsEndStatus[WaitDescriptorIndex]) = 0; //Working
    }

    VirtioGpuMpSubmitChain(Queue, DescriptorIndex);
    VirtioGpuMpKickQueue(Queue, QueueIndex);

    if(WaitDescriptorIndex >= 0) {
        while(!KeTestSpinLock((PKSPIN_LOCK)VirtioGpuMpDeviceExtension->CommandCompleteLock));
        //VideoPortAcquireSpinLockAtDpcLevel((PVOID)VirtioGpuMpDeviceExtension, VirtioGpuMpDeviceExtension->CommandCompleteLock);
        //Queue->DescriptorsEndStatus[WaitDescriptorIndex] != 1);
    }
}

BOOLEAN
VirtioGpuMpSendCmdCommandResponseDpc(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension,
    PVOID Cmd,
    ULONG CmdLength,
    PVOID *Response,
    ULONG ResponseLength)
{
    USHORT i = 0;
    PVIRTIOGPUMP_QUEUE_DESCRIPTOR Descriptor = VirtioGpuMpAllocateDescriptors(VirtioGpuMpDeviceExtension, 0, 2, &i);
    if(Descriptor == NULL)
    {
        ERR_(IHVVIDEO, "First Descriptor NULL\n");
        return FALSE;
    }

    Descriptor->Address = MmGetPhysicalAddress(Cmd).QuadPart;
    Descriptor->Length = CmdLength;
    Descriptor->Flags |= VIRTQ_DESC_F_NEXT;

    USHORT Next = Descriptor->Next;
    Descriptor = VirtioGpuMpGetNextDescriptor(VirtioGpuMpDeviceExtension, 0, i);
    if(Descriptor == NULL) {
        VirtioGpuMpFreeDescriptor(VirtioGpuMpDeviceExtension, 0, i);
        ERR_(IHVVIDEO, "Second Descriptor NULL\n");
        return FALSE;
    }

    PVOID Res = (PVOID)VideoPortAllocatePool((PVOID)VirtioGpuMpDeviceExtension, VpNonPagedPoolCacheAligned,
            ResponseLength, VGPU_TAG);
    VideoPortZeroMemory(Res, ResponseLength);
    *Response = Res;

    Descriptor->Address = MmGetPhysicalAddress(Res).QuadPart;
    Descriptor->Length = ResponseLength;
    Descriptor->Flags |= VIRTQ_DESC_F_WRITE;

    VirtioGpuMpSubmitDescriptor(VirtioGpuMpDeviceExtension, 0, i, Next);
    VirtioGpuMpFreeDescriptor(VirtioGpuMpDeviceExtension, 0, Next);

    return TRUE;
}

BOOLEAN
VirtioGpuMpSendCmdCommandResponse(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension,
    PVOID Cmd,
    ULONG CmdLength,
    PVOID *Response,
    ULONG ResponseLength)
{
    KIRQL OldIrql;
    VideoPortAcquireSpinLock((PVOID)VirtioGpuMpDeviceExtension, VirtioGpuMpDeviceExtension->CommandLock, &OldIrql);
    BOOLEAN Done = VirtioGpuMpSendCmdCommandResponseDpc(VirtioGpuMpDeviceExtension, Cmd, CmdLength, Response, ResponseLength);
    VideoPortReleaseSpinLock((PVOID)VirtioGpuMpDeviceExtension, VirtioGpuMpDeviceExtension->CommandLock, OldIrql);
    return Done;
}

BOOLEAN
VirtioGpuMpSendCursorCommandResponse(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension,
    PVOID Cmd,
    ULONG CmdLength,
    PVOID *Response,
    ULONG ResponseLength)
{
    KIRQL OldIrql;
    VideoPortAcquireSpinLock((PVOID)VirtioGpuMpDeviceExtension, VirtioGpuMpDeviceExtension->CursorLock, &OldIrql);

    USHORT i = 0;
    PVIRTIOGPUMP_QUEUE_DESCRIPTOR Descriptor = VirtioGpuMpAllocateDescriptors(VirtioGpuMpDeviceExtension, 1, 2, &i);
    if(Descriptor == NULL)
    {
        ERR_(IHVVIDEO, "First Descriptor NULL\n");
        VideoPortReleaseSpinLock((PVOID)VirtioGpuMpDeviceExtension, VirtioGpuMpDeviceExtension->CursorLock, OldIrql);
        return FALSE;
    }

    Descriptor->Address = MmGetPhysicalAddress(Cmd).QuadPart;
    Descriptor->Length = CmdLength;
    Descriptor->Flags |= VIRTQ_DESC_F_NEXT;

    USHORT Next = Descriptor->Next;
    Descriptor = VirtioGpuMpGetNextDescriptor(VirtioGpuMpDeviceExtension, 1, i);
    if(Descriptor == NULL) {
        VirtioGpuMpFreeDescriptor(VirtioGpuMpDeviceExtension, 1, i);
        ERR_(IHVVIDEO, "Second Descriptor NULL\n");
        VideoPortReleaseSpinLock((PVOID)VirtioGpuMpDeviceExtension, VirtioGpuMpDeviceExtension->CursorLock, OldIrql);
        return FALSE;
    }

    PVOID Res = (PVOID)VideoPortAllocatePool((PVOID)VirtioGpuMpDeviceExtension, VpNonPagedPoolCacheAligned,
            ResponseLength, VGPU_TAG);
    VideoPortZeroMemory(Res, ResponseLength);
    *Response = Res;

    Descriptor->Address = MmGetPhysicalAddress(Res).QuadPart;
    Descriptor->Length = ResponseLength;
    Descriptor->Flags |= VIRTQ_DESC_F_WRITE;

    VirtioGpuMpSubmitDescriptor(VirtioGpuMpDeviceExtension, 1, i, Next);
    VirtioGpuMpFreeDescriptor(VirtioGpuMpDeviceExtension, 1, Next);

    VideoPortReleaseSpinLock((PVOID)VirtioGpuMpDeviceExtension, VirtioGpuMpDeviceExtension->CursorLock, OldIrql);
    return TRUE;
}

BOOLEAN
FASTCALL
VirtioGpuMpSaveModes(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension
)
{
    if(!VirtioGpuMpDeviceExtension->ModesSaved)
    {
        PVIRTIOGPUMP_CONTROL_HEADER Header = (PVIRTIOGPUMP_CONTROL_HEADER)VideoPortAllocatePool(
            (PVOID)VirtioGpuMpDeviceExtension, VpNonPagedPoolCacheAligned,
            sizeof(VIRTIOGPUMP_CONTROL_HEADER), VGPU_TAG);
        Header->Type = VIRTIO_GPU_CMD_GET_DISPLAY_INFO;

        PVIRTIOGPUMP_DISPLAY_INFO Info = NULL;
        if(VirtioGpuMpSendCmdCommandResponse(VirtioGpuMpDeviceExtension,
            (PVOID)Header, sizeof(VIRTIOGPUMP_CONTROL_HEADER),
            (PVOID *)&Info, sizeof(VIRTIOGPUMP_DISPLAY_INFO)) == FALSE)
            return FALSE;

        for(INT i = 0; i< 16; i++)
        {
            if(Info->Modes[i].Enabled)
                VirtioGpuMpDeviceExtension->EnabledModeCount++;
            VirtioGpuMpDeviceExtension->SavedModes[i] = Info->Modes[i];
            ERR_(IHVVIDEO, "%d: x %d, y %d, w %d, h %d, flags 0x%x\n",
                i, Info->Modes[i].Rect.X, Info->Modes[i].Rect.Y,
                Info->Modes[i].Rect.Width, Info->Modes[i].Rect.Height, Info->Modes[i].Flags);
        }

        VirtioGpuMpDeviceExtension->ModesSaved = TRUE;

        VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Header);
        VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Info);
    }

    return TRUE;
}

VOID
NTAPI
FlushDpcRoutine(IN PKDPC Dpc,
                IN PVOID DeferredContext,
                IN PVOID SystemArgument1,
                IN PVOID SystemArgument2);

/*
 * VirtioGpuMpInitialize
 *
 * Performs the first initialization of the adapter, after the HAL has given
 * up control of the video hardware to the video port driver.
 */

BOOLEAN
NTAPI
VirtioGpuMpInitialize(IN PVOID HwDeviceExtension)
{
    UNIMPLEMENTED;
    TRACE_(IHVVIDEO, "VirtioGpuMpInitialize\n");

    PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension;
    ULONG inIoSpace = VIDEO_MEMORY_SPACE_MEMORY;

    VirtioGpuMpDeviceExtension = (PVIRTIOGPUMP_DEVICE_EXTENSION)HwDeviceExtension;

    for(INT i = 1; i < 5; i++) {
        if(VirtioGpuMpDeviceExtension->BarsToMapLength[i] != 0)
        {
            if(VideoPortMapMemory(HwDeviceExtension,
                          VirtioGpuMpDeviceExtension->BarsToMapPhysStart[i],
                          &VirtioGpuMpDeviceExtension->BarsToMapLength[i],
                          &inIoSpace,
                          &VirtioGpuMpDeviceExtension->BarsToMapVirtStart[i]) != NO_ERROR)
            {
                ERR_(IHVVIDEO, "Failed to map memory\n");
                return FALSE;
            }
        } else {
            VirtioGpuMpDeviceExtension->BarsToMapVirtStart[i] = NULL;
        }
    }

    PVIRTIOGPUMP_PCI_CAP_HEADER TempCapHeader = &VirtioGpuMpDeviceExtension->RegistersBarCap;
    VirtioGpuMpDeviceExtension->VirtRegistersStart = (PVOID)((ULONG_PTR)VirtioGpuMpDeviceExtension->BarsToMapVirtStart[TempCapHeader->Index-1] + TempCapHeader->Offset);
    TempCapHeader = &VirtioGpuMpDeviceExtension->NotifyBarCap;
    VirtioGpuMpDeviceExtension->VirtNotifyStart = (PVOID)((ULONG_PTR)VirtioGpuMpDeviceExtension->BarsToMapVirtStart[TempCapHeader->Index-1] + TempCapHeader->Offset);
    TempCapHeader = &VirtioGpuMpDeviceExtension->IsrBarCap;
    VirtioGpuMpDeviceExtension->VirtIsrStart = (PVOID)((ULONG_PTR)VirtioGpuMpDeviceExtension->BarsToMapVirtStart[TempCapHeader->Index-1] + TempCapHeader->Offset);
    TempCapHeader = &VirtioGpuMpDeviceExtension->ConfigBarCap;
    VirtioGpuMpDeviceExtension->VirtConfigStart = (PVOID)((ULONG_PTR)VirtioGpuMpDeviceExtension->BarsToMapVirtStart[TempCapHeader->Index-1] + TempCapHeader->Offset);

    //ERR_ to be replace with info to see it better
    ERR_(IHVVIDEO, "Register mapped to 0x%x\n",
        VirtioGpuMpDeviceExtension->VirtRegistersStart);

    //Lets init the driver communication with VirtioGPU here

    VirtioGpuMpDeviceExtension->Registers = (PVIRTIOGPUMP_REGISTERS)VirtioGpuMpDeviceExtension->VirtRegistersStart;
    VirtioGpuMpDeviceExtension->Notify = (volatile PVOID)VirtioGpuMpDeviceExtension->VirtNotifyStart;
    VirtioGpuMpDeviceExtension->IsrStatus = (volatile PULONG)VirtioGpuMpDeviceExtension->VirtIsrStart;
    VirtioGpuMpDeviceExtension->DeviceConfig = (volatile PVIRTIOGPUMP_CONFIG)VirtioGpuMpDeviceExtension->VirtConfigStart;

    VirtioGpuMpDeviceExtension->Registers->deviceStatus = 0;
    while(VirtioGpuMpDeviceExtension->Registers->deviceStatus != 0);

    VirtioGpuMpDeviceExtension->Registers->deviceStatus |= VIRTIO_DEVICE_ACKNOWLEDGED;
    VirtioGpuMpDeviceExtension->Registers->deviceStatus |= VIRTIO_DRIVER_LOADED;

    VirtioGpuMpDeviceExtension->Registers->deviceFeaturesSel = 0x00;
    ULONG deviceFeaturesLow = VirtioGpuMpDeviceExtension->Registers->deviceFeatures;
    VirtioGpuMpDeviceExtension->Registers->deviceFeaturesSel = 0x00;
    ULONG deviceFeaturesHigh = VirtioGpuMpDeviceExtension->Registers->deviceFeatures;

    ERR_(IHVVIDEO, "Virtio GPU supports Features High: 0x%x, Low: 0x%x\n",
        deviceFeaturesHigh, deviceFeaturesLow);

    VirtioGpuMpDeviceExtension->Registers->guestFeaturesSel = 0x00;
    VirtioGpuMpDeviceExtension->Registers->guestFeatures = 0x1;
    VirtioGpuMpDeviceExtension->Registers->guestFeaturesSel = 0x01;
    VirtioGpuMpDeviceExtension->Registers->guestFeatures = 0x1;

    VirtioGpuMpDeviceExtension->Registers->deviceStatus |= VIRTIO_FEATURES_OK;

    //TODO: Look at this better
    if(!(VirtioGpuMpDeviceExtension->Registers->deviceStatus & VIRTIO_FEATURES_OK)) return FALSE;

    VirtioGpuMpDeviceExtension->Registers->deviceStatus |= VIRTIO_DRIVER_READY;

    //TODO: Allocate virtual queues here
    VirtioGpuMpDeviceExtension->VirtualQueues =
        (PVIRTIOGPUMP_QUEUE_VIRTQUEUE)VideoPortAllocatePool(HwDeviceExtension, VpNonPagedPool,
            sizeof(VIRTIOGPUMP_QUEUE_VIRTQUEUE) * (VirtioGpuMpDeviceExtension->Registers->numQueues + 1), VGPU_TAG);
    VirtioGpuMpDeviceExtension->ActiveQueues = 0;

    VirtioGpuMpSetupQueue(VirtioGpuMpDeviceExtension, 0);
    VirtioGpuMpSetupQueue(VirtioGpuMpDeviceExtension, 1);

    KeInitializeTimer(&VirtioGpuMpDeviceExtension->FlushTimer);
    KeInitializeDpc(&VirtioGpuMpDeviceExtension->FlushDpc, FlushDpcRoutine, (PVOID)VirtioGpuMpDeviceExtension);
    VideoPortCreateSpinLock((PVOID)VirtioGpuMpDeviceExtension, &VirtioGpuMpDeviceExtension->CommandLock);
    VideoPortCreateSpinLock((PVOID)VirtioGpuMpDeviceExtension, &VirtioGpuMpDeviceExtension->CursorLock);
    VideoPortCreateSpinLock((PVOID)VirtioGpuMpDeviceExtension, &VirtioGpuMpDeviceExtension->DpcLock);
    VideoPortCreateSpinLock((PVOID)VirtioGpuMpDeviceExtension, &VirtioGpuMpDeviceExtension->CommandCompleteLock);

    VirtioGpuMpDeviceExtension->ResourceCurrentAllocatorIdx = 1;
    VirtioGpuMpDeviceExtension->ContextCurrentAllocatorIdx = 1;

    //Allocate the framebuffer, get it bigger enought to be able to fix all scanouts possible
    if(!VirtioGpuMpSaveModes(VirtioGpuMpDeviceExtension))
        return FALSE;

    ULONG MaxSize = 0;
    for(INT i = 0; i < 16; i++)
    {
        PVIRTIOGPUMP_MODE CurrentMode = &VirtioGpuMpDeviceExtension->SavedModes[i];
        if(CurrentMode->Enabled)
        {
            ULONG CurrentSize = CurrentMode->Rect.Width * CurrentMode->Rect.Height * 4; //Each pixel is four byte size
            if(CurrentSize > MaxSize) MaxSize = CurrentSize;
        }
    }

    InitializeListHead(&VirtioGpuMpDeviceExtension->Resource2DListHead);

    LARGE_INTEGER DueTime;
    DueTime.QuadPart = -4000;
    KeSetTimerEx(&VirtioGpuMpDeviceExtension->FlushTimer, DueTime, 16, &VirtioGpuMpDeviceExtension->FlushDpc);

    return TRUE;
}

BOOLEAN
FASTCALL
VirtioGpuMpQueryNumAvailModes(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension,
    OUT PVIDEO_NUM_MODES Modes,
    PSTATUS_BLOCK StatusBlock)
{
    if(!VirtioGpuMpSaveModes(VirtioGpuMpDeviceExtension))
        return FALSE;

    //Get available modes
    Modes->NumModes = VirtioGpuMpDeviceExtension->EnabledModeCount;
    Modes->ModeInformationLength = sizeof(VIDEO_MODE_INFORMATION);
    StatusBlock->Information = sizeof(VIDEO_NUM_MODES); //Modes Length

    return TRUE;
}

BOOLEAN
FASTCALL
VirtioGpuMpQueryAvailModes(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension,
    OUT PVIDEO_MODE_INFORMATION Modes,
    IN ULONG BufferLength,
    PSTATUS_BLOCK StatusBlock)
{
    if(!VirtioGpuMpSaveModes(VirtioGpuMpDeviceExtension))
        return FALSE;

    if (BufferLength < (sizeof(VIDEO_NUM_MODES) * VirtioGpuMpDeviceExtension->EnabledModeCount))
    {
        StatusBlock->Status = ERROR_INSUFFICIENT_BUFFER;
        return TRUE;
    }

    INT CurrentModeIdx = 0;
    for(INT i = 0; i < 16; i++) {
        if(VirtioGpuMpDeviceExtension->SavedModes[i].Enabled) {
            PVIDEO_MODE_INFORMATION VideoMode = &Modes[CurrentModeIdx];
            PVIRTIOGPUMP_MODE CurrentMode = &VirtioGpuMpDeviceExtension->SavedModes[i];

            VideoMode->Length = sizeof(VIDEO_MODE_INFORMATION);
            VideoMode->ModeIndex = CurrentModeIdx;

            VideoMode->VisScreenWidth = CurrentMode->Rect.Width;
            VideoMode->VisScreenHeight = CurrentMode->Rect.Height;

            VideoMode->ScreenStride = VideoMode->VisScreenWidth * 4; //Use 32 bits per pixel
            VideoMode->NumberOfPlanes = 1;
            VideoMode->BitsPerPlane = 32;
            VideoMode->Frequency = 60;
            VideoMode->XMillimeter = 0; /* FIXME */
            VideoMode->YMillimeter = 0; /* FIXME */

            VideoMode->NumberRedBits = 8;
            VideoMode->NumberGreenBits = 8;
            VideoMode->NumberBlueBits = 8;
            VideoMode->RedMask = 0xFF0000;
            VideoMode->GreenMask = 0x00FF00;
            VideoMode->BlueMask = 0x0000FF;

            VideoMode->VideoMemoryBitmapWidth = VideoMode->VisScreenWidth;
            VideoMode->VideoMemoryBitmapHeight = VideoMode->VisScreenHeight;
            VideoMode->AttributeFlags = VIDEO_MODE_GRAPHICS | VIDEO_MODE_COLOR |
                VIDEO_MODE_NO_OFF_SCREEN;
            VideoMode->DriverSpecificAttributeFlags = 0;

            CurrentModeIdx++;
        }

        if(CurrentModeIdx >= VirtioGpuMpDeviceExtension->EnabledModeCount) break;
    }

    StatusBlock->Information = sizeof(VIDEO_MODE_INFORMATION) * VirtioGpuMpDeviceExtension->EnabledModeCount;

    return TRUE;
}

BOOLEAN
FASTCALL
VirtioGpuMpSetCurrentMode(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension,
    IN PVIDEO_MODE Mode,
    PSTATUS_BLOCK StatusBlock)
{
    if(!VirtioGpuMpSaveModes(VirtioGpuMpDeviceExtension))
        return FALSE;

    //Zero out upper 2 bits
    ULONG RequestMode = Mode->RequestedMode & ~(((ULONG)(LONG)-1 >> 1) & ((ULONG)(LONG)-1));
    if(RequestMode >= 16)
        return FALSE;

    PVIRTIOGPUMP_MODE RequestModePointer = &VirtioGpuMpDeviceExtension->SavedModes[RequestMode];

    if(VirtioGpuMpDeviceExtension->ScanoutResourceID != 0)
    {
        PVIRTIOGPUMP_RESOURCE_UNREF Request = (PVIRTIOGPUMP_RESOURCE_UNREF)VideoPortAllocatePool(
            (PVOID)VirtioGpuMpDeviceExtension, VpNonPagedPoolCacheAligned,
            sizeof(VIRTIOGPUMP_RESOURCE_UNREF), VGPU_TAG);

        Request->Header.Type = VIRTIO_GPU_CMD_RESOURCE_UNREF;
        Request->ResourceID = VirtioGpuMpDeviceExtension-> ScanoutResourceID;

        PVIRTIOGPUMP_CONTROL_HEADER Res = NULL;
        if(VirtioGpuMpSendCmdCommandResponse(VirtioGpuMpDeviceExtension,
            (PVOID)Request, sizeof(VIRTIOGPUMP_RESOURCE_UNREF),
            (PVOID *)&Res, sizeof(VIRTIOGPUMP_CONTROL_HEADER)) == FALSE)
            return FALSE;

        VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Request);
        VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Res);
    }
    else
        VirtioGpuMpDeviceExtension-> ScanoutResourceID = VirtioGpuMpDeviceExtension->ResourceCurrentAllocatorIdx++;

    //Create new scanout resource
    {
        PVIRTIOGPUMP_RESOURCE_CREATE_2D Request = (PVIRTIOGPUMP_RESOURCE_CREATE_2D)VideoPortAllocatePool(
            (PVOID)VirtioGpuMpDeviceExtension, VpNonPagedPoolCacheAligned,
            sizeof(VIRTIOGPUMP_RESOURCE_CREATE_2D), VGPU_TAG);

        Request->Header.Type = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D;
        Request->ResourceID = VirtioGpuMpDeviceExtension-> ScanoutResourceID;
        Request->Format = VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM;
        Request->Width = RequestModePointer->Rect.Width;
        Request->Height = RequestModePointer->Rect.Height;

        PVIRTIOGPUMP_CONTROL_HEADER Res = NULL;
        if(VirtioGpuMpSendCmdCommandResponse(VirtioGpuMpDeviceExtension,
            (PVOID)Request, sizeof(VIRTIOGPUMP_RESOURCE_CREATE_2D),
            (PVOID *)&Res, sizeof(VIRTIOGPUMP_CONTROL_HEADER)) == FALSE)
            return FALSE;

        VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Request);
        VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Res);
    }

    //Attach the framebuffer
    {
        PVIRTIOGPUMP_RESOURCE_ATTACH_BACKING Request = (PVIRTIOGPUMP_RESOURCE_ATTACH_BACKING)VideoPortAllocatePool(
            (PVOID)VirtioGpuMpDeviceExtension, VpNonPagedPoolCacheAligned,
            sizeof(VIRTIOGPUMP_RESOURCE_ATTACH_BACKING) + sizeof(VIRTIOGPUMP_MEM_ENTRY), VGPU_TAG);
        PVIRTIOGPUMP_MEM_ENTRY Entries = (PVIRTIOGPUMP_MEM_ENTRY)((ULONG_PTR)Request + sizeof(VIRTIOGPUMP_RESOURCE_ATTACH_BACKING));

        Request->Header.Type = VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING;
        Request->ResourceID = VirtioGpuMpDeviceExtension->ScanoutResourceID;
        Request->EntryCount = 1;
        Entries[0].Address = VirtioGpuMpDeviceExtension->PhysFramebuffer.QuadPart;
        Entries[0].Length = RequestModePointer->Rect.Width * RequestModePointer->Rect.Height * 4;

        PVIRTIOGPUMP_CONTROL_HEADER Res = NULL;
        if(VirtioGpuMpSendCmdCommandResponse(VirtioGpuMpDeviceExtension,
            (PVOID)Request, sizeof(VIRTIOGPUMP_RESOURCE_ATTACH_BACKING) + sizeof(VIRTIOGPUMP_MEM_ENTRY),
            (PVOID *)&Res, sizeof(VIRTIOGPUMP_CONTROL_HEADER)) == FALSE)
            return FALSE;

        VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Request);
        VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Res);
    }

    PVIRTIOGPUMP_SET_SCANOUT Request = (PVIRTIOGPUMP_SET_SCANOUT)VideoPortAllocatePool(
        (PVOID)VirtioGpuMpDeviceExtension, VpNonPagedPoolCacheAligned,
        sizeof(VIRTIOGPUMP_SET_SCANOUT), VGPU_TAG);
    Request->Header.Type = VIRTIO_GPU_CMD_SET_SCANOUT;
    Request->Rect.X = Request->Rect.Y = 0;
    Request->Rect.Width = RequestModePointer->Rect.Width;
    Request->Rect.Height = RequestModePointer->Rect.Height;
    Request->ResourceID = VirtioGpuMpDeviceExtension->ScanoutResourceID;
    Request->ScanoutID = RequestMode;

    PVIRTIOGPUMP_CONTROL_HEADER Res = NULL;
    if(VirtioGpuMpSendCmdCommandResponse(VirtioGpuMpDeviceExtension,
        (PVOID)Request, sizeof(VIRTIOGPUMP_SET_SCANOUT),
        (PVOID *)&Res, sizeof(VIRTIOGPUMP_CONTROL_HEADER)) == FALSE)
        return FALSE;

    VirtioGpuMpDeviceExtension->CurrentMode = RequestMode;

    VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Request);
    VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Res);

    StatusBlock->Information = sizeof(PVIDEO_MODE); //Modes Length

    return TRUE;
}

BOOLEAN
FASTCALL
VirtioGpuMpMapVideoMemory(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension,
    IN PVIDEO_MEMORY VideoMemory,
    OUT PVIDEO_MEMORY_INFORMATION VideoMemoryInfo,
    PSTATUS_BLOCK StatusBlock)
{
    PVIRTIOGPUMP_MODE CurrentMode =
        &VirtioGpuMpDeviceExtension->SavedModes[VirtioGpuMpDeviceExtension->CurrentMode];

    VideoMemoryInfo->VideoRamBase = VideoMemory->RequestedVirtualAddress;
    VideoMemoryInfo->VideoRamLength = CurrentMode->Rect.Width * CurrentMode->Rect.Height * 4;

    VP_STATUS Status;
    ULONG InIoSpace = VIDEO_MEMORY_SPACE_MEMORY;
    if((Status = VideoPortMapMemory((PVOID)VirtioGpuMpDeviceExtension,
                    VirtioGpuMpDeviceExtension->PhysFramebuffer,
                    &VirtioGpuMpDeviceExtension->PhysFramebufferLength,
                    &InIoSpace,
                    &VideoMemoryInfo->VideoRamBase)) != NO_ERROR)
    {
        ERR_(IHVVIDEO, "Failed to map video memory\n");
        StatusBlock->Status = Status;
        return FALSE;
    }

    VideoMemoryInfo->FrameBufferBase = VideoMemoryInfo->VideoRamBase;
    VideoMemoryInfo->FrameBufferLength = VideoMemoryInfo->VideoRamLength;

    StatusBlock->Information = sizeof(VIDEO_MEMORY_INFORMATION);

    return TRUE;
}

BOOLEAN
FASTCALL
VirtioGpuMpUnmapVideoMemory(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension,
    IN PVIDEO_MEMORY VideoMemory,
    PSTATUS_BLOCK StatusBlock)
{
    VP_STATUS Status;
    if((Status = VideoPortUnmapMemory((PVOID)VirtioGpuMpDeviceExtension,
                    VideoMemory->RequestedVirtualAddress, NULL)) != NO_ERROR)
    {
        ERR_(IHVVIDEO, "Failed to map video memory\n");
        StatusBlock->Status = Status;
        return FALSE;
    }

    StatusBlock->Information = sizeof(PVIDEO_MEMORY);

    return TRUE;
}

BOOLEAN
FASTCALL
VirtioGpuMpReset(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension,
    PSTATUS_BLOCK StatusBlock)
{
    //TODO: Research

    return TRUE;
}

VOID
VirtioGpuMpCancelFlushTimer(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension,
    OUT PKIRQL OldIrql
)
{
    //VideoPortAcquireSpinLock((PVOID)VirtioGpuMpDeviceExtension, VirtioGpuMpDeviceExtension->Lock, OldIrql);
    //KeCancelTimer(&VirtioGpuMpDeviceExtension->FlushTimer);
    //while(!VirtioGpuMpDeviceExtension->DpcFinished);
    //VideoPortAcquireDeviceLock((PVOID)VirtioGpuMpDeviceExtension);
}

VOID
VirtioGpuMpStartFlushTimer(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension,
    IN KIRQL NewIrql
)
{
    //VideoPortReleaseSpinLock((PVOID)VirtioGpuMpDeviceExtension, VirtioGpuMpDeviceExtension->Lock, NewIrql);
    //LARGE_INTEGER DueTime;
    //DueTime.QuadPart = -4000;
    //KeSetTimerEx(&VirtioGpuMpDeviceExtension->FlushTimer, DueTime, 16, &VirtioGpuMpDeviceExtension->FlushDpc);
    //VideoPortReleaseDeviceLock((PVOID)VirtioGpuMpDeviceExtension);
}

BOOLEAN
FASTCALL
VirtioGpuMpCreate2DResource(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension,
    IN PVIDEO_CREATE_2D_RESOURCE CreateResource,
    OUT PVIDEO_RESOURCE_ID ResourceIDBuffer,
    PSTATUS_BLOCK StatusBlock)
{
    ULONG Width = CreateResource->Width;
    ULONG Height = CreateResource->Height;

    {
        PVIRTIOGPUMP_RESOURCE_CREATE_2D Request = (PVIRTIOGPUMP_RESOURCE_CREATE_2D)VideoPortAllocatePool(
            (PVOID)VirtioGpuMpDeviceExtension, VpNonPagedPoolCacheAligned,
            sizeof(VIRTIOGPUMP_RESOURCE_CREATE_2D), VGPU_TAG);

        Request->Header.Type = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D;
        Request->ResourceID = VirtioGpuMpDeviceExtension->ResourceCurrentAllocatorIdx++;
        Request->Format = CreateResource->Format;
        Request->Width = Width;
        Request->Height = Height;

        ResourceIDBuffer->ResourceID = Request->ResourceID;

        PVIRTIOGPUMP_CONTROL_HEADER Res = NULL;
        if(VirtioGpuMpSendCmdCommandResponse(VirtioGpuMpDeviceExtension,
            (PVOID)Request, sizeof(VIRTIOGPUMP_RESOURCE_CREATE_2D),
            (PVOID *)&Res, sizeof(VIRTIOGPUMP_CONTROL_HEADER)) == FALSE)
            return FALSE;

        VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Request);
        VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Res);
    }

    ULONG DesiredLength = Width * Height * 4;
    PHYSICAL_ADDRESS PhysAddress;
    PVOID VirtAddress = VideoPortAllocateCommonBuffer((PVOID)VirtioGpuMpDeviceExtension,
                            VirtioGpuMpDeviceExtension->DmaAdapter, DesiredLength, &PhysAddress, FALSE, NULL);
    //PVOID VirtAddress = ExAllocatePoolWithTag(NonPagedPool, DesiredLength, VGPU_TAG);
    if(VirtAddress == NULL)
        return FALSE;

    //PhysAddress = MmGetPhysicalAddress(VirtAddress);

    //VP_STATUS Status;
    //ULONG InIoSpace = VIDEO_MEMORY_SPACE_MEMORY;
    //VirtAddress = NULL;
    //if((Status = VideoPortMapMemory((PVOID)VirtioGpuMpDeviceExtension,
    //                PhysAddress,
    //                &DesiredLength,
    //                &InIoSpace,
    //                &VirtAddress)) != NO_ERROR)
    //{
    //    ERR_(IHVVIDEO, "Failed to map video memory\n");
    //    StatusBlock->Status = Status;
    //    return FALSE;
    //}

    PRESOURCE_2D_ENTRY ResourceEntry = (PRESOURCE_2D_ENTRY)ExAllocatePoolWithTag(PagedPool, sizeof(RESOURCE_2D_ENTRY), VGPU_TAG);
    ResourceEntry->ResourceID = ResourceIDBuffer->ResourceID;
    ResourceEntry->PhysAddressNonAligned = PhysAddress;
    ResourceEntry->PhysAddress = PAGE_ALIGN(PhysAddress.QuadPart);
    ResourceEntry->VirtAddressNonAligned = VirtAddress;
    ResourceEntry->VirtAddress = PAGE_ALIGN(VirtAddress);
    ResourceEntry->BufferLength = DesiredLength;

    InsertTailList(&VirtioGpuMpDeviceExtension->Resource2DListHead, (PLIST_ENTRY)ResourceEntry);

    {
        PVIRTIOGPUMP_RESOURCE_ATTACH_BACKING Request = (PVIRTIOGPUMP_RESOURCE_ATTACH_BACKING)VideoPortAllocatePool(
            (PVOID)VirtioGpuMpDeviceExtension, VpNonPagedPoolCacheAligned,
            sizeof(VIRTIOGPUMP_RESOURCE_ATTACH_BACKING) + sizeof(VIRTIOGPUMP_MEM_ENTRY), VGPU_TAG);
        PVIRTIOGPUMP_MEM_ENTRY Entries = (PVIRTIOGPUMP_MEM_ENTRY)((ULONG_PTR)Request + sizeof(VIRTIOGPUMP_RESOURCE_ATTACH_BACKING));

        Request->Header.Type = VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING;
        Request->ResourceID = ResourceEntry->ResourceID;
        Request->EntryCount = 1;
        Entries[0].Address = (ULONG_PTR)ResourceEntry->PhysAddress;
        Entries[0].Length = DesiredLength;

        PVIRTIOGPUMP_CONTROL_HEADER Res = NULL;
        if(VirtioGpuMpSendCmdCommandResponse(VirtioGpuMpDeviceExtension,
            (PVOID)Request, sizeof(VIRTIOGPUMP_RESOURCE_ATTACH_BACKING) + sizeof(VIRTIOGPUMP_MEM_ENTRY),
            (PVOID *)&Res, sizeof(VIRTIOGPUMP_CONTROL_HEADER)) == FALSE)
            return FALSE;

        VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Request);
        VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Res);
    }

    StatusBlock->Information = sizeof(PVIDEO_CREATE_2D_RESOURCE); //Modes Length

    return TRUE;
}

BOOLEAN
FASTCALL
VirtioGpuMpResourceTransfer(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension,
    IN PVIDEO_TRANSFER_2D FlushCtx,
    PSTATUS_BLOCK StatusBlock)
{
    PLIST_ENTRY temp = &VirtioGpuMpDeviceExtension->Resource2DListHead;
    PRESOURCE_2D_ENTRY ResourceEntry = NULL;
    while(&VirtioGpuMpDeviceExtension->Resource2DListHead != temp->Flink)
    {
        temp = temp->Flink;
        PRESOURCE_2D_ENTRY Resource = CONTAINING_RECORD(temp, RESOURCE_2D_ENTRY, ListEntry);
        if(Resource->ResourceID == FlushCtx->ResourceID)
        {
            ResourceEntry = Resource;
            break;
        }
    }
    if(ResourceEntry == NULL)
        return FALSE;

    if(FlushCtx->BufferLength != 0)
        RtlCopyMemory(ResourceEntry->VirtAddress, (PVOID)((ULONG_PTR)FlushCtx + sizeof(VIDEO_TRANSFER_2D)), (FlushCtx->BufferLength > ResourceEntry->BufferLength) ? ResourceEntry->BufferLength : FlushCtx->BufferLength);

    {
        PVIRTIOGPUMP_TRANSFER_TO_HOST_2D Request = (PVIRTIOGPUMP_TRANSFER_TO_HOST_2D)VideoPortAllocatePool(
            (PVOID)VirtioGpuMpDeviceExtension, VpNonPagedPoolCacheAligned,
            sizeof(VIRTIOGPUMP_TRANSFER_TO_HOST_2D), VGPU_TAG);
        Request->Header.Type = VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D;
        Request->Rect.X = FlushCtx->X;
        Request->Rect.Y = FlushCtx->Y;
        Request->Rect.Width = FlushCtx->Width;
        Request->Rect.Height = FlushCtx->Height;
        Request->ResourceID = FlushCtx->ResourceID;
        Request->Offset = 0;

        ERR_(IHVVIDEO, "Width: %d, Height: %d\n", FlushCtx->Width, FlushCtx->Height);

        PVIRTIOGPUMP_CONTROL_HEADER Res = NULL;
        if(VirtioGpuMpSendCmdCommandResponse(VirtioGpuMpDeviceExtension,
            (PVOID)Request, sizeof(VIRTIOGPUMP_TRANSFER_TO_HOST_2D),
            (PVOID *)&Res, sizeof(VIRTIOGPUMP_CONTROL_HEADER)) == FALSE)
            return FALSE;

        VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Request);
        VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Res);
    }

    //{
    //    PVIRTIOGPUMP_RESOURCE_FLUSH Request = (PVIRTIOGPUMP_RESOURCE_FLUSH)VideoPortAllocatePool(
    //        (PVOID)VirtioGpuMpDeviceExtension, VpNonPagedPoolCacheAligned,
    //        sizeof(VIRTIOGPUMP_RESOURCE_FLUSH), VGPU_TAG);
//
    //    Request->Header.Type = VIRTIO_GPU_CMD_RESOURCE_FLUSH;
    //    Request->ResourceID = FlushCtx->ResourceID;
    //    Request->Rect.Width = FlushCtx->Width;
    //    Request->Rect.Height = FlushCtx->Height;
    //    Request->Rect.X = FlushCtx->X;
    //    Request->Rect.Y = FlushCtx->Y;
//
    //    PVIRTIOGPUMP_CONTROL_HEADER Res = NULL;
    //    if(VirtioGpuMpSendCmdCommandResponse(VirtioGpuMpDeviceExtension,
    //        (PVOID)Request, sizeof(VIRTIOGPUMP_RESOURCE_FLUSH),
    //        (PVOID *)&Res, sizeof(VIRTIOGPUMP_CONTROL_HEADER)) == FALSE)
    //        return FALSE;
//
    //    VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Request);
    //    VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Res);
    //}

    StatusBlock->Information = sizeof(PVIDEO_TRANSFER_2D);

    return TRUE;
}

BOOLEAN
FASTCALL
VirtioGpuMpUpdateMoveCursor(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension,
    IN PVIDEO_CURSOR CursorCtx,
    PSTATUS_BLOCK StatusBlock,
    BOOLEAN IsUpdate)
{
    {
        PVIRTIOGPUMP_UPDATE_CURSOR Request = (PVIRTIOGPUMP_UPDATE_CURSOR)VideoPortAllocatePool(
            (PVOID)VirtioGpuMpDeviceExtension, VpNonPagedPoolCacheAligned,
            sizeof(VIRTIOGPUMP_UPDATE_CURSOR), VGPU_TAG);

        Request->Header.Type = IsUpdate ? VIRTIO_GPU_CMD_UPDATE_CURSOR : VIRTIO_GPU_CMD_MOVE_CURSOR;
        Request->ResourceID = CursorCtx->ResourceID;
        Request->HotX = CursorCtx->HotX;
        Request->HotY = CursorCtx->HotY;
        Request->Pos.X = CursorCtx->X;
        Request->Pos.Y = CursorCtx->Y;
        Request->Pos.ScanoutID = VirtioGpuMpDeviceExtension->CurrentMode;

        PVIRTIOGPUMP_CONTROL_HEADER Res = NULL;
        if(VirtioGpuMpSendCursorCommandResponse(VirtioGpuMpDeviceExtension,
            (PVOID)Request, sizeof(VIRTIOGPUMP_UPDATE_CURSOR),
            (PVOID *)&Res, sizeof(VIRTIOGPUMP_CONTROL_HEADER)) == FALSE)
            return FALSE;

        VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Request);
        VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Res);
    }

    StatusBlock->Information = sizeof(PVIDEO_CURSOR);

    return TRUE;
}

BOOLEAN
FASTCALL
VirtioGpuMpFlushScreen(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension,
    PSTATUS_BLOCK StatusBlock)
{
    if(VirtioGpuMpDeviceExtension->ScanoutResourceID)
    {
        PVIRTIOGPUMP_MODE CurrentMode =
            &VirtioGpuMpDeviceExtension->SavedModes[VirtioGpuMpDeviceExtension->CurrentMode];

        {
            PVIRTIOGPUMP_TRANSFER_TO_HOST_2D Request = (PVIRTIOGPUMP_TRANSFER_TO_HOST_2D)VideoPortAllocatePool(
                (PVOID)VirtioGpuMpDeviceExtension, VpNonPagedPoolCacheAligned,
                sizeof(VIRTIOGPUMP_TRANSFER_TO_HOST_2D), VGPU_TAG);
            Request->Header.Type = VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D;
            Request->Rect.X = Request->Rect.Y = 0;
            Request->Rect.Width = CurrentMode->Rect.Width;
            Request->Rect.Height = CurrentMode->Rect.Height;
            Request->ResourceID = VirtioGpuMpDeviceExtension->ScanoutResourceID;
            Request->Offset = 0;

            PVIRTIOGPUMP_CONTROL_HEADER Res = NULL;
            if(VirtioGpuMpSendCmdCommandResponse(VirtioGpuMpDeviceExtension,
                (PVOID)Request, sizeof(VIRTIOGPUMP_TRANSFER_TO_HOST_2D),
                (PVOID *)&Res, sizeof(VIRTIOGPUMP_CONTROL_HEADER)) == FALSE)
                return FALSE;

            VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Request);
            VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Res);
        }

        {
            PVIRTIOGPUMP_RESOURCE_FLUSH Request = (PVIRTIOGPUMP_RESOURCE_FLUSH)VideoPortAllocatePool(
                (PVOID)VirtioGpuMpDeviceExtension, VpNonPagedPoolCacheAligned,
                sizeof(VIRTIOGPUMP_RESOURCE_FLUSH), VGPU_TAG);
            Request->Header.Type = VIRTIO_GPU_CMD_RESOURCE_FLUSH;
            Request->Rect.X = Request->Rect.Y = 0;
            Request->Rect.Width = CurrentMode->Rect.Width;
            Request->Rect.Height = CurrentMode->Rect.Height;
            Request->ResourceID = VirtioGpuMpDeviceExtension->ScanoutResourceID;

            PVIRTIOGPUMP_CONTROL_HEADER Res = NULL;
            if(VirtioGpuMpSendCmdCommandResponse(VirtioGpuMpDeviceExtension,
                (PVOID)Request, sizeof(VIRTIOGPUMP_RESOURCE_FLUSH),
                (PVOID *)&Res, sizeof(VIRTIOGPUMP_CONTROL_HEADER)) == FALSE)
                return FALSE;

            VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Request);
            VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Res);
        }
    }

    StatusBlock->Information = 0;

    return TRUE;
}

BOOLEAN
FASTCALL
VirtioGpuMpCreateContext(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension,
    IN PVIDEO_CREATE_CONTEXT CreateContext,
    OUT PVIDEO_RESOURCE_ID ContextIDBuffer,
    PSTATUS_BLOCK StatusBlock)
{
    {
        PVIRTIOGPUMP_CREATE_CONTEXT Request = (PVIRTIOGPUMP_CREATE_CONTEXT)VideoPortAllocatePool(
            (PVOID)VirtioGpuMpDeviceExtension, VpNonPagedPoolCacheAligned,
            sizeof(VIRTIOGPUMP_CREATE_CONTEXT), VGPU_TAG);

        Request->Header.Type = VIRTIO_GPU_CMD_CTX_CREATE;
        Request->Header.CtxID = VirtioGpuMpDeviceExtension->ContextCurrentAllocatorIdx++;
        Request->NameLength = CreateContext->NameLength;
        RtlCopyMemory(Request->DebugName, CreateContext->Name, Request->NameLength);

        ContextIDBuffer->ResourceID = Request->Header.CtxID;

        PVIRTIOGPUMP_CONTROL_HEADER Res = NULL;
        if(VirtioGpuMpSendCmdCommandResponse(VirtioGpuMpDeviceExtension,
            (PVOID)Request, sizeof(VIRTIOGPUMP_CREATE_CONTEXT),
            (PVOID *)&Res, sizeof(VIRTIOGPUMP_CONTROL_HEADER)) == FALSE)
        {
            return FALSE;
        }

        VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Request);
        VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Res);
    }

    StatusBlock->Information = sizeof(PVIDEO_CREATE_CONTEXT);

    return TRUE;
}

BOOLEAN
FASTCALL
VirtioGpuMpSetContext(
    IN PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension,
    IN PVIDEO_RESOURCE_ID ResourceID,
    PSTATUS_BLOCK StatusBlock)
{
    VirtioGpuMpDeviceExtension->CurrentContextResourceID = ResourceID->ResourceID;

    StatusBlock->Information = sizeof(PVIDEO_RESOURCE_ID);
    return TRUE;
}

/*
 * VirtioGpuMpStartIO
 *
 * Processes the specified Video Request Packet.
 */

BOOLEAN
NTAPI
VirtioGpuMpStartIO(
    PVOID HwDeviceExtension,
    PVIDEO_REQUEST_PACKET RequestPacket)
{
    ERR_(IHVVIDEO, "StartIO: Request(0x%X)\n",
        RequestPacket->IoControlCode);

    PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension = (PVIRTIOGPUMP_DEVICE_EXTENSION)HwDeviceExtension;

    BOOLEAN Result;
    KIRQL OldIrql;
    switch(RequestPacket->IoControlCode)
    {
        case IOCTL_VIDEO_QUERY_AVAIL_MODES:
        {
            TRACE_(IHVVIDEO, "VirtioGpuMpStartIO IOCTL_VIDEO_QUERY_AVAIL_MODES\n");

            if (RequestPacket->OutputBufferLength < sizeof(VIDEO_MODE_INFORMATION))
            {
                RequestPacket->StatusBlock->Status = ERROR_INSUFFICIENT_BUFFER;
                return TRUE;
            }

            VirtioGpuMpCancelFlushTimer(VirtioGpuMpDeviceExtension, &OldIrql);
            Result = VirtioGpuMpQueryAvailModes(
                VirtioGpuMpDeviceExtension,
                (PVIDEO_MODE_INFORMATION)RequestPacket->OutputBuffer,
                RequestPacket->OutputBufferLength,
                RequestPacket->StatusBlock);
            break;
        }

        case IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES:
        {
            TRACE_(IHVVIDEO, "VirtioGpuMpStartIO IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES\n");

            if (RequestPacket->OutputBufferLength < sizeof(VIDEO_NUM_MODES))
            {
                RequestPacket->StatusBlock->Status = ERROR_INSUFFICIENT_BUFFER;
                return TRUE;
            }

            VirtioGpuMpCancelFlushTimer(VirtioGpuMpDeviceExtension, &OldIrql);
            Result = VirtioGpuMpQueryNumAvailModes(
                VirtioGpuMpDeviceExtension,
                (PVIDEO_NUM_MODES)RequestPacket->OutputBuffer,
                RequestPacket->StatusBlock);
            break;
        }

        case IOCTL_VIDEO_SET_CURRENT_MODE:
        {
            TRACE_(IHVVIDEO, "VirtioGpuMpStartIO IOCTL_VIDEO_SET_CURRENT_MODE\n");

            if (RequestPacket->InputBufferLength < sizeof(VIDEO_MODE))
            {
                RequestPacket->StatusBlock->Status = ERROR_INSUFFICIENT_BUFFER;
                return TRUE;
            }

            VirtioGpuMpCancelFlushTimer(VirtioGpuMpDeviceExtension, &OldIrql);
            Result = VirtioGpuMpSetCurrentMode(
                VirtioGpuMpDeviceExtension,
                (PVIDEO_MODE)RequestPacket->InputBuffer,
                RequestPacket->StatusBlock);
            break;
        }

        case IOCTL_VIDEO_MAP_VIDEO_MEMORY:
        {
            TRACE_(IHVVIDEO, "VirtioGpuMpStartIO IOCTL_VIDEO_MAP_VIDEO_MEMORY\n");

            if (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY))
            {
                RequestPacket->StatusBlock->Status = ERROR_INSUFFICIENT_BUFFER;
                return TRUE;
            }
            if (RequestPacket->OutputBufferLength < sizeof(VIDEO_MEMORY_INFORMATION))
            {
                RequestPacket->StatusBlock->Status = ERROR_INSUFFICIENT_BUFFER;
                return TRUE;
            }

            VirtioGpuMpCancelFlushTimer(VirtioGpuMpDeviceExtension, &OldIrql);
            Result = VirtioGpuMpMapVideoMemory(
                VirtioGpuMpDeviceExtension,
                (PVIDEO_MEMORY)RequestPacket->InputBuffer,
                (PVIDEO_MEMORY_INFORMATION)RequestPacket->OutputBuffer,
                RequestPacket->StatusBlock);
            break;
        }

        case IOCTL_VIDEO_UNMAP_VIDEO_MEMORY:
        {
            TRACE_(IHVVIDEO, "VirtioGpuMpStartIO IOCTL_VIDEO_UNMAP_VIDEO_MEMORY\n");

            if (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY))
            {
                RequestPacket->StatusBlock->Status = ERROR_INSUFFICIENT_BUFFER;
                return TRUE;
            }

            VirtioGpuMpCancelFlushTimer(VirtioGpuMpDeviceExtension, &OldIrql);
            Result = VirtioGpuMpUnmapVideoMemory(
                VirtioGpuMpDeviceExtension,
                (PVIDEO_MEMORY)RequestPacket->InputBuffer,
                RequestPacket->StatusBlock);
            break;
        }

        case IOCTL_VIDEO_RESET_DEVICE:
        {
            TRACE_(IHVVIDEO, "VirtioGpuMpStartIO IOCTL_VIDEO_RESET_DEVICE\n");

            VirtioGpuMpCancelFlushTimer(VirtioGpuMpDeviceExtension, &OldIrql);
            Result = VirtioGpuMpReset(
                VirtioGpuMpDeviceExtension,
                RequestPacket->StatusBlock);
            break;
        }

        case IOCTL_VIRTIOGPU_CREATE_2D_RESOURCE:
        {
            TRACE_(IHVVIDEO, "VirtioGpuMpStartIO IOCTL_VIRTIOGPU_CREATE_2D_RESOURCE\n");

            if (RequestPacket->InputBufferLength < sizeof(VIDEO_CREATE_2D_RESOURCE))
            {
                RequestPacket->StatusBlock->Status = ERROR_INSUFFICIENT_BUFFER;
                return TRUE;
            }
            if (RequestPacket->OutputBufferLength < sizeof(VIDEO_RESOURCE_ID))
            {
                RequestPacket->StatusBlock->Status = ERROR_INSUFFICIENT_BUFFER;
                return TRUE;
            }

            VirtioGpuMpCancelFlushTimer(VirtioGpuMpDeviceExtension, &OldIrql);
            Result = VirtioGpuMpCreate2DResource(
                VirtioGpuMpDeviceExtension,
                (PVIDEO_CREATE_2D_RESOURCE)RequestPacket->InputBuffer,
                (PVIDEO_RESOURCE_ID)RequestPacket->OutputBuffer,
                RequestPacket->StatusBlock);
            break;
        }

        case IOCTL_VIRTIOGPU_RESOURCE_TRANSFER_2D:
        {
            TRACE_(IHVVIDEO, "VirtioGpuMpStartIO IOCTL_VIRTIOGPU_RESOURCE_TRANSFER_2D\n");

            if (RequestPacket->InputBufferLength < sizeof(VIDEO_TRANSFER_2D))
            {
                RequestPacket->StatusBlock->Status = ERROR_INSUFFICIENT_BUFFER;
                return TRUE;
            }

            VirtioGpuMpCancelFlushTimer(VirtioGpuMpDeviceExtension, &OldIrql);
            Result = VirtioGpuMpResourceTransfer(
                VirtioGpuMpDeviceExtension,
                (PVIDEO_TRANSFER_2D)RequestPacket->InputBuffer,
                RequestPacket->StatusBlock);
            break;
        }

        case IOCTL_VIRTIOGPU_UPDATE_CURSOR:
        {
            TRACE_(IHVVIDEO, "VirtioGpuMpStartIO IOCTL_VIRTIOGPU_UPDATE_CURSOR\n");

            if (RequestPacket->InputBufferLength < sizeof(VIDEO_CURSOR))
            {
                RequestPacket->StatusBlock->Status = ERROR_INSUFFICIENT_BUFFER;
                return TRUE;
            }

            VirtioGpuMpCancelFlushTimer(VirtioGpuMpDeviceExtension, &OldIrql);
            Result = VirtioGpuMpUpdateMoveCursor(
                VirtioGpuMpDeviceExtension,
                (PVIDEO_CURSOR)RequestPacket->InputBuffer,
                RequestPacket->StatusBlock, TRUE);
            break;
        }

        case IOCTL_VIRTIOGPU_MOVE_CURSOR:
        {
            TRACE_(IHVVIDEO, "VirtioGpuMpStartIO IOCTL_VIRTIOGPU_MOVE_CURSOR\n");

            if (RequestPacket->InputBufferLength < sizeof(VIDEO_CURSOR))
            {
                RequestPacket->StatusBlock->Status = ERROR_INSUFFICIENT_BUFFER;
                return TRUE;
            }

            VirtioGpuMpCancelFlushTimer(VirtioGpuMpDeviceExtension, &OldIrql);
            Result = VirtioGpuMpUpdateMoveCursor(
                VirtioGpuMpDeviceExtension,
                (PVIDEO_CURSOR)RequestPacket->InputBuffer,
                RequestPacket->StatusBlock, FALSE);
            break;
        }

        case IOCTL_VIRTIOGPU_RESOURCE_FLUSH_SCREEN:
        {
            TRACE_(IHVVIDEO, "VirtioGpuMpStartIO IOCTL_VIRTIOGPU_RESOURCE_FLUSH_SCREEN\n");

            Result = VirtioGpuMpFlushScreen(
                VirtioGpuMpDeviceExtension,
                RequestPacket->StatusBlock);
            break;
        }

        case IOCTL_VIRTIOGPU_CREATE_CONTEXT:
        {
            TRACE_(IHVVIDEO, "VirtioGpuMpStartIO IOCTL_VIRTIOGPU_CREATE_CONTEXT\n");

            if (RequestPacket->InputBufferLength < sizeof(VIDEO_CREATE_CONTEXT))
            {
                RequestPacket->StatusBlock->Status = ERROR_INSUFFICIENT_BUFFER;
                return TRUE;
            }
            if (RequestPacket->OutputBufferLength < sizeof(VIDEO_RESOURCE_ID))
            {
                RequestPacket->StatusBlock->Status = ERROR_INSUFFICIENT_BUFFER;
                return TRUE;
            }

            VirtioGpuMpCancelFlushTimer(VirtioGpuMpDeviceExtension, &OldIrql);
            Result = VirtioGpuMpCreateContext(
                VirtioGpuMpDeviceExtension,
                (PVIDEO_CREATE_CONTEXT)RequestPacket->InputBuffer,
                (PVIDEO_RESOURCE_ID)RequestPacket->OutputBuffer,
                RequestPacket->StatusBlock);
            break;
        }

        case IOCTL_VIRTIOGPU_SET_CONTEXT:
        {
            TRACE_(IHVVIDEO, "VirtioGpuMpStartIO IOCTL_VIRTIOGPU_SET_CONTEXT\n");

            if (RequestPacket->InputBufferLength < sizeof(VIDEO_RESOURCE_ID))
            {
                RequestPacket->StatusBlock->Status = ERROR_INSUFFICIENT_BUFFER;
                return TRUE;
            }

            VirtioGpuMpCancelFlushTimer(VirtioGpuMpDeviceExtension, &OldIrql);
            Result = VirtioGpuMpSetContext(
                VirtioGpuMpDeviceExtension,
                (PVIDEO_RESOURCE_ID)RequestPacket->InputBuffer,
                RequestPacket->StatusBlock);
            break;
        }

        default:
        {
            ERR_(IHVVIDEO, "VirtioGpuMpStartIO 0x%x not implemented\n", RequestPacket->IoControlCode);

            RequestPacket->StatusBlock->Status = ERROR_INVALID_FUNCTION;
            return FALSE;
        }
    }

    if (Result)
    {
        RequestPacket->StatusBlock->Status = NO_ERROR;
    }

    VirtioGpuMpStartFlushTimer(VirtioGpuMpDeviceExtension, OldIrql);

    return TRUE;
}

VOID
NTAPI
FlushDpcRoutine(IN PKDPC Dpc,
                IN PVOID DeferredContext,
                IN PVOID SystemArgument1,
                IN PVOID SystemArgument2)
{
    PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension = (PVIRTIOGPUMP_DEVICE_EXTENSION)DeferredContext;
    VirtioGpuMpDeviceExtension->DpcFinished = 0;

    VideoPortAcquireSpinLockAtDpcLevel((PVOID)VirtioGpuMpDeviceExtension, VirtioGpuMpDeviceExtension->DpcLock);

    if(VirtioGpuMpDeviceExtension->ScanoutResourceID)
    {
        PVIRTIOGPUMP_MODE CurrentMode =
            &VirtioGpuMpDeviceExtension->SavedModes[VirtioGpuMpDeviceExtension->CurrentMode];

        {
            PVIRTIOGPUMP_TRANSFER_TO_HOST_2D Request = (PVIRTIOGPUMP_TRANSFER_TO_HOST_2D)VideoPortAllocatePool(
                (PVOID)VirtioGpuMpDeviceExtension, VpNonPagedPoolCacheAligned,
                sizeof(VIRTIOGPUMP_TRANSFER_TO_HOST_2D), VGPU_TAG);
            Request->Header.Type = VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D;
            Request->Rect.X = Request->Rect.Y = 0;
            Request->Rect.Width = CurrentMode->Rect.Width;
            Request->Rect.Height = CurrentMode->Rect.Height;
            Request->ResourceID = VirtioGpuMpDeviceExtension->ScanoutResourceID;
            Request->Offset = 0;

            PVIRTIOGPUMP_CONTROL_HEADER Res = NULL;
            if(VirtioGpuMpSendCmdCommandResponseDpc(VirtioGpuMpDeviceExtension,
                (PVOID)Request, sizeof(VIRTIOGPUMP_TRANSFER_TO_HOST_2D),
                (PVOID *)&Res, sizeof(VIRTIOGPUMP_CONTROL_HEADER)) == FALSE)
            {
                VideoPortReleaseSpinLockFromDpcLevel((PVOID)VirtioGpuMpDeviceExtension, VirtioGpuMpDeviceExtension->DpcLock);
                return;
            }

            VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Request);
            VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Res);
        }

        {
            PVIRTIOGPUMP_RESOURCE_FLUSH Request = (PVIRTIOGPUMP_RESOURCE_FLUSH)VideoPortAllocatePool(
                (PVOID)VirtioGpuMpDeviceExtension, VpNonPagedPoolCacheAligned,
                sizeof(VIRTIOGPUMP_RESOURCE_FLUSH), VGPU_TAG);
            Request->Header.Type = VIRTIO_GPU_CMD_RESOURCE_FLUSH;
            Request->Rect.X = Request->Rect.Y = 0;
            Request->Rect.Width = CurrentMode->Rect.Width;
            Request->Rect.Height = CurrentMode->Rect.Height;
            Request->ResourceID = VirtioGpuMpDeviceExtension->ScanoutResourceID;

            PVIRTIOGPUMP_CONTROL_HEADER Res = NULL;
            if(VirtioGpuMpSendCmdCommandResponseDpc(VirtioGpuMpDeviceExtension,
                (PVOID)Request, sizeof(VIRTIOGPUMP_RESOURCE_FLUSH),
                (PVOID *)&Res, sizeof(VIRTIOGPUMP_CONTROL_HEADER)) == FALSE)
            {
                VideoPortReleaseSpinLockFromDpcLevel((PVOID)VirtioGpuMpDeviceExtension, VirtioGpuMpDeviceExtension->DpcLock);
                return;
            }

            VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Request);
            VideoPortFreePool((PVOID)VirtioGpuMpDeviceExtension, Res);
        }
    }

    VideoPortReleaseSpinLockFromDpcLevel((PVOID)VirtioGpuMpDeviceExtension, VirtioGpuMpDeviceExtension->DpcLock);

    VirtioGpuMpDeviceExtension->DpcFinished = 1;
}

/*
 * VirtioGpuMpResetHw
 *
 * This function is called to reset the hardware to a known state.
 */

BOOLEAN
NTAPI
VirtioGpuMpResetHw(
    PVOID DeviceExtension,
    ULONG Columns,
    ULONG Rows)
{
    UNIMPLEMENTED;

    return TRUE;
}

/*
 * XboxVmpGetPowerState
 *
 * Queries whether the device can support the requested power state.
 */

VP_STATUS
NTAPI
VirtioGpuMpGetPowerState(
    PVOID HwDeviceExtension,
    ULONG HwId,
    PVIDEO_POWER_MANAGEMENT VideoPowerControl)
{
    UNIMPLEMENTED;

    return ERROR_INVALID_FUNCTION;
}

/*
 * VirtioGpuMpSetPowerState
 *
 * Sets the power state of the specified device
 */

VP_STATUS
NTAPI
VirtioGpuMpSetPowerState(
    PVOID HwDeviceExtension,
    ULONG HwId,
    PVIDEO_POWER_MANAGEMENT VideoPowerControl)
{
    UNIMPLEMENTED;

    return ERROR_INVALID_FUNCTION;
}

BOOLEAN
NTAPI
VirtioGpuMpInterrupt(IN PVOID HwDeviceExtension)
{
    PVIRTIOGPUMP_DEVICE_EXTENSION VirtioGpuMpDeviceExtension = (PVIRTIOGPUMP_DEVICE_EXTENSION)HwDeviceExtension;

    ULONG IsrStatus = *VirtioGpuMpDeviceExtension->IsrStatus;
    if(IsrStatus & 0x1)
    {
        for(ULONG q = 0; q < VirtioGpuMpDeviceExtension->Registers->numQueues; q++)
        {
            PVIRTIOGPUMP_QUEUE_VIRTQUEUE Queue = &VirtioGpuMpDeviceExtension->VirtualQueues[q];
            if((VirtioGpuMpDeviceExtension->ActiveQueues & (1 << q)) == 0) continue;

            USHORT CurrentIndex = Queue->Used->Index;
            for(USHORT i = Queue->LastUsed; i != (CurrentIndex & Queue->QueueMask); i = (i + 1) & Queue->QueueMask)
            {
                volatile PVIRTIOGPUMP_QUEUE_USED_ELEMENT UsedElement = (volatile PVIRTIOGPUMP_QUEUE_USED_ELEMENT)&Queue->Used->Rings[i];

                USHORT j = UsedElement->Index;
                while(TRUE)
                {
                    INT Next;
                    volatile PVIRTIOGPUMP_QUEUE_DESCRIPTOR Descriptor = (volatile PVIRTIOGPUMP_QUEUE_DESCRIPTOR)&Queue->Descriptors[j];

                    if(Descriptor->Flags & VIRTQ_DESC_F_NEXT) Next = Descriptor->Next;
                    else Next = -1;

                    if(*((volatile PUCHAR)&Queue->DescriptorsEndStatus[j]) == 0)
                    {
                        *((volatile PUCHAR)&Queue->DescriptorsEndStatus[j]) = 1;
                        VideoPortReleaseSpinLockFromDpcLevel(HwDeviceExtension, VirtioGpuMpDeviceExtension->CommandCompleteLock);
                        if(Next < 0) break;

                        Queue->Descriptors[j].Next = Queue->FreeList;
                        Queue->Descriptors[j].Address = 0;
                        Queue->FreeList = j;
                        Queue->FreeCount++;

                        j = Next;
                        continue;
                    }

                    Queue->Descriptors[j].Next = Queue->FreeList;
                    Queue->Descriptors[j].Address = 0;
                    Queue->FreeList = j;
                    Queue->FreeCount++;

                    if(Next < 0) break;
                    j = Next;
                }

                Queue->LastUsed = (Queue->LastUsed + 1) & Queue->QueueMask;
            }
        }

        return TRUE;
    }
    else
    {
        __asm__ __volatile__ ("" ::: "memory");
        __atomic_thread_fence(__ATOMIC_SEQ_CST);
        if(VirtioGpuMpDeviceExtension->DeviceConfig->events_read)
        {
            __asm__ __volatile__ ("" ::: "memory");
            __atomic_thread_fence(__ATOMIC_SEQ_CST);
            VirtioGpuMpDeviceExtension->DeviceConfig->events_clear = VirtioGpuMpDeviceExtension->DeviceConfig->events_read;

            return TRUE;
        }
    }
    //TODO: Events

    return FALSE;
}

ULONG
NTAPI
DriverEntry(
    IN PVOID Context1,
    IN PVOID Context2)
{
    VIDEO_HW_INITIALIZATION_DATA InitData;

    VideoPortZeroMemory(&InitData, sizeof(InitData));
    InitData.AdapterInterfaceType = PCIBus;
    InitData.HwInitDataSize = sizeof(VIDEO_HW_INITIALIZATION_DATA);
    InitData.HwFindAdapter = VirtioGpuMpFindAdapter;
    InitData.HwInitialize = VirtioGpuMpInitialize;
    InitData.HwStartIO = VirtioGpuMpStartIO;
    InitData.HwResetHw = VirtioGpuMpResetHw;
    InitData.HwGetPowerState = VirtioGpuMpGetPowerState;
    InitData.HwSetPowerState = VirtioGpuMpSetPowerState;
    InitData.HwInterrupt = VirtioGpuMpInterrupt;
    InitData.HwDeviceExtensionSize = sizeof(VIRTIOGPUMP_DEVICE_EXTENSION);

    return VideoPortInitialize(Context1, Context2, &InitData, NULL);
}
