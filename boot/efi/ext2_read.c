#include "ext2.c"
char ext2_buf[65536];
unsigned int last_read=0xffffffff;
struct ext2_superblock sb;
struct ext2_bgdt *gt;
struct ext2_inode selected_file;
unsigned int ext2_ind_buf[1024];
unsigned int ext2_dind_buf[2048];
unsigned int ext2_tind_buf[3072];
unsigned int ind_nblock[1];
unsigned int dind_nblock[2];
unsigned int tind_nblock[3];
unsigned int groups;
int ext2_init(struct EFI_system_table *table)
{
	unsigned int n;
	unsigned char *buf;
	read_blocks(bio,0,65536,ext2_buf);
	memcpy(&sb,ext2_buf+1024,1024);
	if(sb.block_size>2||sb.magic!=0xef53||sb.feature_incompat&0xfffffffd)
	{
		return -1;
	}
	groups=(sb.blocks-1)/sb.blocks_per_group+1;
	n=(groups*32-1)/65536+2;
	if((buf=palloc(table,n*16))==0)
	{
		return -1;
	}
	read_blocks(bio,0,n*65536,buf);
	if(sb.block_size)
	{
		gt=(void *)(buf+(1<<sb.block_size+10));
	}
	else
	{
		gt=(void *)(buf+2048);
	}
	return 0;
}
void *ext2_read_block(unsigned int nblock)
{
	unsigned int off=nblock,off1;
	unsigned int block_sectors=65536/bio->media->blocksize;
	off1=off%(64>>sb.block_size);
	off-=off1;
	if(off==last_read)
	{
		return ext2_buf+(off1<<sb.block_size+10);
	}
	read_blocks(bio,off/(64>>sb.block_size)*block_sectors,65536,ext2_buf);
	last_read=off;
	return ext2_buf+(off1<<sb.block_size+10);
}
int ext2_load_inode(unsigned int ninode)
{
	unsigned int ngroup=(ninode-1)/sb.inodes_per_group;
	unsigned int index=(ninode-1)%sb.inodes_per_group;
	unsigned int nblock=gt[ngroup].inode_table+index*sb.inode_size/(1<<sb.block_size+10);
	unsigned int off=index*sb.inode_size%(1<<sb.block_size+10);
	unsigned char *buf;
	buf=ext2_read_block(nblock);
	memcpy(&selected_file,buf+off,128);
	return 0;
}
#define ext2_block_not_exist ((void *)-1)
void *ext2_file_read_block(unsigned int nblock)
{
	unsigned int size1=sb.block_size+8,size2=size1+sb.block_size+8,size3=size2+sb.block_size+8;
	unsigned long long int fsize=selected_file.dir_acl;
	unsigned int n1,n2,n3;
	if((selected_file.mode&0170000)==040000)
	{
		fsize=selected_file.size;
	}
	else
	{
		fsize=fsize<<32|selected_file.size;
	}
	if(fsize==0||nblock>=(fsize-1>>sb.block_size+10)+1)
	{
		return ext2_block_not_exist;
	}
	if(nblock<12)
	{
		if(selected_file.block[nblock]==0)
		{
			return (void *)0;
		}
		else
		{
			return ext2_read_block(selected_file.block[nblock]);
		}
	}
	nblock-=12;
	if(nblock<1<<size1)
	{
		if(selected_file.block[12]==0)
		{
			return (void *)0;
		}
		if(ind_nblock[0]==selected_file.block[12])
		{
			if(ext2_ind_buf[nblock]==0)
			{
				return (void *)0;
			}
			else
			{
				return ext2_read_block(ext2_ind_buf[nblock]);
			}
		}
		else
		{
			char *buf=ext2_read_block(selected_file.block[12]);
			memcpy(ext2_ind_buf,buf,4<<size1);
			ind_nblock[0]=selected_file.block[12];
			if(ext2_ind_buf[nblock]==0)
			{
				return (void *)0;
			}
			else
			{
				return ext2_read_block(ext2_ind_buf[nblock]);
			}
		}
	}
	nblock-=1<<size1;
	if(nblock<1<<size2)
	{
		n1=nblock>>size1;
		n2=nblock%(1<<size1);
		if(selected_file.block[13]==0)
		{
			return (void *)0;
		}
		if(dind_nblock[0]==selected_file.block[13])
		{
			if(ext2_dind_buf[n1]==0)
			{
				return (void *)0;
			}
			if(dind_nblock[1]==ext2_dind_buf[n1])
			{
				if(ext2_dind_buf[n2+1024]==0)
				{
					return (void *)0;
				}
				else
				{
					return ext2_read_block(ext2_dind_buf[n2+1024]);
				}
			}
			else
			{
				char *buf=ext2_read_block(ext2_dind_buf[n1]);
				memcpy(ext2_dind_buf+1024,buf,4<<size1);
				dind_nblock[1]=ext2_dind_buf[n1];
				if(ext2_dind_buf[n2+1024]==0)
				{
					return (void *)0;
				}
				else
				{
					return ext2_read_block(ext2_dind_buf[n2+1024]);
				}
			}
		}
		else
		{
			char *buf=ext2_read_block(selected_file.block[13]);
			memcpy(ext2_dind_buf,buf,4<<size1);
			dind_nblock[0]=selected_file.block[13];
			if(ext2_dind_buf[n1]==0)
			{
				return (void *)0;
			}
			if(dind_nblock[1]==ext2_dind_buf[n1])
			{
				if(ext2_dind_buf[n2+1024]==0)
				{
					return (void *)0;
				}
				else
				{
					return ext2_read_block(ext2_dind_buf[n2+1024]);
				}
			}
			else
			{
				char *buf=ext2_read_block(ext2_dind_buf[n1]);
				memcpy(ext2_dind_buf+1024,buf,4<<size1);
				dind_nblock[1]=ext2_dind_buf[n1];
				if(ext2_dind_buf[n2+1024]==0)
				{
					return (void *)0;
				}
				else
				{
					return ext2_read_block(ext2_dind_buf[n2+1024]);
				}
			}
		}
	}
	nblock-=1<<size2;
	if(nblock<1<<size3)
	{
		n1=nblock>>size2;
		n2=nblock%(1<<size2)>>size1;
		n3=nblock%(1<<size1);
		if(selected_file.block[14]==0)
		{
			return (void *)0;
		}
		if(tind_nblock[0]==selected_file.block[14])
		{
			if(ext2_tind_buf[n1]==0)
			{
				return (void *)0;
			}
			if(tind_nblock[1]==ext2_tind_buf[n1])
			{
				if(ext2_tind_buf[n2+1024]==0)
				{
					return (void *)0;
				}
				if(tind_nblock[2]==ext2_tind_buf[n2+1024])
				{
					if(ext2_tind_buf[n3+2048]==0)
					{
						return (void *)0;
					}
					else
					{
						return ext2_read_block(ext2_tind_buf[n3+2048]);
					}
				}
				else
				{
					char *buf=ext2_read_block(ext2_tind_buf[n2+1024]);
					memcpy(ext2_tind_buf+2048,buf,4<<size1);
					tind_nblock[2]=ext2_tind_buf[n2+1024];
					if(ext2_tind_buf[n3+2048]==0)
					{
						return (void *)0;
					}
					else
					{
						return ext2_read_block(ext2_tind_buf[n3+2048]);
					}
				}
			}
			else
			{
				char *buf=ext2_read_block(ext2_tind_buf[n1]);
				memcpy(ext2_tind_buf+1024,buf,4<<size1);
				tind_nblock[1]=ext2_tind_buf[n1];
				if(ext2_tind_buf[n2+1024]==0)
				{
					return (void *)0;
				}
				if(tind_nblock[2]==ext2_tind_buf[n2+1024])
				{
					if(ext2_tind_buf[n3+2048]==0)
					{
						return (void *)0;
					}
					else
					{
						return ext2_read_block(ext2_tind_buf[n3+2048]);
					}
				}
				else
				{
					char *buf=ext2_read_block(ext2_tind_buf[n2+1024]);
					memcpy(ext2_tind_buf+2048,buf,4<<size1);
					tind_nblock[2]=ext2_tind_buf[n2+1024];
					if(ext2_tind_buf[n3+2048]==0)
					{
						return (void *)0;
					}
					else
					{
						return ext2_read_block(ext2_tind_buf[n3+2048]);
					}
				}
			}
		}
		else
		{
			char *buf=ext2_read_block(selected_file.block[14]);
			memcpy(ext2_tind_buf,buf,4<<size1);
			tind_nblock[0]=selected_file.block[14];
			if(ext2_tind_buf[n1]==0)
			{
				return (void *)0;
			}
			if(tind_nblock[1]==ext2_tind_buf[n1])
			{
				if(ext2_tind_buf[n2+1024]==0)
				{
					return (void *)0;
				}
				if(tind_nblock[2]==ext2_tind_buf[n2+1024])
				{
					if(ext2_tind_buf[n3+2048]==0)
					{
						return (void *)0;
					}
					else
					{
						return ext2_read_block(ext2_tind_buf[n3+2048]);
					}
				}
				else
				{
					char *buf=ext2_read_block(ext2_tind_buf[n2+1024]);
					memcpy(ext2_tind_buf+2048,buf,4<<size1);
					tind_nblock[2]=ext2_tind_buf[n2+1024];
					if(ext2_tind_buf[n3+2048]==0)
					{
						return (void *)0;
					}
					else
					{
						return ext2_read_block(ext2_tind_buf[n3+2048]);
					}
				}
			}
			else
			{
				char *buf=ext2_read_block(ext2_tind_buf[n1]);
				memcpy(ext2_tind_buf+1024,buf,4<<size1);
				tind_nblock[1]=ext2_tind_buf[n1];
				if(ext2_tind_buf[n2+1024]==0)
				{
					return (void *)0;
				}
				if(tind_nblock[2]==ext2_tind_buf[n2+1024])
				{
					if(ext2_tind_buf[n3+2048]==0)
					{
						return (void *)0;
					}
					else
					{
						return ext2_read_block(ext2_tind_buf[n3+2048]);
					}
				}
				else
				{
					char *buf=ext2_read_block(ext2_tind_buf[n2+1024]);
					memcpy(ext2_tind_buf+2048,buf,4<<size1);
					tind_nblock[2]=ext2_tind_buf[n2+1024];
					if(ext2_tind_buf[n3+2048]==0)
					{
						return (void *)0;
					}
					else
					{
						return ext2_read_block(ext2_tind_buf[n3+2048]);
					}
				}
			}
		}
	}
	return ext2_block_not_exist;
}
unsigned int ext2_readdir(struct ext2_directory **dirent,unsigned int off)
{
	unsigned int nblock=off>>sb.block_size+10;
	unsigned int off1=off%(1<<sb.block_size+10);
	unsigned char *buf;
	if((selected_file.mode&0170000)!=040000)
	{
		return 0;
	}
	buf=ext2_file_read_block(nblock);
	if(buf==0||buf==ext2_block_not_exist)
	{
		return 0;
	}
	*dirent=(void *)(buf+off1);
	return off+(*dirent)->rec_len;
}
int ext2_search(char *name,int len)
{
	if(len==0||len>255)
	{
		return 0;
	}
	unsigned int off=0;
	struct ext2_directory *dir;
	while(off=ext2_readdir(&dir,off))
	{
		if(dir->inode&&dir->inode<=sb.inodes)
		{
			if(dir->name_len==len&&!memcmp(dir->file_name,name,len))
			{
				ext2_load_inode(dir->inode);
				return 1;
			}
		}
	}
	return 0;
}
int ext2_parse_path(char *path)
{
	ext2_load_inode(2);
	int x1=0,x2;
	while(path[x1]=='/')
	{
		x1++;
	}
	x2=x1;
	while(path[x1])
	{
		if(path[x1]=='/')
		{
			if(ext2_search(path+x2,x1-x2)==0)
			{
				return 0;
			}
			while(path[x1]=='/')
			{
				x1++;
			}
			x2=x1;
		}
		x1++;
	}
	if(x1==x2)
	{
		return 0;
	}
	if(ext2_search(path+x2,x1-x2)==0)
	{
		return 0;
	}
	return 1;
}
void *ext2_load(struct EFI_system_table *table,unsigned long long int *size)
{
	unsigned long long int file_size,off=0;
	char *buf,*buf1;
	unsigned int bsize=1<<sb.block_size+10;
	if((selected_file.mode&0170000)!=0100000)
	{
		return (void *)0;
	}
	file_size=selected_file.dir_acl;
	file_size=file_size<<32|selected_file.size;
	if((buf=palloc(table,(file_size+4095>>12)))==0)
	{
		return (void *)0;
	}
	while(1)
	{
		buf1=ext2_file_read_block(off);
		if(buf1==0)
		{
			memset(buf+off*bsize,0,bsize);
		}
		else if(buf1==ext2_block_not_exist)
		{
			break;
		}
		else
		{
			memcpy(buf+off*bsize,buf1,bsize);
		}
		off++;
	}
	*size=file_size;
	return buf;
}
