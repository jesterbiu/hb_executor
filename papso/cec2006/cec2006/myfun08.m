function f = myfun08(x),
%G08 (Schoenauer and Xanthakis, 1993)
%usage:  [f] = myfun08(x); and [c,ceq] = myfun08(x)
%
%lb = [zeros(2,1)];
%ub = [10*ones(2,1)];
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(2,1).*(ub-lb); % random starting point
%  [X08(:,i),F08(i),exitflag,output] = fmincon('myfun08',x0,[],[],[],[],lb,ub,'mycon08');
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F08); x08 = X08(:,i), [f08,g08,h08] = mlbsuite(x08,2,0,'g08'), FcnCount

[f, g, h] = mlbsuite(x, 2, 0, 'g08');
