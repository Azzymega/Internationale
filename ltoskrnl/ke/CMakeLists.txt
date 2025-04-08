
list(APPEND source
        entry.c
)

add_executable(ltoskrnl ${source})

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
    target_link_options(ltoskrnl PRIVATE "-Wl,-T../targets/ltoskrnl/ia32.ld")
endif ()

target_link_options(ltoskrnl PRIVATE ${KERNEL_LINK_FLAGS})