#! /usr/bin/env python
# -*- coding: UTF-8 -*-
# File: testme.py
# Date: 2015-04-17
# Author: gashero

"""
manual test entry.
"""

import os
import sys
import select

import v4l2

def main():
    vo=v4l2.V4L2('/dev/video0',
            v4l2.V4L2_BUF_TYPE_VIDEO_CAPTURE,
            v4l2.V4L2_MEMORY_MMAP
            #v4l2.V4L2_MEMORY_USERPTR
            )
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
    #print 'queryctrl(V4L2_CID_GAIN)',vo.queryctrl(v4l2.V4L2_CID_GAIN)
    #print 'g_ctrl(V4L2_CID_MIN_BUFFERS_FOR_CAPTURE)',vo.g_ctrl(v4l2.V4L2_CID_MIN_BUFFERS_FOR_CAPTURE)
    #print 'g_ctrl(V4L2_CID_GAIN)',vo.g_ctrl(v4l2.V4L2_CID_GAIN)
    buffercount=vo.reqbufs(4)
    assert buffercount==4,buffercount
    bufmap={}
    for bufidx in range(buffercount):
        bufinfo=vo.querybuf(bufidx)
        addr=vo.mmap(bufinfo['offset'],bufinfo['length'])
        bufmap[bufidx]=(addr,bufinfo['length'])
        vo.qbuf(bufinfo['index'])
    vo.streamon()
    for i in range(2):      #Jump out 2 frame
        assert ([vo.fileno()],[],[])==select.select([vo.fileno()],[],[],2)
        bufidx=vo.dqbuf()
        vo.qbuf(bufidx)
    assert ([vo.fileno()],[],[])==select.select([vo.fileno()],[],[],2)
    bufidx=vo.dqbuf()
    vo.qbuf(bufidx)
    bufinfo=vo.querybuf(bufidx)
    data=vo.getbuffer(*bufmap[bufidx])
    print len(data)
    f=open('xxx.jpg','w')
    f.write(data)
    f.close()
    assert data[-2:]=='\xff\xd9'
    #TODO:
    vo.streamoff()
    for (bufidx,(addr,length)) in bufmap.items():
        vo.munmap(addr,length)
    return

if __name__=='__main__':
    main()
    print '---- finished ----'
