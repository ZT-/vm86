.data
; char byte_403148[64]
byte_403148     db 41h                  ; DATA XREF: sub_401020+A8¡ür
                                        ; sub_401020+10C¡ür ...
                db  42h ; B
                db  43h ; C
                db  44h ; D
                db  45h ; E
                db  46h ; F
                db  47h ; G
                db  48h ; H
                db  49h ; I
                db  4Ah ; J
                db  4Bh ; K
                db  4Ch ; L
                db  4Dh ; M
                db  4Eh ; N
                db  4Fh ; O
                db  50h ; P
                db  51h ; Q
                db  52h ; R
                db  53h ; S
                db  54h ; T
                db  55h ; U
                db  56h ; V
                db  57h ; W
                db  58h ; X
                db  59h ; Y
                db  5Ah ; Z
                db  61h ; a
                db  62h ; b
                db  63h ; c
                db  64h ; d
                db  65h ; e
                db  66h ; f
                db  67h ; g
                db  68h ; h
                db  69h ; i
                db  6Ah ; j
                db  6Bh ; k
                db  6Ch ; l
                db  6Dh ; m
                db  6Eh ; n
                db  6Fh ; o
                db  70h ; p
                db  71h ; q
                db  72h ; r
                db  73h ; s
                db  74h ; t
                db  75h ; u
                db  76h ; v
                db  77h ; w
                db  78h ; x
                db  79h ; y
                db  7Ah ; z
                db  30h ; 0
                db  31h ; 1
                db  32h ; 2
                db  33h ; 3
                db  34h ; 4
                db  35h ; 5
                db  36h ; 6
                db  37h ; 7
                db  38h ; 8
                db  39h ; 9
                db  2Bh ; +
                db  2Fh ; /

.code


sub_401000      proc near               ; CODE XREF: sub_401020+E7¡ýp
                                        ; sub_401020+FE¡ýp ...

arg_0           = byte ptr  8
arg_4           = byte ptr  0Ch
arg_8           = byte ptr  10h

                push    ebp
                mov     ebp, esp
; 2:   return (unsigned __int8)(a1 << a2) >> a3;
                mov     al, [ebp+arg_0]
                mov     cl, [ebp+arg_4]
                shl     al, cl
                mov     [ebp+arg_0], al
                mov     dl, [ebp+arg_0]
                mov     cl, [ebp+arg_8]
                shr     dl, cl
                mov     [ebp+arg_0], dl
                mov     al, [ebp+arg_0]
                pop     ebp
                retn
sub_401000      endp



sub_401020      proc near               ; CODE XREF: sub_401470+73¡ýp

var_20          = dword ptr -20h
var_1C          = dword ptr -1Ch
var_18          = dword ptr -18h
var_14          = dword ptr -14h
var_10          = dword ptr -10h
var_C           = dword ptr -0Ch
var_8           = dword ptr -8
var_1           = byte ptr -1
arg_0           = dword ptr  8
arg_4           = dword ptr  0Ch
arg_8           = dword ptr  10h
arg_C           = dword ptr  14h

                push    ebp
                mov     ebp, esp
                sub     esp, 20h
                push    esi
; 9:   if ( !a1 || !a2 )
                mov     [ebp+var_18], 0
                cmp     [ebp+arg_0], 0
                jz      short loc_40103A
                cmp     [ebp+arg_4], 0
                jnz     short loc_401049
; 10:     return -1;

loc_40103A:                             ; CODE XREF: sub_401020+12¡üj
                mov     [ebp+var_18], 0FFFFFFFFh
                mov     eax, [ebp+var_18]
                jmp     loc_401250
; ---------------------------------------------------------------------------
; 11:   v9 = 0;

loc_401049:                             ; CODE XREF: sub_401020+18¡üj
                mov     [ebp+var_14], 0
                mov     [ebp+var_C], 0
; 12:   if ( a2 % 3 )
                mov     eax, [ebp+arg_4]
                cdq
                mov     ecx, 3
                idiv    ecx
                test    edx, edx
                jz      short loc_40107B
