/*******************************************************************************
 *          test-sdl.c
 * Description:
 *  this file tests some basic sdl video operations on onyx i62hd e-ink device.
 * 
 *  use SDL to deal with image buffers, use mxcfb to deal with epdc operations.
 * History:
 *  2013-05-20 NEW
 ******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// headers for using mxc-epdc
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/mxcfb.h>

// headers for using SDL
#include "SDL.h"
#include "SDL_image.h"

#define MWO_DEBUG_STRING(var)    do { printf(#var" = %s\n", var); } while(0)
#define MWO_DEBUG_FLOAT(var)     do { printf(#var" = %f\n", var); } while(0)
#define MWO_DEBUG_POINTER(var)   do { printf(#var" = %p\n", var); } while(0)
#define MWO_DEBUG_INT(var)       do { printf(#var" = %d\n", var); } while(0)
#define MWO_DEBUG(var)           MWO_DEBUG_INT(var)

void init_sdl(void);
void quit_sdl(void);
void test_videoinfo(void);
void test_drawpixel(void);
void test_fillrectangle(void);
int  test_drawimage(void);
void epdc_update(int left, int top, int width, int height, int waveform, int wait_for_complete, uint flags);
void ditherize(SDL_Surface* surface, SDL_Rect* rect);

struct _TMyScreen{
    SDL_Surface*    surface;
    const SDL_VideoInfo*  info;
    char            driver_name[128];
};
struct _TMyScreen main_screen;

#define NUM_BLITS	10
#define NUM_UPDATES	500
#define FLAG_MASK	(SDL_HWSURFACE | SDL_FULLSCREEN | SDL_DOUBLEBUF | \
                         SDL_SRCCOLORKEY | SDL_SRCALPHA | SDL_RLEACCEL  | \
                         SDL_RLEACCELOK)

static int screen_width  = 0;
static int screen_height = 0;
static int screen_bpp    = 16;
static int screen_flags  = 0;

#define EPDC_STR_ID		"mxc_epdc_fb"

#define WAVEFORM_MODE_INIT	0x0	/* Screen goes to white (clears) */
#define WAVEFORM_MODE_DU	0x1	/* Grey->white/grey->black */
#define WAVEFORM_MODE_GC16	0x2	/* High fidelity (flashing) */
#define WAVEFORM_MODE_GC4	0x3	/* Lower fidelity */
#define WAVEFORM_MODE_A2	0x4	/* Fast black/white animation */
void epdc_update(int left, int top, int width, int height, int waveform, int wait_for_complete, unsigned int flags);



void PrintFlags(Uint32 flags)
{
	printf("0x%8.8x", (flags & FLAG_MASK));
	if ( flags & SDL_HWSURFACE ) {
		printf(" SDL_HWSURFACE");
	} else {
		printf(" SDL_SWSURFACE");
	}
	if ( flags & SDL_FULLSCREEN ) {
		printf(" | SDL_FULLSCREEN");
	}
	if ( flags & SDL_DOUBLEBUF ) {
		printf(" | SDL_DOUBLEBUF");
	}
	if ( flags & SDL_SRCCOLORKEY ) {
		printf(" | SDL_SRCCOLORKEY");
	}
	if ( flags & SDL_SRCALPHA ) {
		printf(" | SDL_SRCALPHA");
	}
	if ( flags & SDL_RLEACCEL ) {
		printf(" | SDL_RLEACCEL");
	}
	if ( flags & SDL_RLEACCELOK ) {
		printf(" | SDL_RLEACCELOK");
	}
}

