:min_caml_float_of_int
	ADDI	r15	r0	31
	ADDI	r14	r0	1
	ADDI	r13	r0	32
	ADDI	r12	r0	8
	ADDI	r11	r0	23
	SHR	r4	r3	r15			#r4 = sig(a)
	SHL	r4	r4	r15
	BLE	r0	r4	:min_caml_float_of_int_L1
	SUB	r3	r0	r3			#負の数なら反転
:min_caml_float_of_int_L1
	ADDI	r5	r0	30
:min_caml_float_of_int_L2					#r5 = exp(f)
	SHR	r6	r3	r5
	BLT	r0	r6	:min_caml_float_of_int_L3
	SUB	r5	r5	r14
	BLE	r0	r5	:min_caml_float_of_int_L2
	ADDI	r3	r0	0			#0は0に
	RET
:min_caml_float_of_int_L3
	SUB	r6	r13	r5
	SHL	r3	r3	r6
	SHR	r3	r3	r12
	ADDI	r3	r3	1
	SHR	r3	r3	r14
	ADDI	r5	r5	127
	SHL	r5	r5	r11
	ADD	r3	r3	r5
	OR	r3	r3	r4
	RET

:min_caml_ftoi
	ADDI	r15	r0	31
	ADDI	r14	r0	1
	ADDI	r13	r0	24
	ADDI	r12	r0	9
	ADDI	r11	r0	150
	ADDI	r10	r0	149
	ADDI	r9	r0	158
	ADDI	r8	r0	23
	SHR	r5	r3	r15			#r5 = sig(a)
	SHL	r4	r3	r14			#r4 = exp(a)
	SHR	r4	r4	r13
	SHL	r6	r14	r8			#24bit目の1を補完
	SHL	r3	r3	r12			#r3 = man(a)
	SHR	r3	r3	r12
	OR	r3	r6	r3			#ここまでok
	BLE	r11	r4	:min_caml_int_of_float_L1	#73
	SUB	r4	r10	r4
	SHR	r3	r3	r4
	ADDI	r3	r3	1
	SHR	r3	r3	r14
	BEQ	r5	r0	:min_caml_int_of_float_L2
	SUB	r3	r0	r3
:min_caml_int_of_float_L2
	RET
:min_caml_int_of_float_L1
	BLE	r9	r4	:min_caml_int_of_float_L3
	SUB	r4	r4	r11
	SHL	r3	r3	r4
	BEQ	r5	r0	:min_caml_int_of_float_L4
	SUB	r3	r0	r3
:min_caml_int_of_float_L4
	RET
:min_caml_int_of_float_L3
	ADDI	r4	r0	16
	BEQ	r5	r0	:min_caml_int_of_float_L5
	ADDI	r3	r0	0x8000
	SHL	r3	r3	r4
	RET
:min_caml_int_of_float_L5
	ADDI	r3	r0	0x7fff
	SHL	r3	r3	r4
	ADDI	r4	r0	0xffff
	OR	r3	r4	r3
	RET

:min_caml_floor
	ADDI	r15	r0	31
	ADDI	r14	r0	1
	ADDI	r13	r0	24
	ADDI	r12	r0	150
	ADDI	r11	r0	127
	ADDI	r10	r0	32
	SHR	r5	r3	r15
	SHL	r4	r3	r14
	SHR	r4	r4	r13			#modified
	BLT	r4	r12	:min_caml_floor_L1
	RET
:min_caml_floor_L1
	BLE	r11	r4	:min_caml_floor_L2
	BEQ	r5	r0	:min_caml_floor_L4
	ADDI	r4	r0	16
	ADDI	r3	r0	0xbf80
	SHL	r3	r3	r4
	RET						#-1を返す
:min_caml_floor_L4
	ADDI	r3	r0	0			#0を返す
	RET
:min_caml_floor_L2
	SUB	r6	r12	r4
	SHR	r4	r3	r6
	BEQ	r5	r0	:min_caml_floor_L3
	SUB	r9	r10	r6
	SHL	r5	r3	r9
	BEQ	r5	r0	:min_caml_floor_L3
	ADDI	r4	r4	1
:min_caml_floor_L3
	SHL	r3	r4	r6
	RET
