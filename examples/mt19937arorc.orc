
.function mt19937ar_mix
.dest 4 mt
.source 4 mt1
.source 4 mt2
.temp 4 y
.temp 4 t1
.temp 4 t2
.const 4 c1 1
.const 4 UPPER_MASK 0x80000000
.const 4 LOWER_MASK 0x7fffffff
.const 4 MATRIX_A 0x9908b0df


loadl t1, mt
andl t1, t1, UPPER_MASK
loadl t2, mt1
andl t2, t2, LOWER_MASK
orl y, t1, t2
andl t1, y, c1
cmpeql t1, t1, c1
andl t1, t1, MATRIX_A
shrul y, y, c1
xorl y, y, t1
xorl mt, mt2, y


.function mt19937ar_temper
.dest 4 d
.source 4 s
.temp 4 y
.temp 4 t

loadl y, s
shrul t, y, 11
xorl y, y, t
shll t, y, 7
andl t, t, 0x9d2c5680
xorl y, y, t
shll t, y, 15 
andl t, t, 0xefc60000
xorl y, y, t
shrul t, y, 18
xorl d, y, t


#.function mt19937ar_mix_temper
#.dest 4 d
#.dest 4 mt
#.source 4 mt1
#.source 4 mt2
#.temp 4 y
#.temp 4 t1
#.temp 4 t2
#.const 4 c1 1
#.const 4 UPPER_MASK 0x80000000
#.const 4 LOWER_MASK 0x7fffffff
#.const 4 MATRIX_A 0x9908b0df
#
#
#loadl t1, mt
#andl t1, t1, UPPER_MASK
#loadl t2, mt1
#andl t2, t2, LOWER_MASK
#orl y, t1, t2
#andl t1, y, c1
#cmpeql t1, t1, c1
#andl t1, t1, MATRIX_A
#shrul y, y, c1
#xorl y, y, t1
#xorl y, mt2, y
#storel mt, y
#shrul t1, y, 11
#xorl y, y, t1
#shll t1, y, 7
#andl t1, t1, 0x9d2c5680
#xorl y, y, t1
#shll t1, y, 15 
#andl t1, t1, 0xefc60000
#xorl y, y, t1
#shrul t1, y, 18
#xorl d, y, t1



