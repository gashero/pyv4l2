/*
 * File: v4l2obj.c
 * Date: 2015-04-17
 * Author: gashero
 */

#include <Python.h>
#include <structmember.h>

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
#include <fcntl.h>

#include <linux/videodev2.h>

#include "v4l2obj.h"

typedef struct {
    PyObject_HEAD
    int     fd;
} V4L2Object;

static int eioctl(int fd, int request, void *argp, char *errmsg) {
    int ret;
    ret=ioctl(fd, request, argp);
    if (request<0) {
        PyErr_Format(PyExc_OSError, "ERROR[%d]=\"%s\", %s",
                errno, strerror(errno), errmsg);
    }
    return ret;
}

// Object system methods -------------------------------------------------------

static void V4L2_dealloc(V4L2Object *self) {
    self->ob_type->tp_free((PyObject*)self);
    close(self->fd);
}

static PyObject *V4L2_New(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    V4L2Object *self;
    self=(V4L2Object*)type->tp_alloc(type,0);
    if (self!=NULL) {
        //创建其他成员
    }
    return (PyObject*)self;
}

static int V4L2_Init(V4L2Object *self, PyObject *args, PyObject *kwds) {
    char *devname;
    if (!PyArg_ParseTuple(args, "s",
                &devname)) {
        return -1;
    }
    self->fd=open(devname,O_RDWR);
    return 0;
}

// Object methods --------------------------------------------------------------

static PyObject *V4L2_querycap(V4L2Object *self) {
    struct v4l2_capability cap;
    if (eioctl(self->fd, VIDIOC_QUERYCAP, &cap, "ioctl(VIDIOC_QUERYCAP)")<0) {
        return NULL;
    }
    return Py_BuildValue("{s:s,s:s,s:s,s:l,s:l}",
            "driver",       cap.driver,
            "card",         cap.card,
            "bus_info",     cap.bus_info,
            "version",      cap.version,
            "capabilities", cap.capabilities
            );
}

// Module level members --------------------------------------------------------

static PyMethodDef V4L2_methods[] = {
    {"querycap",    (PyCFunction)V4L2_querycap,     METH_NOARGS,
        "querycap()"},
    {NULL}
};

static PyMemberDef V4L2_members[] = {
    {NULL}
};

PyTypeObject V4L2Type= {
    PyVarObject_HEAD_INIT(&PyType_Type,0)
    "V4L2",                     //tp_name，类型名字，注意同时包含模块名，以点分隔
    sizeof(V4L2Object),         //tp_basicsize，调用PyObject_New()时需要分配多少内存
    0,                              //tp_itemsize，对列表和字符串，这里是可变长度，暂时忽略
    (destructor)V4L2_dealloc,   //tp_dealloc
    0,0,0,0,0,                      //tp_print,tp_getattr,tp_setattr,tp_compare,tp_repr
    0,0,0,0,0,                      //tp_as_number,tp_as_sequence,tp_as_mapping,tp_as_hash,tp_call
    0,0,0,0,                        //tp_str,tp_getattro,tp_setattro,tp_as_buffer
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,//tp_flags
    "V4L2 Objecct",             //tp_doc
    0,0,0,0,                        //tp_traverse,tp_clear,tp_richcompare,tp_weaklistoffset
    0,0,                            //tp_iter,tp_iternext
    V4L2_methods,               //struct PyMethodDef *tp_methods
    V4L2_members,               //struct PyMemberDef *tp_members
    0,0,0,0,0,                      //tp_getset,tp_base,tp_dict,tp_descr_get,tp_descr_set
    0,                              //tp_dictoffset
    (initproc)V4L2_Init,        //tp_init
    0,                              //tp_alloc
    V4L2_New,                   //tp_new
    0,0,0,0,0,                      //tp_free,tp_is_gc,tp_bases,tp_mro,tp_cache
    0,0,0,0,                        //tp_subclasses,tp_weaklist,tp_del,tp_version_tag
};
