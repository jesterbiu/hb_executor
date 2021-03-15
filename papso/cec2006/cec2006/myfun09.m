function f = myfun09(x),
%G09 (Hock and Schittkowski, 1981, problem 100)
%usage:  [f] = myfun09(x); and [c,ceq] = myfun09(x)
%
%lb = [-10*ones(7,1)];
%ub = [10*ones(7,1)];
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(7,1).*(ub-lb); % random starting point
%  [X09(:,i),F09(i),exitflag,output] = fmincon('myfun09',x0,[],[],[],[],lb,ub,'mycon09');
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F09); x09 = X09(:,i), [f09,g09,h09] = mlbsuite(x09,4,0,'g09'), FcnCount

[f, g, h] = mlbsuite(x,4,0,'g09');
