struct EFI_input_interface
{
	void *reset;
	void *read_key;
	void *event;
};
struct EFI_output_interface
{
	void *reset;
	void *output_string;
	void *test_string;
	void *query_mode;
	void *set_mode;
	void *set_attribute;
	void *clear_screen;
	void *set_cursor_position;
	void *enable_cursor;
	void *c_mode;
};
struct EFI_runtime_services
{
	unsigned int header[6];
	void *gettime;
	void *settime;
	void *getwakeuptime;
	void *setwakeuptime;
	void *setmmap;
	void *convptr;
	void *getval;
	void *getnextval;
	void *setval;
	void *get_next_high_mono_count;
	void *reset_system;
	void *updatecap;
	void *querycapcap;
	void *queryvar;
};
struct EFI_boot_services
{
	unsigned int header[6];
	void *raise_tpl;
	void *restore_tpl;
	void *alloc_pages;
	void *free_pages;
	void *getmemmap;
	void *alloc_pool;
	void *free_pool;
	void *create_event;
	void *set_timer;
	void *waitforevent;
	void *signalevent;
	void *close_event;
	void *check_event;
	void *install_interf;
	void *reinstall_interf;
	void *uninstall_interf;
	void *handle_prot;
	void *handle_prot2;
	void *register_prot;
	void *locate_handle;
	void *locate_devpath;
	void *install_config;
	void *loadimg;
	void *startimg;
	void *exit;
	void *unload;
	void *exit_boot_services;
	void *get_next_mono_count;
	void *stall;
	void *set_watchdog_timer;
	void *connect_controller;
	void *disconnect_controller;
	void *open_prot;
	void *close_prot;
	void *open_prot_info;
	void *prots_per_handle;
	void *locate_handle_buf;
	void *locate_prot;
	void *install_multiple_prot;
	void *uninstall_multiple_prot;
	void *crc32;
	void *copy_mem;
	void *set_mem;
	void *create_event_ex;
};
struct EFI_config_table
{
	unsigned int guid[4];
	void *addr;
};
struct EFI_system_table
{
	unsigned int header[6];
	short int *fwvendor;
	unsigned int fwrev;
	void *coninhandle;
	struct EFI_input_interface *conin;
	void *conouthandle;
	struct EFI_output_interface *conout;
	void *conerrhandle;
	struct EFI_output_interface *conerr;
	struct EFI_runtime_services *runtime_services;
	struct EFI_boot_services *boot_services;
	unsigned long long int n_entries;
	struct EFI_config_table *config_table;
};
long long int _eficall(void *handler,int count,long long int *args);
asm(".text");
asm("_eficall:");
asm("push %rcx");
asm("push %rdx");
asm("push %rbx");
asm("push %rbp");
asm("push %rsi");
asm("push %rdi");
asm("push %r8");
asm("push %r9");
asm("push %r12");
asm("push %r13");
asm("push %r14");
asm("push %r15");
asm("mov %rdi,%rax");
asm("cmp $1,%esi");
asm("jl _efi_end");
asm("mov %rsp,%rbp");
asm("mov (%rdx),%rcx");
asm("cmp $2,%esi");
asm("jl _efi_call");
asm("mov 8(%rdx),%rbx");
asm("cmp $3,%esi");
asm("jl _efi_call");
asm("mov 16(%rdx),%r8");
asm("cmp $4,%esi");
asm("jl _efi_call");
asm("mov 24(%rdx),%r9");
asm("_efi_call:");
asm("pushq -8(%rdx,%rsi,8)");
asm("dec %esi");
asm("jne _efi_call");
asm("mov %rbx,%rdx");
asm("call *%rax");
asm("mov %rbp,%rsp");
asm("_efi_end:");
asm("pop %r15");
asm("pop %r14");
asm("pop %r13");
asm("pop %r12");
asm("pop %r9");
asm("pop %r8");
asm("pop %rdi");
asm("pop %rsi");
asm("pop %rbp");
asm("pop %rbx");
asm("pop %rdx");
asm("pop %rcx");
asm("ret");
#define eficall(handler,n,status,...)  \
{\
	long long int array[(n)+5&0xfffffffe]={__VA_ARGS__}; \
	status=_eficall(handler,(n)+5&0xfffffffe,array); \
}
void efi_reboot(struct EFI_system_table *table)
{
	long long int status;
	eficall(table->runtime_services->reset_system,4,status,0,0,0,0);
}
typedef unsigned long long int i64;
