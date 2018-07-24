project('Serial Lora Lib', 'c', 'cpp',default_options : ['cpp_std=c++11'])

subdir('node_like_fct')
subdir('semtechBase64')

sources_test = [
    'testNodeFctFrame.cpp',
    'forwarder.cpp'
]

sources = [
    'main.cpp'          ,
    'serial_lora.cpp'   ,
    'SerialLora.cpp'    ,
    'forwarder.cpp'     ,
    'cpu.cpp'
]

libs = [
    node_like_fct_lib, # For testing node fct
    base64_lib_sem
]

inc = include_directories(['node_like_fct',
                            'semtechBase64']) 

incBase64 = include_directories('node_like_fct/base64')


executable('GateWay_single_channel_test', sources, 
            link_args : '-pthread',
            include_directories : incBase64,
            link_with : base64_lib
            )
'node_like_fct/base64'

executable('test_node', sources_test, 
            include_directories : inc,
            link_with : libs)
