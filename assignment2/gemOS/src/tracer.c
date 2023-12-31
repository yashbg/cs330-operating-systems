#include <context.h>
#include <memory.h>
#include <lib.h>
#include <entry.h>
#include <file.h>
#include <tracer.h>

///////////////////////////////////////////////////////////////////////////
////        Start of Trace buffer functionality               /////
///////////////////////////////////////////////////////////////////////////

int is_valid_mem_range(unsigned long buff, u32 count, int access_bit) {
    struct exec_context *ctx = get_current_ctx();
    struct mm_segment *mms = ctx->mms;
    struct vm_area *vm_area = ctx->vm_area;

    if (buff >= mms[MM_SEG_CODE].start && buff + count <= mms[MM_SEG_CODE].next_free - 1) {
        return mms[MM_SEG_CODE].access_flags & access_bit;
    }

    else if (buff >= mms[MM_SEG_RODATA].start && buff + count <= mms[MM_SEG_RODATA].next_free - 1) {
        return mms[MM_SEG_RODATA].access_flags & access_bit;
    }

    else if (buff >= mms[MM_SEG_DATA].start && buff + count <= mms[MM_SEG_DATA].next_free - 1) {
        return mms[MM_SEG_DATA].access_flags & access_bit;
    }

    else if (buff >= mms[MM_SEG_STACK].start && buff + count <= mms[MM_SEG_STACK].end - 1) {
        return mms[MM_SEG_STACK].access_flags & access_bit;
    }

    else {
        struct vm_area *cur = vm_area;
        while (cur) {
            if (buff >= cur->vm_start && buff + count <= cur->vm_end - 1) {
                return cur->access_flags & access_bit;
            }

            cur = cur->vm_next;
        }
    }

    return 0;
}

long trace_buffer_close(struct file *filep) {
    if (!filep || filep->type != TRACE_BUFFER) {
        return -EINVAL;
    }

    if (!filep->trace_buffer || !filep->trace_buffer->buf || !filep->fops) {
        return -EINVAL;
    }

    os_page_free(USER_REG, filep->trace_buffer->buf);
    os_free(filep->trace_buffer, sizeof(struct trace_buffer_info));
    os_free(filep->fops, sizeof(struct fileops));
    os_free(filep, sizeof(struct file));
    filep = NULL;

    return 0;
}

int trace_buffer_read(struct file *filep, char *buff, u32 count) {
    if (!filep || filep->type != TRACE_BUFFER || !(filep->mode & O_READ)) {
        return -EINVAL;
    }

    if (!buff) {
        return -EINVAL;
    }

    if (!count) {
        return count;
    }

    struct trace_buffer_info *trace_buffer = filep->trace_buffer;
    if (!trace_buffer->is_full && trace_buffer->r_off == trace_buffer->w_off) {
        return 0;
    }

    u32 used;
    if (trace_buffer->w_off >= trace_buffer->r_off) {
        used = trace_buffer->w_off - trace_buffer->r_off;
    }
    else {
        used = TRACE_BUFFER_MAX_SIZE - (trace_buffer->r_off - trace_buffer->w_off);
    }

    if (count > used) {
        count = used;
    }

    if (!is_valid_mem_range((unsigned long)buff, count, MM_WR)) {
        return -EBADMEM;
    }

    if (count > TRACE_BUFFER_MAX_SIZE - trace_buffer->r_off) {
        u32 rem = TRACE_BUFFER_MAX_SIZE - trace_buffer->r_off;
        memcpy(buff, trace_buffer->buf + trace_buffer->r_off, rem);
        memcpy(buff + rem, trace_buffer->buf, count - rem);
        trace_buffer->r_off = count - rem;
    }
    else {
        memcpy(buff, trace_buffer->buf + trace_buffer->r_off, count);
        trace_buffer->r_off += count;
    }

    trace_buffer->is_full = 0;

    return count;
}

