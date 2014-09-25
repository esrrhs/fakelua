.code
	CallNativeFunc proc 
	
	PUSH	RBP
	MOV		RBP,RSP

	; 函数指针在RCX

	; 塞上参数
	MOV		EAX, 1234

	LEAVE
	RET 
	CallNativeFunc endp
end