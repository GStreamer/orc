
.function orc_add2_rshift_add_s16_22
.dest 2 d1
.source 2 s1
.source 2 s2
.temp 2 t1

addw t1, s1, s2
addw t1, t1, 2
shrsw t1, t1, 2
addw d1, d1, t1

.function orc_add2_rshift_sub_s16_22
.dest 2 d1
.source 2 s1
.source 2 s2
.temp 2 t1

addw t1, s1, s2
addw t1, t1, 2
shrsw t1, t1, 2
subw d1, d1, t1

.function orc_add2_rshift_add_s16_11
.dest 2 d1
.source 2 s1
.source 2 s2
.temp 2 t1

addw t1, s1, s2
addw t1, t1, 1
shrsw t1, t1, 1
addw d1, d1, t1

.function orc_add2_rshift_sub_s16_11
.dest 2 d1
.source 2 s1
.source 2 s2
.temp 2 t1

addw t1, s1, s2
addw t1, t1, 1
shrsw t1, t1, 1
subw d1, d1, t1

.function orc_add_const_rshift_s16_11
.dest 2 d1
.source 2 s1
.temp 2 t1

addw t1, s1, 1
shrsw d1, t1, 1


.function orc_add_s16
.dest 2 d1
.source 2 s1
.source 2 s2
.temp 2 t1

addw t1, s1, s2
shrsw d1, t1, 1


.function orc_lshift1_s16
.dest 2 d1
.source 2 s1

shlw d1, s1, 1

.function orc_lshift2_s16
.dest 2 d1
.source 2 s1

shlw d1, s1, 2


.function orc_mas2_add_s16
.dest 2 d1
.source 2 s1
.source 2 s2
.temp 2 t1
.temp 4 t2
.param 2 p1
.param 2 p2
.param 2 p3

addw t1, s1, s2
mulswl t2, t1, p1
addl t2, t2, p2
shll t2, t2, p3
convlw t1, t2
addw d1, d1, t1


.function orc_mas4_add_s16_1991
.dest 2 d1
.source 2 s0
.source 2 s1
.source 2 s2
.source 2 s3
.param 2 p1
.param 2 p2
.temp 2 t1
.temp 2 t2

addw t1, s1, s2
mullw t1, t1, 9
addw t2, s0, s3
subw t1, t1, t2
addw t1, t1, p1
shrsw t1, t1, p2
addw d1, d1, t1


.function orc_mas4_sub_s16_1991
.dest 2 d1
.source 2 s0
.source 2 s1
.source 2 s2
.source 2 s3
.param 2 p1
.param 2 p2
.temp 2 t1
.temp 2 t2

addw t1, s1, s2
mullw t1, t1, 9
addw t2, s0, s3
subw t1, t1, t2
addw t1, t1, p1
shrsw t1, t1, p2
subw d1, d1, t1


.function orc_subtract_s16
.dest 2 d1
.source 2 s1
.source 2 s2

subw d1, s1, s2


.function orc_memcpy
.dest 1 d1
.source 1 s1

copyb d1, s1


.function orc_add_s16_u8
.dest 2 d1
.source 2 s1
.source 1 s2
.temp 2 t1

convubw t1, s2
addw d1, t1, s1


.function orc_convert_s16_u8
.dest 2 d1
.source 1 s1

convubw d1, s1


.function orc_convert_u8_s16
.dest 1 d1
.source 2 s1

convsuswb d1, s1


.function orc_subtract_s16_u8
.dest 2 d1
.source 2 s1
.source 1 s2
.temp 2 t1

convubw t1, s2
subw d1, s1, t1


.function orc_multiply_and_add_s16_u8
.dest 2 d1
.source 2 s1
.source 2 s2
.source 1 s3
.temp 2 t1

convubw t1, s3
mullw t1, t1, s2
addw d1, s1, t1