int trace_buffer_write(struct file *filep, char *buff, u32 count) {
    if (!filep || filep->type != TRACE_BUFFER || !(filep->mode & O_WRITE)) {
        return -EINVAL;
    }

    if (!buff) {
        return -EINVAL;
    }

    if (!count) {
        return count;
    }
    
    struct trace_buffer_info *trace_buffer = filep->trace_buffer;
    if (trace_buffer->is_full) {
        return 0;
    }

    u32 free;
    if (trace_buffer->w_off >= trace_buffer->r_off) {
        free = TRACE_BUFFER_MAX_SIZE - (trace_buffer->w_off - trace_buffer->r_off);
    }
    else {
        free = trace_buffer->r_off - trace_buffer->w_off;
    }

    if (count > free) {
        count = free;
    }

    if (!is_valid_mem_range((unsigned long)buff, count, MM_RD)) {
        return -EBADMEM;
    }

    if (count > TRACE_BUFFER_MAX_SIZE - trace_buffer->w_off) {
        u32 rem = TRACE_BUFFER_MAX_SIZE - trace_buffer->w_off;
        memcpy(trace_buffer->buf + trace_buffer->w_off, buff, rem);
        memcpy(trace_buffer->buf, buff + rem, count - rem);
        trace_buffer->w_off = count - rem;
    }
    else {
        memcpy(trace_buffer->buf + trace_buffer->w_off, buff, count);
        trace_buffer->w_off += count;
    }

    if (count == free) {
        trace_buffer->is_full = 1;
    }

    return count;
}

int sys_create_trace_buffer(struct exec_context *current, int mode) {
    if (!current) {
        return -EINVAL;
    }

    int fd = -1;
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!current->files[i]) {
            fd = i;
            break;
        }
    }

    if (fd == -1) {
        return -EINVAL;
    }

    current->files[fd] = os_alloc(sizeof(struct file));
    if (!current->files[fd]) {
        return -ENOMEM;
    }

    struct file *filep = current->files[fd];
    filep->type = TRACE_BUFFER;
    filep->mode = mode;
    filep->offp = 0;
    filep->ref_count = 1;
    filep->inode = NULL;

    filep->trace_buffer = os_alloc(sizeof(struct trace_buffer_info));
    if (!filep->trace_buffer) {
        return -ENOMEM;
    }

    struct trace_buffer_info *trace_buffer = filep->trace_buffer;
    trace_buffer->buf = os_page_alloc(USER_REG);
    if (!trace_buffer->buf) {
        return -ENOMEM;
    }

    trace_buffer->r_off = 0;
    trace_buffer->w_off = 0;
    trace_buffer->is_full = 0;

    filep->fops = os_alloc(sizeof(struct fileops));
    if (!filep->fops) {
        return -ENOMEM;
    }

    struct fileops *fops = filep->fops;
    fops->read = &trace_buffer_read;
    fops->write = &trace_buffer_write;
    fops->close = &trace_buffer_close;

    return fd;
}

///////////////////////////////////////////////////////////////////////////
////        Start of strace functionality                     /////
///////////////////////////////////////////////////////////////////////////

int get_num_params(u64 syscall_num) {
    switch (syscall_num) {
        case SYSCALL_GETPID:
        case SYSCALL_GETPPID:
        case SYSCALL_FORK:
        case SYSCALL_CFORK:
        case SYSCALL_VFORK:
        case SYSCALL_PHYS_INFO:
        case SYSCALL_STATS:
        case SYSCALL_GET_USER_P:
        case SYSCALL_GET_COW_F:
        case SYSCALL_END_STRACE:
            return 0;

        case SYSCALL_EXIT:
        case SYSCALL_CONFIGURE:
        case SYSCALL_DUMP_PTT:
        case SYSCALL_SLEEP:
        case SYSCALL_PMAP:
        case SYSCALL_DUP:
        case SYSCALL_CLOSE:
        case SYSCALL_TRACE_BUFFER:
            return 1;
        
        case SYSCALL_SIGNAL:
        case SYSCALL_EXPAND:
        case SYSCALL_CLONE:
        case SYSCALL_MUNMAP:
        case SYSCALL_OPEN:
        case SYSCALL_DUP2:
        case SYSCALL_START_STRACE:
        case SYSCALL_STRACE:
            return 2;
        
        case SYSCALL_MPROTECT:
        case SYSCALL_READ:
        case SYSCALL_WRITE:
        case SYSCALL_LSEEK:
        case SYSCALL_READ_STRACE:
        case SYSCALL_READ_FTRACE:
            return 3;
        
        case SYSCALL_MMAP:
        case SYSCALL_FTRACE:
            return 4;
    }
    
    return -1;
}

