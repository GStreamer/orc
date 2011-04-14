
.function orc_add2_rshift_add_s16_22_op
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.source 2 s3 orc_int16
.temp 2 t1

addw t1, s2, s3
addw t1, t1, 2
shrsw t1, t1, 2
addw d1, s1, t1


.function orc_add2_rshift_add_s16_22
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.temp 2 t1

addw t1, s1, s2
addw t1, t1, 2
shrsw t1, t1, 2
addw d1, d1, t1


.function orc_add2_rshift_sub_s16_22_op
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.source 2 s3 orc_int16
.temp 2 t1

addw t1, s2, s3
addw t1, t1, 2
shrsw t1, t1, 2
subw d1, s1, t1


.function orc_add2_rshift_sub_s16_22
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.temp 2 t1

addw t1, s1, s2
addw t1, t1, 2
shrsw t1, t1, 2
subw d1, d1, t1


.function orc_add2_rshift_add_s16_11_op
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.source 2 s3 orc_int16
.temp 2 t1

avgsw t1, s2, s3
addw d1, s1, t1


.function orc_add2_rshift_add_s16_11
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.temp 2 t1

avgsw t1, s1, s2
addw d1, d1, t1


.function orc_add2_rshift_sub_s16_11_op
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.source 2 s3 orc_int16
.temp 2 t1

avgsw t1, s2, s3
subw d1, s1, t1


.function orc_add2_rshift_sub_s16_11
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.temp 2 t1

avgsw t1, s1, s2
subw d1, d1, t1


.function orc_add_const_rshift_s16_11
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.temp 2 t1

addw t1, s1, 1
shrsw d1, t1, 1


.function orc_add_const_rshift_s16
.dest 2 d1 orc_int16
.param 2 p1
.param 2 p2
.temp 2 t1

addw t1, d1, p1
shrsw d1, t1, p2


.function orc_add_s16
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16

addw d1, s1, s2


.function orc_add_s16_2d
.flags 2d
.dest 2 d1 orc_int16
.source 2 s1 orc_int16

addw d1, d1, s1


.function orc_addc_rshift_s16
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.temp 2 t1
.param 2 p1

addw t1, s1, s2
shrsw d1, t1, p1


.function orc_lshift1_s16
.dest 2 d1 orc_int16
.source 2 s1 orc_int16

shlw d1, s1, 1


.function orc_lshift2_s16
.dest 2 d1 orc_int16
.source 2 s1 orc_int16

shlw d1, s1, 2


.function orc_lshift_s16_ip
.dest 2 d1 orc_int16
.param 2 p1

shlw d1, d1, p1


.function orc_mas2_add_s16_op
.dest 2 d1 orc_int16
.source 2 s0 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
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
addw d1, s0, t1


.function orc_mas2_add_s16_ip
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
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


.function orc_mas2_sub_s16_op
.dest 2 d1 orc_int16
.source 2 s0 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
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
subw d1, s0, t1


.function orc_mas2_sub_s16_ip
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
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


.function orc_mas4_across_add_s16_1991_op
.dest 2 d1 orc_int16
.source 2 s0 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.source 2 s3 orc_int16
.source 2 s4 orc_int16
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
addw d1, s0, t1


.function orc_mas4_across_add_s16_1991_ip
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.source 2 s3 orc_int16
.source 2 s4 orc_int16
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


.function orc_mas4_across_sub_s16_1991_op
.dest 2 d1 orc_int16
.source 2 s0 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.source 2 s3 orc_int16
.source 2 s4 orc_int16
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
subw d1, s0, t1


.function orc_mas4_across_sub_s16_1991_ip
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.source 2 s3 orc_int16
.source 2 s4 orc_int16
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
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16

subw d1, s1, s2


.function orc_add_s16_u8
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 1 s2
.temp 2 t1

convubw t1, s2
addw d1, t1, s1


.function orc_add_s16_u8_2d
.flags 2d
.dest 2 d1 orc_int16
.source 1 s1
.temp 2 t1

convubw t1, s1
addw d1, d1, t1


.function orc_convert_s16_u8
.dest 2 d1
.source 1 s1

convubw d1, s1


.function orc_convert_u8_s16
.dest 1 d1
.source 2 s1 orc_int16

convsuswb d1, s1


.function orc_offsetconvert_u8_s16
.dest 1 d1
.source 2 s1 orc_int16
.temp 2 t1

addw t1, s1, 128
convsuswb d1, t1


