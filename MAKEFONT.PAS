
{Program to create a font file from the existing system 8x8 font}
{Note: this program assumes that the interrupt vector at $1f contains}
{a pointer to a valid upper font definition table as would be obtained}
{after running the MSDOS GRAFTBL program.}
{Writen by Michael Day as of 12/02/92 - public domain}

program MakeFont;

uses Dos;

type  FontChar  = array[0..7] of byte;
      FontArray = array[0..127] of FontChar;

const FontFilename = 'FNTIMG.DAT';
const FontID : FontChar = ($4D,$49,$4B,$45,$20,$44,$41,$59);

var LoFont : FontArray absolute $F000:$FA6E;
    SysFont : ^FontArray;
    UpFont : FontArray;
    f : file of FontArray;

begin
  assign(f,FontFilename);           {Create font file}
  rewrite(f);
  GetIntVec($1F,pointer(SysFont));  {Get upper font table address}
  UpFont := SysFont^;               {Copy to local array}
  UpFont[127] := FontID;            {Insert our font ID mark}
  write(f,LoFont);                  {Write sys font to the file}
  write(f,UpFont);                  {followed by the upper font}
  close(f);                         {Close the file}
end.
