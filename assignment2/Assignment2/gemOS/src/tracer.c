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
    return 0;
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

    current->files[fd]->type = TRACE_BUFFER;
    current->files[fd]->mode = mode;
    current->files[fd]->offp = 0;
    current->files[fd]->ref_count = 1;
    current->files[fd]->inode = NULL;

    current->files[fd]->trace_buffer = os_alloc(sizeof(struct trace_buffer_info));
    if (!current->files[fd]->trace_buffer) {
        return -ENOMEM;
    }

    current->files[fd]->trace_buffer->buf = os_page_alloc(USER_REG);

    current->files[fd]->fops = os_alloc(sizeof(struct fileops));
    if (!current->files[fd]->fops) {
        return -ENOMEM;
    }

    current->files[fd]->fops->read = &trace_buffer_read;
    current->files[fd]->fops->write = &trace_buffer_write;
    current->files[fd]->fops->close = &trace_buffer_close;

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