.function orc_offsetconvert_s16_u8
.dest 2 d1 orc_int16
.source 1 s1
.temp 2 t1

convubw t1, s1
subw d1, t1, 128


.function orc_subtract_s16_u8
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 1 s2
.temp 2 t1

convubw t1, s2
subw d1, s1, t1


.function orc_multiply_and_add_s16_u8
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 1 s2
.temp 2 t1

convubw t1, s2
mullw t1, t1, s1
addw d1, d1, t1


.function orc_splat_s16_ns
.dest 2 d1 orc_int16
.param 2 p1

copyw d1, p1


.function orc_splat_s16_2d_4xn
.n 4
.flags 2d
.dest 2 d1 orc_int16
.param 2 p1

copyw d1, p1


.function orc_splat_s16_2d_8xn
.n 8
.flags 2d
.dest 2 d1 orc_int16
.param 2 p1

copyw d1, p1


.function orc_splat_s16_2d
.flags 2d
.dest 2 d1 orc_int16
.param 2 p1

copyw d1, p1


.function orc_splat_u8_ns
.dest 1 d1
.param 1 p1

copyb d1, p1


.function orc_splat_u8_2d
.flags 2d
.dest 1 d1
.param 1 p1

copyb d1, p1


.function orc_average_u8
.dest 1 d1
.source 1 s1
.source 1 s2

avgub d1, s1, s2


.function orc_rrshift6_add_s16_2d
.flags 2d
.dest 1 d1 orc_uint8
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.temp 2 t1

addw t1, s2, 32
shrsw t1, t1, 6
addw t1, s1, t1
convsuswb d1, t1


.function orc_rrshift6_sub_s16_2d
.flags 2d
.dest 2 d1 orc_int16
.dest 2 d2 orc_int16
.temp 2 t1

subw t1, d2, 8160
shrsw t1, t1, 6
copyw d2, t1
subw d1, d1, t1


.function orc_rrshift6_s16_ip_2d
.flags 2d
.dest 2 d1 orc_int16
.temp 2 t1

subw t1, d1, 8160
shrsw d1, t1, 6


.function orc_rrshift6_s16_ip
.dest 2 d1 orc_int16
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
.source 2 s1 orc_uint8
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
.dest 4 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16

mergewl d1, s1, s2


.function orc_interleave2_rrshift1_s16
.dest 4 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.temp 2 t1
.temp 2 t2

addw t1, s1, 1
shrsw t1, t1, 1
addw t2, s2, 1
shrsw t2, t2, 1
mergewl d1, t1, t2


.function orc_deinterleave2_s16
.dest 2 d1 orc_int16
.dest 2 d2 orc_int16
.source 4 s1 orc_int16
.temp 4 t1

copyl t1, s1
select0lw d1, t1
select1lw d2, t1


.function orc_deinterleave2_lshift1_s16
.dest 2 d1 orc_int16
.dest 2 d2 orc_int16
.source 4 s1 orc_int16
.temp 4 t1
.temp 2 t2
.temp 2 t3

copyl t1, s1
select0lw t2, t1
shlw d1, t2, 1
select1lw t3, t1
shlw d2, t3, 1


.function orc_haar_deint_lshift1_split_s16
.dest 2 d1 orc_int16
.dest 2 d2 orc_int16
.source 4 s1 orc_int16
.temp 2 t1
.temp 2 t2
.temp 4 t3

copyl t3, s1
select0lw t1, t3
select1lw t2, t3
shlw t1, t1, 1
shlw t2, t2, 1
subw t2, t2, t1
copyw d2, t2
avgsw t2, t2, 0
addw d1, t1, t2


.function orc_haar_deint_split_s16
.dest 2 d1 orc_int16
.dest 2 d2 orc_int16
.source 4 s1 orc_int16
.temp 2 t1
.temp 2 t2
.temp 4 t3

copyl t3, s1
select0lw t1, t3
select1lw t2, t3
subw t2, t2, t1
copyw d2, t2
avgsw t2, t2, 0
addw d1, t1, t2


.function orc_haar_split_s16_lo
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.temp 2 t1
.temp 2 t2

copyw t1, s1
subw t2, s2, t1
avgsw t2, t2, 0
addw d1, t1, t2


.function orc_haar_split_s16_hi
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16

subw d1, s2, s1


.function orc_haar_split_s16_op
.dest 2 d1 orc_int16
.dest 2 d2 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.temp 2 t1
.temp 2 t2

