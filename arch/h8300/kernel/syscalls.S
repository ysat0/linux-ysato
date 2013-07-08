/* Systemcall Entry Table */
#include <linux/sys.h>
#include <asm/linkage.h>
#include <asm/unistd.h>
	
	.global _sys_call_table

#if defined(CONFIG_CPU_H8300H)
	.h8300h
#endif
#if defined(CONFIG_CPU_H8S)
	.h8300s
#endif
	.section .text
	.align	2
_sys_call_table:	
	.long	_sys_ni_syscall
	.long	_sys_exit
	.long	_sys_fork
	.long	_sys_read
	.long	_sys_write
	.long	_sys_open
	.long	_sys_close
	.long	_sys_waitpid
	.long	_sys_creat
	.long	_sys_link
	.long	_sys_unlink	;10
	.long	_sys_execve
	.long	_sys_chdir
	.long	_sys_time
	.long	_sys_mknod
	.long	_sys_chmod
	.long	_sys_chown16
	.long	_sys_ni_syscall
	.long	_sys_stat
	.long	_sys_lseek
	.long	_sys_getpid	;20
	.long	_sys_mount
	.long	_sys_oldumount
	.long	_sys_setuid16
	.long	_sys_getuid16
	.long	_sys_stime
	.long	_sys_ptrace
	.long	_sys_alarm
	.long	_sys_fstat
	.long	_sys_pause
	.long	_sys_utime	;30
	.long	_sys_ni_syscall
	.long	_sys_ni_syscall
	.long	_sys_access
	.long	_sys_nice
	.long	_sys_ni_syscall
	.long	_sys_sync
	.long	_sys_kill
	.long	_sys_rename
	.long	_sys_mkdir
	.long	_sys_rmdir	;40
	.long	_sys_dup
	.long	_sys_pipe
	.long	_sys_times
	.long	_sys_ni_syscall
	.long	_sys_brk
	.long	_sys_setgid16
	.long	_sys_getgid16
	.long	_sys_signal
	.long	_sys_geteuid16
	.long	_sys_getegid16	;50
	.long	_sys_acct
	.long	_sys_umount
	.long	_sys_ni_syscall
	.long	_sys_ioctl
	.long	_sys_fcntl
	.long	_sys_ni_syscall
	.long	_sys_setpgid
	.long	_sys_ni_syscall
	.long	_sys_ni_syscall
	.long	_sys_umask	;60
	.long	_sys_chroot
	.long	_sys_ustat
	.long	_sys_dup2
	.long	_sys_getppid
	.long	_sys_getpgrp
	.long	_sys_setsid
	.long	_sys_sigaction
	.long	_sys_sgetmask
	.long	_sys_ssetmask
	.long	_sys_setreuid16	;70
	.long	_sys_setregid16
	.long	_sys_sigsuspend
	.long	_sys_sigpending
	.long	_sys_sethostname
	.long	_sys_setrlimit
	.long	_sys_old_getrlimit
	.long	_sys_getrusage
	.long	_sys_gettimeofday
	.long	_sys_settimeofday
	.long	_sys_getgroups16 ;80
	.long	_sys_setgroups16
	.long	_sys_old_select
	.long	_sys_symlink
	.long	_sys_lstat
	.long	_sys_readlink
	.long	_sys_uselib
	.long	_sys_swapon
	.long	_sys_reboot
	.long	_sys_old_readdir
	.long	_sys_old_mmap	;90
	.long	_sys_munmap
	.long	_sys_truncate
	.long	_sys_ftruncate
	.long	_sys_fchmod
	.long	_sys_fchown16
	.long	_sys_getpriority
	.long	_sys_setpriority
	.long	_sys_ni_syscall
	.long	_sys_statfs	;100
	.long	_sys_fstatfs
	.long	_sys_ni_syscall
	.long	_sys_socketcall
	.long	_sys_syslog
	.long	_sys_setitimer
	.long	_sys_getitimer
	.long	_sys_newstat
	.long	_sys_newlstat
	.long	_sys_newfstat
	.long	_sys_ni_syscall	;110
	.long	_sys_ni_syscall
	.long	_sys_vhangup
	.long	_sys_ni_syscall
	.long	_sys_ni_syscall
	.long	_sys_wait4
	.long	_sys_swapoff
	.long	_sys_sysinfo
	.long	_sys_ipc
	.long	_sys_fsync
	.long	_sys_sigreturn	;120
	.long	_sys_clone
	.long	_sys_setdomainname
	.long	_sys_newuname
	.long	_sys_cacheflush
	.long	_sys_adjtimex
	.long	_sys_ni_syscall
	.long	_sys_sigprocmask
	.long	_sys_ni_syscall
	.long	_sys_init_module
	.long	_sys_delete_module ;130
	.long	_sys_ni_syscall
	.long	_sys_quotactl
	.long	_sys_getpgid
	.long	_sys_fchdir
	.long	_sys_bdflush
	.long	_sys_sysfs
	.long	_sys_personality
	.long	_sys_ni_syscall
	.long	_sys_setfsuid16
	.long	_sys_setfsgid16	;140
	.long	_sys_llseek
	.long	_sys_getdents
	.long	_sys_select
	.long	_sys_flock
	.long	_sys_ni_syscall
	.long	_sys_readv
	.long	_sys_writev
	.long	_sys_getsid
	.long	_sys_fdatasync
	.long	_sys_sysctl	;150
	.long	_sys_ni_syscall
	.long	_sys_ni_syscall
	.long	_sys_ni_syscall
	.long	_sys_ni_syscall
	.long	_sys_sched_setparam
	.long	_sys_sched_getparam
	.long	_sys_sched_setscheduler
	.long	_sys_sched_getscheduler
	.long	_sys_sched_yield
	.long	_sys_sched_get_priority_max ;160
	.long	_sys_sched_get_priority_min
	.long	_sys_sched_rr_get_interval
	.long	_sys_nanosleep
	.long	_sys_ni_syscall
	.long	_sys_setresuid16
	.long	_sys_getresuid16
	.long	_sys_ni_syscall
	.long	_sys_ni_syscall
	.long	_sys_poll
	.long	_sys_ni_syscall	;170
	.long	_sys_setresgid16
	.long	_sys_getresgid16
	.long	_sys_prctl
	.long	_sys_rt_sigreturn
	.long	_sys_rt_sigaction
	.long	_sys_rt_sigprocmask
	.long	_sys_rt_sigpending
	.long	_sys_rt_sigtimedwait
	.long	_sys_rt_sigqueueinfo
	.long	_sys_rt_sigsuspend ;180
	.long	_sys_pread64
	.long	_sys_pwrite64
	.long	_sys_lchown16;
	.long	_sys_getcwd
	.long	_sys_capget
	.long	_sys_capset
	.long	_sys_sigaltstack
	.long	_sys_sendfile
	.long	_sys_ni_syscall
	.long	_sys_ni_syscall	;190
	.long	_sys_vfork
	.long	_sys_getrlimit
	.long	_sys_mmap_pgoff
	.long	_sys_truncate64
	.long	_sys_ftruncate64
	.long	_sys_stat64
	.long	_sys_lstat64
	.long	_sys_fstat64
	.long	_sys_chown
	.long	_sys_getuid	;200
	.long	_sys_getgid
	.long	_sys_geteuid
	.long	_sys_getegid
	.long	_sys_setreuid
	.long	_sys_setregid
	.long	_sys_getgroups
	.long	_sys_setgroups
	.long	_sys_fchown
	.long	_sys_setresuid
	.long	_sys_getresuid	;210
	.long	_sys_setresgid
	.long	_sys_getresgid
	.long	_sys_lchown
	.long	_sys_setuid
	.long	_sys_setgid
	.long	_sys_setfsuid
	.long	_sys_setfsgid
	.long	_sys_pivot_root
	.long	_sys_ni_syscall	;220
	.long	_sys_ni_syscall
	.long	_sys_getdents64
	.long	_sys_fcntl64
	.long	_sys_ni_syscall
	.long	_sys_ni_syscall
	.long	_sys_gettid
	.long	_sys_readahead
	.long	_sys_setxattr
	.long	_sys_lsetxattr
	.long	_sys_fsetxattr	;230
	.long	_sys_getxattr
	.long	_sys_lgetxattr
	.long	_sys_fgetxattr
	.long	_sys_listxattr
	.long	_sys_llistxattr
	.long	_sys_flistxattr
	.long	_sys_removexattr
	.long	_sys_lremovexattr
	.long	_sys_fremovexattr
	.long	_sys_tkill	;240
	.long	_sys_sendfile64
	.long	_sys_futex
	.long	_sys_sched_setaffinity
	.long	_sys_sched_getaffinity
	.long	_sys_ni_syscall
	.long	_sys_ni_syscall
	.long	_sys_io_setup
	.long	_sys_io_destroy
	.long	_sys_io_getevents
	.long	_sys_io_submit	;250
	.long	_sys_io_cancel
	.long	_sys_fadvise64
	.long	_sys_ni_syscall
	.long	_sys_exit_group
	.long	_sys_lookup_dcookie
	.long	_sys_epoll_create
	.long	_sys_epoll_ctl
	.long	_sys_epoll_wait
	.long	_sys_ni_syscall
	.long	_sys_set_tid_address ;260
	.long	_sys_timer_create
	.long	_sys_timer_settime
	.long	_sys_timer_gettime
	.long	_sys_timer_getoverrun
	.long	_sys_timer_delete
	.long	_sys_clock_settime
	.long	_sys_clock_gettime
	.long	_sys_clock_getres
	.long	_sys_clock_nanosleep
	.long	_sys_statfs64	;270
	.long	_sys_fstatfs64
	.long	_sys_tgkill
	.long	_sys_utimes
	.long	_sys_fadvise64_64
	.long	_sys_ni_syscall
	.long	_sys_ni_syscall
	.long	_sys_get_mempolicy
	.long	_sys_set_mempolicy
	.long	_sys_mq_open
	.long	_sys_mq_unlink	;280
	.long	_sys_mq_timedsend
	.long	_sys_mq_timedreceive
	.long	_sys_mq_notify
	.long	_sys_mq_getsetattr
	.long	_sys_waitid
	.long	_sys_ni_syscall
	.long	_sys_add_key
	.long	_sys_request_key
	.long	_sys_keyctl
	.long	_sys_ioprio_set	;290
	.long	_sys_ioprio_get
	.long	_sys_inotify_init
	.long	_sys_inotify_add_watch
	.long	_sys_inotify_rm_watch
	.long	_sys_migrate_pages
	.long	_sys_openat
	.long	_sys_mkdirat
	.long	_sys_mknodat
	.long	_sys_fchownat
	.long	_sys_futimesat	;300
	.long	_sys_fstatat64
	.long	_sys_unlinkat
	.long	_sys_renameat
	.long	_sys_linkat
	.long	_sys_symlinkat
	.long	_sys_readlinkat
	.long	_sys_fchmodat
	.long	_sys_faccessat
	.long	_sys_ni_syscall	;310
	.long	_sys_ni_syscall
	.long	_sys_unshare
	.long	_sys_set_robust_list
	.long	_sys_get_robust_list
	.long	_sys_splice
	.long	_sys_sync_file_range
	.long	_sys_tee
	.long	_sys_vmsplice
	.long	_sys_ni_syscall
	.long	_sys_getcpu	;320
	.long	_sys_ni_syscall
	.long	_sys_utimensat
	.long	_sys_signalfd
	.long	_sys_timerfd_create
	.long	_sys_eventfd
	.long	_sys_fallocate
	.long	_sys_timerfd_settime
	.long	_sys_timerfd_gettime
	.long	_sys_signalfd4
	.long	_sys_eventfd2	;330
	.long	_sys_epoll_create1
	.long	_sys_dup3
	.long	_sys_pipe2
	.long	_sys_inotify_init1
	.long	_sys_preadv
	.long	_sys_pwritev
	.long	_sys_rt_tgsigqueueinfo
	.long	_sys_perf_event_open
	.long	_sys_recvmmsg	;340
	.long	_sys_fanotify_init
	.long	_sys_fanotify_mark
	.long	_sys_prlimit64
	.long	_sys_name_to_handle_at
	.long	_sys_open_by_handle_at
	.long	_sys_clock_adjtime
	.long	_sys_syncfs
	.long	_sys_sendmmsg
	.long	_sys_setns
