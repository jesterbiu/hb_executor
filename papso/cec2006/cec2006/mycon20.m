function [c, ceq] = mycon20(x)
%G20
%usage: [c,ceq] = mycon20(x)

% Jane Jing Liang, Nov 2005

[f,c,h] = mlbsuite(x, 6, 14, 'g20');
ceq=h(1:12,:);