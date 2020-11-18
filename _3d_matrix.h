
#ifndef __3D_MATRIX_H_
#define __3D_MATRIX_H_

#include <stdbool.h>
#include <stdarg.h> //变长参数

#define _3D_PI 3.1415926535897

typedef struct _3D_Camera
{
    /*
     *  定义相机可视范围(相对空间坐标系):
     *      x: -width/2 ~ width/2
     *      y: -height/2 ~ height/2
     *      z: nearZ ~ farZ
     */
    int width;        //摄像屏幕宽
    int height;       //摄像屏幕高
    double ar;        //屏幕宽高比: ar = width / height
    double openAngle; //摄像机视野开角,范围(0,PI)
    int nearZ;        //近端距离(屏幕到相机原点距离,即openAngle所在原点的距离)
    int farZ;         //远端距离(远端到相机原点距离）

    double scroll_xyz[3]; //绕转弧度(单位:rad)
    double move_xyz[3];   //平移量

    double xyz[3];     //相机原点所在坐标
    double xyzCopy[3]; //备份

    struct _3D_Camera *copy; //相机初始状态完全备份
} _3D_Camera_Type;

typedef struct _3D_PPLink
{
    int currentOrder;        //选定端点
    int *targetOrderArray;   //要连接的目标点数组
    int targetOrderNum;      //目标数量
    struct _3D_PPLink *next; //建立下一个连接关系
} _3D_PPLink_Type;

typedef struct _3D_Comment
{
    double xyz[3];
    double xyzCopy[3];
    double xy[2];
    double depth;
    bool range;
    char *comment;
    int color;
    struct _3D_Comment *next;
} _3D_Comment_Type;

typedef struct _3D_PointArray
{
    double scroll_xyz[3];
    double move_xyz[3];

    int pointNum;
    int xyzArrayMemSize;

    double *xyzArray;     //x1,y1,z1;x2,y2,z2;...
    double *xyzArrayCopy; //recover data

    double *xyArray; //x1,y1;x2,y2;...
    double *depthArray;
    bool *rangeArray;

    int *colorArray; //col1;col2...

    _3D_PPLink_Type *link;
    _3D_Comment_Type *comment;

} _3D_PointArray_Type;

_3D_Camera_Type *_3D_camera_init(
    int width,
    int height,
    double openAngle,
    int nearZ,
    int farZ);

void _3D_camera_scroll(
    _3D_Camera_Type *dct,
    double *scrollXYZ,
    double *movXYZ);

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
    double *scrollXYZ,
    double *movXYZ);

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
    double openAngle,
    double xyz[3],
    double ar,
    int nearZ,
    int farZ,
    double *retXY,
    double *retDepth,
    bool *retRange);

#endif
