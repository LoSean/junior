Operating System: Mac OSX
file_in, file_out: absolute pathname

1. Initialize "output_ascii" (line 26-31)

	la $t0, output_ascii
	li $t1, 'X'
	sb $t1, 0($t0)
	sb $t1, 1($t0)
	sb $t1, 2($t0)
	sb $t1, 3($t0)

2. Load the operator into $3 (line 81)

	lb $s3 0($3)

3. Decide the operator type and deal with invalid operator (line 91-99)

	li $t0, '+'
	beq $s3, $t0, addition
	li $t0, '-'
	beq $s3, $t0, substraction
	li $t0, '*'
	beq $s3, $t0, multiplication
	li $t0, '/'
	beq $s3, $t0, division
	jr ret           	#invalid operator
	
4. Calculate the outcome and deal with divided by zero (line 101-120)

addition:
	add	$s4, $s1, $s2	# $s4 <= $s1 + $s2
	jr result

substraction:
	sub $s4, $s1, $s2	# $s4 <= $s1 - $s2
	jr result

multiplication:
	mult $s1, $s2		# $s4 <= $s1 * $s2
	mflo $s4
	jr result

division:
	beq $zero, $s2, ret  # divided by zero
	div $s1, $s2
	mflo $s4
	jr result

5. itoa function (line 136-148)

	li $t0, 10 			 # divider
 	la $t1, output_ascii # string address
	addi $t2, $t1, 4

.itoaloop:	
	addi $t2, $t2, -1
	div $a0, $t0
	mfhi $t5 			 # $a0 % $t0
	addi $t5, $t5, '0'
	sb $t5, 0($t2)
	mflo $t5 			 # $a0 / $t0
	move $a0, $t5
	bne $t2, $t1 .itoaloop

