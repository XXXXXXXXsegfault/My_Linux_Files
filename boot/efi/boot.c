asm(".text");
asm(".globl _start");
asm("_start:");
asm("mov %rcx,%rdi");
asm("mov %rdx,%rsi");
asm("call _main");
asm("ret");
void *image_handle_this;
#include "efi.c"
#include "memset.c"
#include "memcpy.c"
#include "memcmp.c"
#include "palloc.c"
#include "font.c"
#include "graphics.c"
#include "fs.c"
#include "block.c"
#include "ext2_read.c"
struct file_image
{
	unsigned long long int size;
	char *image;
};
unsigned int *bg_resized;
struct file_image config;
struct file_image bg;
void debug_p_i(unsigned long long int a,int x,int y);
unsigned int bg_decode(unsigned int x,unsigned int y)
{
	unsigned short int *bginfo=(void *)bg.image;
	unsigned int *bg_addr=(void *)(bginfo+2);
	if(bg.image==0||(unsigned int)bginfo[0]*bginfo[1]*4+4!=bg.size||x>=sinfo[0]||y>=sinfo[1])
	{
		return 0;
	}
	x*=bginfo[0];
	y*=bginfo[1];
	int x1=x/sinfo[0],y1=y/sinfo[1];
	int x2=x%sinfo[0],y2=y%sinfo[1];
	unsigned int c[4];
	unsigned int off=y1*bginfo[0]+x1;
	int r[4],r1[2];
	int g[4],g1[2];
	int b[4],b1[2];
	int r2,g2,b2;
	unsigned int result;

	c[0]=bg_addr[off];
	if(x1+1<bginfo[0])
	{
		c[1]=bg_addr[off+1];
	}
	else
	{
		c[1]=0;
	}
	if(y1+1<bginfo[1])
	{
		c[2]=bg_addr[off+bginfo[0]];
		if(x1+1<bginfo[0])
		{
			c[3]=bg_addr[off+bginfo[0]+1];
		}
		else
		{
			c[3]=0;
		}
	}
	else
	{
		c[2]=0;
		c[3]=0;
	}
	x=0;
	while(x<4)
	{
		r[x]=c[x]>>16&0xff;
		g[x]=c[x]>>8&0xff;
		b[x]=c[x]&0xff;
		x++;
	}
	r1[0]=r[0]+(r[1]-r[0])*x2/(int)sinfo[0];
	r1[1]=r[2]+(r[3]-r[2])*x2/(int)sinfo[0];
	g1[0]=g[0]+(g[1]-g[0])*x2/(int)sinfo[0];
	g1[1]=g[2]+(g[3]-g[2])*x2/(int)sinfo[0];
	b1[0]=b[0]+(b[1]-b[0])*x2/(int)sinfo[0];
	b1[1]=b[2]+(b[3]-b[2])*x2/(int)sinfo[0];
	r2=r1[0]+(r1[1]-r1[0])*y2/(int)sinfo[1];
	g2=g1[0]+(g1[1]-g1[0])*y2/(int)sinfo[1];
	b2=b1[0]+(b1[1]-b1[0])*y2/(int)sinfo[1];
	result=r2&0xff;
	result=result<<8|g2&0xff;
	result=result<<8|b2&0xff;
	return result;
}
void bg_resize(void)
{
	int x,y=0;
	while(y<sinfo[1])
	{
		x=0;
		while(x<sinfo[0])
		{
			bg_resized[y*line_length/4+x]=bg_decode(x,y);
			x++;
		}
		y++;
	}
}
unsigned int bg_read(int x,int y)
{
	return bg_resized[y*line_length/4+x];
}
void p_char(char c,int x,int y,char m)
{
	int x1,y1=0;
	unsigned int color;
	if((c<33||c>127)&&!m)
	{
		return;
	}
	while(y1<16)
	{
		x1=0;
		while(x1<8)
		{
			color=bg_read(x+x1,y+y1);
			if(m)
			{
				if(c<33||c>127||~font_array[(c-33<<4)+y1]&1<<x1)
				{
					color^=0xffffff;
				}
			}
			else
			{
				if(font_array[(c-33<<4)+y1]&1<<x1)
				{
					color^=0xffffff;
				}
			}
			paint_pixel(x+x1,y+y1,color);
			x1++;
		}
		y1++;
	}
}
void p_str(char *str,int x,int y,char m)
{
	char c;
	while(c=*str++)
	{
		p_char(c,x,y,m);
		x+=8;
	}
}
void debug_p_i(unsigned long long int a,int x,int y)
{
	char *c="0123456789ABCDEF";
	p_char(c[a>>60&0xf],x,y,0);
	p_char(c[a>>56&0xf],x+8,y,0);
	p_char(c[a>>52&0xf],x+16,y,0);
	p_char(c[a>>48&0xf],x+24,y,0);
	p_char(c[a>>44&0xf],x+32,y,0);
	p_char(c[a>>40&0xf],x+40,y,0);
	p_char(c[a>>36&0xf],x+48,y,0);
	p_char(c[a>>32&0xf],x+56,y,0);
	p_char(c[a>>28&0xf],x+64,y,0);
	p_char(c[a>>24&0xf],x+72,y,0);
	p_char(c[a>>20&0xf],x+80,y,0);
	p_char(c[a>>16&0xf],x+88,y,0);
	p_char(c[a>>12&0xf],x+96,y,0);
	p_char(c[a>>8&0xf],x+104,y,0);
	p_char(c[a>>4&0xf],x+112,y,0);
	p_char(c[a&0xf],x+120,y,0);
}
void debug_p_mem_map(unsigned long long int addr)
{
	unsigned char *c=(void *)addr,c1;
	char *str="0123456789ABCDEF";
	int x=0;
	while(x<768)
	{
		c1=*c;
		p_char(str[c1>>4],x%16*24,x/16*16,0);
		p_char(str[c1&0xf],x%16*24+8,x/16*16,0);
		x++;
		c++;
	}
}
void paint_bg(void)
{
	memcpy(video_buf,bg_resized,line_length*sinfo[1]);
}
unsigned int config_read_line(unsigned int off,unsigned int *size)
{
	*size=0;
	if(off==config.size)
	{
		return 0;
	}
	while(off<config.size&&config.image[off]!='\n')
	{
		off++;
		(*size)++;
	}
	while(off<config.size&&config.image[off]=='\n')
	{
		off++;
	}
	return off;
}
char config_line_buf[2048];
unsigned int get_config_name(unsigned int n)
{
	unsigned int off=0,off1=0,l;
	while(1)
	{
		off=config_read_line(off,&l);
		if(off==0)
		{
			return 0;
		}
		if(l>2047)
		{
			l=2047;
		}
		if(l>6&&!memcmp(config.image+off1,"ENTRY ",6))
		{
			if(n)
			{
				n--;
			}
			else
			{
				memcpy(config_line_buf,config.image+off1+6,l-6);
				config_line_buf[l-6]=0;
				return off;
			}
		}
		off1=off;
	}
}
unsigned int config_s=0,config_st=0,config_n;
void p_all(void)
{
	unsigned int x=0;
	paint_bg();
	while(x<config_n)
	{
		if(get_config_name(x+config_st*config_n)==0)
		{
			p_str("Exit",0,x*16,config_s==x);
			break;
		}
		p_str(config_line_buf,0,x*16,config_s==x);
		x++;
	}
	video_display();
}
unsigned int getch(struct EFI_system_table *table)
{
	unsigned int code=0xffffffff;
	long long int status;
	eficall(table->conin->read_key,2,status,(i64)table->conin,(i64)&code);
	return code;
}
#include "linux_boot.c"
int parse_config(struct EFI_system_table *table,unsigned int off)
{
	char type=0;
	unsigned int off1=off,l,x;
	struct file_image kernel={0},initrd={0};
	static char config_buf2[2048];
	static char linux_cmdline[2048];
	unsigned long long int acpi_addr;
	char *str="0123456789abcdef";
	if(off==0)
	{
		return 1;
	}
	while(off=config_read_line(off,&l))
	{
		if(l>6&&!memcmp(config.image+off1,"ENTRY ",6))
		{
			break;
		}
		else if(l>6&&!memcmp(config.image+off1,"LINUX ",6))
		{
			if(!type)
			{
				l-=6;
				if(l>2047)
				{
					l=2047;
				}
				memcpy(config_buf2,config.image+off1+6,l);
				config_buf2[l]=0;
				if(ext2_parse_path(config_buf2))
				{
					if(kernel.image=ext2_load(table,&kernel.size))
					{
						type=1;
					}
				}
			}
			linux_cmdline[0]=0;
		}
		else if(l>8&&!memcmp(config.image+off1,"CMDLINE ",8))
		{
			acpi_addr=locate_acpi(table);
			if(type==1)
			{
				l-=8;
				if(l>2018)
				{
					l=2018;
				}
				memcpy(linux_cmdline,config.image+off1+8,l);
				if(acpi_addr)
				{
					memcpy(linux_cmdline+l," acpi_rsdp=0x0000000000000000",30);
					x=l+28;
					while(acpi_addr)
					{
						linux_cmdline[x]=str[acpi_addr&0xf];
						acpi_addr>>=4;
						x--;
					}
					l+=29;
				}
				linux_cmdline[l]=0;
			}
		}
		else if(l>7&&!memcmp(config.image+off1,"INITRD ",7))
		{
			if(type==1)
			{
				l-=7;
				if(l>2047)
				{
					l=2047;
				}
				memcpy(config_buf2,config.image+off1+7,l);
				config_buf2[l]=0;
				if(ext2_parse_path(config_buf2))
				{
					initrd.image=ext2_load(table,&initrd.size);
				}
			}
		}
		off1=off;
	}
	switch(type)
	{
		case 1:boot_linux(table,&kernel,&initrd,linux_cmdline);
		break;
	}
	if(kernel.image)
	{
		prelease(table,kernel.image,kernel.size+4095>>12);
	}
	if(initrd.image)
	{
		prelease(table,initrd.image,initrd.size+4095>>12);
	}
}
int _main(void *handle,struct EFI_system_table *table)
{
	unsigned int code;
	long long int status;
	image_handle_this=handle;
	eficall(table->boot_services->set_watchdog_timer,4,status,0,0,0,0);
	if(graphics_init(table)<0)
	{
		return -1;
	}
	if(fs_init(handle,table)<0)
	{
		return -1;
	}
	if(block_init(table)<0)
	{
		return -1;
	}
	if(ext2_init(table)<0)
	{
		return -1;
	}
	if(ext2_parse_path("/boot.conf")==0)
	{
		return -1;
	}
	if((config.image=ext2_load(table,&config.size))==0)
	{
		return -1;
	}
	if((bg_resized=palloc(table,line_length*sinfo[1]+4095>>12))==0)
	{
		return -1;
	}
	if(ext2_parse_path("/background.bin"))
	{
		bg.image=ext2_load(table,&bg.size);
	}
	asm("cli");
	bg_resize();
	asm("sti");
	config_n=sinfo[1]/16;

	p_all();
	while(1)
	{
		code=getch(table);
		switch(code&0xffff)
		{
			case 1:
			if(config_s)
			{
				config_s--;
				p_all();
			}
			else if(config_st)
			{
				config_s=config_n-1;
				config_st--;
				p_all();
			}
			break;
			case 2:
			if(get_config_name(config_s+config_st*config_n))
			{
				config_s++;
				if(config_s==config_n)
				{
					config_s=0;
					config_st++;
				}
				p_all();
			}
		}
		if(code>>16==13)
		{
			if(parse_config(table,get_config_name(config_s+config_st*config_n)))
			{
				return 0;
			}
		}
	}
}

