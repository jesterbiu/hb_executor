function [c,ceq] = mycon06(x),
%G06 (Flounds and Pardalos, 1987)
%usage: [c,ceq] = mycon06(x)

[f, c, ceq] = mlbsuite(x, 2, 0, 'g06');
