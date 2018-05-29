project('Serial Lora Lib', 'c', 'cpp')

subdir('node_like_fct')

sources = [
    'main.cpp'          ,
    'serial_lora.cpp'   ,
    'SerialLora.cpp'    ,
    'forwarder.cpp'
]

libs = [
    node_like_fct_lib # For testing node fct
]
executable('program_test_cpp', sources, 
            link_args : '-pthread',
            link_with : libs)
