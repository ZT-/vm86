.code

sub_401000      proc near               ; CODE XREF: sub_401020+32¡ýp
                                        ; sub_401020+4A¡ýp

arg_0           = dword ptr  8

                push    ebp
                mov     ebp, esp
                cmp     [ebp+arg_0], 61h
                jl      short loc_401017
                cmp     [ebp+arg_0], 7Ah
                jg      short loc_401017
                mov     eax, [ebp+arg_0]
                sub     eax, 20h
                jmp     short loc_40101A
; ---------------------------------------------------------------------------

loc_401017:                             ; CODE XREF: sub_401000+7¡üj
                                        ; sub_401000+D¡üj
                mov     eax, [ebp+arg_0]

loc_40101A:                             ; CODE XREF: sub_401000+15¡üj
                pop     ebp
                retn
sub_401000      endp

; ---------------------------------------------------------------------------


sub_401020      proc near               ; CODE XREF: WinMain(x,x,x,x)+7F¡ýp

var_8           = word ptr -8
var_2           = byte ptr -2
var_1           = byte ptr -1
arg_0           = dword ptr  8
arg_4           = dword ptr  0Ch
arg_8           = dword ptr  10h

                push    ebp
                mov     ebp, esp
                sub     esp, 8
                xor     eax, eax
                mov     [ebp+var_8], ax
                jmp     short loc_401039
; ---------------------------------------------------------------------------

loc_40102E:                             ; CODE XREF: sub_401020+AF¡ýj
                movsx   ecx, [ebp+var_8]
                add     ecx, 2
                mov     [ebp+var_8], cx

loc_401039:                             ; CODE XREF: sub_401020+C¡üj
                movsx   edx, [ebp+var_8]
                cmp     edx, [ebp+arg_8]
                jge     loc_4010D4
                movsx   eax, [ebp+var_8]
                mov     ecx, [ebp+arg_0]
                movsx   edx, byte ptr [ecx+eax]
                push    edx
                call    sub_401000
                add     esp, 4
                mov     [ebp+var_1], al
                movsx   eax, [ebp+var_8]
                mov     ecx, [ebp+arg_0]
                movsx   edx, byte ptr [ecx+eax+1]
                push    edx
                call    sub_401000
                add     esp, 4
                mov     [ebp+var_2], al
                movzx   eax, [ebp+var_1]
                cmp     eax, 39h
                jle     short loc_40108A
                movzx   ecx, [ebp+var_1]
                sub     ecx, 37h
                mov     [ebp+var_1], cl
                jmp     short loc_401094
; ---------------------------------------------------------------------------

loc_40108A:                             ; CODE XREF: sub_401020+5C¡üj
                movzx   edx, [ebp+var_1]
                sub     edx, 30h
                mov     [ebp+var_1], dl

loc_401094:                             ; CODE XREF: sub_401020+68¡üj
                movzx   eax, [ebp+var_2]
                cmp     eax, 39h
                jle     short loc_4010A9
                movzx   ecx, [ebp+var_2]
                sub     ecx, 37h
                mov     [ebp+var_2], cl
                jmp     short loc_4010B3
; ---------------------------------------------------------------------------

loc_4010A9:                             ; CODE XREF: sub_401020+7B¡üj
                movzx   edx, [ebp+var_2]
                sub     edx, 30h
                mov     [ebp+var_2], dl

loc_4010B3:                             ; CODE XREF: sub_401020+87¡üj
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
                jmp     loc_40102E
; ---------------------------------------------------------------------------

loc_4010D4:                             ; CODE XREF: sub_401020+20¡üj
                mov     esp, ebp
                pop     ebp
                retn
sub_401020      endp
