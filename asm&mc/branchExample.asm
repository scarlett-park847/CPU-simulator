	mov	1	[0+eight]	load reg1 with 5 (uses symbolic address) (movl)
	mov	2       [1+0]		load reg2 with -1 (uses literal offset) (movl)
start	add	1	2		decrement reg1
	je	2	 		compare Reg1 to Reg2 and goto end of program when cmp flag is 1/ I changed it to (beq 0 1 2) in mc code
	je	start			go back to the beginning of the loop/ I changed it to (beq 0 0 start )in mc code
	noop
done	halt				end of program
eight	dd	8
neg1	dd	-1
stAddr	dd	start			will contain the address of start (2)