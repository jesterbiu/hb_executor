function [c,ceq] = mycon03(x),
%G03 (Michalewicz et. al., 1996)
%usage: [c,ceq] = mycon03(x)

[f, c, ceq] = mlbsuite(x, 0, 1, 'g03');