; 13:     v9 = 3 - a2 % 3;
                mov     eax, [ebp+arg_4]
                cdq
                mov     ecx, 3
                idiv    ecx
                mov     eax, 3
                sub     eax, edx
                mov     [ebp+var_C], eax
; 15:   for ( i = 0; i < v9 + a2; i += 3 )

loc_40107B:                             ; CODE XREF: sub_401020+44¡üj
                mov     ecx, [ebp+arg_4]
                add     ecx, [ebp+var_C]
                mov     [ebp+var_14], ecx
; 46:     *a4 = 8 * (v9 + a2) / 6;
                mov     eax, [ebp+var_14]
                shl     eax, 3
                cdq
                mov     ecx, 6
                idiv    ecx
                mov     [ebp+var_20], eax
; 14:   v10 = a3;
                mov     edx, [ebp+arg_8]
                mov     [ebp+var_8], edx
                mov     [ebp+var_10], 0
                jmp     short loc_4010AD
; ---------------------------------------------------------------------------

loc_4010A4:                             ; CODE XREF: sub_401020+21A¡ýj
                mov     eax, [ebp+var_10]
                add     eax, 3
                mov     [ebp+var_10], eax

loc_4010AD:                             ; CODE XREF: sub_401020+82¡üj
                mov     ecx, [ebp+var_10]
                cmp     ecx, [ebp+var_14]
                jge     loc_40123F
; 17:     *v10 = byte_403148[*a1 >> 2];
                mov     edx, [ebp+arg_0]
                movsx   eax, byte ptr [edx]
                sar     eax, 2
                mov     [ebp+var_1C], eax
                mov     ecx, [ebp+var_1C]
                mov     dl, ds:byte_403148[ecx]
                mov     [ebp+var_1], dl
                mov     eax, [ebp+var_8]
                mov     cl, [ebp+var_1]
                mov     [eax], cl
; 18:     if ( i == a2 + v9 - 3 && v9 )
                mov     edx, [ebp+var_C]
                mov     eax, [ebp+arg_4]
                lea     ecx, [eax+edx-3]
                cmp     [ebp+var_10], ecx
                jnz     loc_40119D
                cmp     [ebp+var_C], 0
                jz      loc_40119D
; 20:       if ( v9 == 1 )
                cmp     [ebp+var_C], 1
                jnz     short loc_401162
; 22:         v5 = (char)sub_401000(*a1, 6, 2);
                push    2
                push    6
                mov     edx, [ebp+arg_0]
                movzx   eax, byte ptr [edx]
                push    eax
                call    sub_401000
                add     esp, 0Ch
                movsx   esi, al
; 23:         v10[1] = byte_403148[v5 + (char)sub_401000(a1[1], 0, 4)];
                push    4
                push    0
                mov     ecx, [ebp+arg_0]
                movzx   edx, byte ptr [ecx+1]
                push    edx
                call    sub_401000
                add     esp, 0Ch
                movsx   eax, al
                mov     ecx, [ebp+var_8]
                mov     dl, ds:byte_403148[esi+eax]
                mov     [ecx+1], dl
; 24:         v10[2] = byte_403148[(char)sub_401000(a1[1], 4, 2)];
                push    2
                push    4
                mov     eax, [ebp+arg_0]
                movzx   ecx, byte ptr [eax+1]
                push    ecx
                call    sub_401000
                add     esp, 0Ch
                movsx   edx, al
                mov     eax, [ebp+var_8]
                mov     cl, ds:byte_403148[edx]
                mov     [eax+2], cl
; 25:         v10[3] = 61;
                mov     edx, [ebp+var_8]
                mov     byte ptr [edx+3], 3Dh
                jmp     short loc_401198
; ---------------------------------------------------------------------------
; 27:       else if ( v9 == 2 )

loc_401162:                             ; CODE XREF: sub_401020+DA¡üj
                cmp     [ebp+var_C], 2
                jnz     short loc_401198
