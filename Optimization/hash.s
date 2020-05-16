section .text

global gnu_hash
;rdi = char* data
;rsi = size_t size

gnu_hash:       mov eax, 5381
                mov rcx, rsi

.Next:          mov esi, eax
                shl esi, 5
                add rax, rsi
                xor rsi, rsi
                mov sil, byte [rdi]
                add eax, esi
                inc rdi
                loop .Next
                ret
