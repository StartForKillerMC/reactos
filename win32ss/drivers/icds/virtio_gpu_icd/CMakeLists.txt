spec2def(virtiogpuicd.dll virtiogpuicd.spec ADD_IMPORTLIB)

list(APPEND SOURCE
    virtio_gpu_icd.c
    virtio_gpu_icd.h
    context.c
    opengl.h)

add_library(virtiogpuicd MODULE
    ${SOURCE}
    virtio_gpu_icd.rc
    ${CMAKE_CURRENT_BINARY_DIR}/virtiogpuicd.def)

set_module_type(virtiogpuicd win32dll ENTRYPOINT DllMain 12)
add_pch(virtiogpuicd virtio_gpu_icd.h SOURCE)
#target_link_libraries(virtiogpuicd wine)
add_importlibs(virtiogpuicd gdi32 kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 ole32 oleaut32 ntdll)
add_dependencies(virtiogpudisp psdk)
add_cd_file(TARGET virtiogpuicd DESTINATION reactos/system32 FOR all)
add_registry_inf(virtiogpuicd_reg.inf)