void trace_buffer_write_num(struct trace_buffer_info *trace_buffer, u64 num) {
    u32 count = sizeof(u64);
    void *num_ptr = (void *)&num;
    if (count > TRACE_BUFFER_MAX_SIZE - trace_buffer->w_off) {
        u32 rem = TRACE_BUFFER_MAX_SIZE - trace_buffer->w_off;
        memcpy(trace_buffer->buf + trace_buffer->w_off, num_ptr, rem);
        memcpy(trace_buffer->buf, num_ptr + rem, count - rem);
        trace_buffer->w_off = count - rem;
    }
    else {
        memcpy(trace_buffer->buf + trace_buffer->w_off, num_ptr, count);
        trace_buffer->w_off += count;
    }
}

int perform_tracing(u64 syscall_num, u64 param1, u64 param2, u64 param3, u64 param4) {
    if (syscall_num == SYSCALL_START_STRACE || syscall_num == SYSCALL_END_STRACE) {
        return 0;
    }

    struct exec_context *ctx = get_current_ctx();
    struct strace_head *strace_head = ctx->st_md_base;
    if (!strace_head) {
        return 0;
    }

    if (!strace_head->is_traced) {
        return 0;
    }

    int strace_fd = strace_head->strace_fd;
    struct file *filep = ctx->files[strace_fd];
    if (!filep || filep->type != TRACE_BUFFER || !(filep->mode & O_WRITE)) {
        return 0;
    }

    struct trace_buffer_info *trace_buffer = filep->trace_buffer;

    if (strace_head->tracing_mode == FULL_TRACING) {
        trace_buffer_write_num(trace_buffer, syscall_num);

        int num_params = get_num_params(syscall_num);
        if (num_params >= 1) {
            trace_buffer_write_num(trace_buffer, param1);
        }
        if (num_params >= 2) {
            trace_buffer_write_num(trace_buffer, param2);
        }
        if (num_params >= 3) {
            trace_buffer_write_num(trace_buffer, param3);
        }
        if (num_params >= 4) {
            trace_buffer_write_num(trace_buffer, param4);
        }

        return 0;
    }

    if (strace_head->tracing_mode == FILTERED_TRACING) {
        struct strace_info *cur = strace_head->next;
        while (cur) {
            if (cur->syscall_num == syscall_num) {
                trace_buffer_write_num(trace_buffer, syscall_num);

                int num_params = get_num_params(syscall_num);
                if (num_params >= 1) {
                    trace_buffer_write_num(trace_buffer, param1);
                }
                if (num_params >= 2) {
                    trace_buffer_write_num(trace_buffer, param2);
                }
                if (num_params >= 3) {
                    trace_buffer_write_num(trace_buffer, param3);
                }
                if (num_params >= 4) {
                    trace_buffer_write_num(trace_buffer, param4);
                }

                return 0;
            }
            
            cur = cur->next;
        }

        return 0;
    }

    return 0;
}

