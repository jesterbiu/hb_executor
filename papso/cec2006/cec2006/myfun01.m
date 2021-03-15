function f = myfun01(x),
%G01 (Floundas and Pardalos, 1987)
%usage:  [f] = myfun01(x); and [c,ceq] = myfun01(x)
%
%lb = zeros(13,1);
%ub = [ones(9,1);100*ones(3,1);1];
%A = [ 2  2  0  0  0  0  0  0  0  1  1  0  0;
%      2  0  2  0  0  0  0  0  0  1  0  1  0;
%      0  2  2  0  0  0  0  0  0  0  1  1  0;
%     -8  0  0  0  0  0  0  0  0  1  0  0  0;
%      0 -8  0  0  0  0  0  0  0  0  1  0  0;
%      0  0 -8  0  0  0  0  0  0  0  0  1  0;
%      0  0  0 -2 -1  0  0  0  0  1  0  0  0;
%      0  0  0  0  0 -2 -1  0  0  0  1  0  0;
%      0  0  0  0  0  0  0 -2 -1  0  0  1  0];
%b = [10 10 10 0 0 0 0 0 0]';
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(13,1).*(ub-lb); % random starting point
%  [X01(:,i),F01(i),exitflag,output] = fmincon('myfun01',x0,A,b,[],[],lb,ub,'mycon01');
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F01); x01 = X01(:,i), [f01,g01,h01] = mlbsuite(x01,9,0,'g01'), FcnCount

[f, c, ceq] = mlbsuite(x, 9, 0, 'g01');
