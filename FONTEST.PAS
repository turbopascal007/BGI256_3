
{program to test the use of externally loaded BGI fonts}
{Writen by Michael Day as of 12/26/92 - public domain}

program FonTest;

{$IFDEF WINDOWS}
  {$DEFINE PMODE}
  --> This program doesn't work in Windows
{$ENDIF}
{$IFDEF DPMI}
  uses WinApi,dos,graph;
  {$DEFINE PMODE}
{$ENDIF}
{$IFNDEF PMODE}
  uses dos,graph;
{$ENDIF}

type  FontDef = array[0..7] of byte;
      FontArray = array[0..255] of FontDef;
      ByteArray = array[0..65520] of byte;
      BytePtr = ^ByteArray;

const FontFileName = 'FNTIMG.DAT';  {user defined bit-map font}
      BGIFileName = 'BGI256.BGI';

const FontSelector : longint = 0;   {protected mode font selector}
      FontPtr  : ^FontArray = nil;  {pointer to our font table}
      BGIptr   : BytePtr = nil;     {pointer to the BGI driver}
      Old1fPtr : pointer = nil;     {storage for old $1f int vec}
      RealIntOfs : word = 0;        {storage for old real mode}
      RealIntSeg : word = 0;        { in vect when in protected mode}
      BGISize : word = 0;           {size of bgi driver}
      StartMode : integer = 0;      {startup mode}

var   gd,gm : integer;
      err : integer;
      s : string;


{----------------------------------------------------------------}

function AutoDet:integer; far;
begin
  AutoDet := StartMode;
end;


{$IFDEF PMODE}
procedure GetRealIntVec(Inum:byte; var OfsVal,SegVal:word);
begin
   asm
     mov ax,$0200   {in pmode we have to use dpmi services}
     mov bl,[Inum]  {to get the real mode interrupt}
     int $31
     les bx,[OfsVal]
     mov es:[bx],dx
     les bx,[SegVal]
     mov es:[bx],cx
   end;
end;

procedure SetRealIntVec(Inum:byte; OfsVal,SegVal:word);
begin
   asm
     mov ax,$0201   {in pmode we have to use dpmi services}
     mov bl,[Inum]  {to set the real mode interrupt}
     mov dx,[OfsVal]
     mov cx,[SegVal]
     int $31
   end;
end;
{$ENDIF}

{---------------------------------------------------------}
{Load the font definition file}
function LoadFont(FontFileName:string):boolean;
var f : file of FontArray;
begin
   LoadFont := false;
   {$IFDEF PMODE}       {if pmode, alloc the data in dos memory area}
     if FontSelector = 0 then
       FontSelector := GlobalDosAlloc(sizeof(FontPtr^));
     if FontSelector = 0 then Exit;
     FontPtr := ptr(loword(FontSelector),0);
   {$ELSE}
     if FontPtr = nil then
     begin
       if MaxAvail < sizeof(FontPtr^) then Exit;
       New(FontPtr);    {if real mode just grab it off the heap}
     end;
   {$ENDIF}

   if IOResult = 0 then {nop};
   assign(f,FontFileName);
   reset(f);
   Read(f,FontPtr^);    {read the font into memory}
   close(f);
   if IOResult = 0 then  
   begin               {if valid file point the int vec at the font}
     {$IFDEF PMODE}
       SetRealIntVec($1f,Ofs(FontPtr^[128]),hiWord(FontSelector));
     {$ENDIF}
     SetIntVec($1f,@FontPtr^[128]);
     LoadFont := true;
   end;
end;

{-------------------------------------------------}
{load the bgi driver into memory}
function LoadBGI(DriverName:string):boolean;
var f:file;
    Count:word;
