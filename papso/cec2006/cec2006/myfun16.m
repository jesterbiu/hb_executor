function f=myfun16(x)
%G16
%usage:  [f] = myfun16(x); and [c,ceq] = myfun16(x)
%
%lb=[704.4148,68.6,0,193,25]';
%ub=[906.3855,288.88,134.75,287.0966,84.1988]';
%A = -[0 1.5 -1 0 0;0 1 1 0 0;0 -1 -1 0 0];
%b = -[0 171.5 -363.63]';
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(5,1).*(ub-lb); % random starting point
%  [X16(:,i),F16(i),exitflag,output] = fmincon('myfun16',x0,A,b,[],[],lb,ub,'mycon16');
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F16); x16 = X16(:,i), [f16,g16,h16] = mlbsuite(x16,38,0,'g16'), FcnCount

% Jane Jing Liang, Nov 2005

[f g h] = mlbsuite(x, 38, 0, 'g16');
