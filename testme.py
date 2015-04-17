#! /usr/bin/env python
# -*- coding: UTF-8 -*-
# File: testme.py
# Date: 2015-04-17
# Author: gashero

"""
manual test entry.
"""

import v4l2

def main():
    vo=v4l2.V4L2('/dev/video0')
    print vo.querycap()
    print vo.enum_fmt(v4l2.V4L2_BUF_TYPE_VIDEO_CAPTURE)
    return

if __name__=='__main__':
    main()
