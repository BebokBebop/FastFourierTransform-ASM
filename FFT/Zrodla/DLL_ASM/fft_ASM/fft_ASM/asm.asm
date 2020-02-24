.data  

piMajf real4 3.141592653589793238460
piMinf real4 0.00000008742278
coEf0f real4 -0.10132118
coEf1f real4 0.0066208798
coEf2f real4 -0.00017350505
coEf3f real4 0.0000025222919
coEf4f real4 -0.000000023317787
coEf5f real4 0.00000000013291342

piHalff real4 1.57079632679489661923
negPi2f real4 -6.28318530717958647692
.code


;math explanation

;r - r values array
;i - imaginary values array
;rO - odd elements of r array
;rE - even elements of r array
;iO - odd elements of i array
;iE - even elements of i array

;r[k] = rE[k] + rT
;i[k] = iE[k] + iT
;r[k+N/2] = rE[k] - rT
;i[k+N/2] = iE[k] - iT
;pR = sin(-2 * PI * k / N + PI / 2) ;r part of a polar complex number
;pI = sin(-2 * PI * k / N)          ;imaginary part of a polar complex number
;rT = pR * rO[k] - pI * iO[k]		;r part of temporary complex number
;rT = a - b,
	;a = pR * rO[k]
	;b = pI * iO[k]
;iT = pR * iO[k] - pI * rO[k]		;imaginary part of temporary complex number
;iT = c + d,
	;c = pR * iO[k]
	;d = pI * rO[k]

funcASM_float_v proc
;int N,		rcx			-> rcx
;float* r,	rdx			-> rdx
;float* i,	r8			-> r8
;float* rO,	r9			-> r9
;float* iO,	rsp + 40	-> r10
;float* rE, rsp + 48	-> r11
;float* iE	rsp + 56	-> r12
;float* r + N/2			-> r13
;float* i + N/2			-> r14

	;saving non-volatile registers
	push rbx
	push r12
	push r13
	push r14
	
	;prepare pointers
	mov r10, qword ptr[rsp + 40 + 32] ;iO
	mov r11, qword ptr[rsp + 48 + 32] ;rE
	mov r12, qword ptr[rsp + 56 + 32] ;iE
	mov r13, rcx
	shl r13,1 ;N/2 * 4 = N*2 = N<<1
	mov r14, r13
	add r13, rdx ;r + N/2
	add r14, r8  ;i + N/2

	xor rbx, rbx

