    .cdecls "main.c" 
    .clink 
    .global START 
    .asg 1000, DELAY 
    .asg 32, PRU0_R31_VEC_VALID 
    .asg 3, PRU_EVTOUT_0  
    .asg 0x0002e000, IEP_BASE 
    .asg 0x00, IEP_TMR_GLB_CFG 
    .asg 0x0c , IEP_TMR_CNT     
 
START: 
    
    
    ; Configure IEP Timer
    LDI32 r0, IEP_BASE
    LDI32 r1, 0x11 
    SBBO &r1, r0, IEP_TMR_GLB_CFG, 4 
    
     ; Load Number of Sensors from Shared Memory (r4)
    LDI32 r13, 0x00010000 
    LBBO &r4, r13, 0, 4 


    ; Load history window size
    ADD r12, r13, 4
    LBBO &r7, r12,0,4

    ; store address
    ADD r14,r13,8
    LDI32 r6, 0

    
    ; clear interrupt pin 
    CLR r30, r30.t7

GO:    
    ; sensor mask
    LDI r5, 1 
    LSL r10, r5, r4 
    SUB r10, r10, 1 

    ; calculate addess sample row
    LDI r15,0
    QBEQ TRIG, r6, 0
    LDI r15,0
    LDI r18,0
MULT:
    ADD r15,r15,r4
    ADD r18,r18,1
    QBNE MULT, r18, r6

TRIG:
    LSL r15, r15, 2
    ADD r15,r15,r14


    ; Trigger
    LDI32 r2, DELAY 
    SET r30, r30.t5 

TRIGGERING: 
    SUB r2, r2, 1 
    QBNE TRIGGERING, r2, 0 
    CLR r30, r30.t5 

ECHO: 
    AND r9, r31, r10            
    QBEQ ECHO, r9, 0        
    ; Breaks out when a sensor goes high.

    ; Clear and start the timer
    LDI32 r1, 0 
    SBBO &r1, r0, IEP_TMR_CNT, 4 
    
    ; sensor index 
    LDI32 r8, 0 

POLLING: 
    LDI r5, 1 
    LSL r5, r5, r8  
    
    AND r9, r10, r5 
    QBEQ RETURN, r9, 0     
    

    AND r9, r31, r5 
    QBEQ CHECK_SENSOR, r9, 0 

RETURN: 
    ADD r8, r8, 1 
    QBNE SKIP_RESET, r8, r4  
    LDI32 r8, 0              

SKIP_RESET:
    QBA POLLING              

CHECK_SENSOR:
    LBBO &r3, r0, IEP_TMR_CNT, 4 
    
    LSL r5, r8, 2       ; Fidning out the sensor offset
    ADD r16, r15, r5    ; address for sensor and sample
    
    SBBO &r3, r16, 0, 4     

    LDI r5, 1 
    LSL r5, r5, r8 
    LDI32 r11, 0xFFFFFFFF        
    XOR r5, r5, r11              
    AND r10, r10, r5 

    QBEQ SENSOR_ONE, r8, 0

CHECK_END:
    QBEQ DELAYING, r10, 0        
    QBA RETURN              

SENSOR_ONE: 
    MOV r17, r3             
    QBA CHECK_END 


DELAYING:
    LDI32 r1, 1000000
    SUB r1, r1, 1
    QBNE DELAYING, r1, 0

END: 
    ADD R6,R6,1
    QBNE  GO, r6, r7
    LDI r6, 0

INTERUPT:
    SET r30, r30.t7
    QBA START
    
