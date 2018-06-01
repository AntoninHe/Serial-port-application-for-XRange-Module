project('Serial Lora Lib', 'c', 'cpp')

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
    'forwarder.cpp'
]

libs = [
    node_like_fct_lib, # For testing node fct
    base64_lib_sem
]

inc = include_directories(['node_like_fct','semtechBase64']) 

executable('GateWay_single_channel_test', sources, 
            link_args : '-pthread')


executable('test_node', sources_test, 
            include_directories : inc,
            link_with : libs)
