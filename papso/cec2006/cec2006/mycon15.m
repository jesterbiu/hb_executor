function [c, ceq] = mycon15(x)
% G15
%usage: [c,ceq] = mycon15(x)

[f,c,h] = mlbsuite(x, 0, 2, 'g15');
ceq=h(1);