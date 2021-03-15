function [c, ceq] = mycon18(x)
%G18
%usage: [c,ceq] = mycon18(x)

[f,c,ceq] = mlbsuite(x, 13, 0, 'g18');
