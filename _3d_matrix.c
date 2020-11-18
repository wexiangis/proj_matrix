
#include "_3d_matrix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// 0:增量式,适合连续变化的旋转(会累积误差)
// 1:每次都从原始坐标进行XYZ依次旋转,适合一次旋转
#define _3D_SCROLL_MODE_SWITCH 1

#define _3D_LINE_SIZE 1

// 矩阵运算: 先旋转后平移
void _3D_matrix_scroll_move_calculate(double rollXYZ[3], double movXYZ[3], double xyz[3])
{
    double x, y, z;
    double Xrad, Yrad, Zrad;

    if (rollXYZ == NULL || movXYZ == NULL || xyz == NULL)
        return;

    x = xyz[0];
    y = xyz[1];
    z = xyz[2];
    Xrad = rollXYZ[0];
    Yrad = rollXYZ[1];
    Zrad = rollXYZ[2];

    /*      [scroll X]
    *   1       0       0
    *   0     cosA    -sinA
    *   0     sinA     cosA
    *
    *       [scroll Y]
    *  cosB     0      sinB
    *   0       1       0
    * -sinB     0      cosB
    *
    *       [scroll Z]
    *  cosC   -sinC     0
    *  sinC    cosC     0
    *   0       0       1
    *
    *                                          |x|
    *   result = [scroll X][scroll Y][scroll Z]|y|
    *                                          |z|
    *
    *            |xyz[0]|
    *          = |xyz[1]|
    *            |xyz[2]|
    *
    *   xyz[*] just like the following ...
    */

    xyz[0] =
        x * cos(Yrad) * cos(Zrad) - y * cos(Yrad) * sin(Zrad) + z * sin(Yrad);
    xyz[1] =
        x * (sin(Xrad) * sin(Yrad) * cos(Zrad) + cos(Xrad) * sin(Zrad)) -
        y * (sin(Xrad) * sin(Yrad) * sin(Zrad) - cos(Xrad) * cos(Zrad)) -
        z * sin(Xrad) * cos(Yrad);
    xyz[2] =
        -x * (cos(Xrad) * sin(Yrad) * cos(Zrad) - sin(Xrad) * sin(Zrad)) +
        y * (cos(Xrad) * sin(Yrad) * sin(Zrad) + sin(Xrad) * cos(Zrad)) +
        z * cos(Xrad) * cos(Yrad);

    // move
    xyz[0] += movXYZ[0];
    xyz[1] += movXYZ[1];
    xyz[2] += movXYZ[2];
}

/*
 *  矩阵运算: 透视矩阵点乘三维坐标,然后除以z(透视除法),返回投影坐标[-ar, ar]U[-1, 1]
 * 
 *  参数:
 *      openAngle: 相机开角(单位:rad,范围:(0,pi))
 *      xyz[3]: 要计算的空间坐标
 *      ar: 相机的屏幕的宽高比
 *      nearZ: 相机近端距离
 *      farZ: 相机远端距离
 *      retXY: 计算结果,一个二维平面坐标(注意其坐标原点是屏幕中心)
 *      retDepth: 计算结果,深度值(远离屏幕的距离,单位:点)
 *      retRange: 计算结果,该点是否在相机可视范围内(用来决定要不要画这个点)
 * 
 *  返回: 0/不再相框内  1/在相框内
 */
