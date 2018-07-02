from setuptools import setup, Extension

setup(name='base36encode',
      version='0.1',
      description='Ultra fast base36 encoder for positive ints up to 128 bits (0 <= x <= 2^128)',
      url='http://github.com/FlyDanoFly/encode-test.git',
      author='Dano!',
      author_email='dano@skilljar.com',
      license='MIT',
      ext_modules=[Extension('base36encode', ['base36encode.c'])],
      zip_safe=False
      )