int sys_strace(struct exec_context *current, int syscall_num, int action) {
    if (!current) {
        return -EINVAL;
    }

    if (!current->st_md_base) {
        current->st_md_base = os_alloc(sizeof(struct strace_head));
        if (!current->st_md_base) {
            return -EINVAL;
        }

        struct strace_head *strace_head = current->st_md_base;
        strace_head->count = 0;
        strace_head->is_traced = 0;
        strace_head->strace_fd = -1;
        strace_head->tracing_mode = -1;
        strace_head->next = NULL;
        strace_head->last = NULL;
    }

    struct strace_head *strace_head = current->st_md_base;
    if (action == ADD_STRACE) {
        if (strace_head->count == STRACE_MAX) {
            return -EINVAL;
        }

        struct strace_info *cur = strace_head->next;
        while (cur) {
            if (cur->syscall_num == syscall_num) {
                return -EINVAL;
            }

            cur = cur->next;
        }

        struct strace_info *new = os_alloc(sizeof(struct strace_info));
        if (!new) {
            return -EINVAL;
        }

        new->syscall_num = syscall_num;
        new->next = NULL;

        if (!strace_head->next) {
            strace_head->next = new;
        }
        else {
            strace_head->last->next = new;
        }

        strace_head->last = new;
        strace_head->count++;

        return 0;
    }

    if (action == REMOVE_STRACE) {
        struct strace_info *cur = strace_head->next;
        struct strace_info *prev = NULL;
        while (cur) {
            if (cur->syscall_num == syscall_num) {
                struct strace_info *next = cur->next;
                os_free(cur, sizeof(struct strace_info));
                if (prev) {
                    prev->next = next;
                }
                else {
                    strace_head->next = next;
                }

                if (!next) {
                    strace_head->last = prev;
                }

                strace_head->count--;
                return 0;
            }
            
            prev = cur;
            cur = cur->next;
        }

        return -EINVAL;
    }

    return -EINVAL;
}

void trace_buffer_read_num(struct trace_buffer_info *trace_buffer, void *buf) {
    u32 count = sizeof(u64);
    if (count > TRACE_BUFFER_MAX_SIZE - trace_buffer->r_off) {
        u32 rem = TRACE_BUFFER_MAX_SIZE - trace_buffer->r_off;
        memcpy(buf, trace_buffer->buf + trace_buffer->r_off, rem);
        memcpy(buf + rem, trace_buffer->buf, count - rem);
        trace_buffer->r_off = count - rem;
    }
    else {
        memcpy(buf, trace_buffer->buf + trace_buffer->r_off, count);
        trace_buffer->r_off += count;
    }
}

u64 trace_buffer_get_num(struct trace_buffer_info *trace_buffer) {
    u64 num;
    void *num_ptr = (void *)&num;
    u32 count = sizeof(u64);
    if (count > TRACE_BUFFER_MAX_SIZE - trace_buffer->r_off) {
        u32 rem = TRACE_BUFFER_MAX_SIZE - trace_buffer->r_off;
        memcpy(num_ptr, trace_buffer->buf + trace_buffer->r_off, rem);
        memcpy(num_ptr + rem, trace_buffer->buf, count - rem);
        trace_buffer->r_off = count - rem;
    }
    else {
        memcpy(num_ptr, trace_buffer->buf + trace_buffer->r_off, count);
        trace_buffer->r_off += count;
    }

    return num;
}

int sys_read_strace(struct file *filep, char *buff, u64 count) {
    if (!filep || filep->type != TRACE_BUFFER || !(filep->mode & O_READ)) {
        return -EINVAL;
    }

    if (!buff) {
        return -EINVAL;
    }

    if (!count) {
        return 0;
    }

    struct trace_buffer_info *trace_buffer = filep->trace_buffer;
    int used;
    if (trace_buffer->w_off >= trace_buffer->r_off) {
        used = trace_buffer->w_off - trace_buffer->r_off;
    }
    else {
        used = TRACE_BUFFER_MAX_SIZE - (trace_buffer->r_off - trace_buffer->w_off);
    }

    int bytes_read = 0;
    trace_buffer->is_full = 0;
    int size = sizeof(u64);
    for (u64 i = 0; i < count; i++) {
        if (used < size) {
            return bytes_read;
        }

        u64 syscall_num = trace_buffer_get_num(trace_buffer);
        memcpy(buff + bytes_read, (void *)&syscall_num, size);
        bytes_read += size;
        used -= size;

        int num_params = get_num_params(syscall_num);
        for (int j = 0; j < num_params; j++) {
            if (used < size) {
                return bytes_read;
            }

            trace_buffer_read_num(trace_buffer, buff + bytes_read);
            bytes_read += size;
            used -= size;
        }
    }

    return bytes_read;
}

