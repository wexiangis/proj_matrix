
#ifndef __3D_MATRIX_H_
#define __3D_MATRIX_H_

#include <stdarg.h>     //变长参数

//---------- data type ----------

typedef struct _3D_Camera{
    int width;
    int hegiht;
    double ar;  //width/height

    double cameraOpenAngle;

    int nearZ;
    int farZ;
    
    double scroll_XYZ[3];
    double move_XYZ[3];

    double xyz[3];
}_3D_Camera_Type;

typedef struct _3D_PointArray{
    double scroll_xyz[3];
    double move_xyz[3];

    double *xyzArray;     //x1,y1,z1;x2,y2,z2;...
    double *xyzArrayCopy; //recover data

    int *xyArray;       //x1,y1;x2,y2;...

    int *colorArray;  //col1;col2...

    int pointNum;
    int arrayMemSize;
    
    struct _3D_PPLink{
        int currentOrder;        //选定端点
        int *targetOrderArray;   //要连接的目标点数组
        int targetOrderNum;      //目标数量
        struct _3D_PPLink *next; //建立下一个连接关系
    }*link;
    
    struct _3D_Comment{
        double xyz[3];
        double xyzCopy[3];
        int xy[2];
        char *comment;
        int color;
        struct _3D_Comment *next;
    }*comment;

}_3D_PointArray_Type;

//---------- function ----------

_3D_Camera_Type *_3D_camera_init(
    int width, int height, 
    int nearZ, int farZ);

void _3D_camera_scroll(
    _3D_Camera_Type *dct, 
    double scrollX, double scrollY, double scrollZ, 
    double movX, double movY, double movZ);

void _3D_camera_show(_3D_Camera_Type *dct, _3D_PointArray_Type *dpat);

_3D_PointArray_Type *_3D_pointArray_init(
    int pointNum, 
    double x, double y, double z, int color, ...);

void _3D_pointArray_scroll(
    _3D_PointArray_Type *dpat, 
    double scrollX, double scrollY, double scrollZ, 
    double movX, double movY, double movZ);

void _3D_pointArray_reset(_3D_PointArray_Type *dpat);

void _3D_pointArray_ppLink_add(
    _3D_PointArray_Type *dpat, 
    int currentOrder, 
    int targetOrderNum, 
    int targetOrder, ...);

void _3D_pointArray_comment_add(
    _3D_PointArray_Type *dpat, 
    double x, double y, double z, 
    char *comment, int color);

#endif

