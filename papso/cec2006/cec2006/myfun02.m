function f = myfun02(x),
%G02 (Keane, 1994)
%usage:  [f] = myfun02(x); and [c,ceq] = myfun02(x)
%
%n = 20
%lb = zeros(n,1);
%ub = [10*ones(n,1)];
%A = ones(1,n);
%b = 7.5*n;
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(n,1).*(ub-lb); % random starting point
%  [X02(:,i),F02(i),exitflag,output] = fmincon('myfun02',x0,A,b,[],[],lb,ub,'mycon02');
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F02); x02 = X02(:,i), [f02,g02,h02] = mlbsuite(x02, 2, 0, 'g02'), FcnCount
%
% xopt02 = [ ...
% 3.16237443645701 3.12819975856112 3.09481384891456 3.06140284777302 ...
% 3.02793443337239 2.99385691314995 2.95870651588255 2.92182183591092 ...
% 0.49455118612682 0.48849305858571 0.48250798063845 0.47695629293225 ...
% 0.47108462715587 0.46594074852233 0.46157984137635 0.45721400967989 ...
% 0.45237696886802 0.44805875597713 0.44435772435707 0.44019839654132]'


[f, c, ceq] = mlbsuite(x, 2, 0, 'g02');
