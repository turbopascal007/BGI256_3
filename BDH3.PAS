{ BDH V3.0 - writes header portion of BGI driver }
{ Copyright (c) 1991,1992 Knight Software        }
{ 28 Dec 92 - fixed prob with trailing in copyright file}
{ 30 Jan 93 - fixed prob with gen V2.00 drivers (bad header)}

{$I-}
USES DOS;

CONST
  VERSION      : byte = 3;    {Version Number of header}
  REVISION     : byte = 0;    {Revision number of header}
  SUBREV       : byte = 1;    {Knight Software revision}
  MIN_VERSION  : byte = 1;    {Minimum Version Number}
  MIN_REVISION : byte = 0;    {Minimum Revision Number}
  HEADER_SIZE  : word = 160;  {Size (in bytes) of header}
  drv_num      : word = 0;    {driver number (not used)}
  Null         : byte = 0;    {Null definition}
CONST
  HEADER_TEXT = 110;          {Max header text length (bytes)}
  SubRev2 = 1;                {Knight Software SubRev for V2.01}
  SubRev3 = 1;                {Knight Software SubRev for V3.01}

TYPE
  ByteArray = array[0..65520] of byte;
  String12  = string[12];
  String3   = string[3];

VAR
  Dname       : String[8];    {Dname is exactly eight char}
  Tname,Mname : String;
  Iname,Oname : String;
  Hname,Stemp : String;
  ISize,OSize : integer;
  HSize       : integer;
  i,Error     : integer;
  Result      : word;
  Ifile,Ofile : file;
  Hfile       : file;
  Mfile       : text;
  Fptr        : ^ByteArray;
  Hdata       : array [0..HEADER_TEXT] of char;
  CodeSize    : word;
  DataSize    : word;
  DataOfs     : word;
  CreateOK    : boolean;
  Year,Month,Day,DoW : word;

CONST
  CopyRightFile = 'COPYRITE.DAT';  {Copyright file name}
  Header1 = 'BGI Device Driver';
  Header2 = 'Copyright (c) 1991,1993 Knight Software';

  MonthStr : array[0..12] of string3 =
    ('   ','Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec');

{----------------------------------------------------------------}
{convert hex number in S to binary value in Result}
{returns false if bad hex number, or none found}
function GetHex(S:string; var Result:word):boolean;
var i,j,k:integer;
begin
  GetHex := false;
  j := 0;
  Result := 0;
  k := pos('H',S);
  if k = 0 then Exit;
  for i := 1 to pred(k) do
  begin
    if S[i] > ' ' then
    begin
      if ((S[i] > '9') and (S[i] < 'A')) or (S[i] < '0') or (S[i] > 'F') then Exit;
      if S[i] > '9' then S[i] := char(ord(S[i])-7);
      Result := (Result*16)+(ord(S[i])and $f);
      inc(j);
    end;
  end;
  if j > 0 then
    GetHex := true;
end;

{----------------------------------------------------------------}
function fstr(I:integer):string;
var S:string;
begin
  str(I,S);
  fstr := S;
end;

function fstr2(I:integer):string;
var S:string;
begin
  str(I,S);
  if length(S) < 2 then S := '0'+S;
  fstr2 := S;
end;

