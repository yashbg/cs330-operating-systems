#include <types.h>
#include <mmap.h>
#include <fork.h>
#include <v2p.h>
#include <page.h>
#include <memory.h>

/* 
 * You may define macros and other helper functions here
 * You must not declare and use any static/global variables 
 * */

#define PAGE_SIZE 4096

#define PAGE_FAULT_PRESENT 0x1
#define PAGE_FAULT_WRITE 0x2

#define PTE_PRESENT 0x1
#define PTE_WRITE 0x8
#define PTE_USER 0x10

int create_dummy_vma(struct exec_context *current) {
    current->vm_area = os_alloc(sizeof(struct vm_area));
    if (!current->vm_area) {
        return -EINVAL;
    }

    stats->num_vm_area++;

    struct vm_area *vm_area = current->vm_area;
    vm_area->vm_start = MMAP_AREA_START;
    vm_area->vm_end = vm_area->vm_start + PAGE_SIZE;
    vm_area->access_flags = 0x0;
    vm_area->vm_next = NULL;
    
    return 0;
}

u8 check_addr_free(struct vm_area *vm_area, u64 addr, int length) {
    struct vm_area *cur = vm_area;
    while (cur) {
        if (cur->vm_start < addr + length && cur->vm_end > addr) {
            return 0;
        }

        cur = cur->vm_next;
    }

    return 1;
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

        os_free(cur, sizeof(struct vm_area));
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
        return -EINVAL;
    }

    stats->num_vm_area++;

    new->vm_start = addr;
    new->vm_end = addr + length;
    new->access_flags = prot;
    new->vm_next = cur;

    prev->vm_next = new;
    return addr;
}

long delete_vmas(struct vm_area *vm_area, u64 addr, int length) {
    struct vm_area *prev = vm_area;
    struct vm_area *cur = vm_area->vm_next;
    while (cur && cur->vm_end <= addr) {
        prev = cur;
        cur = cur->vm_next;
    }

    while (cur && cur->vm_start < addr + length) {
        if (cur->vm_start < addr && cur->vm_end > addr + length) {
            struct vm_area *new = os_alloc(sizeof(struct vm_area));
            if (!new) {
                return -EINVAL;
            }

            stats->num_vm_area++;

            new->vm_start = addr + length;
            new->vm_end = cur->vm_end;
            new->access_flags = cur->access_flags;
            new->vm_next = cur->vm_next;

            cur->vm_end = addr;
            cur->vm_next = new;

            cur = cur->vm_next;
            continue;
        }

        if (cur->vm_start < addr) {
            cur->vm_end = addr;
            cur = cur->vm_next;
            continue;
        }

        if (cur->vm_end > addr + length) {
            cur->vm_start = addr + length;
            cur = cur->vm_next;
            continue;
        }

        prev->vm_next = cur->vm_next;
        os_free(cur, sizeof(struct vm_area));
        stats->num_vm_area--;

        cur = prev->vm_next;
    }
    
    return 0;
}

long merge_vmas(struct vm_area *vm_area) {
    struct vm_area *prev = vm_area;
    struct vm_area *cur = vm_area->vm_next;
    while (cur) {
        if (prev->access_flags == cur->access_flags && prev->vm_end == cur->vm_start) {
            prev->vm_end = cur->vm_end;
            prev->vm_next = cur->vm_next;

            os_free(cur, sizeof(struct vm_area));
            stats->num_vm_area--;
            
            cur = prev->vm_next;
            continue;
        }

        prev = cur;
        cur = cur->vm_next;
    }

    return 0;
}

