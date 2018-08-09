/*  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  
 *  based on `libXNVCtrl/samples/nv-control-info.c`
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <X11/Xlib.h>

#include "NVCtrl/NVCtrl.h"
#include "NVCtrl/NVCtrlLib.h"

typedef struct {
    int major;
    int minor;
}nvctrl_xext_t;

typedef struct nvctrl_screen_info_t {
    struct nvctrl_screen_info_t* next;
    int   id;
    char* product_name;
    char* vbios_version;
    char* driver_version;
}nvctrl_screen_info_t;

typedef struct {
    Display*              dpy;
    nvctrl_xext_t         xext_ver;
    nvctrl_screen_info_t* screen_info;
}nvctrl_handle_t;


int nvctrl_get_screen_info(nvctrl_handle_t* h)
{
    nvctrl_screen_info_t* head = NULL;
    nvctrl_screen_info_t* p = NULL;
    int nr_screens = 0;
    int nr_nv_screens = 0;
    int i;
    int ret;
    char* str;

    nr_screens = ScreenCount(h->dpy);
    if (nr_screens <= 0) {
        return 0;
    }

    head = (nvctrl_screen_info_t*)calloc(nr_screens, sizeof(nvctrl_screen_info_t));
    memset(head, 0, sizeof(nvctrl_screen_info_t) * nr_screens);
    p = head;

    for (i = 0; i < nr_screens; i++) {
        if (!XNVCTRLIsNvScreen(h->dpy, i)) {
            continue;
        }

        if (p != head) {
            // because info is array + link.
            // so we just ++
            p->next = p+1;
            p= p->next;
        }
        p->next = NULL;

        p->id = i;
        nr_nv_screens++;

        ret = XNVCTRLQueryStringAttribute(h->dpy, i,
                0, /* XXX not curently used */
                NV_CTRL_STRING_PRODUCT_NAME,
                &str);
        if (ret) {
            p->product_name = strdup(str);
            XFree(str);
        }
            

        ret = XNVCTRLQueryStringAttribute(h->dpy, i,
                0, /* XXX not curently used */
                NV_CTRL_STRING_VBIOS_VERSION,
                &str);
        if (ret) {
            printf("  VideoBIOS      : %s\n", str);
            p->vbios_version = strdup(str);
            XFree(str);
        }


        ret = XNVCTRLQueryStringAttribute(h->dpy, i,
                0, /* XXX not curently used */
                NV_CTRL_STRING_NVIDIA_DRIVER_VERSION,
                &str);
        if (ret) {
            printf("  Driver version : %s\n", str);
            p->driver_version = strdup(str);
            XFree(str);
        }
    }

    if (nr_nv_screens == 0) {
        free(head);
        return 0;
    }

    h->screen_info = head;
    return nr_nv_screens;
}


nvctrl_handle_t* nvctrl_open(void)
{
    Display* dpy = NULL;
    nvctrl_handle_t* h = NULL;
    Bool ret;
    int event_base;
    int error_base;
    int major;
    int minor;

    /*
     * open a connection to the X server indicated by the DISPLAY
     * environment variable
     */

    dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "Cannot open display '%s'.\n", XDisplayName(NULL));
        return NULL;
    }
    
    /*
     * check if the NV-CONTROL X extension is present on this X server
     */

    ret = XNVCTRLQueryExtension(dpy, &event_base, &error_base);
    if (ret != True) {
        fprintf(stderr, "The NV-CONTROL X extension does not exist on '%s'.\n",
                XDisplayName(NULL));
        goto L_ERROR;
    }

    /*
     * query the major and minor extension version
     */

    ret = XNVCTRLQueryVersion(dpy, &major, &minor);
    if (ret != True) {
        fprintf(stderr, "The NV-CONTROL X extension does not exist on '%s'.\n",
                XDisplayName(NULL));
        goto L_ERROR;
    }

    h = (nvctrl_handle_t*)malloc(sizeof(nvctrl_handle_t));
    memset(h, 0, sizeof(nvctrl_handle_t));
    h->dpy = dpy;
    h->xext_ver.major = major;
    h->xext_ver.minor = minor;

    nvctrl_get_screen_info(h);

    return h;

L_ERROR:
    if (dpy != NULL) {
        XCloseDisplay(dpy);
    }

    return NULL;
}

void nvctrl_close(nvctrl_handle_t* h)
{
    nvctrl_screen_info_t* p;

    if (h == NULL) {
        return;
    }

    if (h->screen_info != NULL) {
        for (p = h->screen_info;p != NULL;p = p->next) {
            if (p->product_name != NULL) {
                free(p->product_name);
            }
            if (p->vbios_version != NULL) {
                free(p->vbios_version);
            }
            if (p->driver_version != NULL) {
                free(p->driver_version);
            }
        }
        free(h->screen_info);
    }

    if (h->dpy != NULL) {
        XCloseDisplay(h->dpy);
    }
    free(h);
}

int main(int argc, char* argv[])
{
    Bool ret;
    int screens;
    int i;
    char *str;
    int val;

    nvctrl_handle_t* h = NULL;
    const nvctrl_screen_info_t* screen = NULL;

    printf("Display%s\n", XDisplayName(NULL));

    h = nvctrl_open();
    if (h == NULL) {
        return -1;
    }
    
    printf("  NV-CONTROL X extension present\n");
    printf("  version        : %d.%d\n", h->xext_ver.major, h->xext_ver.minor);
    printf("\n");

    for (screen = h->screen_info;screen != NULL;screen = screen->next) {
        printf("  Screen:%d\n", screen->id);
        if (screen->product_name) {
            printf("    product name  : %s\n", screen->product_name);
        }
        if (screen->vbios_version) {
            printf("    vbios version : %s\n", screen->vbios_version);
        }
        if (screen->product_name) {
            printf("    driver version: %s\n", screen->driver_version);
        }

        if (XNVCTRLQueryTargetAttribute(h->dpy,
                    NV_CTRL_TARGET_TYPE_X_SCREEN,
                    i, 
                    0, /* XXX not curently used */
                    NV_CTRL_TOTAL_GPU_MEMORY,
                    &val))
        {
            printf("    gpu total memory: ");
            if (val / 1024 / 1024 / 1024 > 0) {
                printf("%.3f Tb\n", 1.0 * val / 1024 / 1024 / 1024);
            } else if (val / 1024 / 1024 > 0) {
                printf("%.3f Gb\n", 1.0 * val / 1024 / 1024);
            } else if (val / 1024 > 0) {
                printf("%.3f Mb\n", 1.0 * val / 1024);
            } else {
                printf("%d Kb\n", val);
            }
        }

        if (XNVCTRLQueryTargetStringAttribute(h->dpy,
                    NV_CTRL_TARGET_TYPE_GPU,
                    i, 
                    0, /* XXX not curently used */
                    NV_CTRL_STRING_GPU_UTILIZATION,
                    &str))
        {
            printf("    gpu utilization (0-100): %s\n", str);
            XFree(str);
        }



    }
    
    nvctrl_close(h);
    h = NULL;

    return 0;
}

