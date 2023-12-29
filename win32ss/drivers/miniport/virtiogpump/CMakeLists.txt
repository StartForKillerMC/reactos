
list(APPEND SOURCE
    virtio_gpu_mp.c
    virtio_gpu_mp.h)

add_library(virtiogpump MODULE
    ${SOURCE}
    virtio_gpu_mp.rc)

set_module_type(virtiogpump kernelmodedriver)
add_importlibs(virtiogpump ntoskrnl hal videoprt)
add_pch(virtiogpump virtio_gpu_mp.h SOURCE)

add_cd_file(TARGET virtiogpump DESTINATION reactos/system32/drivers FOR all)
add_registry_inf(virtiogpump_reg.inf)
add_driver_inf(virtiogpump virtiogpump.inf)