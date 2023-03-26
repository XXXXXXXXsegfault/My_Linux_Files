struct graphics_mode_info
{
	unsigned int version;
	unsigned int hres;
	unsigned int vres;
	unsigned int pixel_format;
	unsigned int red;
	unsigned int green;
	unsigned int blue;
	unsigned int reserved;
	unsigned int line_pixels;
};
struct graphics_output_mode
{
	unsigned int maxmode;
	unsigned int mode;
	struct graphics_mode_info *info;
	unsigned long long int info_size;
	void *addr;
	unsigned long long int size;
};
struct graphics_output_protocol
{
	void *query_mode;
	void *set_mode;
	void *blt;
	struct graphics_output_mode *mode;
};
unsigned char *video_mem;
unsigned long long int video_buf_size;
struct graphics_output_mode *graphics_current_mode;
unsigned char *video_buf;
unsigned short int sinfo[2],line_length;
unsigned char c_info[8];
int lowest_bit(unsigned int a)
{
	int x=0;
	while(x<32)
	{
		if(a&1<<x)
		{
			return x;
		}
		x++;
	}
	return 0;
}
int highest_bit(unsigned int a)
{
	int x=32;
	while(x)
	{
		x--;
		if(a&1<<x)
		{
			return x+1;
		}
	}
	return 0;
}
int graphics_init(struct EFI_system_table *table)
{
	unsigned int guid[4]={0x9042a9de,0x4a3823dc,0xde7afb96,0x6a5180d0};
	struct graphics_output_protocol *gop;
	struct graphics_mode_info *ginfo,*bestmode_info=0;
	long long int status,nmode,x,bestmode=0;
	unsigned long long int info_size=0;
	eficall(table->boot_services->locate_prot,3,status,(i64)guid,0,(i64)&gop);
	if(status<0)
	{
		return -1;
	}
	eficall(gop->set_mode,2,status,(i64)gop,0);
	if(status<0)
	{
		return -1;
	}
	nmode=gop->mode->maxmode;
	x=0;
	while(x<nmode)
	{
		eficall(gop->query_mode,4,status,(i64)gop,x,(i64)&info_size,(i64)&ginfo);
		if(status>=0)
		{
			if(ginfo->hres<=1024&&ginfo->vres<=768&&(!bestmode_info||ginfo->hres>=bestmode_info->hres&&ginfo->vres>=bestmode_info->vres))
			{
				bestmode_info=ginfo;
				bestmode=x;
			}
		}
		x++;
	}
	eficall(gop->set_mode,2,status,(i64)gop,bestmode);
	if(status<0)
	{
		return -1;
	}
	graphics_current_mode=gop->mode;
	video_mem=graphics_current_mode->addr;
	video_buf_size=graphics_current_mode->info->vres*graphics_current_mode->info->line_pixels*4;
	video_buf=palloc(table,video_buf_size+4095>>12);
	if(video_buf==0)
	{
		return -1;
	}
	sinfo[0]=graphics_current_mode->info->hres;
	sinfo[1]=graphics_current_mode->info->vres;
	line_length=4*graphics_current_mode->info->line_pixels;
	switch(graphics_current_mode->info->pixel_format)
	{
		case 0:c_info[1]=0;
		c_info[0]=8;
		c_info[3]=8;
		c_info[2]=8;
		c_info[5]=16;
		c_info[4]=8;
		c_info[6]=8;
		c_info[7]=24;
		break;
		case 1:c_info[1]=16;
		c_info[0]=8;
		c_info[3]=8;
		c_info[2]=8;
		c_info[5]=0;
		c_info[4]=8;
		c_info[6]=8;
		c_info[7]=24;
		break;
		case 2:c_info[1]=lowest_bit(graphics_current_mode->info->red);
		c_info[0]=highest_bit(graphics_current_mode->info->red)-c_info[1];
		c_info[3]=lowest_bit(graphics_current_mode->info->green);
		c_info[2]=highest_bit(graphics_current_mode->info->green)-c_info[3];
		c_info[5]=lowest_bit(graphics_current_mode->info->blue);
		c_info[4]=highest_bit(graphics_current_mode->info->blue)-c_info[5];
		c_info[6]=32-c_info[0]-c_info[2]-c_info[4];
		c_info[7]=32-c_info[6];
	}
	return 0;
}
#define video_display() memcpy(video_mem,video_buf,video_buf_size)
void paint_pixel(unsigned short int x,unsigned short int y,unsigned int color)
{
	unsigned int off=y;
	unsigned int c=0;
	unsigned int r=color>>16&0xff;
	unsigned int g=color>>8&0xff;
	unsigned int b=color&0xff;
	if(x>=sinfo[0]||y>=sinfo[1])
	{
		return;
	}
	off=off*line_length+x*4;
	c|=(r>>(8-c_info[0]))<<c_info[1];
	c|=(g>>(8-c_info[2]))<<c_info[3];
	c|=(b>>(8-c_info[4]))<<c_info[5];
	*(unsigned int *)(video_buf+off)=c;
}