void DrawPixel(SDL_Surface *screen, Sint32 x, Sint32 y, Uint8 R, Uint8 G, Uint8 B)
{
    Uint32 color = SDL_MapRGB(screen->format, R, G, B);

    if ( SDL_MUSTLOCK(screen) ) 
    {
        if ( SDL_LockSurface(screen) < 0 ) 
        {
            return;
        }
    }
    switch (screen->format->BytesPerPixel) 
    {
        case 1: 
            { /* 假定是8-bpp */
                Uint8 *bufp;

                bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
                *bufp = color;
            }
            break;

        case 2: 
            { /* 可能是15-bpp 或者 16-bpp */
                Uint16 *bufp;

                bufp = (Uint16 *)screen->pixels + y*screen->pitch/2 + x;
                *bufp = color;
            }
            break;

        case 3: 
            { /* 慢速的24-bpp模式，通常不用 */
                Uint8 *bufp;

                bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
                *(bufp+screen->format->Rshift/8) = R;
                *(bufp+screen->format->Gshift/8) = G;
                *(bufp+screen->format->Bshift/8) = B;
            }
            break;

        case 4: 
            { /* 可能是32-bpp */
                Uint32 *bufp;

                bufp = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
                *bufp = color;
            }
            break;
    }
    if ( SDL_MUSTLOCK(screen) ) 
    {
        SDL_UnlockSurface(screen);
    }
    SDL_UpdateRect(screen, x, y, 1, 1);
}

void RunVideoTests()
{
	SDL_Surface *screen;
    int i,j;
    int re;

    screen_width = 758;
    screen_height= 1024;

    printf("2\n");
    screen = SDL_SetVideoMode(screen_width, screen_height, 16, SDL_FULLSCREEN);
    printf("3\n");
    if ( ! screen ) 
    {
        printf("Setting video mode failed: %s\n", SDL_GetError());
    }
    else
    {
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0xff, 0xff, 0xff));
        SDL_UpdateRect(screen, 0, 0, 0, 0);
        epdc_update(0,0, screen_width, screen_height, WAVEFORM_MODE_GC16, 1, 0);

        for (i=100; i<200; i++)
        {
            for (j=100; j<200; j++)
            {
                printf("4\n");
                DrawPixel(screen, i,j, 0,0,0);
            }
        }
        
        epdc_update(0,0, screen_width, screen_height, WAVEFORM_MODE_A2, 1, EPDC_FLAG_FORCE_MONOCHROME);
    }


    printf("5\n");
}

/*******************************************************************************
 *          main()
 ******************************************************************************/
int main(int argc, char *argv[])
{
    int re;
    // 系统初始化
    setenv("SDL_NOMOUSE", "1", 1);
    setenv("SDL_VIDEO_FBCON_ROTATION", "UD", 1);

    init_sdl();

    test_videoinfo();
    test_drawpixel();
    re = test_drawimage();

    quit_sdl();

    return(0);
}

void init_sdl(void)
{
    if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) 
    {
        fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(-1);
    }

    // 获取video子系统信息
    main_screen.info = SDL_GetVideoInfo();

    main_screen.surface = SDL_SetVideoMode(main_screen.info->current_w, 
            main_screen.info->current_h, 
            main_screen.info->vfmt->BitsPerPixel, 
            SDL_FULLSCREEN);

    if ( ! main_screen.surface) 
    {
        printf("Setting video mode failed: %s\n", SDL_GetError());
        exit(-1);
    }
}

void quit_sdl(void)
{
    // 释放SDL
    SDL_Quit();
}