LoopLabel:
	mov rax, rcx ;N -> rcx
	shr rax, 1   ;N/2
	cmp rbx, rax ; while(rbx != N/2) { [...] rbx++ }
	je fin

	;float pR = sin(-2 * PI * k / N + PI / 2);
	;float pI = sin(-2 * PI * k / N);
	cvtsi2ss xmm0, rbx ;k
	cvtsi2ss xmm1, rcx ;N
	divss xmm0, xmm1 ;=k/N
	mulss xmm0, negPi2f ;*=-2*PI
	movss xmm4, xmm0
	addss xmm0, piHalff
	;xmm0 = -2 * PI * k / N + PI / 2
	call sineASM_float ;pR -> xmm0
	movss xmm5, xmm0 ;pR -> xmm5

	movss xmm0,xmm4
	call sineASM_float ; pI
	movss xmm2, xmm5
	movss xmm3, xmm0

	;xmm2 - pR
	;xmm3 - pI
	;float* rO,	r9			-> r9
	;float* iO,	rsp + 40	-> r10
	
	movd xmm0, dword ptr[r9] ;rO
	movd xmm1, dword ptr[r10];iO
	movlhps xmm0, xmm1 
	;xmm0 = _,iO,_,rO
	shufps xmm0,xmm0, 00101000b
	;xmm0 = rO,iO,iO,rO
	shufps xmm3, xmm2, 00000000b
	;xmm3 - pR,pR,pI,pI

	mulps xmm0,xmm3
	;xmm0 - a,c,b,d
	movaps xmm1, xmm0
	shufps xmm1, xmm1, 11101110b
	;xmm1 - a,c,a,c
	shufps xmm0, xmm0, 01000100b
	;xmm0 - b,d,b,d
	pcmpeqw xmm2,xmm2
	pslld xmm2,25
	psrld xmm2,2
	;xmm2 - 1,1,1,1
	pcmpeqw xmm3,xmm3
	pslld xmm3,31
	orps xmm3, xmm2
	;xmm3 - -1,-1,-1,-1
	shufps	xmm3,xmm2,00000000b
	;xmm3 -  1, 1,-1,-1
	mulps xmm1, xmm3 
	;xmm1 - a,c,-a,-c

	shufps xmm3, xmm3, 00111100b
	;xmm3 - -1, 1, 1,-1
	mulps xmm0, xmm3
	;xmm0 - -b, d, b,-d
	;xmm1 -  a, c,-a,-c
	addps xmm0,xmm1
	;xmm0 -  (a-b), (c+d), (b-a), (-c-d)
	;xmm0 -  rT, iT,-rT,-iT

	;r11 - float* rE,   
	;r12 - float* iE,   

	movd xmm1, dword ptr[r11] ;rE
	movd xmm2, dword ptr[r12] ;iE
	movlhps xmm1, xmm2 ;_,iE,_,rE
	shufps xmm1, xmm1, 00100010b
	;xmm1 - rE,iE,rE,iE

	addps xmm0, xmm1
	;xmm0 - r[k],i[k],r[k+N/2],i[k+N/2]

	;rdx - float* r,	
	;r8  - float* i,	
	;r13 - float* r + N/2
	;r14 - float* i + N/2

	;i[k+N/2] 
	movd dword ptr[r14], xmm0
	shufps xmm0, xmm0, 11111001b
	;r[k+N/2]
	movd dword ptr[r13], xmm0
	shufps xmm0, xmm0, 11111001b
	;i[k]
	movd dword ptr[r8], xmm0
	shufps xmm0, xmm0, 11111001b
	;r[k]
	movd dword ptr[rdx], xmm0

	inc rbx 
	;move all of the pointers to their next element
	add rdx, 4
	add r8 , 4
	add r9 , 4
	add r10, 4
	add r11, 4
	add r12, 4
	add r13, 4
	add r14, 4

	jmp LoopLabel

fin: 
	;recover non volatile registers
	pop r14
	pop r13
	pop r12
	pop rbx
ret
funcASM_float_v endp

;xmm0,xmm1,xmm2,xmm3 - volatile
sineASM_float proc
;float x2 = x * x;
movss xmm3, xmm0
	
mulss   xmm0,xmm0  
;float p11 = 0.00000000013291342;

;float p9 = p11 * x2 + -0.000000023317787;
movss	xmm1, coEf5f
mulss	xmm1, xmm0
addss	xmm1, coEf4f

;float p7 = p9 * x2 + 0.0000025222919;
movss	xmm2, xmm1
mulss	xmm2, xmm0
addss	xmm2, coEf3f

;float p5 = p7 * x2 + -0.00017350505;
movss	xmm1, xmm2
mulss	xmm1, xmm0
addss	xmm1, coEf2f

;float p3 = p5 * x2 + 0.0066208798;
movss	xmm2, xmm1
mulss	xmm2, xmm0
addss	xmm2, coEf1f

;float p1 = p3 * x2 + -0.10132118;
movss	xmm1, xmm2
mulss	xmm1, xmm0
addss	xmm1, coEf0f

;return (x - 3.141592653589793238460 + 0.00000008742278) * (x + 3.141592653589793238460 + -0.00000008742278) * p1 * x;
movss xmm0, xmm3

movss xmm2, xmm0
addss xmm2, piMajf
subss xmm2, piMinf
mulss xmm1, xmm2
mulss xmm1, xmm0

subss xmm0, piMajf
addss xmm0, piMinf

mulss xmm0, xmm1
ret

sineASM_float endp

END