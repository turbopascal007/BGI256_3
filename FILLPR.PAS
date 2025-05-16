(*
Written by Michael Day as of 30 Jan 1993
Adapted from the following program:

/***************************************************************************
 * Pattern sensitivity test program -- Microsoft 'C' Graphics Library
 *
 * PROBLEM: The _floodfill() library code has pattern sensitive problems.
 *
 *   1.  It may appear to pause while performing excessive calculations.
 *   2.  It may not complete the fill pattern over the allowed area.
 *   3.  It may go into an endless loop - requiring system reboot.
 *
 * This program illustrates all three examples on any system with a CGA
 * (Color Graphics Adapter).
 *
 * Submitted by:  Steve Hathaway      (CompuServ ID = 71237,14)
 *                Hathaway Computer Service
 *                PO Box 25
 *                Wilsonville, OR 97070
 */
*)

program fillpr;
uses CRT,graph,wrmode;

(*
/****************
 * TEST PATTERNS
 */
*)

type maskarray = array[0..7] of byte;
     patternmask = array [0..11] of maskarray;

const viaFlood : boolean = true;
      DoCircle : boolean = true;
      DoPoly   : boolean = false;
      done : boolean = false;
      backgroundcolor : word = 0;

const lstyl : array [0..11] of word = (
         $FFFF, $AAAA, $8888, $CCCC,
         $EEEE, $F6F6, $F249, $FCFC,
         $F39C, $F7BC, $E64C, $FCCC );

      fmask : patternmask = (
         ($FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF),
         ($AA, $55, $AA, $55, $AA, $55, $AA, $55),
         ($CC, $66, $33, $99, $CC, $66, $33, $99),
         ($AA, $AA, $AA, $AA, $AA, $AA, $AA, $AA),
         ($FF, $00, $FF, $00, $FF, $00, $FF, $00),
         ($C8, $32, $8C, $23, $C8, $32, $8C, $23),
         ($66, $00, $66, $00, $66, $00, $66, $00),
         ($88, $22, $88, $22, $88, $22, $88, $22),
         ($CC, $33, $CC, $33, $CC, $33, $CC, $33),
         ($99, $42, $24, $99, $99, $24, $42, $99),
         ($FE, $7C, $38, $10, $08, $1C, $3E, $7F),
         ($81, $42, $24, $18, $18, $24, $42, $81)
         );

const MaxPoly = 50;
var  Pcnt : word;
     Poly : array[1..MaxPoly] of PointType;

var mode, ixs, ixm, iwm, t : word;
    s:string;
    key:char;

{/***********************************
 * PATTERN SENSITIVITY TEST FUNCTION
 */}

function fstr(w:word):string;
var s:string;
begin
  str(w,s);
  fstr := s;
end;

const Hex : array[0..15] of char = '0123456789ABCDEF';
function hexw(w:word):string;
begin
  hexw[1] := hex[(w shr 12) and $f];
  hexw[2] := hex[(w shr 8)  and $f];
  hexw[3] := hex[(w shr 4)  and $f];
  hexw[4] := hex[ w and $f];
  hexw[0] := #4;
end;

function hexb(b:byte):string;
begin
  hexb[1] := hex[b shr  4];
  hexb[2] := hex[b and $f];
  hexb[0] := #2;
end;