begin
   LoadBGI := false;
   assign(f,DriverName);
   reset(f,1);
   if (BGIptr <> nil) and (BGISize <> 0) then
   begin
     FreeMem(BGIptr,BGISize);
   end;

   BGISize := FileSize(f);
   if MaxAvail < BGISize then
   begin
     close(f);
     Exit;
   end;
   GetMem(BGIptr,BGISize);
   BlockRead(f,BGIptr^,BGISize,Count);
   close(f);
   LoadBGI := true;
end;


procedure ShowFont;
var i,x,y:integer;
begin
  x := 1;
  y := 1;
  for i := 0 to 255 do
  begin
    outtextxy(x,y,char(i));
    x := x + 10;
    if i and $1f = $1f then
    begin
      y := y + 10;
      x := 1;
    end;
  end;
end;


{--------------------------------------------}
{release resources and turn things off}
procedure ShutDown;
begin
  closegraph;    {turn off graphics mode}
  if (BGIptr <> nil) and (BGISize <> 0) then
    FreeMem(BGIptr,BGISize);  {release bgi driver memory}
  SetIntVec($1f,Old1fPtr);    {restore old int vect ptr}
  {$IFDEF PMODE}
    SetRealIntVec($1f,RealIntOfs,RealIntSeg);
    if FontSelector <> 0 then {release font dos memory}
       FontSelector := GlobalDosFree(sizeof(FontPtr^));
    FontPtr := nil;
  {$ELSE}
    if FontPtr <> nil then
      Dispose(FontPtr);       {release heap allocation for font}
    FontPtr := nil;
  {$ENDIF}
end;


var i,x,y,z,q:integer;
    c : byte;
    cp : ^Fontarray;

{-Main-}
begin
  if ParamCount > 0 then
    S := ParamStr(1);
  StartMode := ord(s[1]) and $f;

  {$IFDEF PMODE}
    GetRealIntVec($1f,RealIntOfs,RealIntSeg);  {save current vect}
  {$ENDIF}
  GetIntVec($1f,Old1fptr);
  if not LoadFont(FontFileName) then
  begin
    writeln('Error; Could not load font file: ',FontFileName);
    halt(1);
  end;

  if not LoadBGI(BGIFileName) then
  begin
    writeln('Error: Could not load BGI file: ',BGIFileName);
    Halt(1);
  end
  else
  begin
    gd := InstallUserDriver('BGI256',@AutoDet);
    gd := RegisterBGIdriver(BGIptr);   {register our own driver}
    if gd < 0 then
    begin
      writeln('Error: Could not register BGI driver: ',BGIFileName);
      {$IFDEF PMODE}
        {all bgi drivers have the version number at this location}
        if BGIptr^[$86] < 3 then
          writeln(' ** Wrong driver version for Pmode operation **');
      {$ENDIF}
      Halt(1);
    end;
  end;

  gd := 0;
  gm := 0;
  initgraph(gd,gm,'');   {startup the graphics system}
  err := GraphResult;
  if err <> 0 then
  begin
    writeln('Error: Could not init graphics device: ');
    writeln(GraphErrorMsg(Err));
    halt(1);
  end;

  ShowFont;              {show 'em what we got}

(*
  cp := pointer(FontPtr);  {in case you want to see in it a different}
  for i := 0 to 255 do     {way, I provided this. It is useful for}
  begin                    {debugging to prove what is in the font}
    for q := 0 to 7 do     {table that we loaded. Especially useful}
    begin                  {for protected mode when you forget about}
      for z := 0 to 7 do   {real mode requirement for the interrupt}
      begin
        if ((q = 0) and (z = 0)) or ((q = 7) and (z = 7)) or
           ((q = 0) and (z = 7)) or ((q = 7) and (z = 0)) then c := 1 else
        c := ((cp^[i][q] shl z) and $80) shr 6;
        y := ((i shr 5) * 10) + q + 100;
        x := ((i and $1f) * 10) + z;
        putpixel(x,y,c);
      end;
    end;
  end;
*)

  readln;   {hang around until return is hit}

  ShutDown; {close up shop and go home}
end.

