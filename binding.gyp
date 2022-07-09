{
   'targets': [
      {
         'target_name': 'nullbyte',
         'cflags!': ['-fno-exceptions'],
         'cflags_cc!': ['-fno-exceptions'],
         'sources': [ 'lib/nullbyte.cpp', 'lib/util.cpp', 'lib/util.h' ],
         'include_dirs': [
            '<!@(node -p "require(\'node-addon-api\').include")'
         ],
         'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
         'msvs_settings': {
            'VCCLCompilerTool': {
               'ExceptionHandling': '1',
               'AdditionalOptions': ['/EHsc']
            }
         }
      }
   ]
}