int _3D_matrix_project_calculate(
    double openAngle,
    double xyz[3],
    double ar,
    int nearZ,
    int farZ,
    double *retXY,
    double *retDepth,
    bool *retRange)
{
    double hMax, hMin, wMax, wMin;
    double retX, retY, retZ;

    //快速检查
    if (openAngle >= _3D_PI ||
        ar <= 0 ||
        xyz == NULL ||
        xyz[2] == 0 ||
        nearZ >= farZ ||
        xyz[2] < nearZ ||
        xyz[2] > farZ)
        return 0;

    //屏幕高、宽范围(这里是假设屏幕高为2时的数值)
    hMax = 1;
    hMin = -1;
    wMax = ar;
    wMin = -ar;

    /*                          [project matrix]
    *
    *   1/ar/tan(a/2)           0               0               0   
    *       0               1/tan(a/2)          0               0
    *       0                   0     ((-nZ)-fZ)/(nZ-fZ)  2*fZ*nZ/(nZ-fZ)
    *       0                   0               1               0
    *
    *   ar: camera width/height
    *   a: camera open angle
    *   nZ: camera near Z
    *   fZ: camera far Z
    *
    *                   |x|               |retX|
    *   [project matrix]|y| and then /z = |retY|
    *                   |z|               |retZ|
    *                   |1|               | 1  |
    *
    *   output point request: retX in the range of (-ar, ar)
    *                         retY in the range of (-1, 1)
    *                         retZ in the range of (-1, 1)
    */

    retX = xyz[0] / ar / tan(openAngle / 2) / xyz[2];
    retY = xyz[1] / tan(openAngle / 2) / xyz[2];
    retZ = ((-nearZ) - farZ) / (nearZ - farZ) + 2 * farZ * nearZ / (nearZ - farZ) / xyz[2];
    //返回二维坐标
    if (retXY)
    {
        retXY[0] = retX;
        retXY[1] = retY;
    }
    //深度
    if (retDepth)
        *retDepth = xyz[2] - nearZ;
    //是否在相框范围内
    if (retRange)
        *retRange = false;
    if (wMax > retX && retX > wMin &&
        hMax > retY && retY > hMin &&
        1 > retZ && retZ > -1)
    {
        if (retRange)
            *retRange = true;
        return 1;
    }
    return 0;
}

/*
 *  相机初始化(可视范围设置)
 *  参数:
 *      width, height: 相机屏幕宽高
 *      openAngle: 相机视野开角,单位:rad,范围:(0,pi)
 *      nearZ: 相机近端距离,单位:点
 *      farZ: 相机远端端距离,单位:点
 *      rollXYZ, movXYZ: 可选预置摄像机转角和位置,不用则置NULL
 *  说明: 近端和远端决定了一个远近范围,三维坐标点(x,y,z)的z值要在(nearZ,farZ)范围内才会被摄像机看到
 *  返回: NULL/失败
 */
_3D_Camera_Type *_3D_camera_init(
    int width,
    int height,
    double openAngle,
    int nearZ,
    int farZ,
    double *rollXYZ,
    double *movXYZ)
{
    _3D_Camera_Type *dct;

    if (width <= 0 || height <= 0 || openAngle >= _3D_PI)
        return NULL;

    dct = (_3D_Camera_Type *)calloc(1, sizeof(_3D_Camera_Type));
    dct->copy = (_3D_Camera_Type *)calloc(1, sizeof(_3D_Camera_Type));

    dct->width = width;
    dct->height = height;
    dct->ar = width / height;
    dct->openAngle = openAngle;
    dct->nearZ = nearZ;
    dct->farZ = farZ;

    if (rollXYZ)
        memcpy(dct->rollXYZ, rollXYZ, sizeof(double) * 3);
    if (movXYZ)
        memcpy(dct->movXYZ, movXYZ, sizeof(double) * 3);
        
    memcpy(dct->copy, dct, sizeof(_3D_Camera_Type));
    return dct;
}

// 相机: 旋转和平移
void _3D_camera_scroll(_3D_Camera_Type *dct, double *scrollXYZ, double *movXYZ)
{
    if (dct == NULL)
        return;

    dct->rollXYZ[0] += scrollXYZ[0];
    dct->rollXYZ[1] += scrollXYZ[1];
    dct->rollXYZ[2] += scrollXYZ[2];

    if (dct->rollXYZ[0] >= 2 * _3D_PI)
        dct->rollXYZ[0] -= 2 * _3D_PI;
    else if (dct->rollXYZ[0] < 0)
        dct->rollXYZ[0] += 2 * _3D_PI;
    if (dct->rollXYZ[1] >= 2 * _3D_PI)
        dct->rollXYZ[1] -= 2 * _3D_PI;
    else if (dct->rollXYZ[1] < 0)
        dct->rollXYZ[1] += 2 * _3D_PI;
    if (dct->rollXYZ[2] >= 2 * _3D_PI)
        dct->rollXYZ[2] -= 2 * _3D_PI;
    else if (dct->rollXYZ[2] < 0)
        dct->rollXYZ[2] += 2 * _3D_PI;

    dct->movXYZ[0] += movXYZ[0];
    dct->movXYZ[1] += movXYZ[1];
    dct->movXYZ[2] += movXYZ[2];

#if (_3D_SCROLL_MODE_SWITCH) //mode/1: 使用原始的坐标和累积的转角量,一次转换到目标坐标
    memcpy(dct->xyz, dct->xyzCopy, sizeof(dct->xyz));
#endif

    dct->xyz[0] += dct->movXYZ[0];
    dct->xyz[1] += dct->movXYZ[1];
    dct->xyz[2] += dct->movXYZ[2];

#if (!_3D_SCROLL_MODE_SWITCH) //mode/0: 每次转换都使用的上次转换的坐标,转角量使用过后清零
    memset(dct->rollXYZ, 0, sizeof(dct->rollXYZ));
    memset(dct->movXYZ, 0, sizeof(dct->movXYZ));
#endif
}

