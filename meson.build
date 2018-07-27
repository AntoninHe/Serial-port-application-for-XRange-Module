project('Serial Lora Lib', 'c', 'cpp',default_options : ['cpp_std=c++11'])

subdir('node_like_fct')

sources_test = [
    'testNodeFctFrame.cpp',
    'forwarder.cpp'
]

sources = [
    'main.cpp'          ,
    'serial_lora.cpp'   ,
    'forwarder.cpp'     ,
    'cpu.cpp'
]

libs = [
    node_like_fct_lib # For testing node fct
]

incBase64 = include_directories(['node_like_fct/base64','node_like_fct'])


executable('GateWay_single_channel_test', sources, 
            link_args : '-pthread',
            include_directories : incBase64,
            link_with : libs)
'node_like_fct/base64'

executable('test_node', sources_test, 
            include_directories : incBase64,
            link_with : libs)
