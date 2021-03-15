function [c, ceq] = mycon17(x)
%G16
%usage: [c,ceq] = mycon17(x)

[f,c,ceq] = mlbsuite(x, 0, 4, 'g17');