int sys_start_strace(struct exec_context *current, int fd, int tracing_mode) {
    if (!current) {
        return -EINVAL;
    }

    if (tracing_mode != FULL_TRACING && tracing_mode != FILTERED_TRACING) {
        return -EINVAL;
    }

    if (current->st_md_base && current->st_md_base->is_traced) {
        return -EINVAL;
    }

    if (!current->st_md_base) {
        current->st_md_base = os_alloc(sizeof(struct strace_head));
        if (!current->st_md_base) {
            return -EINVAL;
        }

        struct strace_head *strace_head = current->st_md_base;
        strace_head->count = 0;
        strace_head->next = NULL;
        strace_head->last = NULL;
    }

    struct strace_head *strace_head = current->st_md_base;
    strace_head->is_traced = 1;
    strace_head->strace_fd = fd;
    strace_head->tracing_mode = tracing_mode;

    return 0;
}

int sys_end_strace(struct exec_context *current) {
    if (!current) {
        return -EINVAL;
    }

    struct strace_head *strace_head = current->st_md_base;
    if (!strace_head) {
        return -EINVAL;
    }

    if (!strace_head->is_traced) {
        return -EINVAL;
    }

    struct strace_info *cur = strace_head->next;
    while (cur) {
        struct strace_info *next = cur->next;
        os_free(cur, sizeof(struct strace_info));
        cur = next;
    }

    os_free(strace_head, sizeof(struct strace_head));
    current->st_md_base = NULL;

    return 0;
}

///////////////////////////////////////////////////////////////////////////
////        Start of ftrace functionality                     /////
///////////////////////////////////////////////////////////////////////////

struct ret_addr_info {
    u64 ret_addr;
    struct ret_addr_info *next;
};