void test_videoinfo(void)
{
    int i;
    SDL_Rect **modes;

    // 获得video驱动名称
    if ( SDL_VideoDriverName(main_screen.driver_name, sizeof(main_screen.driver_name)) ) {
        printf("Video driver: %s\n", main_screen.driver_name);
    }

    printf( "Current display: %dx%d, %d bits-per-pixel\n", main_screen.info->current_w, main_screen.info->current_h, main_screen.info->vfmt->BitsPerPixel);
    if ( main_screen.info->vfmt->palette == NULL ) 
    {
        printf("	Red Mask = 0x%.8x\n",   main_screen.info->vfmt->Rmask);
        printf("	Green Mask = 0x%.8x\n", main_screen.info->vfmt->Gmask);
        printf("	Blue Mask = 0x%.8x\n",  main_screen.info->vfmt->Bmask);
    }

    // 获取可用的全屏模式
    modes = SDL_ListModes(NULL, SDL_FULLSCREEN);
    if ( modes == (SDL_Rect **)0 ) 
    {
        printf("No available fullscreen video modes\n");
    } 
    else if ( modes == (SDL_Rect **)-1 ) 
    {
        printf("No special fullscreen video modes\n");
    } else 
    {
        printf("Fullscreen video modes:\n");
        for ( i=0; modes[i]; ++i ) 
        {
            printf("\t%dx%dx%d\n", modes[i]->w, modes[i]->h, main_screen.info->vfmt->BitsPerPixel);
        }
    }

    // 是否有硬件显存
    if ( main_screen.info->hw_available ) 
    {
        printf("Hardware surfaces are available (%dK video memory)\n", main_screen.info->video_mem);
    }
    else
    {
        printf("Hardware surfaces are not available.\n");
    }

    // 是否有硬件blit
    if ( main_screen.info->blit_hw ) 
    {
        printf( "Copy blits between hardware surfaces are accelerated\n");
    }

    // 是否有硬件Colorkey blit
    if ( main_screen.info->blit_hw_CC ) {
        printf( "Colorkey blits between hardware surfaces are accelerated\n");
    }

    // 是否有硬件alpha blit
    if ( main_screen.info->blit_hw_A ) 
    {
        printf( "Alpha blits between hardware surfaces are accelerated\n");
    }


    if ( main_screen.info->blit_sw ) 
    {
        printf( "Copy blits from software surfaces to hardware surfaces are accelerated\n");
    }
    if ( main_screen.info->blit_sw_CC ) 
    {
        printf( "Colorkey blits from software surfaces to hardware surfaces are accelerated\n");
    }
    if ( main_screen.info->blit_sw_A ) 
    {
        printf( "Alpha blits from software surfaces to hardware surfaces are accelerated\n");
    }
    if ( main_screen.info->blit_fill ) 
    {
        printf( "Color fills on hardware surfaces are accelerated\n");
    }
}

