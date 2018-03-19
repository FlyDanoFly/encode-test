from setuptools import setup, Extension

setup(name='funniest',
      version='0.1',
      description='The funniest joke in the world',
      url='http://github.com/storborg/funniest',
      author='Flying Circus',
      author_email='flyingcircus@example.com',
      license='MIT',
      packages=['fun'],
      ext_modules=[Extension('mymodule', ['mymodule23.c'])],
      zip_safe=False
      )
