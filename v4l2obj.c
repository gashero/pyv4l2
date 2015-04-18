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
    int         fd;
    uint32_t    type;
    uint32_t    memory;
} V4L2Object;

static int eioctl(int fd, int request, void *argp, char *errmsg) {
    int ret;
    ret=ioctl(fd, request, argp);
    if (ret<0) {
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
    if (!PyArg_ParseTuple(args, "sII",
                &devname,
                &self->type,
                &self->memory)) {
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
    return Py_BuildValue("{s:s,s:s,s:s,s:I,s:I,s:I}",
            "driver",       cap.driver,
            "card",         cap.card,
            "bus_info",     cap.bus_info,
            "version",      cap.version,
            "capabilities", cap.capabilities,
            "device_caps",  cap.device_caps
            );
}

static PyObject *V4L2_enum_fmt(V4L2Object *self) {
    struct v4l2_fmtdesc fmtdesc;
    fmtdesc.index=0;
    PyObject *fmtlist, *fmtinfo;
    fmtdesc.type=self->type;
    fmtlist=PyList_New(0);
    while(ioctl(self->fd, VIDIOC_ENUM_FMT, &fmtdesc)!=-1) {
        fmtinfo=Py_BuildValue("{s:I,s:I,s:I,s:s,s:I}",
                "index",        fmtdesc.index,
                "type",         fmtdesc.type,
                "flags",        fmtdesc.flags,
                "description",  fmtdesc.description,
                "pixelformat",  fmtdesc.pixelformat
                );
        PyList_Append(fmtlist, fmtinfo);
        Py_XDECREF(fmtinfo);
        fmtdesc.index++;
    }
    return fmtlist;
}

static PyObject *V4L2_s_fmt(V4L2Object *self, PyObject *args) {
    struct v4l2_format format;
    if (!PyArg_ParseTuple(args, "IIII",
                &format.fmt.pix.width,
                &format.fmt.pix.height,
                &format.fmt.pix.pixelformat,
                &format.fmt.pix.field)) {
        return NULL;
    }
    format.type=self->type;
    if (eioctl(self->fd, VIDIOC_S_FMT, &format, "ioctl(VIDIOC_S_FMT)")<0) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *V4L2_enuminput(V4L2Object *self) {
    struct v4l2_input input;
    memset(&input, 0, sizeof(struct v4l2_input));
    PyObject *inputlist, *inputinfo;
    input.index=0;
    inputlist=PyList_New(0);
    while(ioctl(self->fd, VIDIOC_ENUMINPUT, &input)!=-1) {
        inputinfo=Py_BuildValue(
                "{s:I,s:s,s:I,s:I,s:I,s:K,s:I,s:I}",
                "index",    input.index,
                "name",     input.name,
                "type",     input.type,
                "audioset", input.audioset,
                "tuner",    input.tuner,
                "std",      input.std,
                "status",   input.status,
                "capabilities",input.capabilities
                );
        PyList_Append(inputlist, inputinfo);
        Py_XDECREF(inputinfo);
        input.index++;
    }
    return inputlist;
}

static PyObject *V4L2_enumaudio(V4L2Object *self) {
    struct v4l2_audio audio;
    memset(&audio, 0, sizeof(struct v4l2_audio));
    PyObject *audiolist, *audioinfo;
    audio.index=0;
    audiolist=PyList_New(0);
    while(ioctl(self->fd, VIDIOC_ENUMAUDIO, &audio)!=-1) {
        audioinfo=Py_BuildValue(
                "{s:I,s:s,s:I,s:I}",
                "index",        audio.index,
                "name",         audio.name,
                "capability",   audio.capability,
                "mode",         audio.mode
                );
        PyList_Append(audiolist, audioinfo);
        Py_XDECREF(audioinfo);
        audio.index++;
    }
    return audiolist;
}

static PyObject *V4L2_s_input(V4L2Object *self, PyObject *args) {
    int inputnum;
    if (!PyArg_ParseTuple(args, "i",
                &inputnum)) {
        return NULL;
    }
    if (eioctl(self->fd, VIDIOC_S_INPUT, &inputnum, "ioctl(VIDIOC_S_INPUT")<0) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *V4L2_g_input(V4L2Object *self) {
    uint32_t inputnum;
    if (eioctl(self->fd, VIDIOC_G_INPUT, &inputnum, "ioctl(VIDIOC_G_INPUT)")<0) {
        return NULL;
    }
    return Py_BuildValue("I", inputnum);
}

static PyObject *V4L2_enumstd(V4L2Object *self) {
    struct v4l2_standard std;
    memset(&std, 0, sizeof(struct v4l2_standard));
    PyObject *stdlist, *stdinfo;
    std.index=0;
    stdlist=PyList_New(0);
    while(ioctl(self->fd, VIDIOC_ENUMSTD, &std)!=-1) {
        stdinfo=Py_BuildValue(
                "{s:I,s:K,s:s,s:I,s:{s:I,s:I}}",
                "index",        std.index,
                "id",           std.id,
                "name",         std.name,
                "framelines",   std.framelines,
                "frameperiod",
                "numerator",    std.frameperiod.numerator,
                "denominator",  std.frameperiod.denominator
                );
        PyList_Append(stdlist, stdinfo);
        Py_XDECREF(stdinfo);
        std.index++;
    }
    return stdlist;
}

static PyObject *V4L2_g_std(V4L2Object *self) {
    v4l2_std_id std_id;
    if (eioctl(self->fd, VIDIOC_G_STD, &std_id,
                "ioctl(VIDIOC_G_STD)")<0) {
        return NULL;
    }
    return Py_BuildValue("K",std_id);
}

static PyObject *V4L2_s_std(V4L2Object *self, PyObject *args) {
    v4l2_std_id std_id=0;
    if (!PyArg_ParseTuple(args, "K",
                &std_id)) {
        return NULL;
    }
    Py_BEGIN_ALLOW_THREADS;
    if (eioctl(self->fd, VIDIOC_S_STD, &std_id,
                "ioctl(VIDIOC_S_STD)")<0) {
        return NULL;
    }
    Py_END_ALLOW_THREADS;
    Py_RETURN_NONE;
}

// s_std()
// g_std()

static PyObject *V4L2_reqbufs(V4L2Object *self, PyObject *args) {
    struct v4l2_requestbuffers reqbufs;
    memset(&reqbufs, 0, sizeof(struct v4l2_requestbuffers));
    if (!PyArg_ParseTuple(args, "I",
                &reqbufs.count)) {
        return NULL;
    }
    reqbufs.type=self->type;
    reqbufs.memory=self->memory;
    if (eioctl(self->fd, VIDIOC_REQBUFS, &reqbufs,
                "ioctl(VIDIOC_REQBUFS)")<0) {
        return NULL;
    }
    return Py_BuildValue("i", reqbufs.count);
}

static PyObject *V4L2_querybuf(V4L2Object *self, PyObject *args) {
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(struct v4l2_buffer));
    PyObject *bufinfo;
    if (!PyArg_ParseTuple(args, "I",
                &buf.index)) {
        return NULL;
    }
    buf.type=self->type;
    buf.memory=self->memory;
    if (eioctl(self->fd, VIDIOC_QUERYBUF, &buf,
                "ioctl(VIDIOC_QUERYBUF)")<0) {
        return NULL;
    }
    bufinfo=Py_BuildValue(
            "{s:I,s:I,s:I,s:I,s:I,s:{s:I,s:I},s:{s:I,s:I,s:I,s:I,s:I,s:I},s:I,s:I,s:K,s:I}",
            "index",        buf.index,
            "type",         buf.type,
            "bytesused",    buf.bytesused,
            "flags",        buf.flags,
            "field",        buf.field,
            "timestamp",
            "tv_sec",       buf.timestamp.tv_sec,
            "tv_usec",      buf.timestamp.tv_usec,
            "timecode",
            "type",         buf.timecode.type,
            "flags",        buf.timecode.flags,
            "frames",       buf.timecode.frames,
            "seconds",      buf.timecode.seconds,
            "minutes",      buf.timecode.minutes,
            "hours",        buf.timecode.hours,
            //"userbits",     buf.timecode.userbits,  4,
            "sequence",     buf.sequence,
            "memory",       buf.memory,
            "offset",       buf.m.offset,
            "length",       buf.length
            //"input",        buf.input     // can not work in linux-3.12-1-amd64
            );
    return bufinfo;
}

static PyObject *V4L2_mmap(V4L2Object *self, PyObject *args) {
    uint64_t offset,length;
    void *addr;
    if (!PyArg_ParseTuple(args, "KK",
                &offset, &length)) {
        return NULL;
    }
    addr=mmap(NULL, length,
            PROT_READ|PROT_WRITE,
            MAP_SHARED,
            self->fd,
            offset);
    if (addr==NULL) {
        PyErr_Format(PyExc_OSError, "ERROR[%d]=\"%s\", %s",
                errno, strerror(errno), "mmap()");
        return NULL;
    }
    return Py_BuildValue("K",addr);
}

static PyObject *V4L2_munmap(V4L2Object *self, PyObject *args) {
    uint64_t length;
    void *addr;
    if (!PyArg_ParseTuple(args, "KK",
                &addr, &length)) {
        return NULL;
    }
    munmap(addr,length);
    Py_RETURN_NONE;
}

static PyObject *V4L2_qbuf(V4L2Object *self, PyObject *args) {
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(struct v4l2_buffer));
    if (!PyArg_ParseTuple(args, "I",
                &buf.index)) {
        return NULL;
    }
    buf.type=self->type;
    buf.memory=self->memory;
    Py_BEGIN_ALLOW_THREADS;
    if (eioctl(self->fd, VIDIOC_QBUF, &buf,
                "ioctl(VIDIOC_QBUF)")<0) {
        return NULL;
    }
    Py_END_ALLOW_THREADS;
    Py_RETURN_NONE;
}

static PyObject *V4L2_streamon(V4L2Object *self) {
    enum v4l2_buf_type type=self->type;
    Py_BEGIN_ALLOW_THREADS;
    if (eioctl(self->fd, VIDIOC_STREAMON, &type,
                "ioctl(VIDIOC_STREAMON)")<0) {
        return NULL;
    }
    Py_END_ALLOW_THREADS;
    Py_RETURN_NONE;
}

static PyObject *V4L2_streamoff(V4L2Object *self) {
    enum v4l2_buf_type type=self->type;
    Py_BEGIN_ALLOW_THREADS;
    if (eioctl(self->fd, VIDIOC_STREAMOFF, &type,
                "ioctl(VIDIOC_STREAMOFF)")<0) {
        return NULL;
    }
    Py_END_ALLOW_THREADS;
    Py_RETURN_NONE;
}

static PyObject *V4L2_dqbuf(V4L2Object *self) {
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(struct v4l2_buffer));
    buf.type=self->type;
    buf.memory=self->memory;
    Py_BEGIN_ALLOW_THREADS;
    if (eioctl(self->fd, VIDIOC_DQBUF, &buf,
                "ioctl(VIDIOC_DQBUF)")<0) {
        return NULL;
    }
    Py_END_ALLOW_THREADS;
    return Py_BuildValue("I", buf.index);
}

static PyObject *V4L2_getbuffer(V4L2Object *self, PyObject *args) {
    uint8_t *addr;
    Py_ssize_t length;
    if (!PyArg_ParseTuple(args, "li",
                &addr, &length)) {
        return NULL;
    }
    return PyBuffer_FromMemory(addr, length);
}

static PyObject *V4L2_getjpeg(V4L2Object *self, PyObject *args) {
    uint8_t *addr;
    Py_ssize_t length;
    if (!PyArg_ParseTuple(args, "li",
                &addr, &length)) {
        return NULL;
    }
    uint32_t i;
    /*for (i=length-1; i>=0 ; i--) {
        if (addr[i] != 0) {
            break;
        }
    }
    return PyBuffer_FromMemory(addr, i+1);*/
    //i=length-1;
    for (i=0; i<length; i++) {
        if (addr[i]==0xd9 && addr[i-1]==0xff) {
            break;
        }
    }
    return PyBuffer_FromMemory(addr, i+1);
}

static PyObject *V4L2_fileno(V4L2Object *self) {
    return Py_BuildValue("K", self->fd);
}

static PyObject *V4L2_queryctrl(V4L2Object *self, PyObject *args) {
    struct v4l2_queryctrl queryctrl;
    memset(&queryctrl, 0, sizeof(struct v4l2_queryctrl));
    if (!PyArg_ParseTuple(args, "I",
                &queryctrl.id)) {
        return NULL;
    }
    if (eioctl(self->fd, VIDIOC_QUERYCTRL, &queryctrl,
                "ioctl(VIDIOC_QUERYCTRL)")<0) {
        return NULL;
    }
    return Py_BuildValue(
            "{s:I,s:I,s:s,s:i,s:i,s:i,s:i,s:I}",
            "id",       queryctrl.id,
            "type",     queryctrl.type,
            "name",     queryctrl.name,
            "minimum",  queryctrl.minimum,
            "maximum",  queryctrl.maximum,
            "step",     queryctrl.step,
            "default_value",    queryctrl.default_value,
            "flags",    queryctrl.flags
            );
}

static PyObject *V4L2_querymenu(V4L2Object *self, PyObject *args) {
    struct v4l2_querymenu querymenu;
    memset(&querymenu, 0, sizeof(struct v4l2_querymenu));
    if (!PyArg_ParseTuple(args, "I",
                &querymenu.id)) {
        return NULL;
    }
    if (eioctl(self->fd, VIDIOC_QUERYMENU, &querymenu,
                "ioctl(VIDIOC_QUERYMENU)")<0) {
        return NULL;
    }
    return Py_BuildValue(
            "{s:I,s:I,s:s}",
            "id",       querymenu.id,
            "index",    querymenu.index,
            "name",     querymenu.name
            );
}

static PyObject *V4L2_g_ctrl(V4L2Object *self, PyObject *args) {
    struct v4l2_control ctrl;
    memset(&ctrl, 0, sizeof(struct v4l2_control));
    if (!PyArg_ParseTuple(args, "I",
                &ctrl.id)) {
        return NULL;
    }
    if (eioctl(self->fd, VIDIOC_G_CTRL, &ctrl,
                "ioctl(VIDIOC_G_CTRL)")<0) {
        return NULL;
    }
    return Py_BuildValue(
            "{s:I,s:i}",
            "id",       ctrl.id,
            "value",    ctrl.value
            );
}

static PyObject *V4L2_s_ctrl(V4L2Object *self, PyObject *args) {
    struct v4l2_control ctrl;
    memset(&ctrl, 0, sizeof(struct v4l2_control));
    if (!PyArg_ParseTuple(args, "Ii",
                &ctrl.id, &ctrl.value)) {
        return NULL;
    }
    if (eioctl(self->fd, VIDIOC_S_CTRL, &ctrl,
                "ioctl(VIDIOC_S_CTRL)")<0) {
        return NULL;
    }
    Py_RETURN_NONE;
}

// Module level members --------------------------------------------------------

static PyMethodDef V4L2_methods[] = {
    {"querycap",    (PyCFunction)V4L2_querycap,     METH_NOARGS,
        "querycap()"},
    {"enum_fmt",    (PyCFunction)V4L2_enum_fmt,     METH_NOARGS,
        "enum_fmt(type)"},
    {"s_fmt",       (PyCFunction)V4L2_s_fmt,        METH_VARARGS,
        "s_fmt(width,height,pixelformat,field)"},
    {"enuminput",   (PyCFunction)V4L2_enuminput,    METH_NOARGS,
        "enuminput()"},
    {"enumaudio",   (PyCFunction)V4L2_enumaudio,    METH_NOARGS,
        "enumaudio()"},
    {"s_input",     (PyCFunction)V4L2_s_input,      METH_VARARGS,
        "s_input(inputnum)"},
    {"g_input",     (PyCFunction)V4L2_g_input,      METH_VARARGS,
        "g_input()"},
    {"enumstd",     (PyCFunction)V4L2_enumstd,      METH_NOARGS,
        "enumstd()"},
    {"g_std",       (PyCFunction)V4L2_g_std,        METH_NOARGS,
        "g_std()"},
    {"s_std",       (PyCFunction)V4L2_s_std,        METH_VARARGS,
        "s_std(std_id)"},
    {"reqbufs",     (PyCFunction)V4L2_reqbufs,      METH_VARARGS,
        "reqbufs(count,type,memory)"},
    {"querybuf",    (PyCFunction)V4L2_querybuf,     METH_VARARGS,
        "querybuf(bufidx,type,memory)"},
    {"mmap",        (PyCFunction)V4L2_mmap,         METH_VARARGS,
        "mmap(offset,length)"},
    {"munmap",      (PyCFunction)V4L2_munmap,       METH_VARARGS,
        "munmap(addr,length)"},
    {"qbuf",        (PyCFunction)V4L2_qbuf,         METH_VARARGS,
        "qbuf(bufidx)"},
    {"streamon",    (PyCFunction)V4L2_streamon,     METH_NOARGS,
        "stream()"},
    {"streamoff",   (PyCFunction)V4L2_streamoff,    METH_NOARGS,
        "streamoff()"},
    {"dqbuf",       (PyCFunction)V4L2_dqbuf,        METH_NOARGS,
        "dqbuf()"},
    {"getbuffer",   (PyCFunction)V4L2_getbuffer,    METH_VARARGS,
        "getbuffer(addr,length)"},
    {"getjpeg",     (PyCFunction)V4L2_getjpeg,      METH_VARARGS,
        "getjpeg(addr,length)"},
    {"fileno",      (PyCFunction)V4L2_fileno,       METH_NOARGS,
        "fileno()"},
    {"queryctrl",   (PyCFunction)V4L2_queryctrl,    METH_VARARGS,
        "queryctrl(ctrl_id)"},
    {"querymenu",   (PyCFunction)V4L2_querymenu,    METH_VARARGS,
        "querymenu(menu_id)"},
    {"g_ctrl",      (PyCFunction)V4L2_g_ctrl,       METH_VARARGS,
        "g_ctrl(ctrl_id)"},
    {"s_ctrl",      (PyCFunction)V4L2_s_ctrl,       METH_VARARGS,
        "s_ctrl(ctrl_id,value)"},
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