long change_vma_protections(struct vm_area *vm_area, u64 addr, int length, int prot) {
    struct vm_area *prev = vm_area;
    struct vm_area *cur = vm_area->vm_next;
    while (cur && cur->vm_end <= addr) {
        prev = cur;
        cur = cur->vm_next;
    }

    while (cur && cur->vm_start < addr + length) {
        if (cur->access_flags == prot) {
            continue;
        }

        if (cur->vm_start < addr && cur->vm_end > addr + length) {
            struct vm_area *new1 = os_alloc(sizeof(struct vm_area));
            if (!new1) {
                return -EINVAL;
            }

            stats->num_vm_area++;

            new1->vm_start = addr;
            new1->vm_end = addr + length;
            new1->access_flags = prot;

            struct vm_area *new2 = os_alloc(sizeof(struct vm_area));
            if (!new2) {
                return -EINVAL;
            }

            stats->num_vm_area++;

            new2->vm_start = addr + length;
            new2->vm_end = cur->vm_end;
            new2->access_flags = cur->access_flags;
            new2->vm_next = cur->vm_next;

            cur->vm_end = addr;
            cur->vm_next = new1;

            new1->vm_next = new2;

            prev = new2;
            cur = new2->vm_next;
            continue;
        }

        if (cur->vm_start < addr) {
            struct vm_area *new = os_alloc(sizeof(struct vm_area));
            if (!new) {
                return -EINVAL;
            }

            stats->num_vm_area++;

            new->vm_start = addr;
            new->vm_end = cur->vm_end;
            new->access_flags = prot;
            new->vm_next = cur->vm_next;
            
            cur->vm_end = addr;

            prev = new;
            cur = new->vm_next;
            continue;
        }

        if (cur->vm_end > addr + length) {
            struct vm_area *new = os_alloc(sizeof(struct vm_area));
            if (!new) {
                return -EINVAL;
            }

            stats->num_vm_area++;

            new->vm_start = addr + length;
            new->vm_end = cur->vm_end;
            new->access_flags = cur->access_flags;
            new->vm_next = cur->vm_next;

            cur->vm_end = addr + length;
            cur->access_flags = prot;
            cur->vm_next = new;
            
            prev = new;
            cur = new->vm_next;
            continue;
        }

        cur->access_flags = prot;
        prev = cur;
        cur = cur->vm_next;
    }

    return merge_vmas(vm_area);
}

u8 check_access_valid(struct vm_area *vm_area, u64 addr, u32 access_flags) {
    struct vm_area *cur = vm_area;
    while (cur->vm_start <= addr) {
        if (cur->vm_start <= addr && cur->vm_end > addr) {
            return cur->access_flags & access_flags;
        }

        cur = cur->vm_next;
    }

    return 0;
}

u32 get_vma_access_flags(struct vm_area *vm_area, u64 addr) {
    struct vm_area *cur = vm_area;
    while (cur->vm_start <= addr) {
        if (cur->vm_start <= addr && cur->vm_end > addr) {
            return cur->access_flags;
        }

        cur = cur->vm_next;
    }

    return 0;
}

/**
 * mprotect System call Implementation.
 */
long vm_area_mprotect(struct exec_context *current, u64 addr, int length, int prot) {
    // check validity of arguments
    if (!current) {
        return -EINVAL;
    }
    if (length <= 0) {
        return -EINVAL;
    }
    if (prot != PROT_READ && prot != (PROT_READ | PROT_WRITE)) {
        return -EINVAL;
    }

    // change protections at page granularity
    length = length % PAGE_SIZE ? length + PAGE_SIZE - (length % PAGE_SIZE) : length;

    // create dummy node if not present
    if (!current->vm_area) {
        if (create_dummy_vma(current) < 0) {
            return -EINVAL;
        }
    }

    // change protections
    return change_vma_protections(current->vm_area, addr, length, prot);
}

/**
 * mmap system call implementation.
 */
long vm_area_map(struct exec_context *current, u64 addr, int length, int prot, int flags) {
    // check validity of arguments
    if (!current) {
        return -EINVAL;
    }
    if (length <= 0) {
        return -EINVAL;
    }
    if (prot != PROT_READ && prot != (PROT_READ | PROT_WRITE)) {
        return -EINVAL;
    }
    if (flags && flags != MAP_FIXED) {
        return -EINVAL;
    }

    // size of VMAs must be a multiple of 4KB
    length = length % PAGE_SIZE ? length + PAGE_SIZE - (length % PAGE_SIZE) : length;

    // create dummy node if not present
    if (!current->vm_area) {
        if (create_dummy_vma(current) < 0) {
            return -EINVAL;
        }
    }

    // create mapping
    struct vm_area *vm_area = current->vm_area;
    if (addr) {
        if (check_addr_free(vm_area, addr, length)) {
            // use addr
            return create_vma(vm_area, addr, length, prot, flags);

        }
    }

    if (flags == MAP_FIXED) {
        return -EINVAL;
    }

    // use lowest available address
    addr = get_lowest_free_addr(vm_area, length);
    return create_vma(vm_area, addr, length, prot, flags);
}