{----------------------------------------------------------------}
{ mainline code here }
begin
  writeln('BGI Driver Builder V3.04 - Feb 08 1993 - Copyright 1992,1993 Knight Software');
  if ParamCount < 1 then
  begin
    writeln('Usage is:');
    writeln('  BDH [drv_name] [input.BIN] [output.BGI] [mapfile.MAP] [version]');
    writeln('    [drv_name] is the device name (i.e. EGA or CGA)');
    writeln('    [input.BIN] is the DRIVER.BIN from EXE2BIN.');
    writeln('    [output.BGI] is the DRIVER.BGI file name.');
    writeln('    [mapfile.MAP] is the DRIVER.MAP file name.');
    writeln('    [version] is the version number of the driver.');
    Halt(1);
  end;
  {note: Map file is not actually read if building V2.00, but a filename}
  {is required as a place holder if you want to specify the version number}

  {--------------------------------------------------------------}
  {get device name - this is the name in () in the header file   }
  fillchar(Dname,length(Dname),0);         {Clear Dname to zeros}
  Dname := ParamStr(1);                    {Driver name from cmdline}
  for i := 1 to length(Dname) do           {Convert name to uppercase}
    Dname[i] := UpCase(Dname[i]);
  Tname := Dname;

  {--------------------------------------------------------------}
  {get input file name - if no input file name given, use header name}
  Iname := Tname;
  if ParamCount > 1 then
    Iname := ParamStr(2);
  if pos('.',Iname) <> 0 then
    Iname := copy(Iname,1,pred(pos('.',Iname)));
  Tname := Iname;
  Mname := Iname;                          {map file may use input name}
  Iname := Iname+'.BIN';
  Assign(Ifile, Iname);
  Reset(Ifile, 1);                         {Open input file}
  Error := IOresult;
  if Error <> 0 then                       {Did the open suceed?}
  begin
    writeln(Error,' ERROR: Could not open input file ',Iname);
    Halt(2);                               {Leave the program}
  end;
  ISize := FileSize(Ifile);                {Get length of the file}
  reset(Ifile, 1);                         {Reset the input file}
  GetMem(Fptr, ISize);
  blockread(Ifile, Fptr^, ISize, Result);  {Read source byte}
  Close(Ifile);			                       {Close input file}
  Error := IOresult;
  if (Error <> 0) or (Result <> ISize) then
  begin
    Writeln(Error,' Error reading input file ',Iname);
    Halt(4);
  end;

  {--------------------------------------------------------------}
  {get output file name - if no output file name given, use input name}
  Oname := Tname;
  if ParamCount > 2 then
    Oname := ParamStr(3);
  if pos('.',Oname) <> 0 then
    Oname := copy(Oname,1,pred(pos('.',Oname)));
  Tname := Oname;
  Oname := Oname+'.BGI';
  Assign(Ofile, Oname);
  Rewrite(Ofile, 1);                       {Open output file}
  Error := IOresult;
  if Error <> 0 then                       {Did the open suceed?}
  begin
    writeln(Error,' ERROR: Could not open output file ',Oname);
    Halt(3);                               {Leave the program}
  end;

  {--------------------------------------------------------------}
  {get map file name - if no map file name given, use input name}
  if ParamCount > 3 then
    Mname := ParamStr(4);
  if pos('.',Mname) <> 0 then
    Mname := copy(Mname,1,pred(pos('.',Mname)));
  Mname := Mname+'.MAP';
  {--------------------------------------------------------------}
  {get the version number - if no version number given, use default}
  if ParamCount > 4 then
  begin
    Stemp := ParamStr(5);
    if Stemp[1] = '2' then
    begin
       VERSION := 2;
       REVISION := 0;
       SUBREV := SubRev2;
       MIN_VERSION := 1;
       MIN_REVISION := 0;
    end
    else if Stemp[1] = '3' then
    begin
       VERSION := 3;
       REVISION := 0;
       SUBREV := SubRev3;
       MIN_VERSION := 1;
       MIN_REVISION := 0;
    end
    else
    begin
      writeln('ERROR: Bad version number: ',Stemp);
      Halt(5);
    end;
  end;

  CreateOK := false;
  if Fptr^[15] <> VERSION then
  begin
    writeln('WARNING: The version number of the binary file (',Fptr^[15],') does not match');
    writeln(' the version specified (',VERSION,'). Please double check the code for validity.');

    if (Fptr^[0] = $1E {pushds}) and (Fptr^[1] = $2E {segcs}) and  {check for V3.00}
       (Fptr^[2] = $8E) and (Fptr^[3] = $1E {movds,[alias]}) then
    begin
       VERSION := 3;
       REVISION := 0;      {declar it as a V3.00 file}
       MIN_VERSION := 3;
       MIN_REVISION := 0;
       if (Fptr^[15] = 0) then
       begin
         writeln('The binary file appears to have been created from the Borland toolkit');
         writeln('for the version 3.00 driver.');
         SUBREV := 0;
         CreateOK := true;
       end
       else if (Fptr^[15] = 3) then
       begin
         SUBREV := SubRev3;   {looks like our V3.00 binary, so assume so}
         CreateOK := true;
       end;
       {else if what ever it is, it isn't us so don't create it}
    end
    else if (Fptr^[0] = $1E {pushds}) and (Fptr^[1] = $0E {pushcs}) and
            (Fptr^[2] = $1F {popds}) and (Fptr^[3] = $FC {cld}) then
    begin  {smells like a V2.0 binary}
       VERSION := 2;
       REVISION := 0;
       if (Fptr^[12] = ord('C')) and (Fptr^[13] = ord('B')) and (Fptr^[15] = 0) then
       begin
         writeln('Binary file appears to be an original Version 2.00 binary.');
         SUBREV := 0;
         MIN_VERSION := 1;
         MIN_REVISION := 0;
         CreateOK := true;
       end
       else if (Fptr^[15] = 2) then  {looks like our V2.00 binary,}
       begin                          {so assume so}
         SUBREV := SubRev2;
         MIN_VERSION := 1;
         MIN_REVISION := 0;
         CreateOK := true;
       end;
    end;         {if none of the above, assume the worst and bomb out}

    if CreateOK then
    begin
      write('Assuming: ');
    end
    else
    begin
      writeln('ERROR: The binary file is of an unknown type. Cannot create the driver.');
      Halt(7);
    end;
  end;

  {--------------------------------------------------------------}
  {we only need to do this if we are trying to build Version 3.00 or higher}
  {Ver 2.00 doesn't use a data segment, so we don't need to look for it}

  if VERSION > 2 then {only if > V2.00}
  begin
    Assign(Mfile,Mname);
    Reset(Mfile);                       {Open map file}
    Error := IOresult;
    if Error <> 0 then                       {Did the open suceed?}
    begin
      writeln(Error,' ERROR: Could not open map file ',Mname);
      Halt(3);                               {Leave the program}
    end;
    repeat
      readln(Mfile,Stemp);
      if pos('DATA',Stemp) = 0 then Stemp := '';
    until eof(Mfile) or (Stemp <> '');
    if not(GetHex(Stemp,Result)) then
    begin
      writeln(Error,' ERROR: Bad map file format ',Mname);
      Halt(3);                               {Leave the program}
    end;

    DataOfs := Result;
    CodeSize := DataOfs;
    DataSize := Isize - DataOfs;
  end;

  {--------------------------------------------------------------}
  {get copyright string - if no file found, use internal msg}
  Hname := CopyRightFile;
  Assign(Hfile, Hname);
  Reset(Hfile, 1);                         {Open header file}
  Error := IOresult;
  if Error <> 0 then                       {Did the open suceed?}
  begin
    writeln(Error,' ERROR: Could not open copyright file ',Hname);
    writeln('Using internal copyright message:');
    writeln(Header2);
  end;
  HSize := FileSize(Hfile);                {Get length of the file}
  if HSize > HEADER_TEXT then              {Limit to max size}
    HSize := HEADER_TEXT;
  reset(Hfile, 1);                         {Reset the header file}
  blockread(Hfile, Hdata, HSize, Result);  {Read source byte}
  Close(Hfile);	                           {Close header file}
  Error := IOresult;
  if (Error <> 0) or (Result <> HSize) then
  begin
    Writeln(Error,' Error reading header file ',Hname);
    Halt(4);
  end;
  if HSize > 0 then
  begin
    i := 1;           {truncate any garbage from the end of Hdata}
    while i < HSize do
    begin
      if Hdata[i] < #32 then HSize := i;
      inc(i);
    end;
  end;

  {--------------------------------------------------------------}
  {create text header to stick in front of the driver}
  GetDate(Year,Month,Day,DoW);
  if VERSION = 2 then
    Stemp := 'pk'+#8+#8  {set the file mark}
  else
    Stemp := 'FBGD'+#8#8#8#8;

  Stemp := Stemp+Header1+' ('+Dname+') '+fstr(VERSION)+'.'+fstr(REVISION)+fstr(SUBREV)+'k  ';
  Stemp := Stemp+MonthStr[Month]+' '+fstr2(Day)+' '+fstr(Year);  {add current date}
  Stemp := Stemp+#13+#10;         {add cr/lf to first line}
  if length(Stemp)+4+Hsize < 127 then
  begin
    for i := 0 to pred(HSize) do             {add in external header msg}
      if not ((HData[i] = #$1A) or (HData[i] = #0)) then
        Stemp := Stemp+HData[i];
  end
  else
  begin
     writeln('ERROR: Copyright file message too long. Using internal copyright message');
     writeln(Header2);
     Stemp := Stemp+Header2;  {tag it with internal copyright notice}
  end;
  Stemp := Stemp+#13#10#0#$1A;         {finish it with null, ^Z}

  {--------------------------------------------------------------}
  {now we create the actual file}
  writeln('Creating ',Dname,' V',VERSION,'.',REVISION,SUBREV,'k  ',
           MonthStr[Month]+' '+fstr2(Day)+' '+fstr(Year),'  BGI driver.');

  blockwrite(Ofile, Stemp[1], length(Stemp)); {write heading info}

  blockwrite(Ofile, word(HEADER_SIZE), 2); {Write size of header}
  blockwrite(Ofile, word(drv_num), 2);     {Write driver number}
  case VERSION of
    2 : begin
          blockwrite(Ofile, word(ISize), 2);       {Size (in bytes) of driver}
          blockwrite(Ofile, byte(VERSION), 1);     {Write the version number}
          blockwrite(Ofile, byte(REVISION), 1);    {Write the revision number}
          blockwrite(Ofile, byte(MIN_VERSION), 1); {Write the version number}
          blockwrite(Ofile, byte(MIN_REVISION), 1);{Write the revision number}
        end;
    3 : begin
          blockwrite(Ofile, word(CodeSize), 2);    {Size (in bytes) of code}
          blockwrite(Ofile, byte(VERSION), 1);     {Write the version number}
          blockwrite(Ofile, byte(REVISION), 1);    {Write the revision number}
          blockwrite(Ofile, byte(MIN_VERSION), 1); {Write the version number}
          blockwrite(Ofile, byte(MIN_REVISION), 1);{Write the revision number}
          blockwrite(Ofile, word(DataOfs), 2);     {Offset of data segment}
          blockwrite(Ofile, word(DataSize), 2);    {Size (in bytes) of data}
        end;
  end; {case}

  OSize := FilePos(Ofile);                 {Find loc in output file}
  for i := OSize to $7F do
    blockwrite(Ofile, byte(null), 1);      {Finish title area with 0s}

  blockwrite(Ofile, word(HEADER_SIZE), 2); {Write size of header}
  blockwrite(Ofile, word(drv_num), 2);     {Write driver number}
  case VERSION of
    2 : begin
          blockwrite(Ofile, word(ISize), 2);       {Size (in bytes) of driver}
          blockwrite(Ofile, byte(VERSION), 1);     {Write the version number}
          blockwrite(Ofile, byte(REVISION), 1);    {Write the revision number}
          blockwrite(Ofile, byte(MIN_VERSION), 1); {Write the version number}
          blockwrite(Ofile, byte(MIN_REVISION), 1);{Write the revision number}
        end;
    3 : begin
          blockwrite(Ofile, word(CodeSize), 2);    {Size (in bytes) of code}
          blockwrite(Ofile, byte(VERSION), 1);     {Write the version number}
          blockwrite(Ofile, byte(REVISION), 1);    {Write the revision number}
          blockwrite(Ofile, byte(MIN_VERSION), 1); {Write the version number}
          blockwrite(Ofile, byte(MIN_REVISION), 1);{Write the revision number}
          blockwrite(Ofile, word(DataOfs), 2);     {Offset of data segment}
          blockwrite(Ofile, word(DataSize), 2);    {Size (in bytes) of data}
        end;
  end; {case}

  blockwrite(Ofile, DName, Length(Dname)+1);{Write device name to file}

  OSize := FilePos(Ofile)+1;               {How big is header so far}
  for i := OSize to HEADER_SIZE do
    blockwrite(Ofile, byte(Null), 1);      {Pad header with zeros}

  blockwrite(Ofile, Fptr^, ISize, Result); {Write the BIN file data}
  FreeMem(Fptr, ISize);                    {Release heap memory}
  Close(Ofile);                            {Close output file}
end.

