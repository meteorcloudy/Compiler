begin
  integer k; 
  
  integer function F(n);
    begin 
      integer x;
      integer n;

        integer function Print(y);
        begin
          integer y;
          y := y*100;
          write(y)
        end;

      if n<=0 then F:=1 else F:=n*F(n-1)       
    end;

  integer function Good(z);
    begin
      integer z;
      read(z)
    end;
  integer m;
  
  read(m);
  k:=F(m);
  m:=Good(k);
  write(k)
end
