        .section .rodata
        .global  _gong_wav
        .type    _gong_wav, %object
        .align   4
_gong_wav:
        .incbin __gong_wav_path
        .global _gong_wav_size
        .type   _gong_wav_size, %object
        .align  4
_gong_wav_size:
        .int    _gong_wav_size - _gong_wav
