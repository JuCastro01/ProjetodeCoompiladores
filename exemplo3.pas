{-
programa calcula o fatorial de um numero lido
-}
program fatorial;
integer fat,num,cont;
begin
read(num);
set fat to 0b1;
for cont of 0b10 to num:
set fat to fat * cont;
write(fat) # imprime o fatorial calculado
end.