copyw t1, s1
subw t2, s2, t1
copyw d2, t2
avgsw t2, t2, 0
addw d1, t1, t2


.function orc_haar_split_s16
.dest 2 d1 orc_int16
.dest 2 d2 orc_int16
.temp 2 t1
.temp 2 t2

copyw t1, d1
copyw t2, d2
subw t2, t2, t1
copyw d2, t2
avgsw t2, t2, 0
addw d1, t1, t2


.function orc_haar_synth_s16_lo
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.temp 2 t1

avgsw t1, s2, 0
subw d1, s1, t1


.function orc_haar_synth_s16_hi
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.temp 2 t1
.temp 2 t2
.temp 2 t3

copyw t2, s2
avgsw t3, t2, 0
subw t1, s1, t3
addw d1, t2, t1


.function orc_haar_synth_s16_op
.dest 2 d1 orc_int16
.dest 2 d2 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.temp 2 t1
.temp 2 t2
.temp 2 t3

copyw t2, s2
avgsw t3, t2, 0
subw t1, s1, t3
copyw d1, t1
addw d2, t2, t1


.function orc_haar_synth_s16
.dest 2 d1 orc_int16
.dest 2 d2 orc_int16
.temp 2 t1
.temp 2 t2
.temp 2 t3

copyw t1, d1
copyw t2, d2
avgsw t3, t2, 0
subw t1, t1, t3
copyw d1, t1
addw d2, t2, t1


.function orc_haar_synth_rrshift1_int_s16
.dest 4 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.temp 2 t1
.temp 2 t2

copyw t2, s2
avgsw t1, t2, 0
subw t1, s1, t1
addw t2, t2, t1
avgsw t1, t1, 0
avgsw t2, t2, 0
mergewl d1, t1, t2


.function orc_haar_synth_int_s16
.dest 4 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.temp 2 t1
.temp 2 t2

copyw t2, s2
avgsw t1, t2, 0
subw t1, s1, t1
addw t2, t2, t1
mergewl d1, t1, t2


.function orc_haar_sub_s16
.dest 2 d1 orc_int16
.source 2 s1 orc_int16

subw d1, d1, s1


.function orc_haar_add_half_s16
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.temp 2 t1

avgsw t1, s1, 0
addw d1, d1, t1


.function orc_haar_add_s16
.dest 2 d1 orc_int16
.source 2 s1 orc_int16

addw d1, d1, s1


.function orc_haar_sub_half_s16
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.temp 2 t1

avgsw t1, s1, 0
subw d1, d1, t1


.function orc_sum_u8
.accumulator 4 a1 orc_int32
.source 1 s1
.temp 2 t1
.temp 4 t2

convubw t1, s1
convuwl t2, t1
accl a1, t2


.function orc_sum_s16
.accumulator 4 a1 orc_int32
.source 2 s1 orc_int16
.temp 4 t1

convswl t1, s1
accl a1, t1


.function orc_sum_square_diff_u8
.accumulator 4 a1 orc_int32
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


.function orc_dequantise_s16_2d_4xn
.n 4
.flags 2d
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
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


.function orc_dequantise_s16_2d_8xn
.n 8
.flags 2d
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
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


.function orc_dequantise_s16_ip_2d_8xn
.n 8
.flags 2d
.dest 2 d1 orc_int16
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


.function orc_dequantise_s16_ip_2d
.flags 2d
.dest 2 d1 orc_int16
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


.function orc_dequantise_s16_ip
.dest 2 d1 orc_int16
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
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
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


.function orc_dequantise_var_s16_ip
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
.source 2 s2 orc_int16
.temp 2 t1
.temp 2 t2

copyw t1, d1
signw t2, t1
absw t1, t1
mullw t1, t1, s1
addw t1, t1, s2
shrsw t1, t1, 2
mullw d1, t1, t2


# only works for values between -16384 and 16384
.function orc_quantise1_s16
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
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
.dest 2 d1 orc_int16
.source 2 s1 orc_int16
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
.dest 2 d1 orc_int16
.dest 2 d2 orc_int16
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
.function orc_quantdequant3_s16
.dest 2 d1 orc_int16
.dest 2 d2 orc_int16
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.param 2 p5
.param 4 p6
.temp 2 t1
.temp 2 t2
.temp 4 t3

