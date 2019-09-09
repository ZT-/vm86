.code

sub_401000      proc near               ; CODE XREF: WinMain(x,x,x,x)+7F¡ýp

var_8           = word ptr -8
var_2           = byte ptr -2
var_1           = byte ptr -1
arg_0           = dword ptr  8
arg_4           = dword ptr  0Ch

                push    ebp
                mov     ebp, esp
                sub     esp, 8
                xor     eax, eax
                mov     [ebp+var_8], ax
                movsx   ecx, [ebp+var_8]
                mov     edx, [ebp+arg_0]
                mov     al, [edx+ecx]
                mov     [ebp+var_1], al
                movsx   ecx, [ebp+var_8]
                mov     edx, [ebp+arg_0]
                mov     al, [edx+ecx+1]
                mov     [ebp+var_2], al
                movzx   ecx, [ebp+var_1]
                cmp     ecx, 39h
                jle     short loc_40103C
                movzx   edx, [ebp+var_1]
                sub     edx, 37h
                mov     [ebp+var_1], dl
                jmp     short loc_401046
; ---------------------------------------------------------------------------

loc_40103C:                             ; CODE XREF: sub_401000+2E¡üj
                movzx   eax, [ebp+var_1]
                sub     eax, 30h
                mov     [ebp+var_1], al

loc_401046:                             ; CODE XREF: sub_401000+3A¡üj
                movzx   ecx, [ebp+var_2]
                cmp     ecx, 39h
                jle     short loc_40105B
                movzx   edx, [ebp+var_2]
                sub     edx, 37h
                mov     [ebp+var_2], dl
                jmp     short loc_401065
; ---------------------------------------------------------------------------

loc_40105B:                             ; CODE XREF: sub_401000+4D¡üj
                movzx   eax, [ebp+var_2]
                sub     eax, 30h
                mov     [ebp+var_2], al

loc_401065:                             ; CODE XREF: sub_401000+59¡üj
                movzx   ecx, [ebp+var_1]
                shl     ecx, 4
                movzx   edx, [ebp+var_2]
                or      ecx, edx
                movsx   eax, [ebp+var_8]
                cdq
                sub     eax, edx
                sar     eax, 1
                mov     edx, [ebp+arg_4]
                mov     [edx+eax], cl
                mov     esp, ebp
                pop     ebp
                retn
sub_401000      endp

; ---------------------------------------------------------------------------
