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
    return 0;
}

long trace_buffer_close(struct file *filep) {
    return 0;   
}

int trace_buffer_read(struct file *filep, char *buff, u32 count) {
    return 0;
}

int trace_buffer_write(struct file *filep, char *buff, u32 count) {
    if (!filep || (filep->type != TRACE_BUFFER) || !(filep->mode & O_WRITE)) {
        return -EINVAL;
    }

    if (!buff) {
        return -EINVAL;
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
    return 0;
}

int sys_end_strace(struct exec_context *current) {
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