// 相机: 复位到初始化状态
void _3D_camera_reset(_3D_Camera_Type *dct)
{
    _3D_Camera_Type *dctCopy;
    if (dct == NULL || dct->copy == NULL)
        return;
    dctCopy = dct->copy;
    memcpy(dct, dctCopy, sizeof(_3D_Camera_Type));
    dct->copy = dctCopy;
}

// 多边图型: 初始化
_3D_PointArray_Type *_3D_pointArray_init(int pointNum, double x, double y, double z, int color, ...)
{
    int i, array_count, col_count;
    va_list ap;
    _3D_PointArray_Type *dpat;
    //
    if (pointNum < 1)
        return NULL;
    //
    dpat = (_3D_PointArray_Type *)calloc(1, sizeof(_3D_PointArray_Type));
    //
    dpat->pointNum = pointNum;
    dpat->xyzArrayMemSize = pointNum * 3 * sizeof(double);
    dpat->xyzArray = (double *)calloc((pointNum + 1) * 3, sizeof(double));
    dpat->xyzArrayCopy = (double *)calloc((pointNum + 1) * 3, sizeof(double));

    dpat->xyArray = (double *)calloc((pointNum + 1) * 2, sizeof(double));
    dpat->depthArray = (double *)calloc(pointNum + 1, sizeof(double));
    dpat->rangeArray = (bool *)calloc(pointNum + 1, sizeof(bool));

    dpat->colorArray = (int *)calloc(pointNum + 1, sizeof(int));
    //
    dpat->xyzArray[0] = x;
    dpat->xyzArray[1] = y;
    dpat->xyzArray[2] = z;
    dpat->colorArray[0] = color;
    va_start(ap, color);
    for (i = 1, array_count = 3, col_count = 1; i < pointNum; i++)
    {
        dpat->xyzArray[array_count++] = va_arg(ap, double);
        dpat->xyzArray[array_count++] = va_arg(ap, double);
        dpat->xyzArray[array_count++] = va_arg(ap, double);
        dpat->colorArray[col_count++] = va_arg(ap, int);
    }
    va_end(ap);
    memcpy(dpat->xyzArrayCopy, dpat->xyzArray, dpat->xyzArrayMemSize);
    //
    return dpat;
}

