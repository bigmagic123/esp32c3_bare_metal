project('example', 'c')

sources = []
inc     = []

subdir( 'driver' )

add_project_arguments('-march=rv32imc', language: 'c')
linker_script1 = join_paths(meson.current_source_dir(), 'ld/esp32c3.ld')
linker_script2 = join_paths(meson.current_source_dir(), 'ld/common.ld')
linker_script3 = join_paths(meson.current_source_dir(), 'ld/romfuncs.ld')

add_project_link_arguments('-T', linker_script1 ,'-T', linker_script2 ,'-T', linker_script3 ,'-march=rv32imc', '--specs=nano.specs', '--specs=nosys.specs', '-nostartfiles', language: 'c')

sources += files('main.c')
sources += files('start.S')
sources += files('vectors.S')

exe = executable('example', sources, include_directories:[inc])

hex = custom_target('demo.bin',
  build_by_default: true,
  input : exe,
  output : 'demo.bin',
  command : [meson.get_external_property('objcopy'), '-O', 'binary','@INPUT@','@OUTPUT@'],
)

