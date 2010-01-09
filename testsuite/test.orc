
.function orc_add2_rshift_add_s16_22
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1

addw t1, s1, s2
addw t1, t1, 2
shrsw t1, t1, 2
addw d1, d1, t1

.function orc_add2_rshift_sub_s16_22
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1

addw t1, s1, s2
addw t1, t1, 2
shrsw t1, t1, 2
subw d1, d1, t1


.function orc_add2_rshift_add_s16_11
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1

avgsw t1, s1, s2
#addw t1, s1, s2
#addw t1, t1, 1
#shrsw t1, t1, 1
addw d1, d1, t1


.function orc_add2_rshift_sub_s16_11
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1

avgsw t1, s1, s2
#addw t1, s1, s2
#addw t1, t1, 1
#shrsw t1, t1, 1
subw d1, d1, t1


.function orc_add_const_rshift_s16_11
.dest 2 d1 int16_t
.source 2 s1 int16_t
.temp 2 t1

addw t1, s1, 1
shrsw d1, t1, 1


.function orc_add_const_rshift_s16
.dest 2 d1 int16_t
.param 2 p1
.param 2 p2
.temp 2 t1

addw t1, d1, p1
shrsw d1, t1, p2


.function orc_add_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t

addw d1, s1, s2


.function orc_addc_rshift_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1
.param 2 p1

addw t1, s1, s2
shrsw d1, t1, p1


.function orc_lshift1_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t

shlw d1, s1, 1


.function orc_lshift2_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t

shlw d1, s1, 2


.function orc_lshift_s16_ip
.dest 2 d1 int16_t
.param 2 p1

shlw d1, d1, p1


.function orc_mas2_add_s16_ip
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1
.temp 4 t2
.param 2 p1
.param 4 p2
.param 4 p3

addw t1, s1, s2
mulswl t2, t1, p1
addl t2, t2, p2
shrsl t2, t2, p3
convlw t1, t2
addw d1, d1, t1


.function orc_mas2_sub_s16_ip
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1
.temp 4 t2
.param 2 p1
.param 4 p2
.param 4 p3

addw t1, s1, s2
mulswl t2, t1, p1
addl t2, t2, p2
shrsl t2, t2, p3
convlw t1, t2
subw d1, d1, t1


.function orc_mas4_across_add_s16_1991_ip
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.source 2 s3 int16_t
.source 2 s4 int16_t
.param 4 p1
.param 4 p2
.temp 2 t1
.temp 2 t2
.temp 4 t3
.temp 4 t4

addw t1, s2, s3
mulswl t3, t1, 9
addw t2, s1, s4
convswl t4, t2
subl t3, t3, t4
addl t3, t3, p1
shrsl t3, t3, p2
convlw t1, t3
addw d1, d1, t1


.function orc_mas4_across_sub_s16_1991_ip
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.source 2 s3 int16_t
.source 2 s4 int16_t
.param 4 p1
.param 4 p2
.temp 2 t1
.temp 2 t2
.temp 4 t3
.temp 4 t4

addw t1, s2, s3
mulswl t3, t1, 9
addw t2, s1, s4
convswl t4, t2
subl t3, t3, t4
addl t3, t3, p1
shrsl t3, t3, p2
convlw t1, t3
subw d1, d1, t1


.function orc_subtract_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t

subw d1, s1, s2


.function orc_memcpy
.dest 1 d1 void
.source 1 s1 void

copyb d1, s1


.function orc_add_s16_u8
.dest 2 d1 int16_t
.source 2 s1 int16_t
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
.source 2 s1 int16_t

convsuswb d1, s1


.function orc_offsetconvert_u8_s16
.dest 1 d1
.source 2 s1 int16_t
.temp 2 t1

addw t1, s1, 128
convsuswb d1, t1


.function orc_offsetconvert_s16_u8
.dest 2 d1 int16_t
.source 1 s1
.temp 2 t1

convubw t1, s1
subw d1, t1, 128


.function orc_subtract_s16_u8
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 1 s2
.temp 2 t1

convubw t1, s2
subw d1, s1, t1


.function orc_multiply_and_add_s16_u8
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 1 s2
.temp 2 t1