copyw t1, d2
signw t2, t1
absw t1, t1
shlw t1, t1, 2
subw t1, t1, p2
muluwl t3, t1, p1
addl t3, t3, p6
shrul t3, t3, p3
convlw t1, t3
mullw t2, t1, t2
copyw d1, t2
signw t2, t2
mullw t1, t1, p4
addw t1, t1, p5
shrsw t1, t1, 2
mullw d2, t1, t2


# only works for values between -16384 and 16384
.function orc_quantdequant2_s16
.dest 2 d1 orc_int16
.dest 2 d2 orc_int16
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
.source 2 s1 orc_uint8
.source 2 s2 orc_uint8
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
.source 2 s1 orc_int16
.accumulator 4 a1 orc_int32
.temp 2 t1
.temp 4 t2

absw t1, s1
subw t1, t1, 2
maxsw t1, t1, 0
convuwl t2, t1
accl a1, t2


.function orc_stats_above_s16
.source 2 s1 orc_int16
.accumulator 4 a1 orc_int32
.temp 2 t1
.temp 4 t2

absw t1, s1
subw t1, t1, 1
maxsw t1, t1, 0
minsw t1, t1, 1
convuwl t2, t1
accl a1, t2


.function orc_accw
.accumulator 2 a1 int
.source 2 s1 orc_int16
.temp 2 t1

absw t1, s1
accw a1, t1


.function orc_avg2_8xn_u8
.flags 2d
.n 8
.dest 1 d1 orc_uint8
.source 1 s1 orc_uint8
.source 1 s2 orc_uint8

avgub d1, s1, s2


.function orc_avg2_12xn_u8
.flags 2d
.n 12
.dest 1 d1 orc_uint8
.source 1 s1 orc_uint8
.source 1 s2 orc_uint8

avgub d1, s1, s2


.function orc_avg2_16xn_u8
.flags 2d
.n 16
.dest 1 d1 orc_uint8
.source 1 s1 orc_uint8
.source 1 s2 orc_uint8

avgub d1, s1, s2


.function orc_avg2_32xn_u8
.flags 2d
.n 32
.dest 1 d1 orc_uint8
.source 1 s1 orc_uint8
.source 1 s2 orc_uint8

avgub d1, s1, s2


.function orc_avg2_nxm_u8
.flags 2d
.dest 1 d1 orc_uint8
.source 1 s1 orc_uint8
.source 1 s2 orc_uint8

avgub d1, s1, s2


.function orc_combine4_8xn_u8
.flags 2d
.n 8
.dest 1 d1 orc_uint8
.source 1 s1 orc_uint8
.source 1 s2 orc_uint8
.source 1 s3 orc_uint8
.source 1 s4 orc_uint8
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.temp 2 t1
.temp 2 t2

convubw t1, s1
mullw t2, t1, p1
convubw t1, s2
mullw t1, t1, p2
addw t2, t2, t1
convubw t1, s3
mullw t1, t1, p3
addw t2, t2, t1
convubw t1, s4
mullw t1, t1, p4
addw t2, t2, t1
addw t2, t2, 8
convsuswb d1, t2


.function orc_combine4_12xn_u8
.flags 2d
.n 12
.dest 1 d1 orc_uint8
.source 1 s1 orc_uint8
.source 1 s2 orc_uint8
.source 1 s3 orc_uint8
.source 1 s4 orc_uint8
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.temp 2 t1
.temp 2 t2

convubw t1, s1
mullw t2, t1, p1
convubw t1, s2
mullw t1, t1, p2
addw t2, t2, t1
convubw t1, s3
mullw t1, t1, p3
addw t2, t2, t1
convubw t1, s4
mullw t1, t1, p4
addw t2, t2, t1
addw t2, t2, 8
convsuswb d1, t2


.function orc_combine4_16xn_u8
.flags 2d
.n 16
.dest 1 d1 orc_uint8
.source 1 s1 orc_uint8
.source 1 s2 orc_uint8
.source 1 s3 orc_uint8
.source 1 s4 orc_uint8
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.temp 2 t1
.temp 2 t2

convubw t1, s1
mullw t2, t1, p1
convubw t1, s2
mullw t1, t1, p2
addw t2, t2, t1
convubw t1, s3
mullw t1, t1, p3
addw t2, t2, t1
convubw t1, s4
mullw t1, t1, p4
addw t2, t2, t1
addw t2, t2, 8
convsuswb d1, t2


