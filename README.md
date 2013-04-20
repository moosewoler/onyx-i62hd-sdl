onyx-i62hd-sdl
==============

port sdl to i62hd

1. 关于键盘和鼠标
-----------------

运行测试程序，会出现：
        could not initialize SDL! Unable to open a console terminal

由于SDL在初始化Video的同时需要初始化键盘和鼠标。鼠标可以通过设置SDL\_NOMOUSE=1来屏蔽。SDL通过打开tty符号文件，并获得相关的vt来访问键盘。不知道是何原因/dev下的几个tty都无法获得合法的vt，目前通过注释掉~/src/video/fbcon/SDL\_fbvideo.c中关于FB\_OpenKeyboard()相关语句来屏蔽键盘。

