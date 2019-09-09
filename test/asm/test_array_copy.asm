.code

sub_401000      proc near               ; CODE XREF: sub_401040+77¡ýp

var_4           = word ptr -4
arg_0           = dword ptr  8
arg_4           = dword ptr  0Ch
arg_8           = dword ptr  10h

                push    ebp
                mov     ebp, esp
                push    ecx
                push    esi
                xor     eax, eax
                mov     [ebp+var_4], ax
                jmp     short loc_401019
; ---------------------------------------------------------------------------

loc_40100D:                             ; CODE XREF: sub_401000+36¡ýj
                mov     cx, [ebp+var_4]
                add     cx, 1
                mov     [ebp+var_4], cx

loc_401019:                             ; CODE XREF: sub_401000+B¡üj
                movsx   edx, [ebp+var_4]
                cmp     edx, [ebp+arg_8]
                jge     short loc_401038
                movsx   eax, [ebp+var_4]
                movsx   ecx, [ebp+var_4]
                mov     edx, [ebp+arg_4]
                mov     esi, [ebp+arg_0]
                mov     al, [esi+eax]
                mov     [edx+ecx], al
                jmp     short loc_40100D
; ---------------------------------------------------------------------------

loc_401038:                             ; CODE XREF: sub_401000+20¡üj
                pop     esi
                mov     esp, ebp
                pop     ebp
                retn
sub_401000      endp

; ---------------------------------------------------------------------------
