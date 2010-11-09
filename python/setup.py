from distutils.core import setup, Extension

setup(name='lwpb',
      version='0.1',
      ext_modules=[
          Extension('lwpb.cext', ['lwpb.py.c'],
              include_dirs=['../src/include'],
              define_macros=[],
              library_dirs=['../src/.libs'],
              libraries=['lwpb'],
          ),
      ],
      packages=['lwpb']
      )
