function f = myfun03(x),
%G03 (Michalewicz et. al., 1996)
%usage:  [f] = myfun03(x); and [c,ceq] = myfun03(x)
%
%n = 10
%lb = zeros(n,1);
%ub = ones(n,1);
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(n,1).*(ub-lb); % random starting point
%  [X03(:,i),F03(i),exitflag,output] = fmincon('myfun03',x0,[],[],[],[],lb,ub,'mycon03');
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F03); x03 = X03(:,i), [f03,g03,h03] = mlbsuite(x03,0,1,'g03'), , FcnCount

[f, c, ceq] = mlbsuite(x, 0, 1, 'g03');
