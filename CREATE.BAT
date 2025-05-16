@rem	Assemble the code
tasm bgi256.asm bgi256.raw bgi256.lst ;
@
@rem	Link the code
tlink /t bgi256.raw, %1.bin, %1.map ;
del bgi256.raw
@
@rem	Convert the .BIN file to the .BGI file.  
bdh3 %1 %1.bin %1.bgi %1.map %2
@
@rem	Convert the driver to a Pascal registerable .OBJ file.
binobj %1.bgi %1.obj %1Proc
@
@rem    Convert the driver to C registerable .OBJ file.
@REM bgiobj    %1.bgi n_%1.obj _%1_driver
@REM bgiobj /F %1.bgi f_%1.obj _%1_driver_far
@