convubw t1, s2
mullw t1, t1, s1
addw d1, d1, t1


.function orc_splat_s16_ns
.dest 2 d1 int16_t
.param 2 p1

copyw d1, p1


.function orc_splat_u8_ns
.dest 1 d1
.param 1 p1

copyb d1, p1


.function orc_average_u8
.dest 1 d1
.source 1 s1
.source 1 s2

avgub d1, s1, s2


.function orc_rrshift6_s16_ip
.dest 2 d1 int16_t
.temp 2 t1

subw t1, d1, 8160
shrsw d1, t1, 6


.function orc_unpack_yuyv_y
.dest 1 d1
.source 2 s1

select0wb d1, s1


.function orc_unpack_yuyv_u
.dest 1 d1
.source 4 s1
.temp 2 t1

select0lw t1, s1
select1wb d1, t1


.function orc_unpack_yuyv_v
.dest 1 d1
.source 4 s1
.temp 2 t1

select1lw t1, s1
select1wb d1, t1


.function orc_packyuyv
.dest 4 d1
.source 2 s1 uint8_t
.source 1 s2
.source 1 s3
.temp 1 t1
.temp 1 t2
.temp 2 t3
.temp 2 t4
.temp 2 t5

copyw t5, s1
select0wb t1, t5
select1wb t2, t5
mergebw t3, t1, s2
mergebw t4, t2, s3
mergewl d1, t3, t4


.function orc_unpack_uyvy_y
.dest 1 d1
.source 2 s1

select1wb d1, s1


.function orc_unpack_uyvy_u
.dest 1 d1
.source 4 s1
.temp 2 t1

select0lw t1, s1
select0wb d1, t1


.function orc_unpack_uyvy_v
.dest 1 d1
.source 4 s1
.temp 2 t1

select1lw t1, s1
select0wb d1, t1


.function orc_interleave2_s16
.dest 4 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t

mergewl d1, s1, s2


.function orc_interleave2_rrshift1_s16
.dest 4 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1
.temp 2 t2

addw t1, s1, 1
shrsw t1, t1, 1
addw t2, s2, 1
shrsw t2, t2, 1
mergewl d1, t1, t2


.function orc_deinterleave2_s16
.dest 2 d1 int16_t
.dest 2 d2 int16_t
.source 4 s1 int16_t
.temp 4 t1

copyl t1, s1
select0lw d1, t1
select1lw d2, t1


.function orc_deinterleave2_lshift1_s16
.dest 2 d1 int16_t
.dest 2 d2 int16_t
.source 4 s1 int16_t
.temp 4 t1
.temp 2 t2
.temp 2 t3

copyl t1, s1
select0lw t2, t1
shlw d1, t2, 1
select1lw t3, t1
shlw d2, t3, 1


.function orc_haar_sub_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t

subw d1, d1, s1


.function orc_haar_add_half_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t
.temp 2 t1

avgsw t1, s1, 0
addw d1, d1, t1


.function orc_haar_add_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t

addw d1, d1, s1


.function orc_haar_sub_half_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t
.temp 2 t1

avgsw t1, s1, 0
subw d1, d1, t1


.function orc_sum_u8
.accumulator 4 a1 int32_t
.source 1 s1
.temp 2 t1
.temp 4 t2

convubw t1, s1
convuwl t2, t1
accl a1, t2


.function orc_sum_s16
.accumulator 4 a1 int32_t
.source 2 s1 int16_t
.temp 4 t1

convswl t1, s1
accl a1, t1


.function orc_sum_square_diff_u8
.accumulator 4 a1 int32_t
.source 1 s1
.source 1 s2
.temp 2 t1
.temp 2 t2
.temp 4 t3

convubw t1, s1
convubw t2, s2
subw t1, t1, t2
mullw t1, t1, t1
convuwl t3, t1
accl a1, t3


.function orc_dequantise_s16_ip
.dest 2 d1 int16_t
.param 2 p1
.param 2 p2
.temp 2 t1
.temp 2 t2

copyw t1, d1
signw t2, t1
absw t1, t1
mullw t1, t1, p1
addw t1, t1, p2
shrsw t1, t1, 2
mullw d1, t1, t2


