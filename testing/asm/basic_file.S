.segment "CODE"          ; Tell linker this is the CODE segment
.org $8000

RESET:  SEI
        CLD
        CLC
        LDA #$05
        ADC #$03
        BRK

.segment "VECTORS"       ; Put vectors in the VECTORS segmen
.org $FFFB
        .word RESET              ; Reset vector at $FFFC
        .word $0000              ; IRQ/BRK vector at $FFFE
