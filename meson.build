project('Serial Lora Lib', 'c', 'cpp')

source = [
    'main.cpp',
    'SerialLora.cpp'
    ]

lib = shared_library( 'SerialLoRa', 'serial_lora.c')

executable('program', source, link_with : lib)
