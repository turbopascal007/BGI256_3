
{How good is your floodfill routine?}
{Can it survive this obstical course?}
{This program generates a worst case pattern for floodfill}
{then calls the floodfill routine to fill the pattern.}
{A badly implemented floodfill will blowup}
{a poorly implemented floodfill will leave holes}
{a typical floodfill will run out of memory and give up }
{ part way through filling the pattern.}
{A fantastic floodfill will fill the whole pattern.}
{Also notice the little color squares at the top or left of the pattern}
{ this shows the type of floodfill you have. If the colors get }
{ overwritten, then you have a border bound floodfill.}
{ If the color squares remain intact, then you have a}
{ seed bound flood fill.}
{ The floodfill should leak out of the hole in the upper left corner}
{ of the inner rectangle, and should be clipped to the middle area}
{ between the two rectangles. If the floodfill fills the entire}
{ area between the inner and outer rectangles, then it is not}
{ clipping correctly to the viewport.
{If the floodfill does not fill the entire pattern and leaves holes}
{ in the areas to be filled, it means that the floodfill does not}
{ fail gracefully. A good floodfill recognizes that it may run out of}
{ memory and will backup filling what it can on the way back.}
{ a poor floodfill will bomb out and leave holes all over the place.}
{Can your floodfill handle patterns? Can it deal with the edge of the}
{ screen? The area outside the test rectangle should be filled with}
{ a blue crosshatch if the floodfill is working correctly.}
{Finally, if everything went right, the GraphResult printed at the}
{bottom of the screen in yellow should show a value of zero.}
{If you got an error because you ran out of memory, it should say that}

PROGRAM FloodTst;
uses crt,graph,WrMode;
var gd,gm,mm:integer;
    imagearray:array[0..1000] of byte;
    RESULT,error:integer;
    s,ModeName:string;
    StartMode : integer;

function fstr(i:integer):string;
var s:string;
begin
  str(i,s);
  fstr := s;
end;

{$F+}
function AutoDet:integer;
begin
   AutoDet := StartMode;
end;
{$F-}

procedure FillPatternHorz(Bx,By,Sx,Sy:integer);
var ix,iy,xp,yp,xs,ys:integer;
begin
   setviewport(0,0,GetMaxX,GetMaxY,true);
   xs := ((Sx-40) div 8);
   ys := ((Sy-70) div 6);
   setcolor(white);
   rectangle(Bx+0,By+0,Bx+(xs*8)+25,By+(ys*6)+26);
   rectangle(Bx+8,By+8,Bx+(xs*8)+17,By+(ys*6)+18);
   putpixel(Bx+8,By+10,black);
   OutTextXY(Bx,By+(ys*6)+28,'Horizontal flood test');

   for ix := 1 to pred(xs) do
   begin
     setfillstyle(1,ix);
     bar(Bx+(ix*8)+11,By+11,Bx+(ix*8)+13,By+13);
   end;

   for iy := 1 to ys do
   begin
     for ix := 1 to xs do
     begin
       xp := Bx+(ix*8);
       yp := By+8+(iy*6);
       line(xp,yp,xp+4,yp+4);
       line(xp+4,yp+4,xp+8,yp);
       line(xp+8,yp+3,xp+12,yp+7);
       line(xp+12,yp+7,xp+16,yp+3);
     end;
   end;

  setviewport(Bx+5,By+5,Bx+(xs*8)+20,By+(ys*6)+21,true);
  setcolor(red);
  setfillstyle(SolidFill,2);
{  putpixel(7,15,red); putpixel(7,17,red); putpixel(6,16,red); putpixel(8,16,red); }
   floodfill(7,16,white)

end;


procedure FillPatternVert(Bx,By,Sx,Sy:integer);
var ix,iy,xp,yp,xs,ys:integer;
begin
   setviewport(0,0,GetMaxX,GetMaxY,true);
   xs := ((Sx-50) div 6);
   ys := ((Sy-60) div 8);
   setcolor(white);
   rectangle(Bx+0,By+0,Bx+(xs*6)+28,By+(ys*8)+21);
   rectangle(Bx+8,By+8,Bx+(xs*6)+20,By+(ys*8)+13);
   putpixel(Bx+8,By+10,black);
   OutTextXY(Bx,By+(ys*8)+23,'Vertical flood test');

   for iy := 1 to pred(ys) do
   begin
     setfillstyle(1,iy);
     bar(Bx+11,By+(iy*8)+11,Bx+13,By+(iy*8)+13);
   end;

   for iy := 1 to ys do
   begin
     for ix := 1 to xs do
     begin
       xp := Bx+(ix*6)+10;
       yp := By+(iy*8);
       line(xp,yp,xp+4,yp+4);
       line(xp+4,yp+4,xp,yp+8);
       line(xp+3,yp+8,xp+7,yp+12);
       line(xp+6,yp+5,xp+4,yp+7);
      end;
   end;

   setviewport(Bx+5,By+5,Bx+(xs*6)+23,By+(ys*8)+16,true);
   setcolor(red);
   setfillstyle(SolidFill,2);
{  putpixel(17,5,red); putpixel(17,7,red); putpixel(16,6,red); putpixel(18,6,red); }
    floodfill(17,6,white)

end;


procedure FillPatternSpiral(Bx,By,Sx,Sy:integer);
var ix,iy,xp,yp,xs,ys:integer;
begin
   setviewport(0,0,GetMaxX,GetMaxY,true);
   xs := ((Sx-40) div 4);
   ys := ((Sy-70) div 4);
   setcolor(white);
   rectangle(Bx+0,By+0,Bx+(xs*4)+25,By+(ys*4)+26);
   rectangle(Bx+8,By+8,Bx+(xs*4)+17,By+(ys*4)+18);
   putpixel(Bx+8,By+10,black);
   OutTextXY(Bx,By+(ys*4)+28,'Spiral flood test'{   x:'+fstr(xs)+' y:'+fstr(ys)});

   moveto(Bx+((xs*4)div 2)+8,By+((ys*4) div 2)+12);
   ix := 2;
   yp := (((ys*4)+10) div 4) -1;
   for iy := 0 to yp do
   begin
     linerel(ix,0);
     linerel(0,ix);
     inc(ix,2);
     linerel(-ix,0);
     linerel(0,-ix);
     inc(ix,2);
   end;

   for iy := 1 to pred(ys) do
   begin
     setfillstyle(1,iy);
     bar(Bx+11,By+(iy*4)+11,Bx+13,By+(iy*4)+13);
   end;

   setviewport(Bx+5,By+5,Bx+(xs*4)+20,By+(ys*4)+21,true);
   setcolor(red);
   setfillstyle(SolidFill,2);
   xs := (xs*4);
{  putpixel(xs,5,red); putpixel(xs,7,red); putpixel(xs-1,6,red); putpixel(xs+1,6,red); }
    floodfill(xs,6,white)

end;


procedure FillPatternGrill(Bx,By,Sx,Sy:integer);
var ix,iy,xp,yp,xs,ys:integer;
begin
   setviewport(0,0,GetMaxX,GetMaxY,true);
   xs := ((Sx-40) div 4);
   ys := ((Sy-70) div 4);
   setcolor(white);
   rectangle(Bx+0,By+0,Bx+(xs*4)+25,By+(ys*4)+26);
   rectangle(Bx+8,By+8,Bx+(xs*4)+17,By+(ys*4)+18);
   putpixel(Bx+8,By+10,0);
   OutTextXY(Bx,By+(ys*4)+28,'Grill flood test'{   x:'+fstr(xs)+' y:'+fstr(ys)});

{   moveto(Bx+((xs*4)div 2)+8,By+((ys*4) div 2)+12); }
   ix := Bx+20;
   iy := By+10;
   yp := (((ys*4)+10) div 4);
   line(ix,iy+(ys*2),ix+(xs*4)-8,iy+(ys*2));

   for xp := 0 to (xs-2)*2 do
   begin
     line(ix+(xp*2),iy,ix+(xp*2),iy+(ys*4)+6);
   end;


   for iy := 1 to ys do
   begin
     setfillstyle(1,iy);
     bar(Bx+11,By+(iy*4)+11,Bx+13,By+(iy*4)+13);
   end;

   setviewport(Bx+5,By+5,Bx+(xs*4)+20,By+(ys*4)+21,true);
   setcolor(red);
   setfillstyle(SolidFill,2);
   xs := (xs*4);
{  putpixel(16,15,red); putpixel(16,17,red); putpixel(15,16,red); putpixel(17,16,red); }
    floodfill(16,16,white)

end;

procedure FillPatternCircle(Bx,By,Sx,Sy:integer);
var r,ix,iy,xp,yp,xs,ys,xf,yf,pat:integer;
    mc,spz,rx,ry:word;
    a : array[0..1000] of pointtype;
begin
   setviewport(0,0,GetMaxX,GetMaxY,true);
   xs := ((Sx-40) div 4);
   ys := ((Sy-70) div 4);
   setcolor(white);
   rectangle(Bx+0,By+0,Bx+(xs*4)+25,By+(ys*4)+26);
   rectangle(Bx+8,By+8,Bx+(xs*4)+17,By+(ys*4)+18);
   putpixel(Bx+8,By+10,0);
   OutTextXY(Bx,By+(ys*4)+28,'Circle flood test'{   x:'+fstr(xs)+' y:'+fstr(ys)});

   for iy := 1 to ys do
   begin
     setfillstyle(1,iy);
     bar(Bx+11,By+(iy*4)+11,Bx+13,By+(iy*4)+13);
   end;

   setviewport(Bx+5,By+5,Bx+(xs*4)+20,By+(ys*4)+21,true);
   ix := ((xs*4)+15) div 2;
   iy := ((ys*4)+16) div 2;
   getaspectratio(rx,ry);

   xp := trunc(iy/(rx/ry));

   spz := 7;
   mc := succ(iy div spz);
   for yp := 1 to MC do
   begin
     r := pred(xp - (yp*spz));
     circle(ix,iy,r);
     a[yp].x := ix - r + 2;
     a[yp].y := iy;
   end;

  a[0].x := 16;
  a[0].y := 16;
  for yp := MC downto 0 do
  begin
    if yp = 0 then xp := green else xp := yp;
    setcolor(xp);
    if yp = 0 then pat := 1 else pat := yp mod 12;

    setfillstyle(pat,xp);
    xf := a[yp].x;
    yf := a[yp].y;
  {  putpixel(xf,yf-1,red); putpixel(xf,yf+1,red); putpixel(xf-1,yf,red); putpixel(xf+1,yf,red); }

    floodfill(xf,yf,white)
  end;
end;

procedure FillOutside(Clipper:boolean);
begin
  setcolor(yellow);
  error := graphresult;
  setfillstyle(XhatchFill,blue);
  setviewport(0,0,GetMaxX,GetMaxY,false);
  rectangle(4,4,GetMaxX-4,GetMaxY-22);
  floodfill(GetMaxX,GetMaxY,white);
  outtextxy(20,GetMaxY-8,'GraphResult:'+fstr(Error)+' '+GraphErrorMsg(Error));
  ModeName := GetModeName(gm);
  outtextxy(20,GetMaxY-17,ModeName);
end;

var temp : integer;

BEGIN
  if ParamCount > 0 then
  begin
    s := ParamStr(1);
    if (s[1] > '1') and (s[1] < '9') then
      StartMode := ord(s[1]) and $f;
  end;
  gd := 0;
  gm := StartMode;
  result := installuserdriver({'EGAVGA'}{'VESA16'}'BGI256',{ @AutoDet}nil);
  ERROR := graphresult;

  setgraphbufsize(65520);
  gd := result;
  initgraph(gd,gm,'');

  SetWriteMode(TextMode+MoveWrite);
  setwritemode(FloodFillType+AutoFill);
  SetWriteMode(FloodFillType+BorderFill);
  SetWriteMode(FillMode+MoveWrite);

{  setwritemode(FloodFillType+SeedFill); }
{  setwritemode(FloodFillType+ComplexFill); }
{  setwritemode(FloodFillType+FillDelayOn);  }
{  setwritemode(FloodFillType+FillTracerOn); }

(*
  setcolor(5);
  setfillstyle(1,1);  {test for seed/border match}
  bar(10,10,100,100); {should stop immediately}
  setcolor(red);      {ie, nothing should happen}
  setfillstyle(1,red);
  bar(50,50,50,50);
  rectangle(9,9,101,101);
  setfillstyle(1,7);
  floodfill(50,50,red);
  readln;
  halt(1);
*)


  cleardevice;
  setcolor(white);
  FillPatternHorz(10,10,GetMaxX,GetMaxY);
(*
  setcolor(white);
  rectangle(0,0,GetMaxX,GetMaxY);
*)
  setwritemode(FillMode+xorWrite);
  FillOutside(false);
  readln;

  cleardevice;
  setwritemode(FloodFillType+SeedFill);
  setwritemode(FillMode+MoveWrite);
  FillPatternVert(10,10,GetMaxX,GetMaxY);
  setwritemode(FloodFillType+SeedFill);
  setwritemode(FillMode+OrWrite);
  FillOutside(false);
  readln;

  cleardevice;
  setwritemode(FloodFillType+BorderFill);
  SetWriteMode(FillMode+MoveWrite);
  FillPatternSpiral(10,10,GetMaxX,GetMaxY);
  setwritemode(FloodFillType+BorderFill);
  setwritemode(FillMode+NotMoveWrite);
  FillOutside(false);
  readln;

  cleardevice;
  setwritemode(FloodFillType+BorderFill);
  SetWriteMode(FillMode+MoveWrite);
  FillPatternGrill(10,10,GetMaxX,GetMaxY);
  setwritemode(FloodFillType+SeedFill);
  setwritemode(FillMode+NotXorWrite);
  FillOutside(false);
  readln;

  setwritemode(FloodFillType+BorderFill);
  setwritemode(FloodFillType+AutoFill);
  setwritemode(FloodFillType+FillDelayOn);
  setwritemode(FloodFillType+FillTracerOn);

  for mm := 0 to 15 do
  begin
    cleardevice;
    SetWriteMode(FillMode+MoveWrite);
    setwritemode(FloodFillType+mm);
    FillPatternCircle(10,10,GetMaxX,GetMaxY);
    setwritemode(FillMode+XorWrite);
    FillOutside(false);
    delay(500);
  end;
  readln;
  closegraph;

END.

