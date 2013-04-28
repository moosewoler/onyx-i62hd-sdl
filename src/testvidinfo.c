
/* Simple program -- figure out what kind of video display we have */

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/mxcfb.h>

#include "SDL.h"

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
        for (i=100; i<110; i++)
        {
            for (j=100; j<110; j++)
            {
                printf("4\n");
                DrawPixel(screen, i,j, 0xFF,0xFF,0xFF);
            }
        }
        epdc_update(0,0, screen_width, screen_height, WAVEFORM_MODE_A2, 1, EPDC_FLAG_FORCE_MONOCHROME);
    }
    printf("5\n");
}

int main(int argc, char *argv[])
{
	const SDL_VideoInfo *info;
	int i;
	SDL_Rect **modes;
	char driver[128];

    // 初始化SDL
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) 
    {
        fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

    // 获得video驱动名称
	if ( SDL_VideoDriverName(driver, sizeof(driver)) ) {
		printf("Video driver: %s\n", driver);
	}
    // 获取video子系统信息
	info = SDL_GetVideoInfo();
    printf( "Current display: %dx%d, %d bits-per-pixel\n", info->current_w, info->current_h, info->vfmt->BitsPerPixel);
	if ( info->vfmt->palette == NULL ) 
    {
		printf("	Red Mask = 0x%.8x\n", info->vfmt->Rmask);
		printf("	Green Mask = 0x%.8x\n", info->vfmt->Gmask);
		printf("	Blue Mask = 0x%.8x\n", info->vfmt->Bmask);
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
			printf("\t%dx%dx%d\n", modes[i]->w, modes[i]->h, info->vfmt->BitsPerPixel);
		}
	}

    // 是否有wm
	if ( info->wm_available ) 
    {
		printf("A window manager is available\n");
	}
    // 是否有硬件显存
	if ( info->hw_available ) 
    {
		printf("Hardware surfaces are available (%dK video memory)\n",
			info->video_mem);
	}
    // 是否有硬件blit
	if ( info->blit_hw ) 
    {
		printf( "Copy blits between hardware surfaces are accelerated\n");
	}
    // 是否有硬件Colorkey blit
	if ( info->blit_hw_CC ) {
		printf( "Colorkey blits between hardware surfaces are accelerated\n");
	}
    // 是否有硬件alpha blit
	if ( info->blit_hw_A ) 
    {
		printf( "Alpha blits between hardware surfaces are accelerated\n");
	}
	if ( info->blit_sw ) 
    {
		printf( "Copy blits from software surfaces to hardware surfaces are accelerated\n");
	}
	if ( info->blit_sw_CC ) 
    {
		printf( "Colorkey blits from software surfaces to hardware surfaces are accelerated\n");
	}
	if ( info->blit_sw_A ) 
    {
		printf( "Alpha blits from software surfaces to hardware surfaces are accelerated\n");
	}
	if ( info->blit_fill ) 
    {
		printf( "Color fills on hardware surfaces are accelerated\n");
	}


    printf("1\n");
    RunVideoTests();
    printf("9\n");

    // 释放SDL
	SDL_Quit();

	return(0);
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
