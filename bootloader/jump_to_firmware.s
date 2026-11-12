
.syntax unified
.section .text.jump_to_firmware
.align 2
.thumb
.global jump_to_firmware
.type jump_to_firmware, %function
.thumb_func
jump_to_firmware:
  msr msp, r0
  bx r1
.size jump_to_firmware, .-jump_to_firmware
