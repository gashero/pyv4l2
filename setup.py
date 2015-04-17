#! /usr/bin/env python
# -*- coding: UTF-8 -*-
# File: setup.py
# Date: 2015-04-17
# Author: gashero

"""
setup.py for pyv4l2
"""

import os

from distutils.core import setup,Extension

mod_v4l2=Extension('v4l2',sources=[
    'v4l2.c','v4l2obj.c',])

extmodlist=[mod_v4l2,]

setup(name='pyv4l2',
        version='1.0',
        description='pyv4l2, V4L2 interface for Python',
        author='gashero',
        author_email='harry.python@gmail.com',
        packages=[],
        ext_modules=extmodlist,
        )
