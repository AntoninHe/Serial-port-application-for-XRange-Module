project('Serial Lora Lib', 'c', 'cpp',default_options : ['cpp_std=c++11'])

sources = [
    'main.cpp'          ,
    'serial_lora.cpp'   ,
    'forwarder.cpp'     ,
    'cpu.cpp'
]

incBase64 = include_directories(['cppcodec'])

executable('GateWay_single_channel_test', sources, 
            link_args : '-pthread',
            include_directories : incBase64)