.function orc_combine4_24xn_u8
.flags 2d
.n 24
.dest 1 d1 orc_uint8
.source 1 s1 orc_uint8
.source 1 s2 orc_uint8
.source 1 s3 orc_uint8
.source 1 s4 orc_uint8
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.temp 2 t1
.temp 2 t2

convubw t1, s1
mullw t2, t1, p1
convubw t1, s2
mullw t1, t1, p2
addw t2, t2, t1
convubw t1, s3
mullw t1, t1, p3
addw t2, t2, t1
convubw t1, s4
mullw t1, t1, p4
addw t2, t2, t1
addw t2, t2, 8
convsuswb d1, t2


.function orc_combine4_32xn_u8
.flags 2d
.n 32
.dest 1 d1 orc_uint8
.source 1 s1 orc_uint8
.source 1 s2 orc_uint8
.source 1 s3 orc_uint8
.source 1 s4 orc_uint8
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.temp 2 t1
.temp 2 t2

convubw t1, s1
mullw t2, t1, p1
convubw t1, s2
mullw t1, t1, p2
addw t2, t2, t1
convubw t1, s3
mullw t1, t1, p3
addw t2, t2, t1
convubw t1, s4
mullw t1, t1, p4
addw t2, t2, t1
addw t2, t2, 8
convsuswb d1, t2


.function orc_combine4_nxm_u8
.flags 2d
.dest 1 d1 orc_uint8
.source 1 s1 orc_uint8
.source 1 s2 orc_uint8
.source 1 s3 orc_uint8
.source 1 s4 orc_uint8
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.temp 2 t1
.temp 2 t2

convubw t1, s1
mullw t2, t1, p1
convubw t1, s2
mullw t1, t1, p2
addw t2, t2, t1
convubw t1, s3
mullw t1, t1, p3
addw t2, t2, t1
convubw t1, s4
mullw t1, t1, p4
addw t2, t2, t1
addw t2, t2, 8
shrsw t2, t2, 4
convsuswb d1, t2


.function orc_combine2_8xn_u8
.flags 2d
.n 8
.dest 1 d1 orc_uint8
.source 1 s1 orc_uint8
.source 1 s2 orc_uint8
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.temp 2 t1
.temp 2 t2

convubw t1, s1
convubw t2, s2
mullw t1, t1, p1
mullw t2, t2, p2
addw t1, t1, t2
addw t1, t1, p3
shrsw t1, t1, p4
convsuswb d1, t1



.function orc_combine2_12xn_u8
.flags 2d
.n 12
.dest 1 d1 orc_uint8
.source 1 s1 orc_uint8
.source 1 s2 orc_uint8
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.temp 2 t1
.temp 2 t2

convubw t1, s1
convubw t2, s2
mullw t1, t1, p1
mullw t2, t2, p2
addw t1, t1, t2
addw t1, t1, p3
shrsw t1, t1, p4
convsuswb d1, t1



.function orc_combine2_16xn_u8
.flags 2d
.n 16
.dest 1 d1 orc_uint8
.source 1 s1 orc_uint8
.source 1 s2 orc_uint8
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.temp 2 t1
.temp 2 t2

convubw t1, s1
convubw t2, s2
mullw t1, t1, p1
mullw t2, t2, p2
addw t1, t1, t2
addw t1, t1, p3
shrsw t1, t1, p4
convsuswb d1, t1



.function orc_combine2_nxm_u8
.flags 2d
.dest 1 d1 orc_uint8
.source 1 s1 orc_uint8
.source 1 s2 orc_uint8
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.temp 2 t1
.temp 2 t2

convubw t1, s1
convubw t2, s2
mullw t1, t1, p1
mullw t2, t2, p2
addw t1, t1, t2
addw t1, t1, p3
shrsw t1, t1, p4
convsuswb d1, t1



.function orc_sad_nxm_u8
.flags 2d
.accumulator 4 a1 orc_uint32
.source 1 s1 orc_uint8
.source 1 s2 orc_uint8

accsadubl a1, s1, s2


.function orc_sad_8x8_u8
.flags 2d
.n 8
.m 8
.accumulator 4 a1 orc_uint32
.source 1 s1 orc_uint8
.source 1 s2 orc_uint8

accsadubl a1, s1, s2



.function orc_sad_12x12_u8
.flags 2d
.n 12
.m 12
.accumulator 4 a1 orc_uint32
.source 1 s1 orc_uint8
.source 1 s2 orc_uint8

accsadubl a1, s1, s2



