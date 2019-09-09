.code

sub_401000      proc near               ; CODE XREF: sub_401030+2D¡ýp

var_1           = byte ptr -1
arg_0           = byte ptr  8
                push    ebp
                mov     ebp, esp
                push    ecx
                mov     al, [ebp+arg_0]
                mov     [ebp+var_1], al
                movsx   ecx, [ebp+arg_0]
                sub     ecx, 61h
                cmp     ecx, 19h
                jg      short loc_401020
                movsx   edx, [ebp+arg_0]
                sub     edx, 20h
                mov     [ebp+var_1], dl

loc_401020:                             ; CODE XREF: sub_401000+14¡üj
                mov     al, [ebp+var_1]
                mov     esp, ebp
                pop     ebp
                retn
sub_401000      endp

; ---------------------------------------------------------------------------

sub_401030      proc near               ; CODE XREF: sub_401080+7

var_4           = word ptr -4
arg_0           = dword ptr  8
arg_4           = dword ptr  0Ch
arg_8           = dword ptr  10h

                push    ebp
                mov     ebp, esp
                push    ecx
                xor     eax, eax
                mov     [ebp+var_4], ax
                jmp     short loc_401048
; ---------------------------------------------------------------------------

loc_40103C:                             ; CODE XREF: sub_401030+3F
                mov     cx, [ebp+var_4]
                add     cx, 1
                mov     [ebp+var_4], cx

loc_401048:                             ; CODE XREF: sub_401030+
                movsx   edx, [ebp+var_4]
                cmp     edx, [ebp+arg_8]
                jge     short loc_401071
                movsx   eax, [ebp+var_4]
                mov     ecx, [ebp+arg_0]
                movzx   edx, byte ptr [ecx+eax]
                push    edx
                call    sub_401000
                add     esp, 4
                movsx   ecx, [ebp+var_4]
                mov     edx, [ebp+arg_4]
                mov     [edx+ecx], al
                jmp     short loc_40103C
; ---------------------------------------------------------------------------

loc_401071:                             ; CODE XREF: sub_401030+1
                mov     esp, ebp
                pop     ebp
                retn
sub_401030      endp

; ---------------------------------------------------------------------------