/**
 * munmap system call implemenations
 */
long vm_area_unmap(struct exec_context *current, u64 addr, int length) {
    // check validity of arguments
    if (!current) {
        return -EINVAL;
    }
    if (length <= 0) {
        return -EINVAL;
    }

    // unmap at page granularity
    length = length % PAGE_SIZE ? length + PAGE_SIZE - (length % PAGE_SIZE) : length;

    // create dummy node if not present
    if (!current->vm_area) {
        if (create_dummy_vma(current) < 0) {
            return -EINVAL;
        }
    }

    // delete mappings
    return delete_vmas(current->vm_area, addr, length);
}

/**
 * Function will invoked whenever there is page fault for an address in the vm area region
 * created using mmap
 */
long vm_area_pagefault(struct exec_context *current, u64 addr, int error_code) {
    if (!current) {
        return -1;
    }

    // check access validity
    struct vm_area *vm_area = current->vm_area;
    u32 access_flags = error_code & PAGE_FAULT_WRITE ? PROT_WRITE : PROT_READ;
    if (!check_access_valid(vm_area, addr, access_flags)) {
        return -1;
    }

    // check CoW access
    if (error_code & PAGE_FAULT_PRESENT) {
        return handle_cow_fault(current, addr, PROT_READ | PROT_WRITE);
    }

    // page walk
    u64 *pgd_addr = osmap(current->pgd);
    int pgd_offset = (addr >> 39) & 0x1FF;
    u64 pgd_t = pgd_addr[pgd_offset];
    if (!(pgd_t & PTE_PRESENT)) {
        // allocate pud
        u64 pud_pfn = os_pfn_alloc(OS_PT_REG);
        if (!pud_pfn) {
            return -1;
        }

        pgd_t = (pud_pfn << 12) | PTE_PRESENT | PTE_WRITE | PTE_USER;
        pgd_addr[pgd_offset] = pgd_t;
    }

    u64 *pud_addr = osmap(pgd_t >> 12);
    int pud_offset = (addr >> 30) & 0x1FF;
    u64 pud_t = pud_addr[pud_offset];
    if (!(pud_t & PTE_PRESENT)) {
        // allocate pmd
        u64 pmd_pfn = os_pfn_alloc(OS_PT_REG);
        if (!pmd_pfn) {
            return -1;
        }

        pud_t = (pmd_pfn << 12) | PTE_PRESENT | PTE_WRITE | PTE_USER;
        pud_addr[pud_offset] = pud_t;
    }

    u64 *pmd_addr = osmap(pud_t >> 12);
    int pmd_offset = (addr >> 21) & 0x1FF;
    u64 pmd_t = pmd_addr[pmd_offset];
    if (!(pmd_t & PTE_PRESENT)) {
        // allocate pte
        u64 pte_pfn = os_pfn_alloc(OS_PT_REG);
        if (!pte_pfn) {
            return -1;
        }

        pmd_t = (pte_pfn << 12) | PTE_PRESENT | PTE_WRITE | PTE_USER;
        pmd_addr[pmd_offset] = pmd_t;
    }

    u64 *pte_addr = osmap(pmd_t >> 12);
    int pte_offset = (addr >> 12) & 0x1FF;
    u64 pte_t = pte_addr[pte_offset];
    if (!(pte_t & PTE_PRESENT)) {
        // allocate page
        u64 page_pfn = os_pfn_alloc(USER_REG);
        if (!page_pfn) {
            return -1;
        }

        u64 vma_access_flags = get_vma_access_flags(vm_area, addr);
        pte_t = (page_pfn << 12) | PTE_PRESENT | PTE_USER;
        pte_t = vma_access_flags & PROT_WRITE ? pte_t | PTE_WRITE : pte_t;
        pte_addr[pte_offset] = pte_t;
    }

    return 1;
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
