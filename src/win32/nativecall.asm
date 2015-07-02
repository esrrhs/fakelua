
.code

	; 从win32调用过来，采用的rdi rsp方式
	call_native_func proc 

	push	rdi
	mov		rdi,rsp
	push	rsi
	push	rax
	push	rbx
	push	rcx
	push	rdx
	push	r8
	push	r9
	push	r10

	; 函数指针在rcx
	; 参数地址在rdx
	; 参数个数在r8

	; 塞上参数
	mov		rax,0
	mov		r10,rsp
	sub		r10,20h
LOOPBEGIN:
	cmp		rax,r8
	jae		LOOPEND

	; 循环copy参数
	; type
	mov		rbx,qword ptr [rdx]
	mov		qword ptr [r10],rbx
	; data
	mov		rbx,qword ptr [rdx+8h]
	mov		qword ptr [r10+8h],rbx
	
	add		rdx,10h
	sub		r10,10h

	add		rax,1
	jmp		LOOPBEGIN

LOOPEND:
	call    rcx
	
	pop		r10
	pop		r9
	pop		r8
	pop		rdx
	pop		rcx
	pop		rbx
	pop		rax
	pop		rsi
	pop		rdi
	ret 

	call_native_func endp
	
	; 从native调用过来，采用的rbp rsp方式
	call_machine_func proc 
	
	push	rbp
	mov		rbp,rsp
	
	push	rax
	push	rcx
	push	rdx
	push	rdi
	
	mov		rax,rdi
	mov		rdi,rbp

	call    rax
	
	pop		rdi
	pop		rdx
	pop		rcx
	pop		rax
	leave
	ret 

	call_machine_func endp

end

