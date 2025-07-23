.segment "CODE"          ; Tell linker this is the CODE segment

RESET:  SEI
        CLD
        CLC
        LDA #$05
        ADC #$03
        BRK

.segment "VECTORS"       ; Put vectors in the VECTORS segment

        .word RESET              ; Reset vector at $FFFC
        .word $0000              ; IRQ/BRK vector at $FFFE