long do_ftrace(struct exec_context *ctx, unsigned long faddr, long action, long nargs, int fd_trace_buffer) {
    if (!ctx) {
        return -EINVAL;
    }

    if (!ctx->ft_md_base) {
        ctx->ft_md_base = os_alloc(sizeof(struct ftrace_head));
        if (!ctx->ft_md_base) {
            return -EINVAL;
        }

        struct ftrace_head *ftrace_head = ctx->ft_md_base;
        ftrace_head->count = 0;
        ftrace_head->next = NULL;
        ftrace_head->last = NULL;
    }

    struct ftrace_head *ftrace_head = ctx->ft_md_base;
    if (action == ADD_FTRACE) {
        if (ftrace_head->count == FTRACE_MAX) {
            return -EINVAL;
        }

        struct ftrace_info *cur = ftrace_head->next;
        while (cur) {
            if (cur->faddr == faddr) {
                return -EINVAL;
            }

            cur = cur->next;
        }

        struct ftrace_info *new = os_alloc(sizeof(struct ftrace_info));
        if (!new) {
            return -EINVAL;
        }

        new->faddr = faddr;
        new->num_args = nargs;
        new->fd = fd_trace_buffer;
        new->capture_backtrace = 0;
        new->next = NULL;
        for (int i = 0; i < 4; i++) {
            new->code_backup[i] = 0;
        }

        if (!ftrace_head->next) {
            ftrace_head->next = new;
        }
        else {
            ftrace_head->last->next = new;
        }

        ftrace_head->last = new;
        ftrace_head->count++;

        return 0;
    }

    if (action == REMOVE_FTRACE) {
        struct ftrace_info *cur = ftrace_head->next;
        struct ftrace_info *prev = NULL;
        while (cur) {
            if (cur->faddr == faddr) {
                u8 *code = (u8 *)faddr;
                if (code[0] == INV_OPCODE) {
                    for (int i = 0; i < 4; i++) {
                        code[i] = cur->code_backup[i];
                        cur->code_backup[i] = 0;
                    }
                }

                struct ftrace_info *next = cur->next;
                os_free(cur, sizeof(struct ftrace_info));
                if (prev) {
                    prev->next = next;
                }
                else {
                    ftrace_head->next = next;
                }

                if (!next) {
                    ftrace_head->last = prev;
                }

                ftrace_head->count--;
                return 0;
            }
            
            prev = cur;
            cur = cur->next;
        }

        return -EINVAL;
    }

    if (action == ENABLE_FTRACE) {
        struct ftrace_info *cur = ftrace_head->next;
        while (cur) {
            if (cur->faddr == faddr) {
                u8 *code = (u8 *)faddr;
                if (code[0] == INV_OPCODE) {
                    return 0;
                }

                // code: 0x55 0x48 0x89 0xE5
                for (int i = 0; i < 4; i++) {
                    cur->code_backup[i] = code[i];
                    code[i] = INV_OPCODE;
                }

                return 0;
            }

            cur = cur->next;
        }

        return -EINVAL;
    }

    if (action == DISABLE_FTRACE) {
        struct ftrace_info *cur = ftrace_head->next;
        while (cur) {
            if (cur->faddr == faddr) {
                u8 *code = (u8 *)faddr;
                if (code[0] != INV_OPCODE) {
                    return 0;
                }

                for (int i = 0; i < 4; i++) {
                    code[i] = cur->code_backup[i];
                    cur->code_backup[i] = 0;
                }

                return 0;
            }

            cur = cur->next;
        }

        return -EINVAL;
    }

    if (action == ENABLE_BACKTRACE) {
        struct ftrace_info *cur = ftrace_head->next;
        while (cur) {
            if (cur->faddr == faddr) {
                u8 *code = (u8 *)faddr;
                if (code[0] != INV_OPCODE) {
                    for (int i = 0; i < 4; i++) {
                        cur->code_backup[i] = code[i];
                        code[i] = INV_OPCODE;
                    }
                }

                cur->capture_backtrace = 1;
                return 0;
            }

            cur = cur->next;
        }

        return -EINVAL;
    }

    if (action == DISABLE_BACKTRACE) {
        struct ftrace_info *cur = ftrace_head->next;
        while (cur) {
            if (cur->faddr == faddr) {
                u8 *code = (u8 *)faddr;
                if (code[0] == INV_OPCODE) {
                    for (int i = 0; i < 4; i++) {
                        code[i] = cur->code_backup[i];
                        cur->code_backup[i] = 0;
                    }
                }

                cur->capture_backtrace = 0;
                return 0;
            }

            cur = cur->next;
        }

        return -EINVAL;
    }

    return -EINVAL;
}

