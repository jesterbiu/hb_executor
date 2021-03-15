function f = myfun04(x),
%G04 (Himmelblau, 1972, problem 11, page 406)
%usage:  [f] = myfun04(x); and [c,ceq] = myfun04(x)
%
%lb = [78;33;27*ones(3,1)];
%ub = [102;45*ones(4,1)];
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(5,1).*(ub-lb); % random starting point
%  [X04(:,i),F04(i),exitflag,output] = fmincon('myfun04',x0,[],[],[],[],lb,ub,'mycon04');
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F04); x04 = X04(:,i), [f04,g04,h04] = mlbsuite(x04,6,0,'g04'), FcnCount

[f, c, ceq] = mlbsuite(x, 6, 0, 'g04');