// 多边图型: 旋转和平移
void _3D_pointArray_scroll(_3D_PointArray_Type *dpat, double *scrollXYZ, double *movXYZ)
{
    int i, j;
    _3D_Comment_Type *comment;
    //
    if (dpat == NULL)
        return;
    //
    dpat->rollXYZ[0] += scrollXYZ[0];
    dpat->rollXYZ[1] += scrollXYZ[1];
    dpat->rollXYZ[2] += scrollXYZ[2];
    //
    if (dpat->rollXYZ[0] >= 2 * _3D_PI)
        dpat->rollXYZ[0] -= 2 * _3D_PI;
    else if (dpat->rollXYZ[0] < 0)
        dpat->rollXYZ[0] += 2 * _3D_PI;

    if (dpat->rollXYZ[1] >= 2 * _3D_PI)
        dpat->rollXYZ[1] -= 2 * _3D_PI;
    else if (dpat->rollXYZ[1] < 0)
        dpat->rollXYZ[1] += 2 * _3D_PI;

    if (dpat->rollXYZ[2] >= 2 * _3D_PI)
        dpat->rollXYZ[2] -= 2 * _3D_PI;
    else if (dpat->rollXYZ[2] < 0)
        dpat->rollXYZ[2] += 2 * _3D_PI;
    //
    dpat->movXYZ[0] += movXYZ[0];
    dpat->movXYZ[1] += movXYZ[1];
    dpat->movXYZ[2] += movXYZ[2];
    //
    for (i = 0, j = 0; i < dpat->pointNum; i++)
    {
#if (_3D_SCROLL_MODE_SWITCH) //mode/1: 使用原始的坐标和累积的转角量,一次转换到目标坐标
        memcpy(&dpat->xyzArray[j], &dpat->xyzArrayCopy[j], 3 * sizeof(double));
#endif
        _3D_matrix_scroll_move_calculate(dpat->rollXYZ, dpat->movXYZ, &dpat->xyzArray[j]);
        j += 3;
    }
    //
    comment = dpat->comment;
    while (comment)
    {
#if (_3D_SCROLL_MODE_SWITCH) //mode/1: 使用原始的坐标和累积的转角量,一次转换到目标坐标
        memcpy(comment->xyz, comment->xyzCopy, sizeof(comment->xyz));
#endif
        _3D_matrix_scroll_move_calculate(dpat->rollXYZ, dpat->movXYZ, comment->xyz);
        comment = comment->next;
    }

#if (!_3D_SCROLL_MODE_SWITCH) //mode/0: 每次转换都使用的上次转换的坐标,转角量使用过后清零
    memset(dpat->rollXYZ, 0, sizeof(dpat->rollXYZ));
    memset(dpat->movXYZ, 0, sizeof(dpat->movXYZ));
#endif
}

// 多边图型: 复位到初始化状态
void _3D_pointArray_reset(_3D_PointArray_Type *dpat)
{
    _3D_Comment_Type *comment;
    //
    if (dpat == NULL)
        return;
    //
    memset(dpat->rollXYZ, 0, sizeof(dpat->rollXYZ));
    memset(dpat->movXYZ, 0, sizeof(dpat->movXYZ));
    //
    memcpy(dpat->xyzArray, dpat->xyzArrayCopy, dpat->xyzArrayMemSize);
    //
    comment = dpat->comment;
    while (comment)
    {
        memcpy(comment->xyz, comment->xyzCopy, sizeof(comment->xyz));
        //
        comment = comment->next;
    }
}

// 多边图型: 添加各顶点连接关系
void _3D_pointArray_ppLink_add(_3D_PointArray_Type *dpat, int currentOrder, int targetOrderNum, int targetOrder, ...)
{
    int i;
    _3D_PPLink_Type *link;
    va_list ap;
    //
    if (dpat == NULL || targetOrderNum < 1)
        return;
    //
    if (dpat->link == NULL)
    {
        dpat->link = (_3D_PPLink_Type *)calloc(1, sizeof(_3D_PPLink_Type));
        link = dpat->link;
    }
    else
    {
        link = dpat->link;
        while (link->next)
            link = link->next;
        link->next = (_3D_PPLink_Type *)calloc(1, sizeof(_3D_PPLink_Type));
        link = link->next;
    }
    //
    link->currentOrder = currentOrder;
    link->targetOrderNum = targetOrderNum;
    link->targetOrderArray = (int *)calloc(targetOrderNum + 1, sizeof(int));
    //
    va_start(ap, targetOrder);
    link->targetOrderArray[0] = targetOrder;
    for (i = 1; i < targetOrderNum; i++)
        link->targetOrderArray[i] = va_arg(ap, int);
    va_end(ap);
}

// 多边图型: 添加特定位置的注释
void _3D_pointArray_comment_add(_3D_PointArray_Type *dpat, double x, double y, double z, char *comment, int color)
{
    _3D_Comment_Type *comm;
    //
    if (dpat == NULL)
        return;
    //
    if (dpat->comment == NULL)
    {
        dpat->comment = (_3D_Comment_Type *)calloc(1, sizeof(_3D_Comment_Type));
        comm = dpat->comment;
    }
    else
    {
        comm = dpat->comment;
        while (comm->next)
            comm = comm->next;
        comm->next = (_3D_Comment_Type *)calloc(1, sizeof(_3D_Comment_Type));
        comm = comm->next;
    }
    //
    comm->xyz[0] = x;
    comm->xyz[1] = y;
    comm->xyz[2] = z;
    memcpy(comm->xyzCopy, comm->xyz, sizeof(comm->xyz));
    comm->color = color;
    comm->comment = (char *)calloc(strlen(comment) + 16, sizeof(char));
    strcpy(comm->comment, comment);
}

