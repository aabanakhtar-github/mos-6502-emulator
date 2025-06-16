    .org $8000

start:
    ldx #$01 
    ldy #$01 

    ; load up the zero page 
    lda #$11 ; test value
    sta $00 ; store this at zero page addr 0 for

.org $FFFC 
    .word start