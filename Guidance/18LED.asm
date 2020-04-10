
;p1口8个跑马灯，用移位指令

	org	0000h
	ajmp	main
	org	0080h
main:
	mov	p1,#00h		;全亮
	lcall	delay		;
	lcall	delay		;
	lcall	delay		;延时一段时间
	mov	a,#0feh		;每次只亮一个灯。
loop:	mov	p1,a		;
	lcall	delay		;
	lcall	delay		;
	lcall	delay		;
	rl	a		;循环左移。
	ajmp	loop		;

delay:	mov	r7,#00h		;延时子程序
tt:	mov	r6,#00h
	djnz	r6,$
	djnz	r7,tt
	ret
	end
