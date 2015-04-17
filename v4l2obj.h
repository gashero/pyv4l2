/*
 * File: v4l2obj.h
 * Date: 2015-04-17
 * Author: gashero
 */

#ifndef V4L2OBJ_H
#define V4L2OBJ_H

#ifdef __cplusplus
extern "C" {
#endif

PyAPI_DATA(PyTypeObject) V4L2Type;

#define V4L2_Check(op) (Py_TYPE(op)==&V4L2Type)

#ifdef __cplusplus
}
#endif

#endif
