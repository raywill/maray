/*
	Name:	page.c
	Copyright: GPL
	Author: Raywill
	Date: 22-12-06
	Description: ????
*/



/*		思考方式：我们只有一个进程！他拥有整个内存空间		*/
		typedef struct{ long a; }PDE;
		typedef struct{ long a; }PTE;


		typedef struct __page_list{
			int	isfree;
			PAGE *page;
			struct __page_list *next;
		}PAGE_LIST;
		
#define NULL_PAGE_LIST	(PAGE_LIST *)0;
#define NULL_PAGE				(PAGE *)0;

		void *alloc_page(long linear_addr)/*return a pointer point to the memory*/
		{
				PDE *pde_p;
				PTE	* pte_p;
				/*now let the task page table and page directory know this page*/
				pde_t=(PDE *)(CR0+(linear_addr>>22)*4);
				if(*pde_t==0){			/* in this case, we need a new entry in PD*/
					PAGE *page_table;
					page_table=find_free_page();
					*pde_t=page_table<<12|SYS_PAGE_DIR_BITS;
				}
				pte_p=(PTE*)(*pde_t>>12);
				/*now,let us get a free PAGE, page point to the start of the free memory*/
				page=find_free_page();
				*pte_p=(page<< 12|SYS_PAGE_TABLE_BITS);
				/*so far ,according to the linear address ,we have written the content to the page table*/
				return page;	/*return the page we have got */				
		}
/* We maintained a list ,recorded all pages we have */
		PAGE *find_free_page(void)
		{
				PAGE_LIST *p;
				while(p!=NULL_PAGE_LIST){
					if(p->isfree)
						return p->page;
					p=p->next;
				}
				return NULL_PAGE;
		}
		
/* Do you really think we need this?
		int fill_page(long pte,long *phyaddr);
		{
				(*phyaddr)=pte;
				return	0;
		}
*/



		