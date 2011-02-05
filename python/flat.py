#!/usr/bin/env python


def unflatten(src):

  dst = {}

  for k, v in src.items():
    path = k.split(".")
    p = dst

    for i in range(0, len(path)):

      if i == len(path)-1:
        default = v
      elif path[i+1].isdigit():
        default = []
      else:
        default = {}

      part = path[i]

      if part.isdigit():
        part = int(part)
        if part >= len(p):
          p.extend([None] * (part-len(p)+1))
        p[part] = default
      elif not part in p:
        p[part] = default

      p = p[part]

  return dst


def flatten(src):

  flat = {}

  if isinstance(src, list):
    for i in range(0, len(src)):
      flat[str(i)] = flatten(src[i])
  elif isinstance(src, dict):
    for k, v in src.items():
      flat[k] = flatten(v)
  else:
    return src

  dst = {}

  for k1, v1 in flat.items():
    if isinstance(v1, dict):
      for k2, v2 in v1.items():
        dst["%s.%s" % (k1,k2)] = v2
    else:
      dst[k1] = v1

  return dst


def test_unflatten():

  src = {
    "a.b.c": 42,
    "a.b.d": 43,
    "d.e.g": 12,
    "d.e.f.0": 10,
    "d.e.f.1": 20,
    "d.e.f.2": 30,
  }

  dst = unflatten(src)
  print dst
  print flatten(dst)

