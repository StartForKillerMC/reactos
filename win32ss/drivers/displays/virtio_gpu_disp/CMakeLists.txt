
list(APPEND SOURCE
    virtio_gpu_disp.c
    screen.c
    surface.c
    pointer.c
    virtio_gpu_disp.h)

add_library(virtiogpudisp MODULE
    ${SOURCE}
    virtio_gpu_disp.rc)

set_module_type(virtiogpudisp kerneldll ENTRYPOINT DrvEnableDriver 12)
add_pch(virtiogpudisp virtio_gpu_disp.h SOURCE)
target_link_libraries(virtiogpudisp libcntpr)
add_importlibs(virtiogpudisp win32k)
add_dependencies(virtiogpudisp psdk)
add_cd_file(TARGET virtiogpudisp DESTINATION reactos/system32 FOR all)
