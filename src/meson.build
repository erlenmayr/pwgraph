pwcheck_gtk_sources = [
  'dictionary.c',
  'graph.c',
  'patterns.c',
  'main.c',
  'pwcheck_gtk-window.c',
]

pwcheck_gtk_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('gtk+-3.0', version: '>= 3.22'),
]

gnome = import('gnome')

pwcheck_gtk_sources += gnome.compile_resources('pwcheck_gtk-resources',
  'pwcheck_gtk.gresource.xml',
  c_name: 'pwcheck_gtk'
)

executable('pwcheck-gtk', pwcheck_gtk_sources,
  dependencies: pwcheck_gtk_deps,
  install: true,
)
