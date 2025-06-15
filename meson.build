project(
    'cz-skia',
    'c','cpp',
    version : run_command('cat', files('VERSION'), check : false).stdout().strip(),
    meson_version: '>= 0.62.0',
    default_options: [
        'warning_level=2',
        'buildtype=release',
        'cpp_std=c++20'])

cpp = meson.get_compiler('cpp')
pkg = import('pkgconfig')
VERSION_MAJOR = meson.project_version().split('.')[0]
VERSION_MINOR = meson.project_version().split('.')[1]
VERSION_PATCH = meson.project_version().split('.')[2]
VERSION_BUILD = run_command('cat', 'BUILD', check : false).stdout()

HEADERS_INSTALL_PATH = join_paths(get_option('prefix'), get_option('includedir'))

add_project_arguments([
    '-DSK_RELEASE',
    '-DSK_ENABLE_OPTIMIZE_SIZE',

    '-DSKCMS_DISABLE_HSW',
    '-DSKCMS_DISABLE_SKX',

    '-DSK_UNICODE_AVAILABLE',
    '-DSK_UNICODE_RUNTIME_ICU_AVAILABLE',
    '-DSK_UNICODE_ICU_IMPLEMENTATION',
    '-DSKUNICODE_IMPLEMENTATION=1',
    '-DSKUNICODE_DLL',
    '-DU_USING_ICU_NAMESPACE=0',

    '-DSK_SHAPER_PRIMITIVE_AVAILABLE',
    '-DSK_SHAPER_HARFBUZZ_AVAILABLE',
    '-DSK_SHAPER_UNICODE_AVAILABLE',

    '-DSK_VULKAN',
    '-DSK_GL',
    '-DSK_GANESH',

    '-DSK_CODEC_DECODES_JPEG',
    '-DSK_CODEC_DECODES_PNG',
    '-DSK_CODEC_DECODES_PNG_WITH_LIBPNG',
    '-DSK_CODEC_DECODES_WEBP',
    '-DSK_CODEC_DECODES_ICO',
    '-DSK_CODEC_DECODES_BMP',

    # '-DSK_CODEC_DECODES_JPEGXL',
    # '-DSK_CODEC_DECODES_GIF',

    '-w',
    '-DqDNGReportErrors=0',
    '-DqDNGThreadSafe=1',
    '-DqDNGUseLibJPEG=1',
    '-DqDNGUseXMP=0',
    '-DqDNGValidate=0',
    '-DqDNGValidateTarget=1',
    '-DUNIX_ENV=1',
    '-DqDNGBigEndian=0'
], language: 'cpp')

deps = [
    dependency('egl'),
    dependency('gl'),
    dependency('glesv2'),
    dependency('vulkan'),
    dependency('harfbuzz'),
    dependency('icu-uc'),
    dependency('fontconfig'),
    dependency('freetype2'),
    dependency('zlib'),
    dependency('libpng'),
    dependency('libwebp'),
    dependency('libwebpdemux'),
    dependency('libwebpmux'),
    dependency('libjpeg'),
    dependency('epoxy'),
    dependency('SPIRV-Tools'),
    dependency('expat'),
    dependency('threads'),
    cpp.find_library('dl'),
    cpp.find_library('z')
]

sources = [
    run_command('find', './src', '-type', 'f', '-name', '*.cpp', check : false).stdout().strip().split('\n'),
    run_command('find', './src', '-type', 'f', '-name', '*.cc', check : false).stdout().strip().split('\n'),
    run_command('find', './modules/skcms', '-type', 'f', '-name', '*.cc', check : false).stdout().strip().split('\n'),
    run_command('find', './modules/skunicode', '-type', 'f', '-name', '*.cpp', check : false).stdout().strip().split('\n'),
    run_command('find', './modules/skshaper', '-type', 'f', '-name', '*.cpp', check : false).stdout().strip().split('\n'),
    run_command('find', './modules/skresources', '-type', 'f', '-name', '*.cpp', check : false).stdout().strip().split('\n'),
    run_command('find', './modules/skparagraph/src', '-type', 'f', '-name', '*.cpp', check : false).stdout().strip().split('\n'),
    run_command('find', './modules/svg', '-type', 'f', '-name', '*.cpp', check : false).stdout().strip().split('\n')
]

install_subdir('headers/skia', install_dir: join_paths(HEADERS_INSTALL_PATH, 'cz'))

cz_skia = library(
    'cz-skia',
    sources : sources,
    include_directories : [
        include_directories('.'),
        include_directories('./src/EXTRA')
    ],
    dependencies : deps,
    soversion: VERSION_MAJOR,
    install : true)

pkg.generate(
    cz_skia,
    name: 'cz-skia',
    filebase: 'cz-skia',
    description: 'Cuarzo modified version of Skia',
    version: meson.project_version(),
    subdirs: ['cz'], # incdir
    libraries: deps,
    extra_cflags: [
    '-DSK_RELEASE',
    '-DSKCMS_DISABLE_HSW',
    '-DSKCMS_DISABLE_SKX',
    '-DSK_UNICODE_AVAILABLE',
    '-DSK_UNICODE_RUNTIME_ICU_AVAILABLE',
    '-DSK_UNICODE_ICU_IMPLEMENTATION',
    '-DSK_SHAPER_PRIMITIVE_AVAILABLE',
    '-DSK_SHAPER_HARFBUZZ_AVAILABLE',
    '-DSK_SHAPER_UNICODE_AVAILABLE',
    '-DSK_VULKAN',
    '-DSK_GL',
    '-DSK_GANESH'
    ]) 