#include "view.h"

// 相机: 多边图型投影到相机
void _3D_camera_show(_3D_Camera_Type *dct, _3D_PointArray_Type *dpat)
{
    int i, j, k;
    int cP, tP, cP2, tP2;
    _3D_Comment_Type *comment;
    _3D_PPLink_Type *link;
    double noMove[3] = {0.0, 0.0, 0.0};
    double tmpXYZ[3];

    if (dct == NULL || dpat == NULL)
        return;

    //---------- 3D to 2D ----------

    //遍历每个点,计算位置并绘制
    for (i = 0, j = 0, k = 0; i < dpat->pointNum; i++)
    {
        //摄像机跑路了,空间坐标点也要跟着跑路
        tmpXYZ[0] = dpat->xyzArray[j] - dct->xyz[0];
        tmpXYZ[1] = dpat->xyzArray[j + 1] - dct->xyz[1];
        tmpXYZ[2] = dpat->xyzArray[j + 2] - dct->xyz[2];
        _3D_matrix_scroll_move_calculate(dct->rollXYZ, noMove, tmpXYZ);
        //三维坐标点投影到摄像机屏幕
        _3D_matrix_project_calculate(
            dct->openAngle,
            tmpXYZ,
            dct->ar,
            dct->nearZ,
            dct->farZ,
            &dpat->xyArray[k],
            &dpat->depthArray[i],
            &dpat->rangeArray[i]);
        //由于投影矩阵计算时是假设屏幕高为2(继而宽为ar)的情况下计算,这里得到的二维坐标需转换一下
        //先把坐标转换为[-1,1]范围内,再通过1+和1-把原点移到屏幕中心,再转换到真实比例
        dpat->xyArray[k] = (1 + dpat->xyArray[k] / dct->ar) * (dct->width / 2);
        dpat->xyArray[k + 1] = (1 - dpat->xyArray[k + 1]) * (dct->height / 2);
        //在屏幕上画点
        view_dot(dpat->colorArray[i], dpat->xyArray[k], dpat->xyArray[k + 1], 2);
        //数组平移
        j += 3;
        k += 2;
    }

    //注释位置计算和绘制
    comment = dpat->comment;
    while (comment)
    {
        //摄像机跑路了,空间坐标点也要跟着跑路
        tmpXYZ[0] = comment->xyz[0] - dct->xyz[0];
        tmpXYZ[1] = comment->xyz[1] - dct->xyz[1];
        tmpXYZ[2] = comment->xyz[2] - dct->xyz[2];
        _3D_matrix_scroll_move_calculate(dct->rollXYZ, noMove, tmpXYZ);
        //三维坐标点投影到摄像机屏幕
        _3D_matrix_project_calculate(
            dct->openAngle,
            tmpXYZ,
            dct->ar,
            dct->nearZ,
            dct->farZ,
            comment->xy,
            &comment->depth,
            &comment->range);
        //同上
        comment->xy[0] = (1 + comment->xy[0] / dct->ar) * dct->width / 2;
        comment->xy[1] = (1 - comment->xy[1]) * dct->height / 2;
        //
        comment = comment->next;
    }

    //---------- draw point to point ----------

    // 点到点连线绘制
    link = dpat->link;
    while (link)
    {
        cP = link->currentOrder;
        cP2 = link->currentOrder * 2;
        for (i = 0; i < link->targetOrderNum; i++)
        {
            tP = link->targetOrderArray[i];
            tP2 = link->targetOrderArray[i] * 2;
            // have one point in range of camera
            if (dpat->rangeArray[cP] || dpat->rangeArray[tP])
            {
                view_line(
                    (dpat->colorArray[cP] + dpat->colorArray[tP]) / 2,
                    dpat->xyArray[cP2], dpat->xyArray[cP2 + 1],
                    dpat->xyArray[tP2], dpat->xyArray[tP2 + 1],
                    _3D_LINE_SIZE, 0);
            }
        }
        //
        link = link->next;
    }
    //
    comment = dpat->comment;
    while (comment)
    {
        if (comment->range)
            view_string(comment->color, -1, comment->comment, comment->xy[0], comment->xy[1], 160, 0);
        //
        comment = comment->next;
    }
}
