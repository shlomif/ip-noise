# 8 "e_log.S"
# 1 "machine/asm.h" 1




# 1 "sysdep.h" 1
# 24 "sysdep.h"
# 1 "sysdeps/unix/i386/sysdep.h" 1
# 19 "sysdeps/unix/i386/sysdep.h"
# 1 "sysdeps/unix/sysdep.h" 1
# 19 "sysdeps/unix/sysdep.h"
# 1 "sysdeps/generic/sysdep.h" 1
# 20 "sysdeps/unix/sysdep.h" 2

# 1 "/usr/include/sys/syscall.h" 1 3
# 25 "/usr/include/sys/syscall.h" 3
# 1 "/usr/include/asm/unistd.h" 1 3
# 26 "/usr/include/sys/syscall.h" 2 3






# 1 "/usr/include/bits/syscall.h" 1 3
# 33 "/usr/include/sys/syscall.h" 2 3
# 22 "sysdeps/unix/sysdep.h" 2
# 20 "sysdeps/unix/i386/sysdep.h" 2
# 1 "sysdeps/i386/sysdep.h" 1
# 20 "sysdeps/i386/sysdep.h"
# 1 "sysdeps/generic/sysdep.h" 1
# 21 "sysdeps/i386/sysdep.h" 2
# 21 "sysdeps/unix/i386/sysdep.h" 2
# 25 "sysdep.h" 2
# 1 "bp-sym.h" 1
# 26 "sysdep.h" 2
# 1 "bp-asm.h" 1
# 27 "sysdep.h" 2
# 6 "machine/asm.h" 2
# 1 "asm-syntax.h" 1
# 7 "machine/asm.h" 2
# 9 "e_log.S" 2
# 22 "e_log.S"
        .section .rodata



        .align 4

one: .double 1.0





limit: .double 0.29
# 44 "e_log.S"
        .text
1: .stabs "",100,0,0,1b; 
1: .stabs "__ieee754_log",100,0,0,1b; 
.globl ___ieee754_log; 
.align 4; 
.stabs "int:t(0,1)=r(0,1);-2147483648;2147483647;",128,0,0,0; 
.stabs "__ieee754_log:F(0,1)",36,0,0,___ieee754_log; 
___ieee754_log:
        fldln2
        fldl 4(%esp)





        fld %st
        fsubl one
        fld %st
        fabs
        fcompl limit
        fnstsw
        andb $0x45, %ah
        jz 2f
        fstp %st(1)
        fyl2xp1
        ret

2: fstp %st(0)
        fyl2x
        ret
1: .stabs "",36,0,0,1b-___ieee754_log; 
