#ifndef _My_OS_Memcmp_
#define _My_OS_Memcmp_
#pragma GCC push_options
#pragma GCC optimize("O2")
int memcmp(void *ptr1,void *ptr2,unsigned long long int size)
{
	unsigned long long int x=0;
	int a1,a2;
	while(size)
	{
		a1=((unsigned char *)ptr1)[x];
		a2=((unsigned char *)ptr2)[x];
		if(a1-=a2)
		{
			return a1;
		}
		x++;
		size--;
	}
	return 0;
}
#pragma GCC pop_options
#endif
