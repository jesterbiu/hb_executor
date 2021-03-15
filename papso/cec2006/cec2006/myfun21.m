function f=myfun21(x)
%G21
%usage:  [f] = myfun21(x); and [c,ceq] = myfun21(x)
%
%lb = [0 0 0 100 6.3 5.9 4.5]';
%ub = [1000 40 40 300 6.7 6.4 6.25]';
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(7,1).*(ub-lb); % random starting point
%  [X21(:,i),F21(i),exitflag,output] = fmincon('myfun21',x0,[],[],[],[],lb,ub,'mycon21');
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F21); x21 = X21(:,i),[f21,g21,h21]=mlbsuite(x21,1,5,'g21'), FcnCount

[f,g,h] = mlbsuite(x, 1, 5, 'g21');