void test_drawpixel(void)
{
    int i,j;
    // 清屏
    SDL_FillRect(main_screen.surface, NULL, SDL_MapRGB(main_screen.surface->format, 0xff, 0xff, 0xff));
    SDL_UpdateRect(main_screen.surface, 0, 0, 0, 0);
    epdc_update(0,0, main_screen.info->current_w, main_screen.info->current_h, WAVEFORM_MODE_GC16, 1, 0);

    for (i=0; i<main_screen.info->current_w; i+=10)
    {
        for (j=0; j<main_screen.info->current_h/2; j+=10)
        {
            DrawPixel(main_screen.surface, i,j, 0,0,0);
        }
    }
    epdc_update(0,0, main_screen.info->current_w, main_screen.info->current_h, WAVEFORM_MODE_GC16, 1, EPDC_FLAG_FORCE_MONOCHROME);
}
void test_fillrectangle(void)
{
}
int test_drawimage(void)
{
    SDL_Rect rect1= { 100, 100, 0, 0 };
    SDL_Rect rect2= { 100, 500, 0, 0 };
    SDL_Surface *image1;
    SDL_Surface *image2;

    image1 = IMG_Load("sdl_logo.png");
    if ( !image1 )
    {
        printf ( "IMG_Load: %s\n", IMG_GetError () );
        return 1;
    }
    image2 = IMG_Load("onyx_logo.png");
    if ( !image2 )
    {
        printf ( "IMG_Load: %s\n", IMG_GetError () );
        return 1;
    }

    // Draws the image on the screen
    SDL_BlitSurface( image1, NULL, main_screen.surface, &rect1 );
    SDL_BlitSurface( image2, NULL, main_screen.surface, &rect2 );
    SDL_UpdateRect(main_screen.surface, 0, 0, 0, 0);
    epdc_update(0,0, main_screen.info->current_w, main_screen.info->current_h, WAVEFORM_MODE_GC16, 1, 0);

    // Draw ditherized image
    {
        SDL_Rect drect1 = {0,0, image1->w, image1->h};
        SDL_Rect drect2 = {0,0, image2->w, image2->h};
        SDL_Rect rect3= { 600, 200, 0, 0 };
        SDL_Rect rect4= { 0, 0, 0, 0 };
        ditherize(image1, &drect1);
        ditherize(image2, &drect2);
        SDL_BlitSurface( image1, NULL, main_screen.surface, &rect3 );
        SDL_BlitSurface( image2, NULL, main_screen.surface, &rect4 );

    }
    SDL_UpdateRect(main_screen.surface, 0, 0, 0, 0);
    epdc_update(0,0, main_screen.info->current_w, main_screen.info->current_h, WAVEFORM_MODE_A2, 1, EPDC_FLAG_FORCE_MONOCHROME);

    // move image
    {
        int i;
        SDL_Rect rect3= { 0, 0, 0, 0 };
        for (i=0;i<200;i+=5)
        {
            // 清屏
            SDL_FillRect(main_screen.surface, &rect3, SDL_MapRGB(main_screen.surface->format, 0xff, 0xff, 0xff));
            SDL_UpdateRect(main_screen.surface, 0, 0, 0, 0);
            rect3.x=i;
            SDL_BlitSurface( image2, NULL, main_screen.surface, &rect3 );
            SDL_UpdateRect(main_screen.surface, 0, 0, 0, 0);
            epdc_update(0,0, main_screen.info->current_w, main_screen.info->current_h, WAVEFORM_MODE_A2, 1, EPDC_FLAG_FORCE_MONOCHROME);
        }
    }
    epdc_update(0,0, main_screen.info->current_w, main_screen.info->current_h, WAVEFORM_MODE_GC16, 1, 0);

    SDL_FreeSurface ( image1 );
    SDL_FreeSurface ( image2 );
    return 0;
}


void epdc_update(int left, int top, int width, int height, int waveform, int wait_for_complete, uint flags)
{
    // 见2013-02-22笔记1号
    struct mxcfb_update_data upd_data;
    __u32   upd_marker_data;
    int     retval;
    int     wait = wait_for_complete | flags;
    int     max_retry = 10;

    // FIXME: 计算出合适的边界值
    //int tleft= -(left+width)+screen_info_.xres;
    //int ttop = -(top+height-1)+screen_info_.yres;
    int tleft= left;
    int ttop = top;

    int fbcon_fd = -1;
    fbcon_fd = open("/dev/fb0", O_RDWR, 0);
    if (fbcon_fd < 0)
    {
        printf("cannot open fbdev\n");
    }
    else
    {
        upd_data.update_mode = UPDATE_MODE_PARTIAL;
        upd_data.waveform_mode = waveform;
        upd_data.update_region.left = tleft;
        upd_data.update_region.width = width;
        upd_data.update_region.top = ttop;
        upd_data.update_region.height = height;
        upd_data.temp = TEMP_USE_AMBIENT;
        upd_data.flags = flags;

        if (wait)
        {
            /* Get unique marker value */
            //upd_data.update_marker = marker_val_++;
        }
        else
        {
            upd_data.update_marker = 0;
        }

        retval = ioctl(fbcon_fd, MXCFB_SEND_UPDATE, &upd_data);
        while (retval < 0) 
        {
            /* We have limited memory available for updates, so wait and
             * then try again after some updates have completed */
            sleep(1);
            retval = ioctl(fbcon_fd, MXCFB_SEND_UPDATE, &upd_data);
            if (--max_retry <= 0) 
            {
                wait = 0;
                flags = 0;
                break;
            }
        }

        // geekmaster said that we should not always wait for operation finished.
        if (wait) 
        {
            upd_marker_data = upd_data.update_marker;

            /* Wait for update to complete */
            retval = ioctl(fbcon_fd, MXCFB_WAIT_FOR_UPDATE_COMPLETE, &upd_marker_data);
            if (retval < 0) 
            {
                flags = 0;
            }
        }
        close(fbcon_fd);
    }
}

