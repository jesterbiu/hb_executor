function f = myfun13(x),
%G13 (Hock And Schittkowski, 1981)
%usage:  [f] = myfun13(x); and [c,ceq] = myfun13(x)
%
%lb = [-2.3;-2.3;-3.2*ones(3,1)];
%ub = [2.3;2.3;3.2*ones(3,1)];
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(5,1).*(ub-lb); % random starting point
%  [X13(:,i),F13(i),exitflag,output] = fmincon('myfun13',x0,[],[],[],[],lb,ub,'mycon13');
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F13); x13 = X13(:,i), [f13,g13,h13] = mlbsuite(x13,0,3,'g13'), FcnCount

[f,c,ceq] = mlbsuite(x,0,3,'g13');

