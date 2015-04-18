/*
 * File: v4l2.c
 * Date: 2015-04-17
 * Author: gashero
 */

#include <Python.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <linux/videodev2.h>

#include "v4l2obj.h"

// Uilt functions --------------------------------------------------------------

/*static int eioctl(int fd, int request, void *argp, char *errmsg) {
    int ret;
    ret=ioctl(fd, request, argp);
    if (ret<0) {
        PyErr_Format(PyExc_OSError, "ERROR[%d]=\"%s\", %s",
                errno, strerror(errno), errmsg);
    }
    return ret;
}*/

// Module leve functions -------------------------------------------------------

static PyMethodDef v4l2_methods[] = {
    {NULL}
};

struct kvlong {
    char        *key;
    long        value;
};

static struct kvlong const_kvlong[] = {
    // enum v4l2_buf_type
    {"V4L2_BUF_TYPE_VIDEO_CAPTURE",     V4L2_BUF_TYPE_VIDEO_CAPTURE},
    {"V4L2_BUF_TYPE_VIDEO_OUTPUT",      V4L2_BUF_TYPE_VIDEO_OUTPUT},
    {"V4L2_BUF_TYPE_VIDEO_OVERLAY",     V4L2_BUF_TYPE_VIDEO_OVERLAY},
    {"V4L2_BUF_TYPE_VBI_CAPTURE",       V4L2_BUF_TYPE_VBI_CAPTURE},
    {"V4L2_BUF_TYPE_VBI_OUTPUT",        V4L2_BUF_TYPE_VBI_OUTPUT},
    {"V4L2_BUF_TYPE_SLICED_VBI_CAPTURE",V4L2_BUF_TYPE_SLICED_VBI_CAPTURE},
    {"V4L2_BUF_TYPE_SLICED_VBI_OUTPUT", V4L2_BUF_TYPE_SLICED_VBI_OUTPUT},
    {"V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE",  V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE},
    {"V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE",   V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE},
    {"V4L2_BUF_TYPE_PRIVATE",           V4L2_BUF_TYPE_PRIVATE},
    // enum v4l2_field, not complete
    {"V4L2_FIELD_ANY",                  V4L2_FIELD_ANY},
    {"V4L2_FIELD_NONE",                 V4L2_FIELD_NONE},
    {"V4L2_FIELD_TOP",                  V4L2_FIELD_TOP},
    {"V4L2_FIELD_BOTTOM",               V4L2_FIELD_BOTTOM},
    {"V4L2_FIELD_INTERLACED",           V4L2_FIELD_INTERLACED},
    // v4l2_pix_fmt, not complete
    {"V4L2_PIX_FMT_YUYV",               V4L2_PIX_FMT_YUYV},
    {"V4L2_PIX_FMT_MJPEG",              V4L2_PIX_FMT_MJPEG},
    // v4l2_memory
    {"V4L2_MEMORY_MMAP",                V4L2_MEMORY_MMAP},
    {"V4L2_MEMORY_USERPTR",             V4L2_MEMORY_USERPTR},
    {"V4L2_MEMORY_OVERLAY",             V4L2_MEMORY_OVERLAY},
    {"V4L2_MEMORY_DMABUF",              V4L2_MEMORY_DMABUF},
    // other constant I interest.
    {"V4L2_CID_BRIGHTNESS",             V4L2_CID_BRIGHTNESS},
    {"V4L2_CID_GAIN",                   V4L2_CID_GAIN},
    {"V4L2_CID_AUTOBRIGHTNESS",         V4L2_CID_AUTOBRIGHTNESS},
    {"V4L2_CID_AUDIO_MUTE",             V4L2_CID_AUDIO_MUTE},
    {"V4L2_CID_MIN_BUFFERS_FOR_CAPTURE",V4L2_CID_MIN_BUFFERS_FOR_CAPTURE},
    {"V4L2_STD_PAL_BG",                 V4L2_STD_PAL_BG},
    {NULL}
};

PyMODINIT_FUNC initv4l2(void) {
    PyObject *m;
    m=Py_InitModule3("v4l2", v4l2_methods, "v4l2 module");
    struct kvlong *_kv;
    for (_kv=const_kvlong; _kv->key!=NULL; _kv++) {
        if (PyModule_AddIntConstant(m, _kv->key, _kv->value)<0)
            return;
    }
    if (PyType_Ready(&V4L2Type)<0)
        return;
    Py_INCREF(&V4L2Type);
    PyModule_AddObject(m, "V4L2", (PyObject*)&V4L2Type);
}