/*******************************************************************************
 * function :   ditherize
 * memo     :   Dither a surface by using ordered dither.  
 * para     :   
 *  [in] surface             
 *  [in] rect
 *  x,y: screen coordinates, c: color(0-64).
 ******************************************************************************/
void ditherize(SDL_Surface* surface, SDL_Rect* rect)
{
    Sint32 x,y;
    // 64 threshold dither table
    // see http://en.wikipedia.org/wiki/Ordered_dithering
    static int dither_map_64[64] = { 
        1, 33,9, 41,3, 35,11,43,
        49,17,57,25,51,19,59,27,
        13,45,5, 37,15,47,7, 39,
        61,29,53,21,63,31,55,23,
        4, 36,12,44,2, 34,10,42,
        52,20,60,28,50,18,58,26,
        16,48,8, 40,14,46,6, 38,
        64,32,56,24,62,30,54,22 }; 

    if ( SDL_MUSTLOCK(surface) ) 
    {
        if ( SDL_LockSurface(surface) < 0 ) 
        {
            return;
        }
    }

    switch (surface->format->BytesPerPixel) 
    {
        case 3: 
            {
                // 24bpp, r8g8b8。 慢速像素操作
                for (y= rect->y; y<rect->y+rect->h; y++)
                {
                    for (x= rect->x; x<rect->x+rect->w; x++)
                    {
                        Uint8* pptr;
                        Uint8  R,G,B;
                        Uint8  color;
                        pptr = (Uint8 *)surface->pixels + y*surface->pitch + x*3;
                        R = *(pptr+surface->format->Rshift/8);
                        G = *(pptr+surface->format->Gshift/8);
                        B = *(pptr+surface->format->Bshift/8);

                        color= 64*( 0.2126*R/256 + 0.7152*G/256 + 0.0722*B/256);// remap rgb into 64-level grayscale. see http://en.wikipedia.org/wiki/Grayscale

                        if (color > dither_map_64[x&7, y&7])
                        {
                            *(pptr+surface->format->Rshift/8) = 0xFF;
                            *(pptr+surface->format->Gshift/8) = 0xFF;
                            *(pptr+surface->format->Bshift/8) = 0xFF;
                        }
                        else
                        {
                            *(pptr+surface->format->Rshift/8) = 0;
                            *(pptr+surface->format->Gshift/8) = 0;
                            *(pptr+surface->format->Bshift/8) = 0;
                        }

                    }
                }
            }
            break;
        case 2: 
            { 
                // 目前只支持16bpp. i62hd用16bpp，r5g6b5
                for (x= rect->x; x<rect->x+rect->w; x++)
                {
                    for (y= rect->y; y<rect->y+rect->h; y++)
                    {
                        Uint16 *pptr;
                        Uint16 color;
                        Uint16  R,G,B;

                        pptr = (Uint16 *)surface->pixels + y*surface->pitch/2 + x;
                        color= *pptr;
                        R = color & 0xF800;
                        G = color & 0x07E0;
                        B = color & 0x001F;
                        color= 64*( 0.2126*R + 0.7152*G + 0.0722*B);            // remap rgb into 64-level grayscale. see http://en.wikipedia.org/wiki/Grayscale

                        if (color > dither_map_64[x&7, y&7])
                        {
                            color = 0xFFFF;
                        }
                        else
                        {
                            color = 0;
                        }
                        *pptr = color;
                    }
                }

            }
            break;
        default:
            break;
    }

    if ( SDL_MUSTLOCK(surface) ) 
    {
        SDL_UnlockSurface(surface);
    }
}
