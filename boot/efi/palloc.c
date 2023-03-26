void *palloc(struct EFI_system_table *table,int pages)
{
	long long int status;
	void *ptr=0;
	eficall(table->boot_services->alloc_pages,4,status,0,2,pages,(i64)&ptr);
	return ptr;
}
void prelease(struct EFI_system_table *table,void *ptr,int pages)
{
	long long int status;
	eficall(table->boot_services->free_pages,2,status,(i64)ptr,pages);
}
