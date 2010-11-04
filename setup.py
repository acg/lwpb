from distutils.core import setup, Extension

setup(name='lwpb',
      version='0.1',
      ext_modules=[
          Extension('lwpb', ['lwpb.py.c'],
              include_dirs=['./lwpb/src/include'],
              define_macros=[],
              library_dirs=['./lwpb/src/.libs'],
              libraries=['lwpb'],
          ),
      ],
      packages=['lwpb']
      )
