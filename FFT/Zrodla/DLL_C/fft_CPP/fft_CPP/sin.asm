.data
piMajf real4 3.141592653589793238460
piMinf real4 0.00000008742278
coEf0f real4 -0.10132118
coEf1f real4 0.0066208798
coEf2f real4 -0.00017350505
coEf3f real4 0.0000025222919
coEf4f real4 -0.000000023317787
coEf5f real4 0.00000000013291342

.code
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

end