platform: Mac OSX
file_in, file_out: absolute pathname

1. caller(line 91-94)
	move $a0, $s1
	move $a1, $s2
	jal RecurFunc
	move $s4, $v0

2. callie(line 209-230)
RecurFunc:
	bgt $a0, 1, .recurse
	move $v0, $a1
	jr  $ra
.recurse:
	sub $sp, $sp, 8
	sw  $ra, 0($sp)
	sw  $a0, 4($sp)
	li  $t0, 2
	div $a0, $t0
	mflo $a0
	jal .RecurFunc
	lw  $a0, 4($sp)
	mult $a1, $a0
	mflo $t1
	mult $v0, $t0
	mflo $v0
	add  $v0, $v0, $t1

	lw   $ra, 0($sp)
	addi $sp, $sp, 8
	jr   $ra
