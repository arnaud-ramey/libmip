% https://www.gnu.org/software/octave/doc/interpreter/Linear-Least-Squares.html
% https://stats.stackexchange.com/questions/32504/simple-multivariate-regression-with-octave
% http://sachinashanbhag.blogspot.fr/2012/07/linear-least-squares-in-gnu-octave-part.html
% https://fr.mathworks.com/help/stats/regress.html

M=[8, 0, 0.1683, 0.0000
8, 8, 0.1678, 0.5942
8, 16, 0.1720, 1.2400
8, 24, 0.1645, 1.9628
8, 32, 0.1686, 2.7203
10, 10, 0.2070, 0.7380
16, 0, 0.3474, 0.0000
16, 8, 0.3367, 0.6544
16, 16, 0.3351, 1.3223
16, 24, 0.3330, 1.9880
16, 32, 0.3250, 2.6400
24, 0, 0.5050, 0.0000
24, 8, 0.4917, 0.7073
24, 16, 0.4841, 1.3858
24, 24, 0.4670, 2.0410
24, 32, 0.4673, 2.7409
32, 0, 0.7152, 0.0000
32, 8, 0.6544, 0.6779
32, 16, 0.6420, 1.3317
32, 24, 0.6228, 2.0299
32, 32, 0.6091, 2.6465];

lin=M(:,1);
ang=M(:,2);
  v=M(:,3);
  w=M(:,4);

% linear regression (v, w, v*w) -> lin
X=[ones(size(v)), v, w, v .* w];
[bLin, sigma, r]=ols(lin, X);
bLin
%~ bLin =  0.50949 45.38459 -0.81221 3.78223
normR=norm(r)
[1 8 8 8*8]*bLin       % simple prediction
linEst=X*bLin          % check model with initial data

scatter3(v, w, lin, 'filled');
hold on
vfit = min(v):.1:max(v);
wfit = min(w):.1:max(w);
[vFIT,wFIT] = meshgrid(vfit,wfit);
YFIT = bLin(1) + bLin(2)*vFIT + bLin(3)*wFIT + bLin(4)*vFIT.*wFIT;
mesh(vFIT,wFIT,YFIT);
hidden('off')
title('linear regression (v, w, v*w) -> lin')
xlabel('v'); ylabel('w'); zlabel('lin')

% linear regression (v, w, v*w) -> ang
X=[ones(size(v)), v, w, v .* w];
[bAng, sigma, r]=ols(ang, X);
bAng
%~ bAng =  0.98163 -1.82084 11.69517 0.36455
normR=norm(r)
[1 8 8 8*8]*bAng       % simple prediction
angEst=X*bAng          % check model with initial data

figure
scatter3(v, w, ang, 'filled');
hold on
vfit = min(v):.1:max(v);
wfit = min(w):.1:max(w);
[vFIT,wFIT] = meshgrid(vfit,wfit);
YFIT = bAng(1) + bAng(2)*vFIT + bAng(3)*wFIT + bAng(4)*vFIT.*wFIT;
mesh(vFIT,wFIT,YFIT);
hidden('off')
title('linear regression (v, w, v*w) -> ang')
xlabel('v'); ylabel('w'); zlabel('ang')
pause
