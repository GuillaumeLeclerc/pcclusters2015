set terminal wxt size 350,262 enhanced font 'Verdana,10' persist
set border linewidth 1.5
set style line 1 linecolor rgb '#0060ad' linetype 1 linewidth 2
set style line 2 linecolor rgb '#dd181f' linetype 1 linewidth 2
set style line 3 linecolor rgb '#18dd1f' linetype 1 linewidth 2
set style line 3 linecolor rgb '#18dddd' linetype 1 linewidth 2
c11 = 1
c12 = 1
c15 = 1
c21 = 1
c22 = 10
c25 = 0.0001
c31 = 1
c32 = 1
c35 = 0.005
c41 = 30
c42 = 30
c45 = 9
set xrange[1:500]
set yrange[0:35]
set xlabel 'number of nodes'
set ylabel 'speedup'
f(x) = c11/(c12/x + c15*log(x))
g(x) = c21/(c22/x + c25*log(x))
h(x) = c31/(c32/x + c35*log(x))
i(x) = c41/(c42/x + c45*log(x))
plot f(x) title 'theoretical speedup with C_1 = C_2 = C_5 = 1' with lines linestyle 1, \
g(x) title 'theoretical speedup with C_1 = 1, C_2 = 10, C_5 = 0.0001' with lines linestyle 2, \
h(x) title 'theoretical speedup with C_1 = C_2 = 1,  C_5 = 0.005' with lines linestyle 3, \
i(x) title 'theoretical speedup with our estimations of the constants' with lines linestyle 4
