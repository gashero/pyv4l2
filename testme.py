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
    vo=v4l2.V4L2('/dev/video0',
            v4l2.V4L2_BUF_TYPE_VIDEO_CAPTURE,
            v4l2.V4L2_MEMORY_MMAP)
    print 'querycap()=',vo.querycap()
    print 'enum_fmt()=',vo.enum_fmt()
    vo.s_fmt(
            1280,720,
            v4l2.V4L2_PIX_FMT_MJPEG,
            v4l2.V4L2_FIELD_ANY)
    print 'enuminput()=',vo.enuminput()
    print 'enumaudio()=',vo.enumaudio()
    vo.s_input(0)
    print 'g_input()=',vo.g_input()
    print 'enumstd()=',vo.enumstd()
    buffercount=vo.reqbufs(4)
    buffercount=4
    assert buffercount==4
    bufmap={}
    for bufidx in range(buffercount):
        print 'x'*10,bufidx,buffercount
        bufinfo=vo.querybuf(bufidx)
        print bufinfo,bufidx
        addr=vo.mmap(bufinfo['offset'],bufinfo['length'])
        bufmap[bufidx]=(addr,bufinfo['length'])
        vo.qbuf(bufinfo['index'])
    vo.streamon()
    #TODO:
    for (bufidx,(addr,length)) in bufmap.items():
        vo.munmap(addr,length)
    return

if __name__=='__main__':
    main()
    print '---- finished ----'
