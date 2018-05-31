inc = include_directories(['utilities','crypto','mac','base64'])

subdir('utilities')
subdir('crypto')
subdir('mac')
subdir('base64')

sources = [
    'main.c'
]

libs = [
    crypto_lib,
    mac_lib,
    utilities_lib,
    base64_lib
]   

#node_like_fct_lib = shared_library('program_test_node',
node_like_fct_lib = static_library('program_test_node',
            sources,
            include_directories : inc,
            link_with : libs)
