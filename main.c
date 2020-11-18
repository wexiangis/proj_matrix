
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "_3d_matrix.h"
#include "view.h"

#define SCROLL_DIV  (_3D_PI/16)
#define MOVE_DIV  10

int main(void)
{
    char input[16];
    double raxyz[3] = {0}, mvxyz[3] = {0};
    double raxyz2[3] = {0}, mvxyz2[3] = {0};
    //预置摄像机位置
    double cRollXYZ[3] = {0.0, -_3D_PI / 4, 0.0};
    double cMovXYZ[3] = {-120, 50, -120};
    // 初始化一个camera
    _3D_Camera_Type *camera;
    //初始化一个多边形
    _3D_PointArray_Type *dpat, *dpat2;

    // camera init
    if((camera = _3D_camera_init(
        VIEW_X_SIZE, 
        VIEW_Y_SIZE, 
        (_3D_PI / 2), 
        5, 
        1000, 
        cRollXYZ, 
        cMovXYZ)) == NULL)
    {
        printf("camera init failed\r\n");
        return -1;
    }
    
    //长方体
    if((dpat = _3D_pointArray_init(8, 
        30.00, 40.00, 70.00, 0xFF00FF,
        30.00, -40.00, 70.00, 0xFFFF00,
        -30.00, -40.00, 70.00, 0x00FFFF,
        -30.00, 40.00, 70.00, 0xFF8000,
        -30.00, 40.00, -70.00, 0xFF00FF,
        -30.00, -40.00, -70.00, 0xFFFF00,
        30.00, -40.00, -70.00, 0x00FFFF,
        30.00, 40.00, -70.00, 0xFF8000)) == NULL)
    {
        printf("_3D_pointArray_init failed\r\n");
        return -1;
    }
    _3D_pointArray_ppLink_add(dpat, 0, 3, 1, 3, 7);
    _3D_pointArray_ppLink_add(dpat, 1, 2, 2, 6);
    _3D_pointArray_ppLink_add(dpat, 2, 2, 3, 5);
    _3D_pointArray_ppLink_add(dpat, 3, 1, 4);
    _3D_pointArray_ppLink_add(dpat, 4, 2, 5, 7);
    _3D_pointArray_ppLink_add(dpat, 5, 1, 6);
    _3D_pointArray_ppLink_add(dpat, 6, 1, 7);
    _3D_pointArray_comment_add(dpat, 30.00, 40.00, 70.00, "A", 0xFFFF00);
    _3D_pointArray_comment_add(dpat, 30.00, -40.00, 70.00, "B", 0x00FF00);
    _3D_pointArray_comment_add(dpat, -30.00, -40.00, 70.00, "C", 0x8080FF);
    _3D_pointArray_comment_add(dpat, -30.00, 40.00, 70.00, "D", 0xFF0000);
    _3D_pointArray_comment_add(dpat, -30.00, 40.00, -70.00, "E", 0xFF00FF);
    _3D_pointArray_comment_add(dpat, -30.00, -40.00, -70.00, "F", 0x00FFFF);
    _3D_pointArray_comment_add(dpat, 30.00, -40.00, -70.00, "G", 0xFF8000);
    _3D_pointArray_comment_add(dpat, 30.00, 40.00, -70.00, "H", 0x0080FF);

    //XYZ
    if((dpat2 = _3D_pointArray_init(6, 
        150.00, 0.00, 0.00, 0xFF0000, 
        -150.00, 0.00, 0.00, 0xFF0000, 
        0.00, 150.00, 0.00, 0x00FF00, 
        0.00, -150.00, 0.00, 0x00FF00, 
        0.00, 0.00, 150.00, 0x00FFFF, 
        0.00, 0.00, -150.00, 0x00FFFF)) == NULL)
    {
        printf("_3D_pointArray_init failed\r\n");
        return -1;
    }
    _3D_pointArray_ppLink_add(dpat2, 0, 1, 1);
    _3D_pointArray_ppLink_add(dpat2, 2, 1, 3);
    _3D_pointArray_ppLink_add(dpat2, 4, 1, 5);
    _3D_pointArray_comment_add(dpat2, 150.00, 0.00, 0.00, "X", 0xFF0000);
    _3D_pointArray_comment_add(dpat2, 0.00, 150.00, 0.00, "Y", 0x00FF00);
    _3D_pointArray_comment_add(dpat2, 0.00, 0.00, 150.00, "Z", 0x00FFFF);

    //初始转角
    // raxyz[0] = _3D_PI/8;
    // raxyz[1] = _3D_PI/8;
    // raxyz[2] = _3D_PI/8;
    //初始平移
    // mvxyz[0] = -10;
    // mvxyz[1] = -10;
    // mvxyz[2] = 0;

    while(1)
    {
        PRINT_CLEAR();

        _3D_camera_scroll(camera, raxyz2, mvxyz2);

        _3D_pointArray_scroll(dpat, raxyz, mvxyz);
        // _3D_pointArray_scroll(dpat2, raxyz, mvxyz);
        
        _3D_camera_show(camera, dpat2);
        _3D_camera_show(camera, dpat);

        PRINT_EN();

        memset(raxyz, 0, sizeof(raxyz));
        memset(mvxyz, 0, sizeof(mvxyz));
        memset(raxyz2, 0, sizeof(raxyz2));
        memset(mvxyz2, 0, sizeof(mvxyz2));

        // raxyz[0] += SCROLL_DIV;
        // raxyz[1] += SCROLL_DIV;
        // raxyz[2] += SCROLL_DIV;
        // mvxyz[0] += 0;
        // mvxyz[1] += 0;
        // mvxyz[2] += 0;

        if(scanf("%s", input))
        // if(0)
        {
            //x scroll
            if(input[0] == 'w')
                raxyz[0] += SCROLL_DIV*strlen(input);
            else if(input[0] == '2')
                raxyz[0] -= SCROLL_DIV*strlen(input);
            //y scroll
            else if(input[0] == 'e')
                raxyz[1] += SCROLL_DIV*strlen(input);
            else if(input[0] == 'q')
                raxyz[1] -= SCROLL_DIV*strlen(input);
            //z scroll
            else if(input[0] == '1')
                raxyz[2] += SCROLL_DIV*strlen(input);
            else if(input[0] == '3')
                raxyz[2] -= SCROLL_DIV*strlen(input);
            //depth
            if(input[0] == 'a')
                mvxyz[2] += MOVE_DIV*strlen(input);
            else if(input[0] == 'd')
                mvxyz[2] -= MOVE_DIV*strlen(input);
            //right/left
            else if(input[0] == 'c')
                mvxyz[0] += MOVE_DIV*strlen(input);
            else if(input[0] == 'z')
                mvxyz[0] -= MOVE_DIV*strlen(input);
            //up/down
            else if(input[0] == 's')
                mvxyz[1] += MOVE_DIV*strlen(input);
            else if(input[0] == 'x')
                mvxyz[1] -= MOVE_DIV*strlen(input);

            else if(input[0] == 'R')
            {
                _3D_camera_reset(camera);
                _3D_pointArray_reset(dpat);
                _3D_pointArray_reset(dpat2);
            }

            //x scroll
            if(input[0] == 't')
                raxyz2[0] += SCROLL_DIV*strlen(input);
            else if(input[0] == '5')
                raxyz2[0] -= SCROLL_DIV*strlen(input);
            //y scroll
            else if(input[0] == 'y')
                raxyz2[1] += SCROLL_DIV*strlen(input);
            else if(input[0] == 'r')
                raxyz2[1] -= SCROLL_DIV*strlen(input);
            //z scroll
            else if(input[0] == '4')
                raxyz2[2] += SCROLL_DIV*strlen(input);
            else if(input[0] == '6')
                raxyz2[2] -= SCROLL_DIV*strlen(input);
            //depth
            if(input[0] == 'f')
                mvxyz2[2] += MOVE_DIV*strlen(input);
            else if(input[0] == 'h')
                mvxyz2[2] -= MOVE_DIV*strlen(input);
            //right/left
            else if(input[0] == 'n')
                mvxyz2[0] += MOVE_DIV*strlen(input);
            else if(input[0] == 'v')
                mvxyz2[0] -= MOVE_DIV*strlen(input);
            //up/down
            else if(input[0] == 'g')
                mvxyz2[1] += MOVE_DIV*strlen(input);
            else if(input[0] == 'b')
                mvxyz2[1] -= MOVE_DIV*strlen(input);

        }

        usleep(100000);
    }
}


