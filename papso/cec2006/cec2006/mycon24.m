function [c, ceq] = mycon24(x)
%G24
%usage: [c,ceq] = mycon24(x)

[f,c,ceq] = mlbsuite(x, 2, 0, 'g24');