.function orc_dequantise_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t
.param 2 p1
.param 2 p2
.temp 2 t1
.temp 2 t2

copyw t1, s1
signw t2, t1
absw t1, t1
mullw t1, t1, p1
addw t1, t1, p2
shrsw t1, t1, 2
mullw d1, t1, t2


# only works for values between -16384 and 16384
.function orc_quantise1_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t
.param 2 p1
.param 2 p2
.param 2 p3
.temp 2 t1
.temp 2 t2

copyw t1, s1
signw t2, t1
absw t1, t1
shlw t1, t1, 2
subw t1, t1, p2
mulhuw t1, t1, p1
shruw t1, t1, p3
mullw d1, t1, t2


# only works for values between -16384 and 16384
.function orc_quantise2_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t
.param 2 p1
.param 2 p2
.temp 2 t1
.temp 2 t2

copyw t1, s1
signw t2, t1
absw t1, t1
shlw t1, t1, 2
subw t1, t1, p2
shruw t1, t1, p1
mullw d1, t1, t2


# only works for values between -16384 and 16384
.function orc_quantdequant1_s16
.dest 2 d1 int16_t
.dest 2 d2 int16_t
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.param 2 p5
.temp 2 t1
.temp 2 t2

copyw t1, d2
signw t2, t1
absw t1, t1
shlw t1, t1, 2
subw t1, t1, p2
mulhuw t1, t1, p1
shruw t1, t1, p3
mullw t2, t1, t2
copyw d1, t2
signw t2, t2
mullw t1, t1, p4
addw t1, t1, p5
shrsw t1, t1, 2
mullw d2, t1, t2


# only works for values between -16384 and 16384
.function orc_quantdequant2_s16
.dest 2 d1 int16_t
.dest 2 d2 int16_t
.param 2 p1
.param 2 p2
.param 2 p4
.param 2 p5
.temp 2 t1
.temp 2 t2

copyw t1, d2
signw t2, t1
absw t1, t1
shlw t1, t1, 2
subw t1, t1, p2
shruw t1, t1, p1
mullw t2, t1, t2
copyw d1, t2
signw t2, t2
mullw t1, t1, p4
addw t1, t1, p5
shrsw t1, t1, 2
mullw d2, t1, t2



.function orc_downsample_vert_u8
.dest 1 d1
.source 1 s1
.source 1 s2
.source 1 s3
.source 1 s4
.temp 2 t1
.temp 2 t2
.temp 2 t3

convubw t1, s1
convubw t2, s4
addw t1, t1, t2
mullw t1, t1, 6
convubw t2, s2
convubw t3, s3
addw t2, t2, t3
mullw t2, t2, 26
addw t2, t2, t1
addw t2, t2, 32
shruw t2, t2, 6
convwb d1, t2


.function orc_downsample_horiz_u8
.dest 1 d1
.source 2 s1 uint8_t
.source 2 s2 uint8_t
.temp 2 t1
.temp 2 t2
.temp 1 t3
.temp 2 t4
.temp 2 t5
.temp 2 t6

copyw t1, s1
copyw t2, s2
select0wb t3, t1
convubw t4, t3
select1wb t3, t2
convubw t5, t3
addw t4, t4, t5
mullw t4, t4, 6
select1wb t3, t1
convubw t5, t3
select0wb t3, t2
convubw t6, t3
addw t5, t5, t6
mullw t5, t5, 26
addw t4, t4, t5
addw t4, t4, 32
shruw t4, t4, 6
convwb d1, t4


.function orc_stats_moment_s16
.source 2 s1 int16_t
.accumulator 4 a1 int32_t
.temp 2 t1
.temp 4 t2

absw t1, s1
subw t1, t1, 2
maxsw t1, t1, 0
convuwl t2, t1
accl a1, t2


.function orc_stats_above_s16
.source 2 s1 int16_t
.accumulator 4 a1 int32_t
.temp 2 t1
.temp 4 t2

absw t1, s1
subw t1, t1, 1
maxsw t1, t1, 0
minsw t1, t1, 1
convuwl t2, t1
accl a1, t2