.function orc_sad_16xn_u8
.flags 2d
.n 16
.accumulator 4 a1 orc_uint32
.source 1 s1 orc_uint8
.source 1 s2 orc_uint8

accsadubl a1, s1, s2



.function orc_sad_32xn_u8
.flags 2d
.n 32
.accumulator 4 a1 orc_uint32
.source 1 s1 orc_uint8
.source 1 s2 orc_uint8

accsadubl a1, s1, s2


.function convert_rgb_to_gray
.dest 1 d1
.source 4 s1
.temp 1 l_t1
.temp 2 l_t2
.temp 2 l_gray2

# Red * ((0.299) * (1<<16) + 0.5)
select0lw l_t2, s1
select0wb l_t1, l_t2
convubw l_gray2, l_t1
swapw l_gray2, l_gray2
mulhuw l_gray2, l_gray2, 19595
       
# Green * ((0.587) * (1<<16) + 0.5)
select0lw l_t2, s1
select1wb l_t1, l_t2
convubw l_t2, l_t1
swapw l_t2, l_t2
mulhuw l_t2, l_t2, 38470
addusw l_gray2, l_gray2, l_t2

# Blue * ((0.114) * (1<<16) + 0.5)
select1lw l_t2, s1
select0wb l_t1, l_t2
convubw l_t2, l_t1
swapw l_t2, l_t2
mulhuw l_t2, l_t2, 7471
addusw l_gray2, l_gray2, l_t2

# Add 1/2 => (1 << (8 - 1))
addusw l_gray2, l_gray2, 128
select1wb d1, l_gray2



.function canny_calc_delta_x
.dest 4 d1 orc_int32
.source 4 s1 orc_uint8
.source 4 s2 orc_uint8
.temp 2 t1
.temp 2 t2
.temp 1 t3
.temp 2 t4
.temp 1 t5
.temp 2 t6
.temp 4 t7
.temp 4 td1

select0lw t2, s1
select1wb t3, t2
select0lw t4, s2
select1wb t5, t4
convubw t4, t3
convubw t6, t5
subw t1, t4, t6
convswl t7, t1
mulll td1, t7, t7

select1lw t2, s1
select0wb t3, t2
select1lw t4, s2
select0wb t5, t4
convubw t4, t3
convubw t6, t5
subw t1, t4, t6
convswl t7, t1
mulll t7, t7, t7
addl td1, td1, t7

select1lw t2, s1
select1wb t3, t2
select1lw t4, s2
select1wb t5, t4
convubw t4, t3
convubw t6, t5
subw t1, t4, t6
convswl t7, t1
mulll t7, t7, t7
addl d1, td1, t7


.function i420_to_ayuv
.dest 4 d1
.source 1 y
.source 1 u
.source 1 v
.param 1 a
.temp 1 tu
.temp 1 tv
.temp 1 ty
.temp 2 t1
.temp 2 t2

loadupdb tu, u
loadupdb tv, v
loadb ty, y
mergebw t1, a, ty
mergebw t2, tu, tv
mergewl d1, t1, t2



.function orc_splat_u16
.dest 2 d1 orc_uint16
.param 2 p1

copyw d1, p1


.function orc_splat_u32
.dest 4 d1 orc_uint32
.param 4 p1

copyl d1, p1


.function orc_splat_u16_2d
.dest 2 d1 orc_uint16
.param 2 p1
.flags 2d

copyw d1, p1


.function orc_splat_u32_2d
.dest 4 d1 orc_uint32
.param 4 p1
.flags 2d

copyl d1, p1


.function orc_copy_u16_2d
.dest 2 d1
.source 2 s1
.flags 2d

copyw d1, s1


.function orc_copy_u32_2d
.dest 4 d1
.source 4 s1
.flags 2d

copyl d1, s1


.function orc_composite_add_8_8_line
.dest 1 d1
.source 1 s1

addusb d1, d1, s1


.function orc_composite_add_n_8_8_line
.dest 1 d1
.source 1 s1
.param 2 p1
.temp 2 t1
.temp 1 t2

#compina t1, p1, s1
convubw t1, s1
mullw t1, t1, p1
div255w t1, t1
convwb t2, t1
addusb d1, d1, t2


.function test_float_constant_1
.dest 4 d1
.const 4 c1 2.0

copyl d1, c1


.function test_float_constant_2
.dest 4 d1

copyl d1, 2.0


.function param64
.dest 8 d
.param 8 s

copyq d, s


