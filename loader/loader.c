/*
 * Loader Implementation
 *
 * 2018, Operating Systems
 */
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#include "exec_parser.h"

static so_exec_t *exec;
static int pageSize = 4096;
static struct sigaction old_action;
int fd, len = 0;
int max_elem = 100;


static void segv_handler(int signum, siginfo_t *info, void *context)
{
	uintptr_t addr;
	int i, found = 0;
	so_seg_t *segm;


	if(info->si_signo != SIGSEGV)
		return;

	addr = (uintptr_t)info->si_addr;
	/* cautam sa vedem daca se afla intr-un segment valid*/
	for (i = 0; i < exec->segments_no; i++) {
		segm = &exec->segments[i];
		if (segm->vaddr <= (uintptr_t)addr &&
		 segm->vaddr + segm->mem_size > (uintptr_t)addr) {
			/*daca l-am gasit atunci ma opresc din cautare*/
			found = 1;
			break;
		}
	}
	if (found == 0) {
		old_action.sa_sigaction(signum, info, context);
	} else {
		int page_index = (addr - segm->vaddr) / pageSize;
		int offset = segm->offset + page_index * pageSize;
		uintptr_t adr_to_map = segm->vaddr + page_index * pageSize;
		uintptr_t *mapari = segm->data;
		int adr_offset = segm->file_size % pageSize;
		int dim_file = ALIGN_DOWN(segm->file_size, pageSize);

		if (mapari[page_index] != 0) {
	/* verific ca nu cumva sa accesez o pagina deja mapata */
			old_action.sa_sigaction(signum, info, context);
		}

		/* verific sa vad daca este intr-o pagina nemapata */
		if (page_index * pageSize >= segm->mem_size) {
		/* inseamna ca incerc sa mapez pe ceva deja mapat*/
		/*deci ar trebui sa apelez seg fault*/
			old_action.sa_sigaction(signum, info, context);
			return;
		}

		void *dst;

		if (segm->vaddr + dim_file > adr_to_map){
			dst = mmap ((void *) adr_to_map, pageSize,
			 PROT_WRITE, MAP_FIXED | MAP_PRIVATE, fd, offset);
			mapari[page_index] = adr_to_map;
		} else if (segm->vaddr + dim_file == adr_to_map &&
		(segm->file_size % pageSize) != 0){

			dst = mmap ((void *) adr_to_map, adr_offset,
			 PROT_WRITE, MAP_FIXED | MAP_PRIVATE, fd, offset);

			memset((void *)segm->vaddr + segm->file_size,
			 0, 4096 - adr_offset);

			mapari[page_index] = adr_to_map;
		} else {
			dst = mmap ((void *) adr_to_map, pageSize,
			PROT_WRITE, MAP_FIXED | MAP_SHARED
			| MAP_ANONYMOUS, -1, 0);

			memset((void *)adr_to_map, 0, 4096);
			mapari[page_index] = adr_to_map;
		}

		if (dst == MAP_FAILED){
			printf("eroare la mmap\n");
			return;
		}
		
		int prot = 0;

		msync(dst, len, MS_SYNC);
		if (segm->perm &PERM_X)
			prot |= PERM_X;
		if (segm->perm & PERM_R)
			prot |= PERM_R;
		if (segm->perm & PERM_W)
			prot |= PERM_W;
		mprotect(dst, 4096, prot);
	}

}

int so_init_loader(void)
{
	/* TODO: initialize on-demand loader */
	struct sigaction action;
	int rc;
	
	action.sa_sigaction = segv_handler;
	sigemptyset(&action.sa_mask);
	sigaddset(&action.sa_mask, SIGSEGV);
	action.sa_flags = SA_SIGINFO;

	rc = sigaction(SIGSEGV, &action, &old_action);
	if (rc == -1)
		return -1;
	return -1;
}

int so_execute(char *path, char *argv[])
{
	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	uintptr_t *vect;

	for(int i = 0; i < exec->segments_no; i++){
		vect = calloc(max_elem, sizeof(uintptr_t));
		exec->segments[i].data = vect;
	}
	fd = open(path, O_RDONLY);
	if (fd == -1){
		printf("eroare la open\n");
		return -1;
	}
	so_start_exec(exec, argv);
	close(fd);
	
	return -1;
}
