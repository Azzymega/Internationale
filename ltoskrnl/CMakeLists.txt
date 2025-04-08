
add_subdirectory(fw)
add_subdirectory(hal)
add_subdirectory(ke)
add_subdirectory(rtl)
add_subdirectory(ps)
add_subdirectory(mm)

target_link_libraries(ltoskrnl PUBLIC fw)
target_link_libraries(ltoskrnl PUBLIC hal)
target_link_libraries(ltoskrnl PUBLIC mm)
target_link_libraries(ltoskrnl PUBLIC ps)
target_link_libraries(ltoskrnl PUBLIC rtl)