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
    if (request<0) {
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
