add_library(gotcha SHARED gotcha_utils.c gotcha.c gotcha_auxv.c libc_wrappers.c elf_ops.c hash.c tool.c)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
install(TARGETS gotcha DESTINATION lib)
add_subdirectory(example)
