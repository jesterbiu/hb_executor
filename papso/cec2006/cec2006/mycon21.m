function [c, ceq] = mycon21(x)
%G21
%usage: [c,ceq] = mycon21(x)

[f,c,ceq] = mlbsuite(x, 1, 5, 'g21');
