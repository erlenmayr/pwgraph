pwgraph_sources = [
  'dictionary.c',
  'graph.c',
  'utils.c',
  'main.c',
  'pwgraph-window.c',
]

# libc math library if not included
cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)

pwgraph_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('gtk+-3.0', version: '>= 3.22'),
  m_dep,
]

gnome = import('gnome')

pwgraph_sources += gnome.compile_resources('pwgraph-resources',
  'pwgraph.gresource.xml',
  c_name: 'pwgraph'
)

executable('pwgraph', pwgraph_sources,
  dependencies: pwgraph_deps,
  install: true,
)
