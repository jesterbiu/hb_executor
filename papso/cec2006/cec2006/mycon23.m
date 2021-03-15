function [c, ceq] = mycon23(x)
%G23
%usage: [c,ceq] = mycon23(x)

% Jane Jing Liang, Nov 2005

[f,c,h] = mlbsuite(x, 2, 4, 'g23');
ceq = h(2,:);