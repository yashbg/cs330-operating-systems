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

    struct file *file = current->files[fd];
    file->type = TRACE_BUFFER;
    file->mode = mode;
    file->offp = 0;
    file->ref_count = 1;
    file->inode = NULL;

    file->trace_buffer = os_alloc(sizeof(struct trace_buffer_info));
    if (!file->trace_buffer) {
        return -ENOMEM;
    }

    struct trace_buffer_info *trace_buffer = file->trace_buffer;
    trace_buffer->buf = os_page_alloc(USER_REG);
    if (!trace_buffer->buf) {
        return -ENOMEM;
    }

    trace_buffer->r_off = 0;
    trace_buffer->w_off = 0;
    trace_buffer->is_full = 0;

    file->fops = os_alloc(sizeof(struct fileops));
    if (!file->fops) {
        return -ENOMEM;
    }

    struct fileops *fops = file->fops;
    fops->read = &trace_buffer_read;
    fops->write = &trace_buffer_write;
    fops->close = &trace_buffer_close;

    return fd;
}

///////////////////////////////////////////////////////////////////////////
////        Start of strace functionality                     /////
///////////////////////////////////////////////////////////////////////////

int perform_tracing(u64 syscall_num, u64 param1, u64 param2, u64 param3, u64 param4) {
    return 0;
}

int sys_strace(struct exec_context *current, int syscall_num, int action) {
    return 0;
}

int sys_read_strace(struct file *filep, char *buff, u64 count) {
    return 0;
}

int sys_start_strace(struct exec_context *current, int fd, int tracing_mode) {
    if (!current) {
        return -EINVAL;
    }

    if (tracing_mode != FULL_TRACING && tracing_mode != FILTERED_TRACING) {
        return -EINVAL;
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
    strace_head->is_traced = 0;
    strace_head->strace_fd = -1;
    strace_head->tracing_mode = -1;

    struct strace_info *cur = strace_head->next;
    while (cur) {
        struct strace_info *next = cur->next;
        os_free(cur, sizeof(struct strace_info));
        cur = next;
    }

    strace_head->next = NULL;
    strace_head->last = NULL;
    strace_head->count = 0;

    // TODO: clear trace buffer

    return 0;
}

///////////////////////////////////////////////////////////////////////////
////        Start of ftrace functionality                     /////
///////////////////////////////////////////////////////////////////////////

long do_ftrace(struct exec_context *ctx, unsigned long faddr, long action, long nargs, int fd_trace_buffer) {
    return 0;
}

// Fault handler
long handle_ftrace_fault(struct user_regs *regs) {
    return 0;
}

int sys_read_ftrace(struct file *filep, char *buff, u64 count) {
    return 0;
}
