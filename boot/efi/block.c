void **block_handles=0;
long long int count_block_handles=0,bufsize;
struct EFI_block_io_media
{
	unsigned int id;
	char removable;
	char present;
	char logicalpartition;
	char readonly;
	char writecaching;
	unsigned int blocksize;
	unsigned int align;
	long long int last_block;
	//unused
};
struct EFI_block_io
{
	unsigned long long int rev;
	struct EFI_block_io_media *media;
	void *reset;
	void *readblocks;
	void *writeblocks;
	void *flushblocks;
} *bio=0;
int read_blocks(struct EFI_block_io *bio,long long int start,long long int size,void *buf)
{
	long long int status;
	if(start>bio->media->last_block)
	{
		return 0;
	}
	if(start*bio->media->blocksize+size>bio->media->last_block*bio->media->blocksize)
	{
		size=(bio->media->last_block-start)*bio->media->blocksize;
	}
	eficall(bio->readblocks,5,status,(i64)bio,bio->media->id,start,size,(i64)buf);
	if(status<0)
	{
		return 0;
	}
	return size;
}
unsigned long long int bootid2[512];
int block_init(struct EFI_system_table *table)
{
	unsigned int bio_guid[4]={0x964e5b21,0x11d26459,0xa000398e,0x3b7269c9};
	long long int status,x;
	struct EFI_block_io *c_bio;
	eficall(table->boot_services->locate_handle,5,status,2,(i64)bio_guid,0,(i64)&count_block_handles,(i64)block_handles);
	if(status!=0x8000000000000005||count_block_handles==0)
	{
		return -1;
	}
	bufsize=count_block_handles;
	eficall(table->boot_services->locate_handle_buf,5,status,2,(i64)bio_guid,0,(i64)&bufsize,(i64)&block_handles);
	if(status<0)
	{
		return -1;
	}
	count_block_handles>>=3;
	x=0;
	while(x<count_block_handles)
	{
		c_bio=0;
		eficall(table->boot_services->handle_prot,3,status,(i64)block_handles[x],(i64)bio_guid,(i64)&c_bio);
		if(status<0||c_bio==0||c_bio->media->blocksize==0)
		{
			x++;
			continue;
		}
		if(read_blocks(c_bio,0,4096,bootid2)==4096&&bootid[0]==bootid2[0]&&bootid[1]==bootid2[1])
		{
			bio=c_bio;
			return 0;
		}
		x++;
	}
	return -1;
}