// Fault handler
long handle_ftrace_fault(struct user_regs *regs) {
    u64 faddr = regs->entry_rip;

    // code_backup: 0x55 0x48 0x89 0xE5
	regs->entry_rsp -= 8;
	*((u64 *)(regs->entry_rsp)) = regs->rbp;
	regs->rbp = regs->entry_rsp;
	regs->entry_rip += 4;

    struct exec_context *ctx = get_current_ctx();
    struct ftrace_head *ftrace_head = ctx->ft_md_base;
    if (!ftrace_head) {
        return -EINVAL;
    }
    
    struct ftrace_info *cur = ftrace_head->next;
    while (cur) {
        if (cur->faddr == faddr) {
            int fd = cur->fd;
            struct file *filep = ctx->files[fd];
            if (!filep || filep->type != TRACE_BUFFER || !(filep->mode & O_WRITE)) {
                return -EINVAL;
            }

            struct ret_addr_info *ret_addr_head;
            u64 bt_count = 0;
            if (cur->capture_backtrace) {
                bt_count = 2;
                u64 rbp = regs->rbp;
                u64 ret_addr = *((u64 *)rbp + 1);

                ret_addr_head = os_alloc(sizeof(struct ret_addr_info));
                ret_addr_head->ret_addr = ret_addr;
                ret_addr_head->next = NULL;

                struct ret_addr_info *ret_addr_cur = ret_addr_head;
                while (ret_addr != END_ADDR) {
                    rbp = *((u64 *)rbp);
                    ret_addr = *((u64 *)rbp + 1);
                    if (ret_addr == END_ADDR) {
                        break;
                    }

                    ret_addr_cur->next = os_alloc(sizeof(struct ret_addr_info));
                    ret_addr_cur = ret_addr_cur->next;
                    ret_addr_cur->ret_addr = ret_addr;
                    ret_addr_cur->next = NULL;

                    bt_count++;
                }
            }

            struct trace_buffer_info *trace_buffer = filep->trace_buffer;
            u64 nargs = cur->num_args;

            trace_buffer_write_num(trace_buffer, 1 + nargs + bt_count);
            trace_buffer_write_num(trace_buffer, faddr);
            if (nargs >= 1) {
                trace_buffer_write_num(trace_buffer, regs->rdi);
            }
            if (nargs >= 2) {
                trace_buffer_write_num(trace_buffer, regs->rsi);
            }
            if (nargs >= 3) {
                trace_buffer_write_num(trace_buffer, regs->rdx);
            }
            if (nargs >= 4) {
                trace_buffer_write_num(trace_buffer, regs->rcx);
            }
            if (nargs >= 5) {
                trace_buffer_write_num(trace_buffer, regs->r8);
            }
            if (cur->capture_backtrace) {
                trace_buffer_write_num(trace_buffer, faddr);
                
                struct ret_addr_info *ret_addr_cur = ret_addr_head;
                while (ret_addr_cur) {
                    trace_buffer_write_num(trace_buffer, ret_addr_cur->ret_addr);
                    ret_addr_cur = ret_addr_cur->next;
                }
            }

            struct ret_addr_info *ret_addr_cur = ret_addr_head;
            while (ret_addr_cur) {
                struct ret_addr_info *next = ret_addr_cur->next;
                os_free(ret_addr_cur, sizeof(struct ret_addr_info));
                ret_addr_cur = next;
            }
            
            return 0;
        }

        cur = cur->next;
    }

    return -EINVAL;
}

int sys_read_ftrace(struct file *filep, char *buff, u64 count) {
    if (!filep || filep->type != TRACE_BUFFER || !(filep->mode & O_READ)) {
        return -EINVAL;
    }

    if (!buff) {
        return -EINVAL;
    }

    if (!count) {
        return 0;
    }

    struct trace_buffer_info *trace_buffer = filep->trace_buffer;
    int used;
    if (trace_buffer->w_off >= trace_buffer->r_off) {
        used = trace_buffer->w_off - trace_buffer->r_off;
    }
    else {
        used = TRACE_BUFFER_MAX_SIZE - (trace_buffer->r_off - trace_buffer->w_off);
    }

    int bytes_read = 0;
    trace_buffer->is_full = 0;
    int size = sizeof(u64);
    for (u64 i = 0; i < count; i++) {
        if (used < size) {
            return bytes_read;
        }

        u64 info_count = trace_buffer_get_num(trace_buffer);
        for (u64 j = 0; j < info_count; j++) {
            if (used < size) {
                return bytes_read;
            }

            trace_buffer_read_num(trace_buffer, buff + bytes_read);
            bytes_read += size;
            used -= size;
        }
    }

    return bytes_read;
}
