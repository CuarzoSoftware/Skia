project(
    'cz-skia-svg',
    'c','cpp',
    version : '0.1.0',
    meson_version: '>= 0.62.0',
    default_options: [
        'warning_level=2',
        'buildtype=release',
        'cpp_std=c++20'])


# -------------- DEPENDENCIES --------------

cpp             = meson.get_compiler('cpp')
pkg             = import('pkgconfig')
skia_dep        = dependency('cuarzo-skia')
expat_dep       = dependency('expat')
deps = [
  skia_dep,
  expat_dep
]


# -------------- SOURCES --------------

sources = run_command('find', './src', '-type', 'f', '-name', '*[.cpp,.c,.h,.hpp]', check : false).stdout().strip().split('\n')

# -------------- LIBRARY --------------

skia_svg = library(
    'cz_skia_svg',
    sources : sources,
    include_directories : [],
    dependencies : deps,
    soversion: 0,
    install : true)

skia_svg_dep = declare_dependency(
    dependencies: deps,
    include_directories : [],
    link_with : skia_svg)

pkg.generate(
    skia_svg,
    name: 'cuarzo-skia-svg',
    description: 'C++ GUI toolkit powered by Skia and Yoga.',
    version: meson.project_version(),
    requires: ['cuarzo-skia'])