; 29:         v10[1] = byte_403148[(char)sub_401000(*a1, 6, 2)];
                push    2
                push    6
                mov     eax, [ebp+arg_0]
                movzx   ecx, byte ptr [eax]
                push    ecx
                call    sub_401000
                add     esp, 0Ch
                movsx   edx, al
                mov     eax, [ebp+var_8]
                mov     cl, ds:byte_403148[edx]
                mov     [eax+1], cl
; 30:         v10[2] = 61;
                mov     edx, [ebp+var_8]
                mov     byte ptr [edx+2], 3Dh
; 31:         v10[3] = 61;
                mov     eax, [ebp+var_8]
                mov     byte ptr [eax+3], 3Dh

loc_401198:                             ; CODE XREF: sub_401020+140¡üj
                                        ; sub_401020+146¡üj
                jmp     loc_401228
; ---------------------------------------------------------------------------
; 36:       v6 = (char)sub_401000(*a1, 6, 2);

loc_40119D:                             ; CODE XREF: sub_401020+C6¡üj
                                        ; sub_401020+D0¡üj
                push    2
                push    6
                mov     ecx, [ebp+arg_0]
                movzx   edx, byte ptr [ecx]
                push    edx
                call    sub_401000
                add     esp, 0Ch
                movsx   esi, al
; 37:       v10[1] = byte_403148[v6 + (char)sub_401000(a1[1], 0, 4)];
                push    4
                push    0
                mov     eax, [ebp+arg_0]
                movzx   ecx, byte ptr [eax+1]
                push    ecx
                call    sub_401000
                add     esp, 0Ch
                movsx   edx, al
                mov     eax, [ebp+var_8]
                mov     cl, ds:byte_403148[esi+edx]
                mov     [eax+1], cl
; 38:       v7 = (char)sub_401000(a1[1], 4, 2);
                push    2
                push    4
                mov     edx, [ebp+arg_0]
                movzx   eax, byte ptr [edx+1]
                push    eax
                call    sub_401000
                add     esp, 0Ch
                movsx   esi, al
; 39:       v10[2] = byte_403148[v7 + (char)sub_401000(a1[2], 0, 6)];
                push    6
                push    0
                mov     ecx, [ebp+arg_0]
                movzx   edx, byte ptr [ecx+2]
                push    edx
                call    sub_401000
                add     esp, 0Ch
                movsx   eax, al
                mov     ecx, [ebp+var_8]
                mov     dl, ds:byte_403148[esi+eax]
                mov     [ecx+2], dl
; 40:       v10[3] = byte_403148[a1[2] & 0x3F];
                mov     eax, [ebp+arg_0]
                movsx   ecx, byte ptr [eax+2]
                and     ecx, 3Fh
                mov     edx, [ebp+var_8]
                mov     al, ds:byte_403148[ecx]
                mov     [edx+3], al
; 42:     v10 += 4;

loc_401228:                             ; CODE XREF: sub_401020:loc_401198¡üj
                mov     ecx, [ebp+var_8]
                add     ecx, 4
                mov     [ebp+var_8], ecx
; 43:     a1 += 3;
                mov     edx, [ebp+arg_0]
                add     edx, 3
                mov     [ebp+arg_0], edx
                jmp     loc_4010A4
; ---------------------------------------------------------------------------
; 45:   if ( a4 )

loc_40123F:                             ; CODE XREF: sub_401020+93¡üj
                cmp     [ebp+arg_C], 0
                jz      short loc_40124D
                mov     eax, [ebp+arg_C]
                mov     ecx, [ebp+var_20]
                mov     [eax], ecx

loc_40124D:                             ; CODE XREF: sub_401020+223¡üj
                mov     eax, [ebp+var_18]
; 47:   return 0;

loc_401250:                             ; CODE XREF: sub_401020+24¡üj
                pop     esi
                mov     esp, ebp
                pop     ebp
                retn
sub_401020      endp

; ---------------------------------------------------------------------------
