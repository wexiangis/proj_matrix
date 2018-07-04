
#ifndef __3D_MATRIX_H_
#define __3D_MATRIX_H_

#include <stdbool.h>
#include <stdarg.h>     //变长参数

//---------- base data ----------

#define  _3D_PI     3.1415926535897

//---------- data type ----------

typedef struct _3D_Camera{
    int width;
    int height;
    double ar;  //width/height

    double cameraOpenAngle;

    int nearZ;
    int farZ;
    
    double scroll_xyz[3];
    double move_xyz[3];

    double xyz[3];
    double xyzCopy[3];
    //
    struct _3D_Camera *copy;
}_3D_Camera_Type;

typedef struct _3D_PPLink{
    int currentOrder;        //选定端点
    int *targetOrderArray;   //要连接的目标点数组
    int targetOrderNum;      //目标数量
    struct _3D_PPLink *next; //建立下一个连接关系
}_3D_PPLink_Type;

typedef struct _3D_Comment{
    double xyz[3];
    double xyzCopy[3];
    double xy[2];
    double depth;
    bool range;
    char *comment;
    int color;
    struct _3D_Comment *next;
}_3D_Comment_Type;

typedef struct _3D_PointArray{
    double scroll_xyz[3];
    double move_xyz[3];

    int pointNum;
    int arrayMemSize;

    double *xyzArray;     //x1,y1,z1;x2,y2,z2;...
    double *xyzArrayCopy; //recover data

    double *xyArray;       //x1,y1;x2,y2;...
    double *depthArray;
    bool *rangeArray;

    int *colorArray;    //col1;col2...
    
    _3D_PPLink_Type *link;
    _3D_Comment_Type *comment;

}_3D_PointArray_Type;

//---------- function ----------

_3D_Camera_Type *_3D_camera_init(
    int width, 
    int height, 
    double cameraOpenAngle, 
    int nearZ, 
    int farZ);

void _3D_camera_scroll(
    _3D_Camera_Type *dct, 
    double scrollX, 
    double scrollY, 
    double scrollZ, 
    double movX, 
    double movY, 
    double movZ);

void _3D_camera_show(
    _3D_Camera_Type *dct, 
    _3D_PointArray_Type *dpat);

void _3D_camera_reset(_3D_Camera_Type *dct);

_3D_PointArray_Type *_3D_pointArray_init(
    int pointNum, 
    double x, 
    double y, 
    double z, 
    int color, ...);

void _3D_pointArray_scroll(
    _3D_PointArray_Type *dpat, 
    double scrollX, 
    double scrollY, 
    double scrollZ, 
    double movX, 
    double movY, 
    double movZ);

void _3D_pointArray_reset(_3D_PointArray_Type *dpat);

void _3D_pointArray_ppLink_add(
    _3D_PointArray_Type *dpat, 
    int currentOrder, 
    int targetOrderNum, 
    int targetOrder, ...);

void _3D_pointArray_comment_add(
    _3D_PointArray_Type *dpat, 
    double x, 
    double y, 
    double z, 
    char *comment, 
    int color);

void _3D_matrix_scroll_move_calculate(
    double scroll_xyz[3], 
    double move_xyz[3], 
    double xyz[3]);

int _3D_matrix_project_calculate(
    double cameraOpenAngle, 
    double xyz[3], 
    double ar, 
    int nearZ, 
    int farZ, 
    double *retXY, 
    double *retDepth, 
    bool *retRange);

#endif