procedure InitPoly(which:byte);
var x1,x2,y1,y2,x,y,h,w:integer;
begin
    Pcnt := 5;
    x1 := 100;
    x2 := GetMaxX-100;
    y1 := 20;
    y2 := GetMaxY-20;
    W := (x2-x1) div 9;
    H := (y2-y1) div 8;
    X := x1+((x2-x1) div 2) - round(2.5 * W);
    Y := y1+((y2-y1) div 2) - (3 * H);

    { Border around viewport is outer part of polygon }
    Poly[1].X := x1;       Poly[1].Y := y1;
    Poly[2].X := x1+x2-x1; Poly[2].Y := y1;
    Poly[3].X := x1+x2-x1; Poly[3].Y := y1+y2-y1;
    Poly[4].X := x1;       Poly[4].Y := y1+y2-y1;
    Poly[5].X := x1;       Poly[5].Y := y1;

    Poly[6].X := X;        Poly[6].Y := Y+H;
    Poly[7].X := X+W;      Poly[7].Y := Y;
    Poly[8].X := X+(5*W);  Poly[8].Y := Y;

    Poly[9].X := X+(5*W);  Poly[9].Y := Y+(5*H);
    Poly[10].X := X+(4*W); Poly[10].Y := Y+(6*H);
    Poly[11].X := X;       Poly[11].Y := Y+(6*H);
    Poly[12].X := X;       Poly[12].Y := Y+H;
    Pcnt := 12;

    if Which > 0 then
    begin
      Poly[9].X := X+(5*W);  Poly[9].Y := Y;
      Poly[10].X := X+W;     Poly[10].Y := Y+(H*4);
      Poly[11].X := X+W;     Poly[11].Y := Y+(H*5);
      Poly[12].X := X+(4*W); Poly[12].Y := Y+(H*5);
      Poly[13].X := X+(4*W); Poly[13].Y := Y+(H*3);
      Poly[14].X := X+(3*W); Poly[14].Y := Y+(H*4);
      Poly[15].X := X+(2*W); Poly[15].Y := Y+(H*4);
      Poly[16].X := X+(5*W); Poly[16].Y := Y+H;
      Poly[17].X := X+(5*W); Poly[17].Y := Y+(H*5);
      Poly[18].X := X+(4*W); Poly[18].Y := Y+(H*6);
      Poly[19].X := X;       Poly[19].Y := Y+(H*6);
      Poly[20].X := X;       Poly[20].Y := Y+(H*4);
      Poly[21].X := X+(3*W); Poly[21].Y := Y+H;
      Poly[22].X := X+(2*W); Poly[22].Y := Y+H;
      Poly[23].X := X;       Poly[23].Y := Y+(H*3);
      Poly[24].X := X;       Poly[24].Y := Y+(H*2);
      Poly[25].X := X+(W);   Poly[25].Y := Y+(H);
      Poly[26].X := X;       Poly[26].Y := Y+H;
      Pcnt := 26;
    end;

end;

{------------------------------------------------------------}
procedure testpattern(styl:word; mask:maskarray; color:byte);
var Xa,Ya,R,x,y,i,ii,md,bc,Px,Py,Sy : word;
    buf : array[0..16] of byte;
    uStk,fStk,tStk:word;
begin
    color := succ(color mod GetMaxColor);
    GetAspectRatio(Xa,Ya);
    x := GetMaxX div 2;
    for y := (GetMaxY div 2) to (GetMaxY div 2)+7 do {/** loop through 8 vertical locations **/ }
    begin
       if keypressed then Exit;
       cleardevice;

       setcolor(backgroundcolor);
       setwritemode(FillMode+SetBackColor);
       SETWRITEMODE(MiscCommand+GetXYStackPeak);
       uStk := GETMAXMODE;
       SETWRITEMODE(MiscCommand+GetXYStackFree);
       fStk := GETMAXMODE;
       SetWriteMode(MiscCommand+GetBackColor);
       bc := GetMaxMode;

       setcolor(white);
       outtextxy(128,0,'BGI ');
       if viaFlood then
         outtextxy(160,0,'Flood ')
       else
         outtextxy(160,0,'Fill  ');
       if DoPoly then
         outtextxy(208,0,'Poly   ');
       if DoCircle then
         outtextxy(208,0,'Circle ');

       outtextxy(0,0,'X:'+fstr(GetMaxX+1)+' Y:'+fstr(GetMaxY+1));
       OUTTEXTXY(0,10,'uS:'+fstr(uStk));  {show previous stack usage}
       OUTTEXTXY(0,20,'fS:'+fstr(fStk));  {show previous free stack space}

       outtextxy(0,GetMaxY-80,'Bc:'+fstr(bc));
       outtextxy(0,GetMaxY-70,'Fc:'+fstr(color));
       outtextxy(0,GetMaxY-60,'Ps:'+fstr(ixm));
       outtextxy(0,GetMaxY-50,'Ls:'+fstr(ixs));
       outtextxy(0,GetMaxY-40,'Wmode:'+fstr(iwm));
       outtextxy(0,GetMaxY-30,'Y:'+fstr(y)+' ('+fstr(y-(GetMaxY div 2))+')');
       outtextxy(0,GetMaxY-20,'L:'+hexw(styl)+'  ');
       moveto(0,GetMaxY-10);
       outtext('M:');
       for i := 0 to 6 do
         outtext(hexb(mask[i])+',');
       outtext(hexb(mask[7])+' ');

       setwritemode(LineMode+ForeMoveWrite);
       setcolor(white);
       setlinestyle(UserBitLn,styl,1);
       moveto(0,y);
       lineto(getMaxX,y);

       Px := GetMaxX div 2;
       Py := GetMaxY div 2;
       if DoPoly then
         Sy := 30
       else
         Sy := GetMaxY div 3;

     {  putpixel(Px+1,Sy,red); putpixel(Px-1,Sy,red);
       putpixel(Px,Sy+1,red); putpixel(Px,Sy-1,red); }

       setfillpattern(fillpatterntype(mask),color);
       setwritemode(LineMode+MoveWrite);

       r := GetMaxX div 3;
       if DoCircle then
       begin
         if viaFlood then
           circle(Px,Py,Py)
         else
           fillellipse(Px,Py,R,R*longint(Xa) div Ya);
       end;

       if DoPoly then
       begin
         if viaFlood then
         begin
           setlinestyle(SolidLn,0,1);
           DrawPoly(Pcnt,Poly);
         end
         else
         begin
           FillPoly(Pcnt,Poly);
         end;
       end;

       if viaFlood then
       begin
         floodfill(Px,Sy,white)
       end;

       if GraphResult <> 0 then
       begin
         outtextxy(GetMaxX-60,0,'*Error*');
         delay(500);
       end;

  {     delay(200); }

    end;
