struct EFI_loaded_image
{
	unsigned int rev;
	void *parent_handle;
	void *system_table;
	void *device_handle;
	void *file_path;
	void *rsv;
	unsigned int load_options_size;
	void *load_options;
	void *image_base;
	unsigned long long int image_size;
	unsigned int image_code_type;
	unsigned int image_data_type;
	void *image_unload;
};
struct EFI_file_io_interface
{
	unsigned long long int rev;
	void *open_volume;
};
struct EFI_file_protocol
{
	unsigned long long int rev;
	void *open;
	void *close;
	void *delete;
	void *read;
	void *write;
	void *get_position;
	void *set_position;
	void *get_info;
	void *set_info;
	void *flush;
	void *open_ex;
	void *read_ex;
	void *write_ex;
	void *flush_ex;
};
unsigned long long int bootid[2];
int fs_init(void *efi_handle,struct EFI_system_table *table)
{
	unsigned int loaded_image_guid[4]={0x5b1b31a1,0x11d29562,0xa0003f8e,0x3b7269c9};
	unsigned int fs_guid[4]={0x964e5b22,0x11d26459,0xa000398e,0x3b7269c9};
	struct EFI_loaded_image *loadedimg=0;
	struct EFI_file_io_interface *fileio=0;
	struct EFI_file_protocol *handle=0,*fh=0;
	long long int status,read_size=16;
	eficall(table->boot_services->handle_prot,3,status,(i64)efi_handle,(i64)loaded_image_guid,(i64)&loadedimg);
	if(status<0)
	{
		return -1;
	}
	eficall(table->boot_services->handle_prot,3,status,(i64)loadedimg->device_handle,(i64)fs_guid,(i64)&fileio);
	if(status<0)
	{
		return -1;
	}
	eficall(fileio->open_volume,2,status,(i64)fileio,(i64)&handle);
	if(status<0)
	{
		return -1;
	}
	eficall(handle->open,5,status,(i64)handle,(i64)&fh,(i64)"E\0F\0I\0\0\0",1,0x37);
	if(status<0)
	{
		return -1;
	}
	eficall(fh->open,5,status,(i64)fh,(i64)&handle,(i64)"B\0O\0O\0T\0\0\0",1,0x37);
	if(status<0)
	{
		return -1;
	}
	eficall(handle->open,5,status,(i64)handle,(i64)&fh,(i64)"B\0O\0O\0T\0I\0D\0\0\0",1,0x37);
	if(status<0)
	{
		return -1;
	}
	eficall(fh->read,3,status,(i64)fh,(i64)&read_size,(i64)bootid);
	if(status<0||read_size!=16)
	{
		return -1;
	}
	eficall(fh->close,1,status,(i64)fh);
	return 0;
}
