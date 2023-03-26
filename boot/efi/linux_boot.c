unsigned long long int locate_acpi(struct EFI_system_table *table)
{
	int x=0;
	long long int status;
	unsigned int guid[4]={0xeb9d2d30,0x11d32d88,0x9000169a,0x4dc13f27};
	unsigned int guid2[4]={0x8868e871,0x11d3e4f1,0x800022bc,0x81883cc7};
	while(x<table->n_entries)
	{
		if(!memcmp(table->config_table[x].guid,guid,16))
		{
			return (unsigned long long int)table->config_table[x].addr;
		}
		if(!memcmp(table->config_table[x].guid,guid2,16))
		{
			return (unsigned long long int)table->config_table[x].addr;
		}
		x++;
	}
	return 0;
}

struct linux_screen_info
{
	unsigned char orig_x;
	unsigned char orig_y;
	unsigned short int ext_mem;
	unsigned short int orig_video_page;
	unsigned char orig_video_mode;
	unsigned char orig_video_cols;
	unsigned char flags;
	unsigned char rsv1;
	unsigned short int orig_video_ega_bx;
	unsigned short int rsv2;
	unsigned char orig_video_lines;
	unsigned char orig_video_isvga;
	unsigned short int orig_video_points;
	unsigned short int width;
	unsigned short int height;
	unsigned short int depth;
	unsigned int base;
	unsigned int size;
	unsigned short int magic;
	unsigned short int offset;
	unsigned short int line_length;
	unsigned char red_size;
	unsigned char red_pos;
	unsigned char green_size;
	unsigned char green_pos;
	unsigned char blue_size;
	unsigned char blue_pos;
	unsigned char rsvd_size;
	unsigned char rsvd_pos;
	unsigned short int vesapm_seg;
	unsigned short int vesapm_off;
	unsigned short int pages;
	unsigned short int attributes;
	unsigned int cap;
	unsigned int ext_base;
	unsigned short int unused2;
}__attribute__((packed));
struct _e820_entry
{
	unsigned long long int addr;
	unsigned long long int size;
	unsigned int type;
}__attribute__((packed));
struct linux_boot_params
{
	struct linux_screen_info sinfo;
	char unused1[0x80];
	unsigned int ext_ramdisk_image;
	unsigned int ext_ramdisk_size;
	unsigned int ext_cmdline; 
	char unused2[0x11c];
	unsigned char n_e820;
	char unused3[8];
	char setup_sects;//0x1f1
	char unused4[0x8];
	unsigned short int vid_mode;//0x1fa
	char unused5[4];
	char jump[2];
	unsigned int magic;
	unsigned short int version;//0x206
	char unused6[0x8];
	unsigned char loader_type; //enter 0xff
	unsigned char load_flags; //0x211 enter 0x80  
	char unused7[6];
	unsigned int ramdisk_image;
	unsigned int ramdisk_size; //0x21c
	char unused8[8];
	unsigned int cmdline; //0x228
	char unused9[0xa4];
	struct _e820_entry _e820_table[1];
}__attribute__((packed));

unsigned long long int GDT[]={0,0,
0x002f9a000000ffff,
0x002f92000000ffff};
struct _GDTR
{
	unsigned short int len;
	void *pos;
} __attribute__((packed));
struct _GDTR GDTR={sizeof(GDT)-1,GDT};

