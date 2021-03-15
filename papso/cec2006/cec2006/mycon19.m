function [c, ceq] = mycon19(x)
%G19
%usage: [c,ceq] = mycon19(x)

[f,c,ceq] = mlbsuite(x, 5, 0, 'g19');
