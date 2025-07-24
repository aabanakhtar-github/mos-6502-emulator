.segment "CODE"          ; Tell linker this is the CODE segment
.org $8000

RESET:  SEI
        CLD
        CLC
        LDA #$05
        ADC #$03
        BRK