#!/usr/bin/env python

from distutils.core import setup, Extension, Command


class TestCommand(Command):
  user_options = []

  def initialize_options(self):
    pass

  def finalize_options(self):
    pass

  def run(self):
    import tests
    tests.run('test/test.proto.pb', 'test/truthdb.txt')


setup(
  name='lwpb',
  version='0.1',
  ext_modules=[
    Extension(
      'lwpb.cext',
      [
        'decoder.c',
        'descriptor.c',
        'encoder.c',
        'module.c',
        'primitives.c',
      ],
      extra_compile_args=['-fPIC','-Wall'],
      include_dirs=['../src/include'],
      define_macros=[],
      library_dirs=['../src/.libs'],
      libraries=['lwpb'],
    ),
  ],
  packages=['lwpb'],
  cmdclass={"test":TestCommand}
)

