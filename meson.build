project('Serial Lora Lib', 'c', 'cpp')

source = [
    'main.cpp',
    'SerialLora.cpp'
]

lib = shared_library( 'SerialLoRa', 'serial_lora.c')

executable('program_test_cpp', source, link_with : lib)

executable('program_test_c', 'main.c', link_with : lib)
