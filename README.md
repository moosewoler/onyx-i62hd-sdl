onyx-i62hd-sdl
==============

port sdl to i62hd

0 程序目的
----------

测试sdl在onyx i62hd的移植是否成功。需要测试的项目有：

    1. 获取系统信息
    2. 基本sdl video功能，画基本图素、填充、blit
    3. 旋转
    4. SDL\_image


1 关于键盘和鼠标
-----------------

运行测试程序，会出现：

        could not initialize SDL! Unable to open a console terminal

由于SDL在初始化Video的同时需要初始化键盘和鼠标。鼠标可以通过设置SDL\_NOMOUSE=1来屏蔽。SDL通过打开tty符号文件，并获得相关的vt来访问键盘。不知道是何原因/dev下的几个tty都无法获得合法的vt，目前通过注释掉~/src/video/fbcon/SDL\_fbvideo.c中关于FB\_OpenKeyboard()相关语句来屏蔽键盘。

2 SDL\_SetVideoMode(width, height, bpp, flags)
-----------------------------------------------

问题：设置视频模式为758x1024x16，无法返回合法的SDL\_Surface。

原因：由于需要使用vt，实际上并没有初始化vt。目前将相关函数注释掉(FB\_EnterGraphicMode)。现在snapshot出的内容是正确的。

函数名：    SDL\_SetVideoMode()

位置：      ~/src/video/SDL\_video.c

描述：      设置视频模式，如果需要的话分配一片影子缓冲。

该函数中有几个可能返回NULL的位置：

    1. 如果current\_video没有初始化的话，会重新初始化video子系统，如果不成功的话会返回NULL
    2. 期望设置的模式可能不被系统支持，所以在设置之前要调用SDL\_GetVideoMode()获得合理的设置，如果无法获得的话，会返回NULL。目前看来是这里的问题。
    3. rcg11292000，返回NULL。
    4. 如果无论如何也无法设置mode，返回NULL。
    5. 如果使用OPENGL的话，在载入GL函数的时候，失败的话也会返回NULL。
    6. 如果使用OPENGL的话，获得上下文失败，会返回NULL。
    7. 如果使用OPENGL的话，创建主表面失败，会返回NULL。
    8. 如果使用OPENGL的话，创建影子表面失败，会返回NULL。

3 输出变量
-----------

        #define MWO_DEBUG_STRING(var)    do { printf(#var" = %s\n", var); } while(0)
        #define MWO_DEBUG_FLOAT(var)     do { printf(#var" = %f\n", var); } while(0)
        #define MWO_DEBUG_POINTER(var)   do { printf(#var" = %p\n", var); } while(0)
        #define MWO_DEBUG_INT(var)       do { printf(#var" = %d\n", var); } while(0)
        #define MWO_DEBUG(var)           MWO_ECHO_INT(var)

