
;p1��8������ƣ�����λָ��

	org	0000h
	ajmp	main
	org	0080h
main:
	mov	p1,#00h		;ȫ��
	lcall	delay		;
	lcall	delay		;
	lcall	delay		;��ʱһ��ʱ��
	mov	a,#0feh		;ÿ��ֻ��һ���ơ�
loop:	mov	p1,a		;
	lcall	delay		;
	lcall	delay		;
	lcall	delay		;
	rl	a		;ѭ�����ơ�
	ajmp	loop		;

delay:	mov	r7,#00h		;��ʱ�ӳ���
tt:	mov	r6,#00h
	djnz	r6,$
	djnz	r7,tt
	ret
	end
