function [c, ceq] = mycon16(x)
%G16
%usage: [c,ceq] = mycon16(x)

% Jane Jing Liang, Nov 2005

[f g h] = mlbsuite(x, 38, 0, 'g16');

g_num=[1,3,4,7:38];
c=g(g_num,:);
ceq=[];