struct EFI_md
{
	unsigned int type;
	unsigned int pad;
	unsigned long long int paddr;
	unsigned long long int vaddr;
	unsigned long long int pages;
	unsigned long long int attr;
};
int fill_e820(struct EFI_system_table *table,struct linux_boot_params *boot_params)
{
	long long int status;
	struct EFI_md *meminfo=0,*info;
	unsigned long long int size=0;
	unsigned long long int key;
	unsigned long long int desc_size;
	unsigned int version;
	unsigned int np;
	unsigned int nentries,x=0;
	eficall(table->boot_services->getmemmap,5,status,(i64)&size,(i64)meminfo,(i64)&key,(i64)&desc_size,(i64)&version);
	if(status!=0x8000000000000005)
	{
		return -1;
	}
	np=size+2*desc_size+4095>>12;
	if((meminfo=palloc(table,np))==0)
	{
		return -1;
	}
	eficall(table->boot_services->getmemmap,5,status,(i64)&size,(i64)meminfo,(i64)&key,(i64)&desc_size,(i64)&version);
	if(status<0)
	{
		prelease(table,meminfo,np);
		return -1;
	}
	nentries=size/desc_size;
	info=meminfo;
	while(x<nentries)
	{
		boot_params->_e820_table[x].addr=info->paddr;
		boot_params->_e820_table[x].size=info->pages<<12;
		switch(info->type)
		{
			case 1:
			case 2:
			case 3:
			case 4:
			case 7:
			boot_params->_e820_table[x].type=1;
			break;
			case 9:
			boot_params->_e820_table[x].type=3;
			break;
			case 10:
			boot_params->_e820_table[x].type=4;
			break;
			case 14:
			boot_params->_e820_table[x].type=7;
			break;
			default:
			boot_params->_e820_table[x].type=2;
		}
		info=(void *)((char *)info+desc_size);
		x++;
	}
	boot_params->n_e820=nentries;
	eficall(table->boot_services->exit_boot_services,2,status,(i64)image_handle_this,key);
	if(status<0)
	{
		prelease(table,meminfo,np);
		return -1;
	}
	return 0;
}
void do_boot_linux(void *image,void *params);
void boot_linux(struct EFI_system_table *table,struct file_image *kernel,struct file_image *initrd,char *cmdline)
{
	unsigned int end_header=kernel->image[0x201];
	struct linux_boot_params *boot_params;
	void *linux_buf;
	unsigned long long int map_key;
	if((boot_params=palloc(table,1))==0)
	{
		return;
	}
	memset(boot_params,0,4096);
	boot_params->sinfo.orig_video_mode=3;
	boot_params->sinfo.orig_video_cols=0x50;
	boot_params->sinfo.orig_video_ega_bx=0x03;
	boot_params->sinfo.orig_video_lines=0x19;
	boot_params->sinfo.orig_video_isvga=0x23;
	boot_params->sinfo.orig_video_points=0x10;
	boot_params->sinfo.width=sinfo[0];
	boot_params->sinfo.height=sinfo[1];
	boot_params->sinfo.depth=32;
	boot_params->sinfo.base=(i64)video_mem;
	boot_params->sinfo.ext_base=(i64)video_mem>>32;
	boot_params->sinfo.size=(i64)sinfo[1]*line_length;
	boot_params->sinfo.line_length=line_length;
	boot_params->sinfo.red_size=c_info[0];
	boot_params->sinfo.red_pos=c_info[1];
	boot_params->sinfo.green_size=c_info[2];
	boot_params->sinfo.green_pos=c_info[3];
	boot_params->sinfo.blue_size=c_info[4];
	boot_params->sinfo.blue_pos=c_info[5];
	boot_params->sinfo.rsvd_size=c_info[6];
	boot_params->sinfo.rsvd_pos=c_info[7];
	boot_params->sinfo.cap=2;
	boot_params->vid_mode=0x318;
	end_header+=0x202;
	memcpy((char *)boot_params+0x1f1,kernel->image+0x1f1,end_header-0x1f1);
	if(boot_params->magic!=0x53726448||boot_params->version<0x20b)
	{
		goto Err;
	}
	if(initrd->image)
	{
		boot_params->ramdisk_image=(i64)initrd->image;
		boot_params->ext_ramdisk_image=(i64)initrd->image>>32;
		boot_params->ramdisk_size=initrd->size;
		boot_params->ext_ramdisk_size=initrd->size>>32;
	}
	boot_params->cmdline=(i64)cmdline;
	boot_params->ext_cmdline=(i64)cmdline>>32;
	boot_params->loader_type=0xff;
	boot_params->load_flags|=0x20;
	if((linux_buf=palloc(table,0x30000))==0)
	{
		goto Err;
	}
	if(fill_e820(table,boot_params)<0)
	{
		goto Err2;
	}
	memset(video_mem,0,(int)sinfo[1]*line_length);
	memcpy(linux_buf,kernel->image,kernel->size);
	do_boot_linux(linux_buf,boot_params);
Err2:
	prelease(table,linux_buf,0x30000);
Err:
	prelease(table,boot_params,1);
}
asm("do_boot_linux:");
asm("pushq $0x10");
asm("lea do_boot_linux_X1(%rip),%rax");
asm("push %rax");
asm("lgdt GDTR(%rip)");
asm("mov $0x18,%ax");
asm("mov %ax,%ds");
asm("mov %ax,%es");
asm("mov %ax,%fs");
asm("mov %ax,%gs");
asm("mov %ax,%ss");
asm("cli");
asm("lretq");
asm("do_boot_linux_X1:");
asm("movzbl 0x1f1(%rdi),%eax");
asm("test %al,%al");
asm("jne do_boot_linux_X2");
asm("mov $4,%al");
asm("do_boot_linux_X2:");
asm("inc %eax");
asm("inc %eax");
asm("shl $9,%eax");
asm("add %rdi,%rax");
asm("jmp *%rax");
