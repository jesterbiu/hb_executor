function [c,ceq] = mycon04(x),
%G04 (Himmelblau, 1972, problem 11, page 406)
%usage: [c,ceq] = mycon04(x)

[f, c, ceq] = mlbsuite(x, 6, 0, 'g04');
