	ADDI	r1	r0	0x42
	ADDI	r2	r0	1
	ADD	r6	r1	r2
	ST	r1	r0	-1
	ST	r6	r0	-1
	ST	r6	r0	-1
	ADDI	r3	r2	0x1
	ADD	r4	r3	r1
	ST	r4	r0	-1
	ADDI	r1	r0	3
	SUB	r7	r4	r1
	ST	r7	r0	-1