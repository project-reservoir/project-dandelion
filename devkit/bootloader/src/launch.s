        AREA    subrout, CODE, READONLY     ; Name this block of code
launch  MOV     sp, r0            ; Subroutine code
        BX      r1                ; Jump into the application
        
        EXPORT launch
        
        END                       ; Mark end of file
        