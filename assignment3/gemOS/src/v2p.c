#include <types.h>
#include <mmap.h>
#include <fork.h>
#include <v2p.h>
#include <page.h>

/* 
 * You may define macros and other helper functions here
 * You must not declare and use any static/global variables 
 * */

u8 check_addr_free(struct vm_area *vm_area, u64 addr, int length) {
    while (vm_area) {
        if (vm_area->vm_start < addr + length && vm_area->vm_end > addr) {
            return 0;
        }
    }

    return 1;
}

long create_vma(struct vm_area *vm_area, u64 addr, int length, int prot, int flags) {
    struct vm_area *prev = vm_area;
    struct vm_area *cur = vm_area->vm_next;
    while (cur && cur->vm_start < addr) {
        prev = cur;
        cur = cur->vm_next;
    }

    u8 merge_left = prev->vm_end == addr && prev->access_flags == prot;
    u8 merge_right = cur && cur->vm_start == addr + length && cur->access_flags == prot;

    if (merge_left && merge_right) {
        prev->vm_end = cur->vm_end;
        prev->vm_next = cur->vm_next;

        os_free(cur);
        stats->num_vm_area--;
        return addr;
    }

    if (merge_left) {
        prev->vm_end = addr + length;
        return addr;
    }

    if (merge_right) {
        cur->vm_start = addr;
        return addr;
    }

    struct vm_area *new = os_alloc(sizeof(struct vm_area));
    if (!new) {
        return -1;
    }

    stats->num_vm_area++;

    new->vm_start = addr;
    new->vm_end = addr + length;
    new->access_flags = prot;
    new->vm_next = cur;

    prev->vm_next = new;
    return addr;
}

long get_lowest_free_addr(struct vm_area *vm_area, int length) {
    struct vm_area *prev = vm_area;
    struct vm_area *cur = vm_area->vm_next;
    while (cur && cur->vm_start - prev->vm_end < length) {
        prev = cur;
        cur = cur->vm_next;
    }

    return prev->vm_end;
}

/**
 * mprotect System call Implementation.
 */
long vm_area_mprotect(struct exec_context *current, u64 addr, int length, int prot) {
    return -EINVAL;
}

/**
 * mmap system call implementation.
 */
long vm_area_map(struct exec_context *current, u64 addr, int length, int prot, int flags) {
    // check validity of arguments
    if (!current) {
        return -1;
    }
    if (length <= 0) {
        return -1;
    }
    if (prot != PROT_READ && prot != (PROT_READ | PROT_WRITE)) {
        return -1;
    }
    if (flags && flags != MAP_FIXED) {
        return -1;
    }

    // size of VMAs must be a multiple of 4KB
    length = length % 4096 ? length + 4096 - (length % 4096) : length;

    // create dummy node if not present
    if (!current->vm_area) {
        current->vm_area = os_alloc(sizeof(struct vm_area));
        if (!current->vm_area) {
            return -1;
        }

        stats->num_vm_area++;
        struct vm_area *vm_area = current->vm_area;
        vm_area->vm_start = MMAP_AREA_START;
        vm_area->vm_end = vm_area->vm_start + 4096;
        vm_area->access_flags = 0x0;
        vm_area->vm_next = NULL;
    }

    struct vm_area *vm_area = current->vm_area;
    if (addr) {
        if (check_addr_free(vm_area, addr, length)) {
            // use addr
            return create_vma(vm_area, addr, length, prot, flags);

        }
    }

    if (flags == MAP_FIXED) {
        return -1;
    }

    // use lowest available address
    addr = get_lowest_free_addr(vm_area, length);
    return create_vma(vm_area, addr, length, prot, flags);
}

/**
 * munmap system call implemenations
 */
long vm_area_unmap(struct exec_context *current, u64 addr, int length) {
    return -EINVAL;
}

/**
 * Function will invoked whenever there is page fault for an address in the vm area region
 * created using mmap
 */
long vm_area_pagefault(struct exec_context *current, u64 addr, int error_code) {
    return -1;
}

/**
 * cfork system call implemenations
 * The parent returns the pid of child process. The return path of
 * the child process is handled separately through the calls at the 
 * end of this function (e.g., setup_child_context etc.)
 */
long do_cfork() {
    u32 pid;
    struct exec_context *new_ctx = get_new_ctx();
    struct exec_context *ctx = get_current_ctx();
     /* Do not modify above lines
     * 
     * */   
     /*--------------------- Your code [start]---------------*/
     

     /*--------------------- Your code [end] ----------------*/
    
     /*
     * The remaining part must not be changed
     */
    copy_os_pts(ctx->pgd, new_ctx->pgd);
    do_file_fork(new_ctx);
    setup_child_context(new_ctx);
    return pid;
}

/* Cow fault handling, for the entire user address space
 * For address belonging to memory segments (i.e., stack, data) 
 * it is called when there is a CoW violation in these areas. 
 *
 * For vm areas, your fault handler 'vm_area_pagefault'
 * should invoke this function
 * */
long handle_cow_fault(struct exec_context *current, u64 vaddr, int access_flags) {
  return -1;
}
