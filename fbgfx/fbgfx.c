#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/vt.h>

struct fb_fix_screeninfo finfo;
struct fb_var_screeninfo vinfo;

long screensize;
uint8_t *fdp;

uint32_t pixel_color(uint8_t r,uint8_t g,uint8_t b){
	return (r<<vinfo.red.offset)|(g<<vinfo.green.offset)|(b<<vinfo.blue.offset);
}

void draw_pixel(int x,int y,uint32_t pixel){
	long location = (x+vinfo.xoffset)*(vinfo.bits_per_pixel/8)+(y+vinfo.yoffset)*finfo.line_length;
	*((uint32_t*)(fdp+location)) = pixel;
}

int main(int argc,char *argv[]){
	int tty_fd = open("/dev/tty0",O_RDWR);
	ioctl(tty_fd,KDSETMODE,KD_GRAPHICS);

	int fb_fd = open("/dev/fb0",O_RDWR);

	ioctl(fb_fd,FBIOGET_FSCREENINFO,&finfo);
	ioctl(fb_fd,FBIOGET_VSCREENINFO,&vinfo);
	vinfo.grayscale = 0;
	vinfo.bits_per_pixel = 32;
	ioctl(fb_fd,FBIOPUT_VSCREENINFO,&vinfo);
	ioctl(fb_fd,FBIOGET_VSCREENINFO,&vinfo);

	screensize = vinfo.yres_virtual*finfo.line_length;
	fdp = mmap(0,screensize,PROT_READ|PROT_WRITE,MAP_SHARED,fb_fd,(off_t)0);

	for (int x=0;x<vinfo.xres;x++)
		for (int y=0;y<vinfo.yres;y++)
			draw_pixel(x,y,pixel_color(0xFF,0x00,0xFF));

	sleep(5);
	close(fb_fd);
	ioctl(tty_fd,KDSETMODE,KD_TEXT);
	close(tty_fd);
	return 0;
}
