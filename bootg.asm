MBOOT_ALIGN     equ 1<<0
MBOOT_MEMINFO   equ 1<<1
MBOOT_VIDEO     equ 1<<2        ; Add video mode flag
MBOOT_MAGIC     equ 0x1BADB002
MBOOT_FLAGS     equ MBOOT_ALIGN | MBOOT_MEMINFO | MBOOT_VIDEO  ; Include video flag
MBOOT_CHECKSUM  equ -(MBOOT_MAGIC + MBOOT_FLAGS)

section .multiboot
align 4
    dd MBOOT_MAGIC
    dd MBOOT_FLAGS
    dd MBOOT_CHECKSUM
    
    dd 0  ; header_addr (unused for ELF)
    dd 0  ; load_addr
    dd 0  ; load_end_addr
    dd 0  ; bss_end_addr
    dd 0  ; entry_addr
    
    ; Video mode specification
    dd 0      ; mode_type: 0 = linear framebuffer, 1 = text mode
    dd 1024   ; width (can be 0 for no preference)
    dd 768    ; height (can be 0 for no preference)
    dd 32     ; depth in bits per pixel (can be 0 for no preference)

section .bss
align 16
stack_bottom:
    resb 131072  ; 128 KB stack
stack_top:

section .text
global _start
extern kmaing
_start:
    mov esp, stack_top
    mov ebp, esp  ; Set up stack pointer
    push ebx            ; Push multiboot info structure pointer
    push eax            ; Push multiboot magic number
    call kmaing         
    
    cli                 ; Disable interrupts
.hang:
    hlt                 ; Halt the CPU
    jmp .hang
