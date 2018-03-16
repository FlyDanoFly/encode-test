from distutils.core import setup, Extension
setup(name='mymodule', version='1.0',  \
      ext_modules=[Extension('mymodule', ['mymodule.c'])])