end;

{/*****************************************
 * PATTERN SENSITIVITY TEST - MAIN PROGRAM
 */}

function autodet:integer; far;
begin
  autodet := mode;
end;


var gd,gm:integer;
begin
 mode := 128;
 if paramcount > 0 then
 begin
   s := paramstr(1);
   mode := ord(s[1]) and $f;
 end;

 SetGraphBufSize(10000);
 gm := 0;
 gd := 0;            {/*** GRAPHICS VIDEO MODE TO TEST ***/}
 InstallUserDriver('BGI256',@autodet);
{ gd := cga; }
 InitGraph(gd,gm,'');
 InitPoly(0);
 setcolor(white);
 setWritemode(FloodFillType+BorderFill);    {init for BGI256 if out there}
 setwritemode(FloodFillType+FillCompressOn);
{ setWritemode(FloodFillType+ComplexFill); }
{ setwritemode(FloodFillType+FillDelayOn);   }
{ setwritemode(FloodFillType+FillTracerOn); }

  done := false;
  FOR IWM := 0 TO 23 DO
  BEGIN
    SetWriteMode(FillMode+Iwm);   {fill mode for BGI256 if out there}
    for ixm := 0 to 11 do
    begin
      for ixs := 0 to 11 do
      begin
        if not(done) then
          testpattern(lstyl[ixs], fmask[ixm], ixs+1);
        if keypressed then
        begin
          key := readkey;
          if key = #0 then key := char(ord(readkey)+$80);
          if key = #$1b then done := true;
          case upcase(key) of
            'F' : begin
                    viaFlood := not(viaFlood);
                  end;
            'P' : begin
                    DoPoly := true;
                    DoCircle := false;
                  end;
            'C' : begin
                    DoCircle := true;
                    DoPoly := false;
                  end;
            'S' : begin
                    SetWriteMode(FloodFillType+SeedFill)
                  end;
            'B' : begin
                    SetWriteMode(FloodFillType+BorderFill)
                  end;
            'N' : InitPoly(0);
            'M' : InitPoly(1);
            '0'..'9': begin
                        BackGroundcolor := ord(key) and $f;
                      end;
          end; {case}
        end;
      end;
    end;
  END;

 closegraph;          { /*** RESTORE VIDEO MODE IF SUCCESS ***/}
 writeln('Done');     { /** otherwise system reboot required */}
end.
