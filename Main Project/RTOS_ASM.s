
;-----------------------------------------------------------------------------
; Device includes, defines, and assembler directives
;-----------------------------------------------------------------------------

   .def loadR0
   .def getR0
   .def getR1
   .def getR2
   .def getR3
   .def getR12
   .def getSP
   .def getPSP
   .def getLR
   .def getPC
   .def getPSR
   .def FPU
   .def PushR4Thru11
   .def PopR4Thru11


.thumb

;-----------------------------------------------------------------------------
; Subroutines
;-----------------------------------------------------------------------------

.text
myaddress  .field 0x20002000,32

loadR0:
               LDR R0, myaddress
               BX LR

getR0:
               BX LR


getR1:
               MOV R0, R1
               BX LR


getR2:
               MOV R0, R2
               BX LR


getR3:
               MOV R0, R3
               BX LR


getR12:
               MOV R0, R12
               BX LR


getSP:
               MOV R0, SP
               BX LR

getPSP:
               MRS R0, PSP
               BX LR


getLR:
               MOV R0, LR
               BX LR



getPC:
			   MOV R0, PC
			   BX LR


getPSR:
			   MRS R0, APSR
			   BX LR

FPU:
			  MOV R0, #2
			  VLDR S1, [R0]

PushR4Thru11:
              MRS R0, PSP
              SUB R0, R0, #4
              STR R4, [R0]
              SUB R0, R0, #4
              STR R5, [R0]
              SUB R0, R0, #4
              STR R6, [R0]
              SUB R0, R0, #4
              STR R7, [R0]
              SUB R0, R0, #4
              STR R8, [R0]
              SUB R0, R0, #4
              STR R9, [R0]
              SUB R0, R0, #4
              STR R10, [R0]
              SUB R0, R0, #4
              STR R11, [R0]
              MSR PSP, R0
              BX LR

PopR4Thru11:
              MRS R0, PSP
              ADD R0, R0, #4
              LDR R4, [R0]
              ADD R0, R0, #4
              LDR R5, [R0]
              ADD R0, R0, #4
              LDR R6, [R0]
              ADD R0, R0, #4
              LDR R7, [R0]
              ADD R0, R0, #4
              LDR R8, [R0]
              ADD R0, R0, #4
              LDR R9, [R0]
              ADD R0, R0, #4
              LDR R10, [R0]
              ADD R0, R0, #4
              LDR R11, [R0]
              MSR PSP, R0
              BX LR

.endm








