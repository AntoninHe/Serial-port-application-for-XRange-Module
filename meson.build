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

inc = include_directories('node_like_fct')

executable('program_test_cpp', sources, 
            link_args : '-pthread',
            include_directories : inc,
            link_with : libs)
