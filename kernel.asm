00000000  BC601C0200        mov esp,0x21c60
00000005  B045              mov al,0x45
00000007  E6E9              out 0xe9,al
00000009  E8A7010000        call 0x1b5
0000000E  B044              mov al,0x44
00000010  E6E9              out 0xe9,al
00000012  FA                cli
00000013  F4                hlt
00000014  EBFD              jmp short 0x13
00000016  6690              xchg ax,ax
00000018  6690              xchg ax,ax
0000001A  6690              xchg ax,ax
0000001C  6690              xchg ax,ax
0000001E  6690              xchg ax,ax
00000020  60                pusha
00000021  1E                push ds
00000022  06                push es
00000023  0FA0              push fs
00000025  0FA8              push gs
00000027  66B81000          mov ax,0x10
0000002B  8ED8              mov ds,eax
0000002D  8EC0              mov es,eax
0000002F  8EE0              mov fs,eax
00000031  8EE8              mov gs,eax
00000033  54                push esp
00000034  E825090000        call 0x95e
00000039  58                pop eax
0000003A  0FA9              pop gs
0000003C  0FA1              pop fs
0000003E  07                pop es
0000003F  1F                pop ds
00000040  61                popa
00000041  83C408            add esp,byte +0x8
00000044  CF                iret
00000045  60                pusha
00000046  1E                push ds
00000047  06                push es
00000048  0FA0              push fs
0000004A  0FA8              push gs
0000004C  66B81000          mov ax,0x10
00000050  8ED8              mov ds,eax
00000052  8EC0              mov es,eax
00000054  8EE0              mov fs,eax
00000056  8EE8              mov gs,eax
00000058  54                push esp
00000059  E806090000        call 0x964
0000005E  58                pop eax
0000005F  0FA9              pop gs
00000061  0FA1              pop fs
00000063  07                pop es
00000064  1F                pop ds
00000065  61                popa
00000066  83C408            add esp,byte +0x8
00000069  CF                iret
0000006A  6A00              push byte +0x0
0000006C  6A00              push byte +0x0
0000006E  EBB0              jmp short 0x20
00000070  6A00              push byte +0x0
00000072  6A01              push byte +0x1
00000074  EBAA              jmp short 0x20
00000076  6A00              push byte +0x0
00000078  6A02              push byte +0x2
0000007A  EBA4              jmp short 0x20
0000007C  6A00              push byte +0x0
0000007E  6A03              push byte +0x3
00000080  EB9E              jmp short 0x20
00000082  6A00              push byte +0x0
00000084  6A04              push byte +0x4
00000086  EB98              jmp short 0x20
00000088  6A00              push byte +0x0
0000008A  6A05              push byte +0x5
0000008C  EB92              jmp short 0x20
0000008E  6A00              push byte +0x0
00000090  6A06              push byte +0x6
00000092  EB8C              jmp short 0x20
00000094  6A00              push byte +0x0
00000096  6A07              push byte +0x7
00000098  EB86              jmp short 0x20
0000009A  6A08              push byte +0x8
0000009C  EB82              jmp short 0x20
0000009E  6A00              push byte +0x0
000000A0  6A09              push byte +0x9
000000A2  E979FFFFFF        jmp 0x20
000000A7  6A0A              push byte +0xa
000000A9  E972FFFFFF        jmp 0x20
000000AE  6A0B              push byte +0xb
000000B0  E96BFFFFFF        jmp 0x20
000000B5  6A0C              push byte +0xc
000000B7  E964FFFFFF        jmp 0x20
000000BC  6A0D              push byte +0xd
000000BE  E95DFFFFFF        jmp 0x20
000000C3  6A0E              push byte +0xe
000000C5  E956FFFFFF        jmp 0x20
000000CA  6A00              push byte +0x0
000000CC  6A0F              push byte +0xf
000000CE  E94DFFFFFF        jmp 0x20
000000D3  6A00              push byte +0x0
000000D5  6A20              push byte +0x20
000000D7  E969FFFFFF        jmp 0x45
000000DC  6A00              push byte +0x0
000000DE  6A21              push byte +0x21
000000E0  E960FFFFFF        jmp 0x45
000000E5  6A00              push byte +0x0
000000E7  6A22              push byte +0x22
000000E9  E957FFFFFF        jmp 0x45
000000EE  6A00              push byte +0x0
000000F0  6A23              push byte +0x23
000000F2  E94EFFFFFF        jmp 0x45
000000F7  6A00              push byte +0x0
000000F9  6A24              push byte +0x24
000000FB  E945FFFFFF        jmp 0x45
00000100  6A00              push byte +0x0
00000102  6A25              push byte +0x25
00000104  E93CFFFFFF        jmp 0x45
00000109  6A00              push byte +0x0
0000010B  6A26              push byte +0x26
0000010D  E933FFFFFF        jmp 0x45
00000112  6A00              push byte +0x0
00000114  6A27              push byte +0x27
00000116  E92AFFFFFF        jmp 0x45
0000011B  6A00              push byte +0x0
0000011D  6A28              push byte +0x28
0000011F  E921FFFFFF        jmp 0x45
00000124  6A00              push byte +0x0
00000126  6A29              push byte +0x29
00000128  E918FFFFFF        jmp 0x45
0000012D  6A00              push byte +0x0
0000012F  6A2A              push byte +0x2a
00000131  E90FFFFFFF        jmp 0x45
00000136  6A00              push byte +0x0
00000138  6A2B              push byte +0x2b
0000013A  E906FFFFFF        jmp 0x45
0000013F  6A00              push byte +0x0
00000141  6A2C              push byte +0x2c
00000143  E9FDFEFFFF        jmp 0x45
00000148  6A00              push byte +0x0
0000014A  6A2D              push byte +0x2d
0000014C  E9F4FEFFFF        jmp 0x45
00000151  6A00              push byte +0x0
00000153  6A2E              push byte +0x2e
00000155  E9EBFEFFFF        jmp 0x45
0000015A  6A00              push byte +0x0
0000015C  6A2F              push byte +0x2f
0000015E  E9E2FEFFFF        jmp 0x45
00000163  6690              xchg ax,ax
00000165  6690              xchg ax,ax
00000167  6690              xchg ax,ax
00000169  6690              xchg ax,ax
0000016B  6690              xchg ax,ax
0000016D  6690              xchg ax,ax
0000016F  90                nop
00000170  8B442404          mov eax,[esp+0x4]
00000174  0F0110            lgdt [eax]
00000177  66B81000          mov ax,0x10
0000017B  8ED8              mov ds,eax
0000017D  8EC0              mov es,eax
0000017F  8EE0              mov fs,eax
00000181  8EE8              mov gs,eax
00000183  8ED0              mov ss,eax
00000185  EA8C0101000800    jmp 0x8:0x1018c
0000018C  C3                ret
0000018D  6690              xchg ax,ax
0000018F  90                nop
00000190  8B442404          mov eax,[esp+0x4]
00000194  0F0118            lidt [eax]
00000197  C3                ret
00000198  55                push ebp
00000199  89E5              mov ebp,esp
0000019B  83EC08            sub esp,byte +0x8
0000019E  68FFFFFF00        push dword 0xffffff
000001A3  FF7508            push dword [ebp+0x8]
000001A6  6AFF              push byte -0x1
000001A8  6A00              push byte +0x0
000001AA  E84D480000        call 0x49fc
000001AF  83C410            add esp,byte +0x10
000001B2  90                nop
000001B3  C9                leave
000001B4  C3                ret
000001B5  55                push ebp
000001B6  89E5              mov ebp,esp
000001B8  83EC38            sub esp,byte +0x38
000001BB  E8624A0000        call 0x4c22
000001C0  83EC0C            sub esp,byte +0xc
000001C3  6860B50100        push dword 0x1b560
000001C8  E8EE4A0000        call 0x4cbb
000001CD  83C410            add esp,byte +0x10
000001D0  83EC08            sub esp,byte +0x8
000001D3  680000E000        push dword 0xe00000
000001D8  6800002000        push dword 0x200000
000001DD  E89D650000        call 0x677f
000001E2  83C410            add esp,byte +0x10
000001E5  83EC0C            sub esp,byte +0xc
000001E8  6874B50100        push dword 0x1b574
000001ED  E8C94A0000        call 0x4cbb
000001F2  83C410            add esp,byte +0x10
000001F5  E806020000        call 0x400
000001FA  83EC0C            sub esp,byte +0xc
000001FD  6882B50100        push dword 0x1b582
00000202  E8B44A0000        call 0x4cbb
00000207  83C410            add esp,byte +0x10
0000020A  83EC0C            sub esp,byte +0xc
0000020D  688AB50100        push dword 0x1b58a
00000212  E8A44A0000        call 0x4cbb
00000217  83C410            add esp,byte +0x10
0000021A  83EC0C            sub esp,byte +0xc
0000021D  68A3B50100        push dword 0x1b5a3
00000222  E8944A0000        call 0x4cbb
00000227  83C410            add esp,byte +0x10
0000022A  E809320000        call 0x3438
0000022F  E804320000        call 0x3438
00000234  B800900000        mov eax,0x9000
00000239  8B00              mov eax,[eax]
0000023B  8945EC            mov [ebp-0x14],eax
0000023E  83EC0C            sub esp,byte +0xc
00000241  68B9B50100        push dword 0x1b5b9
00000246  E8704A0000        call 0x4cbb
0000024B  83C410            add esp,byte +0x10
0000024E  C745D730313233    mov dword [ebp-0x29],0x33323130
00000255  C745DB34353637    mov dword [ebp-0x25],0x37363534
0000025C  C745DF38394142    mov dword [ebp-0x21],0x42413938
00000263  C745E343444546    mov dword [ebp-0x1d],0x46454443
0000026A  C645E700          mov byte [ebp-0x19],0x0
0000026E  C745F41C000000    mov dword [ebp-0xc],0x1c
00000275  EB27              jmp short 0x29e
00000277  8B45F4            mov eax,[ebp-0xc]
0000027A  8B55EC            mov edx,[ebp-0x14]
0000027D  89C1              mov ecx,eax
0000027F  D3EA              shr edx,cl
00000281  89D0              mov eax,edx
00000283  83E00F            and eax,byte +0xf
00000286  0FB64405D7        movzx eax,byte [ebp+eax-0x29]
0000028B  0FBEC0            movsx eax,al
0000028E  83EC0C            sub esp,byte +0xc
00000291  50                push eax
00000292  E8004A0000        call 0x4c97
00000297  83C410            add esp,byte +0x10
0000029A  836DF404          sub dword [ebp-0xc],byte +0x4
0000029E  837DF400          cmp dword [ebp-0xc],byte +0x0
000002A2  79D3              jns 0x277
000002A4  83EC0C            sub esp,byte +0xc
000002A7  68C6B50100        push dword 0x1b5c6
000002AC  E80A4A0000        call 0x4cbb
000002B1  83C410            add esp,byte +0x10
000002B4  83EC0C            sub esp,byte +0xc
000002B7  6A00              push byte +0x0
000002B9  E870320000        call 0x352e
000002BE  83C410            add esp,byte +0x10
000002C1  E812330000        call 0x35d8
000002C6  B800900000        mov eax,0x9000
000002CB  8B00              mov eax,[eax]
000002CD  8945E8            mov [ebp-0x18],eax
000002D0  837DE800          cmp dword [ebp-0x18],byte +0x0
000002D4  743D              jz 0x313
000002D6  C745F000000000    mov dword [ebp-0x10],0x0
000002DD  EB2B              jmp short 0x30a
000002DF  8B55F0            mov edx,[ebp-0x10]
000002E2  8B45E8            mov eax,[ebp-0x18]
000002E5  01D0              add eax,edx
000002E7  C600FF            mov byte [eax],0xff
000002EA  8B45F0            mov eax,[ebp-0x10]
000002ED  8D5001            lea edx,[eax+0x1]
000002F0  8B45E8            mov eax,[ebp-0x18]
000002F3  01D0              add eax,edx
000002F5  C600FF            mov byte [eax],0xff
000002F8  8B45F0            mov eax,[ebp-0x10]
000002FB  8D5002            lea edx,[eax+0x2]
000002FE  8B45E8            mov eax,[ebp-0x18]
00000301  01D0              add eax,edx
00000303  C600FF            mov byte [eax],0xff
00000306  8345F003          add dword [ebp-0x10],byte +0x3
0000030A  817DF0B70B0000    cmp dword [ebp-0x10],0xbb7
00000311  7ECC              jng 0x2df
00000313  83EC0C            sub esp,byte +0xc
00000316  68C8B50100        push dword 0x1b5c8
0000031B  E878FEFFFF        call 0x198
00000320  83C410            add esp,byte +0x10
00000323  FB                sti
00000324  83EC0C            sub esp,byte +0xc
00000327  68E7B50100        push dword 0x1b5e7
0000032C  E867FEFFFF        call 0x198
00000331  83C410            add esp,byte +0x10
00000334  E814A80000        call 0xab4d
00000339  E899B00000        call 0xb3d7
0000033E  F4                hlt
0000033F  EBF8              jmp short 0x339
00000341  55                push ebp
00000342  89E5              mov ebp,esp
00000344  83EC08            sub esp,byte +0x8
00000347  8B5514            mov edx,[ebp+0x14]
0000034A  8B4518            mov eax,[ebp+0x18]
0000034D  8855FC            mov [ebp-0x4],dl
00000350  8845F8            mov [ebp-0x8],al
00000353  8B450C            mov eax,[ebp+0xc]
00000356  89C2              mov edx,eax
00000358  8B4508            mov eax,[ebp+0x8]
0000035B  668914C51E290200  mov [eax*8+0x2291e],dx
00000363  8B450C            mov eax,[ebp+0xc]
00000366  C1E810            shr eax,byte 0x10
00000369  89C2              mov edx,eax
0000036B  8B4508            mov eax,[ebp+0x8]
0000036E  8814C520290200    mov [eax*8+0x22920],dl
00000375  8B450C            mov eax,[ebp+0xc]
00000378  C1E818            shr eax,byte 0x18
0000037B  89C2              mov edx,eax
0000037D  8B4508            mov eax,[ebp+0x8]
00000380  8814C523290200    mov [eax*8+0x22923],dl
00000387  8B4510            mov eax,[ebp+0x10]
0000038A  89C2              mov edx,eax
0000038C  8B4508            mov eax,[ebp+0x8]
0000038F  668914C51C290200  mov [eax*8+0x2291c],dx
00000397  8B4510            mov eax,[ebp+0x10]
0000039A  C1E810            shr eax,byte 0x10
0000039D  83E00F            and eax,byte +0xf
000003A0  89C2              mov edx,eax
000003A2  8B4508            mov eax,[ebp+0x8]
000003A5  8814C522290200    mov [eax*8+0x22922],dl
000003AC  8B4508            mov eax,[ebp+0x8]
000003AF  0FB604C522290200  movzx eax,byte [eax*8+0x22922]
000003B7  89C2              mov edx,eax
000003B9  0FB645F8          movzx eax,byte [ebp-0x8]
000003BD  83E0F0            and eax,byte -0x10
000003C0  09D0              or eax,edx
000003C2  89C2              mov edx,eax
000003C4  8B4508            mov eax,[ebp+0x8]
000003C7  8814C522290200    mov [eax*8+0x22922],dl
000003CE  8B4508            mov eax,[ebp+0x8]
000003D1  0FB655FC          movzx edx,byte [ebp-0x4]
000003D5  8814C521290200    mov [eax*8+0x22921],dl
000003DC  90                nop
000003DD  C9                leave
000003DE  C3                ret
000003DF  55                push ebp
000003E0  89E5              mov ebp,esp
000003E2  8B4508            mov eax,[ebp+0x8]
000003E5  0F0110            lgdt [eax]
000003E8  66B81000          mov ax,0x10
000003EC  8ED8              mov ds,eax
000003EE  8EC0              mov es,eax
000003F0  8EE0              mov fs,eax
000003F2  8EE8              mov gs,eax
000003F4  8ED0              mov ss,eax
000003F6  EAFD0301000800    jmp 0x8:0x103fd
000003FD  90                nop
000003FE  5D                pop ebp
000003FF  C3                ret
00000400  55                push ebp
00000401  89E5              mov ebp,esp
00000403  66C7051429020017  mov word [dword 0x22914],0x17
         -00
0000040C  B81C290200        mov eax,0x2291c
00000411  A316290200        mov [0x22916],eax
00000416  6A00              push byte +0x0
00000418  6A00              push byte +0x0
0000041A  6A00              push byte +0x0
0000041C  6A00              push byte +0x0
0000041E  6A00              push byte +0x0
00000420  E81CFFFFFF        call 0x341
00000425  83C414            add esp,byte +0x14
00000428  68CF000000        push dword 0xcf
0000042D  689A000000        push dword 0x9a
00000432  6AFF              push byte -0x1
00000434  6A00              push byte +0x0
00000436  6A01              push byte +0x1
00000438  E804FFFFFF        call 0x341
0000043D  83C414            add esp,byte +0x14
00000440  68CF000000        push dword 0xcf
00000445  6892000000        push dword 0x92
0000044A  6AFF              push byte -0x1
0000044C  6A00              push byte +0x0
0000044E  6A02              push byte +0x2
00000450  E8ECFEFFFF        call 0x341
00000455  83C414            add esp,byte +0x14
00000458  B814290200        mov eax,0x22914
0000045D  50                push eax
0000045E  E87CFFFFFF        call 0x3df
00000463  83C404            add esp,byte +0x4
00000466  90                nop
00000467  C9                leave
00000468  C3                ret
00000469  55                push ebp
0000046A  89E5              mov ebp,esp
0000046C  83EC0C            sub esp,byte +0xc
0000046F  8B4D08            mov ecx,[ebp+0x8]
00000472  8B5510            mov edx,[ebp+0x10]
00000475  8B4514            mov eax,[ebp+0x14]
00000478  884DFC            mov [ebp-0x4],cl
0000047B  668955F8          mov [ebp-0x8],dx
0000047F  8845F4            mov [ebp-0xc],al
00000482  0FB645FC          movzx eax,byte [ebp-0x4]
00000486  8B550C            mov edx,[ebp+0xc]
00000489  668914C560290200  mov [eax*8+0x22960],dx
00000491  8B450C            mov eax,[ebp+0xc]
00000494  C1E810            shr eax,byte 0x10
00000497  89C2              mov edx,eax
00000499  0FB645FC          movzx eax,byte [ebp-0x4]
0000049D  668914C566290200  mov [eax*8+0x22966],dx
000004A5  0FB645FC          movzx eax,byte [ebp-0x4]
000004A9  0FB755F8          movzx edx,word [ebp-0x8]
000004AD  668914C562290200  mov [eax*8+0x22962],dx
000004B5  0FB645FC          movzx eax,byte [ebp-0x4]
000004B9  C604C56429020000  mov byte [eax*8+0x22964],0x0
000004C1  0FB645FC          movzx eax,byte [ebp-0x4]
000004C5  0FB655F4          movzx edx,byte [ebp-0xc]
000004C9  8814C565290200    mov [eax*8+0x22965],dl
000004D0  90                nop
000004D1  C9                leave
000004D2  C3                ret
000004D3  55                push ebp
000004D4  89E5              mov ebp,esp
000004D6  83EC18            sub esp,byte +0x18
000004D9  66C70540290200FF  mov word [dword 0x22940],0x7ff
         -07
000004E2  B860290200        mov eax,0x22960
000004E7  A342290200        mov [0x22942],eax
000004EC  C745F400000000    mov dword [ebp-0xc],0x0
000004F3  EB19              jmp short 0x50e
000004F5  8B45F4            mov eax,[ebp-0xc]
000004F8  0FB6C0            movzx eax,al
000004FB  6A00              push byte +0x0
000004FD  6A00              push byte +0x0
000004FF  6A00              push byte +0x0
00000501  50                push eax
00000502  E862FFFFFF        call 0x469
00000507  83C410            add esp,byte +0x10
0000050A  8345F401          add dword [ebp-0xc],byte +0x1
0000050E  817DF4FF000000    cmp dword [ebp-0xc],0xff
00000515  7EDE              jng 0x4f5
00000517  B840290200        mov eax,0x22940
0000051C  83EC0C            sub esp,byte +0xc
0000051F  50                push eax
00000520  E86BFCFFFF        call 0x190
00000525  83C410            add esp,byte +0x10
00000528  90                nop
00000529  C9                leave
0000052A  C3                ret
0000052B  55                push ebp
0000052C  89E5              mov ebp,esp
0000052E  83EC08            sub esp,byte +0x8
00000531  83EC08            sub esp,byte +0x8
00000534  6A00              push byte +0x0
00000536  6880000000        push dword 0x80
0000053B  E8A6460000        call 0x4be6
00000540  83C410            add esp,byte +0x10
00000543  90                nop
00000544  C9                leave
00000545  C3                ret
00000546  55                push ebp
00000547  89E5              mov ebp,esp
00000549  83EC18            sub esp,byte +0x18
0000054C  83EC0C            sub esp,byte +0xc
0000054F  6A21              push byte +0x21
00000551  E8AF460000        call 0x4c05
00000556  83C410            add esp,byte +0x10
00000559  8845F7            mov [ebp-0x9],al
0000055C  83EC0C            sub esp,byte +0xc
0000055F  68A1000000        push dword 0xa1
00000564  E89C460000        call 0x4c05
00000569  83C410            add esp,byte +0x10
0000056C  8845F6            mov [ebp-0xa],al
0000056F  83EC08            sub esp,byte +0x8
00000572  6A11              push byte +0x11
00000574  6A20              push byte +0x20
00000576  E86B460000        call 0x4be6
0000057B  83C410            add esp,byte +0x10
0000057E  E8A8FFFFFF        call 0x52b
00000583  83EC08            sub esp,byte +0x8
00000586  6A11              push byte +0x11
00000588  68A0000000        push dword 0xa0
0000058D  E854460000        call 0x4be6
00000592  83C410            add esp,byte +0x10
00000595  E891FFFFFF        call 0x52b
0000059A  83EC08            sub esp,byte +0x8
0000059D  6A20              push byte +0x20
0000059F  6A21              push byte +0x21
000005A1  E840460000        call 0x4be6
000005A6  83C410            add esp,byte +0x10
000005A9  E87DFFFFFF        call 0x52b
000005AE  83EC08            sub esp,byte +0x8
000005B1  6A28              push byte +0x28
000005B3  68A1000000        push dword 0xa1
000005B8  E829460000        call 0x4be6
000005BD  83C410            add esp,byte +0x10
000005C0  E866FFFFFF        call 0x52b
000005C5  83EC08            sub esp,byte +0x8
000005C8  6A04              push byte +0x4
000005CA  6A21              push byte +0x21
000005CC  E815460000        call 0x4be6
000005D1  83C410            add esp,byte +0x10
000005D4  E852FFFFFF        call 0x52b
000005D9  83EC08            sub esp,byte +0x8
000005DC  6A02              push byte +0x2
000005DE  68A1000000        push dword 0xa1
000005E3  E8FE450000        call 0x4be6
000005E8  83C410            add esp,byte +0x10
000005EB  E83BFFFFFF        call 0x52b
000005F0  83EC08            sub esp,byte +0x8
000005F3  6A01              push byte +0x1
000005F5  6A21              push byte +0x21
000005F7  E8EA450000        call 0x4be6
000005FC  83C410            add esp,byte +0x10
000005FF  E827FFFFFF        call 0x52b
00000604  83EC08            sub esp,byte +0x8
00000607  6A01              push byte +0x1
00000609  68A1000000        push dword 0xa1
0000060E  E8D3450000        call 0x4be6
00000613  83C410            add esp,byte +0x10
00000616  E810FFFFFF        call 0x52b
0000061B  83EC08            sub esp,byte +0x8
0000061E  68FF000000        push dword 0xff
00000623  6A21              push byte +0x21
00000625  E8BC450000        call 0x4be6
0000062A  83C410            add esp,byte +0x10
0000062D  83EC08            sub esp,byte +0x8
00000630  68FF000000        push dword 0xff
00000635  68A1000000        push dword 0xa1
0000063A  E8A7450000        call 0x4be6
0000063F  83C410            add esp,byte +0x10
00000642  90                nop
00000643  C9                leave
00000644  C3                ret
00000645  55                push ebp
00000646  89E5              mov ebp,esp
00000648  83EC08            sub esp,byte +0x8
0000064B  6A07              push byte +0x7
0000064D  68FBB50100        push dword 0x1b5fb
00000652  6AFF              push byte -0x1
00000654  6A00              push byte +0x0
00000656  E8A1430000        call 0x49fc
0000065B  83C410            add esp,byte +0x10
0000065E  E870FEFFFF        call 0x4d3
00000663  6A07              push byte +0x7
00000665  680EB60100        push dword 0x1b60e
0000066A  6AFF              push byte -0x1
0000066C  6A00              push byte +0x0
0000066E  E889430000        call 0x49fc
00000673  83C410            add esp,byte +0x10
00000676  E8CBFEFFFF        call 0x546
0000067B  B86A000100        mov eax,0x1006a
00000680  688E000000        push dword 0x8e
00000685  6A08              push byte +0x8
00000687  50                push eax
00000688  6A00              push byte +0x0
0000068A  E8DAFDFFFF        call 0x469
0000068F  83C410            add esp,byte +0x10
00000692  B870000100        mov eax,0x10070
00000697  688E000000        push dword 0x8e
0000069C  6A08              push byte +0x8
0000069E  50                push eax
0000069F  6A01              push byte +0x1
000006A1  E8C3FDFFFF        call 0x469
000006A6  83C410            add esp,byte +0x10
000006A9  B876000100        mov eax,0x10076
000006AE  688E000000        push dword 0x8e
000006B3  6A08              push byte +0x8
000006B5  50                push eax
000006B6  6A02              push byte +0x2
000006B8  E8ACFDFFFF        call 0x469
000006BD  83C410            add esp,byte +0x10
000006C0  B87C000100        mov eax,0x1007c
000006C5  688E000000        push dword 0x8e
000006CA  6A08              push byte +0x8
000006CC  50                push eax
000006CD  6A03              push byte +0x3
000006CF  E895FDFFFF        call 0x469
000006D4  83C410            add esp,byte +0x10
000006D7  B882000100        mov eax,0x10082
000006DC  688E000000        push dword 0x8e
000006E1  6A08              push byte +0x8
000006E3  50                push eax
000006E4  6A04              push byte +0x4
000006E6  E87EFDFFFF        call 0x469
000006EB  83C410            add esp,byte +0x10
000006EE  B888000100        mov eax,0x10088
000006F3  688E000000        push dword 0x8e
000006F8  6A08              push byte +0x8
000006FA  50                push eax
000006FB  6A05              push byte +0x5
000006FD  E867FDFFFF        call 0x469
00000702  83C410            add esp,byte +0x10
00000705  B88E000100        mov eax,0x1008e
0000070A  688E000000        push dword 0x8e
0000070F  6A08              push byte +0x8
00000711  50                push eax
00000712  6A06              push byte +0x6
00000714  E850FDFFFF        call 0x469
00000719  83C410            add esp,byte +0x10
0000071C  B894000100        mov eax,0x10094
00000721  688E000000        push dword 0x8e
00000726  6A08              push byte +0x8
00000728  50                push eax
00000729  6A07              push byte +0x7
0000072B  E839FDFFFF        call 0x469
00000730  83C410            add esp,byte +0x10
00000733  B89A000100        mov eax,0x1009a
00000738  688E000000        push dword 0x8e
0000073D  6A08              push byte +0x8
0000073F  50                push eax
00000740  6A08              push byte +0x8
00000742  E822FDFFFF        call 0x469
00000747  83C410            add esp,byte +0x10
0000074A  B89E000100        mov eax,0x1009e
0000074F  688E000000        push dword 0x8e
00000754  6A08              push byte +0x8
00000756  50                push eax
00000757  6A09              push byte +0x9
00000759  E80BFDFFFF        call 0x469
0000075E  83C410            add esp,byte +0x10
00000761  B8A7000100        mov eax,0x100a7
00000766  688E000000        push dword 0x8e
0000076B  6A08              push byte +0x8
0000076D  50                push eax
0000076E  6A0A              push byte +0xa
00000770  E8F4FCFFFF        call 0x469
00000775  83C410            add esp,byte +0x10
00000778  B8AE000100        mov eax,0x100ae
0000077D  688E000000        push dword 0x8e
00000782  6A08              push byte +0x8
00000784  50                push eax
00000785  6A0B              push byte +0xb
00000787  E8DDFCFFFF        call 0x469
0000078C  83C410            add esp,byte +0x10
0000078F  B8B5000100        mov eax,0x100b5
00000794  688E000000        push dword 0x8e
00000799  6A08              push byte +0x8
0000079B  50                push eax
0000079C  6A0C              push byte +0xc
0000079E  E8C6FCFFFF        call 0x469
000007A3  83C410            add esp,byte +0x10
000007A6  B8BC000100        mov eax,0x100bc
000007AB  688E000000        push dword 0x8e
000007B0  6A08              push byte +0x8
000007B2  50                push eax
000007B3  6A0D              push byte +0xd
000007B5  E8AFFCFFFF        call 0x469
000007BA  83C410            add esp,byte +0x10
000007BD  B8C3000100        mov eax,0x100c3
000007C2  688E000000        push dword 0x8e
000007C7  6A08              push byte +0x8
000007C9  50                push eax
000007CA  6A0E              push byte +0xe
000007CC  E898FCFFFF        call 0x469
000007D1  83C410            add esp,byte +0x10
000007D4  B8CA000100        mov eax,0x100ca
000007D9  688E000000        push dword 0x8e
000007DE  6A08              push byte +0x8
000007E0  50                push eax
000007E1  6A0F              push byte +0xf
000007E3  E881FCFFFF        call 0x469
000007E8  83C410            add esp,byte +0x10
000007EB  B8D3000100        mov eax,0x100d3
000007F0  688E000000        push dword 0x8e
000007F5  6A08              push byte +0x8
000007F7  50                push eax
000007F8  6A20              push byte +0x20
000007FA  E86AFCFFFF        call 0x469
000007FF  83C410            add esp,byte +0x10
00000802  B8DC000100        mov eax,0x100dc
00000807  688E000000        push dword 0x8e
0000080C  6A08              push byte +0x8
0000080E  50                push eax
0000080F  6A21              push byte +0x21
00000811  E853FCFFFF        call 0x469
00000816  83C410            add esp,byte +0x10
00000819  B8E5000100        mov eax,0x100e5
0000081E  688E000000        push dword 0x8e
00000823  6A08              push byte +0x8
00000825  50                push eax
00000826  6A22              push byte +0x22
00000828  E83CFCFFFF        call 0x469
0000082D  83C410            add esp,byte +0x10
00000830  B8EE000100        mov eax,0x100ee
00000835  688E000000        push dword 0x8e
0000083A  6A08              push byte +0x8
0000083C  50                push eax
0000083D  6A23              push byte +0x23
0000083F  E825FCFFFF        call 0x469
00000844  83C410            add esp,byte +0x10
00000847  B8F7000100        mov eax,0x100f7
0000084C  688E000000        push dword 0x8e
00000851  6A08              push byte +0x8
00000853  50                push eax
00000854  6A24              push byte +0x24
00000856  E80EFCFFFF        call 0x469
0000085B  83C410            add esp,byte +0x10
0000085E  B800010100        mov eax,0x10100
00000863  688E000000        push dword 0x8e
00000868  6A08              push byte +0x8
0000086A  50                push eax
0000086B  6A25              push byte +0x25
0000086D  E8F7FBFFFF        call 0x469
00000872  83C410            add esp,byte +0x10
00000875  B809010100        mov eax,0x10109
0000087A  688E000000        push dword 0x8e
0000087F  6A08              push byte +0x8
00000881  50                push eax
00000882  6A26              push byte +0x26
00000884  E8E0FBFFFF        call 0x469
00000889  83C410            add esp,byte +0x10
0000088C  B812010100        mov eax,0x10112
00000891  688E000000        push dword 0x8e
00000896  6A08              push byte +0x8
00000898  50                push eax
00000899  6A27              push byte +0x27
0000089B  E8C9FBFFFF        call 0x469
000008A0  83C410            add esp,byte +0x10
000008A3  B81B010100        mov eax,0x1011b
000008A8  688E000000        push dword 0x8e
000008AD  6A08              push byte +0x8
000008AF  50                push eax
000008B0  6A28              push byte +0x28
000008B2  E8B2FBFFFF        call 0x469
000008B7  83C410            add esp,byte +0x10
000008BA  B824010100        mov eax,0x10124
000008BF  688E000000        push dword 0x8e
000008C4  6A08              push byte +0x8
000008C6  50                push eax
000008C7  6A29              push byte +0x29
000008C9  E89BFBFFFF        call 0x469
000008CE  83C410            add esp,byte +0x10
000008D1  B82D010100        mov eax,0x1012d
000008D6  688E000000        push dword 0x8e
000008DB  6A08              push byte +0x8
000008DD  50                push eax
000008DE  6A2A              push byte +0x2a
000008E0  E884FBFFFF        call 0x469
000008E5  83C410            add esp,byte +0x10
000008E8  B836010100        mov eax,0x10136
000008ED  688E000000        push dword 0x8e
000008F2  6A08              push byte +0x8
000008F4  50                push eax
000008F5  6A2B              push byte +0x2b
000008F7  E86DFBFFFF        call 0x469
000008FC  83C410            add esp,byte +0x10
000008FF  B83F010100        mov eax,0x1013f
00000904  688E000000        push dword 0x8e
00000909  6A08              push byte +0x8
0000090B  50                push eax
0000090C  6A2C              push byte +0x2c
0000090E  E856FBFFFF        call 0x469
00000913  83C410            add esp,byte +0x10
00000916  B848010100        mov eax,0x10148
0000091B  688E000000        push dword 0x8e
00000920  6A08              push byte +0x8
00000922  50                push eax
00000923  6A2D              push byte +0x2d
00000925  E83FFBFFFF        call 0x469
0000092A  83C410            add esp,byte +0x10
0000092D  B851010100        mov eax,0x10151
00000932  688E000000        push dword 0x8e
00000937  6A08              push byte +0x8
00000939  50                push eax
0000093A  6A2E              push byte +0x2e
0000093C  E828FBFFFF        call 0x469
00000941  83C410            add esp,byte +0x10
00000944  B85A010100        mov eax,0x1015a
00000949  688E000000        push dword 0x8e
0000094E  6A08              push byte +0x8
00000950  50                push eax
00000951  6A2F              push byte +0x2f
00000953  E811FBFFFF        call 0x469
00000958  83C410            add esp,byte +0x10
0000095B  90                nop
0000095C  C9                leave
0000095D  C3                ret
0000095E  55                push ebp
0000095F  89E5              mov ebp,esp
00000961  90                nop
00000962  5D                pop ebp
00000963  C3                ret
00000964  55                push ebp
00000965  89E5              mov ebp,esp
00000967  83EC18            sub esp,byte +0x18
0000096A  8B4508            mov eax,[ebp+0x8]
0000096D  8B4030            mov eax,[eax+0x30]
00000970  83F827            cmp eax,byte +0x27
00000973  7612              jna 0x987
00000975  83EC08            sub esp,byte +0x8
00000978  6A20              push byte +0x20
0000097A  68A0000000        push dword 0xa0
0000097F  E862420000        call 0x4be6
00000984  83C410            add esp,byte +0x10
00000987  83EC08            sub esp,byte +0x8
0000098A  6A20              push byte +0x20
0000098C  6A20              push byte +0x20
0000098E  E853420000        call 0x4be6
00000993  83C410            add esp,byte +0x10
00000996  8B4508            mov eax,[ebp+0x8]
00000999  8B4030            mov eax,[eax+0x30]
0000099C  8B048560310200    mov eax,[eax*4+0x23160]
000009A3  85C0              test eax,eax
000009A5  741E              jz 0x9c5
000009A7  8B4508            mov eax,[ebp+0x8]
000009AA  8B4030            mov eax,[eax+0x30]
000009AD  8B048560310200    mov eax,[eax*4+0x23160]
000009B4  8945F4            mov [ebp-0xc],eax
000009B7  83EC0C            sub esp,byte +0xc
000009BA  FF7508            push dword [ebp+0x8]
000009BD  8B45F4            mov eax,[ebp-0xc]
000009C0  FFD0              call eax
000009C2  83C410            add esp,byte +0x10
000009C5  90                nop
000009C6  C9                leave
000009C7  C3                ret
000009C8  55                push ebp
000009C9  89E5              mov ebp,esp
000009CB  83EC04            sub esp,byte +0x4
000009CE  8B4508            mov eax,[ebp+0x8]
000009D1  8845FC            mov [ebp-0x4],al
000009D4  0FB645FC          movzx eax,byte [ebp-0x4]
000009D8  8B550C            mov edx,[ebp+0xc]
000009DB  89148560310200    mov [eax*4+0x23160],edx
000009E2  90                nop
000009E3  C9                leave
000009E4  C3                ret
000009E5  55                push ebp
000009E6  89E5              mov ebp,esp
000009E8  A1A0DA0100        mov eax,[0x1daa0]
000009ED  C74004F4FFDF00    mov dword [eax+0x4],0xdffff4
000009F4  A1A0DA0100        mov eax,[0x1daa0]
000009F9  C7400801000000    mov dword [eax+0x8],0x1
00000A00  A1A0DA0100        mov eax,[0x1daa0]
00000A05  C70000000000      mov dword [eax],0x0
00000A0B  90                nop
00000A0C  5D                pop ebp
00000A0D  C3                ret
00000A0E  55                push ebp
00000A0F  89E5              mov ebp,esp
00000A11  83EC10            sub esp,byte +0x10
00000A14  837D0800          cmp dword [ebp+0x8],byte +0x0
00000A18  750A              jnz 0xa24
00000A1A  B800000000        mov eax,0x0
00000A1F  E9AC000000        jmp 0xad0
00000A24  8B4508            mov eax,[ebp+0x8]
00000A27  83C00F            add eax,byte +0xf
00000A2A  83E0F0            and eax,byte -0x10
00000A2D  894508            mov [ebp+0x8],eax
00000A30  A1A0DA0100        mov eax,[0x1daa0]
00000A35  8945FC            mov [ebp-0x4],eax
00000A38  E984000000        jmp 0xac1
00000A3D  8B45FC            mov eax,[ebp-0x4]
00000A40  8B4008            mov eax,[eax+0x8]
00000A43  85C0              test eax,eax
00000A45  7472              jz 0xab9
00000A47  8B45FC            mov eax,[ebp-0x4]
00000A4A  8B4004            mov eax,[eax+0x4]
00000A4D  394508            cmp [ebp+0x8],eax
00000A50  7767              ja 0xab9
00000A52  8B45FC            mov eax,[ebp-0x4]
00000A55  8B4004            mov eax,[eax+0x4]
00000A58  8B5508            mov edx,[ebp+0x8]
00000A5B  83C21C            add edx,byte +0x1c
00000A5E  39D0              cmp eax,edx
00000A60  7645              jna 0xaa7
00000A62  8B4508            mov eax,[ebp+0x8]
00000A65  8D500C            lea edx,[eax+0xc]
00000A68  8B45FC            mov eax,[ebp-0x4]
00000A6B  01D0              add eax,edx
00000A6D  8945F8            mov [ebp-0x8],eax
00000A70  8B45FC            mov eax,[ebp-0x4]
00000A73  8B4004            mov eax,[eax+0x4]
00000A76  2B4508            sub eax,[ebp+0x8]
00000A79  8D50F4            lea edx,[eax-0xc]
00000A7C  8B45F8            mov eax,[ebp-0x8]
00000A7F  895004            mov [eax+0x4],edx
00000A82  8B45F8            mov eax,[ebp-0x8]
00000A85  C7400801000000    mov dword [eax+0x8],0x1
00000A8C  8B45FC            mov eax,[ebp-0x4]
00000A8F  8B10              mov edx,[eax]
00000A91  8B45F8            mov eax,[ebp-0x8]
00000A94  8910              mov [eax],edx
00000A96  8B45FC            mov eax,[ebp-0x4]
00000A99  8B5508            mov edx,[ebp+0x8]
00000A9C  895004            mov [eax+0x4],edx
00000A9F  8B45FC            mov eax,[ebp-0x4]
00000AA2  8B55F8            mov edx,[ebp-0x8]
00000AA5  8910              mov [eax],edx
00000AA7  8B45FC            mov eax,[ebp-0x4]
00000AAA  C7400800000000    mov dword [eax+0x8],0x0
00000AB1  8B45FC            mov eax,[ebp-0x4]
00000AB4  83C00C            add eax,byte +0xc
00000AB7  EB17              jmp short 0xad0
00000AB9  8B45FC            mov eax,[ebp-0x4]
00000ABC  8B00              mov eax,[eax]
00000ABE  8945FC            mov [ebp-0x4],eax
00000AC1  837DFC00          cmp dword [ebp-0x4],byte +0x0
00000AC5  0F8572FFFFFF      jnz near 0xa3d
00000ACB  B800000000        mov eax,0x0
00000AD0  C9                leave
00000AD1  C3                ret
00000AD2  55                push ebp
00000AD3  89E5              mov ebp,esp
00000AD5  83EC10            sub esp,byte +0x10
00000AD8  837D0800          cmp dword [ebp+0x8],byte +0x0
00000ADC  744F              jz 0xb2d
00000ADE  8B4508            mov eax,[ebp+0x8]
00000AE1  83E80C            sub eax,byte +0xc
00000AE4  8945FC            mov [ebp-0x4],eax
00000AE7  8B45FC            mov eax,[ebp-0x4]
00000AEA  C7400801000000    mov dword [eax+0x8],0x1
00000AF1  8B45FC            mov eax,[ebp-0x4]
00000AF4  8B00              mov eax,[eax]
00000AF6  85C0              test eax,eax
00000AF8  7434              jz 0xb2e
00000AFA  8B45FC            mov eax,[ebp-0x4]
00000AFD  8B00              mov eax,[eax]
00000AFF  8B4008            mov eax,[eax+0x8]
00000B02  85C0              test eax,eax
00000B04  7428              jz 0xb2e
00000B06  8B45FC            mov eax,[ebp-0x4]
00000B09  8B5004            mov edx,[eax+0x4]
00000B0C  8B45FC            mov eax,[ebp-0x4]
00000B0F  8B00              mov eax,[eax]
00000B11  8B4004            mov eax,[eax+0x4]
00000B14  01D0              add eax,edx
00000B16  8D500C            lea edx,[eax+0xc]
00000B19  8B45FC            mov eax,[ebp-0x4]
00000B1C  895004            mov [eax+0x4],edx
00000B1F  8B45FC            mov eax,[ebp-0x4]
00000B22  8B00              mov eax,[eax]
00000B24  8B10              mov edx,[eax]
00000B26  8B45FC            mov eax,[ebp-0x4]
00000B29  8910              mov [eax],edx
00000B2B  EB01              jmp short 0xb2e
00000B2D  90                nop
00000B2E  C9                leave
00000B2F  C3                ret
00000B30  55                push ebp
00000B31  89E5              mov ebp,esp
00000B33  83EC28            sub esp,byte +0x28
00000B36  6A1F              push byte +0x1f
00000B38  6820B60100        push dword 0x1b620
00000B3D  6AFF              push byte -0x1
00000B3F  6A00              push byte +0x0
00000B41  E8B63E0000        call 0x49fc
00000B46  83C410            add esp,byte +0x10
00000B49  C745E40000E000    mov dword [ebp-0x1c],0xe00000
00000B50  C745F400000000    mov dword [ebp-0xc],0x0
00000B57  C745F000000000    mov dword [ebp-0x10],0x0
00000B5E  C745EC00000000    mov dword [ebp-0x14],0x0
00000B65  A1A0DA0100        mov eax,[0x1daa0]
00000B6A  8945E8            mov [ebp-0x18],eax
00000B6D  EB3A              jmp short 0xba9
00000B6F  8B45E8            mov eax,[ebp-0x18]
00000B72  8B4008            mov eax,[eax+0x8]
00000B75  85C0              test eax,eax
00000B77  7513              jnz 0xb8c
00000B79  8B45E8            mov eax,[ebp-0x18]
00000B7C  8B5004            mov edx,[eax+0x4]
00000B7F  8B45F4            mov eax,[ebp-0xc]
00000B82  01D0              add eax,edx
00000B84  83C00C            add eax,byte +0xc
00000B87  8945F4            mov [ebp-0xc],eax
00000B8A  EB11              jmp short 0xb9d
00000B8C  8B45E8            mov eax,[ebp-0x18]
00000B8F  8B5004            mov edx,[eax+0x4]
00000B92  8B45F0            mov eax,[ebp-0x10]
00000B95  01D0              add eax,edx
00000B97  83C00C            add eax,byte +0xc
00000B9A  8945F0            mov [ebp-0x10],eax
00000B9D  8345EC01          add dword [ebp-0x14],byte +0x1
00000BA1  8B45E8            mov eax,[ebp-0x18]
00000BA4  8B00              mov eax,[eax]
00000BA6  8945E8            mov [ebp-0x18],eax
00000BA9  837DE800          cmp dword [ebp-0x18],byte +0x0
00000BAD  75C0              jnz 0xb6f
00000BAF  6A1F              push byte +0x1f
00000BB1  683BB60100        push dword 0x1b63b
00000BB6  6AFF              push byte -0x1
00000BB8  6A00              push byte +0x0
00000BBA  E83D3E0000        call 0x49fc
00000BBF  83C410            add esp,byte +0x10
00000BC2  6A1F              push byte +0x1f
00000BC4  6852B60100        push dword 0x1b652
00000BC9  6AFF              push byte -0x1
00000BCB  6A00              push byte +0x0
00000BCD  E82A3E0000        call 0x49fc
00000BD2  83C410            add esp,byte +0x10
00000BD5  8B45F4            mov eax,[ebp-0xc]
00000BD8  83EC0C            sub esp,byte +0xc
00000BDB  50                push eax
00000BDC  E8063F0000        call 0x4ae7
00000BE1  83C410            add esp,byte +0x10
00000BE4  6A1F              push byte +0x1f
00000BE6  685FB60100        push dword 0x1b65f
00000BEB  6AFF              push byte -0x1
00000BED  6A00              push byte +0x0
00000BEF  E8083E0000        call 0x49fc
00000BF4  83C410            add esp,byte +0x10
00000BF7  6A1F              push byte +0x1f
00000BF9  6861B60100        push dword 0x1b661
00000BFE  6AFF              push byte -0x1
00000C00  6A00              push byte +0x0
00000C02  E8F53D0000        call 0x49fc
00000C07  83C410            add esp,byte +0x10
00000C0A  8B45F0            mov eax,[ebp-0x10]
00000C0D  83EC0C            sub esp,byte +0xc
00000C10  50                push eax
00000C11  E8D13E0000        call 0x4ae7
00000C16  83C410            add esp,byte +0x10
00000C19  6A1F              push byte +0x1f
00000C1B  685FB60100        push dword 0x1b65f
00000C20  6AFF              push byte -0x1
00000C22  6A00              push byte +0x0
00000C24  E8D33D0000        call 0x49fc
00000C29  83C410            add esp,byte +0x10
00000C2C  6A1F              push byte +0x1f
00000C2E  686EB60100        push dword 0x1b66e
00000C33  6AFF              push byte -0x1
00000C35  6A00              push byte +0x0
00000C37  E8C03D0000        call 0x49fc
00000C3C  83C410            add esp,byte +0x10
00000C3F  83EC0C            sub esp,byte +0xc
00000C42  FF75EC            push dword [ebp-0x14]
00000C45  E89D3E0000        call 0x4ae7
00000C4A  83C410            add esp,byte +0x10
00000C4D  6A1F              push byte +0x1f
00000C4F  685FB60100        push dword 0x1b65f
00000C54  6AFF              push byte -0x1
00000C56  6A00              push byte +0x0
00000C58  E89F3D0000        call 0x49fc
00000C5D  83C410            add esp,byte +0x10
00000C60  90                nop
00000C61  C9                leave
00000C62  C3                ret
00000C63  55                push ebp
00000C64  89E5              mov ebp,esp
00000C66  83EC08            sub esp,byte +0x8
00000C69  83EC08            sub esp,byte +0x8
00000C6C  68C0DA0100        push dword 0x1dac0
00000C71  FF7508            push dword [ebp+0x8]
00000C74  E8754F0000        call 0x5bee
00000C79  83C410            add esp,byte +0x10
00000C7C  90                nop
00000C7D  C9                leave
00000C7E  C3                ret
00000C7F  55                push ebp
00000C80  89E5              mov ebp,esp
00000C82  83EC28            sub esp,byte +0x28
00000C85  8B4508            mov eax,[ebp+0x8]
00000C88  668945E4          mov [ebp-0x1c],ax
00000C8C  0FB755E4          movzx edx,word [ebp-0x1c]
00000C90  0FB745E4          movzx eax,word [ebp-0x1c]
00000C94  66D1E8            shr ax,1
00000C97  0FB7C0            movzx eax,ax
00000C9A  01D0              add eax,edx
00000C9C  8945F0            mov [ebp-0x10],eax
00000C9F  8B45F0            mov eax,[ebp-0x10]
00000CA2  C1E809            shr eax,byte 0x9
00000CA5  89C2              mov edx,eax
00000CA7  A168350200        mov eax,[0x23568]
00000CAC  01D0              add eax,edx
00000CAE  8945EC            mov [ebp-0x14],eax
00000CB1  8B45F0            mov eax,[ebp-0x10]
00000CB4  25FF010000        and eax,0x1ff
00000CB9  8945E8            mov [ebp-0x18],eax
00000CBC  83EC08            sub esp,byte +0x8
00000CBF  68801C0200        push dword 0x21c80
00000CC4  FF75EC            push dword [ebp-0x14]
00000CC7  E831510000        call 0x5dfd
00000CCC  83C410            add esp,byte +0x10
00000CCF  0FB745E4          movzx eax,word [ebp-0x1c]
00000CD3  83E001            and eax,byte +0x1
00000CD6  85C0              test eax,eax
00000CD8  7443              jz 0xd1d
00000CDA  8B45E8            mov eax,[ebp-0x18]
00000CDD  05801C0200        add eax,0x21c80
00000CE2  0FB600            movzx eax,byte [eax]
00000CE5  C0E804            shr al,byte 0x4
00000CE8  0FB6C0            movzx eax,al
00000CEB  668945F6          mov [ebp-0xa],ax
00000CEF  8B45E8            mov eax,[ebp-0x18]
00000CF2  83C001            add eax,byte +0x1
00000CF5  3DFF010000        cmp eax,0x1ff
00000CFA  7763              ja 0xd5f
00000CFC  8B45E8            mov eax,[ebp-0x18]
00000CFF  83C001            add eax,byte +0x1
00000D02  0FB680801C0200    movzx eax,byte [eax+0x21c80]
00000D09  0FB6C0            movzx eax,al
00000D0C  C1E004            shl eax,byte 0x4
00000D0F  89C2              mov edx,eax
00000D11  0FB745F6          movzx eax,word [ebp-0xa]
00000D15  09D0              or eax,edx
00000D17  668945F6          mov [ebp-0xa],ax
00000D1B  EB42              jmp short 0xd5f
00000D1D  8B45E8            mov eax,[ebp-0x18]
00000D20  05801C0200        add eax,0x21c80
00000D25  0FB600            movzx eax,byte [eax]
00000D28  0FB6C0            movzx eax,al
00000D2B  668945F6          mov [ebp-0xa],ax
00000D2F  8B45E8            mov eax,[ebp-0x18]
00000D32  83C001            add eax,byte +0x1
00000D35  3DFF010000        cmp eax,0x1ff
00000D3A  7723              ja 0xd5f
00000D3C  8B45E8            mov eax,[ebp-0x18]
00000D3F  83C001            add eax,byte +0x1
00000D42  0FB680801C0200    movzx eax,byte [eax+0x21c80]
00000D49  0FB6C0            movzx eax,al
00000D4C  C1E008            shl eax,byte 0x8
00000D4F  6625000F          and ax,0xf00
00000D53  89C2              mov edx,eax
00000D55  0FB745F6          movzx eax,word [ebp-0xa]
00000D59  09D0              or eax,edx
00000D5B  668945F6          mov [ebp-0xa],ax
00000D5F  0FB745F6          movzx eax,word [ebp-0xa]
00000D63  C9                leave
00000D64  C3                ret
00000D65  55                push ebp
00000D66  89E5              mov ebp,esp
00000D68  83EC18            sub esp,byte +0x18
00000D6B  A1601C0200        mov eax,[0x21c60]
00000D70  85C0              test eax,eax
00000D72  0F85F2000000      jnz near 0xe6a
00000D78  83EC08            sub esp,byte +0x8
00000D7B  68801C0200        push dword 0x21c80
00000D80  6A00              push byte +0x0
00000D82  E876500000        call 0x5dfd
00000D87  83C410            add esp,byte +0x10
00000D8A  85C0              test eax,eax
00000D8C  7515              jnz 0xda3
00000D8E  83EC0C            sub esp,byte +0xc
00000D91  6878B60100        push dword 0x1b678
00000D96  E88F420000        call 0x502a
00000D9B  83C410            add esp,byte +0x10
00000D9E  E9C8000000        jmp 0xe6b
00000DA3  C745F4801C0200    mov dword [ebp-0xc],0x21c80
00000DAA  0FB6057E1E0200    movzx eax,byte [dword 0x21e7e]
00000DB1  3C55              cmp al,0x55
00000DB3  750B              jnz 0xdc0
00000DB5  0FB6057F1E0200    movzx eax,byte [dword 0x21e7f]
00000DBC  3CAA              cmp al,0xaa
00000DBE  7415              jz 0xdd5
00000DC0  83EC0C            sub esp,byte +0xc
00000DC3  689CB60100        push dword 0x1b69c
00000DC8  E85D420000        call 0x502a
00000DCD  83C410            add esp,byte +0x10
00000DD0  E996000000        jmp 0xe6b
00000DD5  8B45F4            mov eax,[ebp-0xc]
00000DD8  0FB7400E          movzx eax,word [eax+0xe]
00000DDC  0FB7C0            movzx eax,ax
00000DDF  A368350200        mov [0x23568],eax
00000DE4  8B45F4            mov eax,[ebp-0xc]
00000DE7  0FB64010          movzx eax,byte [eax+0x10]
00000DEB  0FB6D0            movzx edx,al
00000DEE  8B45F4            mov eax,[ebp-0xc]
00000DF1  0FB74016          movzx eax,word [eax+0x16]
00000DF5  0FB7C0            movzx eax,ax
00000DF8  0FAFC2            imul eax,edx
00000DFB  89C2              mov edx,eax
00000DFD  A168350200        mov eax,[0x23568]
00000E02  01D0              add eax,edx
00000E04  A36C350200        mov [0x2356c],eax
00000E09  8B45F4            mov eax,[ebp-0xc]
00000E0C  0FB74011          movzx eax,word [eax+0x11]
00000E10  0FB7C0            movzx eax,ax
00000E13  C1E005            shl eax,byte 0x5
00000E16  8D90FF010000      lea edx,[eax+0x1ff]
00000E1C  85C0              test eax,eax
00000E1E  0F48C2            cmovs eax,edx
00000E21  C1F809            sar eax,byte 0x9
00000E24  A360350200        mov [0x23560],eax
00000E29  8B45F4            mov eax,[ebp-0xc]
00000E2C  0FB74011          movzx eax,word [eax+0x11]
00000E30  0FB7C0            movzx eax,ax
00000E33  C1E005            shl eax,byte 0x5
00000E36  25E0010000        and eax,0x1e0
00000E3B  85C0              test eax,eax
00000E3D  740D              jz 0xe4c
00000E3F  A160350200        mov eax,[0x23560]
00000E44  83C001            add eax,byte +0x1
00000E47  A360350200        mov [0x23560],eax
00000E4C  8B156C350200      mov edx,[dword 0x2356c]
00000E52  A160350200        mov eax,[0x23560]
00000E57  01D0              add eax,edx
00000E59  A364350200        mov [0x23564],eax
00000E5E  C705601C02000100  mov dword [dword 0x21c60],0x1
         -0000
00000E68  EB01              jmp short 0xe6b
00000E6A  90                nop
00000E6B  C9                leave
00000E6C  C3                ret
00000E6D  55                push ebp
00000E6E  89E5              mov ebp,esp
00000E70  83EC48            sub esp,byte +0x48
00000E73  A1601C0200        mov eax,[0x21c60]
00000E78  85C0              test eax,eax
00000E7A  7505              jnz 0xe81
00000E7C  E8E4FEFFFF        call 0xd65
00000E81  A1601C0200        mov eax,[0x21c60]
00000E86  85C0              test eax,eax
00000E88  0F84C7020000      jz near 0x1155
00000E8E  83EC0C            sub esp,byte +0xc
00000E91  68BBB60100        push dword 0x1b6bb
00000E96  E88F410000        call 0x502a
00000E9B  83C410            add esp,byte +0x10
00000E9E  83EC0C            sub esp,byte +0xc
00000EA1  68C0DA0100        push dword 0x1dac0
00000EA6  E87F410000        call 0x502a
00000EAB  83C410            add esp,byte +0x10
00000EAE  83EC0C            sub esp,byte +0xc
00000EB1  68C9B60100        push dword 0x1b6c9
00000EB6  E86F410000        call 0x502a
00000EBB  83C410            add esp,byte +0x10
00000EBE  C745F400000000    mov dword [ebp-0xc],0x0
00000EC5  0FB705641C0200    movzx eax,word [dword 0x21c64]
00000ECC  668945F2          mov [ebp-0xe],ax
00000ED0  C745EC00000000    mov dword [ebp-0x14],0x0
00000ED7  0FB705641C0200    movzx eax,word [dword 0x21c64]
00000EDE  6685C0            test ax,ax
00000EE1  7532              jnz 0xf15
00000EE3  8B55EC            mov edx,[ebp-0x14]
00000EE6  A160350200        mov eax,[0x23560]
00000EEB  39C2              cmp edx,eax
00000EED  0F8343020000      jnc near 0x1136
00000EF3  8B156C350200      mov edx,[dword 0x2356c]
00000EF9  8B45EC            mov eax,[ebp-0x14]
00000EFC  01D0              add eax,edx
00000EFE  83EC08            sub esp,byte +0x8
00000F01  68801C0200        push dword 0x21c80
00000F06  50                push eax
00000F07  E8F14E0000        call 0x5dfd
00000F0C  83C410            add esp,byte +0x10
00000F0F  8345EC01          add dword [ebp-0x14],byte +0x1
00000F13  EB24              jmp short 0xf39
00000F15  0FB755F2          movzx edx,word [ebp-0xe]
00000F19  A164350200        mov eax,[0x23564]
00000F1E  01D0              add eax,edx
00000F20  83E802            sub eax,byte +0x2
00000F23  8945D4            mov [ebp-0x2c],eax
00000F26  83EC08            sub esp,byte +0x8
00000F29  68801C0200        push dword 0x21c80
00000F2E  FF75D4            push dword [ebp-0x2c]
00000F31  E8C74E0000        call 0x5dfd
00000F36  83C410            add esp,byte +0x10
00000F39  C745D0801C0200    mov dword [ebp-0x30],0x21c80
00000F40  C745E800000000    mov dword [ebp-0x18],0x0
00000F47  E9AF010000        jmp 0x10fb
00000F4C  8B45E8            mov eax,[ebp-0x18]
00000F4F  C1E005            shl eax,byte 0x5
00000F52  89C2              mov edx,eax
00000F54  8B45D0            mov eax,[ebp-0x30]
00000F57  01D0              add eax,edx
00000F59  0FB600            movzx eax,byte [eax]
00000F5C  84C0              test al,al
00000F5E  0F84D5010000      jz near 0x1139
00000F64  8B45E8            mov eax,[ebp-0x18]
00000F67  C1E005            shl eax,byte 0x5
00000F6A  89C2              mov edx,eax
00000F6C  8B45D0            mov eax,[ebp-0x30]
00000F6F  01D0              add eax,edx
00000F71  0FB600            movzx eax,byte [eax]
00000F74  3CE5              cmp al,0xe5
00000F76  0F8477010000      jz near 0x10f3
00000F7C  8B45E8            mov eax,[ebp-0x18]
00000F7F  C1E005            shl eax,byte 0x5
00000F82  89C2              mov edx,eax
00000F84  8B45D0            mov eax,[ebp-0x30]
00000F87  01D0              add eax,edx
00000F89  0FB6400B          movzx eax,byte [eax+0xb]
00000F8D  3C0F              cmp al,0xf
00000F8F  0F8461010000      jz near 0x10f6
00000F95  C745E400000000    mov dword [ebp-0x1c],0x0
00000F9C  C745E000000000    mov dword [ebp-0x20],0x0
00000FA3  EB40              jmp short 0xfe5
00000FA5  8B45E8            mov eax,[ebp-0x18]
00000FA8  C1E005            shl eax,byte 0x5
00000FAB  89C2              mov edx,eax
00000FAD  8B45D0            mov eax,[ebp-0x30]
00000FB0  01C2              add edx,eax
00000FB2  8B45E0            mov eax,[ebp-0x20]
00000FB5  01D0              add eax,edx
00000FB7  0FB600            movzx eax,byte [eax]
00000FBA  3C20              cmp al,0x20
00000FBC  742F              jz 0xfed
00000FBE  8B45E8            mov eax,[ebp-0x18]
00000FC1  C1E005            shl eax,byte 0x5
00000FC4  89C2              mov edx,eax
00000FC6  8B45D0            mov eax,[ebp-0x30]
00000FC9  8D0C02            lea ecx,[edx+eax]
00000FCC  8B45E4            mov eax,[ebp-0x1c]
00000FCF  8D5001            lea edx,[eax+0x1]
00000FD2  8955E4            mov [ebp-0x1c],edx
00000FD5  8B55E0            mov edx,[ebp-0x20]
00000FD8  01CA              add edx,ecx
00000FDA  0FB612            movzx edx,byte [edx]
00000FDD  885405C0          mov [ebp+eax-0x40],dl
00000FE1  8345E001          add dword [ebp-0x20],byte +0x1
00000FE5  837DE007          cmp dword [ebp-0x20],byte +0x7
00000FE9  7EBA              jng 0xfa5
00000FEB  EB01              jmp short 0xfee
00000FED  90                nop
00000FEE  8B45E8            mov eax,[ebp-0x18]
00000FF1  C1E005            shl eax,byte 0x5
00000FF4  89C2              mov edx,eax
00000FF6  8B45D0            mov eax,[ebp-0x30]
00000FF9  01D0              add eax,edx
00000FFB  0FB64008          movzx eax,byte [eax+0x8]
00000FFF  3C20              cmp al,0x20
00001001  7466              jz 0x1069
00001003  8B45E4            mov eax,[ebp-0x1c]
00001006  8D5001            lea edx,[eax+0x1]
00001009  8955E4            mov [ebp-0x1c],edx
0000100C  C64405C02E        mov byte [ebp+eax-0x40],0x2e
00001011  C745DC00000000    mov dword [ebp-0x24],0x0
00001018  EB46              jmp short 0x1060
0000101A  8B45E8            mov eax,[ebp-0x18]
0000101D  C1E005            shl eax,byte 0x5
00001020  89C2              mov edx,eax
00001022  8B45D0            mov eax,[ebp-0x30]
00001025  01C2              add edx,eax
00001027  8B45DC            mov eax,[ebp-0x24]
0000102A  01D0              add eax,edx
0000102C  83C008            add eax,byte +0x8
0000102F  0FB600            movzx eax,byte [eax]
00001032  3C20              cmp al,0x20
00001034  7432              jz 0x1068
00001036  8B45E8            mov eax,[ebp-0x18]
00001039  C1E005            shl eax,byte 0x5
0000103C  89C2              mov edx,eax
0000103E  8B45D0            mov eax,[ebp-0x30]
00001041  8D0C02            lea ecx,[edx+eax]
00001044  8B45E4            mov eax,[ebp-0x1c]
00001047  8D5001            lea edx,[eax+0x1]
0000104A  8955E4            mov [ebp-0x1c],edx
0000104D  8B55DC            mov edx,[ebp-0x24]
00001050  01CA              add edx,ecx
00001052  83C208            add edx,byte +0x8
00001055  0FB612            movzx edx,byte [edx]
00001058  885405C0          mov [ebp+eax-0x40],dl
0000105C  8345DC01          add dword [ebp-0x24],byte +0x1
00001060  837DDC02          cmp dword [ebp-0x24],byte +0x2
00001064  7EB4              jng 0x101a
00001066  EB01              jmp short 0x1069
00001068  90                nop
00001069  8D55C0            lea edx,[ebp-0x40]
0000106C  8B45E4            mov eax,[ebp-0x1c]
0000106F  01D0              add eax,edx
00001071  C60000            mov byte [eax],0x0
00001074  83EC0C            sub esp,byte +0xc
00001077  8D45C0            lea eax,[ebp-0x40]
0000107A  50                push eax
0000107B  E8AA3F0000        call 0x502a
00001080  83C410            add esp,byte +0x10
00001083  B80C000000        mov eax,0xc
00001088  2B45E4            sub eax,[ebp-0x1c]
0000108B  8945CC            mov [ebp-0x34],eax
0000108E  C745D800000000    mov dword [ebp-0x28],0x0
00001095  EB11              jmp short 0x10a8
00001097  83EC0C            sub esp,byte +0xc
0000109A  6A20              push byte +0x20
0000109C  E8173E0000        call 0x4eb8
000010A1  83C410            add esp,byte +0x10
000010A4  8345D801          add dword [ebp-0x28],byte +0x1
000010A8  8B45D8            mov eax,[ebp-0x28]
000010AB  3B45CC            cmp eax,[ebp-0x34]
000010AE  7CE7              jl 0x1097
000010B0  8B45E8            mov eax,[ebp-0x18]
000010B3  C1E005            shl eax,byte 0x5
000010B6  89C2              mov edx,eax
000010B8  8B45D0            mov eax,[ebp-0x30]
000010BB  01D0              add eax,edx
000010BD  0FB6400B          movzx eax,byte [eax+0xb]
000010C1  0FB6C0            movzx eax,al
000010C4  83E010            and eax,byte +0x10
000010C7  85C0              test eax,eax
000010C9  7412              jz 0x10dd
000010CB  83EC0C            sub esp,byte +0xc
000010CE  68CBB60100        push dword 0x1b6cb
000010D3  E8523F0000        call 0x502a
000010D8  83C410            add esp,byte +0x10
000010DB  EB10              jmp short 0x10ed
000010DD  83EC0C            sub esp,byte +0xc
000010E0  68D4B60100        push dword 0x1b6d4
000010E5  E8403F0000        call 0x502a
000010EA  83C410            add esp,byte +0x10
000010ED  8345F401          add dword [ebp-0xc],byte +0x1
000010F1  EB04              jmp short 0x10f7
000010F3  90                nop
000010F4  EB01              jmp short 0x10f7
000010F6  90                nop
000010F7  8345E801          add dword [ebp-0x18],byte +0x1
000010FB  837DE80F          cmp dword [ebp-0x18],byte +0xf
000010FF  0F8E47FEFFFF      jng near 0xf4c
00001105  0FB705641C0200    movzx eax,word [dword 0x21c64]
0000110C  6685C0            test ax,ax
0000110F  0F84C2FDFFFF      jz near 0xed7
00001115  0FB745F2          movzx eax,word [ebp-0xe]
00001119  83EC0C            sub esp,byte +0xc
0000111C  50                push eax
0000111D  E85DFBFFFF        call 0xc7f
00001122  83C410            add esp,byte +0x10
00001125  668945F2          mov [ebp-0xe],ax
00001129  66817DF2F70F      cmp word [ebp-0xe],0xff7
0000112F  770B              ja 0x113c
00001131  E9A1FDFFFF        jmp 0xed7
00001136  90                nop
00001137  EB04              jmp short 0x113d
00001139  90                nop
0000113A  EB01              jmp short 0x113d
0000113C  90                nop
0000113D  837DF400          cmp dword [ebp-0xc],byte +0x0
00001141  7513              jnz 0x1156
00001143  83EC0C            sub esp,byte +0xc
00001146  68DCB60100        push dword 0x1b6dc
0000114B  E8DA3E0000        call 0x502a
00001150  83C410            add esp,byte +0x10
00001153  EB01              jmp short 0x1156
00001155  90                nop
00001156  C9                leave
00001157  C3                ret
00001158  55                push ebp
00001159  89E5              mov ebp,esp
0000115B  83EC48            sub esp,byte +0x48
0000115E  A1601C0200        mov eax,[0x21c60]
00001163  85C0              test eax,eax
00001165  7505              jnz 0x116c
00001167  E8F9FBFFFF        call 0xd65
0000116C  A1601C0200        mov eax,[0x21c60]
00001171  85C0              test eax,eax
00001173  750A              jnz 0x117f
00001175  B800000000        mov eax,0x0
0000117A  E939030000        jmp 0x14b8
0000117F  C745F400000000    mov dword [ebp-0xc],0x0
00001186  0FB705641C0200    movzx eax,word [dword 0x21c64]
0000118D  668945F2          mov [ebp-0xe],ax
00001191  C745EC00000000    mov dword [ebp-0x14],0x0
00001198  0FB705641C0200    movzx eax,word [dword 0x21c64]
0000119F  6685C0            test ax,ax
000011A2  7532              jnz 0x11d6
000011A4  8B55EC            mov edx,[ebp-0x14]
000011A7  A160350200        mov eax,[0x23560]
000011AC  39C2              cmp edx,eax
000011AE  0F83FD020000      jnc near 0x14b1
000011B4  8B156C350200      mov edx,[dword 0x2356c]
000011BA  8B45EC            mov eax,[ebp-0x14]
000011BD  01D0              add eax,edx
000011BF  83EC08            sub esp,byte +0x8
000011C2  68801C0200        push dword 0x21c80
000011C7  50                push eax
000011C8  E8304C0000        call 0x5dfd
000011CD  83C410            add esp,byte +0x10
000011D0  8345EC01          add dword [ebp-0x14],byte +0x1
000011D4  EB24              jmp short 0x11fa
000011D6  0FB755F2          movzx edx,word [ebp-0xe]
000011DA  A164350200        mov eax,[0x23564]
000011DF  01D0              add eax,edx
000011E1  83E802            sub eax,byte +0x2
000011E4  8945D4            mov [ebp-0x2c],eax
000011E7  83EC08            sub esp,byte +0x8
000011EA  68801C0200        push dword 0x21c80
000011EF  FF75D4            push dword [ebp-0x2c]
000011F2  E8064C0000        call 0x5dfd
000011F7  83C410            add esp,byte +0x10
000011FA  C745D0801C0200    mov dword [ebp-0x30],0x21c80
00001201  C745E800000000    mov dword [ebp-0x18],0x0
00001208  E969020000        jmp 0x1476
0000120D  8B45E8            mov eax,[ebp-0x18]
00001210  C1E005            shl eax,byte 0x5
00001213  89C2              mov edx,eax
00001215  8B45D0            mov eax,[ebp-0x30]
00001218  01D0              add eax,edx
0000121A  0FB600            movzx eax,byte [eax]
0000121D  84C0              test al,al
0000121F  7508              jnz 0x1229
00001221  8B45F4            mov eax,[ebp-0xc]
00001224  E98F020000        jmp 0x14b8
00001229  8B45E8            mov eax,[ebp-0x18]
0000122C  C1E005            shl eax,byte 0x5
0000122F  89C2              mov edx,eax
00001231  8B45D0            mov eax,[ebp-0x30]
00001234  01D0              add eax,edx
00001236  0FB600            movzx eax,byte [eax]
00001239  3CE5              cmp al,0xe5
0000123B  0F8427020000      jz near 0x1468
00001241  8B45E8            mov eax,[ebp-0x18]
00001244  C1E005            shl eax,byte 0x5
00001247  89C2              mov edx,eax
00001249  8B45D0            mov eax,[ebp-0x30]
0000124C  01D0              add eax,edx
0000124E  0FB6400B          movzx eax,byte [eax+0xb]
00001252  3C0F              cmp al,0xf
00001254  0F8411020000      jz near 0x146b
0000125A  8B45E8            mov eax,[ebp-0x18]
0000125D  C1E005            shl eax,byte 0x5
00001260  89C2              mov edx,eax
00001262  8B45D0            mov eax,[ebp-0x30]
00001265  01D0              add eax,edx
00001267  0FB6400B          movzx eax,byte [eax+0xb]
0000126B  0FB6C0            movzx eax,al
0000126E  83E008            and eax,byte +0x8
00001271  85C0              test eax,eax
00001273  0F85F5010000      jnz near 0x146e
00001279  8B45F4            mov eax,[ebp-0xc]
0000127C  3B450C            cmp eax,[ebp+0xc]
0000127F  7C08              jl 0x1289
00001281  8B45F4            mov eax,[ebp-0xc]
00001284  E92F020000        jmp 0x14b8
00001289  C745E400000000    mov dword [ebp-0x1c],0x0
00001290  C745E000000000    mov dword [ebp-0x20],0x0
00001297  EB5A              jmp short 0x12f3
00001299  8B45E8            mov eax,[ebp-0x18]
0000129C  C1E005            shl eax,byte 0x5
0000129F  89C2              mov edx,eax
000012A1  8B45D0            mov eax,[ebp-0x30]
000012A4  01C2              add edx,eax
000012A6  8B45E0            mov eax,[ebp-0x20]
000012A9  01D0              add eax,edx
000012AB  0FB600            movzx eax,byte [eax]
000012AE  8845CE            mov [ebp-0x32],al
000012B1  807DCE20          cmp byte [ebp-0x32],0x20
000012B5  7437              jz 0x12ee
000012B7  807DCE40          cmp byte [ebp-0x32],0x40
000012BB  7E06              jng 0x12c3
000012BD  807DCE5A          cmp byte [ebp-0x32],0x5a
000012C1  7E18              jng 0x12db
000012C3  807DCE2F          cmp byte [ebp-0x32],0x2f
000012C7  7E06              jng 0x12cf
000012C9  807DCE39          cmp byte [ebp-0x32],0x39
000012CD  7E0C              jng 0x12db
000012CF  807DCE5F          cmp byte [ebp-0x32],0x5f
000012D3  7406              jz 0x12db
000012D5  807DCE2D          cmp byte [ebp-0x32],0x2d
000012D9  7514              jnz 0x12ef
000012DB  8B45E4            mov eax,[ebp-0x1c]
000012DE  8D5001            lea edx,[eax+0x1]
000012E1  8955E4            mov [ebp-0x1c],edx
000012E4  0FB655CE          movzx edx,byte [ebp-0x32]
000012E8  885405C1          mov [ebp+eax-0x3f],dl
000012EC  EB01              jmp short 0x12ef
000012EE  90                nop
000012EF  8345E001          add dword [ebp-0x20],byte +0x1
000012F3  837DE007          cmp dword [ebp-0x20],byte +0x7
000012F7  7EA0              jng 0x1299
000012F9  837DE400          cmp dword [ebp-0x1c],byte +0x0
000012FD  7518              jnz 0x1317
000012FF  8B45E8            mov eax,[ebp-0x18]
00001302  C1E005            shl eax,byte 0x5
00001305  89C2              mov edx,eax
00001307  8B45D0            mov eax,[ebp-0x30]
0000130A  01D0              add eax,edx
0000130C  0FB600            movzx eax,byte [eax]
0000130F  3C2E              cmp al,0x2e
00001311  0F855A010000      jnz near 0x1471
00001317  8B45E8            mov eax,[ebp-0x18]
0000131A  C1E005            shl eax,byte 0x5
0000131D  89C2              mov edx,eax
0000131F  8B45D0            mov eax,[ebp-0x30]
00001322  01D0              add eax,edx
00001324  0FB64008          movzx eax,byte [eax+0x8]
00001328  3C20              cmp al,0x20
0000132A  746E              jz 0x139a
0000132C  8B45E4            mov eax,[ebp-0x1c]
0000132F  8D5001            lea edx,[eax+0x1]
00001332  8955E4            mov [ebp-0x1c],edx
00001335  C64405C12E        mov byte [ebp+eax-0x3f],0x2e
0000133A  C745DC00000000    mov dword [ebp-0x24],0x0
00001341  EB4E              jmp short 0x1391
00001343  8B45E8            mov eax,[ebp-0x18]
00001346  C1E005            shl eax,byte 0x5
00001349  89C2              mov edx,eax
0000134B  8B45D0            mov eax,[ebp-0x30]
0000134E  01C2              add edx,eax
00001350  8B45DC            mov eax,[ebp-0x24]
00001353  01D0              add eax,edx
00001355  83C008            add eax,byte +0x8
00001358  0FB600            movzx eax,byte [eax]
0000135B  8845CF            mov [ebp-0x31],al
0000135E  807DCF20          cmp byte [ebp-0x31],0x20
00001362  7435              jz 0x1399
00001364  807DCF40          cmp byte [ebp-0x31],0x40
00001368  7E06              jng 0x1370
0000136A  807DCF5A          cmp byte [ebp-0x31],0x5a
0000136E  7E0C              jng 0x137c
00001370  807DCF2F          cmp byte [ebp-0x31],0x2f
00001374  7E17              jng 0x138d
00001376  807DCF39          cmp byte [ebp-0x31],0x39
0000137A  7F11              jg 0x138d
0000137C  8B45E4            mov eax,[ebp-0x1c]
0000137F  8D5001            lea edx,[eax+0x1]
00001382  8955E4            mov [ebp-0x1c],edx
00001385  0FB655CF          movzx edx,byte [ebp-0x31]
00001389  885405C1          mov [ebp+eax-0x3f],dl
0000138D  8345DC01          add dword [ebp-0x24],byte +0x1
00001391  837DDC02          cmp dword [ebp-0x24],byte +0x2
00001395  7EAC              jng 0x1343
00001397  EB01              jmp short 0x139a
00001399  90                nop
0000139A  8D55C1            lea edx,[ebp-0x3f]
0000139D  8B45E4            mov eax,[ebp-0x1c]
000013A0  01D0              add eax,edx
000013A2  C60000            mov byte [eax],0x0
000013A5  C745D800000000    mov dword [ebp-0x28],0x0
000013AC  EB2B              jmp short 0x13d9
000013AE  8B55F4            mov edx,[ebp-0xc]
000013B1  89D0              mov eax,edx
000013B3  C1E002            shl eax,byte 0x2
000013B6  01D0              add eax,edx
000013B8  C1E002            shl eax,byte 0x2
000013BB  89C2              mov edx,eax
000013BD  8B4508            mov eax,[ebp+0x8]
000013C0  8D0C02            lea ecx,[edx+eax]
000013C3  8D55C1            lea edx,[ebp-0x3f]
000013C6  8B45D8            mov eax,[ebp-0x28]
000013C9  01D0              add eax,edx
000013CB  0FB600            movzx eax,byte [eax]
000013CE  8B55D8            mov edx,[ebp-0x28]
000013D1  01CA              add edx,ecx
000013D3  8802              mov [edx],al
000013D5  8345D801          add dword [ebp-0x28],byte +0x1
000013D9  8D55C1            lea edx,[ebp-0x3f]
000013DC  8B45D8            mov eax,[ebp-0x28]
000013DF  01D0              add eax,edx
000013E1  0FB600            movzx eax,byte [eax]
000013E4  84C0              test al,al
000013E6  75C6              jnz 0x13ae
000013E8  8B55F4            mov edx,[ebp-0xc]
000013EB  89D0              mov eax,edx
000013ED  C1E002            shl eax,byte 0x2
000013F0  01D0              add eax,edx
000013F2  C1E002            shl eax,byte 0x2
000013F5  89C2              mov edx,eax
000013F7  8B4508            mov eax,[ebp+0x8]
000013FA  01C2              add edx,eax
000013FC  8B45D8            mov eax,[ebp-0x28]
000013FF  01D0              add eax,edx
00001401  C60000            mov byte [eax],0x0
00001404  8B45E8            mov eax,[ebp-0x18]
00001407  C1E005            shl eax,byte 0x5
0000140A  89C2              mov edx,eax
0000140C  8B45D0            mov eax,[ebp-0x30]
0000140F  01D0              add eax,edx
00001411  8B481C            mov ecx,[eax+0x1c]
00001414  8B55F4            mov edx,[ebp-0xc]
00001417  89D0              mov eax,edx
00001419  C1E002            shl eax,byte 0x2
0000141C  01D0              add eax,edx
0000141E  C1E002            shl eax,byte 0x2
00001421  89C2              mov edx,eax
00001423  8B4508            mov eax,[ebp+0x8]
00001426  01D0              add eax,edx
00001428  89CA              mov edx,ecx
0000142A  89500C            mov [eax+0xc],edx
0000142D  8B45E8            mov eax,[ebp-0x18]
00001430  C1E005            shl eax,byte 0x5
00001433  89C2              mov edx,eax
00001435  8B45D0            mov eax,[ebp-0x30]
00001438  01D0              add eax,edx
0000143A  0FB6400B          movzx eax,byte [eax+0xb]
0000143E  0FB6C0            movzx eax,al
00001441  C1F804            sar eax,byte 0x4
00001444  89C1              mov ecx,eax
00001446  8B55F4            mov edx,[ebp-0xc]
00001449  89D0              mov eax,edx
0000144B  C1E002            shl eax,byte 0x2
0000144E  01D0              add eax,edx
00001450  C1E002            shl eax,byte 0x2
00001453  89C2              mov edx,eax
00001455  8B4508            mov eax,[ebp+0x8]
00001458  01D0              add eax,edx
0000145A  83E101            and ecx,byte +0x1
0000145D  89CA              mov edx,ecx
0000145F  895010            mov [eax+0x10],edx
00001462  8345F401          add dword [ebp-0xc],byte +0x1
00001466  EB0A              jmp short 0x1472
00001468  90                nop
00001469  EB07              jmp short 0x1472
0000146B  90                nop
0000146C  EB04              jmp short 0x1472
0000146E  90                nop
0000146F  EB01              jmp short 0x1472
00001471  90                nop
00001472  8345E801          add dword [ebp-0x18],byte +0x1
00001476  837DE80F          cmp dword [ebp-0x18],byte +0xf
0000147A  0F8E8DFDFFFF      jng near 0x120d
00001480  0FB705641C0200    movzx eax,word [dword 0x21c64]
00001487  6685C0            test ax,ax
0000148A  0F8408FDFFFF      jz near 0x1198
00001490  0FB745F2          movzx eax,word [ebp-0xe]
00001494  83EC0C            sub esp,byte +0xc
00001497  50                push eax
00001498  E8E2F7FFFF        call 0xc7f
0000149D  83C410            add esp,byte +0x10
000014A0  668945F2          mov [ebp-0xe],ax
000014A4  66817DF2F70F      cmp word [ebp-0xe],0xff7
000014AA  7708              ja 0x14b4
000014AC  E9E7FCFFFF        jmp 0x1198
000014B1  90                nop
000014B2  EB01              jmp short 0x14b5
000014B4  90                nop
000014B5  8B45F4            mov eax,[ebp-0xc]
000014B8  C9                leave
000014B9  C3                ret
000014BA  55                push ebp
000014BB  89E5              mov ebp,esp
000014BD  83EC58            sub esp,byte +0x58
000014C0  A1601C0200        mov eax,[0x21c60]
000014C5  85C0              test eax,eax
000014C7  7505              jnz 0x14ce
000014C9  E897F8FFFF        call 0xd65
000014CE  A1601C0200        mov eax,[0x21c60]
000014D3  85C0              test eax,eax
000014D5  750A              jnz 0x14e1
000014D7  B800000000        mov eax,0x0
000014DC  E9DA020000        jmp 0x17bb
000014E1  0FB705641C0200    movzx eax,word [dword 0x21c64]
000014E8  668945F6          mov [ebp-0xa],ax
000014EC  C745F000000000    mov dword [ebp-0x10],0x0
000014F3  0FB705641C0200    movzx eax,word [dword 0x21c64]
000014FA  6685C0            test ax,ax
000014FD  7532              jnz 0x1531
000014FF  8B55F0            mov edx,[ebp-0x10]
00001502  A160350200        mov eax,[0x23560]
00001507  39C2              cmp edx,eax
00001509  0F83A3020000      jnc near 0x17b2
0000150F  8B156C350200      mov edx,[dword 0x2356c]
00001515  8B45F0            mov eax,[ebp-0x10]
00001518  01D0              add eax,edx
0000151A  83EC08            sub esp,byte +0x8
0000151D  68801C0200        push dword 0x21c80
00001522  50                push eax
00001523  E8D5480000        call 0x5dfd
00001528  83C410            add esp,byte +0x10
0000152B  8345F001          add dword [ebp-0x10],byte +0x1
0000152F  EB24              jmp short 0x1555
00001531  0FB755F6          movzx edx,word [ebp-0xa]
00001535  A164350200        mov eax,[0x23564]
0000153A  01D0              add eax,edx
0000153C  83E802            sub eax,byte +0x2
0000153F  8945CC            mov [ebp-0x34],eax
00001542  83EC08            sub esp,byte +0x8
00001545  68801C0200        push dword 0x21c80
0000154A  FF75CC            push dword [ebp-0x34]
0000154D  E8AB480000        call 0x5dfd
00001552  83C410            add esp,byte +0x10
00001555  C745C8801C0200    mov dword [ebp-0x38],0x21c80
0000155C  C745EC00000000    mov dword [ebp-0x14],0x0
00001563  E90F020000        jmp 0x1777
00001568  8B45EC            mov eax,[ebp-0x14]
0000156B  C1E005            shl eax,byte 0x5
0000156E  89C2              mov edx,eax
00001570  8B45C8            mov eax,[ebp-0x38]
00001573  01D0              add eax,edx
00001575  0FB600            movzx eax,byte [eax]
00001578  84C0              test al,al
0000157A  750A              jnz 0x1586
0000157C  B800000000        mov eax,0x0
00001581  E935020000        jmp 0x17bb
00001586  8B45EC            mov eax,[ebp-0x14]
00001589  C1E005            shl eax,byte 0x5
0000158C  89C2              mov edx,eax
0000158E  8B45C8            mov eax,[ebp-0x38]
00001591  01D0              add eax,edx
00001593  0FB600            movzx eax,byte [eax]
00001596  3CE5              cmp al,0xe5
00001598  0F84D1010000      jz near 0x176f
0000159E  8B45EC            mov eax,[ebp-0x14]
000015A1  C1E005            shl eax,byte 0x5
000015A4  89C2              mov edx,eax
000015A6  8B45C8            mov eax,[ebp-0x38]
000015A9  01D0              add eax,edx
000015AB  0FB6400B          movzx eax,byte [eax+0xb]
000015AF  3C0F              cmp al,0xf
000015B1  0F84BB010000      jz near 0x1772
000015B7  C745E800000000    mov dword [ebp-0x18],0x0
000015BE  C745E400000000    mov dword [ebp-0x1c],0x0
000015C5  EB40              jmp short 0x1607
000015C7  8B45EC            mov eax,[ebp-0x14]
000015CA  C1E005            shl eax,byte 0x5
000015CD  89C2              mov edx,eax
000015CF  8B45C8            mov eax,[ebp-0x38]
000015D2  01C2              add edx,eax
000015D4  8B45E4            mov eax,[ebp-0x1c]
000015D7  01D0              add eax,edx
000015D9  0FB600            movzx eax,byte [eax]
000015DC  3C20              cmp al,0x20
000015DE  742F              jz 0x160f
000015E0  8B45EC            mov eax,[ebp-0x14]
000015E3  C1E005            shl eax,byte 0x5
000015E6  89C2              mov edx,eax
000015E8  8B45C8            mov eax,[ebp-0x38]
000015EB  8D0C02            lea ecx,[edx+eax]
000015EE  8B45E8            mov eax,[ebp-0x18]
000015F1  8D5001            lea edx,[eax+0x1]
000015F4  8955E8            mov [ebp-0x18],edx
000015F7  8B55E4            mov edx,[ebp-0x1c]
000015FA  01CA              add edx,ecx
000015FC  0FB612            movzx edx,byte [edx]
000015FF  885405B3          mov [ebp+eax-0x4d],dl
00001603  8345E401          add dword [ebp-0x1c],byte +0x1
00001607  837DE407          cmp dword [ebp-0x1c],byte +0x7
0000160B  7EBA              jng 0x15c7
0000160D  EB01              jmp short 0x1610
0000160F  90                nop
00001610  8B45EC            mov eax,[ebp-0x14]
00001613  C1E005            shl eax,byte 0x5
00001616  89C2              mov edx,eax
00001618  8B45C8            mov eax,[ebp-0x38]
0000161B  01D0              add eax,edx
0000161D  0FB64008          movzx eax,byte [eax+0x8]
00001621  3C20              cmp al,0x20
00001623  7466              jz 0x168b
00001625  8B45E8            mov eax,[ebp-0x18]
00001628  8D5001            lea edx,[eax+0x1]
0000162B  8955E8            mov [ebp-0x18],edx
0000162E  C64405B32E        mov byte [ebp+eax-0x4d],0x2e
00001633  C745E000000000    mov dword [ebp-0x20],0x0
0000163A  EB46              jmp short 0x1682
0000163C  8B45EC            mov eax,[ebp-0x14]
0000163F  C1E005            shl eax,byte 0x5
00001642  89C2              mov edx,eax
00001644  8B45C8            mov eax,[ebp-0x38]
00001647  01C2              add edx,eax
00001649  8B45E0            mov eax,[ebp-0x20]
0000164C  01D0              add eax,edx
0000164E  83C008            add eax,byte +0x8
00001651  0FB600            movzx eax,byte [eax]
00001654  3C20              cmp al,0x20
00001656  7432              jz 0x168a
00001658  8B45EC            mov eax,[ebp-0x14]
0000165B  C1E005            shl eax,byte 0x5
0000165E  89C2              mov edx,eax
00001660  8B45C8            mov eax,[ebp-0x38]
00001663  8D0C02            lea ecx,[edx+eax]
00001666  8B45E8            mov eax,[ebp-0x18]
00001669  8D5001            lea edx,[eax+0x1]
0000166C  8955E8            mov [ebp-0x18],edx
0000166F  8B55E0            mov edx,[ebp-0x20]
00001672  01CA              add edx,ecx
00001674  83C208            add edx,byte +0x8
00001677  0FB612            movzx edx,byte [edx]
0000167A  885405B3          mov [ebp+eax-0x4d],dl
0000167E  8345E001          add dword [ebp-0x20],byte +0x1
00001682  837DE002          cmp dword [ebp-0x20],byte +0x2
00001686  7EB4              jng 0x163c
00001688  EB01              jmp short 0x168b
0000168A  90                nop
0000168B  8D55B3            lea edx,[ebp-0x4d]
0000168E  8B45E8            mov eax,[ebp-0x18]
00001691  01D0              add eax,edx
00001693  C60000            mov byte [eax],0x0
00001696  C745DC01000000    mov dword [ebp-0x24],0x1
0000169D  8B4508            mov eax,[ebp+0x8]
000016A0  8945D8            mov [ebp-0x28],eax
000016A3  8D45B3            lea eax,[ebp-0x4d]
000016A6  8945D4            mov [ebp-0x2c],eax
000016A9  EB58              jmp short 0x1703
000016AB  8B45D8            mov eax,[ebp-0x28]
000016AE  0FB600            movzx eax,byte [eax]
000016B1  8845D3            mov [ebp-0x2d],al
000016B4  8B45D4            mov eax,[ebp-0x2c]
000016B7  0FB600            movzx eax,byte [eax]
000016BA  8845D2            mov [ebp-0x2e],al
000016BD  807DD360          cmp byte [ebp-0x2d],0x60
000016C1  7E10              jng 0x16d3
000016C3  807DD37A          cmp byte [ebp-0x2d],0x7a
000016C7  7F0A              jg 0x16d3
000016C9  0FB645D3          movzx eax,byte [ebp-0x2d]
000016CD  83E820            sub eax,byte +0x20
000016D0  8845D3            mov [ebp-0x2d],al
000016D3  807DD260          cmp byte [ebp-0x2e],0x60
000016D7  7E10              jng 0x16e9
000016D9  807DD27A          cmp byte [ebp-0x2e],0x7a
000016DD  7F0A              jg 0x16e9
000016DF  0FB645D2          movzx eax,byte [ebp-0x2e]
000016E3  83E820            sub eax,byte +0x20
000016E6  8845D2            mov [ebp-0x2e],al
000016E9  0FB645D3          movzx eax,byte [ebp-0x2d]
000016ED  3A45D2            cmp al,[ebp-0x2e]
000016F0  7409              jz 0x16fb
000016F2  C745DC00000000    mov dword [ebp-0x24],0x0
000016F9  EB1C              jmp short 0x1717
000016FB  8345D801          add dword [ebp-0x28],byte +0x1
000016FF  8345D401          add dword [ebp-0x2c],byte +0x1
00001703  8B45D8            mov eax,[ebp-0x28]
00001706  0FB600            movzx eax,byte [eax]
00001709  84C0              test al,al
0000170B  740A              jz 0x1717
0000170D  8B45D4            mov eax,[ebp-0x2c]
00001710  0FB600            movzx eax,byte [eax]
00001713  84C0              test al,al
00001715  7594              jnz 0x16ab
00001717  837DDC00          cmp dword [ebp-0x24],byte +0x0
0000171B  7456              jz 0x1773
0000171D  8B45D8            mov eax,[ebp-0x28]
00001720  0FB600            movzx eax,byte [eax]
00001723  84C0              test al,al
00001725  754C              jnz 0x1773
00001727  8B45D4            mov eax,[ebp-0x2c]
0000172A  0FB600            movzx eax,byte [eax]
0000172D  84C0              test al,al
0000172F  7542              jnz 0x1773
00001731  8B45EC            mov eax,[ebp-0x14]
00001734  C1E005            shl eax,byte 0x5
00001737  89C2              mov edx,eax
00001739  8B45C8            mov eax,[ebp-0x38]
0000173C  01D0              add eax,edx
0000173E  0FB7401A          movzx eax,word [eax+0x1a]
00001742  668945C6          mov [ebp-0x3a],ax
00001746  0FB755C6          movzx edx,word [ebp-0x3a]
0000174A  A164350200        mov eax,[0x23564]
0000174F  01D0              add eax,edx
00001751  83E802            sub eax,byte +0x2
00001754  8945C0            mov [ebp-0x40],eax
00001757  83EC08            sub esp,byte +0x8
0000175A  FF750C            push dword [ebp+0xc]
0000175D  FF75C0            push dword [ebp-0x40]
00001760  E898460000        call 0x5dfd
00001765  83C410            add esp,byte +0x10
00001768  B801000000        mov eax,0x1
0000176D  EB4C              jmp short 0x17bb
0000176F  90                nop
00001770  EB01              jmp short 0x1773
00001772  90                nop
00001773  8345EC01          add dword [ebp-0x14],byte +0x1
00001777  837DEC0F          cmp dword [ebp-0x14],byte +0xf
0000177B  0F8EE7FDFFFF      jng near 0x1568
00001781  0FB705641C0200    movzx eax,word [dword 0x21c64]
00001788  6685C0            test ax,ax
0000178B  0F8462FDFFFF      jz near 0x14f3
00001791  0FB745F6          movzx eax,word [ebp-0xa]
00001795  83EC0C            sub esp,byte +0xc
00001798  50                push eax
00001799  E8E1F4FFFF        call 0xc7f
0000179E  83C410            add esp,byte +0x10
000017A1  668945F6          mov [ebp-0xa],ax
000017A5  66817DF6F70F      cmp word [ebp-0xa],0xff7
000017AB  7708              ja 0x17b5
000017AD  E941FDFFFF        jmp 0x14f3
000017B2  90                nop
000017B3  EB01              jmp short 0x17b6
000017B5  90                nop
000017B6  B800000000        mov eax,0x0
000017BB  C9                leave
000017BC  C3                ret
000017BD  55                push ebp
000017BE  89E5              mov ebp,esp
000017C0  83EC28            sub esp,byte +0x28
000017C3  8B5508            mov edx,[ebp+0x8]
000017C6  8B450C            mov eax,[ebp+0xc]
000017C9  668955E4          mov [ebp-0x1c],dx
000017CD  668945E0          mov [ebp-0x20],ax
000017D1  0FB755E4          movzx edx,word [ebp-0x1c]
000017D5  0FB745E4          movzx eax,word [ebp-0x1c]
000017D9  66D1E8            shr ax,1
000017DC  0FB7C0            movzx eax,ax
000017DF  01D0              add eax,edx
000017E1  8945F4            mov [ebp-0xc],eax
000017E4  8B45F4            mov eax,[ebp-0xc]
000017E7  C1E809            shr eax,byte 0x9
000017EA  89C2              mov edx,eax
000017EC  A168350200        mov eax,[0x23568]
000017F1  01D0              add eax,edx
000017F3  8945F0            mov [ebp-0x10],eax
000017F6  8B45F4            mov eax,[ebp-0xc]
000017F9  25FF010000        and eax,0x1ff
000017FE  8945EC            mov [ebp-0x14],eax
00001801  83EC08            sub esp,byte +0x8
00001804  68801C0200        push dword 0x21c80
00001809  FF75F0            push dword [ebp-0x10]
0000180C  E8EC450000        call 0x5dfd
00001811  83C410            add esp,byte +0x10
00001814  0FB745E4          movzx eax,word [ebp-0x1c]
00001818  83E001            and eax,byte +0x1
0000181B  85C0              test eax,eax
0000181D  744A              jz 0x1869
0000181F  8B45EC            mov eax,[ebp-0x14]
00001822  05801C0200        add eax,0x21c80
00001827  0FB600            movzx eax,byte [eax]
0000182A  83E00F            and eax,byte +0xf
0000182D  89C2              mov edx,eax
0000182F  0FB745E0          movzx eax,word [ebp-0x20]
00001833  C1E004            shl eax,byte 0x4
00001836  09D0              or eax,edx
00001838  89C2              mov edx,eax
0000183A  8B45EC            mov eax,[ebp-0x14]
0000183D  05801C0200        add eax,0x21c80
00001842  8810              mov [eax],dl
00001844  8B45EC            mov eax,[ebp-0x14]
00001847  83C001            add eax,byte +0x1
0000184A  3DFF010000        cmp eax,0x1ff
0000184F  7760              ja 0x18b1
00001851  0FB745E0          movzx eax,word [ebp-0x20]
00001855  66C1E804          shr ax,byte 0x4
00001859  89C2              mov edx,eax
0000185B  8B45EC            mov eax,[ebp-0x14]
0000185E  83C001            add eax,byte +0x1
00001861  8890801C0200      mov [eax+0x21c80],dl
00001867  EB48              jmp short 0x18b1
00001869  0FB745E0          movzx eax,word [ebp-0x20]
0000186D  89C2              mov edx,eax
0000186F  8B45EC            mov eax,[ebp-0x14]
00001872  05801C0200        add eax,0x21c80
00001877  8810              mov [eax],dl
00001879  8B45EC            mov eax,[ebp-0x14]
0000187C  83C001            add eax,byte +0x1
0000187F  3DFF010000        cmp eax,0x1ff
00001884  772B              ja 0x18b1
00001886  8B45EC            mov eax,[ebp-0x14]
00001889  83C001            add eax,byte +0x1
0000188C  0FB680801C0200    movzx eax,byte [eax+0x21c80]
00001893  83E0F0            and eax,byte -0x10
00001896  89C2              mov edx,eax
00001898  0FB745E0          movzx eax,word [ebp-0x20]
0000189C  66C1E808          shr ax,byte 0x8
000018A0  83E00F            and eax,byte +0xf
000018A3  09C2              or edx,eax
000018A5  8B45EC            mov eax,[ebp-0x14]
000018A8  83C001            add eax,byte +0x1
000018AB  8890801C0200      mov [eax+0x21c80],dl
000018B1  83EC08            sub esp,byte +0x8
000018B4  68801C0200        push dword 0x21c80
000018B9  FF75F0            push dword [ebp-0x10]
000018BC  E8E7450000        call 0x5ea8
000018C1  83C410            add esp,byte +0x10
000018C4  90                nop
000018C5  C9                leave
000018C6  C3                ret
000018C7  55                push ebp
000018C8  89E5              mov ebp,esp
000018CA  81EC98000000      sub esp,0x98
000018D0  A1601C0200        mov eax,[0x21c60]
000018D5  85C0              test eax,eax
000018D7  7505              jnz 0x18de
000018D9  E887F4FFFF        call 0xd65
000018DE  A1601C0200        mov eax,[0x21c60]
000018E3  85C0              test eax,eax
000018E5  750A              jnz 0x18f1
000018E7  B800000000        mov eax,0x0
000018EC  E9FF070000        jmp 0x20f0
000018F1  0FB705641C0200    movzx eax,word [dword 0x21c64]
000018F8  668945F6          mov [ebp-0xa],ax
000018FC  C745F000000000    mov dword [ebp-0x10],0x0
00001903  0FB705641C0200    movzx eax,word [dword 0x21c64]
0000190A  6685C0            test ax,ax
0000190D  7520              jnz 0x192f
0000190F  8B55F0            mov edx,[ebp-0x10]
00001912  A160350200        mov eax,[0x23560]
00001917  39C2              cmp edx,eax
00001919  0F8362030000      jnc near 0x1c81
0000191F  8B156C350200      mov edx,[dword 0x2356c]
00001925  8B45F0            mov eax,[ebp-0x10]
00001928  01D0              add eax,edx
0000192A  8945EC            mov [ebp-0x14],eax
0000192D  EB11              jmp short 0x1940
0000192F  0FB755F6          movzx edx,word [ebp-0xa]
00001933  A164350200        mov eax,[0x23564]
00001938  01D0              add eax,edx
0000193A  83E802            sub eax,byte +0x2
0000193D  8945EC            mov [ebp-0x14],eax
00001940  83EC08            sub esp,byte +0x8
00001943  68801C0200        push dword 0x21c80
00001948  FF75EC            push dword [ebp-0x14]
0000194B  E8AD440000        call 0x5dfd
00001950  83C410            add esp,byte +0x10
00001953  C74598801C0200    mov dword [ebp-0x68],0x21c80
0000195A  C745E800000000    mov dword [ebp-0x18],0x0
00001961  E9D8020000        jmp 0x1c3e
00001966  8B45E8            mov eax,[ebp-0x18]
00001969  C1E005            shl eax,byte 0x5
0000196C  89C2              mov edx,eax
0000196E  8B4598            mov eax,[ebp-0x68]
00001971  01D0              add eax,edx
00001973  0FB600            movzx eax,byte [eax]
00001976  84C0              test al,al
00001978  0F84CC020000      jz near 0x1c4a
0000197E  8B45E8            mov eax,[ebp-0x18]
00001981  C1E005            shl eax,byte 0x5
00001984  89C2              mov edx,eax
00001986  8B4598            mov eax,[ebp-0x68]
00001989  01D0              add eax,edx
0000198B  0FB600            movzx eax,byte [eax]
0000198E  3CE5              cmp al,0xe5
00001990  0F849D020000      jz near 0x1c33
00001996  8B45E8            mov eax,[ebp-0x18]
00001999  C1E005            shl eax,byte 0x5
0000199C  89C2              mov edx,eax
0000199E  8B4598            mov eax,[ebp-0x68]
000019A1  01D0              add eax,edx
000019A3  0FB6400B          movzx eax,byte [eax+0xb]
000019A7  3C0F              cmp al,0xf
000019A9  0F8487020000      jz near 0x1c36
000019AF  8B45E8            mov eax,[ebp-0x18]
000019B2  C1E005            shl eax,byte 0x5
000019B5  89C2              mov edx,eax
000019B7  8B4598            mov eax,[ebp-0x68]
000019BA  01D0              add eax,edx
000019BC  0FB6400B          movzx eax,byte [eax+0xb]
000019C0  0FB6C0            movzx eax,al
000019C3  83E010            and eax,byte +0x10
000019C6  85C0              test eax,eax
000019C8  0F856B020000      jnz near 0x1c39
000019CE  C745E400000000    mov dword [ebp-0x1c],0x0
000019D5  C745E000000000    mov dword [ebp-0x20],0x0
000019DC  EB43              jmp short 0x1a21
000019DE  8B45E8            mov eax,[ebp-0x18]
000019E1  C1E005            shl eax,byte 0x5
000019E4  89C2              mov edx,eax
000019E6  8B4598            mov eax,[ebp-0x68]
000019E9  01C2              add edx,eax
000019EB  8B45E0            mov eax,[ebp-0x20]
000019EE  01D0              add eax,edx
000019F0  0FB600            movzx eax,byte [eax]
000019F3  3C20              cmp al,0x20
000019F5  7432              jz 0x1a29
000019F7  8B45E8            mov eax,[ebp-0x18]
000019FA  C1E005            shl eax,byte 0x5
000019FD  89C2              mov edx,eax
000019FF  8B4598            mov eax,[ebp-0x68]
00001A02  8D0C02            lea ecx,[edx+eax]
00001A05  8B45E4            mov eax,[ebp-0x1c]
00001A08  8D5001            lea edx,[eax+0x1]
00001A0B  8955E4            mov [ebp-0x1c],edx
00001A0E  8B55E0            mov edx,[ebp-0x20]
00001A11  01CA              add edx,ecx
00001A13  0FB612            movzx edx,byte [edx]
00001A16  8894056BFFFFFF    mov [ebp+eax-0x95],dl
00001A1D  8345E001          add dword [ebp-0x20],byte +0x1
00001A21  837DE007          cmp dword [ebp-0x20],byte +0x7
00001A25  7EB7              jng 0x19de
00001A27  EB01              jmp short 0x1a2a
00001A29  90                nop
00001A2A  8B45E8            mov eax,[ebp-0x18]
00001A2D  C1E005            shl eax,byte 0x5
00001A30  89C2              mov edx,eax
00001A32  8B4598            mov eax,[ebp-0x68]
00001A35  01D0              add eax,edx
00001A37  0FB64008          movzx eax,byte [eax+0x8]
00001A3B  3C20              cmp al,0x20
00001A3D  746C              jz 0x1aab
00001A3F  8B45E4            mov eax,[ebp-0x1c]
00001A42  8D5001            lea edx,[eax+0x1]
00001A45  8955E4            mov [ebp-0x1c],edx
00001A48  C684056BFFFFFF2E  mov byte [ebp+eax-0x95],0x2e
00001A50  C745DC00000000    mov dword [ebp-0x24],0x0
00001A57  EB49              jmp short 0x1aa2
00001A59  8B45E8            mov eax,[ebp-0x18]
00001A5C  C1E005            shl eax,byte 0x5
00001A5F  89C2              mov edx,eax
00001A61  8B4598            mov eax,[ebp-0x68]
00001A64  01C2              add edx,eax
00001A66  8B45DC            mov eax,[ebp-0x24]
00001A69  01D0              add eax,edx
00001A6B  83C008            add eax,byte +0x8
00001A6E  0FB600            movzx eax,byte [eax]
00001A71  3C20              cmp al,0x20
00001A73  7435              jz 0x1aaa
00001A75  8B45E8            mov eax,[ebp-0x18]
00001A78  C1E005            shl eax,byte 0x5
00001A7B  89C2              mov edx,eax
00001A7D  8B4598            mov eax,[ebp-0x68]
00001A80  8D0C02            lea ecx,[edx+eax]
00001A83  8B45E4            mov eax,[ebp-0x1c]
00001A86  8D5001            lea edx,[eax+0x1]
00001A89  8955E4            mov [ebp-0x1c],edx
00001A8C  8B55DC            mov edx,[ebp-0x24]
00001A8F  01CA              add edx,ecx
00001A91  83C208            add edx,byte +0x8
00001A94  0FB612            movzx edx,byte [edx]
00001A97  8894056BFFFFFF    mov [ebp+eax-0x95],dl
00001A9E  8345DC01          add dword [ebp-0x24],byte +0x1
00001AA2  837DDC02          cmp dword [ebp-0x24],byte +0x2
00001AA6  7EB1              jng 0x1a59
00001AA8  EB01              jmp short 0x1aab
00001AAA  90                nop
00001AAB  8D956BFFFFFF      lea edx,[ebp-0x95]
00001AB1  8B45E4            mov eax,[ebp-0x1c]
00001AB4  01D0              add eax,edx
00001AB6  C60000            mov byte [eax],0x0
00001AB9  C745D801000000    mov dword [ebp-0x28],0x1
00001AC0  8B4508            mov eax,[ebp+0x8]
00001AC3  8945D4            mov [ebp-0x2c],eax
00001AC6  8D856BFFFFFF      lea eax,[ebp-0x95]
00001ACC  8945D0            mov [ebp-0x30],eax
00001ACF  EB6A              jmp short 0x1b3b
00001AD1  8B45D4            mov eax,[ebp-0x2c]
00001AD4  0FB600            movzx eax,byte [eax]
00001AD7  3C60              cmp al,0x60
00001AD9  7E15              jng 0x1af0
00001ADB  8B45D4            mov eax,[ebp-0x2c]
00001ADE  0FB600            movzx eax,byte [eax]
00001AE1  3C7A              cmp al,0x7a
00001AE3  7F0B              jg 0x1af0
00001AE5  8B45D4            mov eax,[ebp-0x2c]
00001AE8  0FB600            movzx eax,byte [eax]
00001AEB  83E820            sub eax,byte +0x20
00001AEE  EB06              jmp short 0x1af6
00001AF0  8B45D4            mov eax,[ebp-0x2c]
00001AF3  0FB600            movzx eax,byte [eax]
00001AF6  884597            mov [ebp-0x69],al
00001AF9  8B45D0            mov eax,[ebp-0x30]
00001AFC  0FB600            movzx eax,byte [eax]
00001AFF  3C60              cmp al,0x60
00001B01  7E15              jng 0x1b18
00001B03  8B45D0            mov eax,[ebp-0x30]
00001B06  0FB600            movzx eax,byte [eax]
00001B09  3C7A              cmp al,0x7a
00001B0B  7F0B              jg 0x1b18
00001B0D  8B45D0            mov eax,[ebp-0x30]
00001B10  0FB600            movzx eax,byte [eax]
00001B13  83E820            sub eax,byte +0x20
00001B16  EB06              jmp short 0x1b1e
00001B18  8B45D0            mov eax,[ebp-0x30]
00001B1B  0FB600            movzx eax,byte [eax]
00001B1E  884596            mov [ebp-0x6a],al
00001B21  0FB64597          movzx eax,byte [ebp-0x69]
00001B25  3A4596            cmp al,[ebp-0x6a]
00001B28  7409              jz 0x1b33
00001B2A  C745D800000000    mov dword [ebp-0x28],0x0
00001B31  EB1C              jmp short 0x1b4f
00001B33  8345D401          add dword [ebp-0x2c],byte +0x1
00001B37  8345D001          add dword [ebp-0x30],byte +0x1
00001B3B  8B45D4            mov eax,[ebp-0x2c]
00001B3E  0FB600            movzx eax,byte [eax]
00001B41  84C0              test al,al
00001B43  740A              jz 0x1b4f
00001B45  8B45D0            mov eax,[ebp-0x30]
00001B48  0FB600            movzx eax,byte [eax]
00001B4B  84C0              test al,al
00001B4D  7582              jnz 0x1ad1
00001B4F  837DD800          cmp dword [ebp-0x28],byte +0x0
00001B53  0F84E1000000      jz near 0x1c3a
00001B59  8B45D4            mov eax,[ebp-0x2c]
00001B5C  0FB600            movzx eax,byte [eax]
00001B5F  84C0              test al,al
00001B61  0F85D3000000      jnz near 0x1c3a
00001B67  8B45D0            mov eax,[ebp-0x30]
00001B6A  0FB600            movzx eax,byte [eax]
00001B6D  84C0              test al,al
00001B6F  0F85C5000000      jnz near 0x1c3a
00001B75  8B45E8            mov eax,[ebp-0x18]
00001B78  C1E005            shl eax,byte 0x5
00001B7B  89C2              mov edx,eax
00001B7D  8B4598            mov eax,[ebp-0x68]
00001B80  01D0              add eax,edx
00001B82  0FB7401A          movzx eax,word [eax+0x1a]
00001B86  66894594          mov [ebp-0x6c],ax
00001B8A  0FB75594          movzx edx,word [ebp-0x6c]
00001B8E  A164350200        mov eax,[0x23564]
00001B93  01D0              add eax,edx
00001B95  83E802            sub eax,byte +0x2
00001B98  894590            mov [ebp-0x70],eax
00001B9B  83EC04            sub esp,byte +0x4
00001B9E  6800020000        push dword 0x200
00001BA3  6A00              push byte +0x0
00001BA5  68C01E0200        push dword 0x21ec0
00001BAA  E811400000        call 0x5bc0
00001BAF  83C410            add esp,byte +0x10
00001BB2  B800020000        mov eax,0x200
00001BB7  817D1000020000    cmp dword [ebp+0x10],0x200
00001BBE  0F464510          cmovna eax,[ebp+0x10]
00001BC2  89458C            mov [ebp-0x74],eax
00001BC5  C745CC00000000    mov dword [ebp-0x34],0x0
00001BCC  EB1A              jmp short 0x1be8
00001BCE  8B550C            mov edx,[ebp+0xc]
00001BD1  8B45CC            mov eax,[ebp-0x34]
00001BD4  01D0              add eax,edx
00001BD6  0FB600            movzx eax,byte [eax]
00001BD9  8B55CC            mov edx,[ebp-0x34]
00001BDC  81C2C01E0200      add edx,0x21ec0
00001BE2  8802              mov [edx],al
00001BE4  8345CC01          add dword [ebp-0x34],byte +0x1
00001BE8  8B45CC            mov eax,[ebp-0x34]
00001BEB  3B458C            cmp eax,[ebp-0x74]
00001BEE  72DE              jc 0x1bce
00001BF0  83EC08            sub esp,byte +0x8
00001BF3  68C01E0200        push dword 0x21ec0
00001BF8  FF7590            push dword [ebp-0x70]
00001BFB  E8A8420000        call 0x5ea8
00001C00  83C410            add esp,byte +0x10
00001C03  8B45E8            mov eax,[ebp-0x18]
00001C06  C1E005            shl eax,byte 0x5
00001C09  89C2              mov edx,eax
00001C0B  8B4598            mov eax,[ebp-0x68]
00001C0E  01C2              add edx,eax
00001C10  8B4510            mov eax,[ebp+0x10]
00001C13  89421C            mov [edx+0x1c],eax
00001C16  83EC08            sub esp,byte +0x8
00001C19  68801C0200        push dword 0x21c80
00001C1E  FF75EC            push dword [ebp-0x14]
00001C21  E882420000        call 0x5ea8
00001C26  83C410            add esp,byte +0x10
00001C29  B801000000        mov eax,0x1
00001C2E  E9BD040000        jmp 0x20f0
00001C33  90                nop
00001C34  EB04              jmp short 0x1c3a
00001C36  90                nop
00001C37  EB01              jmp short 0x1c3a
00001C39  90                nop
00001C3A  8345E801          add dword [ebp-0x18],byte +0x1
00001C3E  837DE80F          cmp dword [ebp-0x18],byte +0xf
00001C42  0F8E1EFDFFFF      jng near 0x1966
00001C48  EB01              jmp short 0x1c4b
00001C4A  90                nop
00001C4B  0FB705641C0200    movzx eax,word [dword 0x21c64]
00001C52  6685C0            test ax,ax
00001C55  7509              jnz 0x1c60
00001C57  8345F001          add dword [ebp-0x10],byte +0x1
00001C5B  E9A3FCFFFF        jmp 0x1903
00001C60  0FB745F6          movzx eax,word [ebp-0xa]
00001C64  83EC0C            sub esp,byte +0xc
00001C67  50                push eax
00001C68  E812F0FFFF        call 0xc7f
00001C6D  83C410            add esp,byte +0x10
00001C70  668945F6          mov [ebp-0xa],ax
00001C74  66817DF6F70F      cmp word [ebp-0xa],0xff7
00001C7A  7708              ja 0x1c84
00001C7C  E982FCFFFF        jmp 0x1903
00001C81  90                nop
00001C82  EB01              jmp short 0x1c85
00001C84  90                nop
00001C85  C745C800000000    mov dword [ebp-0x38],0x0
00001C8C  C745C4FFFFFFFF    mov dword [ebp-0x3c],0xffffffff
00001C93  0FB705641C0200    movzx eax,word [dword 0x21c64]
00001C9A  668945C2          mov [ebp-0x3e],ax
00001C9E  C745BC00000000    mov dword [ebp-0x44],0x0
00001CA5  0FB705641C0200    movzx eax,word [dword 0x21c64]
00001CAC  6685C0            test ax,ax
00001CAF  7520              jnz 0x1cd1
00001CB1  8B55BC            mov edx,[ebp-0x44]
00001CB4  A160350200        mov eax,[0x23560]
00001CB9  39C2              cmp edx,eax
00001CBB  0F83B4000000      jnc near 0x1d75
00001CC1  8B156C350200      mov edx,[dword 0x2356c]
00001CC7  8B45BC            mov eax,[ebp-0x44]
00001CCA  01D0              add eax,edx
00001CCC  8945C8            mov [ebp-0x38],eax
00001CCF  EB11              jmp short 0x1ce2
00001CD1  0FB755C2          movzx edx,word [ebp-0x3e]
00001CD5  A164350200        mov eax,[0x23564]
00001CDA  01D0              add eax,edx
00001CDC  83E802            sub eax,byte +0x2
00001CDF  8945C8            mov [ebp-0x38],eax
00001CE2  83EC08            sub esp,byte +0x8
00001CE5  68801C0200        push dword 0x21c80
00001CEA  FF75C8            push dword [ebp-0x38]
00001CED  E80B410000        call 0x5dfd
00001CF2  83C410            add esp,byte +0x10
00001CF5  C74588801C0200    mov dword [ebp-0x78],0x21c80
00001CFC  C745B800000000    mov dword [ebp-0x48],0x0
00001D03  EB34              jmp short 0x1d39
00001D05  8B45B8            mov eax,[ebp-0x48]
00001D08  C1E005            shl eax,byte 0x5
00001D0B  89C2              mov edx,eax
00001D0D  8B4588            mov eax,[ebp-0x78]
00001D10  01D0              add eax,edx
00001D12  0FB600            movzx eax,byte [eax]
00001D15  84C0              test al,al
00001D17  7414              jz 0x1d2d
00001D19  8B45B8            mov eax,[ebp-0x48]
00001D1C  C1E005            shl eax,byte 0x5
00001D1F  89C2              mov edx,eax
00001D21  8B4588            mov eax,[ebp-0x78]
00001D24  01D0              add eax,edx
00001D26  0FB600            movzx eax,byte [eax]
00001D29  3CE5              cmp al,0xe5
00001D2B  7508              jnz 0x1d35
00001D2D  8B45B8            mov eax,[ebp-0x48]
00001D30  8945C4            mov [ebp-0x3c],eax
00001D33  EB44              jmp short 0x1d79
00001D35  8345B801          add dword [ebp-0x48],byte +0x1
00001D39  837DB80F          cmp dword [ebp-0x48],byte +0xf
00001D3D  7EC6              jng 0x1d05
00001D3F  0FB705641C0200    movzx eax,word [dword 0x21c64]
00001D46  6685C0            test ax,ax
00001D49  7509              jnz 0x1d54
00001D4B  8345BC01          add dword [ebp-0x44],byte +0x1
00001D4F  E951FFFFFF        jmp 0x1ca5
00001D54  0FB745C2          movzx eax,word [ebp-0x3e]
00001D58  83EC0C            sub esp,byte +0xc
00001D5B  50                push eax
00001D5C  E81EEFFFFF        call 0xc7f
00001D61  83C410            add esp,byte +0x10
00001D64  668945C2          mov [ebp-0x3e],ax
00001D68  66817DC2F70F      cmp word [ebp-0x3e],0xff7
00001D6E  7708              ja 0x1d78
00001D70  E930FFFFFF        jmp 0x1ca5
00001D75  90                nop
00001D76  EB01              jmp short 0x1d79
00001D78  90                nop
00001D79  837DC4FF          cmp dword [ebp-0x3c],byte -0x1
00001D7D  751A              jnz 0x1d99
00001D7F  83EC0C            sub esp,byte +0xc
00001D82  68E5B60100        push dword 0x1b6e5
00001D87  E89E320000        call 0x502a
00001D8C  83C410            add esp,byte +0x10
00001D8F  B800000000        mov eax,0x0
00001D94  E957030000        jmp 0x20f0
00001D99  66C745B60000      mov word [ebp-0x4a],0x0
00001D9F  A168350200        mov eax,[0x23568]
00001DA4  83EC08            sub esp,byte +0x8
00001DA7  68801C0200        push dword 0x21c80
00001DAC  50                push eax
00001DAD  E84B400000        call 0x5dfd
00001DB2  83C410            add esp,byte +0x10
00001DB5  C745B002000000    mov dword [ebp-0x50],0x2
00001DBC  E992000000        jmp 0x1e53
00001DC1  8B45B0            mov eax,[ebp-0x50]
00001DC4  89C2              mov edx,eax
00001DC6  C1EA1F            shr edx,byte 0x1f
00001DC9  01D0              add eax,edx
00001DCB  D1F8              sar eax,1
00001DCD  89C2              mov edx,eax
00001DCF  8B45B0            mov eax,[ebp-0x50]
00001DD2  01D0              add eax,edx
00001DD4  894584            mov [ebp-0x7c],eax
00001DD7  8B45B0            mov eax,[ebp-0x50]
00001DDA  83E001            and eax,byte +0x1
00001DDD  85C0              test eax,eax
00001DDF  742C              jz 0x1e0d
00001DE1  8B4584            mov eax,[ebp-0x7c]
00001DE4  05801C0200        add eax,0x21c80
00001DE9  0FB600            movzx eax,byte [eax]
00001DEC  C0E804            shr al,byte 0x4
00001DEF  0FB6C0            movzx eax,al
00001DF2  8B5584            mov edx,[ebp-0x7c]
00001DF5  83C201            add edx,byte +0x1
00001DF8  0FB692801C0200    movzx edx,byte [edx+0x21c80]
00001DFF  0FB6D2            movzx edx,dl
00001E02  C1E204            shl edx,byte 0x4
00001E05  09D0              or eax,edx
00001E07  668945AE          mov [ebp-0x52],ax
00001E0B  EB2C              jmp short 0x1e39
00001E0D  8B4584            mov eax,[ebp-0x7c]
00001E10  05801C0200        add eax,0x21c80
00001E15  0FB600            movzx eax,byte [eax]
00001E18  0FB6C0            movzx eax,al
00001E1B  8B5584            mov edx,[ebp-0x7c]
00001E1E  83C201            add edx,byte +0x1
00001E21  0FB692801C0200    movzx edx,byte [edx+0x21c80]
00001E28  0FB6D2            movzx edx,dl
00001E2B  C1E208            shl edx,byte 0x8
00001E2E  6681E2000F        and dx,0xf00
00001E33  09D0              or eax,edx
00001E35  668945AE          mov [ebp-0x52],ax
00001E39  0FB745AE          movzx eax,word [ebp-0x52]
00001E3D  25FF0F0000        and eax,0xfff
00001E42  85C0              test eax,eax
00001E44  7509              jnz 0x1e4f
00001E46  8B45B0            mov eax,[ebp-0x50]
00001E49  668945B6          mov [ebp-0x4a],ax
00001E4D  EB0E              jmp short 0x1e5d
00001E4F  8345B001          add dword [ebp-0x50],byte +0x1
00001E53  837DB07F          cmp dword [ebp-0x50],byte +0x7f
00001E57  0F8E64FFFFFF      jng near 0x1dc1
00001E5D  66837DB600        cmp word [ebp-0x4a],byte +0x0
00001E62  751A              jnz 0x1e7e
00001E64  83EC0C            sub esp,byte +0xc
00001E67  68FAB60100        push dword 0x1b6fa
00001E6C  E8B9310000        call 0x502a
00001E71  83C410            add esp,byte +0x10
00001E74  B800000000        mov eax,0x0
00001E79  E972020000        jmp 0x20f0
00001E7E  0FB755B6          movzx edx,word [ebp-0x4a]
00001E82  A164350200        mov eax,[0x23564]
00001E87  01D0              add eax,edx
00001E89  83E802            sub eax,byte +0x2
00001E8C  894580            mov [ebp-0x80],eax
00001E8F  83EC04            sub esp,byte +0x4
00001E92  6800020000        push dword 0x200
00001E97  6A00              push byte +0x0
00001E99  68C0200200        push dword 0x220c0
00001E9E  E81D3D0000        call 0x5bc0
00001EA3  83C410            add esp,byte +0x10
00001EA6  B800020000        mov eax,0x200
00001EAB  817D1000020000    cmp dword [ebp+0x10],0x200
00001EB2  0F464510          cmovna eax,[ebp+0x10]
00001EB6  89857CFFFFFF      mov [ebp-0x84],eax
00001EBC  C745A800000000    mov dword [ebp-0x58],0x0
00001EC3  EB1A              jmp short 0x1edf
00001EC5  8B55A8            mov edx,[ebp-0x58]
00001EC8  8B450C            mov eax,[ebp+0xc]
00001ECB  01D0              add eax,edx
00001ECD  0FB600            movzx eax,byte [eax]
00001ED0  8B55A8            mov edx,[ebp-0x58]
00001ED3  81C2C0200200      add edx,0x220c0
00001ED9  8802              mov [edx],al
00001EDB  8345A801          add dword [ebp-0x58],byte +0x1
00001EDF  8B45A8            mov eax,[ebp-0x58]
00001EE2  3B857CFFFFFF      cmp eax,[ebp-0x84]
00001EE8  7CDB              jl 0x1ec5
00001EEA  83EC08            sub esp,byte +0x8
00001EED  68C0200200        push dword 0x220c0
00001EF2  FF7580            push dword [ebp-0x80]
00001EF5  E8AE3F0000        call 0x5ea8
00001EFA  83C410            add esp,byte +0x10
00001EFD  0FB745B6          movzx eax,word [ebp-0x4a]
00001F01  83EC08            sub esp,byte +0x8
00001F04  68FF0F0000        push dword 0xfff
00001F09  50                push eax
00001F0A  E8AEF8FFFF        call 0x17bd
00001F0F  83C410            add esp,byte +0x10
00001F12  83EC08            sub esp,byte +0x8
00001F15  68801C0200        push dword 0x21c80
00001F1A  FF75C8            push dword [ebp-0x38]
00001F1D  E8DB3E0000        call 0x5dfd
00001F22  83C410            add esp,byte +0x10
00001F25  C78578FFFFFF801C  mov dword [ebp-0x88],0x21c80
         -0200
00001F2F  8B45C4            mov eax,[ebp-0x3c]
00001F32  C1E005            shl eax,byte 0x5
00001F35  89C2              mov edx,eax
00001F37  8B8578FFFFFF      mov eax,[ebp-0x88]
00001F3D  01D0              add eax,edx
00001F3F  83EC04            sub esp,byte +0x4
00001F42  6A08              push byte +0x8
00001F44  6A20              push byte +0x20
00001F46  50                push eax
00001F47  E8743C0000        call 0x5bc0
00001F4C  83C410            add esp,byte +0x10
00001F4F  8B45C4            mov eax,[ebp-0x3c]
00001F52  C1E005            shl eax,byte 0x5
00001F55  89C2              mov edx,eax
00001F57  8B8578FFFFFF      mov eax,[ebp-0x88]
00001F5D  01D0              add eax,edx
00001F5F  83C008            add eax,byte +0x8
00001F62  83EC04            sub esp,byte +0x4
00001F65  6A03              push byte +0x3
00001F67  6A20              push byte +0x20
00001F69  50                push eax
00001F6A  E8513C0000        call 0x5bc0
00001F6F  83C410            add esp,byte +0x10
00001F72  C745A400000000    mov dword [ebp-0x5c],0x0
00001F79  C745A000000000    mov dword [ebp-0x60],0x0
00001F80  EB4D              jmp short 0x1fcf
00001F82  8B45A4            mov eax,[ebp-0x5c]
00001F85  8D5001            lea edx,[eax+0x1]
00001F88  8955A4            mov [ebp-0x5c],edx
00001F8B  89C2              mov edx,eax
00001F8D  8B4508            mov eax,[ebp+0x8]
00001F90  01D0              add eax,edx
00001F92  0FB600            movzx eax,byte [eax]
00001F95  88459F            mov [ebp-0x61],al
00001F98  807D9F60          cmp byte [ebp-0x61],0x60
00001F9C  7E10              jng 0x1fae
00001F9E  807D9F7A          cmp byte [ebp-0x61],0x7a
00001FA2  7F0A              jg 0x1fae
00001FA4  0FB6459F          movzx eax,byte [ebp-0x61]
00001FA8  83E820            sub eax,byte +0x20
00001FAB  88459F            mov [ebp-0x61],al
00001FAE  8B45C4            mov eax,[ebp-0x3c]
00001FB1  C1E005            shl eax,byte 0x5
00001FB4  89C2              mov edx,eax
00001FB6  8B8578FFFFFF      mov eax,[ebp-0x88]
00001FBC  8D0C02            lea ecx,[edx+eax]
00001FBF  8B45A0            mov eax,[ebp-0x60]
00001FC2  8D5001            lea edx,[eax+0x1]
00001FC5  8955A0            mov [ebp-0x60],edx
00001FC8  0FB6559F          movzx edx,byte [ebp-0x61]
00001FCC  881401            mov [ecx+eax],dl
00001FCF  8B55A4            mov edx,[ebp-0x5c]
00001FD2  8B4508            mov eax,[ebp+0x8]
00001FD5  01D0              add eax,edx
00001FD7  0FB600            movzx eax,byte [eax]
00001FDA  84C0              test al,al
00001FDC  741B              jz 0x1ff9
00001FDE  8B55A4            mov edx,[ebp-0x5c]
00001FE1  8B4508            mov eax,[ebp+0x8]
00001FE4  01D0              add eax,edx
00001FE6  0FB600            movzx eax,byte [eax]
00001FE9  3C2E              cmp al,0x2e
00001FEB  740C              jz 0x1ff9
00001FED  837DA007          cmp dword [ebp-0x60],byte +0x7
00001FF1  7E8F              jng 0x1f82
00001FF3  EB04              jmp short 0x1ff9
00001FF5  8345A401          add dword [ebp-0x5c],byte +0x1
00001FF9  8B55A4            mov edx,[ebp-0x5c]
00001FFC  8B4508            mov eax,[ebp+0x8]
00001FFF  01D0              add eax,edx
00002001  0FB600            movzx eax,byte [eax]
00002004  84C0              test al,al
00002006  740F              jz 0x2017
00002008  8B55A4            mov edx,[ebp-0x5c]
0000200B  8B4508            mov eax,[ebp+0x8]
0000200E  01D0              add eax,edx
00002010  0FB600            movzx eax,byte [eax]
00002013  3C2E              cmp al,0x2e
00002015  75DE              jnz 0x1ff5
00002017  8B55A4            mov edx,[ebp-0x5c]
0000201A  8B4508            mov eax,[ebp+0x8]
0000201D  01D0              add eax,edx
0000201F  0FB600            movzx eax,byte [eax]
00002022  3C2E              cmp al,0x2e
00002024  7504              jnz 0x202a
00002026  8345A401          add dword [ebp-0x5c],byte +0x1
0000202A  C745A000000000    mov dword [ebp-0x60],0x0
00002031  EB4E              jmp short 0x2081
00002033  8B45A4            mov eax,[ebp-0x5c]
00002036  8D5001            lea edx,[eax+0x1]
00002039  8955A4            mov [ebp-0x5c],edx
0000203C  89C2              mov edx,eax
0000203E  8B4508            mov eax,[ebp+0x8]
00002041  01D0              add eax,edx
00002043  0FB600            movzx eax,byte [eax]
00002046  88459E            mov [ebp-0x62],al
00002049  807D9E60          cmp byte [ebp-0x62],0x60
0000204D  7E10              jng 0x205f
0000204F  807D9E7A          cmp byte [ebp-0x62],0x7a
00002053  7F0A              jg 0x205f
00002055  0FB6459E          movzx eax,byte [ebp-0x62]
00002059  83E820            sub eax,byte +0x20
0000205C  88459E            mov [ebp-0x62],al
0000205F  8B45C4            mov eax,[ebp-0x3c]
00002062  C1E005            shl eax,byte 0x5
00002065  89C2              mov edx,eax
00002067  8B8578FFFFFF      mov eax,[ebp-0x88]
0000206D  8D0C02            lea ecx,[edx+eax]
00002070  8B45A0            mov eax,[ebp-0x60]
00002073  8D5001            lea edx,[eax+0x1]
00002076  8955A0            mov [ebp-0x60],edx
00002079  0FB6559E          movzx edx,byte [ebp-0x62]
0000207D  88540108          mov [ecx+eax+0x8],dl
00002081  8B55A4            mov edx,[ebp-0x5c]
00002084  8B4508            mov eax,[ebp+0x8]
00002087  01D0              add eax,edx
00002089  0FB600            movzx eax,byte [eax]
0000208C  84C0              test al,al
0000208E  7406              jz 0x2096
00002090  837DA002          cmp dword [ebp-0x60],byte +0x2
00002094  7E9D              jng 0x2033
00002096  8B45C4            mov eax,[ebp-0x3c]
00002099  C1E005            shl eax,byte 0x5
0000209C  89C2              mov edx,eax
0000209E  8B8578FFFFFF      mov eax,[ebp-0x88]
000020A4  01D0              add eax,edx
000020A6  C6400B20          mov byte [eax+0xb],0x20
000020AA  8B45C4            mov eax,[ebp-0x3c]
000020AD  C1E005            shl eax,byte 0x5
000020B0  89C2              mov edx,eax
000020B2  8B8578FFFFFF      mov eax,[ebp-0x88]
000020B8  01C2              add edx,eax
000020BA  0FB745B6          movzx eax,word [ebp-0x4a]
000020BE  6689421A          mov [edx+0x1a],ax
000020C2  8B45C4            mov eax,[ebp-0x3c]
000020C5  C1E005            shl eax,byte 0x5
000020C8  89C2              mov edx,eax
000020CA  8B8578FFFFFF      mov eax,[ebp-0x88]
000020D0  01C2              add edx,eax
000020D2  8B4510            mov eax,[ebp+0x10]
000020D5  89421C            mov [edx+0x1c],eax
000020D8  83EC08            sub esp,byte +0x8
000020DB  68801C0200        push dword 0x21c80
000020E0  FF75C8            push dword [ebp-0x38]
000020E3  E8C03D0000        call 0x5ea8
000020E8  83C410            add esp,byte +0x10
000020EB  B801000000        mov eax,0x1
000020F0  C9                leave
000020F1  C3                ret
000020F2  55                push ebp
000020F3  89E5              mov ebp,esp
000020F5  83EC58            sub esp,byte +0x58
000020F8  A1601C0200        mov eax,[0x21c60]
000020FD  85C0              test eax,eax
000020FF  7505              jnz 0x2106
00002101  E85FECFFFF        call 0xd65
00002106  0FB705641C0200    movzx eax,word [dword 0x21c64]
0000210D  668945F6          mov [ebp-0xa],ax
00002111  C745F000000000    mov dword [ebp-0x10],0x0
00002118  0FB705641C0200    movzx eax,word [dword 0x21c64]
0000211F  6685C0            test ax,ax
00002122  7524              jnz 0x2148
00002124  8B55F0            mov edx,[ebp-0x10]
00002127  A160350200        mov eax,[0x23560]
0000212C  39C2              cmp edx,eax
0000212E  0F83D3020000      jnc near 0x2407
00002134  8B156C350200      mov edx,[dword 0x2356c]
0000213A  8B45F0            mov eax,[ebp-0x10]
0000213D  01D0              add eax,edx
0000213F  8945EC            mov [ebp-0x14],eax
00002142  8345F001          add dword [ebp-0x10],byte +0x1
00002146  EB11              jmp short 0x2159
00002148  0FB755F6          movzx edx,word [ebp-0xa]
0000214C  A164350200        mov eax,[0x23564]
00002151  01D0              add eax,edx
00002153  83E802            sub eax,byte +0x2
00002156  8945EC            mov [ebp-0x14],eax
00002159  83EC08            sub esp,byte +0x8
0000215C  68801C0200        push dword 0x21c80
00002161  FF75EC            push dword [ebp-0x14]
00002164  E8943C0000        call 0x5dfd
00002169  83C410            add esp,byte +0x10
0000216C  C745C8801C0200    mov dword [ebp-0x38],0x21c80
00002173  C745E800000000    mov dword [ebp-0x18],0x0
0000217A  E94D020000        jmp 0x23cc
0000217F  8B45E8            mov eax,[ebp-0x18]
00002182  C1E005            shl eax,byte 0x5
00002185  89C2              mov edx,eax
00002187  8B45C8            mov eax,[ebp-0x38]
0000218A  01D0              add eax,edx
0000218C  0FB600            movzx eax,byte [eax]
0000218F  84C0              test al,al
00002191  750A              jnz 0x219d
00002193  B800000000        mov eax,0x0
00002198  E973020000        jmp 0x2410
0000219D  8B45E8            mov eax,[ebp-0x18]
000021A0  C1E005            shl eax,byte 0x5
000021A3  89C2              mov edx,eax
000021A5  8B45C8            mov eax,[ebp-0x38]
000021A8  01D0              add eax,edx
000021AA  0FB600            movzx eax,byte [eax]
000021AD  3CE5              cmp al,0xe5
000021AF  0F8412020000      jz near 0x23c7
000021B5  C745E400000000    mov dword [ebp-0x1c],0x0
000021BC  C745E000000000    mov dword [ebp-0x20],0x0
000021C3  EB40              jmp short 0x2205
000021C5  8B45E8            mov eax,[ebp-0x18]
000021C8  C1E005            shl eax,byte 0x5
000021CB  89C2              mov edx,eax
000021CD  8B45C8            mov eax,[ebp-0x38]
000021D0  01C2              add edx,eax
000021D2  8B45E0            mov eax,[ebp-0x20]
000021D5  01D0              add eax,edx
000021D7  0FB600            movzx eax,byte [eax]
000021DA  3C20              cmp al,0x20
000021DC  742F              jz 0x220d
000021DE  8B45E8            mov eax,[ebp-0x18]
000021E1  C1E005            shl eax,byte 0x5
000021E4  89C2              mov edx,eax
000021E6  8B45C8            mov eax,[ebp-0x38]
000021E9  8D0C02            lea ecx,[edx+eax]
000021EC  8B45E4            mov eax,[ebp-0x1c]
000021EF  8D5001            lea edx,[eax+0x1]
000021F2  8955E4            mov [ebp-0x1c],edx
000021F5  8B55E0            mov edx,[ebp-0x20]
000021F8  01CA              add edx,ecx
000021FA  0FB612            movzx edx,byte [edx]
000021FD  885405B7          mov [ebp+eax-0x49],dl
00002201  8345E001          add dword [ebp-0x20],byte +0x1
00002205  837DE007          cmp dword [ebp-0x20],byte +0x7
00002209  7EBA              jng 0x21c5
0000220B  EB01              jmp short 0x220e
0000220D  90                nop
0000220E  8B45E8            mov eax,[ebp-0x18]
00002211  C1E005            shl eax,byte 0x5
00002214  89C2              mov edx,eax
00002216  8B45C8            mov eax,[ebp-0x38]
00002219  01D0              add eax,edx
0000221B  0FB64008          movzx eax,byte [eax+0x8]
0000221F  3C20              cmp al,0x20
00002221  7466              jz 0x2289
00002223  8B45E4            mov eax,[ebp-0x1c]
00002226  8D5001            lea edx,[eax+0x1]
00002229  8955E4            mov [ebp-0x1c],edx
0000222C  C64405B72E        mov byte [ebp+eax-0x49],0x2e
00002231  C745DC00000000    mov dword [ebp-0x24],0x0
00002238  EB46              jmp short 0x2280
0000223A  8B45E8            mov eax,[ebp-0x18]
0000223D  C1E005            shl eax,byte 0x5
00002240  89C2              mov edx,eax
00002242  8B45C8            mov eax,[ebp-0x38]
00002245  01C2              add edx,eax
00002247  8B45DC            mov eax,[ebp-0x24]
0000224A  01D0              add eax,edx
0000224C  83C008            add eax,byte +0x8
0000224F  0FB600            movzx eax,byte [eax]
00002252  3C20              cmp al,0x20
00002254  7432              jz 0x2288
00002256  8B45E8            mov eax,[ebp-0x18]
00002259  C1E005            shl eax,byte 0x5
0000225C  89C2              mov edx,eax
0000225E  8B45C8            mov eax,[ebp-0x38]
00002261  8D0C02            lea ecx,[edx+eax]
00002264  8B45E4            mov eax,[ebp-0x1c]
00002267  8D5001            lea edx,[eax+0x1]
0000226A  8955E4            mov [ebp-0x1c],edx
0000226D  8B55DC            mov edx,[ebp-0x24]
00002270  01CA              add edx,ecx
00002272  83C208            add edx,byte +0x8
00002275  0FB612            movzx edx,byte [edx]
00002278  885405B7          mov [ebp+eax-0x49],dl
0000227C  8345DC01          add dword [ebp-0x24],byte +0x1
00002280  837DDC02          cmp dword [ebp-0x24],byte +0x2
00002284  7EB4              jng 0x223a
00002286  EB01              jmp short 0x2289
00002288  90                nop
00002289  8D55B7            lea edx,[ebp-0x49]
0000228C  8B45E4            mov eax,[ebp-0x1c]
0000228F  01D0              add eax,edx
00002291  C60000            mov byte [eax],0x0
00002294  C745D801000000    mov dword [ebp-0x28],0x1
0000229B  8B4508            mov eax,[ebp+0x8]
0000229E  8945D4            mov [ebp-0x2c],eax
000022A1  8D45B7            lea eax,[ebp-0x49]
000022A4  8945D0            mov [ebp-0x30],eax
000022A7  EB6A              jmp short 0x2313
000022A9  8B45D4            mov eax,[ebp-0x2c]
000022AC  0FB600            movzx eax,byte [eax]
000022AF  3C60              cmp al,0x60
000022B1  7E15              jng 0x22c8
000022B3  8B45D4            mov eax,[ebp-0x2c]
000022B6  0FB600            movzx eax,byte [eax]
000022B9  3C7A              cmp al,0x7a
000022BB  7F0B              jg 0x22c8
000022BD  8B45D4            mov eax,[ebp-0x2c]
000022C0  0FB600            movzx eax,byte [eax]
000022C3  83E820            sub eax,byte +0x20
000022C6  EB06              jmp short 0x22ce
000022C8  8B45D4            mov eax,[ebp-0x2c]
000022CB  0FB600            movzx eax,byte [eax]
000022CE  8845C7            mov [ebp-0x39],al
000022D1  8B45D0            mov eax,[ebp-0x30]
000022D4  0FB600            movzx eax,byte [eax]
000022D7  3C60              cmp al,0x60
000022D9  7E15              jng 0x22f0
000022DB  8B45D0            mov eax,[ebp-0x30]
000022DE  0FB600            movzx eax,byte [eax]
000022E1  3C7A              cmp al,0x7a
000022E3  7F0B              jg 0x22f0
000022E5  8B45D0            mov eax,[ebp-0x30]
000022E8  0FB600            movzx eax,byte [eax]
000022EB  83E820            sub eax,byte +0x20
000022EE  EB06              jmp short 0x22f6
000022F0  8B45D0            mov eax,[ebp-0x30]
000022F3  0FB600            movzx eax,byte [eax]
000022F6  8845C6            mov [ebp-0x3a],al
000022F9  0FB645C7          movzx eax,byte [ebp-0x39]
000022FD  3A45C6            cmp al,[ebp-0x3a]
00002300  7409              jz 0x230b
00002302  C745D800000000    mov dword [ebp-0x28],0x0
00002309  EB1C              jmp short 0x2327
0000230B  8345D401          add dword [ebp-0x2c],byte +0x1
0000230F  8345D001          add dword [ebp-0x30],byte +0x1
00002313  8B45D4            mov eax,[ebp-0x2c]
00002316  0FB600            movzx eax,byte [eax]
00002319  84C0              test al,al
0000231B  740A              jz 0x2327
0000231D  8B45D0            mov eax,[ebp-0x30]
00002320  0FB600            movzx eax,byte [eax]
00002323  84C0              test al,al
00002325  7582              jnz 0x22a9
00002327  837DD800          cmp dword [ebp-0x28],byte +0x0
0000232B  0F8497000000      jz near 0x23c8
00002331  8B45D4            mov eax,[ebp-0x2c]
00002334  0FB600            movzx eax,byte [eax]
00002337  84C0              test al,al
00002339  0F8589000000      jnz near 0x23c8
0000233F  8B45D0            mov eax,[ebp-0x30]
00002342  0FB600            movzx eax,byte [eax]
00002345  84C0              test al,al
00002347  757F              jnz 0x23c8
00002349  8B45E8            mov eax,[ebp-0x18]
0000234C  C1E005            shl eax,byte 0x5
0000234F  89C2              mov edx,eax
00002351  8B45C8            mov eax,[ebp-0x38]
00002354  01D0              add eax,edx
00002356  0FB7401A          movzx eax,word [eax+0x1a]
0000235A  668945CE          mov [ebp-0x32],ax
0000235E  8B45E8            mov eax,[ebp-0x18]
00002361  C1E005            shl eax,byte 0x5
00002364  89C2              mov edx,eax
00002366  8B45C8            mov eax,[ebp-0x38]
00002369  01D0              add eax,edx
0000236B  C600E5            mov byte [eax],0xe5
0000236E  83EC08            sub esp,byte +0x8
00002371  68801C0200        push dword 0x21c80
00002376  FF75EC            push dword [ebp-0x14]
00002379  E82A3B0000        call 0x5ea8
0000237E  83C410            add esp,byte +0x10
00002381  EB2E              jmp short 0x23b1
00002383  0FB745CE          movzx eax,word [ebp-0x32]
00002387  83EC0C            sub esp,byte +0xc
0000238A  50                push eax
0000238B  E8EFE8FFFF        call 0xc7f
00002390  83C410            add esp,byte +0x10
00002393  668945C4          mov [ebp-0x3c],ax
00002397  0FB745CE          movzx eax,word [ebp-0x32]
0000239B  83EC08            sub esp,byte +0x8
0000239E  6A00              push byte +0x0
000023A0  50                push eax
000023A1  E817F4FFFF        call 0x17bd
000023A6  83C410            add esp,byte +0x10
000023A9  0FB745C4          movzx eax,word [ebp-0x3c]
000023AD  668945CE          mov [ebp-0x32],ax
000023B1  66837DCE01        cmp word [ebp-0x32],byte +0x1
000023B6  7608              jna 0x23c0
000023B8  66817DCEEF0F      cmp word [ebp-0x32],0xfef
000023BE  76C3              jna 0x2383
000023C0  B801000000        mov eax,0x1
000023C5  EB49              jmp short 0x2410
000023C7  90                nop
000023C8  8345E801          add dword [ebp-0x18],byte +0x1
000023CC  837DE80F          cmp dword [ebp-0x18],byte +0xf
000023D0  0F8EA9FDFFFF      jng near 0x217f
000023D6  0FB705641C0200    movzx eax,word [dword 0x21c64]
000023DD  6685C0            test ax,ax
000023E0  0F8432FDFFFF      jz near 0x2118
000023E6  0FB745F6          movzx eax,word [ebp-0xa]
000023EA  83EC0C            sub esp,byte +0xc
000023ED  50                push eax
000023EE  E88CE8FFFF        call 0xc7f
000023F3  83C410            add esp,byte +0x10
000023F6  668945F6          mov [ebp-0xa],ax
000023FA  66817DF6F70F      cmp word [ebp-0xa],0xff7
00002400  7708              ja 0x240a
00002402  E911FDFFFF        jmp 0x2118
00002407  90                nop
00002408  EB01              jmp short 0x240b
0000240A  90                nop
0000240B  B800000000        mov eax,0x0
00002410  C9                leave
00002411  C3                ret
00002412  55                push ebp
00002413  89E5              mov ebp,esp
00002415  83EC58            sub esp,byte +0x58
00002418  A1601C0200        mov eax,[0x21c60]
0000241D  85C0              test eax,eax
0000241F  7505              jnz 0x2426
00002421  E83FE9FFFF        call 0xd65
00002426  0FB705641C0200    movzx eax,word [dword 0x21c64]
0000242D  668945F6          mov [ebp-0xa],ax
00002431  C745F000000000    mov dword [ebp-0x10],0x0
00002438  0FB705641C0200    movzx eax,word [dword 0x21c64]
0000243F  6685C0            test ax,ax
00002442  7524              jnz 0x2468
00002444  8B55F0            mov edx,[ebp-0x10]
00002447  A160350200        mov eax,[0x23560]
0000244C  39C2              cmp edx,eax
0000244E  0F83CE030000      jnc near 0x2822
00002454  8B156C350200      mov edx,[dword 0x2356c]
0000245A  8B45F0            mov eax,[ebp-0x10]
0000245D  01D0              add eax,edx
0000245F  8945EC            mov [ebp-0x14],eax
00002462  8345F001          add dword [ebp-0x10],byte +0x1
00002466  EB11              jmp short 0x2479
00002468  0FB755F6          movzx edx,word [ebp-0xa]
0000246C  A164350200        mov eax,[0x23564]
00002471  01D0              add eax,edx
00002473  83E802            sub eax,byte +0x2
00002476  8945EC            mov [ebp-0x14],eax
00002479  83EC08            sub esp,byte +0x8
0000247C  68801C0200        push dword 0x21c80
00002481  FF75EC            push dword [ebp-0x14]
00002484  E874390000        call 0x5dfd
00002489  83C410            add esp,byte +0x10
0000248C  C745C0801C0200    mov dword [ebp-0x40],0x21c80
00002493  C745E800000000    mov dword [ebp-0x18],0x0
0000249A  E948030000        jmp 0x27e7
0000249F  8B45E8            mov eax,[ebp-0x18]
000024A2  C1E005            shl eax,byte 0x5
000024A5  89C2              mov edx,eax
000024A7  8B45C0            mov eax,[ebp-0x40]
000024AA  01D0              add eax,edx
000024AC  0FB600            movzx eax,byte [eax]
000024AF  84C0              test al,al
000024B1  750A              jnz 0x24bd
000024B3  B800000000        mov eax,0x0
000024B8  E96E030000        jmp 0x282b
000024BD  8B45E8            mov eax,[ebp-0x18]
000024C0  C1E005            shl eax,byte 0x5
000024C3  89C2              mov edx,eax
000024C5  8B45C0            mov eax,[ebp-0x40]
000024C8  01D0              add eax,edx
000024CA  0FB600            movzx eax,byte [eax]
000024CD  3CE5              cmp al,0xe5
000024CF  0F840D030000      jz near 0x27e2
000024D5  C745E400000000    mov dword [ebp-0x1c],0x0
000024DC  C745E000000000    mov dword [ebp-0x20],0x0
000024E3  EB40              jmp short 0x2525
000024E5  8B45E8            mov eax,[ebp-0x18]
000024E8  C1E005            shl eax,byte 0x5
000024EB  89C2              mov edx,eax
000024ED  8B45C0            mov eax,[ebp-0x40]
000024F0  01C2              add edx,eax
000024F2  8B45E0            mov eax,[ebp-0x20]
000024F5  01D0              add eax,edx
000024F7  0FB600            movzx eax,byte [eax]
000024FA  3C20              cmp al,0x20
000024FC  742F              jz 0x252d
000024FE  8B45E8            mov eax,[ebp-0x18]
00002501  C1E005            shl eax,byte 0x5
00002504  89C2              mov edx,eax
00002506  8B45C0            mov eax,[ebp-0x40]
00002509  8D0C02            lea ecx,[edx+eax]
0000250C  8B45E4            mov eax,[ebp-0x1c]
0000250F  8D5001            lea edx,[eax+0x1]
00002512  8955E4            mov [ebp-0x1c],edx
00002515  8B55E0            mov edx,[ebp-0x20]
00002518  01CA              add edx,ecx
0000251A  0FB612            movzx edx,byte [edx]
0000251D  885405B1          mov [ebp+eax-0x4f],dl
00002521  8345E001          add dword [ebp-0x20],byte +0x1
00002525  837DE007          cmp dword [ebp-0x20],byte +0x7
00002529  7EBA              jng 0x24e5
0000252B  EB01              jmp short 0x252e
0000252D  90                nop
0000252E  8B45E8            mov eax,[ebp-0x18]
00002531  C1E005            shl eax,byte 0x5
00002534  89C2              mov edx,eax
00002536  8B45C0            mov eax,[ebp-0x40]
00002539  01D0              add eax,edx
0000253B  0FB64008          movzx eax,byte [eax+0x8]
0000253F  3C20              cmp al,0x20
00002541  7466              jz 0x25a9
00002543  8B45E4            mov eax,[ebp-0x1c]
00002546  8D5001            lea edx,[eax+0x1]
00002549  8955E4            mov [ebp-0x1c],edx
0000254C  C64405B12E        mov byte [ebp+eax-0x4f],0x2e
00002551  C745DC00000000    mov dword [ebp-0x24],0x0
00002558  EB46              jmp short 0x25a0
0000255A  8B45E8            mov eax,[ebp-0x18]
0000255D  C1E005            shl eax,byte 0x5
00002560  89C2              mov edx,eax
00002562  8B45C0            mov eax,[ebp-0x40]
00002565  01C2              add edx,eax
00002567  8B45DC            mov eax,[ebp-0x24]
0000256A  01D0              add eax,edx
0000256C  83C008            add eax,byte +0x8
0000256F  0FB600            movzx eax,byte [eax]
00002572  3C20              cmp al,0x20
00002574  7432              jz 0x25a8
00002576  8B45E8            mov eax,[ebp-0x18]
00002579  C1E005            shl eax,byte 0x5
0000257C  89C2              mov edx,eax
0000257E  8B45C0            mov eax,[ebp-0x40]
00002581  8D0C02            lea ecx,[edx+eax]
00002584  8B45E4            mov eax,[ebp-0x1c]
00002587  8D5001            lea edx,[eax+0x1]
0000258A  8955E4            mov [ebp-0x1c],edx
0000258D  8B55DC            mov edx,[ebp-0x24]
00002590  01CA              add edx,ecx
00002592  83C208            add edx,byte +0x8
00002595  0FB612            movzx edx,byte [edx]
00002598  885405B1          mov [ebp+eax-0x4f],dl
0000259C  8345DC01          add dword [ebp-0x24],byte +0x1
000025A0  837DDC02          cmp dword [ebp-0x24],byte +0x2
000025A4  7EB4              jng 0x255a
000025A6  EB01              jmp short 0x25a9
000025A8  90                nop
000025A9  8D55B1            lea edx,[ebp-0x4f]
000025AC  8B45E4            mov eax,[ebp-0x1c]
000025AF  01D0              add eax,edx
000025B1  C60000            mov byte [eax],0x0
000025B4  C745D801000000    mov dword [ebp-0x28],0x1
000025BB  8B4508            mov eax,[ebp+0x8]
000025BE  8945D4            mov [ebp-0x2c],eax
000025C1  8D45B1            lea eax,[ebp-0x4f]
000025C4  8945D0            mov [ebp-0x30],eax
000025C7  EB6A              jmp short 0x2633
000025C9  8B45D4            mov eax,[ebp-0x2c]
000025CC  0FB600            movzx eax,byte [eax]
000025CF  3C60              cmp al,0x60
000025D1  7E15              jng 0x25e8
000025D3  8B45D4            mov eax,[ebp-0x2c]
000025D6  0FB600            movzx eax,byte [eax]
000025D9  3C7A              cmp al,0x7a
000025DB  7F0B              jg 0x25e8
000025DD  8B45D4            mov eax,[ebp-0x2c]
000025E0  0FB600            movzx eax,byte [eax]
000025E3  83E820            sub eax,byte +0x20
000025E6  EB06              jmp short 0x25ee
000025E8  8B45D4            mov eax,[ebp-0x2c]
000025EB  0FB600            movzx eax,byte [eax]
000025EE  8845BF            mov [ebp-0x41],al
000025F1  8B45D0            mov eax,[ebp-0x30]
000025F4  0FB600            movzx eax,byte [eax]
000025F7  3C60              cmp al,0x60
000025F9  7E15              jng 0x2610
000025FB  8B45D0            mov eax,[ebp-0x30]
000025FE  0FB600            movzx eax,byte [eax]
00002601  3C7A              cmp al,0x7a
00002603  7F0B              jg 0x2610
00002605  8B45D0            mov eax,[ebp-0x30]
00002608  0FB600            movzx eax,byte [eax]
0000260B  83E820            sub eax,byte +0x20
0000260E  EB06              jmp short 0x2616
00002610  8B45D0            mov eax,[ebp-0x30]
00002613  0FB600            movzx eax,byte [eax]
00002616  8845BE            mov [ebp-0x42],al
00002619  0FB645BF          movzx eax,byte [ebp-0x41]
0000261D  3A45BE            cmp al,[ebp-0x42]
00002620  7409              jz 0x262b
00002622  C745D800000000    mov dword [ebp-0x28],0x0
00002629  EB1C              jmp short 0x2647
0000262B  8345D401          add dword [ebp-0x2c],byte +0x1
0000262F  8345D001          add dword [ebp-0x30],byte +0x1
00002633  8B45D4            mov eax,[ebp-0x2c]
00002636  0FB600            movzx eax,byte [eax]
00002639  84C0              test al,al
0000263B  740A              jz 0x2647
0000263D  8B45D0            mov eax,[ebp-0x30]
00002640  0FB600            movzx eax,byte [eax]
00002643  84C0              test al,al
00002645  7582              jnz 0x25c9
00002647  837DD800          cmp dword [ebp-0x28],byte +0x0
0000264B  0F8492010000      jz near 0x27e3
00002651  8B45D4            mov eax,[ebp-0x2c]
00002654  0FB600            movzx eax,byte [eax]
00002657  84C0              test al,al
00002659  0F8584010000      jnz near 0x27e3
0000265F  8B45D0            mov eax,[ebp-0x30]
00002662  0FB600            movzx eax,byte [eax]
00002665  84C0              test al,al
00002667  0F8576010000      jnz near 0x27e3
0000266D  8B45E8            mov eax,[ebp-0x18]
00002670  C1E005            shl eax,byte 0x5
00002673  89C2              mov edx,eax
00002675  8B45C0            mov eax,[ebp-0x40]
00002678  01D0              add eax,edx
0000267A  83EC04            sub esp,byte +0x4
0000267D  6A08              push byte +0x8
0000267F  6A20              push byte +0x20
00002681  50                push eax
00002682  E839350000        call 0x5bc0
00002687  83C410            add esp,byte +0x10
0000268A  8B45E8            mov eax,[ebp-0x18]
0000268D  C1E005            shl eax,byte 0x5
00002690  89C2              mov edx,eax
00002692  8B45C0            mov eax,[ebp-0x40]
00002695  01D0              add eax,edx
00002697  83C008            add eax,byte +0x8
0000269A  83EC04            sub esp,byte +0x4
0000269D  6A03              push byte +0x3
0000269F  6A20              push byte +0x20
000026A1  50                push eax
000026A2  E819350000        call 0x5bc0
000026A7  83C410            add esp,byte +0x10
000026AA  C745CC00000000    mov dword [ebp-0x34],0x0
000026B1  C745C800000000    mov dword [ebp-0x38],0x0
000026B8  EB4A              jmp short 0x2704
000026BA  8B45CC            mov eax,[ebp-0x34]
000026BD  8D5001            lea edx,[eax+0x1]
000026C0  8955CC            mov [ebp-0x34],edx
000026C3  89C2              mov edx,eax
000026C5  8B450C            mov eax,[ebp+0xc]
000026C8  01D0              add eax,edx
000026CA  0FB600            movzx eax,byte [eax]
000026CD  8845C7            mov [ebp-0x39],al
000026D0  807DC760          cmp byte [ebp-0x39],0x60
000026D4  7E10              jng 0x26e6
000026D6  807DC77A          cmp byte [ebp-0x39],0x7a
000026DA  7F0A              jg 0x26e6
000026DC  0FB645C7          movzx eax,byte [ebp-0x39]
000026E0  83E820            sub eax,byte +0x20
000026E3  8845C7            mov [ebp-0x39],al
000026E6  8B45E8            mov eax,[ebp-0x18]
000026E9  C1E005            shl eax,byte 0x5
000026EC  89C2              mov edx,eax
000026EE  8B45C0            mov eax,[ebp-0x40]
000026F1  8D0C02            lea ecx,[edx+eax]
000026F4  8B45C8            mov eax,[ebp-0x38]
000026F7  8D5001            lea edx,[eax+0x1]
000026FA  8955C8            mov [ebp-0x38],edx
000026FD  0FB655C7          movzx edx,byte [ebp-0x39]
00002701  881401            mov [ecx+eax],dl
00002704  8B55CC            mov edx,[ebp-0x34]
00002707  8B450C            mov eax,[ebp+0xc]
0000270A  01D0              add eax,edx
0000270C  0FB600            movzx eax,byte [eax]
0000270F  84C0              test al,al
00002711  741B              jz 0x272e
00002713  8B55CC            mov edx,[ebp-0x34]
00002716  8B450C            mov eax,[ebp+0xc]
00002719  01D0              add eax,edx
0000271B  0FB600            movzx eax,byte [eax]
0000271E  3C2E              cmp al,0x2e
00002720  740C              jz 0x272e
00002722  837DC807          cmp dword [ebp-0x38],byte +0x7
00002726  7E92              jng 0x26ba
00002728  EB04              jmp short 0x272e
0000272A  8345CC01          add dword [ebp-0x34],byte +0x1
0000272E  8B55CC            mov edx,[ebp-0x34]
00002731  8B450C            mov eax,[ebp+0xc]
00002734  01D0              add eax,edx
00002736  0FB600            movzx eax,byte [eax]
00002739  84C0              test al,al
0000273B  740F              jz 0x274c
0000273D  8B55CC            mov edx,[ebp-0x34]
00002740  8B450C            mov eax,[ebp+0xc]
00002743  01D0              add eax,edx
00002745  0FB600            movzx eax,byte [eax]
00002748  3C2E              cmp al,0x2e
0000274A  75DE              jnz 0x272a
0000274C  8B55CC            mov edx,[ebp-0x34]
0000274F  8B450C            mov eax,[ebp+0xc]
00002752  01D0              add eax,edx
00002754  0FB600            movzx eax,byte [eax]
00002757  3C2E              cmp al,0x2e
00002759  7504              jnz 0x275f
0000275B  8345CC01          add dword [ebp-0x34],byte +0x1
0000275F  C745C800000000    mov dword [ebp-0x38],0x0
00002766  EB4B              jmp short 0x27b3
00002768  8B45CC            mov eax,[ebp-0x34]
0000276B  8D5001            lea edx,[eax+0x1]
0000276E  8955CC            mov [ebp-0x34],edx
00002771  89C2              mov edx,eax
00002773  8B450C            mov eax,[ebp+0xc]
00002776  01D0              add eax,edx
00002778  0FB600            movzx eax,byte [eax]
0000277B  8845C6            mov [ebp-0x3a],al
0000277E  807DC660          cmp byte [ebp-0x3a],0x60
00002782  7E10              jng 0x2794
00002784  807DC67A          cmp byte [ebp-0x3a],0x7a
00002788  7F0A              jg 0x2794
0000278A  0FB645C6          movzx eax,byte [ebp-0x3a]
0000278E  83E820            sub eax,byte +0x20
00002791  8845C6            mov [ebp-0x3a],al
00002794  8B45E8            mov eax,[ebp-0x18]
00002797  C1E005            shl eax,byte 0x5
0000279A  89C2              mov edx,eax
0000279C  8B45C0            mov eax,[ebp-0x40]
0000279F  8D0C02            lea ecx,[edx+eax]
000027A2  8B45C8            mov eax,[ebp-0x38]
000027A5  8D5001            lea edx,[eax+0x1]
000027A8  8955C8            mov [ebp-0x38],edx
000027AB  0FB655C6          movzx edx,byte [ebp-0x3a]
000027AF  88540108          mov [ecx+eax+0x8],dl
000027B3  8B55CC            mov edx,[ebp-0x34]
000027B6  8B450C            mov eax,[ebp+0xc]
000027B9  01D0              add eax,edx
000027BB  0FB600            movzx eax,byte [eax]
000027BE  84C0              test al,al
000027C0  7406              jz 0x27c8
000027C2  837DC802          cmp dword [ebp-0x38],byte +0x2
000027C6  7EA0              jng 0x2768
000027C8  83EC08            sub esp,byte +0x8
000027CB  68801C0200        push dword 0x21c80
000027D0  FF75EC            push dword [ebp-0x14]
000027D3  E8D0360000        call 0x5ea8
000027D8  83C410            add esp,byte +0x10
000027DB  B801000000        mov eax,0x1
000027E0  EB49              jmp short 0x282b
000027E2  90                nop
000027E3  8345E801          add dword [ebp-0x18],byte +0x1
000027E7  837DE80F          cmp dword [ebp-0x18],byte +0xf
000027EB  0F8EAEFCFFFF      jng near 0x249f
000027F1  0FB705641C0200    movzx eax,word [dword 0x21c64]
000027F8  6685C0            test ax,ax
000027FB  0F8437FCFFFF      jz near 0x2438
00002801  0FB745F6          movzx eax,word [ebp-0xa]
00002805  83EC0C            sub esp,byte +0xc
00002808  50                push eax
00002809  E871E4FFFF        call 0xc7f
0000280E  83C410            add esp,byte +0x10
00002811  668945F6          mov [ebp-0xa],ax
00002815  66817DF6F70F      cmp word [ebp-0xa],0xff7
0000281B  7708              ja 0x2825
0000281D  E916FCFFFF        jmp 0x2438
00002822  90                nop
00002823  EB01              jmp short 0x2826
00002825  90                nop
00002826  B800000000        mov eax,0x0
0000282B  C9                leave
0000282C  C3                ret
0000282D  55                push ebp
0000282E  89E5              mov ebp,esp
00002830  83EC48            sub esp,byte +0x48
00002833  A1601C0200        mov eax,[0x21c60]
00002838  85C0              test eax,eax
0000283A  7505              jnz 0x2841
0000283C  E824E5FFFF        call 0xd65
00002841  0FB705641C0200    movzx eax,word [dword 0x21c64]
00002848  668945F6          mov [ebp-0xa],ax
0000284C  C745F000000000    mov dword [ebp-0x10],0x0
00002853  0FB705641C0200    movzx eax,word [dword 0x21c64]
0000285A  6685C0            test ax,ax
0000285D  7532              jnz 0x2891
0000285F  8B55F0            mov edx,[ebp-0x10]
00002862  A160350200        mov eax,[0x23560]
00002867  39C2              cmp edx,eax
00002869  0F8373030000      jnc near 0x2be2
0000286F  8B156C350200      mov edx,[dword 0x2356c]
00002875  8B45F0            mov eax,[ebp-0x10]
00002878  01D0              add eax,edx
0000287A  83EC08            sub esp,byte +0x8
0000287D  68801C0200        push dword 0x21c80
00002882  50                push eax
00002883  E875350000        call 0x5dfd
00002888  83C410            add esp,byte +0x10
0000288B  8345F001          add dword [ebp-0x10],byte +0x1
0000288F  EB24              jmp short 0x28b5
00002891  0FB755F6          movzx edx,word [ebp-0xa]
00002895  A164350200        mov eax,[0x23564]
0000289A  01D0              add eax,edx
0000289C  83E802            sub eax,byte +0x2
0000289F  8945CC            mov [ebp-0x34],eax
000028A2  83EC08            sub esp,byte +0x8
000028A5  68801C0200        push dword 0x21c80
000028AA  FF75CC            push dword [ebp-0x34]
000028AD  E84B350000        call 0x5dfd
000028B2  83C410            add esp,byte +0x10
000028B5  C745C8801C0200    mov dword [ebp-0x38],0x21c80
000028BC  C745EC00000000    mov dword [ebp-0x14],0x0
000028C3  E9DF020000        jmp 0x2ba7
000028C8  8B45EC            mov eax,[ebp-0x14]
000028CB  C1E005            shl eax,byte 0x5
000028CE  89C2              mov edx,eax
000028D0  8B45C8            mov eax,[ebp-0x38]
000028D3  01D0              add eax,edx
000028D5  0FB600            movzx eax,byte [eax]
000028D8  84C0              test al,al
000028DA  750A              jnz 0x28e6
000028DC  B800000000        mov eax,0x0
000028E1  E905030000        jmp 0x2beb
000028E6  8B45EC            mov eax,[ebp-0x14]
000028E9  C1E005            shl eax,byte 0x5
000028EC  89C2              mov edx,eax
000028EE  8B45C8            mov eax,[ebp-0x38]
000028F1  01D0              add eax,edx
000028F3  0FB600            movzx eax,byte [eax]
000028F6  3CE5              cmp al,0xe5
000028F8  0F84A4020000      jz near 0x2ba2
000028FE  C745E800000000    mov dword [ebp-0x18],0x0
00002905  C745E400000000    mov dword [ebp-0x1c],0x0
0000290C  EB40              jmp short 0x294e
0000290E  8B45EC            mov eax,[ebp-0x14]
00002911  C1E005            shl eax,byte 0x5
00002914  89C2              mov edx,eax
00002916  8B45C8            mov eax,[ebp-0x38]
00002919  01C2              add edx,eax
0000291B  8B45E4            mov eax,[ebp-0x1c]
0000291E  01D0              add eax,edx
00002920  0FB600            movzx eax,byte [eax]
00002923  3C20              cmp al,0x20
00002925  742F              jz 0x2956
00002927  8B45EC            mov eax,[ebp-0x14]
0000292A  C1E005            shl eax,byte 0x5
0000292D  89C2              mov edx,eax
0000292F  8B45C8            mov eax,[ebp-0x38]
00002932  8D0C02            lea ecx,[edx+eax]
00002935  8B45E8            mov eax,[ebp-0x18]
00002938  8D5001            lea edx,[eax+0x1]
0000293B  8955E8            mov [ebp-0x18],edx
0000293E  8B55E4            mov edx,[ebp-0x1c]
00002941  01CA              add edx,ecx
00002943  0FB612            movzx edx,byte [edx]
00002946  885405B9          mov [ebp+eax-0x47],dl
0000294A  8345E401          add dword [ebp-0x1c],byte +0x1
0000294E  837DE407          cmp dword [ebp-0x1c],byte +0x7
00002952  7EBA              jng 0x290e
00002954  EB01              jmp short 0x2957
00002956  90                nop
00002957  8B45EC            mov eax,[ebp-0x14]
0000295A  C1E005            shl eax,byte 0x5
0000295D  89C2              mov edx,eax
0000295F  8B45C8            mov eax,[ebp-0x38]
00002962  01D0              add eax,edx
00002964  0FB64008          movzx eax,byte [eax+0x8]
00002968  3C20              cmp al,0x20
0000296A  7466              jz 0x29d2
0000296C  8B45E8            mov eax,[ebp-0x18]
0000296F  8D5001            lea edx,[eax+0x1]
00002972  8955E8            mov [ebp-0x18],edx
00002975  C64405B92E        mov byte [ebp+eax-0x47],0x2e
0000297A  C745E000000000    mov dword [ebp-0x20],0x0
00002981  EB46              jmp short 0x29c9
00002983  8B45EC            mov eax,[ebp-0x14]
00002986  C1E005            shl eax,byte 0x5
00002989  89C2              mov edx,eax
0000298B  8B45C8            mov eax,[ebp-0x38]
0000298E  01C2              add edx,eax
00002990  8B45E0            mov eax,[ebp-0x20]
00002993  01D0              add eax,edx
00002995  83C008            add eax,byte +0x8
00002998  0FB600            movzx eax,byte [eax]
0000299B  3C20              cmp al,0x20
0000299D  7432              jz 0x29d1
0000299F  8B45EC            mov eax,[ebp-0x14]
000029A2  C1E005            shl eax,byte 0x5
000029A5  89C2              mov edx,eax
000029A7  8B45C8            mov eax,[ebp-0x38]
000029AA  8D0C02            lea ecx,[edx+eax]
000029AD  8B45E8            mov eax,[ebp-0x18]
000029B0  8D5001            lea edx,[eax+0x1]
000029B3  8955E8            mov [ebp-0x18],edx
000029B6  8B55E0            mov edx,[ebp-0x20]
000029B9  01CA              add edx,ecx
000029BB  83C208            add edx,byte +0x8
000029BE  0FB612            movzx edx,byte [edx]
000029C1  885405B9          mov [ebp+eax-0x47],dl
000029C5  8345E001          add dword [ebp-0x20],byte +0x1
000029C9  837DE002          cmp dword [ebp-0x20],byte +0x2
000029CD  7EB4              jng 0x2983
000029CF  EB01              jmp short 0x29d2
000029D1  90                nop
000029D2  8D55B9            lea edx,[ebp-0x47]
000029D5  8B45E8            mov eax,[ebp-0x18]
000029D8  01D0              add eax,edx
000029DA  C60000            mov byte [eax],0x0
000029DD  C745DC01000000    mov dword [ebp-0x24],0x1
000029E4  8B4508            mov eax,[ebp+0x8]
000029E7  8945D8            mov [ebp-0x28],eax
000029EA  8D45B9            lea eax,[ebp-0x47]
000029ED  8945D4            mov [ebp-0x2c],eax
000029F0  EB6A              jmp short 0x2a5c
000029F2  8B45D8            mov eax,[ebp-0x28]
000029F5  0FB600            movzx eax,byte [eax]
000029F8  3C60              cmp al,0x60
000029FA  7E15              jng 0x2a11
000029FC  8B45D8            mov eax,[ebp-0x28]
000029FF  0FB600            movzx eax,byte [eax]
00002A02  3C7A              cmp al,0x7a
00002A04  7F0B              jg 0x2a11
00002A06  8B45D8            mov eax,[ebp-0x28]
00002A09  0FB600            movzx eax,byte [eax]
00002A0C  83E820            sub eax,byte +0x20
00002A0F  EB06              jmp short 0x2a17
00002A11  8B45D8            mov eax,[ebp-0x28]
00002A14  0FB600            movzx eax,byte [eax]
00002A17  8845C7            mov [ebp-0x39],al
00002A1A  8B45D4            mov eax,[ebp-0x2c]
00002A1D  0FB600            movzx eax,byte [eax]
00002A20  3C60              cmp al,0x60
00002A22  7E15              jng 0x2a39
00002A24  8B45D4            mov eax,[ebp-0x2c]
00002A27  0FB600            movzx eax,byte [eax]
00002A2A  3C7A              cmp al,0x7a
00002A2C  7F0B              jg 0x2a39
00002A2E  8B45D4            mov eax,[ebp-0x2c]
00002A31  0FB600            movzx eax,byte [eax]
00002A34  83E820            sub eax,byte +0x20
00002A37  EB06              jmp short 0x2a3f
00002A39  8B45D4            mov eax,[ebp-0x2c]
00002A3C  0FB600            movzx eax,byte [eax]
00002A3F  8845C6            mov [ebp-0x3a],al
00002A42  0FB645C7          movzx eax,byte [ebp-0x39]
00002A46  3A45C6            cmp al,[ebp-0x3a]
00002A49  7409              jz 0x2a54
00002A4B  C745DC00000000    mov dword [ebp-0x24],0x0
00002A52  EB1C              jmp short 0x2a70
00002A54  8345D801          add dword [ebp-0x28],byte +0x1
00002A58  8345D401          add dword [ebp-0x2c],byte +0x1
00002A5C  8B45D8            mov eax,[ebp-0x28]
00002A5F  0FB600            movzx eax,byte [eax]
00002A62  84C0              test al,al
00002A64  740A              jz 0x2a70
00002A66  8B45D4            mov eax,[ebp-0x2c]
00002A69  0FB600            movzx eax,byte [eax]
00002A6C  84C0              test al,al
00002A6E  7582              jnz 0x29f2
00002A70  837DDC00          cmp dword [ebp-0x24],byte +0x0
00002A74  0F8429010000      jz near 0x2ba3
00002A7A  8B45D8            mov eax,[ebp-0x28]
00002A7D  0FB600            movzx eax,byte [eax]
00002A80  84C0              test al,al
00002A82  0F851B010000      jnz near 0x2ba3
00002A88  8B45D4            mov eax,[ebp-0x2c]
00002A8B  0FB600            movzx eax,byte [eax]
00002A8E  84C0              test al,al
00002A90  0F850D010000      jnz near 0x2ba3
00002A96  8B45EC            mov eax,[ebp-0x14]
00002A99  C1E005            shl eax,byte 0x5
00002A9C  89C2              mov edx,eax
00002A9E  8B45C8            mov eax,[ebp-0x38]
00002AA1  01D0              add eax,edx
00002AA3  0FB6400B          movzx eax,byte [eax+0xb]
00002AA7  0FB6C0            movzx eax,al
00002AAA  83E010            and eax,byte +0x10
00002AAD  85C0              test eax,eax
00002AAF  0F84D6000000      jz near 0x2b8b
00002AB5  8B45EC            mov eax,[ebp-0x14]
00002AB8  C1E005            shl eax,byte 0x5
00002ABB  89C2              mov edx,eax
00002ABD  8B45C8            mov eax,[ebp-0x38]
00002AC0  01D0              add eax,edx
00002AC2  0FB7401A          movzx eax,word [eax+0x1a]
00002AC6  66A3641C0200      mov [0x21c64],ax
00002ACC  8B4508            mov eax,[ebp+0x8]
00002ACF  0FB600            movzx eax,byte [eax]
00002AD2  3C2E              cmp al,0x2e
00002AD4  7562              jnz 0x2b38
00002AD6  8B4508            mov eax,[ebp+0x8]
00002AD9  83C001            add eax,byte +0x1
00002ADC  0FB600            movzx eax,byte [eax]
00002ADF  3C2E              cmp al,0x2e
00002AE1  7555              jnz 0x2b38
00002AE3  8B4508            mov eax,[ebp+0x8]
00002AE6  83C002            add eax,byte +0x2
00002AE9  0FB600            movzx eax,byte [eax]
00002AEC  84C0              test al,al
00002AEE  7548              jnz 0x2b38
00002AF0  83EC0C            sub esp,byte +0xc
00002AF3  68C0DA0100        push dword 0x1dac0
00002AF8  E89C300000        call 0x5b99
00002AFD  83C410            add esp,byte +0x10
00002B00  8945D0            mov [ebp-0x30],eax
00002B03  EB04              jmp short 0x2b09
00002B05  836DD001          sub dword [ebp-0x30],byte +0x1
00002B09  837DD000          cmp dword [ebp-0x30],byte +0x0
00002B0D  7E0F              jng 0x2b1e
00002B0F  8B45D0            mov eax,[ebp-0x30]
00002B12  05C0DA0100        add eax,0x1dac0
00002B17  0FB600            movzx eax,byte [eax]
00002B1A  3C2F              cmp al,0x2f
00002B1C  75E7              jnz 0x2b05
00002B1E  837DD000          cmp dword [ebp-0x30],byte +0x0
00002B22  7507              jnz 0x2b2b
00002B24  C745D001000000    mov dword [ebp-0x30],0x1
00002B2B  8B45D0            mov eax,[ebp-0x30]
00002B2E  05C0DA0100        add eax,0x1dac0
00002B33  C60000            mov byte [eax],0x0
00002B36  EB4C              jmp short 0x2b84
00002B38  8B4508            mov eax,[ebp+0x8]
00002B3B  0FB600            movzx eax,byte [eax]
00002B3E  3C2E              cmp al,0x2e
00002B40  7442              jz 0x2b84
00002B42  83EC08            sub esp,byte +0x8
00002B45  680AB70100        push dword 0x1b70a
00002B4A  68C0DA0100        push dword 0x1dac0
00002B4F  E806300000        call 0x5b5a
00002B54  83C410            add esp,byte +0x10
00002B57  85C0              test eax,eax
00002B59  7415              jz 0x2b70
00002B5B  83EC08            sub esp,byte +0x8
00002B5E  680AB70100        push dword 0x1b70a
00002B63  68C0DA0100        push dword 0x1dac0
00002B68  E83D310000        call 0x5caa
00002B6D  83C410            add esp,byte +0x10
00002B70  83EC08            sub esp,byte +0x8
00002B73  8D45B9            lea eax,[ebp-0x47]
00002B76  50                push eax
00002B77  68C0DA0100        push dword 0x1dac0
00002B7C  E829310000        call 0x5caa
00002B81  83C410            add esp,byte +0x10
00002B84  B801000000        mov eax,0x1
00002B89  EB60              jmp short 0x2beb
00002B8B  83EC0C            sub esp,byte +0xc
00002B8E  680CB70100        push dword 0x1b70c
00002B93  E892240000        call 0x502a
00002B98  83C410            add esp,byte +0x10
00002B9B  B800000000        mov eax,0x0
00002BA0  EB49              jmp short 0x2beb
00002BA2  90                nop
00002BA3  8345EC01          add dword [ebp-0x14],byte +0x1
00002BA7  837DEC0F          cmp dword [ebp-0x14],byte +0xf
00002BAB  0F8E17FDFFFF      jng near 0x28c8
00002BB1  0FB705641C0200    movzx eax,word [dword 0x21c64]
00002BB8  6685C0            test ax,ax
00002BBB  0F8492FCFFFF      jz near 0x2853
00002BC1  0FB745F6          movzx eax,word [ebp-0xa]
00002BC5  83EC0C            sub esp,byte +0xc
00002BC8  50                push eax
00002BC9  E8B1E0FFFF        call 0xc7f
00002BCE  83C410            add esp,byte +0x10
00002BD1  668945F6          mov [ebp-0xa],ax
00002BD5  66817DF6F70F      cmp word [ebp-0xa],0xff7
00002BDB  7708              ja 0x2be5
00002BDD  E971FCFFFF        jmp 0x2853
00002BE2  90                nop
00002BE3  EB01              jmp short 0x2be6
00002BE5  90                nop
00002BE6  B800000000        mov eax,0x0
00002BEB  C9                leave
00002BEC  C3                ret
00002BED  55                push ebp
00002BEE  89E5              mov ebp,esp
00002BF0  83EC48            sub esp,byte +0x48
00002BF3  A1601C0200        mov eax,[0x21c60]
00002BF8  85C0              test eax,eax
00002BFA  7505              jnz 0x2c01
00002BFC  E864E1FFFF        call 0xd65
00002C01  66C745F60000      mov word [ebp-0xa],0x0
00002C07  A168350200        mov eax,[0x23568]
00002C0C  83EC08            sub esp,byte +0x8
00002C0F  68801C0200        push dword 0x21c80
00002C14  50                push eax
00002C15  E8E3310000        call 0x5dfd
00002C1A  83C410            add esp,byte +0x10
00002C1D  C745F002000000    mov dword [ebp-0x10],0x2
00002C24  E992000000        jmp 0x2cbb
00002C29  8B45F0            mov eax,[ebp-0x10]
00002C2C  89C2              mov edx,eax
00002C2E  C1EA1F            shr edx,byte 0x1f
00002C31  01D0              add eax,edx
00002C33  D1F8              sar eax,1
00002C35  89C2              mov edx,eax
00002C37  8B45F0            mov eax,[ebp-0x10]
00002C3A  01D0              add eax,edx
00002C3C  8945D0            mov [ebp-0x30],eax
00002C3F  8B45F0            mov eax,[ebp-0x10]
00002C42  83E001            and eax,byte +0x1
00002C45  85C0              test eax,eax
00002C47  742C              jz 0x2c75
00002C49  8B45D0            mov eax,[ebp-0x30]
00002C4C  05801C0200        add eax,0x21c80
00002C51  0FB600            movzx eax,byte [eax]
00002C54  C0E804            shr al,byte 0x4
00002C57  0FB6C0            movzx eax,al
00002C5A  8B55D0            mov edx,[ebp-0x30]
00002C5D  83C201            add edx,byte +0x1
00002C60  0FB692801C0200    movzx edx,byte [edx+0x21c80]
00002C67  0FB6D2            movzx edx,dl
00002C6A  C1E204            shl edx,byte 0x4
00002C6D  09D0              or eax,edx
00002C6F  668945EE          mov [ebp-0x12],ax
00002C73  EB2C              jmp short 0x2ca1
00002C75  8B45D0            mov eax,[ebp-0x30]
00002C78  05801C0200        add eax,0x21c80
00002C7D  0FB600            movzx eax,byte [eax]
00002C80  0FB6C0            movzx eax,al
00002C83  8B55D0            mov edx,[ebp-0x30]
00002C86  83C201            add edx,byte +0x1
00002C89  0FB692801C0200    movzx edx,byte [edx+0x21c80]
00002C90  0FB6D2            movzx edx,dl
00002C93  C1E208            shl edx,byte 0x8
00002C96  6681E2000F        and dx,0xf00
00002C9B  09D0              or eax,edx
00002C9D  668945EE          mov [ebp-0x12],ax
00002CA1  0FB745EE          movzx eax,word [ebp-0x12]
00002CA5  25FF0F0000        and eax,0xfff
00002CAA  85C0              test eax,eax
00002CAC  7509              jnz 0x2cb7
00002CAE  8B45F0            mov eax,[ebp-0x10]
00002CB1  668945F6          mov [ebp-0xa],ax
00002CB5  EB0E              jmp short 0x2cc5
00002CB7  8345F001          add dword [ebp-0x10],byte +0x1
00002CBB  837DF07F          cmp dword [ebp-0x10],byte +0x7f
00002CBF  0F8E64FFFFFF      jng near 0x2c29
00002CC5  66837DF600        cmp word [ebp-0xa],byte +0x0
00002CCA  751A              jnz 0x2ce6
00002CCC  83EC0C            sub esp,byte +0xc
00002CCF  68FAB60100        push dword 0x1b6fa
00002CD4  E851230000        call 0x502a
00002CD9  83C410            add esp,byte +0x10
00002CDC  B800000000        mov eax,0x0
00002CE1  E9AB020000        jmp 0x2f91
00002CE6  83EC04            sub esp,byte +0x4
00002CE9  6800020000        push dword 0x200
00002CEE  6A00              push byte +0x0
00002CF0  68C0220200        push dword 0x222c0
00002CF5  E8C62E0000        call 0x5bc0
00002CFA  83C410            add esp,byte +0x10
00002CFD  C745CCC0220200    mov dword [ebp-0x34],0x222c0
00002D04  8B45CC            mov eax,[ebp-0x34]
00002D07  83EC04            sub esp,byte +0x4
00002D0A  6A0B              push byte +0xb
00002D0C  6A20              push byte +0x20
00002D0E  50                push eax
00002D0F  E8AC2E0000        call 0x5bc0
00002D14  83C410            add esp,byte +0x10
00002D17  8B45CC            mov eax,[ebp-0x34]
00002D1A  C6002E            mov byte [eax],0x2e
00002D1D  8B45CC            mov eax,[ebp-0x34]
00002D20  C6400B10          mov byte [eax+0xb],0x10
00002D24  8B45CC            mov eax,[ebp-0x34]
00002D27  0FB755F6          movzx edx,word [ebp-0xa]
00002D2B  6689501A          mov [eax+0x1a],dx
00002D2F  8B45CC            mov eax,[ebp-0x34]
00002D32  83C020            add eax,byte +0x20
00002D35  83EC04            sub esp,byte +0x4
00002D38  6A0B              push byte +0xb
00002D3A  6A20              push byte +0x20
00002D3C  50                push eax
00002D3D  E87E2E0000        call 0x5bc0
00002D42  83C410            add esp,byte +0x10
00002D45  8B45CC            mov eax,[ebp-0x34]
00002D48  83C020            add eax,byte +0x20
00002D4B  C6002E            mov byte [eax],0x2e
00002D4E  8B45CC            mov eax,[ebp-0x34]
00002D51  83C020            add eax,byte +0x20
00002D54  C640012E          mov byte [eax+0x1],0x2e
00002D58  8B45CC            mov eax,[ebp-0x34]
00002D5B  83C020            add eax,byte +0x20
00002D5E  C6400B10          mov byte [eax+0xb],0x10
00002D62  8B45CC            mov eax,[ebp-0x34]
00002D65  8D5020            lea edx,[eax+0x20]
00002D68  0FB705641C0200    movzx eax,word [dword 0x21c64]
00002D6F  6689421A          mov [edx+0x1a],ax
00002D73  0FB755F6          movzx edx,word [ebp-0xa]
00002D77  A164350200        mov eax,[0x23564]
00002D7C  01D0              add eax,edx
00002D7E  83E802            sub eax,byte +0x2
00002D81  8945C8            mov [ebp-0x38],eax
00002D84  83EC08            sub esp,byte +0x8
00002D87  68C0220200        push dword 0x222c0
00002D8C  FF75C8            push dword [ebp-0x38]
00002D8F  E814310000        call 0x5ea8
00002D94  83C410            add esp,byte +0x10
00002D97  0FB745F6          movzx eax,word [ebp-0xa]
00002D9B  83EC08            sub esp,byte +0x8
00002D9E  68FF0F0000        push dword 0xfff
00002DA3  50                push eax
00002DA4  E814EAFFFF        call 0x17bd
00002DA9  83C410            add esp,byte +0x10
00002DAC  0FB705641C0200    movzx eax,word [dword 0x21c64]
00002DB3  668945EC          mov [ebp-0x14],ax
00002DB7  C745E800000000    mov dword [ebp-0x18],0x0
00002DBE  C745C400000000    mov dword [ebp-0x3c],0x0
00002DC5  0FB705641C0200    movzx eax,word [dword 0x21c64]
00002DCC  6685C0            test ax,ax
00002DCF  7520              jnz 0x2df1
00002DD1  8B55E8            mov edx,[ebp-0x18]
00002DD4  A160350200        mov eax,[0x23560]
00002DD9  39C2              cmp edx,eax
00002DDB  0F8397010000      jnc near 0x2f78
00002DE1  8B156C350200      mov edx,[dword 0x2356c]
00002DE7  8B45E8            mov eax,[ebp-0x18]
00002DEA  01D0              add eax,edx
00002DEC  8945E4            mov [ebp-0x1c],eax
00002DEF  EB11              jmp short 0x2e02
00002DF1  0FB755EC          movzx edx,word [ebp-0x14]
00002DF5  A164350200        mov eax,[0x23564]
00002DFA  01D0              add eax,edx
00002DFC  83E802            sub eax,byte +0x2
00002DFF  8945E4            mov [ebp-0x1c],eax
00002E02  83EC08            sub esp,byte +0x8
00002E05  68801C0200        push dword 0x21c80
00002E0A  FF75E4            push dword [ebp-0x1c]
00002E0D  E8EB2F0000        call 0x5dfd
00002E12  83C410            add esp,byte +0x10
00002E15  C745C0801C0200    mov dword [ebp-0x40],0x21c80
00002E1C  C745E000000000    mov dword [ebp-0x20],0x0
00002E23  E910010000        jmp 0x2f38
00002E28  8B45E0            mov eax,[ebp-0x20]
00002E2B  C1E005            shl eax,byte 0x5
00002E2E  89C2              mov edx,eax
00002E30  8B45C0            mov eax,[ebp-0x40]
00002E33  01D0              add eax,edx
00002E35  0FB600            movzx eax,byte [eax]
00002E38  84C0              test al,al
00002E3A  7418              jz 0x2e54
00002E3C  8B45E0            mov eax,[ebp-0x20]
00002E3F  C1E005            shl eax,byte 0x5
00002E42  89C2              mov edx,eax
00002E44  8B45C0            mov eax,[ebp-0x40]
00002E47  01D0              add eax,edx
00002E49  0FB600            movzx eax,byte [eax]
00002E4C  3CE5              cmp al,0xe5
00002E4E  0F85E0000000      jnz near 0x2f34
00002E54  8B45E0            mov eax,[ebp-0x20]
00002E57  C1E005            shl eax,byte 0x5
00002E5A  89C2              mov edx,eax
00002E5C  8B45C0            mov eax,[ebp-0x40]
00002E5F  01D0              add eax,edx
00002E61  83EC04            sub esp,byte +0x4
00002E64  6A0B              push byte +0xb
00002E66  6A20              push byte +0x20
00002E68  50                push eax
00002E69  E8522D0000        call 0x5bc0
00002E6E  83C410            add esp,byte +0x10
00002E71  C745DC00000000    mov dword [ebp-0x24],0x0
00002E78  C745D800000000    mov dword [ebp-0x28],0x0
00002E7F  EB4A              jmp short 0x2ecb
00002E81  8B45DC            mov eax,[ebp-0x24]
00002E84  8D5001            lea edx,[eax+0x1]
00002E87  8955DC            mov [ebp-0x24],edx
00002E8A  89C2              mov edx,eax
00002E8C  8B4508            mov eax,[ebp+0x8]
00002E8F  01D0              add eax,edx
00002E91  0FB600            movzx eax,byte [eax]
00002E94  8845D7            mov [ebp-0x29],al
00002E97  807DD760          cmp byte [ebp-0x29],0x60
00002E9B  7E10              jng 0x2ead
00002E9D  807DD77A          cmp byte [ebp-0x29],0x7a
00002EA1  7F0A              jg 0x2ead
00002EA3  0FB645D7          movzx eax,byte [ebp-0x29]
00002EA7  83E820            sub eax,byte +0x20
00002EAA  8845D7            mov [ebp-0x29],al
00002EAD  8B45E0            mov eax,[ebp-0x20]
00002EB0  C1E005            shl eax,byte 0x5
00002EB3  89C2              mov edx,eax
00002EB5  8B45C0            mov eax,[ebp-0x40]
00002EB8  8D0C02            lea ecx,[edx+eax]
00002EBB  8B45D8            mov eax,[ebp-0x28]
00002EBE  8D5001            lea edx,[eax+0x1]
00002EC1  8955D8            mov [ebp-0x28],edx
00002EC4  0FB655D7          movzx edx,byte [ebp-0x29]
00002EC8  881401            mov [ecx+eax],dl
00002ECB  8B55DC            mov edx,[ebp-0x24]
00002ECE  8B4508            mov eax,[ebp+0x8]
00002ED1  01D0              add eax,edx
00002ED3  0FB600            movzx eax,byte [eax]
00002ED6  84C0              test al,al
00002ED8  7406              jz 0x2ee0
00002EDA  837DD807          cmp dword [ebp-0x28],byte +0x7
00002EDE  7EA1              jng 0x2e81
00002EE0  8B45E0            mov eax,[ebp-0x20]
00002EE3  C1E005            shl eax,byte 0x5
00002EE6  89C2              mov edx,eax
00002EE8  8B45C0            mov eax,[ebp-0x40]
00002EEB  01D0              add eax,edx
00002EED  C6400B10          mov byte [eax+0xb],0x10
00002EF1  8B45E0            mov eax,[ebp-0x20]
00002EF4  C1E005            shl eax,byte 0x5
00002EF7  89C2              mov edx,eax
00002EF9  8B45C0            mov eax,[ebp-0x40]
00002EFC  01C2              add edx,eax
00002EFE  0FB745F6          movzx eax,word [ebp-0xa]
00002F02  6689421A          mov [edx+0x1a],ax
00002F06  8B45E0            mov eax,[ebp-0x20]
00002F09  C1E005            shl eax,byte 0x5
00002F0C  89C2              mov edx,eax
00002F0E  8B45C0            mov eax,[ebp-0x40]
00002F11  01D0              add eax,edx
00002F13  C7401C00000000    mov dword [eax+0x1c],0x0
00002F1A  83EC08            sub esp,byte +0x8
00002F1D  68801C0200        push dword 0x21c80
00002F22  FF75E4            push dword [ebp-0x1c]
00002F25  E87E2F0000        call 0x5ea8
00002F2A  83C410            add esp,byte +0x10
00002F2D  B801000000        mov eax,0x1
00002F32  EB5D              jmp short 0x2f91
00002F34  8345E001          add dword [ebp-0x20],byte +0x1
00002F38  837DE00F          cmp dword [ebp-0x20],byte +0xf
00002F3C  0F8EE6FEFFFF      jng near 0x2e28
00002F42  0FB705641C0200    movzx eax,word [dword 0x21c64]
00002F49  6685C0            test ax,ax
00002F4C  7509              jnz 0x2f57
00002F4E  8345E801          add dword [ebp-0x18],byte +0x1
00002F52  E96EFEFFFF        jmp 0x2dc5
00002F57  0FB745EC          movzx eax,word [ebp-0x14]
00002F5B  83EC0C            sub esp,byte +0xc
00002F5E  50                push eax
00002F5F  E81BDDFFFF        call 0xc7f
00002F64  83C410            add esp,byte +0x10
00002F67  668945EC          mov [ebp-0x14],ax
00002F6B  66817DECF70F      cmp word [ebp-0x14],0xff7
00002F71  7708              ja 0x2f7b
00002F73  E94DFEFFFF        jmp 0x2dc5
00002F78  90                nop
00002F79  EB01              jmp short 0x2f7c
00002F7B  90                nop
00002F7C  83EC0C            sub esp,byte +0xc
00002F7F  68E5B60100        push dword 0x1b6e5
00002F84  E8A1200000        call 0x502a
00002F89  83C410            add esp,byte +0x10
00002F8C  B800000000        mov eax,0x0
00002F91  C9                leave
00002F92  C3                ret
00002F93  55                push ebp
00002F94  89E5              mov ebp,esp
00002F96  83EC04            sub esp,byte +0x4
00002F99  8B4510            mov eax,[ebp+0x10]
00002F9C  8845FC            mov [ebp-0x4],al
00002F9F  837D0800          cmp dword [ebp+0x8],byte +0x0
00002FA3  783B              js 0x2fe0
00002FA5  817D083F010000    cmp dword [ebp+0x8],0x13f
00002FAC  7F32              jg 0x2fe0
00002FAE  837D0C00          cmp dword [ebp+0xc],byte +0x0
00002FB2  782C              js 0x2fe0
00002FB4  817D0CC7000000    cmp dword [ebp+0xc],0xc7
00002FBB  7F23              jg 0x2fe0
00002FBD  8B0DC0DB0100      mov ecx,[dword 0x1dbc0]
00002FC3  8B550C            mov edx,[ebp+0xc]
00002FC6  89D0              mov eax,edx
00002FC8  C1E002            shl eax,byte 0x2
00002FCB  01D0              add eax,edx
00002FCD  C1E006            shl eax,byte 0x6
00002FD0  89C2              mov edx,eax
00002FD2  8B4508            mov eax,[ebp+0x8]
00002FD5  01D0              add eax,edx
00002FD7  8D1401            lea edx,[ecx+eax]
00002FDA  0FB645FC          movzx eax,byte [ebp-0x4]
00002FDE  8802              mov [edx],al
00002FE0  90                nop
00002FE1  C9                leave
00002FE2  C3                ret
00002FE3  55                push ebp
00002FE4  89E5              mov ebp,esp
00002FE6  83EC14            sub esp,byte +0x14
00002FE9  8B4518            mov eax,[ebp+0x18]
00002FEC  8845EC            mov [ebp-0x14],al
00002FEF  8B450C            mov eax,[ebp+0xc]
00002FF2  8945FC            mov [ebp-0x4],eax
00002FF5  EB30              jmp short 0x3027
00002FF7  8B4508            mov eax,[ebp+0x8]
00002FFA  8945F8            mov [ebp-0x8],eax
00002FFD  EB17              jmp short 0x3016
00002FFF  0FB645EC          movzx eax,byte [ebp-0x14]
00003003  50                push eax
00003004  FF75FC            push dword [ebp-0x4]
00003007  FF75F8            push dword [ebp-0x8]
0000300A  E884FFFFFF        call 0x2f93
0000300F  83C40C            add esp,byte +0xc
00003012  8345F801          add dword [ebp-0x8],byte +0x1
00003016  8B5508            mov edx,[ebp+0x8]
00003019  8B4510            mov eax,[ebp+0x10]
0000301C  01D0              add eax,edx
0000301E  3945F8            cmp [ebp-0x8],eax
00003021  7CDC              jl 0x2fff
00003023  8345FC01          add dword [ebp-0x4],byte +0x1
00003027  8B550C            mov edx,[ebp+0xc]
0000302A  8B4514            mov eax,[ebp+0x14]
0000302D  01D0              add eax,edx
0000302F  3945FC            cmp [ebp-0x4],eax
00003032  7CC3              jl 0x2ff7
00003034  90                nop
00003035  C9                leave
00003036  C3                ret
00003037  55                push ebp
00003038  89E5              mov ebp,esp
0000303A  83EC10            sub esp,byte +0x10
0000303D  C745FC00000000    mov dword [ebp-0x4],0x0
00003044  E99E000000        jmp 0x30e7
00003049  C745F800000000    mov dword [ebp-0x8],0x0
00003050  E984000000        jmp 0x30d9
00003055  8B5508            mov edx,[ebp+0x8]
00003058  8B45F8            mov eax,[ebp-0x8]
0000305B  01D0              add eax,edx
0000305D  8945F4            mov [ebp-0xc],eax
00003060  8B550C            mov edx,[ebp+0xc]
00003063  8B45FC            mov eax,[ebp-0x4]
00003066  01D0              add eax,edx
00003068  8945F0            mov [ebp-0x10],eax
0000306B  837DF400          cmp dword [ebp-0xc],byte +0x0
0000306F  784E              js 0x30bf
00003071  817DF43F010000    cmp dword [ebp-0xc],0x13f
00003078  7F45              jg 0x30bf
0000307A  837DF000          cmp dword [ebp-0x10],byte +0x0
0000307E  783F              js 0x30bf
00003080  817DF0C7000000    cmp dword [ebp-0x10],0xc7
00003087  7F36              jg 0x30bf
00003089  8B0DC0DB0100      mov ecx,[dword 0x1dbc0]
0000308F  8B55F0            mov edx,[ebp-0x10]
00003092  89D0              mov eax,edx
00003094  C1E002            shl eax,byte 0x2
00003097  01D0              add eax,edx
00003099  C1E006            shl eax,byte 0x6
0000309C  89C2              mov edx,eax
0000309E  8B45F4            mov eax,[ebp-0xc]
000030A1  01D0              add eax,edx
000030A3  01C8              add eax,ecx
000030A5  8B55FC            mov edx,[ebp-0x4]
000030A8  8D0CD500000000    lea ecx,[edx*8+0x0]
000030AF  8B55F8            mov edx,[ebp-0x8]
000030B2  01CA              add edx,ecx
000030B4  0FB600            movzx eax,byte [eax]
000030B7  8882C0240200      mov [edx+0x224c0],al
000030BD  EB16              jmp short 0x30d5
000030BF  8B45FC            mov eax,[ebp-0x4]
000030C2  8D14C500000000    lea edx,[eax*8+0x0]
000030C9  8B45F8            mov eax,[ebp-0x8]
000030CC  01D0              add eax,edx
000030CE  C680C024020000    mov byte [eax+0x224c0],0x0
000030D5  8345F801          add dword [ebp-0x8],byte +0x1
000030D9  837DF807          cmp dword [ebp-0x8],byte +0x7
000030DD  0F8E72FFFFFF      jng near 0x3055
000030E3  8345FC01          add dword [ebp-0x4],byte +0x1
000030E7  837DFC07          cmp dword [ebp-0x4],byte +0x7
000030EB  0F8E58FFFFFF      jng near 0x3049
000030F1  90                nop
000030F2  C9                leave
000030F3  C3                ret
000030F4  55                push ebp
000030F5  89E5              mov ebp,esp
000030F7  53                push ebx
000030F8  83EC10            sub esp,byte +0x10
000030FB  C745F800000000    mov dword [ebp-0x8],0x0
00003102  E981000000        jmp 0x3188
00003107  C745F400000000    mov dword [ebp-0xc],0x0
0000310E  EB6E              jmp short 0x317e
00003110  8B5508            mov edx,[ebp+0x8]
00003113  8B45F4            mov eax,[ebp-0xc]
00003116  01D0              add eax,edx
00003118  8945F0            mov [ebp-0x10],eax
0000311B  8B550C            mov edx,[ebp+0xc]
0000311E  8B45F8            mov eax,[ebp-0x8]
00003121  01D0              add eax,edx
00003123  8945EC            mov [ebp-0x14],eax
00003126  837DF000          cmp dword [ebp-0x10],byte +0x0
0000312A  784E              js 0x317a
0000312C  817DF03F010000    cmp dword [ebp-0x10],0x13f
00003133  7F45              jg 0x317a
00003135  837DEC00          cmp dword [ebp-0x14],byte +0x0
00003139  783F              js 0x317a
0000313B  817DECC7000000    cmp dword [ebp-0x14],0xc7
00003142  7F36              jg 0x317a
00003144  8B45F8            mov eax,[ebp-0x8]
00003147  8D14C500000000    lea edx,[eax*8+0x0]
0000314E  8B45F4            mov eax,[ebp-0xc]
00003151  8D0C02            lea ecx,[edx+eax]
00003154  8B1DC0DB0100      mov ebx,[dword 0x1dbc0]
0000315A  8B55EC            mov edx,[ebp-0x14]
0000315D  89D0              mov eax,edx
0000315F  C1E002            shl eax,byte 0x2
00003162  01D0              add eax,edx
00003164  C1E006            shl eax,byte 0x6
00003167  89C2              mov edx,eax
00003169  8B45F0            mov eax,[ebp-0x10]
0000316C  01D0              add eax,edx
0000316E  8D1403            lea edx,[ebx+eax]
00003171  0FB681C0240200    movzx eax,byte [ecx+0x224c0]
00003178  8802              mov [edx],al
0000317A  8345F401          add dword [ebp-0xc],byte +0x1
0000317E  837DF407          cmp dword [ebp-0xc],byte +0x7
00003182  7E8C              jng 0x3110
00003184  8345F801          add dword [ebp-0x8],byte +0x1
00003188  837DF807          cmp dword [ebp-0x8],byte +0x7
0000318C  0F8E75FFFFFF      jng near 0x3107
00003192  90                nop
00003193  83C410            add esp,byte +0x10
00003196  5B                pop ebx
00003197  5D                pop ebp
00003198  C3                ret
00003199  55                push ebp
0000319A  89E5              mov ebp,esp
0000319C  83EC10            sub esp,byte +0x10
0000319F  C745FC00000000    mov dword [ebp-0x4],0x0
000031A6  EB4D              jmp short 0x31f5
000031A8  C745F800000000    mov dword [ebp-0x8],0x0
000031AF  EB3A              jmp short 0x31eb
000031B1  8B45FC            mov eax,[ebp-0x4]
000031B4  8D14C500000000    lea edx,[eax*8+0x0]
000031BB  8B45F8            mov eax,[ebp-0x8]
000031BE  01D0              add eax,edx
000031C0  0FB68020B70100    movzx eax,byte [eax+0x1b720]
000031C7  3C0F              cmp al,0xf
000031C9  751C              jnz 0x31e7
000031CB  8B550C            mov edx,[ebp+0xc]
000031CE  8B45FC            mov eax,[ebp-0x4]
000031D1  01C2              add edx,eax
000031D3  8B4D08            mov ecx,[ebp+0x8]
000031D6  8B45F8            mov eax,[ebp-0x8]
000031D9  01C8              add eax,ecx
000031DB  6A0F              push byte +0xf
000031DD  52                push edx
000031DE  50                push eax
000031DF  E8AFFDFFFF        call 0x2f93
000031E4  83C40C            add esp,byte +0xc
000031E7  8345F801          add dword [ebp-0x8],byte +0x1
000031EB  837DF807          cmp dword [ebp-0x8],byte +0x7
000031EF  7EC0              jng 0x31b1
000031F1  8345FC01          add dword [ebp-0x4],byte +0x1
000031F5  837DFC07          cmp dword [ebp-0x4],byte +0x7
000031F9  7EAD              jng 0x31a8
000031FB  90                nop
000031FC  C9                leave
000031FD  C3                ret
000031FE  55                push ebp
000031FF  89E5              mov ebp,esp
00003201  83EC48            sub esp,byte +0x48
00003204  C745F400000000    mov dword [ebp-0xc],0x0
0000320B  EB12              jmp short 0x321f
0000320D  8B15C0DB0100      mov edx,[dword 0x1dbc0]
00003213  8B45F4            mov eax,[ebp-0xc]
00003216  01D0              add eax,edx
00003218  C60001            mov byte [eax],0x1
0000321B  8345F401          add dword [ebp-0xc],byte +0x1
0000321F  817DF4FFF90000    cmp dword [ebp-0xc],0xf9ff
00003226  7EE5              jng 0x320d
00003228  C645B828          mov byte [ebp-0x48],0x28
0000322C  C645B92F          mov byte [ebp-0x47],0x2f
00003230  C645BA01          mov byte [ebp-0x46],0x1
00003234  C645BB0E          mov byte [ebp-0x45],0xe
00003238  C645BC09          mov byte [ebp-0x44],0x9
0000323C  C645BD0D          mov byte [ebp-0x43],0xd
00003240  C645BE06          mov byte [ebp-0x42],0x6
00003244  C645BF0F          mov byte [ebp-0x41],0xf
00003248  C745F000000000    mov dword [ebp-0x10],0x0
0000324F  EB2F              jmp short 0x3280
00003251  8D55B8            lea edx,[ebp-0x48]
00003254  8B45F0            mov eax,[ebp-0x10]
00003257  01D0              add eax,edx
00003259  0FB600            movzx eax,byte [eax]
0000325C  0FB6C8            movzx ecx,al
0000325F  8B55F0            mov edx,[ebp-0x10]
00003262  89D0              mov eax,edx
00003264  C1E002            shl eax,byte 0x2
00003267  01D0              add eax,edx
00003269  C1E003            shl eax,byte 0x3
0000326C  51                push ecx
0000326D  6A14              push byte +0x14
0000326F  6A28              push byte +0x28
00003271  6A00              push byte +0x0
00003273  50                push eax
00003274  E86AFDFFFF        call 0x2fe3
00003279  83C414            add esp,byte +0x14
0000327C  8345F001          add dword [ebp-0x10],byte +0x1
00003280  837DF007          cmp dword [ebp-0x10],byte +0x7
00003284  7ECB              jng 0x3251
00003286  6A0F              push byte +0xf
00003288  6A02              push byte +0x2
0000328A  6840010000        push dword 0x140
0000328F  6A14              push byte +0x14
00003291  6A00              push byte +0x0
00003293  E84BFDFFFF        call 0x2fe3
00003298  83C414            add esp,byte +0x14
0000329B  E813340000        call 0x66b3
000032A0  FB                sti
000032A1  E83D320000        call 0x64e3
000032A6  8945D8            mov [ebp-0x28],eax
000032A9  E83F320000        call 0x64ed
000032AE  8945D4            mov [ebp-0x2c],eax
000032B1  8B45D8            mov eax,[ebp-0x28]
000032B4  8945EC            mov [ebp-0x14],eax
000032B7  8B45D4            mov eax,[ebp-0x2c]
000032BA  8945E8            mov [ebp-0x18],eax
000032BD  C645E728          mov byte [ebp-0x19],0x28
000032C1  83EC08            sub esp,byte +0x8
000032C4  FF75D4            push dword [ebp-0x2c]
000032C7  FF75D8            push dword [ebp-0x28]
000032CA  E868FDFFFF        call 0x3037
000032CF  83C410            add esp,byte +0x10
000032D2  83EC08            sub esp,byte +0x8
000032D5  FF75D4            push dword [ebp-0x2c]
000032D8  FF75D8            push dword [ebp-0x28]
000032DB  E8B9FEFFFF        call 0x3199
000032E0  83C410            add esp,byte +0x10
000032E3  E8FB310000        call 0x64e3
000032E8  8945D0            mov [ebp-0x30],eax
000032EB  E8FD310000        call 0x64ed
000032F0  8945CC            mov [ebp-0x34],eax
000032F3  E8FF310000        call 0x64f7
000032F8  8845CB            mov [ebp-0x35],al
000032FB  0FB645CB          movzx eax,byte [ebp-0x35]
000032FF  83E001            and eax,byte +0x1
00003302  8945C4            mov [ebp-0x3c],eax
00003305  8B45D0            mov eax,[ebp-0x30]
00003308  3B45EC            cmp eax,[ebp-0x14]
0000330B  7508              jnz 0x3315
0000330D  8B45CC            mov eax,[ebp-0x34]
00003310  3B45E8            cmp eax,[ebp-0x18]
00003313  743F              jz 0x3354
00003315  83EC08            sub esp,byte +0x8
00003318  FF75E8            push dword [ebp-0x18]
0000331B  FF75EC            push dword [ebp-0x14]
0000331E  E8D1FDFFFF        call 0x30f4
00003323  83C410            add esp,byte +0x10
00003326  83EC08            sub esp,byte +0x8
00003329  FF75CC            push dword [ebp-0x34]
0000332C  FF75D0            push dword [ebp-0x30]
0000332F  E803FDFFFF        call 0x3037
00003334  83C410            add esp,byte +0x10
00003337  83EC08            sub esp,byte +0x8
0000333A  FF75CC            push dword [ebp-0x34]
0000333D  FF75D0            push dword [ebp-0x30]
00003340  E854FEFFFF        call 0x3199
00003345  83C410            add esp,byte +0x10
00003348  8B45D0            mov eax,[ebp-0x30]
0000334B  8945EC            mov [ebp-0x14],eax
0000334E  8B45CC            mov eax,[ebp-0x34]
00003351  8945E8            mov [ebp-0x18],eax
00003354  837DC400          cmp dword [ebp-0x3c],byte +0x0
00003358  7489              jz 0x32e3
0000335A  837DCC15          cmp dword [ebp-0x34],byte +0x15
0000335E  7F42              jg 0x33a2
00003360  8B4DD0            mov ecx,[ebp-0x30]
00003363  BA67666666        mov edx,0x66666667
00003368  89C8              mov eax,ecx
0000336A  F7EA              imul edx
0000336C  C1FA04            sar edx,byte 0x4
0000336F  89C8              mov eax,ecx
00003371  C1F81F            sar eax,byte 0x1f
00003374  29C2              sub edx,eax
00003376  89D0              mov eax,edx
00003378  8945C0            mov [ebp-0x40],eax
0000337B  837DC000          cmp dword [ebp-0x40],byte +0x0
0000337F  0F885EFFFFFF      js near 0x32e3
00003385  837DC007          cmp dword [ebp-0x40],byte +0x7
00003389  0F8F54FFFFFF      jg near 0x32e3
0000338F  8D55B8            lea edx,[ebp-0x48]
00003392  8B45C0            mov eax,[ebp-0x40]
00003395  01D0              add eax,edx
00003397  0FB600            movzx eax,byte [eax]
0000339A  8845E7            mov [ebp-0x19],al
0000339D  E941FFFFFF        jmp 0x32e3
000033A2  83EC08            sub esp,byte +0x8
000033A5  FF75CC            push dword [ebp-0x34]
000033A8  FF75D0            push dword [ebp-0x30]
000033AB  E844FDFFFF        call 0x30f4
000033B0  83C410            add esp,byte +0x10
000033B3  8B45CC            mov eax,[ebp-0x34]
000033B6  83E801            sub eax,byte +0x1
000033B9  8945E0            mov [ebp-0x20],eax
000033BC  EB34              jmp short 0x33f2
000033BE  8B45D0            mov eax,[ebp-0x30]
000033C1  83E801            sub eax,byte +0x1
000033C4  8945DC            mov [ebp-0x24],eax
000033C7  EB1A              jmp short 0x33e3
000033C9  0FB645E7          movzx eax,byte [ebp-0x19]
000033CD  83EC04            sub esp,byte +0x4
000033D0  50                push eax
000033D1  FF75E0            push dword [ebp-0x20]
000033D4  FF75DC            push dword [ebp-0x24]
000033D7  E8B7FBFFFF        call 0x2f93
000033DC  83C410            add esp,byte +0x10
000033DF  8345DC01          add dword [ebp-0x24],byte +0x1
000033E3  8B45D0            mov eax,[ebp-0x30]
000033E6  83C001            add eax,byte +0x1
000033E9  3945DC            cmp [ebp-0x24],eax
000033EC  7EDB              jng 0x33c9
000033EE  8345E001          add dword [ebp-0x20],byte +0x1
000033F2  8B45CC            mov eax,[ebp-0x34]
000033F5  83C001            add eax,byte +0x1
000033F8  3945E0            cmp [ebp-0x20],eax
000033FB  7EC1              jng 0x33be
000033FD  83EC08            sub esp,byte +0x8
00003400  FF75CC            push dword [ebp-0x34]
00003403  FF75D0            push dword [ebp-0x30]
00003406  E82CFCFFFF        call 0x3037
0000340B  83C410            add esp,byte +0x10
0000340E  83EC08            sub esp,byte +0x8
00003411  FF75CC            push dword [ebp-0x34]
00003414  FF75D0            push dword [ebp-0x30]
00003417  E87DFDFFFF        call 0x3199
0000341C  83C410            add esp,byte +0x10
0000341F  E9BFFEFFFF        jmp 0x32e3
00003424  55                push ebp
00003425  89E5              mov ebp,esp
00003427  83EC10            sub esp,byte +0x10
0000342A  C745FC00900000    mov dword [ebp-0x4],0x9000
00003431  8B45FC            mov eax,[ebp-0x4]
00003434  8B00              mov eax,[eax]
00003436  C9                leave
00003437  C3                ret
00003438  55                push ebp
00003439  89E5              mov ebp,esp
0000343B  83EC18            sub esp,byte +0x18
0000343E  C705042502000000  mov dword [dword 0x22504],0x240000
         -2400
00003448  A104250200        mov eax,[0x22504]
0000344D  83EC0C            sub esp,byte +0xc
00003450  50                push eax
00003451  E847330000        call 0x679d
00003456  83C410            add esp,byte +0x10
00003459  A300250200        mov [0x22500],eax
0000345E  A100250200        mov eax,[0x22500]
00003463  85C0              test eax,eax
00003465  7425              jz 0x348c
00003467  C745F400000000    mov dword [ebp-0xc],0x0
0000346E  EB12              jmp short 0x3482
00003470  8B1500250200      mov edx,[dword 0x22500]
00003476  8B45F4            mov eax,[ebp-0xc]
00003479  01D0              add eax,edx
0000347B  C60000            mov byte [eax],0x0
0000347E  8345F401          add dword [ebp-0xc],byte +0x1
00003482  A104250200        mov eax,[0x22504]
00003487  3945F4            cmp [ebp-0xc],eax
0000348A  72E4              jc 0x3470
0000348C  90                nop
0000348D  C9                leave
0000348E  C3                ret
0000348F  55                push ebp
00003490  89E5              mov ebp,esp
00003492  83EC10            sub esp,byte +0x10
00003495  A100250200        mov eax,[0x22500]
0000349A  85C0              test eax,eax
0000349C  0F8489000000      jz near 0x352b
000034A2  837D0800          cmp dword [ebp+0x8],byte +0x0
000034A6  0F8880000000      js near 0x352c
000034AC  817D08FF030000    cmp dword [ebp+0x8],0x3ff
000034B3  7F77              jg 0x352c
000034B5  837D0C00          cmp dword [ebp+0xc],byte +0x0
000034B9  7871              js 0x352c
000034BB  817D0CFF020000    cmp dword [ebp+0xc],0x2ff
000034C2  7F68              jg 0x352c
000034C4  8B450C            mov eax,[ebp+0xc]
000034C7  C1E00A            shl eax,byte 0xa
000034CA  89C2              mov edx,eax
000034CC  8B4508            mov eax,[ebp+0x8]
000034CF  01C2              add edx,eax
000034D1  89D0              mov eax,edx
000034D3  01C0              add eax,eax
000034D5  01D0              add eax,edx
000034D7  8945FC            mov [ebp-0x4],eax
000034DA  8B4510            mov eax,[ebp+0x10]
000034DD  8845FB            mov [ebp-0x5],al
000034E0  8B4510            mov eax,[ebp+0x10]
000034E3  C1E808            shr eax,byte 0x8
000034E6  8845FA            mov [ebp-0x6],al
000034E9  8B4510            mov eax,[ebp+0x10]
000034EC  C1E810            shr eax,byte 0x10
000034EF  8845F9            mov [ebp-0x7],al
000034F2  8B1500250200      mov edx,[dword 0x22500]
000034F8  8B45FC            mov eax,[ebp-0x4]
000034FB  01C2              add edx,eax
000034FD  0FB645FB          movzx eax,byte [ebp-0x5]
00003501  8802              mov [edx],al
00003503  A100250200        mov eax,[0x22500]
00003508  8B55FC            mov edx,[ebp-0x4]
0000350B  83C201            add edx,byte +0x1
0000350E  01C2              add edx,eax
00003510  0FB645FA          movzx eax,byte [ebp-0x6]
00003514  8802              mov [edx],al
00003516  A100250200        mov eax,[0x22500]
0000351B  8B55FC            mov edx,[ebp-0x4]
0000351E  83C202            add edx,byte +0x2
00003521  01C2              add edx,eax
00003523  0FB645F9          movzx eax,byte [ebp-0x7]
00003527  8802              mov [edx],al
00003529  EB01              jmp short 0x352c
0000352B  90                nop
0000352C  C9                leave
0000352D  C3                ret
0000352E  55                push ebp
0000352F  89E5              mov ebp,esp
00003531  83EC18            sub esp,byte +0x18
00003534  A100250200        mov eax,[0x22500]
00003539  85C0              test eax,eax
0000353B  7505              jnz 0x3542
0000353D  E8F6FEFFFF        call 0x3438
00003542  A100250200        mov eax,[0x22500]
00003547  85C0              test eax,eax
00003549  0F8486000000      jz near 0x35d5
0000354F  837D080F          cmp dword [ebp+0x8],byte +0xf
00003553  770D              ja 0x3562
00003555  8B4508            mov eax,[ebp+0x8]
00003558  8B0485E0DB0100    mov eax,[eax*4+0x1dbe0]
0000355F  894508            mov [ebp+0x8],eax
00003562  8B4508            mov eax,[ebp+0x8]
00003565  8845F3            mov [ebp-0xd],al
00003568  8B4508            mov eax,[ebp+0x8]
0000356B  C1E808            shr eax,byte 0x8
0000356E  8845F2            mov [ebp-0xe],al
00003571  8B4508            mov eax,[ebp+0x8]
00003574  C1E810            shr eax,byte 0x10
00003577  8845F1            mov [ebp-0xf],al
0000357A  C745F400000000    mov dword [ebp-0xc],0x0
00003581  EB47              jmp short 0x35ca
00003583  8B55F4            mov edx,[ebp-0xc]
00003586  89D0              mov eax,edx
00003588  01C0              add eax,eax
0000358A  01D0              add eax,edx
0000358C  8945EC            mov [ebp-0x14],eax
0000358F  8B1500250200      mov edx,[dword 0x22500]
00003595  8B45EC            mov eax,[ebp-0x14]
00003598  01C2              add edx,eax
0000359A  0FB645F3          movzx eax,byte [ebp-0xd]
0000359E  8802              mov [edx],al
000035A0  A100250200        mov eax,[0x22500]
000035A5  8B55EC            mov edx,[ebp-0x14]
000035A8  83C201            add edx,byte +0x1
000035AB  01C2              add edx,eax
000035AD  0FB645F2          movzx eax,byte [ebp-0xe]
000035B1  8802              mov [edx],al
000035B3  A100250200        mov eax,[0x22500]
000035B8  8B55EC            mov edx,[ebp-0x14]
000035BB  83C202            add edx,byte +0x2
000035BE  01C2              add edx,eax
000035C0  0FB645F1          movzx eax,byte [ebp-0xf]
000035C4  8802              mov [edx],al
000035C6  8345F401          add dword [ebp-0xc],byte +0x1
000035CA  817DF4FFFF0B00    cmp dword [ebp-0xc],0xbffff
000035D1  76B0              jna 0x3583
000035D3  EB01              jmp short 0x35d6
000035D5  90                nop
000035D6  C9                leave
000035D7  C3                ret
000035D8  55                push ebp
000035D9  89E5              mov ebp,esp
000035DB  83EC20            sub esp,byte +0x20
000035DE  A100250200        mov eax,[0x22500]
000035E3  85C0              test eax,eax
000035E5  7460              jz 0x3647
000035E7  E838FEFFFF        call 0x3424
000035EC  8945F8            mov [ebp-0x8],eax
000035EF  837DF800          cmp dword [ebp-0x8],byte +0x0
000035F3  7455              jz 0x364a
000035F5  A100250200        mov eax,[0x22500]
000035FA  8945F4            mov [ebp-0xc],eax
000035FD  8B45F8            mov eax,[ebp-0x8]
00003600  8945F0            mov [ebp-0x10],eax
00003603  A104250200        mov eax,[0x22504]
00003608  C1E802            shr eax,byte 0x2
0000360B  8945EC            mov [ebp-0x14],eax
0000360E  C745FC00000000    mov dword [ebp-0x4],0x0
00003615  EB26              jmp short 0x363d
00003617  8B45FC            mov eax,[ebp-0x4]
0000361A  8D148500000000    lea edx,[eax*4+0x0]
00003621  8B45F4            mov eax,[ebp-0xc]
00003624  01D0              add eax,edx
00003626  8B55FC            mov edx,[ebp-0x4]
00003629  8D0C9500000000    lea ecx,[edx*4+0x0]
00003630  8B55F0            mov edx,[ebp-0x10]
00003633  01CA              add edx,ecx
00003635  8B00              mov eax,[eax]
00003637  8902              mov [edx],eax
00003639  8345FC01          add dword [ebp-0x4],byte +0x1
0000363D  8B45FC            mov eax,[ebp-0x4]
00003640  3B45EC            cmp eax,[ebp-0x14]
00003643  72D2              jc 0x3617
00003645  EB04              jmp short 0x364b
00003647  90                nop
00003648  EB01              jmp short 0x364b
0000364A  90                nop
0000364B  C9                leave
0000364C  C3                ret
0000364D  55                push ebp
0000364E  89E5              mov ebp,esp
00003650  83EC10            sub esp,byte +0x10
00003653  A100250200        mov eax,[0x22500]
00003658  85C0              test eax,eax
0000365A  0F84C8000000      jz near 0x3728
00003660  8B4518            mov eax,[ebp+0x18]
00003663  8845F7            mov [ebp-0x9],al
00003666  8B4518            mov eax,[ebp+0x18]
00003669  C1E808            shr eax,byte 0x8
0000366C  8845F6            mov [ebp-0xa],al
0000366F  8B4518            mov eax,[ebp+0x18]
00003672  C1E810            shr eax,byte 0x10
00003675  8845F5            mov [ebp-0xb],al
00003678  8B450C            mov eax,[ebp+0xc]
0000367B  8945FC            mov [ebp-0x4],eax
0000367E  E992000000        jmp 0x3715
00003683  837DFC00          cmp dword [ebp-0x4],byte +0x0
00003687  0F8883000000      js near 0x3710
0000368D  817DFCFF020000    cmp dword [ebp-0x4],0x2ff
00003694  7F7A              jg 0x3710
00003696  8B4508            mov eax,[ebp+0x8]
00003699  8945F8            mov [ebp-0x8],eax
0000369C  EB63              jmp short 0x3701
0000369E  837DF800          cmp dword [ebp-0x8],byte +0x0
000036A2  7858              js 0x36fc
000036A4  817DF8FF030000    cmp dword [ebp-0x8],0x3ff
000036AB  7F4F              jg 0x36fc
000036AD  8B45FC            mov eax,[ebp-0x4]
000036B0  C1E00A            shl eax,byte 0xa
000036B3  89C2              mov edx,eax
000036B5  8B45F8            mov eax,[ebp-0x8]
000036B8  01C2              add edx,eax
000036BA  89D0              mov eax,edx
000036BC  01C0              add eax,eax
000036BE  01D0              add eax,edx
000036C0  8945F0            mov [ebp-0x10],eax
000036C3  8B1500250200      mov edx,[dword 0x22500]
000036C9  8B45F0            mov eax,[ebp-0x10]
000036CC  01C2              add edx,eax
000036CE  0FB645F7          movzx eax,byte [ebp-0x9]
000036D2  8802              mov [edx],al
000036D4  A100250200        mov eax,[0x22500]
000036D9  8B55F0            mov edx,[ebp-0x10]
000036DC  83C201            add edx,byte +0x1
000036DF  01C2              add edx,eax
000036E1  0FB645F6          movzx eax,byte [ebp-0xa]
000036E5  8802              mov [edx],al
000036E7  A100250200        mov eax,[0x22500]
000036EC  8B55F0            mov edx,[ebp-0x10]
000036EF  83C202            add edx,byte +0x2
000036F2  01C2              add edx,eax
000036F4  0FB645F5          movzx eax,byte [ebp-0xb]
000036F8  8802              mov [edx],al
000036FA  EB01              jmp short 0x36fd
000036FC  90                nop
000036FD  8345F801          add dword [ebp-0x8],byte +0x1
00003701  8B5508            mov edx,[ebp+0x8]
00003704  8B4510            mov eax,[ebp+0x10]
00003707  01D0              add eax,edx
00003709  3945F8            cmp [ebp-0x8],eax
0000370C  7C90              jl 0x369e
0000370E  EB01              jmp short 0x3711
00003710  90                nop
00003711  8345FC01          add dword [ebp-0x4],byte +0x1
00003715  8B550C            mov edx,[ebp+0xc]
00003718  8B4514            mov eax,[ebp+0x14]
0000371B  01D0              add eax,edx
0000371D  3945FC            cmp [ebp-0x4],eax
00003720  0F8C5DFFFFFF      jl near 0x3683
00003726  EB01              jmp short 0x3729
00003728  90                nop
00003729  C9                leave
0000372A  C3                ret
0000372B  55                push ebp
0000372C  89E5              mov ebp,esp
0000372E  83EC10            sub esp,byte +0x10
00003731  A100250200        mov eax,[0x22500]
00003736  85C0              test eax,eax
00003738  0F84E5000000      jz near 0x3823
0000373E  E8E1FCFFFF        call 0x3424
00003743  8945F4            mov [ebp-0xc],eax
00003746  837DF400          cmp dword [ebp-0xc],byte +0x0
0000374A  0F84D6000000      jz near 0x3826
00003750  8B450C            mov eax,[ebp+0xc]
00003753  8945FC            mov [ebp-0x4],eax
00003756  E9B5000000        jmp 0x3810
0000375B  837DFC00          cmp dword [ebp-0x4],byte +0x0
0000375F  0F88A6000000      js near 0x380b
00003765  817DFCFF020000    cmp dword [ebp-0x4],0x2ff
0000376C  0F8F99000000      jg near 0x380b
00003772  8B4508            mov eax,[ebp+0x8]
00003775  8945F8            mov [ebp-0x8],eax
00003778  EB7E              jmp short 0x37f8
0000377A  837DF800          cmp dword [ebp-0x8],byte +0x0
0000377E  7873              js 0x37f3
00003780  817DF8FF030000    cmp dword [ebp-0x8],0x3ff
00003787  7F6A              jg 0x37f3
00003789  8B45FC            mov eax,[ebp-0x4]
0000378C  C1E00A            shl eax,byte 0xa
0000378F  89C2              mov edx,eax
00003791  8B45F8            mov eax,[ebp-0x8]
00003794  01C2              add edx,eax
00003796  89D0              mov eax,edx
00003798  01C0              add eax,eax
0000379A  01D0              add eax,edx
0000379C  8945F0            mov [ebp-0x10],eax
0000379F  8B1500250200      mov edx,[dword 0x22500]
000037A5  8B45F0            mov eax,[ebp-0x10]
000037A8  01D0              add eax,edx
000037AA  8B4DF4            mov ecx,[ebp-0xc]
000037AD  8B55F0            mov edx,[ebp-0x10]
000037B0  01CA              add edx,ecx
000037B2  0FB600            movzx eax,byte [eax]
000037B5  8802              mov [edx],al
000037B7  A100250200        mov eax,[0x22500]
000037BC  8B55F0            mov edx,[ebp-0x10]
000037BF  83C201            add edx,byte +0x1
000037C2  01D0              add eax,edx
000037C4  8B55F0            mov edx,[ebp-0x10]
000037C7  8D4A01            lea ecx,[edx+0x1]
000037CA  8B55F4            mov edx,[ebp-0xc]
000037CD  01CA              add edx,ecx
000037CF  0FB600            movzx eax,byte [eax]
000037D2  8802              mov [edx],al
000037D4  A100250200        mov eax,[0x22500]
000037D9  8B55F0            mov edx,[ebp-0x10]
000037DC  83C202            add edx,byte +0x2
000037DF  01D0              add eax,edx
000037E1  8B55F0            mov edx,[ebp-0x10]
000037E4  8D4A02            lea ecx,[edx+0x2]
000037E7  8B55F4            mov edx,[ebp-0xc]
000037EA  01CA              add edx,ecx
000037EC  0FB600            movzx eax,byte [eax]
000037EF  8802              mov [edx],al
000037F1  EB01              jmp short 0x37f4
000037F3  90                nop
000037F4  8345F801          add dword [ebp-0x8],byte +0x1
000037F8  8B5508            mov edx,[ebp+0x8]
000037FB  8B4510            mov eax,[ebp+0x10]
000037FE  01D0              add eax,edx
00003800  3945F8            cmp [ebp-0x8],eax
00003803  0F8C71FFFFFF      jl near 0x377a
00003809  EB01              jmp short 0x380c
0000380B  90                nop
0000380C  8345FC01          add dword [ebp-0x4],byte +0x1
00003810  8B550C            mov edx,[ebp+0xc]
00003813  8B4514            mov eax,[ebp+0x14]
00003816  01D0              add eax,edx
00003818  3945FC            cmp [ebp-0x4],eax
0000381B  0F8C3AFFFFFF      jl near 0x375b
00003821  EB04              jmp short 0x3827
00003823  90                nop
00003824  EB01              jmp short 0x3827
00003826  90                nop
00003827  C9                leave
00003828  C3                ret
00003829  55                push ebp
0000382A  89E5              mov ebp,esp
0000382C  83EC10            sub esp,byte +0x10
0000382F  E8F0FBFFFF        call 0x3424
00003834  8945FC            mov [ebp-0x4],eax
00003837  837DFC00          cmp dword [ebp-0x4],byte +0x0
0000383B  746D              jz 0x38aa
0000383D  837D0800          cmp dword [ebp+0x8],byte +0x0
00003841  7868              js 0x38ab
00003843  817D08FF030000    cmp dword [ebp+0x8],0x3ff
0000384A  7F5F              jg 0x38ab
0000384C  837D0C00          cmp dword [ebp+0xc],byte +0x0
00003850  7859              js 0x38ab
00003852  817D0CFF020000    cmp dword [ebp+0xc],0x2ff
00003859  7F50              jg 0x38ab
0000385B  8B450C            mov eax,[ebp+0xc]
0000385E  C1E00A            shl eax,byte 0xa
00003861  89C2              mov edx,eax
00003863  8B4508            mov eax,[ebp+0x8]
00003866  01C2              add edx,eax
00003868  89D0              mov eax,edx
0000386A  01C0              add eax,eax
0000386C  01D0              add eax,edx
0000386E  8945F8            mov [ebp-0x8],eax
00003871  8B55FC            mov edx,[ebp-0x4]
00003874  8B45F8            mov eax,[ebp-0x8]
00003877  01D0              add eax,edx
00003879  8B5510            mov edx,[ebp+0x10]
0000387C  8810              mov [eax],dl
0000387E  8B4510            mov eax,[ebp+0x10]
00003881  C1E808            shr eax,byte 0x8
00003884  89C2              mov edx,eax
00003886  8B45F8            mov eax,[ebp-0x8]
00003889  8D4801            lea ecx,[eax+0x1]
0000388C  8B45FC            mov eax,[ebp-0x4]
0000388F  01C8              add eax,ecx
00003891  8810              mov [eax],dl
00003893  8B4510            mov eax,[ebp+0x10]
00003896  C1E810            shr eax,byte 0x10
00003899  89C2              mov edx,eax
0000389B  8B45F8            mov eax,[ebp-0x8]
0000389E  8D4802            lea ecx,[eax+0x2]
000038A1  8B45FC            mov eax,[ebp-0x4]
000038A4  01C8              add eax,ecx
000038A6  8810              mov [eax],dl
000038A8  EB01              jmp short 0x38ab
000038AA  90                nop
000038AB  C9                leave
000038AC  C3                ret
000038AD  55                push ebp
000038AE  89E5              mov ebp,esp
000038B0  83EC10            sub esp,byte +0x10
000038B3  C745FC00000000    mov dword [ebp-0x4],0x0
000038BA  EB2F              jmp short 0x38eb
000038BC  C745F800000000    mov dword [ebp-0x8],0x0
000038C3  EB1C              jmp short 0x38e1
000038C5  8B55FC            mov edx,[ebp-0x4]
000038C8  89D0              mov eax,edx
000038CA  01C0              add eax,eax
000038CC  01D0              add eax,edx
000038CE  01C0              add eax,eax
000038D0  8B55F8            mov edx,[ebp-0x8]
000038D3  01D0              add eax,edx
000038D5  0520250200        add eax,0x22520
000038DA  C60000            mov byte [eax],0x0
000038DD  8345F801          add dword [ebp-0x8],byte +0x1
000038E1  837DF805          cmp dword [ebp-0x8],byte +0x5
000038E5  7EDE              jng 0x38c5
000038E7  8345FC01          add dword [ebp-0x4],byte +0x1
000038EB  837DFC7F          cmp dword [ebp-0x4],byte +0x7f
000038EF  7ECB              jng 0x38bc
000038F1  C6054026020060    mov byte [dword 0x22640],0x60
000038F8  C6054126020090    mov byte [dword 0x22641],0x90
000038FF  C6054226020090    mov byte [dword 0x22642],0x90
00003906  C6054326020090    mov byte [dword 0x22643],0x90
0000390D  C6054426020060    mov byte [dword 0x22644],0x60
00003914  C6054526020000    mov byte [dword 0x22645],0x0
0000391B  C6054626020040    mov byte [dword 0x22646],0x40
00003922  C60547260200C0    mov byte [dword 0x22647],0xc0
00003929  C6054826020040    mov byte [dword 0x22648],0x40
00003930  C6054926020040    mov byte [dword 0x22649],0x40
00003937  C6054A260200E0    mov byte [dword 0x2264a],0xe0
0000393E  C6054B26020000    mov byte [dword 0x2264b],0x0
00003945  C6054C260200E0    mov byte [dword 0x2264c],0xe0
0000394C  C6054D26020020    mov byte [dword 0x2264d],0x20
00003953  C6054E260200E0    mov byte [dword 0x2264e],0xe0
0000395A  C6054F26020080    mov byte [dword 0x2264f],0x80
00003961  C60550260200E0    mov byte [dword 0x22650],0xe0
00003968  C6055126020000    mov byte [dword 0x22651],0x0
0000396F  C60552260200E0    mov byte [dword 0x22652],0xe0
00003976  C6055326020020    mov byte [dword 0x22653],0x20
0000397D  C6055426020060    mov byte [dword 0x22654],0x60
00003984  C6055526020020    mov byte [dword 0x22655],0x20
0000398B  C60556260200E0    mov byte [dword 0x22656],0xe0
00003992  C6055726020000    mov byte [dword 0x22657],0x0
00003999  C6055826020080    mov byte [dword 0x22658],0x80
000039A0  C6055926020080    mov byte [dword 0x22659],0x80
000039A7  C6055A260200E0    mov byte [dword 0x2265a],0xe0
000039AE  C6055B26020020    mov byte [dword 0x2265b],0x20
000039B5  C6055C26020020    mov byte [dword 0x2265c],0x20
000039BC  C6055D26020000    mov byte [dword 0x2265d],0x0
000039C3  C6055E260200E0    mov byte [dword 0x2265e],0xe0
000039CA  C6055F26020080    mov byte [dword 0x2265f],0x80
000039D1  C60560260200E0    mov byte [dword 0x22660],0xe0
000039D8  C6056126020020    mov byte [dword 0x22661],0x20
000039DF  C60562260200E0    mov byte [dword 0x22662],0xe0
000039E6  C6056326020000    mov byte [dword 0x22663],0x0
000039ED  C6056426020060    mov byte [dword 0x22664],0x60
000039F4  C6056526020080    mov byte [dword 0x22665],0x80
000039FB  C60566260200E0    mov byte [dword 0x22666],0xe0
00003A02  C6056726020090    mov byte [dword 0x22667],0x90
00003A09  C6056826020060    mov byte [dword 0x22668],0x60
00003A10  C6056926020000    mov byte [dword 0x22669],0x0
00003A17  C6056A260200E0    mov byte [dword 0x2266a],0xe0
00003A1E  C6056B26020020    mov byte [dword 0x2266b],0x20
00003A25  C6056C26020040    mov byte [dword 0x2266c],0x40
00003A2C  C6056D26020080    mov byte [dword 0x2266d],0x80
00003A33  C6056E26020080    mov byte [dword 0x2266e],0x80
00003A3A  C6056F26020000    mov byte [dword 0x2266f],0x0
00003A41  C6057026020060    mov byte [dword 0x22670],0x60
00003A48  C6057126020090    mov byte [dword 0x22671],0x90
00003A4F  C6057226020060    mov byte [dword 0x22672],0x60
00003A56  C6057326020090    mov byte [dword 0x22673],0x90
00003A5D  C6057426020060    mov byte [dword 0x22674],0x60
00003A64  C6057526020000    mov byte [dword 0x22675],0x0
00003A6B  C6057626020060    mov byte [dword 0x22676],0x60
00003A72  C6057726020090    mov byte [dword 0x22677],0x90
00003A79  C6057826020070    mov byte [dword 0x22678],0x70
00003A80  C6057926020010    mov byte [dword 0x22679],0x10
00003A87  C6057A26020060    mov byte [dword 0x2267a],0x60
00003A8E  C6057B26020000    mov byte [dword 0x2267b],0x0
00003A95  C605E625020040    mov byte [dword 0x225e6],0x40
00003A9C  C605E725020040    mov byte [dword 0x225e7],0x40
00003AA3  C605E825020040    mov byte [dword 0x225e8],0x40
00003AAA  C605E925020000    mov byte [dword 0x225e9],0x0
00003AB1  C605EA25020040    mov byte [dword 0x225ea],0x40
00003AB8  C605EB25020000    mov byte [dword 0x225eb],0x0
00003ABF  C605EC250200A0    mov byte [dword 0x225ec],0xa0
00003AC6  C605ED250200A0    mov byte [dword 0x225ed],0xa0
00003ACD  C605EE25020000    mov byte [dword 0x225ee],0x0
00003AD4  C605EF25020000    mov byte [dword 0x225ef],0x0
00003ADB  C605F025020000    mov byte [dword 0x225f0],0x0
00003AE2  C605F125020000    mov byte [dword 0x225f1],0x0
00003AE9  C605F2250200A0    mov byte [dword 0x225f2],0xa0
00003AF0  C605F3250200E0    mov byte [dword 0x225f3],0xe0
00003AF7  C605F4250200A0    mov byte [dword 0x225f4],0xa0
00003AFE  C605F5250200E0    mov byte [dword 0x225f5],0xe0
00003B05  C605F6250200A0    mov byte [dword 0x225f6],0xa0
00003B0C  C605F725020000    mov byte [dword 0x225f7],0x0
00003B13  C605F825020040    mov byte [dword 0x225f8],0x40
00003B1A  C605F9250200E0    mov byte [dword 0x225f9],0xe0
00003B21  C605FA250200C0    mov byte [dword 0x225fa],0xc0
00003B28  C605FB25020060    mov byte [dword 0x225fb],0x60
00003B2F  C605FC250200E0    mov byte [dword 0x225fc],0xe0
00003B36  C605FD25020040    mov byte [dword 0x225fd],0x40
00003B3D  C605FE250200A0    mov byte [dword 0x225fe],0xa0
00003B44  C605FF25020020    mov byte [dword 0x225ff],0x20
00003B4B  C6050026020040    mov byte [dword 0x22600],0x40
00003B52  C6050126020080    mov byte [dword 0x22601],0x80
00003B59  C60502260200A0    mov byte [dword 0x22602],0xa0
00003B60  C6050326020000    mov byte [dword 0x22603],0x0
00003B67  C6050426020040    mov byte [dword 0x22604],0x40
00003B6E  C60505260200A0    mov byte [dword 0x22605],0xa0
00003B75  C6050626020040    mov byte [dword 0x22606],0x40
00003B7C  C60507260200A0    mov byte [dword 0x22607],0xa0
00003B83  C6050826020050    mov byte [dword 0x22608],0x50
00003B8A  C6050926020000    mov byte [dword 0x22609],0x0
00003B91  C6050A26020040    mov byte [dword 0x2260a],0x40
00003B98  C6050B26020040    mov byte [dword 0x2260b],0x40
00003B9F  C6050C26020000    mov byte [dword 0x2260c],0x0
00003BA6  C6050D26020000    mov byte [dword 0x2260d],0x0
00003BAD  C6050E26020000    mov byte [dword 0x2260e],0x0
00003BB4  C6050F26020000    mov byte [dword 0x2260f],0x0
00003BBB  C6051026020020    mov byte [dword 0x22610],0x20
00003BC2  C6051126020040    mov byte [dword 0x22611],0x40
00003BC9  C6051226020040    mov byte [dword 0x22612],0x40
00003BD0  C6051326020040    mov byte [dword 0x22613],0x40
00003BD7  C6051426020020    mov byte [dword 0x22614],0x20
00003BDE  C6051526020000    mov byte [dword 0x22615],0x0
00003BE5  C6051626020080    mov byte [dword 0x22616],0x80
00003BEC  C6051726020040    mov byte [dword 0x22617],0x40
00003BF3  C6051826020040    mov byte [dword 0x22618],0x40
00003BFA  C6051926020040    mov byte [dword 0x22619],0x40
00003C01  C6051A26020080    mov byte [dword 0x2261a],0x80
00003C08  C6051B26020000    mov byte [dword 0x2261b],0x0
00003C0F  C6051C26020000    mov byte [dword 0x2261c],0x0
00003C16  C6051D260200A0    mov byte [dword 0x2261d],0xa0
00003C1D  C6051E26020040    mov byte [dword 0x2261e],0x40
00003C24  C6051F260200A0    mov byte [dword 0x2261f],0xa0
00003C2B  C6052026020000    mov byte [dword 0x22620],0x0
00003C32  C6052126020000    mov byte [dword 0x22621],0x0
00003C39  C6052226020000    mov byte [dword 0x22622],0x0
00003C40  C6052326020040    mov byte [dword 0x22623],0x40
00003C47  C60524260200E0    mov byte [dword 0x22624],0xe0
00003C4E  C6052526020040    mov byte [dword 0x22625],0x40
00003C55  C6052626020000    mov byte [dword 0x22626],0x0
00003C5C  C6052726020000    mov byte [dword 0x22627],0x0
00003C63  C6052826020000    mov byte [dword 0x22628],0x0
00003C6A  C6052926020000    mov byte [dword 0x22629],0x0
00003C71  C6052A26020000    mov byte [dword 0x2262a],0x0
00003C78  C6052B26020040    mov byte [dword 0x2262b],0x40
00003C7F  C6052C26020040    mov byte [dword 0x2262c],0x40
00003C86  C6052D26020080    mov byte [dword 0x2262d],0x80
00003C8D  C6052E26020000    mov byte [dword 0x2262e],0x0
00003C94  C6052F26020000    mov byte [dword 0x2262f],0x0
00003C9B  C60530260200E0    mov byte [dword 0x22630],0xe0
00003CA2  C6053126020000    mov byte [dword 0x22631],0x0
00003CA9  C6053226020000    mov byte [dword 0x22632],0x0
00003CB0  C6053326020000    mov byte [dword 0x22633],0x0
00003CB7  C6053426020000    mov byte [dword 0x22634],0x0
00003CBE  C6053526020000    mov byte [dword 0x22635],0x0
00003CC5  C6053626020000    mov byte [dword 0x22636],0x0
00003CCC  C6053726020000    mov byte [dword 0x22637],0x0
00003CD3  C6053826020040    mov byte [dword 0x22638],0x40
00003CDA  C6053926020000    mov byte [dword 0x22639],0x0
00003CE1  C6053A26020000    mov byte [dword 0x2263a],0x0
00003CE8  C6053B26020020    mov byte [dword 0x2263b],0x20
00003CEF  C6053C26020040    mov byte [dword 0x2263c],0x40
00003CF6  C6053D26020080    mov byte [dword 0x2263d],0x80
00003CFD  C6053E26020000    mov byte [dword 0x2263e],0x0
00003D04  C6053F26020000    mov byte [dword 0x2263f],0x0
00003D0B  C6057C26020000    mov byte [dword 0x2267c],0x0
00003D12  C6057D26020040    mov byte [dword 0x2267d],0x40
00003D19  C6057E26020000    mov byte [dword 0x2267e],0x0
00003D20  C6057F26020040    mov byte [dword 0x2267f],0x40
00003D27  C6058026020000    mov byte [dword 0x22680],0x0
00003D2E  C6058126020000    mov byte [dword 0x22681],0x0
00003D35  C6058226020000    mov byte [dword 0x22682],0x0
00003D3C  C6058326020040    mov byte [dword 0x22683],0x40
00003D43  C6058426020000    mov byte [dword 0x22684],0x0
00003D4A  C6058526020040    mov byte [dword 0x22685],0x40
00003D51  C6058626020080    mov byte [dword 0x22686],0x80
00003D58  C6058726020000    mov byte [dword 0x22687],0x0
00003D5F  C6058826020020    mov byte [dword 0x22688],0x20
00003D66  C6058926020040    mov byte [dword 0x22689],0x40
00003D6D  C6058A26020080    mov byte [dword 0x2268a],0x80
00003D74  C6058B26020040    mov byte [dword 0x2268b],0x40
00003D7B  C6058C26020020    mov byte [dword 0x2268c],0x20
00003D82  C6058D26020000    mov byte [dword 0x2268d],0x0
00003D89  C6058E26020000    mov byte [dword 0x2268e],0x0
00003D90  C6058F260200E0    mov byte [dword 0x2268f],0xe0
00003D97  C6059026020000    mov byte [dword 0x22690],0x0
00003D9E  C60591260200E0    mov byte [dword 0x22691],0xe0
00003DA5  C6059226020000    mov byte [dword 0x22692],0x0
00003DAC  C6059326020000    mov byte [dword 0x22693],0x0
00003DB3  C6059426020080    mov byte [dword 0x22694],0x80
00003DBA  C6059526020040    mov byte [dword 0x22695],0x40
00003DC1  C6059626020020    mov byte [dword 0x22696],0x20
00003DC8  C6059726020040    mov byte [dword 0x22697],0x40
00003DCF  C6059826020080    mov byte [dword 0x22698],0x80
00003DD6  C6059926020000    mov byte [dword 0x22699],0x0
00003DDD  C6059A260200E0    mov byte [dword 0x2269a],0xe0
00003DE4  C6059B26020020    mov byte [dword 0x2269b],0x20
00003DEB  C6059C26020040    mov byte [dword 0x2269c],0x40
00003DF2  C6059D26020000    mov byte [dword 0x2269d],0x0
00003DF9  C6059E26020040    mov byte [dword 0x2269e],0x40
00003E00  C6059F26020000    mov byte [dword 0x2269f],0x0
00003E07  C605A026020060    mov byte [dword 0x226a0],0x60
00003E0E  C605A126020090    mov byte [dword 0x226a1],0x90
00003E15  C605A2260200B0    mov byte [dword 0x226a2],0xb0
00003E1C  C605A326020080    mov byte [dword 0x226a3],0x80
00003E23  C605A426020070    mov byte [dword 0x226a4],0x70
00003E2A  C605A526020000    mov byte [dword 0x226a5],0x0
00003E31  C605A626020040    mov byte [dword 0x226a6],0x40
00003E38  C605A7260200A0    mov byte [dword 0x226a7],0xa0
00003E3F  C605A8260200E0    mov byte [dword 0x226a8],0xe0
00003E46  C605A9260200A0    mov byte [dword 0x226a9],0xa0
00003E4D  C605AA260200A0    mov byte [dword 0x226aa],0xa0
00003E54  C605AB26020000    mov byte [dword 0x226ab],0x0
00003E5B  C605AC260200C0    mov byte [dword 0x226ac],0xc0
00003E62  C605AD260200A0    mov byte [dword 0x226ad],0xa0
00003E69  C605AE260200C0    mov byte [dword 0x226ae],0xc0
00003E70  C605AF260200A0    mov byte [dword 0x226af],0xa0
00003E77  C605B0260200C0    mov byte [dword 0x226b0],0xc0
00003E7E  C605B126020000    mov byte [dword 0x226b1],0x0
00003E85  C605B226020060    mov byte [dword 0x226b2],0x60
00003E8C  C605B326020080    mov byte [dword 0x226b3],0x80
00003E93  C605B426020080    mov byte [dword 0x226b4],0x80
00003E9A  C605B526020080    mov byte [dword 0x226b5],0x80
00003EA1  C605B626020060    mov byte [dword 0x226b6],0x60
00003EA8  C605B726020000    mov byte [dword 0x226b7],0x0
00003EAF  C605B8260200C0    mov byte [dword 0x226b8],0xc0
00003EB6  C605B9260200A0    mov byte [dword 0x226b9],0xa0
00003EBD  C605BA260200A0    mov byte [dword 0x226ba],0xa0
00003EC4  C605BB260200A0    mov byte [dword 0x226bb],0xa0
00003ECB  C605BC260200C0    mov byte [dword 0x226bc],0xc0
00003ED2  C605BD26020000    mov byte [dword 0x226bd],0x0
00003ED9  C605BE260200E0    mov byte [dword 0x226be],0xe0
00003EE0  C605BF26020080    mov byte [dword 0x226bf],0x80
00003EE7  C605C0260200E0    mov byte [dword 0x226c0],0xe0
00003EEE  C605C126020080    mov byte [dword 0x226c1],0x80
00003EF5  C605C2260200E0    mov byte [dword 0x226c2],0xe0
00003EFC  C605C326020000    mov byte [dword 0x226c3],0x0
00003F03  C605C4260200E0    mov byte [dword 0x226c4],0xe0
00003F0A  C605C526020080    mov byte [dword 0x226c5],0x80
00003F11  C605C6260200E0    mov byte [dword 0x226c6],0xe0
00003F18  C605C726020080    mov byte [dword 0x226c7],0x80
00003F1F  C605C826020080    mov byte [dword 0x226c8],0x80
00003F26  C605C926020000    mov byte [dword 0x226c9],0x0
00003F2D  C605CA26020060    mov byte [dword 0x226ca],0x60
00003F34  C605CB26020080    mov byte [dword 0x226cb],0x80
00003F3B  C605CC260200B0    mov byte [dword 0x226cc],0xb0
00003F42  C605CD26020090    mov byte [dword 0x226cd],0x90
00003F49  C605CE26020060    mov byte [dword 0x226ce],0x60
00003F50  C605CF26020000    mov byte [dword 0x226cf],0x0
00003F57  C605D0260200A0    mov byte [dword 0x226d0],0xa0
00003F5E  C605D1260200A0    mov byte [dword 0x226d1],0xa0
00003F65  C605D2260200E0    mov byte [dword 0x226d2],0xe0
00003F6C  C605D3260200A0    mov byte [dword 0x226d3],0xa0
00003F73  C605D4260200A0    mov byte [dword 0x226d4],0xa0
00003F7A  C605D526020000    mov byte [dword 0x226d5],0x0
00003F81  C605D6260200E0    mov byte [dword 0x226d6],0xe0
00003F88  C605D726020040    mov byte [dword 0x226d7],0x40
00003F8F  C605D826020040    mov byte [dword 0x226d8],0x40
00003F96  C605D926020040    mov byte [dword 0x226d9],0x40
00003F9D  C605DA260200E0    mov byte [dword 0x226da],0xe0
00003FA4  C605DB26020000    mov byte [dword 0x226db],0x0
00003FAB  C605DC26020020    mov byte [dword 0x226dc],0x20
00003FB2  C605DD26020020    mov byte [dword 0x226dd],0x20
00003FB9  C605DE26020020    mov byte [dword 0x226de],0x20
00003FC0  C605DF260200A0    mov byte [dword 0x226df],0xa0
00003FC7  C605E026020040    mov byte [dword 0x226e0],0x40
00003FCE  C605E126020000    mov byte [dword 0x226e1],0x0
00003FD5  C605E2260200A0    mov byte [dword 0x226e2],0xa0
00003FDC  C605E3260200C0    mov byte [dword 0x226e3],0xc0
00003FE3  C605E4260200C0    mov byte [dword 0x226e4],0xc0
00003FEA  C605E5260200A0    mov byte [dword 0x226e5],0xa0
00003FF1  C605E6260200A0    mov byte [dword 0x226e6],0xa0
00003FF8  C605E726020000    mov byte [dword 0x226e7],0x0
00003FFF  C605E826020080    mov byte [dword 0x226e8],0x80
00004006  C605E926020080    mov byte [dword 0x226e9],0x80
0000400D  C605EA26020080    mov byte [dword 0x226ea],0x80
00004014  C605EB26020080    mov byte [dword 0x226eb],0x80
0000401B  C605EC260200E0    mov byte [dword 0x226ec],0xe0
00004022  C605ED26020000    mov byte [dword 0x226ed],0x0
00004029  C605EE260200A0    mov byte [dword 0x226ee],0xa0
00004030  C605EF260200E0    mov byte [dword 0x226ef],0xe0
00004037  C605F0260200E0    mov byte [dword 0x226f0],0xe0
0000403E  C605F1260200A0    mov byte [dword 0x226f1],0xa0
00004045  C605F2260200A0    mov byte [dword 0x226f2],0xa0
0000404C  C605F326020000    mov byte [dword 0x226f3],0x0
00004053  C605F4260200A0    mov byte [dword 0x226f4],0xa0
0000405A  C605F5260200E0    mov byte [dword 0x226f5],0xe0
00004061  C605F6260200E0    mov byte [dword 0x226f6],0xe0
00004068  C605F7260200B0    mov byte [dword 0x226f7],0xb0
0000406F  C605F8260200A0    mov byte [dword 0x226f8],0xa0
00004076  C605F926020000    mov byte [dword 0x226f9],0x0
0000407D  C605FA26020040    mov byte [dword 0x226fa],0x40
00004084  C605FB260200A0    mov byte [dword 0x226fb],0xa0
0000408B  C605FC260200A0    mov byte [dword 0x226fc],0xa0
00004092  C605FD260200A0    mov byte [dword 0x226fd],0xa0
00004099  C605FE26020040    mov byte [dword 0x226fe],0x40
000040A0  C605FF26020000    mov byte [dword 0x226ff],0x0
000040A7  C60500270200C0    mov byte [dword 0x22700],0xc0
000040AE  C60501270200A0    mov byte [dword 0x22701],0xa0
000040B5  C60502270200C0    mov byte [dword 0x22702],0xc0
000040BC  C6050327020080    mov byte [dword 0x22703],0x80
000040C3  C6050427020080    mov byte [dword 0x22704],0x80
000040CA  C6050527020000    mov byte [dword 0x22705],0x0
000040D1  C6050627020040    mov byte [dword 0x22706],0x40
000040D8  C60507270200A0    mov byte [dword 0x22707],0xa0
000040DF  C60508270200A0    mov byte [dword 0x22708],0xa0
000040E6  C60509270200C0    mov byte [dword 0x22709],0xc0
000040ED  C6050A27020060    mov byte [dword 0x2270a],0x60
000040F4  C6050B27020000    mov byte [dword 0x2270b],0x0
000040FB  C6050C270200C0    mov byte [dword 0x2270c],0xc0
00004102  C6050D270200A0    mov byte [dword 0x2270d],0xa0
00004109  C6050E270200C0    mov byte [dword 0x2270e],0xc0
00004110  C6050F270200A0    mov byte [dword 0x2270f],0xa0
00004117  C60510270200A0    mov byte [dword 0x22710],0xa0
0000411E  C6051127020000    mov byte [dword 0x22711],0x0
00004125  C6051227020060    mov byte [dword 0x22712],0x60
0000412C  C6051327020080    mov byte [dword 0x22713],0x80
00004133  C6051427020040    mov byte [dword 0x22714],0x40
0000413A  C6051527020020    mov byte [dword 0x22715],0x20
00004141  C60516270200C0    mov byte [dword 0x22716],0xc0
00004148  C6051727020000    mov byte [dword 0x22717],0x0
0000414F  C60518270200E0    mov byte [dword 0x22718],0xe0
00004156  C6051927020040    mov byte [dword 0x22719],0x40
0000415D  C6051A27020040    mov byte [dword 0x2271a],0x40
00004164  C6051B27020040    mov byte [dword 0x2271b],0x40
0000416B  C6051C27020040    mov byte [dword 0x2271c],0x40
00004172  C6051D27020000    mov byte [dword 0x2271d],0x0
00004179  C6051E270200A0    mov byte [dword 0x2271e],0xa0
00004180  C6051F270200A0    mov byte [dword 0x2271f],0xa0
00004187  C60520270200A0    mov byte [dword 0x22720],0xa0
0000418E  C60521270200A0    mov byte [dword 0x22721],0xa0
00004195  C6052227020040    mov byte [dword 0x22722],0x40
0000419C  C6052327020000    mov byte [dword 0x22723],0x0
000041A3  C60524270200A0    mov byte [dword 0x22724],0xa0
000041AA  C60525270200A0    mov byte [dword 0x22725],0xa0
000041B1  C60526270200A0    mov byte [dword 0x22726],0xa0
000041B8  C60527270200A0    mov byte [dword 0x22727],0xa0
000041BF  C6052827020040    mov byte [dword 0x22728],0x40
000041C6  C6052927020000    mov byte [dword 0x22729],0x0
000041CD  C6052A270200A0    mov byte [dword 0x2272a],0xa0
000041D4  C6052B270200A0    mov byte [dword 0x2272b],0xa0
000041DB  C6052C270200E0    mov byte [dword 0x2272c],0xe0
000041E2  C6052D270200E0    mov byte [dword 0x2272d],0xe0
000041E9  C6052E270200A0    mov byte [dword 0x2272e],0xa0
000041F0  C6052F27020000    mov byte [dword 0x2272f],0x0
000041F7  C60530270200A0    mov byte [dword 0x22730],0xa0
000041FE  C60531270200A0    mov byte [dword 0x22731],0xa0
00004205  C6053227020040    mov byte [dword 0x22732],0x40
0000420C  C60533270200A0    mov byte [dword 0x22733],0xa0
00004213  C60534270200A0    mov byte [dword 0x22734],0xa0
0000421A  C6053527020000    mov byte [dword 0x22735],0x0
00004221  C60536270200A0    mov byte [dword 0x22736],0xa0
00004228  C60537270200A0    mov byte [dword 0x22737],0xa0
0000422F  C6053827020040    mov byte [dword 0x22738],0x40
00004236  C6053927020040    mov byte [dword 0x22739],0x40
0000423D  C6053A27020040    mov byte [dword 0x2273a],0x40
00004244  C6053B27020000    mov byte [dword 0x2273b],0x0
0000424B  C6053C270200E0    mov byte [dword 0x2273c],0xe0
00004252  C6053D27020020    mov byte [dword 0x2273d],0x20
00004259  C6053E27020040    mov byte [dword 0x2273e],0x40
00004260  C6053F27020080    mov byte [dword 0x2273f],0x80
00004267  C60540270200E0    mov byte [dword 0x22740],0xe0
0000426E  C6054127020000    mov byte [dword 0x22741],0x0
00004275  C60542270200C0    mov byte [dword 0x22742],0xc0
0000427C  C6054327020080    mov byte [dword 0x22743],0x80
00004283  C6054427020080    mov byte [dword 0x22744],0x80
0000428A  C6054527020080    mov byte [dword 0x22745],0x80
00004291  C60546270200C0    mov byte [dword 0x22746],0xc0
00004298  C6054727020000    mov byte [dword 0x22747],0x0
0000429F  C6054827020080    mov byte [dword 0x22748],0x80
000042A6  C6054927020040    mov byte [dword 0x22749],0x40
000042AD  C6054A27020020    mov byte [dword 0x2274a],0x20
000042B4  C6054B27020010    mov byte [dword 0x2274b],0x10
000042BB  C6054C27020000    mov byte [dword 0x2274c],0x0
000042C2  C6054D27020000    mov byte [dword 0x2274d],0x0
000042C9  C6054E270200C0    mov byte [dword 0x2274e],0xc0
000042D0  C6054F27020040    mov byte [dword 0x2274f],0x40
000042D7  C6055027020040    mov byte [dword 0x22750],0x40
000042DE  C6055127020040    mov byte [dword 0x22751],0x40
000042E5  C60552270200C0    mov byte [dword 0x22752],0xc0
000042EC  C6055327020000    mov byte [dword 0x22753],0x0
000042F3  C6055427020040    mov byte [dword 0x22754],0x40
000042FA  C60555270200A0    mov byte [dword 0x22755],0xa0
00004301  C6055627020000    mov byte [dword 0x22756],0x0
00004308  C6055727020000    mov byte [dword 0x22757],0x0
0000430F  C6055827020000    mov byte [dword 0x22758],0x0
00004316  C6055927020000    mov byte [dword 0x22759],0x0
0000431D  C6055A27020000    mov byte [dword 0x2275a],0x0
00004324  C6055B27020000    mov byte [dword 0x2275b],0x0
0000432B  C6055C27020000    mov byte [dword 0x2275c],0x0
00004332  C6055D27020000    mov byte [dword 0x2275d],0x0
00004339  C6055E270200E0    mov byte [dword 0x2275e],0xe0
00004340  C6055F27020000    mov byte [dword 0x2275f],0x0
00004347  C6056027020080    mov byte [dword 0x22760],0x80
0000434E  C6056127020040    mov byte [dword 0x22761],0x40
00004355  C6056227020000    mov byte [dword 0x22762],0x0
0000435C  C6056327020000    mov byte [dword 0x22763],0x0
00004363  C6056427020000    mov byte [dword 0x22764],0x0
0000436A  C6056527020000    mov byte [dword 0x22765],0x0
00004371  C6056627020000    mov byte [dword 0x22766],0x0
00004378  C6056727020060    mov byte [dword 0x22767],0x60
0000437F  C60568270200A0    mov byte [dword 0x22768],0xa0
00004386  C60569270200E0    mov byte [dword 0x22769],0xe0
0000438D  C6056A270200A0    mov byte [dword 0x2276a],0xa0
00004394  C6056B27020000    mov byte [dword 0x2276b],0x0
0000439B  C6056C27020080    mov byte [dword 0x2276c],0x80
000043A2  C6056D270200C0    mov byte [dword 0x2276d],0xc0
000043A9  C6056E270200A0    mov byte [dword 0x2276e],0xa0
000043B0  C6056F270200A0    mov byte [dword 0x2276f],0xa0
000043B7  C60570270200C0    mov byte [dword 0x22770],0xc0
000043BE  C6057127020000    mov byte [dword 0x22771],0x0
000043C5  C6057227020000    mov byte [dword 0x22772],0x0
000043CC  C6057327020060    mov byte [dword 0x22773],0x60
000043D3  C6057427020080    mov byte [dword 0x22774],0x80
000043DA  C6057527020080    mov byte [dword 0x22775],0x80
000043E1  C6057627020060    mov byte [dword 0x22776],0x60
000043E8  C6057727020000    mov byte [dword 0x22777],0x0
000043EF  C6057827020020    mov byte [dword 0x22778],0x20
000043F6  C6057927020060    mov byte [dword 0x22779],0x60
000043FD  C6057A270200A0    mov byte [dword 0x2277a],0xa0
00004404  C6057B270200A0    mov byte [dword 0x2277b],0xa0
0000440B  C6057C27020060    mov byte [dword 0x2277c],0x60
00004412  C6057D27020000    mov byte [dword 0x2277d],0x0
00004419  C6057E27020000    mov byte [dword 0x2277e],0x0
00004420  C6057F27020060    mov byte [dword 0x2277f],0x60
00004427  C60580270200C0    mov byte [dword 0x22780],0xc0
0000442E  C6058127020080    mov byte [dword 0x22781],0x80
00004435  C6058227020060    mov byte [dword 0x22782],0x60
0000443C  C6058327020000    mov byte [dword 0x22783],0x0
00004443  C6058427020060    mov byte [dword 0x22784],0x60
0000444A  C6058527020080    mov byte [dword 0x22785],0x80
00004451  C60586270200E0    mov byte [dword 0x22786],0xe0
00004458  C6058727020080    mov byte [dword 0x22787],0x80
0000445F  C6058827020080    mov byte [dword 0x22788],0x80
00004466  C6058927020000    mov byte [dword 0x22789],0x0
0000446D  C6058A27020000    mov byte [dword 0x2278a],0x0
00004474  C6058B27020070    mov byte [dword 0x2278b],0x70
0000447B  C6058C27020090    mov byte [dword 0x2278c],0x90
00004482  C6058D27020060    mov byte [dword 0x2278d],0x60
00004489  C6058E27020010    mov byte [dword 0x2278e],0x10
00004490  C6058F27020060    mov byte [dword 0x2278f],0x60
00004497  C6059027020080    mov byte [dword 0x22790],0x80
0000449E  C60591270200C0    mov byte [dword 0x22791],0xc0
000044A5  C60592270200A0    mov byte [dword 0x22792],0xa0
000044AC  C60593270200A0    mov byte [dword 0x22793],0xa0
000044B3  C60594270200A0    mov byte [dword 0x22794],0xa0
000044BA  C6059527020000    mov byte [dword 0x22795],0x0
000044C1  C6059627020040    mov byte [dword 0x22796],0x40
000044C8  C6059727020000    mov byte [dword 0x22797],0x0
000044CF  C60598270200C0    mov byte [dword 0x22798],0xc0
000044D6  C6059927020040    mov byte [dword 0x22799],0x40
000044DD  C6059A270200E0    mov byte [dword 0x2279a],0xe0
000044E4  C6059B27020000    mov byte [dword 0x2279b],0x0
000044EB  C6059C27020020    mov byte [dword 0x2279c],0x20
000044F2  C6059D27020000    mov byte [dword 0x2279d],0x0
000044F9  C6059E27020020    mov byte [dword 0x2279e],0x20
00004500  C6059F27020020    mov byte [dword 0x2279f],0x20
00004507  C605A0270200C0    mov byte [dword 0x227a0],0xc0
0000450E  C605A127020000    mov byte [dword 0x227a1],0x0
00004515  C605A227020080    mov byte [dword 0x227a2],0x80
0000451C  C605A3270200A0    mov byte [dword 0x227a3],0xa0
00004523  C605A4270200C0    mov byte [dword 0x227a4],0xc0
0000452A  C605A5270200A0    mov byte [dword 0x227a5],0xa0
00004531  C605A6270200A0    mov byte [dword 0x227a6],0xa0
00004538  C605A727020000    mov byte [dword 0x227a7],0x0
0000453F  C605A8270200C0    mov byte [dword 0x227a8],0xc0
00004546  C605A927020040    mov byte [dword 0x227a9],0x40
0000454D  C605AA27020040    mov byte [dword 0x227aa],0x40
00004554  C605AB27020040    mov byte [dword 0x227ab],0x40
0000455B  C605AC270200E0    mov byte [dword 0x227ac],0xe0
00004562  C605AD27020000    mov byte [dword 0x227ad],0x0
00004569  C605AE27020000    mov byte [dword 0x227ae],0x0
00004570  C605AF270200E0    mov byte [dword 0x227af],0xe0
00004577  C605B0270200E0    mov byte [dword 0x227b0],0xe0
0000457E  C605B1270200A0    mov byte [dword 0x227b1],0xa0
00004585  C605B2270200A0    mov byte [dword 0x227b2],0xa0
0000458C  C605B327020000    mov byte [dword 0x227b3],0x0
00004593  C605B427020000    mov byte [dword 0x227b4],0x0
0000459A  C605B5270200C0    mov byte [dword 0x227b5],0xc0
000045A1  C605B6270200A0    mov byte [dword 0x227b6],0xa0
000045A8  C605B7270200A0    mov byte [dword 0x227b7],0xa0
000045AF  C605B8270200A0    mov byte [dword 0x227b8],0xa0
000045B6  C605B927020000    mov byte [dword 0x227b9],0x0
000045BD  C605BA27020000    mov byte [dword 0x227ba],0x0
000045C4  C605BB27020060    mov byte [dword 0x227bb],0x60
000045CB  C605BC270200A0    mov byte [dword 0x227bc],0xa0
000045D2  C605BD270200A0    mov byte [dword 0x227bd],0xa0
000045D9  C605BE27020060    mov byte [dword 0x227be],0x60
000045E0  C605BF27020000    mov byte [dword 0x227bf],0x0
000045E7  C605C027020000    mov byte [dword 0x227c0],0x0
000045EE  C605C1270200C0    mov byte [dword 0x227c1],0xc0
000045F5  C605C2270200A0    mov byte [dword 0x227c2],0xa0
000045FC  C605C3270200C0    mov byte [dword 0x227c3],0xc0
00004603  C605C427020080    mov byte [dword 0x227c4],0x80
0000460A  C605C527020000    mov byte [dword 0x227c5],0x0
00004611  C605C627020000    mov byte [dword 0x227c6],0x0
00004618  C605C727020060    mov byte [dword 0x227c7],0x60
0000461F  C605C8270200A0    mov byte [dword 0x227c8],0xa0
00004626  C605C927020060    mov byte [dword 0x227c9],0x60
0000462D  C605CA27020020    mov byte [dword 0x227ca],0x20
00004634  C605CB27020000    mov byte [dword 0x227cb],0x0
0000463B  C605CC27020000    mov byte [dword 0x227cc],0x0
00004642  C605CD270200A0    mov byte [dword 0x227cd],0xa0
00004649  C605CE270200C0    mov byte [dword 0x227ce],0xc0
00004650  C605CF27020080    mov byte [dword 0x227cf],0x80
00004657  C605D027020080    mov byte [dword 0x227d0],0x80
0000465E  C605D127020000    mov byte [dword 0x227d1],0x0
00004665  C605D227020000    mov byte [dword 0x227d2],0x0
0000466C  C605D327020060    mov byte [dword 0x227d3],0x60
00004673  C605D427020040    mov byte [dword 0x227d4],0x40
0000467A  C605D527020020    mov byte [dword 0x227d5],0x20
00004681  C605D6270200C0    mov byte [dword 0x227d6],0xc0
00004688  C605D727020000    mov byte [dword 0x227d7],0x0
0000468F  C605D827020040    mov byte [dword 0x227d8],0x40
00004696  C605D9270200E0    mov byte [dword 0x227d9],0xe0
0000469D  C605DA27020040    mov byte [dword 0x227da],0x40
000046A4  C605DB27020040    mov byte [dword 0x227db],0x40
000046AB  C605DC27020060    mov byte [dword 0x227dc],0x60
000046B2  C605DD27020000    mov byte [dword 0x227dd],0x0
000046B9  C605DE27020000    mov byte [dword 0x227de],0x0
000046C0  C605DF270200A0    mov byte [dword 0x227df],0xa0
000046C7  C605E0270200A0    mov byte [dword 0x227e0],0xa0
000046CE  C605E1270200A0    mov byte [dword 0x227e1],0xa0
000046D5  C605E227020060    mov byte [dword 0x227e2],0x60
000046DC  C605E327020000    mov byte [dword 0x227e3],0x0
000046E3  C605E427020000    mov byte [dword 0x227e4],0x0
000046EA  C605E5270200A0    mov byte [dword 0x227e5],0xa0
000046F1  C605E6270200A0    mov byte [dword 0x227e6],0xa0
000046F8  C605E727020060    mov byte [dword 0x227e7],0x60
000046FF  C605E827020040    mov byte [dword 0x227e8],0x40
00004706  C605E927020000    mov byte [dword 0x227e9],0x0
0000470D  C605EA27020000    mov byte [dword 0x227ea],0x0
00004714  C605EB270200A0    mov byte [dword 0x227eb],0xa0
0000471B  C605EC270200E0    mov byte [dword 0x227ec],0xe0
00004722  C605ED270200E0    mov byte [dword 0x227ed],0xe0
00004729  C605EE270200A0    mov byte [dword 0x227ee],0xa0
00004730  C605EF27020000    mov byte [dword 0x227ef],0x0
00004737  C605F027020000    mov byte [dword 0x227f0],0x0
0000473E  C605F1270200A0    mov byte [dword 0x227f1],0xa0
00004745  C605F227020040    mov byte [dword 0x227f2],0x40
0000474C  C605F3270200A0    mov byte [dword 0x227f3],0xa0
00004753  C605F4270200A0    mov byte [dword 0x227f4],0xa0
0000475A  C605F527020000    mov byte [dword 0x227f5],0x0
00004761  C605F627020000    mov byte [dword 0x227f6],0x0
00004768  C605F7270200A0    mov byte [dword 0x227f7],0xa0
0000476F  C605F8270200A0    mov byte [dword 0x227f8],0xa0
00004776  C605F927020060    mov byte [dword 0x227f9],0x60
0000477D  C605FA27020040    mov byte [dword 0x227fa],0x40
00004784  C605FB27020040    mov byte [dword 0x227fb],0x40
0000478B  C605FC27020000    mov byte [dword 0x227fc],0x0
00004792  C605FD270200E0    mov byte [dword 0x227fd],0xe0
00004799  C605FE27020020    mov byte [dword 0x227fe],0x20
000047A0  C605FF27020040    mov byte [dword 0x227ff],0x40
000047A7  C60500280200E0    mov byte [dword 0x22800],0xe0
000047AE  C6050128020000    mov byte [dword 0x22801],0x0
000047B5  C6050228020060    mov byte [dword 0x22802],0x60
000047BC  C6050328020040    mov byte [dword 0x22803],0x40
000047C3  C6050428020080    mov byte [dword 0x22804],0x80
000047CA  C6050528020040    mov byte [dword 0x22805],0x40
000047D1  C6050628020060    mov byte [dword 0x22806],0x60
000047D8  C6050728020000    mov byte [dword 0x22807],0x0
000047DF  C6050828020040    mov byte [dword 0x22808],0x40
000047E6  C6050928020040    mov byte [dword 0x22809],0x40
000047ED  C6050A28020040    mov byte [dword 0x2280a],0x40
000047F4  C6050B28020040    mov byte [dword 0x2280b],0x40
000047FB  C6050C28020040    mov byte [dword 0x2280c],0x40
00004802  C6050D28020000    mov byte [dword 0x2280d],0x0
00004809  C6050E280200C0    mov byte [dword 0x2280e],0xc0
00004810  C6050F28020040    mov byte [dword 0x2280f],0x40
00004817  C6051028020020    mov byte [dword 0x22810],0x20
0000481E  C6051128020040    mov byte [dword 0x22811],0x40
00004825  C60512280200C0    mov byte [dword 0x22812],0xc0
0000482C  C6051328020000    mov byte [dword 0x22813],0x0
00004833  C6051428020050    mov byte [dword 0x22814],0x50
0000483A  C60515280200A0    mov byte [dword 0x22815],0xa0
00004841  C6051628020000    mov byte [dword 0x22816],0x0
00004848  C6051728020000    mov byte [dword 0x22817],0x0
0000484F  C6051828020000    mov byte [dword 0x22818],0x0
00004856  C6051928020000    mov byte [dword 0x22819],0x0
0000485D  C705202802000100  mov dword [dword 0x22820],0x1
         -0000
00004867  90                nop
00004868  C9                leave
00004869  C3                ret
0000486A  55                push ebp
0000486B  89E5              mov ebp,esp
0000486D  83EC14            sub esp,byte +0x14
00004870  8B4510            mov eax,[ebp+0x10]
00004873  8845EC            mov [ebp-0x14],al
00004876  807DEC00          cmp byte [ebp-0x14],0x0
0000487A  0F882E010000      js near 0x49ae
00004880  A120280200        mov eax,[0x22820]
00004885  85C0              test eax,eax
00004887  7505              jnz 0x488e
00004889  E81FF0FFFF        call 0x38ad
0000488E  837D140F          cmp dword [ebp+0x14],byte +0xf
00004892  770D              ja 0x48a1
00004894  8B4514            mov eax,[ebp+0x14]
00004897  8B0485E0DB0100    mov eax,[eax*4+0x1dbe0]
0000489E  894514            mov [ebp+0x14],eax
000048A1  0FBE55EC          movsx edx,byte [ebp-0x14]
000048A5  89D0              mov eax,edx
000048A7  01C0              add eax,eax
000048A9  01D0              add eax,edx
000048AB  01C0              add eax,eax
000048AD  0520250200        add eax,0x22520
000048B2  8945F4            mov [ebp-0xc],eax
000048B5  C745FC00000000    mov dword [ebp-0x4],0x0
000048BC  E9E1000000        jmp 0x49a2
000048C1  8B55FC            mov edx,[ebp-0x4]
000048C4  8B45F4            mov eax,[ebp-0xc]
000048C7  01D0              add eax,edx
000048C9  0FB600            movzx eax,byte [eax]
000048CC  8845F3            mov [ebp-0xd],al
000048CF  C745F800000000    mov dword [ebp-0x8],0x0
000048D6  E9B9000000        jmp 0x4994
000048DB  0FB655F3          movzx edx,byte [ebp-0xd]
000048DF  B807000000        mov eax,0x7
000048E4  2B45F8            sub eax,[ebp-0x8]
000048E7  89C1              mov ecx,eax
000048E9  D3FA              sar edx,cl
000048EB  89D0              mov eax,edx
000048ED  83E001            and eax,byte +0x1
000048F0  85C0              test eax,eax
000048F2  0F8498000000      jz near 0x4990
000048F8  8B45FC            mov eax,[ebp-0x4]
000048FB  8D1400            lea edx,[eax+eax]
000048FE  8B450C            mov eax,[ebp+0xc]
00004901  01C2              add edx,eax
00004903  8B45F8            mov eax,[ebp-0x8]
00004906  8D0C00            lea ecx,[eax+eax]
00004909  8B4508            mov eax,[ebp+0x8]
0000490C  01C8              add eax,ecx
0000490E  FF7514            push dword [ebp+0x14]
00004911  52                push edx
00004912  50                push eax
00004913  E877EBFFFF        call 0x348f
00004918  83C40C            add esp,byte +0xc
0000491B  8B45FC            mov eax,[ebp-0x4]
0000491E  8D1400            lea edx,[eax+eax]
00004921  8B450C            mov eax,[ebp+0xc]
00004924  01C2              add edx,eax
00004926  8B45F8            mov eax,[ebp-0x8]
00004929  8D0C00            lea ecx,[eax+eax]
0000492C  8B4508            mov eax,[ebp+0x8]
0000492F  01C8              add eax,ecx
00004931  83C001            add eax,byte +0x1
00004934  FF7514            push dword [ebp+0x14]
00004937  52                push edx
00004938  50                push eax
00004939  E851EBFFFF        call 0x348f
0000493E  83C40C            add esp,byte +0xc
00004941  8B45FC            mov eax,[ebp-0x4]
00004944  8D1400            lea edx,[eax+eax]
00004947  8B450C            mov eax,[ebp+0xc]
0000494A  01D0              add eax,edx
0000494C  8D5001            lea edx,[eax+0x1]
0000494F  8B45F8            mov eax,[ebp-0x8]
00004952  8D0C00            lea ecx,[eax+eax]
00004955  8B4508            mov eax,[ebp+0x8]
00004958  01C8              add eax,ecx
0000495A  FF7514            push dword [ebp+0x14]
0000495D  52                push edx
0000495E  50                push eax
0000495F  E82BEBFFFF        call 0x348f
00004964  83C40C            add esp,byte +0xc
00004967  8B45FC            mov eax,[ebp-0x4]
0000496A  8D1400            lea edx,[eax+eax]
0000496D  8B450C            mov eax,[ebp+0xc]
00004970  01D0              add eax,edx
00004972  8D5001            lea edx,[eax+0x1]
00004975  8B45F8            mov eax,[ebp-0x8]
00004978  8D0C00            lea ecx,[eax+eax]
0000497B  8B4508            mov eax,[ebp+0x8]
0000497E  01C8              add eax,ecx
00004980  83C001            add eax,byte +0x1
00004983  FF7514            push dword [ebp+0x14]
00004986  52                push edx
00004987  50                push eax
00004988  E802EBFFFF        call 0x348f
0000498D  83C40C            add esp,byte +0xc
00004990  8345F801          add dword [ebp-0x8],byte +0x1
00004994  837DF803          cmp dword [ebp-0x8],byte +0x3
00004998  0F8E3DFFFFFF      jng near 0x48db
0000499E  8345FC01          add dword [ebp-0x4],byte +0x1
000049A2  837DFC05          cmp dword [ebp-0x4],byte +0x5
000049A6  0F8E15FFFFFF      jng near 0x48c1
000049AC  EB01              jmp short 0x49af
000049AE  90                nop
000049AF  C9                leave
000049B0  C3                ret
000049B1  55                push ebp
000049B2  89E5              mov ebp,esp
000049B4  EB39              jmp short 0x49ef
000049B6  8B4510            mov eax,[ebp+0x10]
000049B9  8D5001            lea edx,[eax+0x1]
000049BC  895510            mov [ebp+0x10],edx
000049BF  0FB600            movzx eax,byte [eax]
000049C2  0FBEC0            movsx eax,al
000049C5  FF7514            push dword [ebp+0x14]
000049C8  50                push eax
000049C9  FF750C            push dword [ebp+0xc]
000049CC  FF7508            push dword [ebp+0x8]
000049CF  E896FEFFFF        call 0x486a
000049D4  83C410            add esp,byte +0x10
000049D7  8345080A          add dword [ebp+0x8],byte +0xa
000049DB  817D08FF030000    cmp dword [ebp+0x8],0x3ff
000049E2  7E0B              jng 0x49ef
000049E4  C7450800000000    mov dword [ebp+0x8],0x0
000049EB  83450C10          add dword [ebp+0xc],byte +0x10
000049EF  8B4510            mov eax,[ebp+0x10]
000049F2  0FB600            movzx eax,byte [eax]
000049F5  84C0              test al,al
000049F7  75BD              jnz 0x49b6
000049F9  90                nop
000049FA  C9                leave
000049FB  C3                ret
000049FC  55                push ebp
000049FD  89E5              mov ebp,esp
000049FF  83EC10            sub esp,byte +0x10
00004A02  837D0CFF          cmp dword [ebp+0xc],byte -0x1
00004A06  0F94C0            setz al
00004A09  0FB6C0            movzx eax,al
00004A0C  8945F4            mov [ebp-0xc],eax
00004A0F  837DF400          cmp dword [ebp-0xc],byte +0x0
00004A13  7407              jz 0x4a1c
00004A15  A124280200        mov eax,[0x22824]
00004A1A  EB03              jmp short 0x4a1f
00004A1C  8B4508            mov eax,[ebp+0x8]
00004A1F  8945FC            mov [ebp-0x4],eax
00004A22  837DF400          cmp dword [ebp-0xc],byte +0x0
00004A26  7407              jz 0x4a2f
00004A28  A128280200        mov eax,[0x22828]
00004A2D  EB03              jmp short 0x4a32
00004A2F  8B450C            mov eax,[ebp+0xc]
00004A32  8945F8            mov [ebp-0x8],eax
00004A35  E981000000        jmp 0x4abb
00004A3A  8B4510            mov eax,[ebp+0x10]
00004A3D  8D5001            lea edx,[eax+0x1]
00004A40  895510            mov [ebp+0x10],edx
00004A43  0FB600            movzx eax,byte [eax]
00004A46  8845F3            mov [ebp-0xd],al
00004A49  807DF30A          cmp byte [ebp-0xd],0xa
00004A4D  751D              jnz 0x4a6c
00004A4F  C745FC00000000    mov dword [ebp-0x4],0x0
00004A56  8345F810          add dword [ebp-0x8],byte +0x10
00004A5A  817DF8FF020000    cmp dword [ebp-0x8],0x2ff
00004A61  7E58              jng 0x4abb
00004A63  C745F800000000    mov dword [ebp-0x8],0x0
00004A6A  EB4F              jmp short 0x4abb
00004A6C  807DF30D          cmp byte [ebp-0xd],0xd
00004A70  7509              jnz 0x4a7b
00004A72  C745FC00000000    mov dword [ebp-0x4],0x0
00004A79  EB40              jmp short 0x4abb
00004A7B  807DF308          cmp byte [ebp-0xd],0x8
00004A7F  750C              jnz 0x4a8d
00004A81  837DFC09          cmp dword [ebp-0x4],byte +0x9
00004A85  7E34              jng 0x4abb
00004A87  836DFC0A          sub dword [ebp-0x4],byte +0xa
00004A8B  EB2E              jmp short 0x4abb
00004A8D  0FBE45F3          movsx eax,byte [ebp-0xd]
00004A91  FF7514            push dword [ebp+0x14]
00004A94  50                push eax
00004A95  FF75F8            push dword [ebp-0x8]
00004A98  FF75FC            push dword [ebp-0x4]
00004A9B  E8CAFDFFFF        call 0x486a
00004AA0  83C410            add esp,byte +0x10
00004AA3  8345FC0A          add dword [ebp-0x4],byte +0xa
00004AA7  817DFCFF030000    cmp dword [ebp-0x4],0x3ff
00004AAE  7E0B              jng 0x4abb
00004AB0  C745FC00000000    mov dword [ebp-0x4],0x0
00004AB7  8345F810          add dword [ebp-0x8],byte +0x10
00004ABB  8B4510            mov eax,[ebp+0x10]
00004ABE  0FB600            movzx eax,byte [eax]
00004AC1  84C0              test al,al
00004AC3  0F8571FFFFFF      jnz near 0x4a3a
00004AC9  837DF400          cmp dword [ebp-0xc],byte +0x0
00004ACD  7410              jz 0x4adf
00004ACF  8B45FC            mov eax,[ebp-0x4]
00004AD2  A324280200        mov [0x22824],eax
00004AD7  8B45F8            mov eax,[ebp-0x8]
00004ADA  A328280200        mov [0x22828],eax
00004ADF  E8F4EAFFFF        call 0x35d8
00004AE4  90                nop
00004AE5  C9                leave
00004AE6  C3                ret
00004AE7  55                push ebp
00004AE8  89E5              mov ebp,esp
00004AEA  83EC30            sub esp,byte +0x30
00004AED  C745FC00000000    mov dword [ebp-0x4],0x0
00004AF4  837D0800          cmp dword [ebp+0x8],byte +0x0
00004AF8  7513              jnz 0x4b0d
00004AFA  8B45FC            mov eax,[ebp-0x4]
00004AFD  8D5001            lea edx,[eax+0x1]
00004B00  8955FC            mov [ebp-0x4],edx
00004B03  C64405E030        mov byte [ebp+eax-0x20],0x30
00004B08  E98B000000        jmp 0x4b98
00004B0D  8B4508            mov eax,[ebp+0x8]
00004B10  8945F8            mov [ebp-0x8],eax
00004B13  C745F400000000    mov dword [ebp-0xc],0x0
00004B1A  837DF800          cmp dword [ebp-0x8],byte +0x0
00004B1E  795E              jns 0x4b7e
00004B20  C745F401000000    mov dword [ebp-0xc],0x1
00004B27  F75DF8            neg dword [ebp-0x8]
00004B2A  EB52              jmp short 0x4b7e
00004B2C  8B4DF8            mov ecx,[ebp-0x8]
00004B2F  BA67666666        mov edx,0x66666667
00004B34  89C8              mov eax,ecx
00004B36  F7EA              imul edx
00004B38  C1FA02            sar edx,byte 0x2
00004B3B  89C8              mov eax,ecx
00004B3D  C1F81F            sar eax,byte 0x1f
00004B40  29C2              sub edx,eax
00004B42  89D0              mov eax,edx
00004B44  C1E002            shl eax,byte 0x2
00004B47  01D0              add eax,edx
00004B49  01C0              add eax,eax
00004B4B  29C1              sub ecx,eax
00004B4D  89CA              mov edx,ecx
00004B4F  89D0              mov eax,edx
00004B51  8D4830            lea ecx,[eax+0x30]
00004B54  8B45FC            mov eax,[ebp-0x4]
00004B57  8D5001            lea edx,[eax+0x1]
00004B5A  8955FC            mov [ebp-0x4],edx
00004B5D  89CA              mov edx,ecx
00004B5F  885405E0          mov [ebp+eax-0x20],dl
00004B63  8B4DF8            mov ecx,[ebp-0x8]
00004B66  BA67666666        mov edx,0x66666667
00004B6B  89C8              mov eax,ecx
00004B6D  F7EA              imul edx
00004B6F  C1FA02            sar edx,byte 0x2
00004B72  89C8              mov eax,ecx
00004B74  C1F81F            sar eax,byte 0x1f
00004B77  29C2              sub edx,eax
00004B79  89D0              mov eax,edx
00004B7B  8945F8            mov [ebp-0x8],eax
00004B7E  837DF800          cmp dword [ebp-0x8],byte +0x0
00004B82  7FA8              jg 0x4b2c
00004B84  837DF400          cmp dword [ebp-0xc],byte +0x0
00004B88  740E              jz 0x4b98
00004B8A  8B45FC            mov eax,[ebp-0x4]
00004B8D  8D5001            lea edx,[eax+0x1]
00004B90  8955FC            mov [ebp-0x4],edx
00004B93  C64405E02D        mov byte [ebp+eax-0x20],0x2d
00004B98  C745F000000000    mov dword [ebp-0x10],0x0
00004B9F  EB1C              jmp short 0x4bbd
00004BA1  836DFC01          sub dword [ebp-0x4],byte +0x1
00004BA5  8B45F0            mov eax,[ebp-0x10]
00004BA8  8D5001            lea edx,[eax+0x1]
00004BAB  8955F0            mov [ebp-0x10],edx
00004BAE  8D4DE0            lea ecx,[ebp-0x20]
00004BB1  8B55FC            mov edx,[ebp-0x4]
00004BB4  01CA              add edx,ecx
00004BB6  0FB612            movzx edx,byte [edx]
00004BB9  885405D0          mov [ebp+eax-0x30],dl
00004BBD  837DFC00          cmp dword [ebp-0x4],byte +0x0
00004BC1  7FDE              jg 0x4ba1
00004BC3  8D55D0            lea edx,[ebp-0x30]
00004BC6  8B45F0            mov eax,[ebp-0x10]
00004BC9  01D0              add eax,edx
00004BCB  C60000            mov byte [eax],0x0
00004BCE  68FFFFFF00        push dword 0xffffff
00004BD3  8D45D0            lea eax,[ebp-0x30]
00004BD6  50                push eax
00004BD7  6AFF              push byte -0x1
00004BD9  6A00              push byte +0x0
00004BDB  E81CFEFFFF        call 0x49fc
00004BE0  83C410            add esp,byte +0x10
00004BE3  90                nop
00004BE4  C9                leave
00004BE5  C3                ret
00004BE6  55                push ebp
00004BE7  89E5              mov ebp,esp
00004BE9  83EC08            sub esp,byte +0x8
00004BEC  8B5508            mov edx,[ebp+0x8]
00004BEF  8B450C            mov eax,[ebp+0xc]
00004BF2  668955FC          mov [ebp-0x4],dx
00004BF6  8845F8            mov [ebp-0x8],al
00004BF9  0FB645F8          movzx eax,byte [ebp-0x8]
00004BFD  0FB755FC          movzx edx,word [ebp-0x4]
00004C01  EE                out dx,al
00004C02  90                nop
00004C03  C9                leave
00004C04  C3                ret
00004C05  55                push ebp
00004C06  89E5              mov ebp,esp
00004C08  83EC14            sub esp,byte +0x14
00004C0B  8B4508            mov eax,[ebp+0x8]
00004C0E  668945EC          mov [ebp-0x14],ax
00004C12  0FB745EC          movzx eax,word [ebp-0x14]
00004C16  89C2              mov edx,eax
00004C18  EC                in al,dx
00004C19  8845FF            mov [ebp-0x1],al
00004C1C  0FB645FF          movzx eax,byte [ebp-0x1]
00004C20  C9                leave
00004C21  C3                ret
00004C22  55                push ebp
00004C23  89E5              mov ebp,esp
00004C25  6A00              push byte +0x0
00004C27  68F9030000        push dword 0x3f9
00004C2C  E8B5FFFFFF        call 0x4be6
00004C31  83C408            add esp,byte +0x8
00004C34  6880000000        push dword 0x80
00004C39  68FB030000        push dword 0x3fb
00004C3E  E8A3FFFFFF        call 0x4be6
00004C43  83C408            add esp,byte +0x8
00004C46  6A03              push byte +0x3
00004C48  68F8030000        push dword 0x3f8
00004C4D  E894FFFFFF        call 0x4be6
00004C52  83C408            add esp,byte +0x8
00004C55  6A00              push byte +0x0
00004C57  68F9030000        push dword 0x3f9
00004C5C  E885FFFFFF        call 0x4be6
00004C61  83C408            add esp,byte +0x8
00004C64  6A03              push byte +0x3
00004C66  68FB030000        push dword 0x3fb
00004C6B  E876FFFFFF        call 0x4be6
00004C70  83C408            add esp,byte +0x8
00004C73  68C7000000        push dword 0xc7
00004C78  68FA030000        push dword 0x3fa
00004C7D  E864FFFFFF        call 0x4be6
00004C82  83C408            add esp,byte +0x8
00004C85  6A0B              push byte +0xb
00004C87  68FC030000        push dword 0x3fc
00004C8C  E855FFFFFF        call 0x4be6
00004C91  83C408            add esp,byte +0x8
00004C94  90                nop
00004C95  C9                leave
00004C96  C3                ret
00004C97  55                push ebp
00004C98  89E5              mov ebp,esp
00004C9A  83EC04            sub esp,byte +0x4
00004C9D  8B4508            mov eax,[ebp+0x8]
00004CA0  8845FC            mov [ebp-0x4],al
00004CA3  0FB645FC          movzx eax,byte [ebp-0x4]
00004CA7  0FB6C0            movzx eax,al
00004CAA  50                push eax
00004CAB  68E9000000        push dword 0xe9
00004CB0  E831FFFFFF        call 0x4be6
00004CB5  83C408            add esp,byte +0x8
00004CB8  90                nop
00004CB9  C9                leave
00004CBA  C3                ret
00004CBB  55                push ebp
00004CBC  89E5              mov ebp,esp
00004CBE  EB18              jmp short 0x4cd8
00004CC0  8B4508            mov eax,[ebp+0x8]
00004CC3  8D5001            lea edx,[eax+0x1]
00004CC6  895508            mov [ebp+0x8],edx
00004CC9  0FB600            movzx eax,byte [eax]
00004CCC  0FBEC0            movsx eax,al
00004CCF  50                push eax
00004CD0  E8C2FFFFFF        call 0x4c97
00004CD5  83C404            add esp,byte +0x4
00004CD8  8B4508            mov eax,[ebp+0x8]
00004CDB  0FB600            movzx eax,byte [eax]
00004CDE  84C0              test al,al
00004CE0  75DE              jnz 0x4cc0
00004CE2  90                nop
00004CE3  C9                leave
00004CE4  C3                ret
00004CE5  55                push ebp
00004CE6  89E5              mov ebp,esp
00004CE8  A12C280200        mov eax,[0x2282c]
00004CED  83C001            add eax,byte +0x1
00004CF0  A32C280200        mov [0x2282c],eax
00004CF5  90                nop
00004CF6  5D                pop ebp
00004CF7  C3                ret
00004CF8  55                push ebp
00004CF9  89E5              mov ebp,esp
00004CFB  83EC18            sub esp,byte +0x18
00004CFE  83EC08            sub esp,byte +0x8
00004D01  68E54C0100        push dword 0x14ce5
00004D06  6A20              push byte +0x20
00004D08  E8BBBCFFFF        call 0x9c8
00004D0D  83C410            add esp,byte +0x10
00004D10  B8DC341200        mov eax,0x1234dc
00004D15  BA00000000        mov edx,0x0
00004D1A  F77508            div dword [ebp+0x8]
00004D1D  8945F4            mov [ebp-0xc],eax
00004D20  83EC08            sub esp,byte +0x8
00004D23  6A36              push byte +0x36
00004D25  6A43              push byte +0x43
00004D27  E8BAFEFFFF        call 0x4be6
00004D2C  83C410            add esp,byte +0x10
00004D2F  8B45F4            mov eax,[ebp-0xc]
00004D32  8845F3            mov [ebp-0xd],al
00004D35  8B45F4            mov eax,[ebp-0xc]
00004D38  C1E808            shr eax,byte 0x8
00004D3B  8845F2            mov [ebp-0xe],al
00004D3E  0FB645F3          movzx eax,byte [ebp-0xd]
00004D42  83EC08            sub esp,byte +0x8
00004D45  50                push eax
00004D46  6A40              push byte +0x40
00004D48  E899FEFFFF        call 0x4be6
00004D4D  83C410            add esp,byte +0x10
00004D50  0FB645F2          movzx eax,byte [ebp-0xe]
00004D54  83EC08            sub esp,byte +0x8
00004D57  50                push eax
00004D58  6A40              push byte +0x40
00004D5A  E887FEFFFF        call 0x4be6
00004D5F  83C410            add esp,byte +0x10
00004D62  90                nop
00004D63  C9                leave
00004D64  C3                ret
00004D65  55                push ebp
00004D66  89E5              mov ebp,esp
00004D68  83EC14            sub esp,byte +0x14
00004D6B  8B4508            mov eax,[ebp+0x8]
00004D6E  668945EC          mov [ebp-0x14],ax
00004D72  0FB745EC          movzx eax,word [ebp-0x14]
00004D76  89C2              mov edx,eax
00004D78  EC                in al,dx
00004D79  8845FF            mov [ebp-0x1],al
00004D7C  0FB645FF          movzx eax,byte [ebp-0x1]
00004D80  C9                leave
00004D81  C3                ret
00004D82  55                push ebp
00004D83  89E5              mov ebp,esp
00004D85  83EC04            sub esp,byte +0x4
00004D88  8B4508            mov eax,[ebp+0x8]
00004D8B  8845FC            mov [ebp-0x4],al
00004D8E  0FB645FC          movzx eax,byte [ebp-0x4]
00004D92  A220DC0100        mov [0x1dc20],al
00004D97  90                nop
00004D98  C9                leave
00004D99  C3                ret
00004D9A  55                push ebp
00004D9B  89E5              mov ebp,esp
00004D9D  C705382802000000  mov dword [dword 0x22838],0x0
         -0000
00004DA7  C7053C2802000000  mov dword [dword 0x2283c],0x0
         -0000
00004DB1  90                nop
00004DB2  5D                pop ebp
00004DB3  C3                ret
00004DB4  55                push ebp
00004DB5  89E5              mov ebp,esp
00004DB7  A144280200        mov eax,[0x22844]
00004DBC  85C0              test eax,eax
00004DBE  7411              jz 0x4dd1
00004DC0  C705442802000000  mov dword [dword 0x22844],0x0
         -0000
00004DCA  B880350200        mov eax,0x23580
00004DCF  EB05              jmp short 0x4dd6
00004DD1  B800000000        mov eax,0x0
00004DD6  5D                pop ebp
00004DD7  C3                ret
00004DD8  55                push ebp
00004DD9  89E5              mov ebp,esp
00004DDB  C705402802000000  mov dword [dword 0x22840],0x0
         -0000
00004DE5  C6058035020000    mov byte [dword 0x23580],0x0
00004DEC  90                nop
00004DED  5D                pop ebp
00004DEE  C3                ret
00004DEF  55                push ebp
00004DF0  89E5              mov ebp,esp
00004DF2  83EC10            sub esp,byte +0x10
00004DF5  A140280200        mov eax,[0x22840]
00004DFA  85C0              test eax,eax
00004DFC  7E4C              jng 0x4e4a
00004DFE  0FB60580350200    movzx eax,byte [dword 0x23580]
00004E05  8845FB            mov [ebp-0x5],al
00004E08  C745FC00000000    mov dword [ebp-0x4],0x0
00004E0F  EB1C              jmp short 0x4e2d
00004E11  8B45FC            mov eax,[ebp-0x4]
00004E14  83C001            add eax,byte +0x1
00004E17  0FB68080350200    movzx eax,byte [eax+0x23580]
00004E1E  8B55FC            mov edx,[ebp-0x4]
00004E21  81C280350200      add edx,0x23580
00004E27  8802              mov [edx],al
00004E29  8345FC01          add dword [ebp-0x4],byte +0x1
00004E2D  A140280200        mov eax,[0x22840]
00004E32  3945FC            cmp [ebp-0x4],eax
00004E35  7CDA              jl 0x4e11
00004E37  A140280200        mov eax,[0x22840]
00004E3C  83E801            sub eax,byte +0x1
00004E3F  A340280200        mov [0x22840],eax
00004E44  0FB645FB          movzx eax,byte [ebp-0x5]
00004E48  EB05              jmp short 0x4e4f
00004E4A  B800000000        mov eax,0x0
00004E4F  C9                leave
00004E50  C3                ret
00004E51  55                push ebp
00004E52  89E5              mov ebp,esp
00004E54  83EC04            sub esp,byte +0x4
00004E57  8B4508            mov eax,[ebp+0x8]
00004E5A  8845FC            mov [ebp-0x4],al
00004E5D  A140280200        mov eax,[0x22840]
00004E62  3DFE000000        cmp eax,0xfe
00004E67  7F24              jg 0x4e8d
00004E69  A140280200        mov eax,[0x22840]
00004E6E  8D5001            lea edx,[eax+0x1]
00004E71  891540280200      mov [dword 0x22840],edx
00004E77  0FB655FC          movzx edx,byte [ebp-0x4]
00004E7B  889080350200      mov [eax+0x23580],dl
00004E81  A140280200        mov eax,[0x22840]
00004E86  C6808035020000    mov byte [eax+0x23580],0x0
00004E8D  90                nop
00004E8E  C9                leave
00004E8F  C3                ret
00004E90  55                push ebp
00004E91  89E5              mov ebp,esp
00004E93  A140280200        mov eax,[0x22840]
00004E98  85C0              test eax,eax
00004E9A  7E19              jng 0x4eb5
00004E9C  A140280200        mov eax,[0x22840]
00004EA1  83E801            sub eax,byte +0x1
00004EA4  A340280200        mov [0x22840],eax
00004EA9  A140280200        mov eax,[0x22840]
00004EAE  C6808035020000    mov byte [eax+0x23580],0x0
00004EB5  90                nop
00004EB6  5D                pop ebp
00004EB7  C3                ret
00004EB8  55                push ebp
00004EB9  89E5              mov ebp,esp
00004EBB  53                push ebx
00004EBC  83EC24            sub esp,byte +0x24
00004EBF  8B4508            mov eax,[ebp+0x8]
00004EC2  8845E4            mov [ebp-0x1c],al
00004EC5  807DE40A          cmp byte [ebp-0x1c],0xa
00004EC9  751C              jnz 0x4ee7
00004ECB  C705382802000000  mov dword [dword 0x22838],0x0
         -0000
00004ED5  A13C280200        mov eax,[0x2283c]
00004EDA  83C006            add eax,byte +0x6
00004EDD  A33C280200        mov [0x2283c],eax
00004EE2  E9AE000000        jmp 0x4f95
00004EE7  807DE408          cmp byte [ebp-0x1c],0x8
00004EEB  7576              jnz 0x4f63
00004EED  A138280200        mov eax,[0x22838]
00004EF2  85C0              test eax,eax
00004EF4  7E2D              jng 0x4f23
00004EF6  A138280200        mov eax,[0x22838]
00004EFB  83E804            sub eax,byte +0x4
00004EFE  A338280200        mov [0x22838],eax
00004F03  8B153C280200      mov edx,[dword 0x2283c]
00004F09  A138280200        mov eax,[0x22838]
00004F0E  83EC0C            sub esp,byte +0xc
00004F11  6A00              push byte +0x0
00004F13  6A06              push byte +0x6
00004F15  6A04              push byte +0x4
00004F17  52                push edx
00004F18  50                push eax
00004F19  E82FE7FFFF        call 0x364d
00004F1E  83C420            add esp,byte +0x20
00004F21  EB72              jmp short 0x4f95
00004F23  A13C280200        mov eax,[0x2283c]
00004F28  85C0              test eax,eax
00004F2A  7E69              jng 0x4f95
00004F2C  A13C280200        mov eax,[0x2283c]
00004F31  83E806            sub eax,byte +0x6
00004F34  A33C280200        mov [0x2283c],eax
00004F39  C705382802003C01  mov dword [dword 0x22838],0x13c
         -0000
00004F43  8B153C280200      mov edx,[dword 0x2283c]
00004F49  A138280200        mov eax,[0x22838]
00004F4E  83EC0C            sub esp,byte +0xc
00004F51  6A00              push byte +0x0
00004F53  6A06              push byte +0x6
00004F55  6A04              push byte +0x4
00004F57  52                push edx
00004F58  50                push eax
00004F59  E8EFE6FFFF        call 0x364d
00004F5E  83C420            add esp,byte +0x20
00004F61  EB32              jmp short 0x4f95
00004F63  0FB60520DC0100    movzx eax,byte [dword 0x1dc20]
00004F6A  0FB6D8            movzx ebx,al
00004F6D  0FBE4DE4          movsx ecx,byte [ebp-0x1c]
00004F71  8B153C280200      mov edx,[dword 0x2283c]
00004F77  A138280200        mov eax,[0x22838]
00004F7C  53                push ebx
00004F7D  51                push ecx
00004F7E  52                push edx
00004F7F  50                push eax
00004F80  E8E5F8FFFF        call 0x486a
00004F85  83C410            add esp,byte +0x10
00004F88  A138280200        mov eax,[0x22838]
00004F8D  83C004            add eax,byte +0x4
00004F90  A338280200        mov [0x22838],eax
00004F95  A138280200        mov eax,[0x22838]
00004F9A  3D3F010000        cmp eax,0x13f
00004F9F  7E17              jng 0x4fb8
00004FA1  C705382802000000  mov dword [dword 0x22838],0x0
         -0000
00004FAB  A13C280200        mov eax,[0x2283c]
00004FB0  83C006            add eax,byte +0x6
00004FB3  A33C280200        mov [0x2283c],eax
00004FB8  A13C280200        mov eax,[0x2283c]
00004FBD  3DC1000000        cmp eax,0xc1
00004FC2  7E60              jng 0x5024
00004FC4  C745F000000A00    mov dword [ebp-0x10],0xa0000
00004FCB  C745F400000000    mov dword [ebp-0xc],0x0
00004FD2  EB1F              jmp short 0x4ff3
00004FD4  8B45F4            mov eax,[ebp-0xc]
00004FD7  8D9080070000      lea edx,[eax+0x780]
00004FDD  8B45F0            mov eax,[ebp-0x10]
00004FE0  01D0              add eax,edx
00004FE2  8B4DF4            mov ecx,[ebp-0xc]
00004FE5  8B55F0            mov edx,[ebp-0x10]
00004FE8  01CA              add edx,ecx
00004FEA  0FB600            movzx eax,byte [eax]
00004FED  8802              mov [edx],al
00004FEF  8345F401          add dword [ebp-0xc],byte +0x1
00004FF3  817DF47FF20000    cmp dword [ebp-0xc],0xf27f
00004FFA  7ED8              jng 0x4fd4
00004FFC  83EC0C            sub esp,byte +0xc
00004FFF  6A00              push byte +0x0
00005001  6A06              push byte +0x6
00005003  6840010000        push dword 0x140
00005008  68C2000000        push dword 0xc2
0000500D  6A00              push byte +0x0
0000500F  E839E6FFFF        call 0x364d
00005014  83C420            add esp,byte +0x20
00005017  A13C280200        mov eax,[0x2283c]
0000501C  83E806            sub eax,byte +0x6
0000501F  A33C280200        mov [0x2283c],eax
00005024  90                nop
00005025  8B5DFC            mov ebx,[ebp-0x4]
00005028  C9                leave
00005029  C3                ret
0000502A  55                push ebp
0000502B  89E5              mov ebp,esp
0000502D  83EC08            sub esp,byte +0x8
00005030  EB1B              jmp short 0x504d
00005032  8B4508            mov eax,[ebp+0x8]
00005035  8D5001            lea edx,[eax+0x1]
00005038  895508            mov [ebp+0x8],edx
0000503B  0FB600            movzx eax,byte [eax]
0000503E  0FBEC0            movsx eax,al
00005041  83EC0C            sub esp,byte +0xc
00005044  50                push eax
00005045  E86EFEFFFF        call 0x4eb8
0000504A  83C410            add esp,byte +0x10
0000504D  8B4508            mov eax,[ebp+0x8]
00005050  0FB600            movzx eax,byte [eax]
00005053  84C0              test al,al
00005055  75DB              jnz 0x5032
00005057  90                nop
00005058  C9                leave
00005059  C3                ret
0000505A  55                push ebp
0000505B  89E5              mov ebp,esp
0000505D  83EC18            sub esp,byte +0x18
00005060  6A60              push byte +0x60
00005062  E8FEFCFFFF        call 0x4d65
00005067  83C404            add esp,byte +0x4
0000506A  8845F6            mov [ebp-0xa],al
0000506D  807DF62A          cmp byte [ebp-0xa],0x2a
00005071  7406              jz 0x5079
00005073  807DF636          cmp byte [ebp-0xa],0x36
00005077  750F              jnz 0x5088
00005079  C705302802000100  mov dword [dword 0x22830],0x1
         -0000
00005083  E997000000        jmp 0x511f
00005088  807DF6AA          cmp byte [ebp-0xa],0xaa
0000508C  7406              jz 0x5094
0000508E  807DF6B6          cmp byte [ebp-0xa],0xb6
00005092  750C              jnz 0x50a0
00005094  C705302802000000  mov dword [dword 0x22830],0x0
         -0000
0000509E  EB7F              jmp short 0x511f
000050A0  0FB645F6          movzx eax,byte [ebp-0xa]
000050A4  84C0              test al,al
000050A6  7873              js 0x511b
000050A8  807DF6E0          cmp byte [ebp-0xa],0xe0
000050AC  7470              jz 0x511e
000050AE  C645F700          mov byte [ebp-0x9],0x0
000050B2  807DF648          cmp byte [ebp-0xa],0x48
000050B6  7506              jnz 0x50be
000050B8  C645F780          mov byte [ebp-0x9],0x80
000050BC  EB45              jmp short 0x5103
000050BE  807DF650          cmp byte [ebp-0xa],0x50
000050C2  7506              jnz 0x50ca
000050C4  C645F781          mov byte [ebp-0x9],0x81
000050C8  EB39              jmp short 0x5103
000050CA  807DF63C          cmp byte [ebp-0xa],0x3c
000050CE  7506              jnz 0x50d6
000050D0  C645F780          mov byte [ebp-0x9],0x80
000050D4  EB2D              jmp short 0x5103
000050D6  807DF659          cmp byte [ebp-0xa],0x59
000050DA  7727              ja 0x5103
000050DC  A130280200        mov eax,[0x22830]
000050E1  85C0              test eax,eax
000050E3  7410              jz 0x50f5
000050E5  0FB645F6          movzx eax,byte [ebp-0xa]
000050E9  0FB680C0B70100    movzx eax,byte [eax+0x1b7c0]
000050F0  8845F7            mov [ebp-0x9],al
000050F3  EB0E              jmp short 0x5103
000050F5  0FB645F6          movzx eax,byte [ebp-0xa]
000050F9  0FB68060B70100    movzx eax,byte [eax+0x1b760]
00005100  8845F7            mov [ebp-0x9],al
00005103  807DF700          cmp byte [ebp-0x9],0x0
00005107  7416              jz 0x511f
00005109  0FBE45F7          movsx eax,byte [ebp-0x9]
0000510D  83EC0C            sub esp,byte +0xc
00005110  50                push eax
00005111  E87F480000        call 0x9995
00005116  83C410            add esp,byte +0x10
00005119  EB04              jmp short 0x511f
0000511B  90                nop
0000511C  EB01              jmp short 0x511f
0000511E  90                nop
0000511F  C9                leave
00005120  C3                ret
00005121  55                push ebp
00005122  89E5              mov ebp,esp
00005124  83EC08            sub esp,byte +0x8
00005127  83EC08            sub esp,byte +0x8
0000512A  685A500100        push dword 0x1505a
0000512F  6A21              push byte +0x21
00005131  E892B8FFFF        call 0x9c8
00005136  83C410            add esp,byte +0x10
00005139  90                nop
0000513A  C9                leave
0000513B  C3                ret
0000513C  55                push ebp
0000513D  89E5              mov ebp,esp
0000513F  83EC08            sub esp,byte +0x8
00005142  A148280200        mov eax,[0x22848]
00005147  85C0              test eax,eax
00005149  7416              jz 0x5161
0000514B  A148280200        mov eax,[0x22848]
00005150  83EC08            sub esp,byte +0x8
00005153  FF7510            push dword [ebp+0x10]
00005156  50                push eax
00005157  E8ED410000        call 0x9349
0000515C  83C410            add esp,byte +0x10
0000515F  EB14              jmp short 0x5175
00005161  FF7514            push dword [ebp+0x14]
00005164  FF7510            push dword [ebp+0x10]
00005167  FF750C            push dword [ebp+0xc]
0000516A  FF7508            push dword [ebp+0x8]
0000516D  E88AF8FFFF        call 0x49fc
00005172  83C410            add esp,byte +0x10
00005175  90                nop
00005176  C9                leave
00005177  C3                ret
00005178  55                push ebp
00005179  89E5              mov ebp,esp
0000517B  83EC08            sub esp,byte +0x8
0000517E  A124DC0100        mov eax,[0x1dc24]
00005183  50                push eax
00005184  681CB80100        push dword 0x1b81c
00005189  6AFF              push byte -0x1
0000518B  6A00              push byte +0x0
0000518D  E8AAFFFFFF        call 0x513c
00005192  83C410            add esp,byte +0x10
00005195  A124DC0100        mov eax,[0x1dc24]
0000519A  50                push eax
0000519B  6838B80100        push dword 0x1b838
000051A0  6AFF              push byte -0x1
000051A2  6A00              push byte +0x0
000051A4  E893FFFFFF        call 0x513c
000051A9  83C410            add esp,byte +0x10
000051AC  A124DC0100        mov eax,[0x1dc24]
000051B1  50                push eax
000051B2  681CB80100        push dword 0x1b81c
000051B7  6AFF              push byte -0x1
000051B9  6A00              push byte +0x0
000051BB  E87CFFFFFF        call 0x513c
000051C0  83C410            add esp,byte +0x10
000051C3  A124DC0100        mov eax,[0x1dc24]
000051C8  50                push eax
000051C9  6854B80100        push dword 0x1b854
000051CE  6AFF              push byte -0x1
000051D0  6A00              push byte +0x0
000051D2  E865FFFFFF        call 0x513c
000051D7  83C410            add esp,byte +0x10
000051DA  A124DC0100        mov eax,[0x1dc24]
000051DF  50                push eax
000051E0  686BB80100        push dword 0x1b86b
000051E5  6AFF              push byte -0x1
000051E7  6A00              push byte +0x0
000051E9  E84EFFFFFF        call 0x513c
000051EE  83C410            add esp,byte +0x10
000051F1  A124DC0100        mov eax,[0x1dc24]
000051F6  50                push eax
000051F7  6882B80100        push dword 0x1b882
000051FC  6AFF              push byte -0x1
000051FE  6A00              push byte +0x0
00005200  E837FFFFFF        call 0x513c
00005205  83C410            add esp,byte +0x10
00005208  A124DC0100        mov eax,[0x1dc24]
0000520D  50                push eax
0000520E  6899B80100        push dword 0x1b899
00005213  6AFF              push byte -0x1
00005215  6A00              push byte +0x0
00005217  E820FFFFFF        call 0x513c
0000521C  83C410            add esp,byte +0x10
0000521F  A124DC0100        mov eax,[0x1dc24]
00005224  50                push eax
00005225  68B4B80100        push dword 0x1b8b4
0000522A  6AFF              push byte -0x1
0000522C  6A00              push byte +0x0
0000522E  E809FFFFFF        call 0x513c
00005233  83C410            add esp,byte +0x10
00005236  A124DC0100        mov eax,[0x1dc24]
0000523B  50                push eax
0000523C  68CDB80100        push dword 0x1b8cd
00005241  6AFF              push byte -0x1
00005243  6A00              push byte +0x0
00005245  E8F2FEFFFF        call 0x513c
0000524A  83C410            add esp,byte +0x10
0000524D  90                nop
0000524E  C9                leave
0000524F  C3                ret
00005250  55                push ebp
00005251  89E5              mov ebp,esp
00005253  83EC18            sub esp,byte +0x18
00005256  A124DC0100        mov eax,[0x1dc24]
0000525B  50                push eax
0000525C  68E3B80100        push dword 0x1b8e3
00005261  6AFF              push byte -0x1
00005263  6A00              push byte +0x0
00005265  E8D2FEFFFF        call 0x513c
0000526A  83C410            add esp,byte +0x10
0000526D  C645F702          mov byte [ebp-0x9],0x2
00005271  EB10              jmp short 0x5283
00005273  83EC0C            sub esp,byte +0xc
00005276  6A64              push byte +0x64
00005278  E888F9FFFF        call 0x4c05
0000527D  83C410            add esp,byte +0x10
00005280  8845F7            mov [ebp-0x9],al
00005283  0FB645F7          movzx eax,byte [ebp-0x9]
00005287  83E002            and eax,byte +0x2
0000528A  85C0              test eax,eax
0000528C  75E5              jnz 0x5273
0000528E  83EC08            sub esp,byte +0x8
00005291  68FE000000        push dword 0xfe
00005296  6A64              push byte +0x64
00005298  E849F9FFFF        call 0x4be6
0000529D  83C410            add esp,byte +0x10
000052A0  F4                hlt
000052A1  90                nop
000052A2  C9                leave
000052A3  C3                ret
000052A4  55                push ebp
000052A5  89E5              mov ebp,esp
000052A7  83EC08            sub esp,byte +0x8
000052AA  A124DC0100        mov eax,[0x1dc24]
000052AF  50                push eax
000052B0  68F8B80100        push dword 0x1b8f8
000052B5  6AFF              push byte -0x1
000052B7  6A00              push byte +0x0
000052B9  E87EFEFFFF        call 0x513c
000052BE  83C410            add esp,byte +0x10
000052C1  A124DC0100        mov eax,[0x1dc24]
000052C6  50                push eax
000052C7  680DB90100        push dword 0x1b90d
000052CC  6AFF              push byte -0x1
000052CE  6A00              push byte +0x0
000052D0  E867FEFFFF        call 0x513c
000052D5  83C410            add esp,byte +0x10
000052D8  A124DC0100        mov eax,[0x1dc24]
000052DD  50                push eax
000052DE  6828B90100        push dword 0x1b928
000052E3  6AFF              push byte -0x1
000052E5  6A00              push byte +0x0
000052E7  E850FEFFFF        call 0x513c
000052EC  83C410            add esp,byte +0x10
000052EF  A124DC0100        mov eax,[0x1dc24]
000052F4  50                push eax
000052F5  6847B90100        push dword 0x1b947
000052FA  6AFF              push byte -0x1
000052FC  6A00              push byte +0x0
000052FE  E839FEFFFF        call 0x513c
00005303  83C410            add esp,byte +0x10
00005306  A124DC0100        mov eax,[0x1dc24]
0000530B  50                push eax
0000530C  6860B90100        push dword 0x1b960
00005311  6AFF              push byte -0x1
00005313  6A00              push byte +0x0
00005315  E822FEFFFF        call 0x513c
0000531A  83C410            add esp,byte +0x10
0000531D  A124DC0100        mov eax,[0x1dc24]
00005322  50                push eax
00005323  6877B90100        push dword 0x1b977
00005328  6AFF              push byte -0x1
0000532A  6A00              push byte +0x0
0000532C  E80BFEFFFF        call 0x513c
00005331  83C410            add esp,byte +0x10
00005334  A124DC0100        mov eax,[0x1dc24]
00005339  50                push eax
0000533A  6895B90100        push dword 0x1b995
0000533F  6AFF              push byte -0x1
00005341  6A00              push byte +0x0
00005343  E8F4FDFFFF        call 0x513c
00005348  83C410            add esp,byte +0x10
0000534B  90                nop
0000534C  C9                leave
0000534D  C3                ret
0000534E  55                push ebp
0000534F  89E5              mov ebp,esp
00005351  83EC58            sub esp,byte +0x58
00005354  8B4508            mov eax,[ebp+0x8]
00005357  A348280200        mov [0x22848],eax
0000535C  83EC0C            sub esp,byte +0xc
0000535F  FF750C            push dword [ebp+0xc]
00005362  E832080000        call 0x5b99
00005367  83C410            add esp,byte +0x10
0000536A  85C0              test eax,eax
0000536C  0F84A8060000      jz near 0x5a1a
00005372  83EC08            sub esp,byte +0x8
00005375  68ACB90100        push dword 0x1b9ac
0000537A  FF750C            push dword [ebp+0xc]
0000537D  E8D8070000        call 0x5b5a
00005382  83C410            add esp,byte +0x10
00005385  85C0              test eax,eax
00005387  750A              jnz 0x5393
00005389  E816FFFFFF        call 0x52a4
0000538E  E988060000        jmp 0x5a1b
00005393  83EC08            sub esp,byte +0x8
00005396  68B1B90100        push dword 0x1b9b1
0000539B  FF750C            push dword [ebp+0xc]
0000539E  E8B7070000        call 0x5b5a
000053A3  83C410            add esp,byte +0x10
000053A6  85C0              test eax,eax
000053A8  750A              jnz 0x53b4
000053AA  E8C9FDFFFF        call 0x5178
000053AF  E967060000        jmp 0x5a1b
000053B4  83EC08            sub esp,byte +0x8
000053B7  68B6B90100        push dword 0x1b9b6
000053BC  FF750C            push dword [ebp+0xc]
000053BF  E896070000        call 0x5b5a
000053C4  83C410            add esp,byte +0x10
000053C7  85C0              test eax,eax
000053C9  7519              jnz 0x53e4
000053CB  A124DC0100        mov eax,[0x1dc24]
000053D0  C1F804            sar eax,byte 0x4
000053D3  83EC0C            sub esp,byte +0xc
000053D6  50                push eax
000053D7  E852E1FFFF        call 0x352e
000053DC  83C410            add esp,byte +0x10
000053DF  E937060000        jmp 0x5a1b
000053E4  83EC08            sub esp,byte +0x8
000053E7  68BCB90100        push dword 0x1b9bc
000053EC  FF750C            push dword [ebp+0xc]
000053EF  E866070000        call 0x5b5a
000053F4  83C410            add esp,byte +0x10
000053F7  85C0              test eax,eax
000053F9  750A              jnz 0x5405
000053FB  E850FEFFFF        call 0x5250
00005400  E916060000        jmp 0x5a1b
00005405  83EC08            sub esp,byte +0x8
00005408  68C3B90100        push dword 0x1b9c3
0000540D  FF750C            push dword [ebp+0xc]
00005410  E845070000        call 0x5b5a
00005415  83C410            add esp,byte +0x10
00005418  85C0              test eax,eax
0000541A  7556              jnz 0x5472
0000541C  A124DC0100        mov eax,[0x1dc24]
00005421  83E00F            and eax,byte +0xf
00005424  8945F4            mov [ebp-0xc],eax
00005427  8B45F4            mov eax,[ebp-0xc]
0000542A  83C001            add eax,byte +0x1
0000542D  83E00F            and eax,byte +0xf
00005430  8945F4            mov [ebp-0xc],eax
00005433  A124DC0100        mov eax,[0x1dc24]
00005438  C1F804            sar eax,byte 0x4
0000543B  3945F4            cmp [ebp-0xc],eax
0000543E  7504              jnz 0x5444
00005440  8345F401          add dword [ebp-0xc],byte +0x1
00005444  A124DC0100        mov eax,[0x1dc24]
00005449  25F0000000        and eax,0xf0
0000544E  0B45F4            or eax,[ebp-0xc]
00005451  A324DC0100        mov [0x1dc24],eax
00005456  A124DC0100        mov eax,[0x1dc24]
0000545B  50                push eax
0000545C  68C9B90100        push dword 0x1b9c9
00005461  6AFF              push byte -0x1
00005463  6A00              push byte +0x0
00005465  E8D2FCFFFF        call 0x513c
0000546A  83C410            add esp,byte +0x10
0000546D  E9A9050000        jmp 0x5a1b
00005472  83EC04            sub esp,byte +0x4
00005475  6A05              push byte +0x5
00005477  68D9B90100        push dword 0x1b9d9
0000547C  FF750C            push dword [ebp+0xc]
0000547F  E8D0070000        call 0x5c54
00005484  83C410            add esp,byte +0x10
00005487  85C0              test eax,eax
00005489  7537              jnz 0x54c2
0000548B  A124DC0100        mov eax,[0x1dc24]
00005490  89C2              mov edx,eax
00005492  8B450C            mov eax,[ebp+0xc]
00005495  83C005            add eax,byte +0x5
00005498  52                push edx
00005499  50                push eax
0000549A  6AFF              push byte -0x1
0000549C  6A00              push byte +0x0
0000549E  E899FCFFFF        call 0x513c
000054A3  83C410            add esp,byte +0x10
000054A6  A124DC0100        mov eax,[0x1dc24]
000054AB  50                push eax
000054AC  68DFB90100        push dword 0x1b9df
000054B1  6AFF              push byte -0x1
000054B3  6A00              push byte +0x0
000054B5  E882FCFFFF        call 0x513c
000054BA  83C410            add esp,byte +0x10
000054BD  E959050000        jmp 0x5a1b
000054C2  83EC08            sub esp,byte +0x8
000054C5  68E1B90100        push dword 0x1b9e1
000054CA  FF750C            push dword [ebp+0xc]
000054CD  E888060000        call 0x5b5a
000054D2  83C410            add esp,byte +0x10
000054D5  85C0              test eax,eax
000054D7  750A              jnz 0x54e3
000054D9  E88FB9FFFF        call 0xe6d
000054DE  E938050000        jmp 0x5a1b
000054E3  83EC08            sub esp,byte +0x8
000054E6  68E4B90100        push dword 0x1b9e4
000054EB  FF750C            push dword [ebp+0xc]
000054EE  E867060000        call 0x5b5a
000054F3  83C410            add esp,byte +0x10
000054F6  85C0              test eax,eax
000054F8  0F85ED010000      jnz near 0x56eb
000054FE  83EC0C            sub esp,byte +0xc
00005501  6800020000        push dword 0x200
00005506  E803B5FFFF        call 0xa0e
0000550B  83C410            add esp,byte +0x10
0000550E  8945D4            mov [ebp-0x2c],eax
00005511  83EC08            sub esp,byte +0x8
00005514  FF75D4            push dword [ebp-0x2c]
00005517  6A00              push byte +0x0
00005519  E8DF080000        call 0x5dfd
0000551E  83C410            add esp,byte +0x10
00005521  85C0              test eax,eax
00005523  0F84AF010000      jz near 0x56d8
00005529  6A07              push byte +0x7
0000552B  68ECB90100        push dword 0x1b9ec
00005530  6AFF              push byte -0x1
00005532  6A00              push byte +0x0
00005534  E803FCFFFF        call 0x513c
00005539  83C410            add esp,byte +0x10
0000553C  6A07              push byte +0x7
0000553E  68FCB90100        push dword 0x1b9fc
00005543  6AFF              push byte -0x1
00005545  6A00              push byte +0x0
00005547  E8F0FBFFFF        call 0x513c
0000554C  83C410            add esp,byte +0x10
0000554F  C745BE30313233    mov dword [ebp-0x42],0x33323130
00005556  C745C234353637    mov dword [ebp-0x3e],0x37363534
0000555D  C745C638394142    mov dword [ebp-0x3a],0x42413938
00005564  C745CA43444546    mov dword [ebp-0x36],0x46454443
0000556B  C645CE00          mov byte [ebp-0x32],0x0
0000556F  8B45D4            mov eax,[ebp-0x2c]
00005572  05FE010000        add eax,0x1fe
00005577  0FB600            movzx eax,byte [eax]
0000557A  C0E804            shr al,byte 0x4
0000557D  0FB6C0            movzx eax,al
00005580  83E00F            and eax,byte +0xf
00005583  0FB64405BE        movzx eax,byte [ebp+eax-0x42]
00005588  8845CF            mov [ebp-0x31],al
0000558B  8B45D4            mov eax,[ebp-0x2c]
0000558E  05FE010000        add eax,0x1fe
00005593  0FB600            movzx eax,byte [eax]
00005596  0FB6C0            movzx eax,al
00005599  83E00F            and eax,byte +0xf
0000559C  0FB64405BE        movzx eax,byte [ebp+eax-0x42]
000055A1  8845D0            mov [ebp-0x30],al
000055A4  8B45D4            mov eax,[ebp-0x2c]
000055A7  05FF010000        add eax,0x1ff
000055AC  0FB600            movzx eax,byte [eax]
000055AF  C0E804            shr al,byte 0x4
000055B2  0FB6C0            movzx eax,al
000055B5  83E00F            and eax,byte +0xf
000055B8  0FB64405BE        movzx eax,byte [ebp+eax-0x42]
000055BD  8845D1            mov [ebp-0x2f],al
000055C0  8B45D4            mov eax,[ebp-0x2c]
000055C3  05FF010000        add eax,0x1ff
000055C8  0FB600            movzx eax,byte [eax]
000055CB  0FB6C0            movzx eax,al
000055CE  83E00F            and eax,byte +0xf
000055D1  0FB64405BE        movzx eax,byte [ebp+eax-0x42]
000055D6  8845D2            mov [ebp-0x2e],al
000055D9  C645D300          mov byte [ebp-0x2d],0x0
000055DD  6A0E              push byte +0xe
000055DF  8D45CF            lea eax,[ebp-0x31]
000055E2  50                push eax
000055E3  6AFF              push byte -0x1
000055E5  6A00              push byte +0x0
000055E7  E850FBFFFF        call 0x513c
000055EC  83C410            add esp,byte +0x10
000055EF  6A07              push byte +0x7
000055F1  68DFB90100        push dword 0x1b9df
000055F6  6AFF              push byte -0x1
000055F8  6A00              push byte +0x0
000055FA  E83DFBFFFF        call 0x513c
000055FF  83C410            add esp,byte +0x10
00005602  6A07              push byte +0x7
00005604  6802BA0100        push dword 0x1ba02
00005609  6AFF              push byte -0x1
0000560B  6A00              push byte +0x0
0000560D  E82AFBFFFF        call 0x513c
00005612  83C410            add esp,byte +0x10
00005615  C745F000000000    mov dword [ebp-0x10],0x0
0000561C  EB20              jmp short 0x563e
0000561E  8B45F0            mov eax,[ebp-0x10]
00005621  83C003            add eax,byte +0x3
00005624  89C2              mov edx,eax
00005626  8B45D4            mov eax,[ebp-0x2c]
00005629  01D0              add eax,edx
0000562B  0FB600            movzx eax,byte [eax]
0000562E  89C1              mov ecx,eax
00005630  8D55B5            lea edx,[ebp-0x4b]
00005633  8B45F0            mov eax,[ebp-0x10]
00005636  01D0              add eax,edx
00005638  8808              mov [eax],cl
0000563A  8345F001          add dword [ebp-0x10],byte +0x1
0000563E  837DF007          cmp dword [ebp-0x10],byte +0x7
00005642  7EDA              jng 0x561e
00005644  C645BD00          mov byte [ebp-0x43],0x0
00005648  6A0F              push byte +0xf
0000564A  8D45B5            lea eax,[ebp-0x4b]
0000564D  50                push eax
0000564E  6AFF              push byte -0x1
00005650  6A00              push byte +0x0
00005652  E8E5FAFFFF        call 0x513c
00005657  83C410            add esp,byte +0x10
0000565A  6A07              push byte +0x7
0000565C  68DFB90100        push dword 0x1b9df
00005661  6AFF              push byte -0x1
00005663  6A00              push byte +0x0
00005665  E8D2FAFFFF        call 0x513c
0000566A  83C410            add esp,byte +0x10
0000566D  6A07              push byte +0x7
0000566F  6808BA0100        push dword 0x1ba08
00005674  6AFF              push byte -0x1
00005676  6A00              push byte +0x0
00005678  E8BFFAFFFF        call 0x513c
0000567D  83C410            add esp,byte +0x10
00005680  C745EC00000000    mov dword [ebp-0x14],0x0
00005687  EB20              jmp short 0x56a9
00005689  8B45EC            mov eax,[ebp-0x14]
0000568C  83C036            add eax,byte +0x36
0000568F  89C2              mov edx,eax
00005691  8B45D4            mov eax,[ebp-0x2c]
00005694  01D0              add eax,edx
00005696  0FB600            movzx eax,byte [eax]
00005699  89C1              mov ecx,eax
0000569B  8D55AC            lea edx,[ebp-0x54]
0000569E  8B45EC            mov eax,[ebp-0x14]
000056A1  01D0              add eax,edx
000056A3  8808              mov [eax],cl
000056A5  8345EC01          add dword [ebp-0x14],byte +0x1
000056A9  837DEC07          cmp dword [ebp-0x14],byte +0x7
000056AD  7EDA              jng 0x5689
000056AF  C645B400          mov byte [ebp-0x4c],0x0
000056B3  6A0F              push byte +0xf
000056B5  8D45AC            lea eax,[ebp-0x54]
000056B8  50                push eax
000056B9  6AFF              push byte -0x1
000056BB  6A00              push byte +0x0
000056BD  E87AFAFFFF        call 0x513c
000056C2  83C410            add esp,byte +0x10
000056C5  6A07              push byte +0x7
000056C7  68DFB90100        push dword 0x1b9df
000056CC  6AFF              push byte -0x1
000056CE  6A00              push byte +0x0
000056D0  E867FAFFFF        call 0x513c
000056D5  83C410            add esp,byte +0x10
000056D8  83EC0C            sub esp,byte +0xc
000056DB  FF75D4            push dword [ebp-0x2c]
000056DE  E8EFB3FFFF        call 0xad2
000056E3  83C410            add esp,byte +0x10
000056E6  E930030000        jmp 0x5a1b
000056EB  83EC04            sub esp,byte +0x4
000056EE  6A06              push byte +0x6
000056F0  680FBA0100        push dword 0x1ba0f
000056F5  FF750C            push dword [ebp+0xc]
000056F8  E857050000        call 0x5c54
000056FD  83C410            add esp,byte +0x10
00005700  85C0              test eax,eax
00005702  754A              jnz 0x574e
00005704  A124DC0100        mov eax,[0x1dc24]
00005709  50                push eax
0000570A  6816BA0100        push dword 0x1ba16
0000570F  6AFF              push byte -0x1
00005711  6A00              push byte +0x0
00005713  E824FAFFFF        call 0x513c
00005718  83C410            add esp,byte +0x10
0000571B  A124DC0100        mov eax,[0x1dc24]
00005720  89C2              mov edx,eax
00005722  8B450C            mov eax,[ebp+0xc]
00005725  83C006            add eax,byte +0x6
00005728  52                push edx
00005729  50                push eax
0000572A  6AFF              push byte -0x1
0000572C  6A00              push byte +0x0
0000572E  E809FAFFFF        call 0x513c
00005733  83C410            add esp,byte +0x10
00005736  6A07              push byte +0x7
00005738  6828BA0100        push dword 0x1ba28
0000573D  6AFF              push byte -0x1
0000573F  6A00              push byte +0x0
00005741  E8F6F9FFFF        call 0x513c
00005746  83C410            add esp,byte +0x10
00005749  E9CD020000        jmp 0x5a1b
0000574E  83EC04            sub esp,byte +0x4
00005751  6A04              push byte +0x4
00005753  684ABA0100        push dword 0x1ba4a
00005758  FF750C            push dword [ebp+0xc]
0000575B  E8F4040000        call 0x5c54
00005760  83C410            add esp,byte +0x10
00005763  85C0              test eax,eax
00005765  0F85CB000000      jnz near 0x5836
0000576B  8B450C            mov eax,[ebp+0xc]
0000576E  83C004            add eax,byte +0x4
00005771  8945DC            mov [ebp-0x24],eax
00005774  A124DC0100        mov eax,[0x1dc24]
00005779  50                push eax
0000577A  684FBA0100        push dword 0x1ba4f
0000577F  6AFF              push byte -0x1
00005781  6A00              push byte +0x0
00005783  E8B4F9FFFF        call 0x513c
00005788  83C410            add esp,byte +0x10
0000578B  A124DC0100        mov eax,[0x1dc24]
00005790  50                push eax
00005791  FF75DC            push dword [ebp-0x24]
00005794  6AFF              push byte -0x1
00005796  6A00              push byte +0x0
00005798  E89FF9FFFF        call 0x513c
0000579D  83C410            add esp,byte +0x10
000057A0  A124DC0100        mov eax,[0x1dc24]
000057A5  50                push eax
000057A6  68DFB90100        push dword 0x1b9df
000057AB  6AFF              push byte -0x1
000057AD  6A00              push byte +0x0
000057AF  E888F9FFFF        call 0x513c
000057B4  83C410            add esp,byte +0x10
000057B7  83EC0C            sub esp,byte +0xc
000057BA  6800020000        push dword 0x200
000057BF  E84AB2FFFF        call 0xa0e
000057C4  83C410            add esp,byte +0x10
000057C7  8945D8            mov [ebp-0x28],eax
000057CA  83EC08            sub esp,byte +0x8
000057CD  FF75D8            push dword [ebp-0x28]
000057D0  FF75DC            push dword [ebp-0x24]
000057D3  E8E2BCFFFF        call 0x14ba
000057D8  83C410            add esp,byte +0x10
000057DB  85C0              test eax,eax
000057DD  7431              jz 0x5810
000057DF  8B45D8            mov eax,[ebp-0x28]
000057E2  05FF010000        add eax,0x1ff
000057E7  C60000            mov byte [eax],0x0
000057EA  6A0F              push byte +0xf
000057EC  FF75D8            push dword [ebp-0x28]
000057EF  6AFF              push byte -0x1
000057F1  6A00              push byte +0x0
000057F3  E844F9FFFF        call 0x513c
000057F8  83C410            add esp,byte +0x10
000057FB  6A0F              push byte +0xf
000057FD  68DFB90100        push dword 0x1b9df
00005802  6AFF              push byte -0x1
00005804  6A00              push byte +0x0
00005806  E831F9FFFF        call 0x513c
0000580B  83C410            add esp,byte +0x10
0000580E  EB13              jmp short 0x5823
00005810  6A0C              push byte +0xc
00005812  6859BA0100        push dword 0x1ba59
00005817  6AFF              push byte -0x1
00005819  6A00              push byte +0x0
0000581B  E81CF9FFFF        call 0x513c
00005820  83C410            add esp,byte +0x10
00005823  83EC0C            sub esp,byte +0xc
00005826  FF75D8            push dword [ebp-0x28]
00005829  E8A4B2FFFF        call 0xad2
0000582E  83C410            add esp,byte +0x10
00005831  E9E5010000        jmp 0x5a1b
00005836  83EC08            sub esp,byte +0x8
00005839  686ABA0100        push dword 0x1ba6a
0000583E  FF750C            push dword [ebp+0xc]
00005841  E814030000        call 0x5b5a
00005846  83C410            add esp,byte +0x10
00005849  85C0              test eax,eax
0000584B  751C              jnz 0x5869
0000584D  A124DC0100        mov eax,[0x1dc24]
00005852  50                push eax
00005853  686EBA0100        push dword 0x1ba6e
00005858  6AFF              push byte -0x1
0000585A  6A00              push byte +0x0
0000585C  E8DBF8FFFF        call 0x513c
00005861  83C410            add esp,byte +0x10
00005864  E9B2010000        jmp 0x5a1b
00005869  83EC04            sub esp,byte +0x4
0000586C  6A05              push byte +0x5
0000586E  688CBA0100        push dword 0x1ba8c
00005873  FF750C            push dword [ebp+0xc]
00005876  E8D9030000        call 0x5c54
0000587B  83C410            add esp,byte +0x10
0000587E  85C0              test eax,eax
00005880  0F851C010000      jnz near 0x59a2
00005886  C745E800000000    mov dword [ebp-0x18],0x0
0000588D  8B450C            mov eax,[ebp+0xc]
00005890  83C005            add eax,byte +0x5
00005893  0FB600            movzx eax,byte [eax]
00005896  3C2F              cmp al,0x2f
00005898  7E1F              jng 0x58b9
0000589A  8B450C            mov eax,[ebp+0xc]
0000589D  83C005            add eax,byte +0x5
000058A0  0FB600            movzx eax,byte [eax]
000058A3  3C39              cmp al,0x39
000058A5  7F12              jg 0x58b9
000058A7  8B450C            mov eax,[ebp+0xc]
000058AA  83C005            add eax,byte +0x5
000058AD  0FB600            movzx eax,byte [eax]
000058B0  0FBEC0            movsx eax,al
000058B3  83E830            sub eax,byte +0x30
000058B6  8945E8            mov [ebp-0x18],eax
000058B9  83EC0C            sub esp,byte +0xc
000058BC  6800020000        push dword 0x200
000058C1  E848B1FFFF        call 0xa0e
000058C6  83C410            add esp,byte +0x10
000058C9  8945E0            mov [ebp-0x20],eax
000058CC  A124DC0100        mov eax,[0x1dc24]
000058D1  50                push eax
000058D2  6892BA0100        push dword 0x1ba92
000058D7  6AFF              push byte -0x1
000058D9  6A00              push byte +0x0
000058DB  E85CF8FFFF        call 0x513c
000058E0  83C410            add esp,byte +0x10
000058E3  83EC08            sub esp,byte +0x8
000058E6  FF75E0            push dword [ebp-0x20]
000058E9  FF75E8            push dword [ebp-0x18]
000058EC  E80C050000        call 0x5dfd
000058F1  83C410            add esp,byte +0x10
000058F4  85C0              test eax,eax
000058F6  0F8483000000      jz near 0x597f
000058FC  C745E400000000    mov dword [ebp-0x1c],0x0
00005903  EB5B              jmp short 0x5960
00005905  8B55E4            mov edx,[ebp-0x1c]
00005908  8B45E0            mov eax,[ebp-0x20]
0000590B  01D0              add eax,edx
0000590D  0FB600            movzx eax,byte [eax]
00005910  C0E804            shr al,byte 0x4
00005913  0FB6C0            movzx eax,al
00005916  83E00F            and eax,byte +0xf
00005919  0FB68054BB0100    movzx eax,byte [eax+0x1bb54]
00005920  8845A8            mov [ebp-0x58],al
00005923  8B55E4            mov edx,[ebp-0x1c]
00005926  8B45E0            mov eax,[ebp-0x20]
00005929  01D0              add eax,edx
0000592B  0FB600            movzx eax,byte [eax]
0000592E  0FB6C0            movzx eax,al
00005931  83E00F            and eax,byte +0xf
00005934  0FB68054BB0100    movzx eax,byte [eax+0x1bb54]
0000593B  8845A9            mov [ebp-0x57],al
0000593E  C645AA20          mov byte [ebp-0x56],0x20
00005942  C645AB00          mov byte [ebp-0x55],0x0
00005946  A124DC0100        mov eax,[0x1dc24]
0000594B  50                push eax
0000594C  8D45A8            lea eax,[ebp-0x58]
0000594F  50                push eax
00005950  6AFF              push byte -0x1
00005952  6A00              push byte +0x0
00005954  E8E3F7FFFF        call 0x513c
00005959  83C410            add esp,byte +0x10
0000595C  8345E401          add dword [ebp-0x1c],byte +0x1
00005960  837DE40F          cmp dword [ebp-0x1c],byte +0xf
00005964  7E9F              jng 0x5905
00005966  A124DC0100        mov eax,[0x1dc24]
0000596B  50                push eax
0000596C  68DFB90100        push dword 0x1b9df
00005971  6AFF              push byte -0x1
00005973  6A00              push byte +0x0
00005975  E8C2F7FFFF        call 0x513c
0000597A  83C410            add esp,byte +0x10
0000597D  EB13              jmp short 0x5992
0000597F  6A0C              push byte +0xc
00005981  68A3BA0100        push dword 0x1baa3
00005986  6AFF              push byte -0x1
00005988  6A00              push byte +0x0
0000598A  E8ADF7FFFF        call 0x513c
0000598F  83C410            add esp,byte +0x10
00005992  83EC0C            sub esp,byte +0xc
00005995  FF75E0            push dword [ebp-0x20]
00005998  E835B1FFFF        call 0xad2
0000599D  83C410            add esp,byte +0x10
000059A0  EB79              jmp short 0x5a1b
000059A2  83EC08            sub esp,byte +0x8
000059A5  68B0BA0100        push dword 0x1bab0
000059AA  FF750C            push dword [ebp+0xc]
000059AD  E8A8010000        call 0x5b5a
000059B2  83C410            add esp,byte +0x10
000059B5  85C0              test eax,eax
000059B7  7528              jnz 0x59e1
000059B9  6A0C              push byte +0xc
000059BB  68B8BA0100        push dword 0x1bab8
000059C0  6AFF              push byte -0x1
000059C2  6A00              push byte +0x0
000059C4  E873F7FFFF        call 0x513c
000059C9  83C410            add esp,byte +0x10
000059CC  6A0C              push byte +0xc
000059CE  68E8BA0100        push dword 0x1bae8
000059D3  6AFF              push byte -0x1
000059D5  6A00              push byte +0x0
000059D7  E860F7FFFF        call 0x513c
000059DC  83C410            add esp,byte +0x10
000059DF  EB3A              jmp short 0x5a1b
000059E1  6A4F              push byte +0x4f
000059E3  6811BB0100        push dword 0x1bb11
000059E8  6AFF              push byte -0x1
000059EA  6A00              push byte +0x0
000059EC  E84BF7FFFF        call 0x513c
000059F1  83C410            add esp,byte +0x10
000059F4  6A4F              push byte +0x4f
000059F6  FF750C            push dword [ebp+0xc]
000059F9  6AFF              push byte -0x1
000059FB  6A00              push byte +0x0
000059FD  E83AF7FFFF        call 0x513c
00005A02  83C410            add esp,byte +0x10
00005A05  6A4F              push byte +0x4f
00005A07  68DFB90100        push dword 0x1b9df
00005A0C  6AFF              push byte -0x1
00005A0E  6A00              push byte +0x0
00005A10  E827F7FFFF        call 0x513c
00005A15  83C410            add esp,byte +0x10
00005A18  EB01              jmp short 0x5a1b
00005A1A  90                nop
00005A1B  C9                leave
00005A1C  C3                ret
00005A1D  55                push ebp
00005A1E  89E5              mov ebp,esp
00005A20  83EC78            sub esp,byte +0x78
00005A23  83EC0C            sub esp,byte +0xc
00005A26  6A01              push byte +0x1
00005A28  E801DBFFFF        call 0x352e
00005A2D  83C410            add esp,byte +0x10
00005A30  6A1F              push byte +0x1f
00005A32  6824BB0100        push dword 0x1bb24
00005A37  6A00              push byte +0x0
00005A39  6A00              push byte +0x0
00005A3B  E8FCF6FFFF        call 0x513c
00005A40  83C410            add esp,byte +0x10
00005A43  6A1F              push byte +0x1f
00005A45  6845BB0100        push dword 0x1bb45
00005A4A  6AFF              push byte -0x1
00005A4C  6A00              push byte +0x0
00005A4E  E8E9F6FFFF        call 0x513c
00005A53  83C410            add esp,byte +0x10
00005A56  C745F400000000    mov dword [ebp-0xc],0x0
00005A5D  E88DF3FFFF        call 0x4def
00005A62  8845F3            mov [ebp-0xd],al
00005A65  807DF300          cmp byte [ebp-0xd],0x0
00005A69  0F84E5000000      jz near 0x5b54
00005A6F  807DF30A          cmp byte [ebp-0xd],0xa
00005A73  7553              jnz 0x5ac8
00005A75  A124DC0100        mov eax,[0x1dc24]
00005A7A  50                push eax
00005A7B  68DFB90100        push dword 0x1b9df
00005A80  6AFF              push byte -0x1
00005A82  6A00              push byte +0x0
00005A84  E8B3F6FFFF        call 0x513c
00005A89  83C410            add esp,byte +0x10
00005A8C  8D558F            lea edx,[ebp-0x71]
00005A8F  8B45F4            mov eax,[ebp-0xc]
00005A92  01D0              add eax,edx
00005A94  C60000            mov byte [eax],0x0
00005A97  83EC08            sub esp,byte +0x8
00005A9A  8D458F            lea eax,[ebp-0x71]
00005A9D  50                push eax
00005A9E  6A00              push byte +0x0
00005AA0  E8A9F8FFFF        call 0x534e
00005AA5  83C410            add esp,byte +0x10
00005AA8  C745F400000000    mov dword [ebp-0xc],0x0
00005AAF  A124DC0100        mov eax,[0x1dc24]
00005AB4  50                push eax
00005AB5  6845BB0100        push dword 0x1bb45
00005ABA  6AFF              push byte -0x1
00005ABC  6A00              push byte +0x0
00005ABE  E879F6FFFF        call 0x513c
00005AC3  83C410            add esp,byte +0x10
00005AC6  EB95              jmp short 0x5a5d
00005AC8  807DF308          cmp byte [ebp-0xd],0x8
00005ACC  7531              jnz 0x5aff
00005ACE  837DF400          cmp dword [ebp-0xc],byte +0x0
00005AD2  7E89              jng 0x5a5d
00005AD4  836DF401          sub dword [ebp-0xc],byte +0x1
00005AD8  8D558F            lea edx,[ebp-0x71]
00005ADB  8B45F4            mov eax,[ebp-0xc]
00005ADE  01D0              add eax,edx
00005AE0  C60000            mov byte [eax],0x0
00005AE3  A124DC0100        mov eax,[0x1dc24]
00005AE8  50                push eax
00005AE9  684EBB0100        push dword 0x1bb4e
00005AEE  6AFF              push byte -0x1
00005AF0  6A00              push byte +0x0
00005AF2  E845F6FFFF        call 0x513c
00005AF7  83C410            add esp,byte +0x10
00005AFA  E95EFFFFFF        jmp 0x5a5d
00005AFF  807DF31F          cmp byte [ebp-0xd],0x1f
00005B03  0F8E54FFFFFF      jng near 0x5a5d
00005B09  807DF37F          cmp byte [ebp-0xd],0x7f
00005B0D  0F844AFFFFFF      jz near 0x5a5d
00005B13  837DF462          cmp dword [ebp-0xc],byte +0x62
00005B17  0F8F40FFFFFF      jg near 0x5a5d
00005B1D  8B45F4            mov eax,[ebp-0xc]
00005B20  8D5001            lea edx,[eax+0x1]
00005B23  8955F4            mov [ebp-0xc],edx
00005B26  0FB655F3          movzx edx,byte [ebp-0xd]
00005B2A  8854058F          mov [ebp+eax-0x71],dl
00005B2E  0FB645F3          movzx eax,byte [ebp-0xd]
00005B32  88458D            mov [ebp-0x73],al
00005B35  C6458E00          mov byte [ebp-0x72],0x0
00005B39  A124DC0100        mov eax,[0x1dc24]
00005B3E  50                push eax
00005B3F  8D458D            lea eax,[ebp-0x73]
00005B42  50                push eax
00005B43  6AFF              push byte -0x1
00005B45  6A00              push byte +0x0
00005B47  E8F0F5FFFF        call 0x513c
00005B4C  83C410            add esp,byte +0x10
00005B4F  E909FFFFFF        jmp 0x5a5d
00005B54  90                nop
00005B55  E903FFFFFF        jmp 0x5a5d
00005B5A  55                push ebp
00005B5B  89E5              mov ebp,esp
00005B5D  EB08              jmp short 0x5b67
00005B5F  83450801          add dword [ebp+0x8],byte +0x1
00005B63  83450C01          add dword [ebp+0xc],byte +0x1
00005B67  8B4508            mov eax,[ebp+0x8]
00005B6A  0FB600            movzx eax,byte [eax]
00005B6D  84C0              test al,al
00005B6F  7410              jz 0x5b81
00005B71  8B4508            mov eax,[ebp+0x8]
00005B74  0FB610            movzx edx,byte [eax]
00005B77  8B450C            mov eax,[ebp+0xc]
00005B7A  0FB600            movzx eax,byte [eax]
00005B7D  38C2              cmp dl,al
00005B7F  74DE              jz 0x5b5f
00005B81  8B4508            mov eax,[ebp+0x8]
00005B84  0FB600            movzx eax,byte [eax]
00005B87  0FB6D0            movzx edx,al
00005B8A  8B450C            mov eax,[ebp+0xc]
00005B8D  0FB600            movzx eax,byte [eax]
00005B90  0FB6C0            movzx eax,al
00005B93  29C2              sub edx,eax
00005B95  89D0              mov eax,edx
00005B97  5D                pop ebp
00005B98  C3                ret
00005B99  55                push ebp
00005B9A  89E5              mov ebp,esp
00005B9C  83EC10            sub esp,byte +0x10
00005B9F  C745FC00000000    mov dword [ebp-0x4],0x0
00005BA6  EB04              jmp short 0x5bac
00005BA8  8345FC01          add dword [ebp-0x4],byte +0x1
00005BAC  8B55FC            mov edx,[ebp-0x4]
00005BAF  8B4508            mov eax,[ebp+0x8]
00005BB2  01D0              add eax,edx
00005BB4  0FB600            movzx eax,byte [eax]
00005BB7  84C0              test al,al
00005BB9  75ED              jnz 0x5ba8
00005BBB  8B45FC            mov eax,[ebp-0x4]
00005BBE  C9                leave
00005BBF  C3                ret
00005BC0  55                push ebp
00005BC1  89E5              mov ebp,esp
00005BC3  83EC10            sub esp,byte +0x10
00005BC6  8B4508            mov eax,[ebp+0x8]
00005BC9  8945FC            mov [ebp-0x4],eax
00005BCC  EB0E              jmp short 0x5bdc
00005BCE  8B45FC            mov eax,[ebp-0x4]
00005BD1  8D5001            lea edx,[eax+0x1]
00005BD4  8955FC            mov [ebp-0x4],edx
00005BD7  8B550C            mov edx,[ebp+0xc]
00005BDA  8810              mov [eax],dl
00005BDC  8B4510            mov eax,[ebp+0x10]
00005BDF  8D50FF            lea edx,[eax-0x1]
00005BE2  895510            mov [ebp+0x10],edx
00005BE5  85C0              test eax,eax
00005BE7  75E5              jnz 0x5bce
00005BE9  8B4508            mov eax,[ebp+0x8]
00005BEC  C9                leave
00005BED  C3                ret
00005BEE  55                push ebp
00005BEF  89E5              mov ebp,esp
00005BF1  83EC10            sub esp,byte +0x10
00005BF4  8B4508            mov eax,[ebp+0x8]
00005BF7  8945FC            mov [ebp-0x4],eax
00005BFA  EB17              jmp short 0x5c13
00005BFC  8B550C            mov edx,[ebp+0xc]
00005BFF  8D4201            lea eax,[edx+0x1]
00005C02  89450C            mov [ebp+0xc],eax
00005C05  8B4508            mov eax,[ebp+0x8]
00005C08  8D4801            lea ecx,[eax+0x1]
00005C0B  894D08            mov [ebp+0x8],ecx
00005C0E  0FB612            movzx edx,byte [edx]
00005C11  8810              mov [eax],dl
00005C13  8B450C            mov eax,[ebp+0xc]
00005C16  0FB600            movzx eax,byte [eax]
00005C19  84C0              test al,al
00005C1B  75DF              jnz 0x5bfc
00005C1D  8B4508            mov eax,[ebp+0x8]
00005C20  C60000            mov byte [eax],0x0
00005C23  8B45FC            mov eax,[ebp-0x4]
00005C26  C9                leave
00005C27  C3                ret
00005C28  55                push ebp
00005C29  89E5              mov ebp,esp
00005C2B  EB16              jmp short 0x5c43
00005C2D  8B4508            mov eax,[ebp+0x8]
00005C30  0FB600            movzx eax,byte [eax]
00005C33  8B550C            mov edx,[ebp+0xc]
00005C36  38D0              cmp al,dl
00005C38  7505              jnz 0x5c3f
00005C3A  8B4508            mov eax,[ebp+0x8]
00005C3D  EB13              jmp short 0x5c52
00005C3F  83450801          add dword [ebp+0x8],byte +0x1
00005C43  8B4508            mov eax,[ebp+0x8]
00005C46  0FB600            movzx eax,byte [eax]
00005C49  84C0              test al,al
00005C4B  75E0              jnz 0x5c2d
00005C4D  B800000000        mov eax,0x0
00005C52  5D                pop ebp
00005C53  C3                ret
00005C54  55                push ebp
00005C55  89E5              mov ebp,esp
00005C57  EB0C              jmp short 0x5c65
00005C59  83450801          add dword [ebp+0x8],byte +0x1
00005C5D  83450C01          add dword [ebp+0xc],byte +0x1
00005C61  836D1001          sub dword [ebp+0x10],byte +0x1
00005C65  837D1000          cmp dword [ebp+0x10],byte +0x0
00005C69  741A              jz 0x5c85
00005C6B  8B4508            mov eax,[ebp+0x8]
00005C6E  0FB600            movzx eax,byte [eax]
00005C71  84C0              test al,al
00005C73  7410              jz 0x5c85
00005C75  8B4508            mov eax,[ebp+0x8]
00005C78  0FB610            movzx edx,byte [eax]
00005C7B  8B450C            mov eax,[ebp+0xc]
00005C7E  0FB600            movzx eax,byte [eax]
00005C81  38C2              cmp dl,al
00005C83  74D4              jz 0x5c59
00005C85  837D1000          cmp dword [ebp+0x10],byte +0x0
00005C89  7507              jnz 0x5c92
00005C8B  B800000000        mov eax,0x0
00005C90  EB16              jmp short 0x5ca8
00005C92  8B4508            mov eax,[ebp+0x8]
00005C95  0FB600            movzx eax,byte [eax]
00005C98  0FB6D0            movzx edx,al
00005C9B  8B450C            mov eax,[ebp+0xc]
00005C9E  0FB600            movzx eax,byte [eax]
00005CA1  0FB6C0            movzx eax,al
00005CA4  29C2              sub edx,eax
00005CA6  89D0              mov eax,edx
00005CA8  5D                pop ebp
00005CA9  C3                ret
00005CAA  55                push ebp
00005CAB  89E5              mov ebp,esp
00005CAD  83EC10            sub esp,byte +0x10
00005CB0  FF7508            push dword [ebp+0x8]
00005CB3  E8E1FEFFFF        call 0x5b99
00005CB8  83C404            add esp,byte +0x4
00005CBB  89C2              mov edx,eax
00005CBD  8B4508            mov eax,[ebp+0x8]
00005CC0  01D0              add eax,edx
00005CC2  8945FC            mov [ebp-0x4],eax
00005CC5  EB17              jmp short 0x5cde
00005CC7  8B550C            mov edx,[ebp+0xc]
00005CCA  8D4201            lea eax,[edx+0x1]
00005CCD  89450C            mov [ebp+0xc],eax
00005CD0  8B45FC            mov eax,[ebp-0x4]
00005CD3  8D4801            lea ecx,[eax+0x1]
00005CD6  894DFC            mov [ebp-0x4],ecx
00005CD9  0FB612            movzx edx,byte [edx]
00005CDC  8810              mov [eax],dl
00005CDE  8B450C            mov eax,[ebp+0xc]
00005CE1  0FB600            movzx eax,byte [eax]
00005CE4  84C0              test al,al
00005CE6  75DF              jnz 0x5cc7
00005CE8  8B45FC            mov eax,[ebp-0x4]
00005CEB  C60000            mov byte [eax],0x0
00005CEE  8B4508            mov eax,[ebp+0x8]
00005CF1  C9                leave
00005CF2  C3                ret
00005CF3  55                push ebp
00005CF4  89E5              mov ebp,esp
00005CF6  83EC08            sub esp,byte +0x8
00005CF9  8B5508            mov edx,[ebp+0x8]
00005CFC  8B450C            mov eax,[ebp+0xc]
00005CFF  668955FC          mov [ebp-0x4],dx
00005D03  8845F8            mov [ebp-0x8],al
00005D06  0FB645F8          movzx eax,byte [ebp-0x8]
00005D0A  0FB755FC          movzx edx,word [ebp-0x4]
00005D0E  EE                out dx,al
00005D0F  90                nop
00005D10  C9                leave
00005D11  C3                ret
00005D12  55                push ebp
00005D13  89E5              mov ebp,esp
00005D15  83EC14            sub esp,byte +0x14
00005D18  8B4508            mov eax,[ebp+0x8]
00005D1B  668945EC          mov [ebp-0x14],ax
00005D1F  0FB745EC          movzx eax,word [ebp-0x14]
00005D23  89C2              mov edx,eax
00005D25  EC                in al,dx
00005D26  8845FF            mov [ebp-0x1],al
00005D29  0FB645FF          movzx eax,byte [ebp-0x1]
00005D2D  C9                leave
00005D2E  C3                ret
00005D2F  55                push ebp
00005D30  89E5              mov ebp,esp
00005D32  57                push edi
00005D33  53                push ebx
00005D34  83EC04            sub esp,byte +0x4
00005D37  8B4508            mov eax,[ebp+0x8]
00005D3A  668945F4          mov [ebp-0xc],ax
00005D3E  0FB755F4          movzx edx,word [ebp-0xc]
00005D42  8B4D0C            mov ecx,[ebp+0xc]
00005D45  8B4510            mov eax,[ebp+0x10]
00005D48  89CB              mov ebx,ecx
00005D4A  89DF              mov edi,ebx
00005D4C  89C1              mov ecx,eax
00005D4E  FC                cld
00005D4F  66F36D            rep insw
00005D52  89C8              mov eax,ecx
00005D54  89FB              mov ebx,edi
00005D56  895D0C            mov [ebp+0xc],ebx
00005D59  894510            mov [ebp+0x10],eax
00005D5C  90                nop
00005D5D  83C404            add esp,byte +0x4
00005D60  5B                pop ebx
00005D61  5F                pop edi
00005D62  5D                pop ebp
00005D63  C3                ret
00005D64  55                push ebp
00005D65  89E5              mov ebp,esp
00005D67  56                push esi
00005D68  53                push ebx
00005D69  83EC04            sub esp,byte +0x4
00005D6C  8B4508            mov eax,[ebp+0x8]
00005D6F  668945F4          mov [ebp-0xc],ax
00005D73  0FB755F4          movzx edx,word [ebp-0xc]
00005D77  8B4D0C            mov ecx,[ebp+0xc]
00005D7A  8B4510            mov eax,[ebp+0x10]
00005D7D  89CB              mov ebx,ecx
00005D7F  89DE              mov esi,ebx
00005D81  89C1              mov ecx,eax
00005D83  FC                cld
00005D84  66F36F            rep outsw
00005D87  89C8              mov eax,ecx
00005D89  89F3              mov ebx,esi
00005D8B  895D0C            mov [ebp+0xc],ebx
00005D8E  894510            mov [ebp+0x10],eax
00005D91  90                nop
00005D92  83C404            add esp,byte +0x4
00005D95  5B                pop ebx
00005D96  5E                pop esi
00005D97  5D                pop ebp
00005D98  C3                ret
00005D99  55                push ebp
00005D9A  89E5              mov ebp,esp
00005D9C  83EC10            sub esp,byte +0x10
00005D9F  C745FCA0860100    mov dword [ebp-0x4],0x186a0
00005DA6  90                nop
00005DA7  68F7010000        push dword 0x1f7
00005DAC  E861FFFFFF        call 0x5d12
00005DB1  83C404            add esp,byte +0x4
00005DB4  84C0              test al,al
00005DB6  790D              jns 0x5dc5
00005DB8  8B45FC            mov eax,[ebp-0x4]
00005DBB  8D50FF            lea edx,[eax-0x1]
00005DBE  8955FC            mov [ebp-0x4],edx
00005DC1  85C0              test eax,eax
00005DC3  75E2              jnz 0x5da7
00005DC5  90                nop
00005DC6  C9                leave
00005DC7  C3                ret
00005DC8  55                push ebp
00005DC9  89E5              mov ebp,esp
00005DCB  83EC10            sub esp,byte +0x10
00005DCE  C745FCA0860100    mov dword [ebp-0x4],0x186a0
00005DD5  90                nop
00005DD6  68F7010000        push dword 0x1f7
00005DDB  E832FFFFFF        call 0x5d12
00005DE0  83C404            add esp,byte +0x4
00005DE3  0FB6C0            movzx eax,al
00005DE6  83E008            and eax,byte +0x8
00005DE9  85C0              test eax,eax
00005DEB  750D              jnz 0x5dfa
00005DED  8B45FC            mov eax,[ebp-0x4]
00005DF0  8D50FF            lea edx,[eax-0x1]
00005DF3  8955FC            mov [ebp-0x4],edx
00005DF6  85C0              test eax,eax
00005DF8  75DC              jnz 0x5dd6
00005DFA  90                nop
00005DFB  C9                leave
00005DFC  C3                ret
00005DFD  55                push ebp
00005DFE  89E5              mov ebp,esp
00005E00  E894FFFFFF        call 0x5d99
00005E05  8B4508            mov eax,[ebp+0x8]
00005E08  C1E818            shr eax,byte 0x18
00005E0B  83E00F            and eax,byte +0xf
00005E0E  83C8E0            or eax,byte -0x20
00005E11  0FB6C0            movzx eax,al
00005E14  50                push eax
00005E15  68F6010000        push dword 0x1f6
00005E1A  E8D4FEFFFF        call 0x5cf3
00005E1F  83C408            add esp,byte +0x8
00005E22  6A01              push byte +0x1
00005E24  68F2010000        push dword 0x1f2
00005E29  E8C5FEFFFF        call 0x5cf3
00005E2E  83C408            add esp,byte +0x8
00005E31  8B4508            mov eax,[ebp+0x8]
00005E34  0FB6C0            movzx eax,al
00005E37  50                push eax
00005E38  68F3010000        push dword 0x1f3
00005E3D  E8B1FEFFFF        call 0x5cf3
00005E42  83C408            add esp,byte +0x8
00005E45  8B4508            mov eax,[ebp+0x8]
00005E48  C1E808            shr eax,byte 0x8
00005E4B  0FB6C0            movzx eax,al
00005E4E  50                push eax
00005E4F  68F4010000        push dword 0x1f4
00005E54  E89AFEFFFF        call 0x5cf3
00005E59  83C408            add esp,byte +0x8
00005E5C  8B4508            mov eax,[ebp+0x8]
00005E5F  C1E810            shr eax,byte 0x10
00005E62  0FB6C0            movzx eax,al
00005E65  50                push eax
00005E66  68F5010000        push dword 0x1f5
00005E6B  E883FEFFFF        call 0x5cf3
00005E70  83C408            add esp,byte +0x8
00005E73  6A20              push byte +0x20
00005E75  68F7010000        push dword 0x1f7
00005E7A  E874FEFFFF        call 0x5cf3
00005E7F  83C408            add esp,byte +0x8
00005E82  E812FFFFFF        call 0x5d99
00005E87  E83CFFFFFF        call 0x5dc8
00005E8C  6800010000        push dword 0x100
00005E91  FF750C            push dword [ebp+0xc]
00005E94  68F0010000        push dword 0x1f0
00005E99  E891FEFFFF        call 0x5d2f
00005E9E  83C40C            add esp,byte +0xc
00005EA1  B801000000        mov eax,0x1
00005EA6  C9                leave
00005EA7  C3                ret
00005EA8  55                push ebp
00005EA9  89E5              mov ebp,esp
00005EAB  E8E9FEFFFF        call 0x5d99
00005EB0  8B4508            mov eax,[ebp+0x8]
00005EB3  C1E818            shr eax,byte 0x18
00005EB6  83E00F            and eax,byte +0xf
00005EB9  83C8E0            or eax,byte -0x20
00005EBC  0FB6C0            movzx eax,al
00005EBF  50                push eax
00005EC0  68F6010000        push dword 0x1f6
00005EC5  E829FEFFFF        call 0x5cf3
00005ECA  83C408            add esp,byte +0x8
00005ECD  6A01              push byte +0x1
00005ECF  68F2010000        push dword 0x1f2
00005ED4  E81AFEFFFF        call 0x5cf3
00005ED9  83C408            add esp,byte +0x8
00005EDC  8B4508            mov eax,[ebp+0x8]
00005EDF  0FB6C0            movzx eax,al
00005EE2  50                push eax
00005EE3  68F3010000        push dword 0x1f3
00005EE8  E806FEFFFF        call 0x5cf3
00005EED  83C408            add esp,byte +0x8
00005EF0  8B4508            mov eax,[ebp+0x8]
00005EF3  C1E808            shr eax,byte 0x8
00005EF6  0FB6C0            movzx eax,al
00005EF9  50                push eax
00005EFA  68F4010000        push dword 0x1f4
00005EFF  E8EFFDFFFF        call 0x5cf3
00005F04  83C408            add esp,byte +0x8
00005F07  8B4508            mov eax,[ebp+0x8]
00005F0A  C1E810            shr eax,byte 0x10
00005F0D  0FB6C0            movzx eax,al
00005F10  50                push eax
00005F11  68F5010000        push dword 0x1f5
00005F16  E8D8FDFFFF        call 0x5cf3
00005F1B  83C408            add esp,byte +0x8
00005F1E  6A30              push byte +0x30
00005F20  68F7010000        push dword 0x1f7
00005F25  E8C9FDFFFF        call 0x5cf3
00005F2A  83C408            add esp,byte +0x8
00005F2D  E867FEFFFF        call 0x5d99
00005F32  E891FEFFFF        call 0x5dc8
00005F37  6800010000        push dword 0x100
00005F3C  FF750C            push dword [ebp+0xc]
00005F3F  68F0010000        push dword 0x1f0
00005F44  E81BFEFFFF        call 0x5d64
00005F49  83C40C            add esp,byte +0xc
00005F4C  90                nop
00005F4D  C9                leave
00005F4E  C3                ret
00005F4F  55                push ebp
00005F50  89E5              mov ebp,esp
00005F52  83EC04            sub esp,byte +0x4
00005F55  8B4510            mov eax,[ebp+0x10]
00005F58  8845FC            mov [ebp-0x4],al
00005F5B  837D0800          cmp dword [ebp+0x8],byte +0x0
00005F5F  7838              js 0x5f99
00005F61  817D083F010000    cmp dword [ebp+0x8],0x13f
00005F68  7F2F              jg 0x5f99
00005F6A  837D0C00          cmp dword [ebp+0xc],byte +0x0
00005F6E  7829              js 0x5f99
00005F70  817D0CC7000000    cmp dword [ebp+0xc],0xc7
00005F77  7F20              jg 0x5f99
00005F79  8B550C            mov edx,[ebp+0xc]
00005F7C  89D0              mov eax,edx
00005F7E  C1E002            shl eax,byte 0x2
00005F81  01D0              add eax,edx
00005F83  C1E006            shl eax,byte 0x6
00005F86  89C2              mov edx,eax
00005F88  8B4508            mov eax,[ebp+0x8]
00005F8B  01D0              add eax,edx
00005F8D  8D9000000A00      lea edx,[eax+0xa0000]
00005F93  0FB645FC          movzx eax,byte [ebp-0x4]
00005F97  8802              mov [edx],al
00005F99  90                nop
00005F9A  C9                leave
00005F9B  C3                ret
00005F9C  55                push ebp
00005F9D  89E5              mov ebp,esp
00005F9F  837D0800          cmp dword [ebp+0x8],byte +0x0
00005FA3  7836              js 0x5fdb
00005FA5  817D083F010000    cmp dword [ebp+0x8],0x13f
00005FAC  7F2D              jg 0x5fdb
00005FAE  837D0C00          cmp dword [ebp+0xc],byte +0x0
00005FB2  7827              js 0x5fdb
00005FB4  817D0CC7000000    cmp dword [ebp+0xc],0xc7
00005FBB  7F1E              jg 0x5fdb
00005FBD  8B550C            mov edx,[ebp+0xc]
00005FC0  89D0              mov eax,edx
00005FC2  C1E002            shl eax,byte 0x2
00005FC5  01D0              add eax,edx
00005FC7  C1E006            shl eax,byte 0x6
00005FCA  89C2              mov edx,eax
00005FCC  8B4508            mov eax,[ebp+0x8]
00005FCF  01D0              add eax,edx
00005FD1  0500000A00        add eax,0xa0000
00005FD6  0FB600            movzx eax,byte [eax]
00005FD9  EB05              jmp short 0x5fe0
00005FDB  B800000000        mov eax,0x0
00005FE0  5D                pop ebp
00005FE1  C3                ret
00005FE2  55                push ebp
00005FE3  89E5              mov ebp,esp
00005FE5  83EC14            sub esp,byte +0x14
00005FE8  8B4508            mov eax,[ebp+0x8]
00005FEB  8845EC            mov [ebp-0x14],al
00005FEE  C745FC00000000    mov dword [ebp-0x4],0x0
00005FF5  EB13              jmp short 0x600a
00005FF7  8B45FC            mov eax,[ebp-0x4]
00005FFA  8D9000000A00      lea edx,[eax+0xa0000]
00006000  0FB645EC          movzx eax,byte [ebp-0x14]
00006004  8802              mov [edx],al
00006006  8345FC01          add dword [ebp-0x4],byte +0x1
0000600A  817DFCFFF90000    cmp dword [ebp-0x4],0xf9ff
00006011  7EE4              jng 0x5ff7
00006013  90                nop
00006014  C9                leave
00006015  C3                ret
00006016  55                push ebp
00006017  89E5              mov ebp,esp
00006019  83EC14            sub esp,byte +0x14
0000601C  8B4518            mov eax,[ebp+0x18]
0000601F  8845EC            mov [ebp-0x14],al
00006022  8B450C            mov eax,[ebp+0xc]
00006025  8945FC            mov [ebp-0x4],eax
00006028  EB30              jmp short 0x605a
0000602A  8B4508            mov eax,[ebp+0x8]
0000602D  8945F8            mov [ebp-0x8],eax
00006030  EB17              jmp short 0x6049
00006032  0FB645EC          movzx eax,byte [ebp-0x14]
00006036  50                push eax
00006037  FF75FC            push dword [ebp-0x4]
0000603A  FF75F8            push dword [ebp-0x8]
0000603D  E80DFFFFFF        call 0x5f4f
00006042  83C40C            add esp,byte +0xc
00006045  8345F801          add dword [ebp-0x8],byte +0x1
00006049  8B5508            mov edx,[ebp+0x8]
0000604C  8B4510            mov eax,[ebp+0x10]
0000604F  01D0              add eax,edx
00006051  3945F8            cmp [ebp-0x8],eax
00006054  7CDC              jl 0x6032
00006056  8345FC01          add dword [ebp-0x4],byte +0x1
0000605A  8B550C            mov edx,[ebp+0xc]
0000605D  8B4514            mov eax,[ebp+0x14]
00006060  01D0              add eax,edx
00006062  3945FC            cmp [ebp-0x4],eax
00006065  7CC3              jl 0x602a
00006067  90                nop
00006068  C9                leave
00006069  C3                ret
0000606A  55                push ebp
0000606B  89E5              mov ebp,esp
0000606D  53                push ebx
0000606E  83EC10            sub esp,byte +0x10
00006071  6A01              push byte +0x1
00006073  E86AFFFFFF        call 0x5fe2
00006078  83C404            add esp,byte +0x4
0000607B  6A28              push byte +0x28
0000607D  6A32              push byte +0x32
0000607F  6A64              push byte +0x64
00006081  6A14              push byte +0x14
00006083  6A14              push byte +0x14
00006085  E88CFFFFFF        call 0x6016
0000608A  83C414            add esp,byte +0x14
0000608D  6A30              push byte +0x30
0000608F  6A32              push byte +0x32
00006091  6A64              push byte +0x64
00006093  6A14              push byte +0x14
00006095  6896000000        push dword 0x96
0000609A  E877FFFFFF        call 0x6016
0000609F  83C414            add esp,byte +0x14
000060A2  C745F800000000    mov dword [ebp-0x8],0x0
000060A9  EB4E              jmp short 0x60f9
000060AB  8B45F8            mov eax,[ebp-0x8]
000060AE  0FB6C8            movzx ecx,al
000060B1  8B45F8            mov eax,[ebp-0x8]
000060B4  8D501F            lea edx,[eax+0x1f]
000060B7  85C0              test eax,eax
000060B9  0F48C2            cmovs eax,edx
000060BC  C1F805            sar eax,byte 0x5
000060BF  89C2              mov edx,eax
000060C1  89D0              mov eax,edx
000060C3  C1E002            shl eax,byte 0x2
000060C6  01D0              add eax,edx
000060C8  01C0              add eax,eax
000060CA  8D5864            lea ebx,[eax+0x64]
000060CD  8B45F8            mov eax,[ebp-0x8]
000060D0  99                cdq
000060D1  C1EA1B            shr edx,byte 0x1b
000060D4  01D0              add eax,edx
000060D6  83E01F            and eax,byte +0x1f
000060D9  29D0              sub eax,edx
000060DB  89C2              mov edx,eax
000060DD  89D0              mov eax,edx
000060DF  C1E002            shl eax,byte 0x2
000060E2  01D0              add eax,edx
000060E4  01C0              add eax,eax
000060E6  51                push ecx
000060E7  6A0A              push byte +0xa
000060E9  6A0A              push byte +0xa
000060EB  53                push ebx
000060EC  50                push eax
000060ED  E824FFFFFF        call 0x6016
000060F2  83C414            add esp,byte +0x14
000060F5  8345F801          add dword [ebp-0x8],byte +0x1
000060F9  817DF8FF000000    cmp dword [ebp-0x8],0xff
00006100  7EA9              jng 0x60ab
00006102  90                nop
00006103  8B5DFC            mov ebx,[ebp-0x4]
00006106  C9                leave
00006107  C3                ret
00006108  55                push ebp
00006109  89E5              mov ebp,esp
0000610B  83EC20            sub esp,byte +0x20
0000610E  C745FC00000000    mov dword [ebp-0x4],0x0
00006115  C745F800000000    mov dword [ebp-0x8],0x0
0000611C  E989000000        jmp 0x61aa
00006121  C745F400000000    mov dword [ebp-0xc],0x0
00006128  EB76              jmp short 0x61a0
0000612A  8B5508            mov edx,[ebp+0x8]
0000612D  8B45F4            mov eax,[ebp-0xc]
00006130  01D0              add eax,edx
00006132  8945F0            mov [ebp-0x10],eax
00006135  8B550C            mov edx,[ebp+0xc]
00006138  8B45F8            mov eax,[ebp-0x8]
0000613B  01D0              add eax,edx
0000613D  8945EC            mov [ebp-0x14],eax
00006140  837DF000          cmp dword [ebp-0x10],byte +0x0
00006144  7846              js 0x618c
00006146  817DF03F010000    cmp dword [ebp-0x10],0x13f
0000614D  7F3D              jg 0x618c
0000614F  837DEC00          cmp dword [ebp-0x14],byte +0x0
00006153  7837              js 0x618c
00006155  817DECC7000000    cmp dword [ebp-0x14],0xc7
0000615C  7F2E              jg 0x618c
0000615E  8B55EC            mov edx,[ebp-0x14]
00006161  89D0              mov eax,edx
00006163  C1E002            shl eax,byte 0x2
00006166  01D0              add eax,edx
00006168  C1E006            shl eax,byte 0x6
0000616B  89C2              mov edx,eax
0000616D  8B45F0            mov eax,[ebp-0x10]
00006170  01D0              add eax,edx
00006172  8D8800000A00      lea ecx,[eax+0xa0000]
00006178  8B45FC            mov eax,[ebp-0x4]
0000617B  8D5001            lea edx,[eax+0x1]
0000617E  8955FC            mov [ebp-0x4],edx
00006181  0FB611            movzx edx,byte [ecx]
00006184  889080360200      mov [eax+0x23680],dl
0000618A  EB10              jmp short 0x619c
0000618C  8B45FC            mov eax,[ebp-0x4]
0000618F  8D5001            lea edx,[eax+0x1]
00006192  8955FC            mov [ebp-0x4],edx
00006195  C6808036020000    mov byte [eax+0x23680],0x0
0000619C  8345F401          add dword [ebp-0xc],byte +0x1
000061A0  837DF40B          cmp dword [ebp-0xc],byte +0xb
000061A4  7E84              jng 0x612a
000061A6  8345F801          add dword [ebp-0x8],byte +0x1
000061AA  837DF812          cmp dword [ebp-0x8],byte +0x12
000061AE  0F8E6DFFFFFF      jng near 0x6121
000061B4  90                nop
000061B5  C9                leave
000061B6  C3                ret
000061B7  55                push ebp
000061B8  89E5              mov ebp,esp
000061BA  83EC20            sub esp,byte +0x20
000061BD  C745FC00000000    mov dword [ebp-0x4],0x0
000061C4  C745F800000000    mov dword [ebp-0x8],0x0
000061CB  EB76              jmp short 0x6243
000061CD  C745F400000000    mov dword [ebp-0xc],0x0
000061D4  EB63              jmp short 0x6239
000061D6  8B5508            mov edx,[ebp+0x8]
000061D9  8B45F4            mov eax,[ebp-0xc]
000061DC  01D0              add eax,edx
000061DE  8945F0            mov [ebp-0x10],eax
000061E1  8B550C            mov edx,[ebp+0xc]
000061E4  8B45F8            mov eax,[ebp-0x8]
000061E7  01D0              add eax,edx
000061E9  8945EC            mov [ebp-0x14],eax
000061EC  837DF000          cmp dword [ebp-0x10],byte +0x0
000061F0  783F              js 0x6231
000061F2  817DF03F010000    cmp dword [ebp-0x10],0x13f
000061F9  7F36              jg 0x6231
000061FB  837DEC00          cmp dword [ebp-0x14],byte +0x0
000061FF  7830              js 0x6231
00006201  817DECC7000000    cmp dword [ebp-0x14],0xc7
00006208  7F27              jg 0x6231
0000620A  8B55EC            mov edx,[ebp-0x14]
0000620D  89D0              mov eax,edx
0000620F  C1E002            shl eax,byte 0x2
00006212  01D0              add eax,edx
00006214  C1E006            shl eax,byte 0x6
00006217  89C2              mov edx,eax
00006219  8B45F0            mov eax,[ebp-0x10]
0000621C  01D0              add eax,edx
0000621E  8D9000000A00      lea edx,[eax+0xa0000]
00006224  8B45FC            mov eax,[ebp-0x4]
00006227  0580360200        add eax,0x23680
0000622C  0FB600            movzx eax,byte [eax]
0000622F  8802              mov [edx],al
00006231  8345FC01          add dword [ebp-0x4],byte +0x1
00006235  8345F401          add dword [ebp-0xc],byte +0x1
00006239  837DF40B          cmp dword [ebp-0xc],byte +0xb
0000623D  7E97              jng 0x61d6
0000623F  8345F801          add dword [ebp-0x8],byte +0x1
00006243  837DF812          cmp dword [ebp-0x8],byte +0x12
00006247  7E84              jng 0x61cd
00006249  90                nop
0000624A  C9                leave
0000624B  C3                ret
0000624C  55                push ebp
0000624D  89E5              mov ebp,esp
0000624F  83EC10            sub esp,byte +0x10
00006252  C745FC00000000    mov dword [ebp-0x4],0x0
00006259  EB51              jmp short 0x62ac
0000625B  C745F800000000    mov dword [ebp-0x8],0x0
00006262  EB3E              jmp short 0x62a2
00006264  8B55FC            mov edx,[ebp-0x4]
00006267  89D0              mov eax,edx
00006269  01C0              add eax,eax
0000626B  01D0              add eax,edx
0000626D  C1E002            shl eax,byte 0x2
00006270  89C2              mov edx,eax
00006272  8B45F8            mov eax,[ebp-0x8]
00006275  01D0              add eax,edx
00006277  0FB68080BB0100    movzx eax,byte [eax+0x1bb80]
0000627E  84C0              test al,al
00006280  741C              jz 0x629e
00006282  8B550C            mov edx,[ebp+0xc]
00006285  8B45FC            mov eax,[ebp-0x4]
00006288  01C2              add edx,eax
0000628A  8B4D08            mov ecx,[ebp+0x8]
0000628D  8B45F8            mov eax,[ebp-0x8]
00006290  01C8              add eax,ecx
00006292  6A0F              push byte +0xf
00006294  52                push edx
00006295  50                push eax
00006296  E8B4FCFFFF        call 0x5f4f
0000629B  83C40C            add esp,byte +0xc
0000629E  8345F801          add dword [ebp-0x8],byte +0x1
000062A2  837DF80B          cmp dword [ebp-0x8],byte +0xb
000062A6  7EBC              jng 0x6264
000062A8  8345FC01          add dword [ebp-0x4],byte +0x1
000062AC  837DFC12          cmp dword [ebp-0x4],byte +0x12
000062B0  7EA9              jng 0x625b
000062B2  90                nop
000062B3  C9                leave
000062B4  C3                ret
000062B5  55                push ebp
000062B6  89E5              mov ebp,esp
000062B8  83EC28            sub esp,byte +0x28
000062BB  8B4508            mov eax,[ebp+0x8]
000062BE  8845E4            mov [ebp-0x1c],al
000062C1  C745F4A0860100    mov dword [ebp-0xc],0x186a0
000062C8  807DE400          cmp byte [ebp-0x1c],0x0
000062CC  7540              jnz 0x630e
000062CE  EB18              jmp short 0x62e8
000062D0  83EC0C            sub esp,byte +0xc
000062D3  6A64              push byte +0x64
000062D5  E82BE9FFFF        call 0x4c05
000062DA  83C410            add esp,byte +0x10
000062DD  0FB6C0            movzx eax,al
000062E0  83E001            and eax,byte +0x1
000062E3  83F801            cmp eax,byte +0x1
000062E6  7435              jz 0x631d
000062E8  8B45F4            mov eax,[ebp-0xc]
000062EB  8D50FF            lea edx,[eax-0x1]
000062EE  8955F4            mov [ebp-0xc],edx
000062F1  85C0              test eax,eax
000062F3  75DB              jnz 0x62d0
000062F5  EB2A              jmp short 0x6321
000062F7  83EC0C            sub esp,byte +0xc
000062FA  6A64              push byte +0x64
000062FC  E804E9FFFF        call 0x4c05
00006301  83C410            add esp,byte +0x10
00006304  0FB6C0            movzx eax,al
00006307  83E002            and eax,byte +0x2
0000630A  85C0              test eax,eax
0000630C  7412              jz 0x6320
0000630E  8B45F4            mov eax,[ebp-0xc]
00006311  8D50FF            lea edx,[eax-0x1]
00006314  8955F4            mov [ebp-0xc],edx
00006317  85C0              test eax,eax
00006319  75DC              jnz 0x62f7
0000631B  EB04              jmp short 0x6321
0000631D  90                nop
0000631E  EB01              jmp short 0x6321
00006320  90                nop
00006321  C9                leave
00006322  C3                ret
00006323  55                push ebp
00006324  89E5              mov ebp,esp
00006326  83EC18            sub esp,byte +0x18
00006329  8B4508            mov eax,[ebp+0x8]
0000632C  8845F4            mov [ebp-0xc],al
0000632F  83EC0C            sub esp,byte +0xc
00006332  6A01              push byte +0x1
00006334  E87CFFFFFF        call 0x62b5
00006339  83C410            add esp,byte +0x10
0000633C  83EC08            sub esp,byte +0x8
0000633F  68D4000000        push dword 0xd4
00006344  6A64              push byte +0x64
00006346  E89BE8FFFF        call 0x4be6
0000634B  83C410            add esp,byte +0x10
0000634E  83EC0C            sub esp,byte +0xc
00006351  6A01              push byte +0x1
00006353  E85DFFFFFF        call 0x62b5
00006358  83C410            add esp,byte +0x10
0000635B  0FB645F4          movzx eax,byte [ebp-0xc]
0000635F  83EC08            sub esp,byte +0x8
00006362  50                push eax
00006363  6A60              push byte +0x60
00006365  E87CE8FFFF        call 0x4be6
0000636A  83C410            add esp,byte +0x10
0000636D  90                nop
0000636E  C9                leave
0000636F  C3                ret
00006370  55                push ebp
00006371  89E5              mov ebp,esp
00006373  83EC08            sub esp,byte +0x8
00006376  83EC0C            sub esp,byte +0xc
00006379  6A00              push byte +0x0
0000637B  E835FFFFFF        call 0x62b5
00006380  83C410            add esp,byte +0x10
00006383  83EC0C            sub esp,byte +0xc
00006386  6A60              push byte +0x60
00006388  E878E8FFFF        call 0x4c05
0000638D  83C410            add esp,byte +0x10
00006390  C9                leave
00006391  C3                ret
00006392  55                push ebp
00006393  89E5              mov ebp,esp
00006395  83EC28            sub esp,byte +0x28
00006398  C745F400000000    mov dword [ebp-0xc],0x0
0000639F  E932010000        jmp 0x64d6
000063A4  C745F000000000    mov dword [ebp-0x10],0x0
000063AB  E918010000        jmp 0x64c8
000063B0  8B55F4            mov edx,[ebp-0xc]
000063B3  89D0              mov eax,edx
000063B5  C1E002            shl eax,byte 0x2
000063B8  01D0              add eax,edx
000063BA  8B55F0            mov edx,[ebp-0x10]
000063BD  01D0              add eax,edx
000063BF  0530DC0100        add eax,0x1dc30
000063C4  0FB600            movzx eax,byte [eax]
000063C7  8845EF            mov [ebp-0x11],al
000063CA  807DEF00          cmp byte [ebp-0x11],0x0
000063CE  0F84EF000000      jz near 0x64c3
000063D4  8B45F0            mov eax,[ebp-0x10]
000063D7  8D1400            lea edx,[eax+eax]
000063DA  A128DC0100        mov eax,[0x1dc28]
000063DF  01D0              add eax,edx
000063E1  8945E8            mov [ebp-0x18],eax
000063E4  8B45F4            mov eax,[ebp-0xc]
000063E7  8D1400            lea edx,[eax+eax]
000063EA  A12CDC0100        mov eax,[0x1dc2c]
000063EF  01D0              add eax,edx
000063F1  8945E4            mov [ebp-0x1c],eax
000063F4  807DEF01          cmp byte [ebp-0x11],0x1
000063F8  7507              jnz 0x6401
000063FA  B8FFFFFF00        mov eax,0xffffff
000063FF  EB05              jmp short 0x6406
00006401  B800000000        mov eax,0x0
00006406  8945E0            mov [ebp-0x20],eax
00006409  817DE8FE030000    cmp dword [ebp-0x18],0x3fe
00006410  7F1D              jg 0x642f
00006412  817DE4FE020000    cmp dword [ebp-0x1c],0x2fe
00006419  7F14              jg 0x642f
0000641B  83EC04            sub esp,byte +0x4
0000641E  FF75E0            push dword [ebp-0x20]
00006421  FF75E4            push dword [ebp-0x1c]
00006424  FF75E8            push dword [ebp-0x18]
00006427  E863D0FFFF        call 0x348f
0000642C  83C410            add esp,byte +0x10
0000642F  8B45E8            mov eax,[ebp-0x18]
00006432  83C001            add eax,byte +0x1
00006435  3DFE030000        cmp eax,0x3fe
0000643A  7F21              jg 0x645d
0000643C  817DE4FE020000    cmp dword [ebp-0x1c],0x2fe
00006443  7F18              jg 0x645d
00006445  8B45E8            mov eax,[ebp-0x18]
00006448  83C001            add eax,byte +0x1
0000644B  83EC04            sub esp,byte +0x4
0000644E  FF75E0            push dword [ebp-0x20]
00006451  FF75E4            push dword [ebp-0x1c]
00006454  50                push eax
00006455  E835D0FFFF        call 0x348f
0000645A  83C410            add esp,byte +0x10
0000645D  817DE8FE030000    cmp dword [ebp-0x18],0x3fe
00006464  7F25              jg 0x648b
00006466  8B45E4            mov eax,[ebp-0x1c]
00006469  83C001            add eax,byte +0x1
0000646C  3DFE020000        cmp eax,0x2fe
00006471  7F18              jg 0x648b
00006473  8B45E4            mov eax,[ebp-0x1c]
00006476  83C001            add eax,byte +0x1
00006479  83EC04            sub esp,byte +0x4
0000647C  FF75E0            push dword [ebp-0x20]
0000647F  50                push eax
00006480  FF75E8            push dword [ebp-0x18]
00006483  E807D0FFFF        call 0x348f
00006488  83C410            add esp,byte +0x10
0000648B  8B45E8            mov eax,[ebp-0x18]
0000648E  83C001            add eax,byte +0x1
00006491  3DFE030000        cmp eax,0x3fe
00006496  7F2C              jg 0x64c4
00006498  8B45E4            mov eax,[ebp-0x1c]
0000649B  83C001            add eax,byte +0x1
0000649E  3DFE020000        cmp eax,0x2fe
000064A3  7F1F              jg 0x64c4
000064A5  8B45E4            mov eax,[ebp-0x1c]
000064A8  8D5001            lea edx,[eax+0x1]
000064AB  8B45E8            mov eax,[ebp-0x18]
000064AE  83C001            add eax,byte +0x1
000064B1  83EC04            sub esp,byte +0x4
000064B4  FF75E0            push dword [ebp-0x20]
000064B7  52                push edx
000064B8  50                push eax
000064B9  E8D1CFFFFF        call 0x348f
000064BE  83C410            add esp,byte +0x10
000064C1  EB01              jmp short 0x64c4
000064C3  90                nop
000064C4  8345F001          add dword [ebp-0x10],byte +0x1
000064C8  837DF004          cmp dword [ebp-0x10],byte +0x4
000064CC  0F8EDEFEFFFF      jng near 0x63b0
000064D2  8345F401          add dword [ebp-0xc],byte +0x1
000064D6  837DF404          cmp dword [ebp-0xc],byte +0x4
000064DA  0F8EC4FEFFFF      jng near 0x63a4
000064E0  90                nop
000064E1  C9                leave
000064E2  C3                ret
000064E3  55                push ebp
000064E4  89E5              mov ebp,esp
000064E6  A128DC0100        mov eax,[0x1dc28]
000064EB  5D                pop ebp
000064EC  C3                ret
000064ED  55                push ebp
000064EE  89E5              mov ebp,esp
000064F0  A12CDC0100        mov eax,[0x1dc2c]
000064F5  5D                pop ebp
000064F6  C3                ret
000064F7  55                push ebp
000064F8  89E5              mov ebp,esp
000064FA  83EC10            sub esp,byte +0x10
000064FD  8B1554280200      mov edx,[dword 0x22854]
00006503  A14C280200        mov eax,[0x2284c]
00006508  09D0              or eax,edx
0000650A  8945FC            mov [ebp-0x4],eax
0000650D  C7054C2802000000  mov dword [dword 0x2284c],0x0
         -0000
00006517  8B45FC            mov eax,[ebp-0x4]
0000651A  C9                leave
0000651B  C3                ret
0000651C  55                push ebp
0000651D  89E5              mov ebp,esp
0000651F  0FB60551280200    movzx eax,byte [dword 0x22851]
00006526  0FBEC0            movsx eax,al
00006529  5D                pop ebp
0000652A  C3                ret
0000652B  55                push ebp
0000652C  89E5              mov ebp,esp
0000652E  0FB60552280200    movzx eax,byte [dword 0x22852]
00006535  0FBEC0            movsx eax,al
00006538  5D                pop ebp
00006539  C3                ret
0000653A  55                push ebp
0000653B  89E5              mov ebp,esp
0000653D  0FB60553280200    movzx eax,byte [dword 0x22853]
00006544  0FBEC0            movsx eax,al
00006547  5D                pop ebp
00006548  C3                ret
00006549  55                push ebp
0000654A  89E5              mov ebp,esp
0000654C  83EC18            sub esp,byte +0x18
0000654F  83EC0C            sub esp,byte +0xc
00006552  6A64              push byte +0x64
00006554  E8ACE6FFFF        call 0x4c05
00006559  83C410            add esp,byte +0x10
0000655C  8845EF            mov [ebp-0x11],al
0000655F  0FB645EF          movzx eax,byte [ebp-0x11]
00006563  83E020            and eax,byte +0x20
00006566  85C0              test eax,eax
00006568  0F8442010000      jz near 0x66b0
0000656E  83EC0C            sub esp,byte +0xc
00006571  6A60              push byte +0x60
00006573  E88DE6FFFF        call 0x4c05
00006578  83C410            add esp,byte +0x10
0000657B  8845EE            mov [ebp-0x12],al
0000657E  0FB60550280200    movzx eax,byte [dword 0x22850]
00006585  84C0              test al,al
00006587  7509              jnz 0x6592
00006589  0FB645EE          movzx eax,byte [ebp-0x12]
0000658D  A251280200        mov [0x22851],al
00006592  0FB60550280200    movzx eax,byte [dword 0x22850]
00006599  0FB6C0            movzx eax,al
0000659C  0FB655EE          movzx edx,byte [ebp-0x12]
000065A0  889051280200      mov [eax+0x22851],dl
000065A6  0FB60550280200    movzx eax,byte [dword 0x22850]
000065AD  83C001            add eax,byte +0x1
000065B0  A250280200        mov [0x22850],al
000065B5  0FB60550280200    movzx eax,byte [dword 0x22850]
000065BC  3C03              cmp al,0x3
000065BE  0F85ED000000      jnz near 0x66b1
000065C4  C6055028020000    mov byte [dword 0x22850],0x0
000065CB  0FB60551280200    movzx eax,byte [dword 0x22851]
000065D2  8845ED            mov [ebp-0x13],al
000065D5  0FB645ED          movzx eax,byte [ebp-0x13]
000065D9  83E007            and eax,byte +0x7
000065DC  A354280200        mov [0x22854],eax
000065E1  8B154C280200      mov edx,[dword 0x2284c]
000065E7  A154280200        mov eax,[0x22854]
000065EC  09D0              or eax,edx
000065EE  A34C280200        mov [0x2284c],eax
000065F3  0FB60552280200    movzx eax,byte [dword 0x22852]
000065FA  0FBEC0            movsx eax,al
000065FD  8945F4            mov [ebp-0xc],eax
00006600  0FB60553280200    movzx eax,byte [dword 0x22853]
00006607  0FBEC0            movsx eax,al
0000660A  8945F0            mov [ebp-0x10],eax
0000660D  0FB645ED          movzx eax,byte [ebp-0x13]
00006611  83E010            and eax,byte +0x10
00006614  85C0              test eax,eax
00006616  740B              jz 0x6623
00006618  8B45F4            mov eax,[ebp-0xc]
0000661B  0D00FFFFFF        or eax,0xffffff00
00006620  8945F4            mov [ebp-0xc],eax
00006623  0FB645ED          movzx eax,byte [ebp-0x13]
00006627  83E020            and eax,byte +0x20
0000662A  85C0              test eax,eax
0000662C  740B              jz 0x6639
0000662E  8B45F0            mov eax,[ebp-0x10]
00006631  0D00FFFFFF        or eax,0xffffff00
00006636  8945F0            mov [ebp-0x10],eax
00006639  F75DF0            neg dword [ebp-0x10]
0000663C  8B1528DC0100      mov edx,[dword 0x1dc28]
00006642  8B45F4            mov eax,[ebp-0xc]
00006645  01D0              add eax,edx
00006647  A328DC0100        mov [0x1dc28],eax
0000664C  8B152CDC0100      mov edx,[dword 0x1dc2c]
00006652  8B45F0            mov eax,[ebp-0x10]
00006655  01D0              add eax,edx
00006657  A32CDC0100        mov [0x1dc2c],eax
0000665C  A128DC0100        mov eax,[0x1dc28]
00006661  85C0              test eax,eax
00006663  790A              jns 0x666f
00006665  C70528DC01000000  mov dword [dword 0x1dc28],0x0
         -0000
0000666F  A128DC0100        mov eax,[0x1dc28]
00006674  3DFB030000        cmp eax,0x3fb
00006679  7E0A              jng 0x6685
0000667B  C70528DC0100FC03  mov dword [dword 0x1dc28],0x3fc
         -0000
00006685  A12CDC0100        mov eax,[0x1dc2c]
0000668A  85C0              test eax,eax
0000668C  790A              jns 0x6698
0000668E  C7052CDC01000000  mov dword [dword 0x1dc2c],0x0
         -0000
00006698  A12CDC0100        mov eax,[0x1dc2c]
0000669D  3DFB020000        cmp eax,0x2fb
000066A2  7E0D              jng 0x66b1
000066A4  C7052CDC0100FC02  mov dword [dword 0x1dc2c],0x2fc
         -0000
000066AE  EB01              jmp short 0x66b1
000066B0  90                nop
000066B1  C9                leave
000066B2  C3                ret
000066B3  55                push ebp
000066B4  89E5              mov ebp,esp
000066B6  83EC18            sub esp,byte +0x18
000066B9  83EC0C            sub esp,byte +0xc
000066BC  6A01              push byte +0x1
000066BE  E8F2FBFFFF        call 0x62b5
000066C3  83C410            add esp,byte +0x10
000066C6  83EC08            sub esp,byte +0x8
000066C9  68A8000000        push dword 0xa8
000066CE  6A64              push byte +0x64
000066D0  E811E5FFFF        call 0x4be6
000066D5  83C410            add esp,byte +0x10
000066D8  83EC0C            sub esp,byte +0xc
000066DB  6A01              push byte +0x1
000066DD  E8D3FBFFFF        call 0x62b5
000066E2  83C410            add esp,byte +0x10
000066E5  83EC08            sub esp,byte +0x8
000066E8  6A20              push byte +0x20
000066EA  6A64              push byte +0x64
000066EC  E8F5E4FFFF        call 0x4be6
000066F1  83C410            add esp,byte +0x10
000066F4  83EC0C            sub esp,byte +0xc
000066F7  6A00              push byte +0x0
000066F9  E8B7FBFFFF        call 0x62b5
000066FE  83C410            add esp,byte +0x10
00006701  83EC0C            sub esp,byte +0xc
00006704  6A60              push byte +0x60
00006706  E8FAE4FFFF        call 0x4c05
0000670B  83C410            add esp,byte +0x10
0000670E  83C803            or eax,byte +0x3
00006711  83E0EF            and eax,byte -0x11
00006714  8845F7            mov [ebp-0x9],al
00006717  83EC0C            sub esp,byte +0xc
0000671A  6A01              push byte +0x1
0000671C  E894FBFFFF        call 0x62b5
00006721  83C410            add esp,byte +0x10
00006724  83EC08            sub esp,byte +0x8
00006727  6A60              push byte +0x60
00006729  6A64              push byte +0x64
0000672B  E8B6E4FFFF        call 0x4be6
00006730  83C410            add esp,byte +0x10
00006733  83EC0C            sub esp,byte +0xc
00006736  6A01              push byte +0x1
00006738  E878FBFFFF        call 0x62b5
0000673D  83C410            add esp,byte +0x10
00006740  0FB645F7          movzx eax,byte [ebp-0x9]
00006744  83EC08            sub esp,byte +0x8
00006747  50                push eax
00006748  6A60              push byte +0x60
0000674A  E897E4FFFF        call 0x4be6
0000674F  83C410            add esp,byte +0x10
00006752  83EC0C            sub esp,byte +0xc
00006755  68F6000000        push dword 0xf6
0000675A  E8C4FBFFFF        call 0x6323
0000675F  83C410            add esp,byte +0x10
00006762  E809FCFFFF        call 0x6370
00006767  83EC0C            sub esp,byte +0xc
0000676A  68F4000000        push dword 0xf4
0000676F  E8AFFBFFFF        call 0x6323
00006774  83C410            add esp,byte +0x10
00006777  E8F4FBFFFF        call 0x6370
0000677C  90                nop
0000677D  C9                leave
0000677E  C3                ret
0000677F  55                push ebp
00006780  89E5              mov ebp,esp
00006782  8B4508            mov eax,[ebp+0x8]
00006785  A34CDC0100        mov [0x1dc4c],eax
0000678A  8B4508            mov eax,[ebp+0x8]
0000678D  A35C280200        mov [0x2285c],eax
00006792  8B450C            mov eax,[ebp+0xc]
00006795  A358280200        mov [0x22858],eax
0000679A  90                nop
0000679B  5D                pop ebp
0000679C  C3                ret
0000679D  55                push ebp
0000679E  89E5              mov ebp,esp
000067A0  83EC10            sub esp,byte +0x10
000067A3  A14CDC0100        mov eax,[0x1dc4c]
000067A8  8945FC            mov [ebp-0x4],eax
000067AB  8B154CDC0100      mov edx,[dword 0x1dc4c]
000067B1  8B4508            mov eax,[ebp+0x8]
000067B4  01D0              add eax,edx
000067B6  A34CDC0100        mov [0x1dc4c],eax
000067BB  8B45FC            mov eax,[ebp-0x4]
000067BE  C9                leave
000067BF  C3                ret
000067C0  55                push ebp
000067C1  89E5              mov ebp,esp
000067C3  90                nop
000067C4  5D                pop ebp
000067C5  C3                ret
000067C6  55                push ebp
000067C7  89E5              mov ebp,esp
000067C9  8B154CDC0100      mov edx,[dword 0x1dc4c]
000067CF  A15C280200        mov eax,[0x2285c]
000067D4  29C2              sub edx,eax
000067D6  89D0              mov eax,edx
000067D8  5D                pop ebp
000067D9  C3                ret
000067DA  55                push ebp
000067DB  89E5              mov ebp,esp
000067DD  A158280200        mov eax,[0x22858]
000067E2  5D                pop ebp
000067E3  C3                ret
000067E4  55                push ebp
000067E5  89E5              mov ebp,esp
000067E7  83EC10            sub esp,byte +0x10
000067EA  C745FC00000000    mov dword [ebp-0x4],0x0
000067F1  EB19              jmp short 0x680c
000067F3  8B55FC            mov edx,[ebp-0x4]
000067F6  8B4508            mov eax,[ebp+0x8]
000067F9  01D0              add eax,edx
000067FB  8B4DFC            mov ecx,[ebp-0x4]
000067FE  8B550C            mov edx,[ebp+0xc]
00006801  01CA              add edx,ecx
00006803  0FB600            movzx eax,byte [eax]
00006806  8802              mov [edx],al
00006808  8345FC01          add dword [ebp-0x4],byte +0x1
0000680C  8B45FC            mov eax,[ebp-0x4]
0000680F  3B4510            cmp eax,[ebp+0x10]
00006812  7CDF              jl 0x67f3
00006814  90                nop
00006815  C9                leave
00006816  C3                ret
00006817  55                push ebp
00006818  89E5              mov ebp,esp
0000681A  83EC08            sub esp,byte +0x8
0000681D  83EC08            sub esp,byte +0x8
00006820  6A0A              push byte +0xa
00006822  6A70              push byte +0x70
00006824  E8BDE3FFFF        call 0x4be6
00006829  83C410            add esp,byte +0x10
0000682C  83EC0C            sub esp,byte +0xc
0000682F  6A71              push byte +0x71
00006831  E8CFE3FFFF        call 0x4c05
00006836  83C410            add esp,byte +0x10
00006839  0FB6C0            movzx eax,al
0000683C  2580000000        and eax,0x80
00006841  C9                leave
00006842  C3                ret
00006843  55                push ebp
00006844  89E5              mov ebp,esp
00006846  83EC08            sub esp,byte +0x8
00006849  8B4508            mov eax,[ebp+0x8]
0000684C  0FB6C0            movzx eax,al
0000684F  83EC08            sub esp,byte +0x8
00006852  50                push eax
00006853  6A70              push byte +0x70
00006855  E88CE3FFFF        call 0x4be6
0000685A  83C410            add esp,byte +0x10
0000685D  83EC0C            sub esp,byte +0xc
00006860  6A71              push byte +0x71
00006862  E89EE3FFFF        call 0x4c05
00006867  83C410            add esp,byte +0x10
0000686A  C9                leave
0000686B  C3                ret
0000686C  55                push ebp
0000686D  89E5              mov ebp,esp
0000686F  90                nop
00006870  5D                pop ebp
00006871  C3                ret
00006872  55                push ebp
00006873  89E5              mov ebp,esp
00006875  83EC18            sub esp,byte +0x18
00006878  90                nop
00006879  E899FFFFFF        call 0x6817
0000687E  85C0              test eax,eax
00006880  75F7              jnz 0x6879
00006882  83EC0C            sub esp,byte +0xc
00006885  6A00              push byte +0x0
00006887  E8B7FFFFFF        call 0x6843
0000688C  83C410            add esp,byte +0x10
0000688F  89C2              mov edx,eax
00006891  8B4508            mov eax,[ebp+0x8]
00006894  8810              mov [eax],dl
00006896  83EC0C            sub esp,byte +0xc
00006899  6A02              push byte +0x2
0000689B  E8A3FFFFFF        call 0x6843
000068A0  83C410            add esp,byte +0x10
000068A3  89C2              mov edx,eax
000068A5  8B4508            mov eax,[ebp+0x8]
000068A8  885001            mov [eax+0x1],dl
000068AB  83EC0C            sub esp,byte +0xc
000068AE  6A04              push byte +0x4
000068B0  E88EFFFFFF        call 0x6843
000068B5  83C410            add esp,byte +0x10
000068B8  89C2              mov edx,eax
000068BA  8B4508            mov eax,[ebp+0x8]
000068BD  885002            mov [eax+0x2],dl
000068C0  83EC0C            sub esp,byte +0xc
000068C3  6A07              push byte +0x7
000068C5  E879FFFFFF        call 0x6843
000068CA  83C410            add esp,byte +0x10
000068CD  89C2              mov edx,eax
000068CF  8B4508            mov eax,[ebp+0x8]
000068D2  885003            mov [eax+0x3],dl
000068D5  83EC0C            sub esp,byte +0xc
000068D8  6A08              push byte +0x8
000068DA  E864FFFFFF        call 0x6843
000068DF  83C410            add esp,byte +0x10
000068E2  89C2              mov edx,eax
000068E4  8B4508            mov eax,[ebp+0x8]
000068E7  885004            mov [eax+0x4],dl
000068EA  83EC0C            sub esp,byte +0xc
000068ED  6A09              push byte +0x9
000068EF  E84FFFFFFF        call 0x6843
000068F4  83C410            add esp,byte +0x10
000068F7  89C2              mov edx,eax
000068F9  8B4508            mov eax,[ebp+0x8]
000068FC  885005            mov [eax+0x5],dl
000068FF  83EC0C            sub esp,byte +0xc
00006902  6A0B              push byte +0xb
00006904  E83AFFFFFF        call 0x6843
00006909  83C410            add esp,byte +0x10
0000690C  8845F7            mov [ebp-0x9],al
0000690F  0FB645F7          movzx eax,byte [ebp-0x9]
00006913  83E004            and eax,byte +0x4
00006916  85C0              test eax,eax
00006918  0F8516010000      jnz near 0x6a34
0000691E  8B4508            mov eax,[ebp+0x8]
00006921  0FB600            movzx eax,byte [eax]
00006924  83E00F            and eax,byte +0xf
00006927  89C1              mov ecx,eax
00006929  8B4508            mov eax,[ebp+0x8]
0000692C  0FB600            movzx eax,byte [eax]
0000692F  C0E804            shr al,byte 0x4
00006932  89C2              mov edx,eax
00006934  89D0              mov eax,edx
00006936  C1E002            shl eax,byte 0x2
00006939  01D0              add eax,edx
0000693B  01C0              add eax,eax
0000693D  8D1401            lea edx,[ecx+eax]
00006940  8B4508            mov eax,[ebp+0x8]
00006943  8810              mov [eax],dl
00006945  8B4508            mov eax,[ebp+0x8]
00006948  0FB64001          movzx eax,byte [eax+0x1]
0000694C  83E00F            and eax,byte +0xf
0000694F  89C1              mov ecx,eax
00006951  8B4508            mov eax,[ebp+0x8]
00006954  0FB64001          movzx eax,byte [eax+0x1]
00006958  C0E804            shr al,byte 0x4
0000695B  89C2              mov edx,eax
0000695D  89D0              mov eax,edx
0000695F  C1E002            shl eax,byte 0x2
00006962  01D0              add eax,edx
00006964  01C0              add eax,eax
00006966  8D1401            lea edx,[ecx+eax]
00006969  8B4508            mov eax,[ebp+0x8]
0000696C  885001            mov [eax+0x1],dl
0000696F  8B4508            mov eax,[ebp+0x8]
00006972  0FB64002          movzx eax,byte [eax+0x2]
00006976  83E00F            and eax,byte +0xf
00006979  89C1              mov ecx,eax
0000697B  8B4508            mov eax,[ebp+0x8]
0000697E  0FB64002          movzx eax,byte [eax+0x2]
00006982  0FB6C0            movzx eax,al
00006985  83E070            and eax,byte +0x70
00006988  8D500F            lea edx,[eax+0xf]
0000698B  85C0              test eax,eax
0000698D  0F48C2            cmovs eax,edx
00006990  C1F804            sar eax,byte 0x4
00006993  89C2              mov edx,eax
00006995  89D0              mov eax,edx
00006997  C1E002            shl eax,byte 0x2
0000699A  01D0              add eax,edx
0000699C  01C0              add eax,eax
0000699E  01C8              add eax,ecx
000069A0  89C2              mov edx,eax
000069A2  8B4508            mov eax,[ebp+0x8]
000069A5  0FB64002          movzx eax,byte [eax+0x2]
000069A9  83E080            and eax,byte -0x80
000069AC  09D0              or eax,edx
000069AE  89C2              mov edx,eax
000069B0  8B4508            mov eax,[ebp+0x8]
000069B3  885002            mov [eax+0x2],dl
000069B6  8B4508            mov eax,[ebp+0x8]
000069B9  0FB64003          movzx eax,byte [eax+0x3]
000069BD  83E00F            and eax,byte +0xf
000069C0  89C1              mov ecx,eax
000069C2  8B4508            mov eax,[ebp+0x8]
000069C5  0FB64003          movzx eax,byte [eax+0x3]
000069C9  C0E804            shr al,byte 0x4
000069CC  89C2              mov edx,eax
000069CE  89D0              mov eax,edx
000069D0  C1E002            shl eax,byte 0x2
000069D3  01D0              add eax,edx
000069D5  01C0              add eax,eax
000069D7  8D1401            lea edx,[ecx+eax]
000069DA  8B4508            mov eax,[ebp+0x8]
000069DD  885003            mov [eax+0x3],dl
000069E0  8B4508            mov eax,[ebp+0x8]
000069E3  0FB64004          movzx eax,byte [eax+0x4]
000069E7  83E00F            and eax,byte +0xf
000069EA  89C1              mov ecx,eax
000069EC  8B4508            mov eax,[ebp+0x8]
000069EF  0FB64004          movzx eax,byte [eax+0x4]
000069F3  C0E804            shr al,byte 0x4
000069F6  89C2              mov edx,eax
000069F8  89D0              mov eax,edx
000069FA  C1E002            shl eax,byte 0x2
000069FD  01D0              add eax,edx
000069FF  01C0              add eax,eax
00006A01  8D1401            lea edx,[ecx+eax]
00006A04  8B4508            mov eax,[ebp+0x8]
00006A07  885004            mov [eax+0x4],dl
00006A0A  8B4508            mov eax,[ebp+0x8]
00006A0D  0FB64005          movzx eax,byte [eax+0x5]
00006A11  83E00F            and eax,byte +0xf
00006A14  89C1              mov ecx,eax
00006A16  8B4508            mov eax,[ebp+0x8]
00006A19  0FB64005          movzx eax,byte [eax+0x5]
00006A1D  C0E804            shr al,byte 0x4
00006A20  89C2              mov edx,eax
00006A22  89D0              mov eax,edx
00006A24  C1E002            shl eax,byte 0x2
00006A27  01D0              add eax,edx
00006A29  01C0              add eax,eax
00006A2B  8D1401            lea edx,[ecx+eax]
00006A2E  8B4508            mov eax,[ebp+0x8]
00006A31  885005            mov [eax+0x5],dl
00006A34  0FB645F7          movzx eax,byte [ebp-0x9]
00006A38  83E002            and eax,byte +0x2
00006A3B  85C0              test eax,eax
00006A3D  7541              jnz 0x6a80
00006A3F  8B4508            mov eax,[ebp+0x8]
00006A42  0FB64002          movzx eax,byte [eax+0x2]
00006A46  84C0              test al,al
00006A48  7936              jns 0x6a80
00006A4A  8B4508            mov eax,[ebp+0x8]
00006A4D  0FB64002          movzx eax,byte [eax+0x2]
00006A51  0FB6C0            movzx eax,al
00006A54  83E07F            and eax,byte +0x7f
00006A57  8D480C            lea ecx,[eax+0xc]
00006A5A  BAABAAAA2A        mov edx,0x2aaaaaab
00006A5F  89C8              mov eax,ecx
00006A61  F7EA              imul edx
00006A63  C1FA02            sar edx,byte 0x2
00006A66  89C8              mov eax,ecx
00006A68  C1F81F            sar eax,byte 0x1f
00006A6B  29C2              sub edx,eax
00006A6D  89D0              mov eax,edx
00006A6F  01C0              add eax,eax
00006A71  01D0              add eax,edx
00006A73  C1E003            shl eax,byte 0x3
00006A76  29C1              sub ecx,eax
00006A78  89CA              mov edx,ecx
00006A7A  8B4508            mov eax,[ebp+0x8]
00006A7D  885002            mov [eax+0x2],dl
00006A80  90                nop
00006A81  C9                leave
00006A82  C3                ret
00006A83  55                push ebp
00006A84  89E5              mov ebp,esp
00006A86  8B4508            mov eax,[ebp+0x8]
00006A89  99                cdq
00006A8A  89D0              mov eax,edx
00006A8C  334508            xor eax,[ebp+0x8]
00006A8F  29D0              sub eax,edx
00006A91  5D                pop ebp
00006A92  C3                ret
00006A93  55                push ebp
00006A94  89E5              mov ebp,esp
00006A96  83EC18            sub esp,byte +0x18
00006A99  8B4508            mov eax,[ebp+0x8]
00006A9C  8B4010            mov eax,[eax+0x10]
00006A9F  83E804            sub eax,byte +0x4
00006AA2  8945F0            mov [ebp-0x10],eax
00006AA5  8B4508            mov eax,[ebp+0x8]
00006AA8  8B4014            mov eax,[eax+0x14]
00006AAB  83E84A            sub eax,byte +0x4a
00006AAE  8945EC            mov [ebp-0x14],eax
00006AB1  8B45F0            mov eax,[ebp-0x10]
00006AB4  0FAF45EC          imul eax,[ebp-0x14]
00006AB8  C1E002            shl eax,byte 0x2
00006ABB  83EC0C            sub esp,byte +0xc
00006ABE  50                push eax
00006ABF  E8D9FCFFFF        call 0x679d
00006AC4  83C410            add esp,byte +0x10
00006AC7  89C2              mov edx,eax
00006AC9  8B4508            mov eax,[ebp+0x8]
00006ACC  8990DC0C0000      mov [eax+0xcdc],edx
00006AD2  8B4508            mov eax,[ebp+0x8]
00006AD5  C780E00C00000000  mov dword [eax+0xce0],0x0
         -0000
00006ADF  8B4508            mov eax,[ebp+0x8]
00006AE2  C780E40C00000200  mov dword [eax+0xce4],0x2
         -0000
00006AEC  8B4508            mov eax,[ebp+0x8]
00006AEF  C780E80C0000FFFF  mov dword [eax+0xce8],0xffffffff
         -FFFF
00006AF9  8B4508            mov eax,[ebp+0x8]
00006AFC  C780EC0C0000FFFF  mov dword [eax+0xcec],0xffffffff
         -FFFF
00006B06  8B4508            mov eax,[ebp+0x8]
00006B09  8B80DC0C0000      mov eax,[eax+0xcdc]
00006B0F  85C0              test eax,eax
00006B11  7430              jz 0x6b43
00006B13  C745F400000000    mov dword [ebp-0xc],0x0
00006B1A  EB1B              jmp short 0x6b37
00006B1C  8B4508            mov eax,[ebp+0x8]
00006B1F  8B80DC0C0000      mov eax,[eax+0xcdc]
00006B25  8B55F4            mov edx,[ebp-0xc]
00006B28  C1E202            shl edx,byte 0x2
00006B2B  01D0              add eax,edx
00006B2D  C700FFFFFF00      mov dword [eax],0xffffff
00006B33  8345F401          add dword [ebp-0xc],byte +0x1
00006B37  8B45F0            mov eax,[ebp-0x10]
00006B3A  0FAF45EC          imul eax,[ebp-0x14]
00006B3E  3945F4            cmp [ebp-0xc],eax
00006B41  7CD9              jl 0x6b1c
00006B43  90                nop
00006B44  C9                leave
00006B45  C3                ret
00006B46  55                push ebp
00006B47  89E5              mov ebp,esp
00006B49  53                push ebx
00006B4A  83EC64            sub esp,byte +0x64
00006B4D  8B4508            mov eax,[ebp+0x8]
00006B50  8B80DC0C0000      mov eax,[eax+0xcdc]
00006B56  85C0              test eax,eax
00006B58  0F845D020000      jz near 0x6dbb
00006B5E  8B4508            mov eax,[ebp+0x8]
00006B61  8B4008            mov eax,[eax+0x8]
00006B64  83C002            add eax,byte +0x2
00006B67  8945E8            mov [ebp-0x18],eax
00006B6A  8B4508            mov eax,[ebp+0x8]
00006B6D  8B400C            mov eax,[eax+0xc]
00006B70  83C020            add eax,byte +0x20
00006B73  8945E4            mov [ebp-0x1c],eax
00006B76  8B4508            mov eax,[ebp+0x8]
00006B79  8B4010            mov eax,[eax+0x10]
00006B7C  83E804            sub eax,byte +0x4
00006B7F  8945E0            mov [ebp-0x20],eax
00006B82  83EC0C            sub esp,byte +0xc
00006B85  68D0D0D000        push dword 0xd0d0d0
00006B8A  6A28              push byte +0x28
00006B8C  FF75E0            push dword [ebp-0x20]
00006B8F  FF75E4            push dword [ebp-0x1c]
00006B92  FF75E8            push dword [ebp-0x18]
00006B95  E8B3CAFFFF        call 0x364d
00006B9A  83C420            add esp,byte +0x20
00006B9D  C7459C00000000    mov dword [ebp-0x64],0x0
00006BA4  C745A0FFFFFF00    mov dword [ebp-0x60],0xffffff
00006BAB  C745A40000FF00    mov dword [ebp-0x5c],0xff0000
00006BB2  C745A800FF0000    mov dword [ebp-0x58],0xff00
00006BB9  C745ACFF000000    mov dword [ebp-0x54],0xff
00006BC0  C745B000FFFF00    mov dword [ebp-0x50],0xffff00
00006BC7  C745B4FFFF0000    mov dword [ebp-0x4c],0xffff
00006BCE  C745B8FF00FF00    mov dword [ebp-0x48],0xff00ff
00006BD5  C745BC80808000    mov dword [ebp-0x44],0x808080
00006BDC  C745F400000000    mov dword [ebp-0xc],0x0
00006BE3  EB6A              jmp short 0x6c4f
00006BE5  8B45E8            mov eax,[ebp-0x18]
00006BE8  8D500A            lea edx,[eax+0xa]
00006BEB  8B45F4            mov eax,[ebp-0xc]
00006BEE  6BC01E            imul eax,eax,byte +0x1e
00006BF1  01D0              add eax,edx
00006BF3  8945C4            mov [ebp-0x3c],eax
00006BF6  8B45E4            mov eax,[ebp-0x1c]
00006BF9  83C005            add eax,byte +0x5
00006BFC  8945C0            mov [ebp-0x40],eax
00006BFF  8B45F4            mov eax,[ebp-0xc]
00006C02  8B44859C          mov eax,[ebp+eax*4-0x64]
00006C06  83EC0C            sub esp,byte +0xc
00006C09  50                push eax
00006C0A  6A19              push byte +0x19
00006C0C  6A19              push byte +0x19
00006C0E  FF75C0            push dword [ebp-0x40]
00006C11  FF75C4            push dword [ebp-0x3c]
00006C14  E834CAFFFF        call 0x364d
00006C19  83C420            add esp,byte +0x20
00006C1C  8B4508            mov eax,[ebp+0x8]
00006C1F  8B90E00C0000      mov edx,[eax+0xce0]
00006C25  8B45F4            mov eax,[ebp-0xc]
00006C28  8B44859C          mov eax,[ebp+eax*4-0x64]
00006C2C  39C2              cmp edx,eax
00006C2E  751B              jnz 0x6c4b
00006C30  8B45C0            mov eax,[ebp-0x40]
00006C33  83C01A            add eax,byte +0x1a
00006C36  83EC0C            sub esp,byte +0xc
00006C39  6A00              push byte +0x0
00006C3B  6A02              push byte +0x2
00006C3D  6A19              push byte +0x19
00006C3F  50                push eax
00006C40  FF75C4            push dword [ebp-0x3c]
00006C43  E805CAFFFF        call 0x364d
00006C48  83C420            add esp,byte +0x20
00006C4B  8345F401          add dword [ebp-0xc],byte +0x1
00006C4F  837DF408          cmp dword [ebp-0xc],byte +0x8
00006C53  7E90              jng 0x6be5
00006C55  8B45E4            mov eax,[ebp-0x1c]
00006C58  8D500A            lea edx,[eax+0xa]
00006C5B  8B45E8            mov eax,[ebp-0x18]
00006C5E  052C010000        add eax,0x12c
00006C63  6A00              push byte +0x0
00006C65  6880C10100        push dword 0x1c180
00006C6A  52                push edx
00006C6B  50                push eax
00006C6C  E840DDFFFF        call 0x49b1
00006C71  83C410            add esp,byte +0x10
00006C74  8B4508            mov eax,[ebp+0x8]
00006C77  8B90E40C0000      mov edx,[eax+0xce4]
00006C7D  89D0              mov eax,edx
00006C7F  01C0              add eax,eax
00006C81  8D1C10            lea ebx,[eax+edx]
00006C84  8B4508            mov eax,[ebp+0x8]
00006C87  8B90E40C0000      mov edx,[eax+0xce4]
00006C8D  89D0              mov eax,edx
00006C8F  01C0              add eax,eax
00006C91  8D0C10            lea ecx,[eax+edx]
00006C94  8B45E4            mov eax,[ebp-0x1c]
00006C97  8D500A            lea edx,[eax+0xa]
00006C9A  8B45E8            mov eax,[ebp-0x18]
00006C9D  055E010000        add eax,0x15e
00006CA2  83EC0C            sub esp,byte +0xc
00006CA5  6A00              push byte +0x0
00006CA7  53                push ebx
00006CA8  51                push ecx
00006CA9  52                push edx
00006CAA  50                push eax
00006CAB  E89DC9FFFF        call 0x364d
00006CB0  83C420            add esp,byte +0x20
00006CB3  8B45E4            mov eax,[ebp-0x1c]
00006CB6  83C028            add eax,byte +0x28
00006CB9  8945DC            mov [ebp-0x24],eax
00006CBC  8B4508            mov eax,[ebp+0x8]
00006CBF  8B4014            mov eax,[eax+0x14]
00006CC2  83E84A            sub eax,byte +0x4a
00006CC5  8945D8            mov [ebp-0x28],eax
00006CC8  A100250200        mov eax,[0x22500]
00006CCD  85C0              test eax,eax
00006CCF  0F84E9000000      jz near 0x6dbe
00006CD5  C745F000000000    mov dword [ebp-0x10],0x0
00006CDC  E9CC000000        jmp 0x6dad
00006CE1  8B55DC            mov edx,[ebp-0x24]
00006CE4  8B45F0            mov eax,[ebp-0x10]
00006CE7  01D0              add eax,edx
00006CE9  8945D4            mov [ebp-0x2c],eax
00006CEC  817DD4FF020000    cmp dword [ebp-0x2c],0x2ff
00006CF3  0F8FC8000000      jg near 0x6dc1
00006CF9  C745EC00000000    mov dword [ebp-0x14],0x0
00006D00  E995000000        jmp 0x6d9a
00006D05  8B55E8            mov edx,[ebp-0x18]
00006D08  8B45EC            mov eax,[ebp-0x14]
00006D0B  01D0              add eax,edx
00006D0D  8945D0            mov [ebp-0x30],eax
00006D10  817DD0FF030000    cmp dword [ebp-0x30],0x3ff
00006D17  0F8F8B000000      jg near 0x6da8
00006D1D  8B4508            mov eax,[ebp+0x8]
00006D20  8B90DC0C0000      mov edx,[eax+0xcdc]
00006D26  8B45F0            mov eax,[ebp-0x10]
00006D29  0FAF45E0          imul eax,[ebp-0x20]
00006D2D  89C1              mov ecx,eax
00006D2F  8B45EC            mov eax,[ebp-0x14]
00006D32  01C8              add eax,ecx
00006D34  C1E002            shl eax,byte 0x2
00006D37  01D0              add eax,edx
00006D39  8B00              mov eax,[eax]
00006D3B  8945CC            mov [ebp-0x34],eax
00006D3E  8B45D4            mov eax,[ebp-0x2c]
00006D41  C1E00A            shl eax,byte 0xa
00006D44  89C2              mov edx,eax
00006D46  8B45D0            mov eax,[ebp-0x30]
00006D49  01C2              add edx,eax
00006D4B  89D0              mov eax,edx
00006D4D  01C0              add eax,eax
00006D4F  01D0              add eax,edx
00006D51  8945C8            mov [ebp-0x38],eax
00006D54  8B1500250200      mov edx,[dword 0x22500]
00006D5A  8B45C8            mov eax,[ebp-0x38]
00006D5D  01D0              add eax,edx
00006D5F  8B55CC            mov edx,[ebp-0x34]
00006D62  8810              mov [eax],dl
00006D64  8B45CC            mov eax,[ebp-0x34]
00006D67  C1E808            shr eax,byte 0x8
00006D6A  89C1              mov ecx,eax
00006D6C  A100250200        mov eax,[0x22500]
00006D71  8B55C8            mov edx,[ebp-0x38]
00006D74  83C201            add edx,byte +0x1
00006D77  01D0              add eax,edx
00006D79  89CA              mov edx,ecx
00006D7B  8810              mov [eax],dl
00006D7D  8B45CC            mov eax,[ebp-0x34]
00006D80  C1E810            shr eax,byte 0x10
00006D83  89C1              mov ecx,eax
00006D85  A100250200        mov eax,[0x22500]
00006D8A  8B55C8            mov edx,[ebp-0x38]
00006D8D  83C202            add edx,byte +0x2
00006D90  01D0              add eax,edx
00006D92  89CA              mov edx,ecx
00006D94  8810              mov [eax],dl
00006D96  8345EC01          add dword [ebp-0x14],byte +0x1
00006D9A  8B45EC            mov eax,[ebp-0x14]
00006D9D  3B45E0            cmp eax,[ebp-0x20]
00006DA0  0F8C5FFFFFFF      jl near 0x6d05
00006DA6  EB01              jmp short 0x6da9
00006DA8  90                nop
00006DA9  8345F001          add dword [ebp-0x10],byte +0x1
00006DAD  8B45F0            mov eax,[ebp-0x10]
00006DB0  3B45D8            cmp eax,[ebp-0x28]
00006DB3  0F8C28FFFFFF      jl near 0x6ce1
00006DB9  EB07              jmp short 0x6dc2
00006DBB  90                nop
00006DBC  EB04              jmp short 0x6dc2
00006DBE  90                nop
00006DBF  EB01              jmp short 0x6dc2
00006DC1  90                nop
00006DC2  8B5DFC            mov ebx,[ebp-0x4]
00006DC5  C9                leave
00006DC6  C3                ret
00006DC7  55                push ebp
00006DC8  89E5              mov ebp,esp
00006DCA  83EC40            sub esp,byte +0x40
00006DCD  8B4508            mov eax,[ebp+0x8]
00006DD0  8B4008            mov eax,[eax+0x8]
00006DD3  83C002            add eax,byte +0x2
00006DD6  8945F8            mov [ebp-0x8],eax
00006DD9  8B4508            mov eax,[ebp+0x8]
00006DDC  8B400C            mov eax,[eax+0xc]
00006DDF  83C020            add eax,byte +0x20
00006DE2  8945F4            mov [ebp-0xc],eax
00006DE5  8B45F4            mov eax,[ebp-0xc]
00006DE8  83C028            add eax,byte +0x28
00006DEB  394510            cmp [ebp+0x10],eax
00006DEE  0F8DEB000000      jnl near 0x6edf
00006DF4  C745FC00000000    mov dword [ebp-0x4],0x0
00006DFB  E996000000        jmp 0x6e96
00006E00  8B45F8            mov eax,[ebp-0x8]
00006E03  8D500A            lea edx,[eax+0xa]
00006E06  8B45FC            mov eax,[ebp-0x4]
00006E09  6BC01E            imul eax,eax,byte +0x1e
00006E0C  01D0              add eax,edx
00006E0E  8945F0            mov [ebp-0x10],eax
00006E11  8B45F4            mov eax,[ebp-0xc]
00006E14  83C005            add eax,byte +0x5
00006E17  8945EC            mov [ebp-0x14],eax
00006E1A  8B450C            mov eax,[ebp+0xc]
00006E1D  3B45F0            cmp eax,[ebp-0x10]
00006E20  7C70              jl 0x6e92
00006E22  8B45F0            mov eax,[ebp-0x10]
00006E25  83C019            add eax,byte +0x19
00006E28  39450C            cmp [ebp+0xc],eax
00006E2B  7D65              jnl 0x6e92
00006E2D  8B4510            mov eax,[ebp+0x10]
00006E30  3B45EC            cmp eax,[ebp-0x14]
00006E33  7C5D              jl 0x6e92
00006E35  8B45EC            mov eax,[ebp-0x14]
00006E38  83C019            add eax,byte +0x19
00006E3B  394510            cmp [ebp+0x10],eax
00006E3E  7D52              jnl 0x6e92
00006E40  C745C800000000    mov dword [ebp-0x38],0x0
00006E47  C745CCFFFFFF00    mov dword [ebp-0x34],0xffffff
00006E4E  C745D00000FF00    mov dword [ebp-0x30],0xff0000
00006E55  C745D400FF0000    mov dword [ebp-0x2c],0xff00
00006E5C  C745D8FF000000    mov dword [ebp-0x28],0xff
00006E63  C745DC00FFFF00    mov dword [ebp-0x24],0xffff00
00006E6A  C745E0FFFF0000    mov dword [ebp-0x20],0xffff
00006E71  C745E4FF00FF00    mov dword [ebp-0x1c],0xff00ff
00006E78  C745E880808000    mov dword [ebp-0x18],0x808080
00006E7F  8B45FC            mov eax,[ebp-0x4]
00006E82  8B5485C8          mov edx,[ebp+eax*4-0x38]
00006E86  8B4508            mov eax,[ebp+0x8]
00006E89  8990E00C0000      mov [eax+0xce0],edx
00006E8F  90                nop
00006E90  EB74              jmp short 0x6f06
00006E92  8345FC01          add dword [ebp-0x4],byte +0x1
00006E96  837DFC08          cmp dword [ebp-0x4],byte +0x8
00006E9A  0F8E60FFFFFF      jng near 0x6e00
00006EA0  8B45F8            mov eax,[ebp-0x8]
00006EA3  052C010000        add eax,0x12c
00006EA8  39450C            cmp [ebp+0xc],eax
00006EAB  7E59              jng 0x6f06
00006EAD  8B4508            mov eax,[ebp+0x8]
00006EB0  8B80E40C0000      mov eax,[eax+0xce4]
00006EB6  8D5001            lea edx,[eax+0x1]
00006EB9  8B4508            mov eax,[ebp+0x8]
00006EBC  8990E40C0000      mov [eax+0xce4],edx
00006EC2  8B4508            mov eax,[ebp+0x8]
00006EC5  8B80E40C0000      mov eax,[eax+0xce4]
00006ECB  83F805            cmp eax,byte +0x5
00006ECE  7E36              jng 0x6f06
00006ED0  8B4508            mov eax,[ebp+0x8]
00006ED3  C780E40C00000100  mov dword [eax+0xce4],0x1
         -0000
00006EDD  EB27              jmp short 0x6f06
00006EDF  8B450C            mov eax,[ebp+0xc]
00006EE2  2B45F8            sub eax,[ebp-0x8]
00006EE5  89C2              mov edx,eax
00006EE7  8B4508            mov eax,[ebp+0x8]
00006EEA  8990E80C0000      mov [eax+0xce8],edx
00006EF0  8B45F4            mov eax,[ebp-0xc]
00006EF3  8D5028            lea edx,[eax+0x28]
00006EF6  8B4510            mov eax,[ebp+0x10]
00006EF9  29D0              sub eax,edx
00006EFB  89C2              mov edx,eax
00006EFD  8B4508            mov eax,[ebp+0x8]
00006F00  8990EC0C0000      mov [eax+0xcec],edx
00006F06  C9                leave
00006F07  C3                ret
00006F08  55                push ebp
00006F09  89E5              mov ebp,esp
00006F0B  83EC40            sub esp,byte +0x40
00006F0E  8B4508            mov eax,[ebp+0x8]
00006F11  8B4010            mov eax,[eax+0x10]
00006F14  83E804            sub eax,byte +0x4
00006F17  8945F0            mov [ebp-0x10],eax
00006F1A  8B4508            mov eax,[ebp+0x8]
00006F1D  8B4014            mov eax,[eax+0x14]
00006F20  83E84A            sub eax,byte +0x4a
00006F23  8945EC            mov [ebp-0x14],eax
00006F26  8B4514            mov eax,[ebp+0x14]
00006F29  2B450C            sub eax,[ebp+0xc]
00006F2C  50                push eax
00006F2D  E851FBFFFF        call 0x6a83
00006F32  83C404            add esp,byte +0x4
00006F35  8945E8            mov [ebp-0x18],eax
00006F38  8B450C            mov eax,[ebp+0xc]
00006F3B  3B4514            cmp eax,[ebp+0x14]
00006F3E  7D07              jnl 0x6f47
00006F40  B801000000        mov eax,0x1
00006F45  EB05              jmp short 0x6f4c
00006F47  B8FFFFFFFF        mov eax,0xffffffff
00006F4C  8945E4            mov [ebp-0x1c],eax
00006F4F  8B4518            mov eax,[ebp+0x18]
00006F52  2B4510            sub eax,[ebp+0x10]
00006F55  50                push eax
00006F56  E828FBFFFF        call 0x6a83
00006F5B  83C404            add esp,byte +0x4
00006F5E  F7D8              neg eax
00006F60  8945E0            mov [ebp-0x20],eax
00006F63  8B4510            mov eax,[ebp+0x10]
00006F66  3B4518            cmp eax,[ebp+0x18]
00006F69  7D07              jnl 0x6f72
00006F6B  B801000000        mov eax,0x1
00006F70  EB05              jmp short 0x6f77
00006F72  B8FFFFFFFF        mov eax,0xffffffff
00006F77  8945DC            mov [ebp-0x24],eax
00006F7A  8B55E8            mov edx,[ebp-0x18]
00006F7D  8B45E0            mov eax,[ebp-0x20]
00006F80  01D0              add eax,edx
00006F82  8945FC            mov [ebp-0x4],eax
00006F85  8B4508            mov eax,[ebp+0x8]
00006F88  8B80E40C0000      mov eax,[eax+0xce4]
00006F8E  8945D8            mov [ebp-0x28],eax
00006F91  C745F800000000    mov dword [ebp-0x8],0x0
00006F98  EB72              jmp short 0x700c
00006F9A  C745F400000000    mov dword [ebp-0xc],0x0
00006FA1  EB5D              jmp short 0x7000
00006FA3  8B550C            mov edx,[ebp+0xc]
00006FA6  8B45F4            mov eax,[ebp-0xc]
00006FA9  01D0              add eax,edx
00006FAB  8945D0            mov [ebp-0x30],eax
00006FAE  8B5510            mov edx,[ebp+0x10]
00006FB1  8B45F8            mov eax,[ebp-0x8]
00006FB4  01D0              add eax,edx
00006FB6  8945CC            mov [ebp-0x34],eax
00006FB9  837DD000          cmp dword [ebp-0x30],byte +0x0
00006FBD  783D              js 0x6ffc
00006FBF  8B45D0            mov eax,[ebp-0x30]
00006FC2  3B45F0            cmp eax,[ebp-0x10]
00006FC5  7D35              jnl 0x6ffc
00006FC7  837DCC00          cmp dword [ebp-0x34],byte +0x0
00006FCB  782F              js 0x6ffc
00006FCD  8B45CC            mov eax,[ebp-0x34]
00006FD0  3B45EC            cmp eax,[ebp-0x14]
00006FD3  7D27              jnl 0x6ffc
00006FD5  8B4508            mov eax,[ebp+0x8]
00006FD8  8B90DC0C0000      mov edx,[eax+0xcdc]
00006FDE  8B45CC            mov eax,[ebp-0x34]
00006FE1  0FAF45F0          imul eax,[ebp-0x10]
00006FE5  89C1              mov ecx,eax
00006FE7  8B45D0            mov eax,[ebp-0x30]
00006FEA  01C8              add eax,ecx
00006FEC  C1E002            shl eax,byte 0x2
00006FEF  01C2              add edx,eax
00006FF1  8B4508            mov eax,[ebp+0x8]
00006FF4  8B80E00C0000      mov eax,[eax+0xce0]
00006FFA  8902              mov [edx],eax
00006FFC  8345F401          add dword [ebp-0xc],byte +0x1
00007000  8B45F4            mov eax,[ebp-0xc]
00007003  3B45D8            cmp eax,[ebp-0x28]
00007006  7C9B              jl 0x6fa3
00007008  8345F801          add dword [ebp-0x8],byte +0x1
0000700C  8B45F8            mov eax,[ebp-0x8]
0000700F  3B45D8            cmp eax,[ebp-0x28]
00007012  7C86              jl 0x6f9a
00007014  8B450C            mov eax,[ebp+0xc]
00007017  3B4514            cmp eax,[ebp+0x14]
0000701A  7508              jnz 0x7024
0000701C  8B4510            mov eax,[ebp+0x10]
0000701F  3B4518            cmp eax,[ebp+0x18]
00007022  7439              jz 0x705d
00007024  8B45FC            mov eax,[ebp-0x4]
00007027  01C0              add eax,eax
00007029  8945D4            mov [ebp-0x2c],eax
0000702C  8B45D4            mov eax,[ebp-0x2c]
0000702F  3B45E0            cmp eax,[ebp-0x20]
00007032  7C0C              jl 0x7040
00007034  8B45E0            mov eax,[ebp-0x20]
00007037  0145FC            add [ebp-0x4],eax
0000703A  8B45E4            mov eax,[ebp-0x1c]
0000703D  01450C            add [ebp+0xc],eax
00007040  8B45D4            mov eax,[ebp-0x2c]
00007043  3B45E8            cmp eax,[ebp-0x18]
00007046  0F8F39FFFFFF      jg near 0x6f85
0000704C  8B45E8            mov eax,[ebp-0x18]
0000704F  0145FC            add [ebp-0x4],eax
00007052  8B45DC            mov eax,[ebp-0x24]
00007055  014510            add [ebp+0x10],eax
00007058  E928FFFFFF        jmp 0x6f85
0000705D  90                nop
0000705E  90                nop
0000705F  C9                leave
00007060  C3                ret
00007061  55                push ebp
00007062  89E5              mov ebp,esp
00007064  83EC10            sub esp,byte +0x10
00007067  8B4508            mov eax,[ebp+0x8]
0000706A  8B4008            mov eax,[eax+0x8]
0000706D  83C002            add eax,byte +0x2
00007070  8945FC            mov [ebp-0x4],eax
00007073  8B4508            mov eax,[ebp+0x8]
00007076  8B400C            mov eax,[eax+0xc]
00007079  83C048            add eax,byte +0x48
0000707C  8945F8            mov [ebp-0x8],eax
0000707F  8B450C            mov eax,[ebp+0xc]
00007082  2B45FC            sub eax,[ebp-0x4]
00007085  8945F4            mov [ebp-0xc],eax
00007088  8B4510            mov eax,[ebp+0x10]
0000708B  2B45F8            sub eax,[ebp-0x8]
0000708E  8945F0            mov [ebp-0x10],eax
00007091  8B4508            mov eax,[ebp+0x8]
00007094  8B80E80C0000      mov eax,[eax+0xce8]
0000709A  83F8FF            cmp eax,byte -0x1
0000709D  7425              jz 0x70c4
0000709F  8B4508            mov eax,[ebp+0x8]
000070A2  8B90EC0C0000      mov edx,[eax+0xcec]
000070A8  8B4508            mov eax,[ebp+0x8]
000070AB  8B80E80C0000      mov eax,[eax+0xce8]
000070B1  FF75F0            push dword [ebp-0x10]
000070B4  FF75F4            push dword [ebp-0xc]
000070B7  52                push edx
000070B8  50                push eax
000070B9  FF7508            push dword [ebp+0x8]
000070BC  E847FEFFFF        call 0x6f08
000070C1  83C414            add esp,byte +0x14
000070C4  8B4508            mov eax,[ebp+0x8]
000070C7  8B55F4            mov edx,[ebp-0xc]
000070CA  8990E80C0000      mov [eax+0xce8],edx
000070D0  8B4508            mov eax,[ebp+0x8]
000070D3  8B55F0            mov edx,[ebp-0x10]
000070D6  8990EC0C0000      mov [eax+0xcec],edx
000070DC  90                nop
000070DD  C9                leave
000070DE  C3                ret
000070DF  55                push ebp
000070E0  89E5              mov ebp,esp
000070E2  83EC18            sub esp,byte +0x18
000070E5  C745F400000000    mov dword [ebp-0xc],0x0
000070EC  EB17              jmp short 0x7105
000070EE  8B4508            mov eax,[ebp+0x8]
000070F1  8B55F4            mov edx,[ebp-0xc]
000070F4  81C23C030000      add edx,0x33c
000070FA  C7049000000000    mov dword [eax+edx*4],0x0
00007101  8345F401          add dword [ebp-0xc],byte +0x1
00007105  837DF43B          cmp dword [ebp-0xc],byte +0x3b
00007109  7EE3              jng 0x70ee
0000710B  8B4508            mov eax,[ebp+0x8]
0000710E  C780E00D00000000  mov dword [eax+0xde0],0x0
         -0000
00007118  83EC0C            sub esp,byte +0xc
0000711B  8D45EE            lea eax,[ebp-0x12]
0000711E  50                push eax
0000711F  E84EF7FFFF        call 0x6872
00007124  83C410            add esp,byte +0x10
00007127  0FB645EE          movzx eax,byte [ebp-0x12]
0000712B  0FB6D0            movzx edx,al
0000712E  0FB645EF          movzx eax,byte [ebp-0x11]
00007132  0FB6C0            movzx eax,al
00007135  6BC03C            imul eax,eax,byte +0x3c
00007138  01C2              add edx,eax
0000713A  0FB645F0          movzx eax,byte [ebp-0x10]
0000713E  0FB6C0            movzx eax,al
00007141  69C0100E0000      imul eax,eax,dword 0xe10
00007147  01D0              add eax,edx
00007149  89C2              mov edx,eax
0000714B  8B4508            mov eax,[ebp+0x8]
0000714E  8990E40D0000      mov [eax+0xde4],edx
00007154  90                nop
00007155  C9                leave
00007156  C3                ret
00007157  55                push ebp
00007158  89E5              mov ebp,esp
0000715A  83EC28            sub esp,byte +0x28
0000715D  83EC0C            sub esp,byte +0xc
00007160  8D45DE            lea eax,[ebp-0x22]
00007163  50                push eax
00007164  E809F7FFFF        call 0x6872
00007169  83C410            add esp,byte +0x10
0000716C  0FB645DE          movzx eax,byte [ebp-0x22]
00007170  0FB6D0            movzx edx,al
00007173  0FB645DF          movzx eax,byte [ebp-0x21]
00007177  0FB6C0            movzx eax,al
0000717A  6BC03C            imul eax,eax,byte +0x3c
0000717D  01C2              add edx,eax
0000717F  0FB645E0          movzx eax,byte [ebp-0x20]
00007183  0FB6C0            movzx eax,al
00007186  69C0100E0000      imul eax,eax,dword 0xe10
0000718C  01D0              add eax,edx
0000718E  8945F0            mov [ebp-0x10],eax
00007191  8B4508            mov eax,[ebp+0x8]
00007194  8B80E00D0000      mov eax,[eax+0xde0]
0000719A  3945F0            cmp [ebp-0x10],eax
0000719D  0F8482000000      jz near 0x7225
000071A3  8B4508            mov eax,[ebp+0x8]
000071A6  8B55F0            mov edx,[ebp-0x10]
000071A9  8990E00D0000      mov [eax+0xde0],edx
000071AF  C745F400000000    mov dword [ebp-0xc],0x0
000071B6  EB25              jmp short 0x71dd
000071B8  8B45F4            mov eax,[ebp-0xc]
000071BB  8D5001            lea edx,[eax+0x1]
000071BE  8B4508            mov eax,[ebp+0x8]
000071C1  81C23C030000      add edx,0x33c
000071C7  8B1490            mov edx,[eax+edx*4]
000071CA  8B4508            mov eax,[ebp+0x8]
000071CD  8B4DF4            mov ecx,[ebp-0xc]
000071D0  81C13C030000      add ecx,0x33c
000071D6  891488            mov [eax+ecx*4],edx
000071D9  8345F401          add dword [ebp-0xc],byte +0x1
000071DD  837DF43A          cmp dword [ebp-0xc],byte +0x3a
000071E1  7ED5              jng 0x71b8
000071E3  E8DEF5FFFF        call 0x67c6
000071E8  8945EC            mov [ebp-0x14],eax
000071EB  E8EAF5FFFF        call 0x67da
000071F0  8945E8            mov [ebp-0x18],eax
000071F3  837DE800          cmp dword [ebp-0x18],byte +0x0
000071F7  741F              jz 0x7218
000071F9  8B45EC            mov eax,[ebp-0x14]
000071FC  6BC064            imul eax,eax,byte +0x64
000071FF  BA00000000        mov edx,0x0
00007204  F775E8            div dword [ebp-0x18]
00007207  8945E4            mov [ebp-0x1c],eax
0000720A  8B4508            mov eax,[ebp+0x8]
0000720D  8B55E4            mov edx,[ebp-0x1c]
00007210  8990DC0D0000      mov [eax+0xddc],edx
00007216  EB0D              jmp short 0x7225
00007218  8B4508            mov eax,[ebp+0x8]
0000721B  C780DC0D00000000  mov dword [eax+0xddc],0x0
         -0000
00007225  90                nop
00007226  C9                leave
00007227  C3                ret
00007228  55                push ebp
00007229  89E5              mov ebp,esp
0000722B  53                push ebx
0000722C  81ECA4000000      sub esp,0xa4
00007232  8B4508            mov eax,[ebp+0x8]
00007235  8B4008            mov eax,[eax+0x8]
00007238  83C002            add eax,byte +0x2
0000723B  8945E4            mov [ebp-0x1c],eax
0000723E  8B4508            mov eax,[ebp+0x8]
00007241  8B400C            mov eax,[eax+0xc]
00007244  83C020            add eax,byte +0x20
00007247  8945E0            mov [ebp-0x20],eax
0000724A  8B4508            mov eax,[ebp+0x8]
0000724D  8B4010            mov eax,[eax+0x10]
00007250  83E804            sub eax,byte +0x4
00007253  8945DC            mov [ebp-0x24],eax
00007256  8B4508            mov eax,[ebp+0x8]
00007259  8B4014            mov eax,[eax+0x14]
0000725C  83E823            sub eax,byte +0x23
0000725F  8945D8            mov [ebp-0x28],eax
00007262  83EC0C            sub esp,byte +0xc
00007265  68E0E0E000        push dword 0xe0e0e0
0000726A  FF75D8            push dword [ebp-0x28]
0000726D  FF75DC            push dword [ebp-0x24]
00007270  FF75E0            push dword [ebp-0x20]
00007273  FF75E4            push dword [ebp-0x1c]
00007276  E8D2C3FFFF        call 0x364d
0000727B  83C420            add esp,byte +0x20
0000727E  E843F5FFFF        call 0x67c6
00007283  C1E814            shr eax,byte 0x14
00007286  8945D4            mov [ebp-0x2c],eax
00007289  E84CF5FFFF        call 0x67da
0000728E  C1E814            shr eax,byte 0x14
00007291  8945D0            mov [ebp-0x30],eax
00007294  8B45E0            mov eax,[ebp-0x20]
00007297  8D500A            lea edx,[eax+0xa]
0000729A  8B45E4            mov eax,[ebp-0x1c]
0000729D  83C00A            add eax,byte +0xa
000072A0  6A00              push byte +0x0
000072A2  6886C10100        push dword 0x1c186
000072A7  52                push edx
000072A8  50                push eax
000072A9  E803D7FFFF        call 0x49b1
000072AE  83C410            add esp,byte +0x10
000072B1  8B45E0            mov eax,[ebp-0x20]
000072B4  8D501E            lea edx,[eax+0x1e]
000072B7  8B45E4            mov eax,[ebp-0x1c]
000072BA  83C00A            add eax,byte +0xa
000072BD  6A00              push byte +0x0
000072BF  6895C10100        push dword 0x1c195
000072C4  52                push edx
000072C5  50                push eax
000072C6  E8E6D6FFFF        call 0x49b1
000072CB  83C410            add esp,byte +0x10
000072CE  C745F400000000    mov dword [ebp-0xc],0x0
000072D5  8B45D4            mov eax,[ebp-0x2c]
000072D8  8945F0            mov [ebp-0x10],eax
000072DB  837DF000          cmp dword [ebp-0x10],byte +0x0
000072DF  7562              jnz 0x7343
000072E1  8B45F4            mov eax,[ebp-0xc]
000072E4  8D5001            lea edx,[eax+0x1]
000072E7  8955F4            mov [ebp-0xc],edx
000072EA  C644058430        mov byte [ebp+eax-0x7c],0x30
000072EF  EB52              jmp short 0x7343
000072F1  8B4DF0            mov ecx,[ebp-0x10]
000072F4  BA67666666        mov edx,0x66666667
000072F9  89C8              mov eax,ecx
000072FB  F7EA              imul edx
000072FD  C1FA02            sar edx,byte 0x2
00007300  89C8              mov eax,ecx
00007302  C1F81F            sar eax,byte 0x1f
00007305  29C2              sub edx,eax
00007307  89D0              mov eax,edx
00007309  C1E002            shl eax,byte 0x2
0000730C  01D0              add eax,edx
0000730E  01C0              add eax,eax
00007310  29C1              sub ecx,eax
00007312  89CA              mov edx,ecx
00007314  89D0              mov eax,edx
00007316  8D4830            lea ecx,[eax+0x30]
00007319  8B45F4            mov eax,[ebp-0xc]
0000731C  8D5001            lea edx,[eax+0x1]
0000731F  8955F4            mov [ebp-0xc],edx
00007322  89CA              mov edx,ecx
00007324  88540584          mov [ebp+eax-0x7c],dl
00007328  8B4DF0            mov ecx,[ebp-0x10]
0000732B  BA67666666        mov edx,0x66666667
00007330  89C8              mov eax,ecx
00007332  F7EA              imul edx
00007334  C1FA02            sar edx,byte 0x2
00007337  89C8              mov eax,ecx
00007339  C1F81F            sar eax,byte 0x1f
0000733C  29C2              sub edx,eax
0000733E  89D0              mov eax,edx
00007340  8945F0            mov [ebp-0x10],eax
00007343  837DF000          cmp dword [ebp-0x10],byte +0x0
00007347  7FA8              jg 0x72f1
00007349  C745EC00000000    mov dword [ebp-0x14],0x0
00007350  EB1F              jmp short 0x7371
00007352  836DF401          sub dword [ebp-0xc],byte +0x1
00007356  8B45EC            mov eax,[ebp-0x14]
00007359  8D5001            lea edx,[eax+0x1]
0000735C  8955EC            mov [ebp-0x14],edx
0000735F  8D4D84            lea ecx,[ebp-0x7c]
00007362  8B55F4            mov edx,[ebp-0xc]
00007365  01CA              add edx,ecx
00007367  0FB612            movzx edx,byte [edx]
0000736A  88940564FFFFFF    mov [ebp+eax-0x9c],dl
00007371  837DF400          cmp dword [ebp-0xc],byte +0x0
00007375  7FDB              jg 0x7352
00007377  8D9564FFFFFF      lea edx,[ebp-0x9c]
0000737D  8B45EC            mov eax,[ebp-0x14]
00007380  01D0              add eax,edx
00007382  C60000            mov byte [eax],0x0
00007385  8B45E0            mov eax,[ebp-0x20]
00007388  8D481E            lea ecx,[eax+0x1e]
0000738B  8B45E4            mov eax,[ebp-0x1c]
0000738E  8D5046            lea edx,[eax+0x46]
00007391  6A00              push byte +0x0
00007393  8D8564FFFFFF      lea eax,[ebp-0x9c]
00007399  50                push eax
0000739A  51                push ecx
0000739B  52                push edx
0000739C  E810D6FFFF        call 0x49b1
000073A1  83C410            add esp,byte +0x10
000073A4  8B45E0            mov eax,[ebp-0x20]
000073A7  8D501E            lea edx,[eax+0x1e]
000073AA  8B45E4            mov eax,[ebp-0x1c]
000073AD  83C064            add eax,byte +0x64
000073B0  6A00              push byte +0x0
000073B2  689DC10100        push dword 0x1c19d
000073B7  52                push edx
000073B8  50                push eax
000073B9  E8F3D5FFFF        call 0x49b1
000073BE  83C410            add esp,byte +0x10
000073C1  C745F400000000    mov dword [ebp-0xc],0x0
000073C8  8B45D0            mov eax,[ebp-0x30]
000073CB  8945F0            mov [ebp-0x10],eax
000073CE  837DF000          cmp dword [ebp-0x10],byte +0x0
000073D2  7562              jnz 0x7436
000073D4  8B45F4            mov eax,[ebp-0xc]
000073D7  8D5001            lea edx,[eax+0x1]
000073DA  8955F4            mov [ebp-0xc],edx
000073DD  C644058430        mov byte [ebp+eax-0x7c],0x30
000073E2  EB52              jmp short 0x7436
000073E4  8B4DF0            mov ecx,[ebp-0x10]
000073E7  BA67666666        mov edx,0x66666667
000073EC  89C8              mov eax,ecx
000073EE  F7EA              imul edx
000073F0  C1FA02            sar edx,byte 0x2
000073F3  89C8              mov eax,ecx
000073F5  C1F81F            sar eax,byte 0x1f
000073F8  29C2              sub edx,eax
000073FA  89D0              mov eax,edx
000073FC  C1E002            shl eax,byte 0x2
000073FF  01D0              add eax,edx
00007401  01C0              add eax,eax
00007403  29C1              sub ecx,eax
00007405  89CA              mov edx,ecx
00007407  89D0              mov eax,edx
00007409  8D4830            lea ecx,[eax+0x30]
0000740C  8B45F4            mov eax,[ebp-0xc]
0000740F  8D5001            lea edx,[eax+0x1]
00007412  8955F4            mov [ebp-0xc],edx
00007415  89CA              mov edx,ecx
00007417  88540584          mov [ebp+eax-0x7c],dl
0000741B  8B4DF0            mov ecx,[ebp-0x10]
0000741E  BA67666666        mov edx,0x66666667
00007423  89C8              mov eax,ecx
00007425  F7EA              imul edx
00007427  C1FA02            sar edx,byte 0x2
0000742A  89C8              mov eax,ecx
0000742C  C1F81F            sar eax,byte 0x1f
0000742F  29C2              sub edx,eax
00007431  89D0              mov eax,edx
00007433  8945F0            mov [ebp-0x10],eax
00007436  837DF000          cmp dword [ebp-0x10],byte +0x0
0000743A  7FA8              jg 0x73e4
0000743C  C745EC00000000    mov dword [ebp-0x14],0x0
00007443  EB1F              jmp short 0x7464
00007445  836DF401          sub dword [ebp-0xc],byte +0x1
00007449  8B45EC            mov eax,[ebp-0x14]
0000744C  8D5001            lea edx,[eax+0x1]
0000744F  8955EC            mov [ebp-0x14],edx
00007452  8D4D84            lea ecx,[ebp-0x7c]
00007455  8B55F4            mov edx,[ebp-0xc]
00007458  01CA              add edx,ecx
0000745A  0FB612            movzx edx,byte [edx]
0000745D  88940564FFFFFF    mov [ebp+eax-0x9c],dl
00007464  837DF400          cmp dword [ebp-0xc],byte +0x0
00007468  7FDB              jg 0x7445
0000746A  8D9564FFFFFF      lea edx,[ebp-0x9c]
00007470  8B45EC            mov eax,[ebp-0x14]
00007473  01D0              add eax,edx
00007475  C60000            mov byte [eax],0x0
00007478  8B45E0            mov eax,[ebp-0x20]
0000747B  8D481E            lea ecx,[eax+0x1e]
0000747E  8B45E4            mov eax,[ebp-0x1c]
00007481  8D5078            lea edx,[eax+0x78]
00007484  6A00              push byte +0x0
00007486  8D8564FFFFFF      lea eax,[ebp-0x9c]
0000748C  50                push eax
0000748D  51                push ecx
0000748E  52                push edx
0000748F  E81DD5FFFF        call 0x49b1
00007494  83C410            add esp,byte +0x10
00007497  8B45E0            mov eax,[ebp-0x20]
0000749A  8D501E            lea edx,[eax+0x1e]
0000749D  8B45E4            mov eax,[ebp-0x1c]
000074A0  0596000000        add eax,0x96
000074A5  6A00              push byte +0x0
000074A7  689FC10100        push dword 0x1c19f
000074AC  52                push edx
000074AD  50                push eax
000074AE  E8FED4FFFF        call 0x49b1
000074B3  83C410            add esp,byte +0x10
000074B6  8B45E0            mov eax,[ebp-0x20]
000074B9  8D501E            lea edx,[eax+0x1e]
000074BC  8B45E4            mov eax,[ebp-0x1c]
000074BF  05C8000000        add eax,0xc8
000074C4  6A00              push byte +0x0
000074C6  68A2C10100        push dword 0x1c1a2
000074CB  52                push edx
000074CC  50                push eax
000074CD  E8DFD4FFFF        call 0x49b1
000074D2  83C410            add esp,byte +0x10
000074D5  C745F400000000    mov dword [ebp-0xc],0x0
000074DC  8B4508            mov eax,[ebp+0x8]
000074DF  8B90E00D0000      mov edx,[eax+0xde0]
000074E5  8B4508            mov eax,[ebp+0x8]
000074E8  8B80E40D0000      mov eax,[eax+0xde4]
000074EE  29C2              sub edx,eax
000074F0  89D0              mov eax,edx
000074F2  8945F0            mov [ebp-0x10],eax
000074F5  837DF000          cmp dword [ebp-0x10],byte +0x0
000074F9  7907              jns 0x7502
000074FB  C745F000000000    mov dword [ebp-0x10],0x0
00007502  837DF000          cmp dword [ebp-0x10],byte +0x0
00007506  7562              jnz 0x756a
00007508  8B45F4            mov eax,[ebp-0xc]
0000750B  8D5001            lea edx,[eax+0x1]
0000750E  8955F4            mov [ebp-0xc],edx
00007511  C644058430        mov byte [ebp+eax-0x7c],0x30
00007516  EB52              jmp short 0x756a
00007518  8B4DF0            mov ecx,[ebp-0x10]
0000751B  BA67666666        mov edx,0x66666667
00007520  89C8              mov eax,ecx
00007522  F7EA              imul edx
00007524  C1FA02            sar edx,byte 0x2
00007527  89C8              mov eax,ecx
00007529  C1F81F            sar eax,byte 0x1f
0000752C  29C2              sub edx,eax
0000752E  89D0              mov eax,edx
00007530  C1E002            shl eax,byte 0x2
00007533  01D0              add eax,edx
00007535  01C0              add eax,eax
00007537  29C1              sub ecx,eax
00007539  89CA              mov edx,ecx
0000753B  89D0              mov eax,edx
0000753D  8D4830            lea ecx,[eax+0x30]
00007540  8B45F4            mov eax,[ebp-0xc]
00007543  8D5001            lea edx,[eax+0x1]
00007546  8955F4            mov [ebp-0xc],edx
00007549  89CA              mov edx,ecx
0000754B  88540584          mov [ebp+eax-0x7c],dl
0000754F  8B4DF0            mov ecx,[ebp-0x10]
00007552  BA67666666        mov edx,0x66666667
00007557  89C8              mov eax,ecx
00007559  F7EA              imul edx
0000755B  C1FA02            sar edx,byte 0x2
0000755E  89C8              mov eax,ecx
00007560  C1F81F            sar eax,byte 0x1f
00007563  29C2              sub edx,eax
00007565  89D0              mov eax,edx
00007567  8945F0            mov [ebp-0x10],eax
0000756A  837DF000          cmp dword [ebp-0x10],byte +0x0
0000756E  7FA8              jg 0x7518
00007570  C745EC00000000    mov dword [ebp-0x14],0x0
00007577  EB1F              jmp short 0x7598
00007579  836DF401          sub dword [ebp-0xc],byte +0x1
0000757D  8B45EC            mov eax,[ebp-0x14]
00007580  8D5001            lea edx,[eax+0x1]
00007583  8955EC            mov [ebp-0x14],edx
00007586  8D4D84            lea ecx,[ebp-0x7c]
00007589  8B55F4            mov edx,[ebp-0xc]
0000758C  01CA              add edx,ecx
0000758E  0FB612            movzx edx,byte [edx]
00007591  88940564FFFFFF    mov [ebp+eax-0x9c],dl
00007598  837DF400          cmp dword [ebp-0xc],byte +0x0
0000759C  7FDB              jg 0x7579
0000759E  8D9564FFFFFF      lea edx,[ebp-0x9c]
000075A4  8B45EC            mov eax,[ebp-0x14]
000075A7  01D0              add eax,edx
000075A9  C60000            mov byte [eax],0x0
000075AC  8B45E0            mov eax,[ebp-0x20]
000075AF  8D481E            lea ecx,[eax+0x1e]
000075B2  8B45E4            mov eax,[ebp-0x1c]
000075B5  8D9004010000      lea edx,[eax+0x104]
000075BB  6A00              push byte +0x0
000075BD  8D8564FFFFFF      lea eax,[ebp-0x9c]
000075C3  50                push eax
000075C4  51                push ecx
000075C5  52                push edx
000075C6  E8E6D3FFFF        call 0x49b1
000075CB  83C410            add esp,byte +0x10
000075CE  8B45E0            mov eax,[ebp-0x20]
000075D1  8D501E            lea edx,[eax+0x1e]
000075D4  8B45E4            mov eax,[ebp-0x1c]
000075D7  0522010000        add eax,0x122
000075DC  6A00              push byte +0x0
000075DE  68AAC10100        push dword 0x1c1aa
000075E3  52                push edx
000075E4  50                push eax
000075E5  E8C7D3FFFF        call 0x49b1
000075EA  83C410            add esp,byte +0x10
000075ED  8B45E4            mov eax,[ebp-0x1c]
000075F0  83C00A            add eax,byte +0xa
000075F3  8945CC            mov [ebp-0x34],eax
000075F6  8B45E0            mov eax,[ebp-0x20]
000075F9  83C032            add eax,byte +0x32
000075FC  8945C8            mov [ebp-0x38],eax
000075FF  8B45DC            mov eax,[ebp-0x24]
00007602  83E814            sub eax,byte +0x14
00007605  8945C4            mov [ebp-0x3c],eax
00007608  C745C064000000    mov dword [ebp-0x40],0x64
0000760F  83EC0C            sub esp,byte +0xc
00007612  6A00              push byte +0x0
00007614  FF75C0            push dword [ebp-0x40]
00007617  FF75C4            push dword [ebp-0x3c]
0000761A  FF75C8            push dword [ebp-0x38]
0000761D  FF75CC            push dword [ebp-0x34]
00007620  E828C0FFFF        call 0x364d
00007625  83C420            add esp,byte +0x20
00007628  8B45C0            mov eax,[ebp-0x40]
0000762B  8D58FE            lea ebx,[eax-0x2]
0000762E  8B45C4            mov eax,[ebp-0x3c]
00007631  8D48FE            lea ecx,[eax-0x2]
00007634  8B45C8            mov eax,[ebp-0x38]
00007637  8D5001            lea edx,[eax+0x1]
0000763A  8B45CC            mov eax,[ebp-0x34]
0000763D  83C001            add eax,byte +0x1
00007640  83EC0C            sub esp,byte +0xc
00007643  68FFFFFF00        push dword 0xffffff
00007648  53                push ebx
00007649  51                push ecx
0000764A  52                push edx
0000764B  50                push eax
0000764C  E8FCBFFFFF        call 0x364d
00007651  83C420            add esp,byte +0x20
00007654  8B45C4            mov eax,[ebp-0x3c]
00007657  8D48FE            lea ecx,[eax-0x2]
0000765A  BA89888888        mov edx,0x88888889
0000765F  89C8              mov eax,ecx
00007661  F7EA              imul edx
00007663  8D040A            lea eax,[edx+ecx]
00007666  C1F805            sar eax,byte 0x5
00007669  89C2              mov edx,eax
0000766B  89C8              mov eax,ecx
0000766D  C1F81F            sar eax,byte 0x1f
00007670  29C2              sub edx,eax
00007672  89D0              mov eax,edx
00007674  8945BC            mov [ebp-0x44],eax
00007677  C745E800000000    mov dword [ebp-0x18],0x0
0000767E  E9D2000000        jmp 0x7755
00007683  8B4508            mov eax,[ebp+0x8]
00007686  8B55E8            mov edx,[ebp-0x18]
00007689  81C23C030000      add edx,0x33c
0000768F  8B0490            mov eax,[eax+edx*4]
00007692  8945B8            mov [ebp-0x48],eax
00007695  8B45E8            mov eax,[ebp-0x18]
00007698  8D5001            lea edx,[eax+0x1]
0000769B  8B4508            mov eax,[ebp+0x8]
0000769E  81C23C030000      add edx,0x33c
000076A4  8B0490            mov eax,[eax+edx*4]
000076A7  8945B4            mov [ebp-0x4c],eax
000076AA  8B55C8            mov edx,[ebp-0x38]
000076AD  8B45C0            mov eax,[ebp-0x40]
000076B0  01D0              add eax,edx
000076B2  8D58FF            lea ebx,[eax-0x1]
000076B5  8B45C0            mov eax,[ebp-0x40]
000076B8  83E802            sub eax,byte +0x2
000076BB  0FAF45B8          imul eax,[ebp-0x48]
000076BF  89C1              mov ecx,eax
000076C1  BA1F85EB51        mov edx,0x51eb851f
000076C6  89C8              mov eax,ecx
000076C8  F7EA              imul edx
000076CA  89D0              mov eax,edx
000076CC  C1F805            sar eax,byte 0x5
000076CF  C1F91F            sar ecx,byte 0x1f
000076D2  89CA              mov edx,ecx
000076D4  29C2              sub edx,eax
000076D6  89D0              mov eax,edx
000076D8  01D8              add eax,ebx
000076DA  8945B0            mov [ebp-0x50],eax
000076DD  8B55C8            mov edx,[ebp-0x38]
000076E0  8B45C0            mov eax,[ebp-0x40]
000076E3  01D0              add eax,edx
000076E5  8D58FF            lea ebx,[eax-0x1]
000076E8  8B45C0            mov eax,[ebp-0x40]
000076EB  83E802            sub eax,byte +0x2
000076EE  0FAF45B4          imul eax,[ebp-0x4c]
000076F2  89C1              mov ecx,eax
000076F4  BA1F85EB51        mov edx,0x51eb851f
000076F9  89C8              mov eax,ecx
000076FB  F7EA              imul edx
000076FD  89D0              mov eax,edx
000076FF  C1F805            sar eax,byte 0x5
00007702  C1F91F            sar ecx,byte 0x1f
00007705  89CA              mov edx,ecx
00007707  29C2              sub edx,eax
00007709  89D0              mov eax,edx
0000770B  01D8              add eax,ebx
0000770D  8945AC            mov [ebp-0x54],eax
00007710  8B45CC            mov eax,[ebp-0x34]
00007713  8D5001            lea edx,[eax+0x1]
00007716  8B45E8            mov eax,[ebp-0x18]
00007719  0FAF45BC          imul eax,[ebp-0x44]
0000771D  01D0              add eax,edx
0000771F  8945A8            mov [ebp-0x58],eax
00007722  8B45CC            mov eax,[ebp-0x34]
00007725  8D5001            lea edx,[eax+0x1]
00007728  8B45E8            mov eax,[ebp-0x18]
0000772B  83C001            add eax,byte +0x1
0000772E  0FAF45BC          imul eax,[ebp-0x44]
00007732  01D0              add eax,edx
00007734  8945A4            mov [ebp-0x5c],eax
00007737  83EC0C            sub esp,byte +0xc
0000773A  6800FF0000        push dword 0xff00
0000773F  6A02              push byte +0x2
00007741  6A02              push byte +0x2
00007743  FF75B0            push dword [ebp-0x50]
00007746  FF75A8            push dword [ebp-0x58]
00007749  E8FFBEFFFF        call 0x364d
0000774E  83C420            add esp,byte +0x20
00007751  8345E801          add dword [ebp-0x18],byte +0x1
00007755  837DE83A          cmp dword [ebp-0x18],byte +0x3a
00007759  0F8E24FFFFFF      jng near 0x7683
0000775F  90                nop
00007760  8B5DFC            mov ebx,[ebp-0x4]
00007763  C9                leave
00007764  C3                ret
00007765  55                push ebp
00007766  89E5              mov ebp,esp
00007768  53                push ebx
00007769  83EC04            sub esp,byte +0x4
0000776C  83EC0C            sub esp,byte +0xc
0000776F  6800100000        push dword 0x1000
00007774  E824F0FFFF        call 0x679d
00007779  83C410            add esp,byte +0x10
0000777C  89C2              mov edx,eax
0000777E  8B4508            mov eax,[ebp+0x8]
00007781  8990A00C0000      mov [eax+0xca0],edx
00007787  8B4508            mov eax,[ebp+0x8]
0000778A  C780A80C00000004  mov dword [eax+0xca8],0x400
         -0000
00007794  8B4508            mov eax,[ebp+0x8]
00007797  8B98A00C0000      mov ebx,[eax+0xca0]
0000779D  83EC0C            sub esp,byte +0xc
000077A0  6A01              push byte +0x1
000077A2  E8F6EFFFFF        call 0x679d
000077A7  83C410            add esp,byte +0x10
000077AA  8903              mov [ebx],eax
000077AC  8B4508            mov eax,[ebp+0x8]
000077AF  8B80A00C0000      mov eax,[eax+0xca0]
000077B5  8B00              mov eax,[eax]
000077B7  C60000            mov byte [eax],0x0
000077BA  8B4508            mov eax,[ebp+0x8]
000077BD  C780A40C00000100  mov dword [eax+0xca4],0x1
         -0000
000077C7  8B4508            mov eax,[ebp+0x8]
000077CA  C780B00C00000000  mov dword [eax+0xcb0],0x0
         -0000
000077D4  8B4508            mov eax,[ebp+0x8]
000077D7  C780AC0C00000000  mov dword [eax+0xcac],0x0
         -0000
000077E1  8B4508            mov eax,[ebp+0x8]
000077E4  C780B40C00000000  mov dword [eax+0xcb4],0x0
         -0000
000077EE  8B4508            mov eax,[ebp+0x8]
000077F1  C780B80C00000000  mov dword [eax+0xcb8],0x0
         -0000
000077FB  8B4508            mov eax,[ebp+0x8]
000077FE  C780BC0C00000100  mov dword [eax+0xcbc],0x1
         -0000
00007808  90                nop
00007809  8B5DFC            mov ebx,[ebp-0x4]
0000780C  C9                leave
0000780D  C3                ret
0000780E  55                push ebp
0000780F  89E5              mov ebp,esp
00007811  57                push edi
00007812  56                push esi
00007813  53                push ebx
00007814  83EC6C            sub esp,byte +0x6c
00007817  8D4594            lea eax,[ebp-0x6c]
0000781A  BB40C20100        mov ebx,0x1c240
0000781F  BA14000000        mov edx,0x14
00007824  89C7              mov edi,eax
00007826  89DE              mov esi,ebx
00007828  89D1              mov ecx,edx
0000782A  F3A5              rep movsd
0000782C  C745E400000000    mov dword [ebp-0x1c],0x0
00007833  EB25              jmp short 0x785a
00007835  8B45E4            mov eax,[ebp-0x1c]
00007838  8B448594          mov eax,[ebp+eax*4-0x6c]
0000783C  83EC08            sub esp,byte +0x8
0000783F  50                push eax
00007840  FF7508            push dword [ebp+0x8]
00007843  E812E3FFFF        call 0x5b5a
00007848  83C410            add esp,byte +0x10
0000784B  85C0              test eax,eax
0000784D  7507              jnz 0x7856
0000784F  B801000000        mov eax,0x1
00007854  EB14              jmp short 0x786a
00007856  8345E401          add dword [ebp-0x1c],byte +0x1
0000785A  8B45E4            mov eax,[ebp-0x1c]
0000785D  8B448594          mov eax,[ebp+eax*4-0x6c]
00007861  85C0              test eax,eax
00007863  75D0              jnz 0x7835
00007865  B800000000        mov eax,0x0
0000786A  8D65F4            lea esp,[ebp-0xc]
0000786D  5B                pop ebx
0000786E  5E                pop esi
0000786F  5F                pop edi
00007870  5D                pop ebp
00007871  C3                ret
00007872  55                push ebp
00007873  89E5              mov ebp,esp
00007875  83EC48            sub esp,byte +0x48
00007878  C745F400000000    mov dword [ebp-0xc],0x0
0000787F  8B4508            mov eax,[ebp+0x8]
00007882  8945F0            mov [ebp-0x10],eax
00007885  E978010000        jmp 0x7a02
0000788A  C745EC00000000    mov dword [ebp-0x14],0x0
00007891  8B55F4            mov edx,[ebp-0xc]
00007894  8B4510            mov eax,[ebp+0x10]
00007897  01D0              add eax,edx
00007899  0FB600            movzx eax,byte [eax]
0000789C  3C2F              cmp al,0x2f
0000789E  7533              jnz 0x78d3
000078A0  8B45F4            mov eax,[ebp-0xc]
000078A3  8D5001            lea edx,[eax+0x1]
000078A6  8B4510            mov eax,[ebp+0x10]
000078A9  01D0              add eax,edx
000078AB  0FB600            movzx eax,byte [eax]
000078AE  3C2F              cmp al,0x2f
000078B0  7521              jnz 0x78d3
000078B2  8B55F4            mov edx,[ebp-0xc]
000078B5  8B4510            mov eax,[ebp+0x10]
000078B8  01D0              add eax,edx
000078BA  6800800000        push dword 0x8000
000078BF  50                push eax
000078C0  FF750C            push dword [ebp+0xc]
000078C3  FF75F0            push dword [ebp-0x10]
000078C6  E8E6D0FFFF        call 0x49b1
000078CB  83C410            add esp,byte +0x10
000078CE  E942010000        jmp 0x7a15
000078D3  8B55F4            mov edx,[ebp-0xc]
000078D6  8B4510            mov eax,[ebp+0x10]
000078D9  01D0              add eax,edx
000078DB  0FB600            movzx eax,byte [eax]
000078DE  3C22              cmp al,0x22
000078E0  7507              jnz 0x78e9
000078E2  C745EC0000A000    mov dword [ebp-0x14],0xa00000
000078E9  837DF400          cmp dword [ebp-0xc],byte +0x0
000078ED  7416              jz 0x7905
000078EF  8B45F4            mov eax,[ebp-0xc]
000078F2  8D50FF            lea edx,[eax-0x1]
000078F5  8B4510            mov eax,[ebp+0x10]
000078F8  01D0              add eax,edx
000078FA  0FB600            movzx eax,byte [eax]
000078FD  3C20              cmp al,0x20
000078FF  0F85D5000000      jnz near 0x79da
00007905  8B55F4            mov edx,[ebp-0xc]
00007908  8B4510            mov eax,[ebp+0x10]
0000790B  01D0              add eax,edx
0000790D  0FB600            movzx eax,byte [eax]
00007910  3C60              cmp al,0x60
00007912  0F8EC2000000      jng near 0x79da
00007918  8B55F4            mov edx,[ebp-0xc]
0000791B  8B4510            mov eax,[ebp+0x10]
0000791E  01D0              add eax,edx
00007920  0FB600            movzx eax,byte [eax]
00007923  3C7A              cmp al,0x7a
00007925  0F8FAF000000      jg near 0x79da
0000792B  C745E800000000    mov dword [ebp-0x18],0x0
00007932  8B45F4            mov eax,[ebp-0xc]
00007935  8945E4            mov [ebp-0x1c],eax
00007938  EB21              jmp short 0x795b
0000793A  8B45E4            mov eax,[ebp-0x1c]
0000793D  8D5001            lea edx,[eax+0x1]
00007940  8955E4            mov [ebp-0x1c],edx
00007943  89C2              mov edx,eax
00007945  8B4510            mov eax,[ebp+0x10]
00007948  8D0C02            lea ecx,[edx+eax]
0000794B  8B45E8            mov eax,[ebp-0x18]
0000794E  8D5001            lea edx,[eax+0x1]
00007951  8955E8            mov [ebp-0x18],edx
00007954  0FB611            movzx edx,byte [ecx]
00007957  885405C4          mov [ebp+eax-0x3c],dl
0000795B  8B55E4            mov edx,[ebp-0x1c]
0000795E  8B4510            mov eax,[ebp+0x10]
00007961  01D0              add eax,edx
00007963  0FB600            movzx eax,byte [eax]
00007966  84C0              test al,al
00007968  7424              jz 0x798e
0000796A  8B55E4            mov edx,[ebp-0x1c]
0000796D  8B4510            mov eax,[ebp+0x10]
00007970  01D0              add eax,edx
00007972  0FB600            movzx eax,byte [eax]
00007975  3C60              cmp al,0x60
00007977  7E15              jng 0x798e
00007979  8B55E4            mov edx,[ebp-0x1c]
0000797C  8B4510            mov eax,[ebp+0x10]
0000797F  01D0              add eax,edx
00007981  0FB600            movzx eax,byte [eax]
00007984  3C7A              cmp al,0x7a
00007986  7F06              jg 0x798e
00007988  837DE81E          cmp dword [ebp-0x18],byte +0x1e
0000798C  7EAC              jng 0x793a
0000798E  8D55C4            lea edx,[ebp-0x3c]
00007991  8B45E8            mov eax,[ebp-0x18]
00007994  01D0              add eax,edx
00007996  C60000            mov byte [eax],0x0
00007999  83EC0C            sub esp,byte +0xc
0000799C  8D45C4            lea eax,[ebp-0x3c]
0000799F  50                push eax
000079A0  E869FEFFFF        call 0x780e
000079A5  83C410            add esp,byte +0x10
000079A8  85C0              test eax,eax
000079AA  742E              jz 0x79da
000079AC  68FF000000        push dword 0xff
000079B1  8D45C4            lea eax,[ebp-0x3c]
000079B4  50                push eax
000079B5  FF750C            push dword [ebp+0xc]
000079B8  FF75F0            push dword [ebp-0x10]
000079BB  E8F1CFFFFF        call 0x49b1
000079C0  83C410            add esp,byte +0x10
000079C3  8B55E8            mov edx,[ebp-0x18]
000079C6  89D0              mov eax,edx
000079C8  C1E002            shl eax,byte 0x2
000079CB  01D0              add eax,edx
000079CD  01C0              add eax,eax
000079CF  0145F0            add [ebp-0x10],eax
000079D2  8B45E4            mov eax,[ebp-0x1c]
000079D5  8945F4            mov [ebp-0xc],eax
000079D8  EB28              jmp short 0x7a02
000079DA  8B55F4            mov edx,[ebp-0xc]
000079DD  8B4510            mov eax,[ebp+0x10]
000079E0  01D0              add eax,edx
000079E2  0FB600            movzx eax,byte [eax]
000079E5  0FBEC0            movsx eax,al
000079E8  FF75EC            push dword [ebp-0x14]
000079EB  50                push eax
000079EC  FF750C            push dword [ebp+0xc]
000079EF  FF75F0            push dword [ebp-0x10]
000079F2  E873CEFFFF        call 0x486a
000079F7  83C410            add esp,byte +0x10
000079FA  8345F00A          add dword [ebp-0x10],byte +0xa
000079FE  8345F401          add dword [ebp-0xc],byte +0x1
00007A02  8B55F4            mov edx,[ebp-0xc]
00007A05  8B4510            mov eax,[ebp+0x10]
00007A08  01D0              add eax,edx
00007A0A  0FB600            movzx eax,byte [eax]
00007A0D  84C0              test al,al
00007A0F  0F8575FEFFFF      jnz near 0x788a
00007A15  C9                leave
00007A16  C3                ret
00007A17  55                push ebp
00007A18  89E5              mov ebp,esp
00007A1A  83EC58            sub esp,byte +0x58
00007A1D  8B4508            mov eax,[ebp+0x8]
00007A20  8B4008            mov eax,[eax+0x8]
00007A23  83C002            add eax,byte +0x2
00007A26  8945E4            mov [ebp-0x1c],eax
00007A29  8B4508            mov eax,[ebp+0x8]
00007A2C  8B400C            mov eax,[eax+0xc]
00007A2F  83C020            add eax,byte +0x20
00007A32  8945E0            mov [ebp-0x20],eax
00007A35  8B4508            mov eax,[ebp+0x8]
00007A38  8B4010            mov eax,[eax+0x10]
00007A3B  83E804            sub eax,byte +0x4
00007A3E  8945DC            mov [ebp-0x24],eax
00007A41  8B4508            mov eax,[ebp+0x8]
00007A44  8B4014            mov eax,[eax+0x14]
00007A47  83E823            sub eax,byte +0x23
00007A4A  8945D8            mov [ebp-0x28],eax
00007A4D  83EC0C            sub esp,byte +0xc
00007A50  68FFFFFF00        push dword 0xffffff
00007A55  FF75D8            push dword [ebp-0x28]
00007A58  FF75DC            push dword [ebp-0x24]
00007A5B  FF75E0            push dword [ebp-0x20]
00007A5E  FF75E4            push dword [ebp-0x1c]
00007A61  E8E7BBFFFF        call 0x364d
00007A66  83C420            add esp,byte +0x20
00007A69  83EC0C            sub esp,byte +0xc
00007A6C  68E0E0E000        push dword 0xe0e0e0
00007A71  FF75D8            push dword [ebp-0x28]
00007A74  6A28              push byte +0x28
00007A76  FF75E0            push dword [ebp-0x20]
00007A79  FF75E4            push dword [ebp-0x1c]
00007A7C  E8CCBBFFFF        call 0x364d
00007A81  83C420            add esp,byte +0x20
00007A84  8B45D8            mov eax,[ebp-0x28]
00007A87  8D500F            lea edx,[eax+0xf]
00007A8A  85C0              test eax,eax
00007A8C  0F48C2            cmovs eax,edx
00007A8F  C1F804            sar eax,byte 0x4
00007A92  8945D4            mov [ebp-0x2c],eax
00007A95  C745F400000000    mov dword [ebp-0xc],0x0
00007A9C  E970010000        jmp 0x7c11
00007AA1  8B4508            mov eax,[ebp+0x8]
00007AA4  8B90AC0C0000      mov edx,[eax+0xcac]
00007AAA  8B45F4            mov eax,[ebp-0xc]
00007AAD  01D0              add eax,edx
00007AAF  8945D0            mov [ebp-0x30],eax
00007AB2  8B4508            mov eax,[ebp+0x8]
00007AB5  8B80A40C0000      mov eax,[eax+0xca4]
00007ABB  3945D0            cmp [ebp-0x30],eax
00007ABE  0F8D5B010000      jnl near 0x7c1f
00007AC4  8B45F4            mov eax,[ebp-0xc]
00007AC7  C1E004            shl eax,byte 0x4
00007ACA  89C2              mov edx,eax
00007ACC  8B45E0            mov eax,[ebp-0x20]
00007ACF  01D0              add eax,edx
00007AD1  8945CC            mov [ebp-0x34],eax
00007AD4  8B45D0            mov eax,[ebp-0x30]
00007AD7  83C001            add eax,byte +0x1
00007ADA  8945F0            mov [ebp-0x10],eax
00007ADD  C745EC00000000    mov dword [ebp-0x14],0x0
00007AE4  837DF000          cmp dword [ebp-0x10],byte +0x0
00007AE8  7562              jnz 0x7b4c
00007AEA  8B45EC            mov eax,[ebp-0x14]
00007AED  8D5001            lea edx,[eax+0x1]
00007AF0  8955EC            mov [ebp-0x14],edx
00007AF3  C64405A830        mov byte [ebp+eax-0x58],0x30
00007AF8  EB52              jmp short 0x7b4c
00007AFA  8B4DF0            mov ecx,[ebp-0x10]
00007AFD  BA67666666        mov edx,0x66666667
00007B02  89C8              mov eax,ecx
00007B04  F7EA              imul edx
00007B06  C1FA02            sar edx,byte 0x2
00007B09  89C8              mov eax,ecx
00007B0B  C1F81F            sar eax,byte 0x1f
00007B0E  29C2              sub edx,eax
00007B10  89D0              mov eax,edx
00007B12  C1E002            shl eax,byte 0x2
00007B15  01D0              add eax,edx
00007B17  01C0              add eax,eax
00007B19  29C1              sub ecx,eax
00007B1B  89CA              mov edx,ecx
00007B1D  89D0              mov eax,edx
00007B1F  8D4830            lea ecx,[eax+0x30]
00007B22  8B45EC            mov eax,[ebp-0x14]
00007B25  8D5001            lea edx,[eax+0x1]
00007B28  8955EC            mov [ebp-0x14],edx
00007B2B  89CA              mov edx,ecx
00007B2D  885405A8          mov [ebp+eax-0x58],dl
00007B31  8B4DF0            mov ecx,[ebp-0x10]
00007B34  BA67666666        mov edx,0x66666667
00007B39  89C8              mov eax,ecx
00007B3B  F7EA              imul edx
00007B3D  C1FA02            sar edx,byte 0x2
00007B40  89C8              mov eax,ecx
00007B42  C1F81F            sar eax,byte 0x1f
00007B45  29C2              sub edx,eax
00007B47  89D0              mov eax,edx
00007B49  8945F0            mov [ebp-0x10],eax
00007B4C  837DF000          cmp dword [ebp-0x10],byte +0x0
00007B50  7FA8              jg 0x7afa
00007B52  C745E800000000    mov dword [ebp-0x18],0x0
00007B59  EB3B              jmp short 0x7b96
00007B5B  8D55A8            lea edx,[ebp-0x58]
00007B5E  8B45E8            mov eax,[ebp-0x18]
00007B61  01D0              add eax,edx
00007B63  0FB600            movzx eax,byte [eax]
00007B66  8845CB            mov [ebp-0x35],al
00007B69  8B45EC            mov eax,[ebp-0x14]
00007B6C  83E801            sub eax,byte +0x1
00007B6F  2B45E8            sub eax,[ebp-0x18]
00007B72  0FB64405A8        movzx eax,byte [ebp+eax-0x58]
00007B77  8D4DA8            lea ecx,[ebp-0x58]
00007B7A  8B55E8            mov edx,[ebp-0x18]
00007B7D  01CA              add edx,ecx
00007B7F  8802              mov [edx],al
00007B81  8B45EC            mov eax,[ebp-0x14]
00007B84  83E801            sub eax,byte +0x1
00007B87  2B45E8            sub eax,[ebp-0x18]
00007B8A  0FB655CB          movzx edx,byte [ebp-0x35]
00007B8E  885405A8          mov [ebp+eax-0x58],dl
00007B92  8345E801          add dword [ebp-0x18],byte +0x1
00007B96  8B45EC            mov eax,[ebp-0x14]
00007B99  89C2              mov edx,eax
00007B9B  C1EA1F            shr edx,byte 0x1f
00007B9E  01D0              add eax,edx
00007BA0  D1F8              sar eax,1
00007BA2  3945E8            cmp [ebp-0x18],eax
00007BA5  7CB4              jl 0x7b5b
00007BA7  8D55A8            lea edx,[ebp-0x58]
00007BAA  8B45EC            mov eax,[ebp-0x14]
00007BAD  01D0              add eax,edx
00007BAF  C60000            mov byte [eax],0x0
00007BB2  8B45E4            mov eax,[ebp-0x1c]
00007BB5  8D5005            lea edx,[eax+0x5]
00007BB8  6840404000        push dword 0x404040
00007BBD  8D45A8            lea eax,[ebp-0x58]
00007BC0  50                push eax
00007BC1  FF75CC            push dword [ebp-0x34]
00007BC4  52                push edx
00007BC5  E8E7CDFFFF        call 0x49b1
00007BCA  83C410            add esp,byte +0x10
00007BCD  8B4508            mov eax,[ebp+0x8]
00007BD0  8B80A00C0000      mov eax,[eax+0xca0]
00007BD6  8B55D0            mov edx,[ebp-0x30]
00007BD9  C1E202            shl edx,byte 0x2
00007BDC  01D0              add eax,edx
00007BDE  8B00              mov eax,[eax]
00007BE0  85C0              test eax,eax
00007BE2  7429              jz 0x7c0d
00007BE4  8B4508            mov eax,[ebp+0x8]
00007BE7  8B80A00C0000      mov eax,[eax+0xca0]
00007BED  8B55D0            mov edx,[ebp-0x30]
00007BF0  C1E202            shl edx,byte 0x2
00007BF3  01D0              add eax,edx
00007BF5  8B00              mov eax,[eax]
00007BF7  8B55E4            mov edx,[ebp-0x1c]
00007BFA  83C22D            add edx,byte +0x2d
00007BFD  83EC04            sub esp,byte +0x4
00007C00  50                push eax
00007C01  FF75CC            push dword [ebp-0x34]
00007C04  52                push edx
00007C05  E868FCFFFF        call 0x7872
00007C0A  83C410            add esp,byte +0x10
00007C0D  8345F401          add dword [ebp-0xc],byte +0x1
00007C11  8B45F4            mov eax,[ebp-0xc]
00007C14  3B45D4            cmp eax,[ebp-0x2c]
00007C17  0F8C84FEFFFF      jl near 0x7aa1
00007C1D  EB01              jmp short 0x7c20
00007C1F  90                nop
00007C20  8B4508            mov eax,[ebp+0x8]
00007C23  8B90B40C0000      mov edx,[eax+0xcb4]
00007C29  8B4508            mov eax,[ebp+0x8]
00007C2C  8B80AC0C0000      mov eax,[eax+0xcac]
00007C32  29C2              sub edx,eax
00007C34  89D0              mov eax,edx
00007C36  C1E004            shl eax,byte 0x4
00007C39  8945C4            mov [ebp-0x3c],eax
00007C3C  8B4508            mov eax,[ebp+0x8]
00007C3F  8B90B80C0000      mov edx,[eax+0xcb8]
00007C45  89D0              mov eax,edx
00007C47  C1E002            shl eax,byte 0x2
00007C4A  01D0              add eax,edx
00007C4C  01C0              add eax,eax
00007C4E  83C02D            add eax,byte +0x2d
00007C51  8945C0            mov [ebp-0x40],eax
00007C54  837DC400          cmp dword [ebp-0x3c],byte +0x0
00007C58  7835              js 0x7c8f
00007C5A  8B45C4            mov eax,[ebp-0x3c]
00007C5D  3B45D8            cmp eax,[ebp-0x28]
00007C60  7D2D              jnl 0x7c8f
00007C62  8B55E4            mov edx,[ebp-0x1c]
00007C65  8B45C0            mov eax,[ebp-0x40]
00007C68  01D0              add eax,edx
00007C6A  8945BC            mov [ebp-0x44],eax
00007C6D  8B55E0            mov edx,[ebp-0x20]
00007C70  8B45C4            mov eax,[ebp-0x3c]
00007C73  01D0              add eax,edx
00007C75  8945B8            mov [ebp-0x48],eax
00007C78  83EC0C            sub esp,byte +0xc
00007C7B  6A00              push byte +0x0
00007C7D  6A10              push byte +0x10
00007C7F  6A02              push byte +0x2
00007C81  FF75B8            push dword [ebp-0x48]
00007C84  FF75BC            push dword [ebp-0x44]
00007C87  E8C1B9FFFF        call 0x364d
00007C8C  83C420            add esp,byte +0x20
00007C8F  8B4508            mov eax,[ebp+0x8]
00007C92  8B80A40C0000      mov eax,[eax+0xca4]
00007C98  3945D4            cmp [ebp-0x2c],eax
00007C9B  7D5D              jnl 0x7cfa
00007C9D  8B45D4            mov eax,[ebp-0x2c]
00007CA0  0FAF45D8          imul eax,[ebp-0x28]
00007CA4  8B5508            mov edx,[ebp+0x8]
00007CA7  8B8AA40C0000      mov ecx,[edx+0xca4]
00007CAD  99                cdq
00007CAE  F7F9              idiv ecx
00007CB0  8945B4            mov [ebp-0x4c],eax
00007CB3  8B4508            mov eax,[ebp+0x8]
00007CB6  8B80AC0C0000      mov eax,[eax+0xcac]
00007CBC  0FAF45D8          imul eax,[ebp-0x28]
00007CC0  8B5508            mov edx,[ebp+0x8]
00007CC3  8B8AA40C0000      mov ecx,[edx+0xca4]
00007CC9  99                cdq
00007CCA  F7F9              idiv ecx
00007CCC  89C2              mov edx,eax
00007CCE  8B45E0            mov eax,[ebp-0x20]
00007CD1  01D0              add eax,edx
00007CD3  8945B0            mov [ebp-0x50],eax
00007CD6  8B55E4            mov edx,[ebp-0x1c]
00007CD9  8B45DC            mov eax,[ebp-0x24]
00007CDC  01D0              add eax,edx
00007CDE  83E80A            sub eax,byte +0xa
00007CE1  83EC0C            sub esp,byte +0xc
00007CE4  6880808000        push dword 0x808080
00007CE9  FF75B4            push dword [ebp-0x4c]
00007CEC  6A08              push byte +0x8
00007CEE  FF75B0            push dword [ebp-0x50]
00007CF1  50                push eax
00007CF2  E856B9FFFF        call 0x364d
00007CF7  83C420            add esp,byte +0x20
00007CFA  90                nop
00007CFB  C9                leave
00007CFC  C3                ret
00007CFD  55                push ebp
00007CFE  89E5              mov ebp,esp
00007D00  53                push ebx
00007D01  83EC64            sub esp,byte +0x64
00007D04  837D0C48          cmp dword [ebp+0xc],byte +0x48
00007D08  752B              jnz 0x7d35
00007D0A  8B4508            mov eax,[ebp+0x8]
00007D0D  8B80B40C0000      mov eax,[eax+0xcb4]
00007D13  85C0              test eax,eax
00007D15  0F8ECC030000      jng near 0x80e7
00007D1B  8B4508            mov eax,[ebp+0x8]
00007D1E  8B80B40C0000      mov eax,[eax+0xcb4]
00007D24  8D50FF            lea edx,[eax-0x1]
00007D27  8B4508            mov eax,[ebp+0x8]
00007D2A  8990B40C0000      mov [eax+0xcb4],edx
00007D30  E9B2030000        jmp 0x80e7
00007D35  837D0C50          cmp dword [ebp+0xc],byte +0x50
00007D39  7537              jnz 0x7d72
00007D3B  8B4508            mov eax,[ebp+0x8]
00007D3E  8B90B40C0000      mov edx,[eax+0xcb4]
00007D44  8B4508            mov eax,[ebp+0x8]
00007D47  8B80A40C0000      mov eax,[eax+0xca4]
00007D4D  83E801            sub eax,byte +0x1
00007D50  39C2              cmp edx,eax
00007D52  0F8D8F030000      jnl near 0x80e7
00007D58  8B4508            mov eax,[ebp+0x8]
00007D5B  8B80B40C0000      mov eax,[eax+0xcb4]
00007D61  8D5001            lea edx,[eax+0x1]
00007D64  8B4508            mov eax,[ebp+0x8]
00007D67  8990B40C0000      mov [eax+0xcb4],edx
00007D6D  E975030000        jmp 0x80e7
00007D72  837D0C4B          cmp dword [ebp+0xc],byte +0x4b
00007D76  752B              jnz 0x7da3
00007D78  8B4508            mov eax,[ebp+0x8]
00007D7B  8B80B80C0000      mov eax,[eax+0xcb8]
00007D81  85C0              test eax,eax
00007D83  0F8E5E030000      jng near 0x80e7
00007D89  8B4508            mov eax,[ebp+0x8]
00007D8C  8B80B80C0000      mov eax,[eax+0xcb8]
00007D92  8D50FF            lea edx,[eax-0x1]
00007D95  8B4508            mov eax,[ebp+0x8]
00007D98  8990B80C0000      mov [eax+0xcb8],edx
00007D9E  E944030000        jmp 0x80e7
00007DA3  837D0C4D          cmp dword [ebp+0xc],byte +0x4d
00007DA7  7554              jnz 0x7dfd
00007DA9  8B4508            mov eax,[ebp+0x8]
00007DAC  8B90A00C0000      mov edx,[eax+0xca0]
00007DB2  8B4508            mov eax,[ebp+0x8]
00007DB5  8B80B40C0000      mov eax,[eax+0xcb4]
00007DBB  C1E002            shl eax,byte 0x2
00007DBE  01D0              add eax,edx
00007DC0  8B00              mov eax,[eax]
00007DC2  83EC0C            sub esp,byte +0xc
00007DC5  50                push eax
00007DC6  E8CEDDFFFF        call 0x5b99
00007DCB  83C410            add esp,byte +0x10
00007DCE  8945B0            mov [ebp-0x50],eax
00007DD1  8B4508            mov eax,[ebp+0x8]
00007DD4  8B80B80C0000      mov eax,[eax+0xcb8]
00007DDA  3945B0            cmp [ebp-0x50],eax
00007DDD  0F8E04030000      jng near 0x80e7
00007DE3  8B4508            mov eax,[ebp+0x8]
00007DE6  8B80B80C0000      mov eax,[eax+0xcb8]
00007DEC  8D5001            lea edx,[eax+0x1]
00007DEF  8B4508            mov eax,[ebp+0x8]
00007DF2  8990B80C0000      mov [eax+0xcb8],edx
00007DF8  E9EA020000        jmp 0x80e7
00007DFD  837D0C0A          cmp dword [ebp+0xc],byte +0xa
00007E01  0F85F2000000      jnz near 0x7ef9
00007E07  8B4508            mov eax,[ebp+0x8]
00007E0A  8B90A40C0000      mov edx,[eax+0xca4]
00007E10  8B4508            mov eax,[ebp+0x8]
00007E13  8B80A80C0000      mov eax,[eax+0xca8]
00007E19  83E801            sub eax,byte +0x1
00007E1C  39C2              cmp edx,eax
00007E1E  0F8DC3020000      jnl near 0x80e7
00007E24  8B4508            mov eax,[ebp+0x8]
00007E27  8B80A40C0000      mov eax,[eax+0xca4]
00007E2D  8945F4            mov [ebp-0xc],eax
00007E30  EB31              jmp short 0x7e63
00007E32  8B4508            mov eax,[ebp+0x8]
00007E35  8B80A00C0000      mov eax,[eax+0xca0]
00007E3B  8B55F4            mov edx,[ebp-0xc]
00007E3E  81C2FFFFFF3F      add edx,0x3fffffff
00007E44  C1E202            shl edx,byte 0x2
00007E47  8D0C10            lea ecx,[eax+edx]
00007E4A  8B4508            mov eax,[ebp+0x8]
00007E4D  8B80A00C0000      mov eax,[eax+0xca0]
00007E53  8B55F4            mov edx,[ebp-0xc]
00007E56  C1E202            shl edx,byte 0x2
00007E59  01C2              add edx,eax
00007E5B  8B01              mov eax,[ecx]
00007E5D  8902              mov [edx],eax
00007E5F  836DF401          sub dword [ebp-0xc],byte +0x1
00007E63  8B4508            mov eax,[ebp+0x8]
00007E66  8B80B40C0000      mov eax,[eax+0xcb4]
00007E6C  83C001            add eax,byte +0x1
00007E6F  3945F4            cmp [ebp-0xc],eax
00007E72  7FBE              jg 0x7e32
00007E74  8B4508            mov eax,[ebp+0x8]
00007E77  8B90A00C0000      mov edx,[eax+0xca0]
00007E7D  8B4508            mov eax,[ebp+0x8]
00007E80  8B80B40C0000      mov eax,[eax+0xcb4]
00007E86  83C001            add eax,byte +0x1
00007E89  C1E002            shl eax,byte 0x2
00007E8C  8D1C02            lea ebx,[edx+eax]
00007E8F  83EC0C            sub esp,byte +0xc
00007E92  6A01              push byte +0x1
00007E94  E804E9FFFF        call 0x679d
00007E99  83C410            add esp,byte +0x10
00007E9C  8903              mov [ebx],eax
00007E9E  8B4508            mov eax,[ebp+0x8]
00007EA1  8B90A00C0000      mov edx,[eax+0xca0]
00007EA7  8B4508            mov eax,[ebp+0x8]
00007EAA  8B80B40C0000      mov eax,[eax+0xcb4]
00007EB0  83C001            add eax,byte +0x1
00007EB3  C1E002            shl eax,byte 0x2
00007EB6  01D0              add eax,edx
00007EB8  8B00              mov eax,[eax]
00007EBA  C60000            mov byte [eax],0x0
00007EBD  8B4508            mov eax,[ebp+0x8]
00007EC0  8B80A40C0000      mov eax,[eax+0xca4]
00007EC6  8D5001            lea edx,[eax+0x1]
00007EC9  8B4508            mov eax,[ebp+0x8]
00007ECC  8990A40C0000      mov [eax+0xca4],edx
00007ED2  8B4508            mov eax,[ebp+0x8]
00007ED5  8B80B40C0000      mov eax,[eax+0xcb4]
00007EDB  8D5001            lea edx,[eax+0x1]
00007EDE  8B4508            mov eax,[ebp+0x8]
00007EE1  8990B40C0000      mov [eax+0xcb4],edx
00007EE7  8B4508            mov eax,[ebp+0x8]
00007EEA  C780B80C00000000  mov dword [eax+0xcb8],0x0
         -0000
00007EF4  E9EE010000        jmp 0x80e7
00007EF9  837D0C08          cmp dword [ebp+0xc],byte +0x8
00007EFD  0F85EE000000      jnz near 0x7ff1
00007F03  8B4508            mov eax,[ebp+0x8]
00007F06  8B80B80C0000      mov eax,[eax+0xcb8]
00007F0C  85C0              test eax,eax
00007F0E  0F8E82000000      jng near 0x7f96
00007F14  8B4508            mov eax,[ebp+0x8]
00007F17  8B90A00C0000      mov edx,[eax+0xca0]
00007F1D  8B4508            mov eax,[ebp+0x8]
00007F20  8B80B40C0000      mov eax,[eax+0xcb4]
00007F26  C1E002            shl eax,byte 0x2
00007F29  01D0              add eax,edx
00007F2B  8B00              mov eax,[eax]
00007F2D  8945BC            mov [ebp-0x44],eax
00007F30  83EC0C            sub esp,byte +0xc
00007F33  FF75BC            push dword [ebp-0x44]
00007F36  E85EDCFFFF        call 0x5b99
00007F3B  83C410            add esp,byte +0x10
00007F3E  8945B8            mov [ebp-0x48],eax
00007F41  8B4508            mov eax,[ebp+0x8]
00007F44  8B80B80C0000      mov eax,[eax+0xcb8]
00007F4A  8945B4            mov [ebp-0x4c],eax
00007F4D  8B45B4            mov eax,[ebp-0x4c]
00007F50  83E801            sub eax,byte +0x1
00007F53  8945F0            mov [ebp-0x10],eax
00007F56  EB1C              jmp short 0x7f74
00007F58  8B45F0            mov eax,[ebp-0x10]
00007F5B  8D5001            lea edx,[eax+0x1]
00007F5E  8B45BC            mov eax,[ebp-0x44]
00007F61  01D0              add eax,edx
00007F63  8B4DF0            mov ecx,[ebp-0x10]
00007F66  8B55BC            mov edx,[ebp-0x44]
00007F69  01CA              add edx,ecx
00007F6B  0FB600            movzx eax,byte [eax]
00007F6E  8802              mov [edx],al
00007F70  8345F001          add dword [ebp-0x10],byte +0x1
00007F74  8B45F0            mov eax,[ebp-0x10]
00007F77  3B45B8            cmp eax,[ebp-0x48]
00007F7A  7CDC              jl 0x7f58
00007F7C  8B4508            mov eax,[ebp+0x8]
00007F7F  8B80B80C0000      mov eax,[eax+0xcb8]
00007F85  8D50FF            lea edx,[eax-0x1]
00007F88  8B4508            mov eax,[ebp+0x8]
00007F8B  8990B80C0000      mov [eax+0xcb8],edx
00007F91  E951010000        jmp 0x80e7
00007F96  8B4508            mov eax,[ebp+0x8]
00007F99  8B80B40C0000      mov eax,[eax+0xcb4]
00007F9F  85C0              test eax,eax
00007FA1  0F8E40010000      jng near 0x80e7
00007FA7  8B4508            mov eax,[ebp+0x8]
00007FAA  8B80B40C0000      mov eax,[eax+0xcb4]
00007FB0  8D50FF            lea edx,[eax-0x1]
00007FB3  8B4508            mov eax,[ebp+0x8]
00007FB6  8990B40C0000      mov [eax+0xcb4],edx
00007FBC  8B4508            mov eax,[ebp+0x8]
00007FBF  8B90A00C0000      mov edx,[eax+0xca0]
00007FC5  8B4508            mov eax,[ebp+0x8]
00007FC8  8B80B40C0000      mov eax,[eax+0xcb4]
00007FCE  C1E002            shl eax,byte 0x2
00007FD1  01D0              add eax,edx
00007FD3  8B00              mov eax,[eax]
00007FD5  83EC0C            sub esp,byte +0xc
00007FD8  50                push eax
00007FD9  E8BBDBFFFF        call 0x5b99
00007FDE  83C410            add esp,byte +0x10
00007FE1  89C2              mov edx,eax
00007FE3  8B4508            mov eax,[ebp+0x8]
00007FE6  8990B80C0000      mov [eax+0xcb8],edx
00007FEC  E9F6000000        jmp 0x80e7
00007FF1  837D0C1F          cmp dword [ebp+0xc],byte +0x1f
00007FF5  0F8EEC000000      jng near 0x80e7
00007FFB  837D0C7E          cmp dword [ebp+0xc],byte +0x7e
00007FFF  0F8FE2000000      jg near 0x80e7
00008005  8B4508            mov eax,[ebp+0x8]
00008008  8B90A00C0000      mov edx,[eax+0xca0]
0000800E  8B4508            mov eax,[ebp+0x8]
00008011  8B80B40C0000      mov eax,[eax+0xcb4]
00008017  C1E002            shl eax,byte 0x2
0000801A  01D0              add eax,edx
0000801C  8B00              mov eax,[eax]
0000801E  8945CC            mov [ebp-0x34],eax
00008021  83EC0C            sub esp,byte +0xc
00008024  FF75CC            push dword [ebp-0x34]
00008027  E86DDBFFFF        call 0x5b99
0000802C  83C410            add esp,byte +0x10
0000802F  8945C8            mov [ebp-0x38],eax
00008032  8B45C8            mov eax,[ebp-0x38]
00008035  83C002            add eax,byte +0x2
00008038  83EC0C            sub esp,byte +0xc
0000803B  50                push eax
0000803C  E85CE7FFFF        call 0x679d
00008041  83C410            add esp,byte +0x10
00008044  8945C4            mov [ebp-0x3c],eax
00008047  8B4508            mov eax,[ebp+0x8]
0000804A  8B80B80C0000      mov eax,[eax+0xcb8]
00008050  8945C0            mov [ebp-0x40],eax
00008053  C745EC00000000    mov dword [ebp-0x14],0x0
0000805A  EB19              jmp short 0x8075
0000805C  8B55EC            mov edx,[ebp-0x14]
0000805F  8B45CC            mov eax,[ebp-0x34]
00008062  01D0              add eax,edx
00008064  8B4DEC            mov ecx,[ebp-0x14]
00008067  8B55C4            mov edx,[ebp-0x3c]
0000806A  01CA              add edx,ecx
0000806C  0FB600            movzx eax,byte [eax]
0000806F  8802              mov [edx],al
00008071  8345EC01          add dword [ebp-0x14],byte +0x1
00008075  8B45EC            mov eax,[ebp-0x14]
00008078  3B45C0            cmp eax,[ebp-0x40]
0000807B  7CDF              jl 0x805c
0000807D  8B55C0            mov edx,[ebp-0x40]
00008080  8B45C4            mov eax,[ebp-0x3c]
00008083  01D0              add eax,edx
00008085  8B550C            mov edx,[ebp+0xc]
00008088  8810              mov [eax],dl
0000808A  8B45C0            mov eax,[ebp-0x40]
0000808D  8945E8            mov [ebp-0x18],eax
00008090  EB1C              jmp short 0x80ae
00008092  8B55E8            mov edx,[ebp-0x18]
00008095  8B45CC            mov eax,[ebp-0x34]
00008098  01D0              add eax,edx
0000809A  8B55E8            mov edx,[ebp-0x18]
0000809D  8D4A01            lea ecx,[edx+0x1]
000080A0  8B55C4            mov edx,[ebp-0x3c]
000080A3  01CA              add edx,ecx
000080A5  0FB600            movzx eax,byte [eax]
000080A8  8802              mov [edx],al
000080AA  8345E801          add dword [ebp-0x18],byte +0x1
000080AE  8B45E8            mov eax,[ebp-0x18]
000080B1  3B45C8            cmp eax,[ebp-0x38]
000080B4  7EDC              jng 0x8092
000080B6  8B4508            mov eax,[ebp+0x8]
000080B9  8B90A00C0000      mov edx,[eax+0xca0]
000080BF  8B4508            mov eax,[ebp+0x8]
000080C2  8B80B40C0000      mov eax,[eax+0xcb4]
000080C8  C1E002            shl eax,byte 0x2
000080CB  01C2              add edx,eax
000080CD  8B45C4            mov eax,[ebp-0x3c]
000080D0  8902              mov [edx],eax
000080D2  8B4508            mov eax,[ebp+0x8]
000080D5  8B80B80C0000      mov eax,[eax+0xcb8]
000080DB  8D5001            lea edx,[eax+0x1]
000080DE  8B4508            mov eax,[ebp+0x8]
000080E1  8990B80C0000      mov [eax+0xcb8],edx
000080E7  8B4508            mov eax,[ebp+0x8]
000080EA  8B4014            mov eax,[eax+0x14]
000080ED  83E823            sub eax,byte +0x23
000080F0  8D500F            lea edx,[eax+0xf]
000080F3  85C0              test eax,eax
000080F5  0F48C2            cmovs eax,edx
000080F8  C1F804            sar eax,byte 0x4
000080FB  8945AC            mov [ebp-0x54],eax
000080FE  8B4508            mov eax,[ebp+0x8]
00008101  8B90B40C0000      mov edx,[eax+0xcb4]
00008107  8B4508            mov eax,[ebp+0x8]
0000810A  8B80AC0C0000      mov eax,[eax+0xcac]
00008110  39C2              cmp edx,eax
00008112  7D12              jnl 0x8126
00008114  8B4508            mov eax,[ebp+0x8]
00008117  8B90B40C0000      mov edx,[eax+0xcb4]
0000811D  8B4508            mov eax,[ebp+0x8]
00008120  8990AC0C0000      mov [eax+0xcac],edx
00008126  8B4508            mov eax,[ebp+0x8]
00008129  8B90B40C0000      mov edx,[eax+0xcb4]
0000812F  8B4508            mov eax,[ebp+0x8]
00008132  8B88AC0C0000      mov ecx,[eax+0xcac]
00008138  8B45AC            mov eax,[ebp-0x54]
0000813B  01C8              add eax,ecx
0000813D  39C2              cmp edx,eax
0000813F  7C18              jl 0x8159
00008141  8B4508            mov eax,[ebp+0x8]
00008144  8B80B40C0000      mov eax,[eax+0xcb4]
0000814A  2B45AC            sub eax,[ebp-0x54]
0000814D  8D5001            lea edx,[eax+0x1]
00008150  8B4508            mov eax,[ebp+0x8]
00008153  8990AC0C0000      mov [eax+0xcac],edx
00008159  817D0C80000000    cmp dword [ebp+0xc],0x80
00008160  0F852A010000      jnz near 0x8290
00008166  8B4508            mov eax,[ebp+0x8]
00008169  83C018            add eax,byte +0x18
0000816C  8945E4            mov [ebp-0x1c],eax
0000816F  83EC0C            sub esp,byte +0xc
00008172  FF75E4            push dword [ebp-0x1c]
00008175  E81FDAFFFF        call 0x5b99
0000817A  83C410            add esp,byte +0x10
0000817D  83F809            cmp eax,byte +0x9
00008180  7E0E              jng 0x8190
00008182  8B45E4            mov eax,[ebp-0x1c]
00008185  0FB600            movzx eax,byte [eax]
00008188  3C45              cmp al,0x45
0000818A  7504              jnz 0x8190
0000818C  8345E409          add dword [ebp-0x1c],byte +0x9
00008190  C745E000000000    mov dword [ebp-0x20],0x0
00008197  C745DC00000000    mov dword [ebp-0x24],0x0
0000819E  EB29              jmp short 0x81c9
000081A0  8B4508            mov eax,[ebp+0x8]
000081A3  8B80A00C0000      mov eax,[eax+0xca0]
000081A9  8B55DC            mov edx,[ebp-0x24]
000081AC  C1E202            shl edx,byte 0x2
000081AF  01D0              add eax,edx
000081B1  8B00              mov eax,[eax]
000081B3  83EC0C            sub esp,byte +0xc
000081B6  50                push eax
000081B7  E8DDD9FFFF        call 0x5b99
000081BC  83C410            add esp,byte +0x10
000081BF  83C001            add eax,byte +0x1
000081C2  0145E0            add [ebp-0x20],eax
000081C5  8345DC01          add dword [ebp-0x24],byte +0x1
000081C9  8B4508            mov eax,[ebp+0x8]
000081CC  8B80A40C0000      mov eax,[eax+0xca4]
000081D2  3945DC            cmp [ebp-0x24],eax
000081D5  7CC9              jl 0x81a0
000081D7  8B45E0            mov eax,[ebp-0x20]
000081DA  83C001            add eax,byte +0x1
000081DD  83EC0C            sub esp,byte +0xc
000081E0  50                push eax
000081E1  E8B7E5FFFF        call 0x679d
000081E6  83C410            add esp,byte +0x10
000081E9  8945A8            mov [ebp-0x58],eax
000081EC  C745D800000000    mov dword [ebp-0x28],0x0
000081F3  C745D400000000    mov dword [ebp-0x2c],0x0
000081FA  EB71              jmp short 0x826d
000081FC  8B4508            mov eax,[ebp+0x8]
000081FF  8B80A00C0000      mov eax,[eax+0xca0]
00008205  8B55D4            mov edx,[ebp-0x2c]
00008208  C1E202            shl edx,byte 0x2
0000820B  01D0              add eax,edx
0000820D  8B00              mov eax,[eax]
0000820F  8945A4            mov [ebp-0x5c],eax
00008212  83EC0C            sub esp,byte +0xc
00008215  FF75A4            push dword [ebp-0x5c]
00008218  E87CD9FFFF        call 0x5b99
0000821D  83C410            add esp,byte +0x10
00008220  8945A0            mov [ebp-0x60],eax
00008223  C745D000000000    mov dword [ebp-0x30],0x0
0000822A  EB22              jmp short 0x824e
0000822C  8B55D0            mov edx,[ebp-0x30]
0000822F  8B45A4            mov eax,[ebp-0x5c]
00008232  8D0C02            lea ecx,[edx+eax]
00008235  8B45D8            mov eax,[ebp-0x28]
00008238  8D5001            lea edx,[eax+0x1]
0000823B  8955D8            mov [ebp-0x28],edx
0000823E  89C2              mov edx,eax
00008240  8B45A8            mov eax,[ebp-0x58]
00008243  01C2              add edx,eax
00008245  0FB601            movzx eax,byte [ecx]
00008248  8802              mov [edx],al
0000824A  8345D001          add dword [ebp-0x30],byte +0x1
0000824E  8B45D0            mov eax,[ebp-0x30]
00008251  3B45A0            cmp eax,[ebp-0x60]
00008254  7CD6              jl 0x822c
00008256  8B45D8            mov eax,[ebp-0x28]
00008259  8D5001            lea edx,[eax+0x1]
0000825C  8955D8            mov [ebp-0x28],edx
0000825F  89C2              mov edx,eax
00008261  8B45A8            mov eax,[ebp-0x58]
00008264  01D0              add eax,edx
00008266  C6000A            mov byte [eax],0xa
00008269  8345D401          add dword [ebp-0x2c],byte +0x1
0000826D  8B4508            mov eax,[ebp+0x8]
00008270  8B80A40C0000      mov eax,[eax+0xca4]
00008276  3945D4            cmp [ebp-0x2c],eax
00008279  7C81              jl 0x81fc
0000827B  8B45D8            mov eax,[ebp-0x28]
0000827E  83EC04            sub esp,byte +0x4
00008281  50                push eax
00008282  FF75A8            push dword [ebp-0x58]
00008285  FF75E4            push dword [ebp-0x1c]
00008288  E83A96FFFF        call 0x18c7
0000828D  83C410            add esp,byte +0x10
00008290  90                nop
00008291  8B5DFC            mov ebx,[ebp-0x4]
00008294  C9                leave
00008295  C3                ret
00008296  55                push ebp
00008297  89E5              mov ebp,esp
00008299  53                push ebx
0000829A  83EC34            sub esp,byte +0x34
0000829D  83EC0C            sub esp,byte +0xc
000082A0  FF7508            push dword [ebp+0x8]
000082A3  E8BDF4FFFF        call 0x7765
000082A8  83C410            add esp,byte +0x10
000082AB  83EC0C            sub esp,byte +0xc
000082AE  68007D0000        push dword 0x7d00
000082B3  E8E5E4FFFF        call 0x679d
000082B8  83C410            add esp,byte +0x10
000082BB  8945E4            mov [ebp-0x1c],eax
000082BE  837DE400          cmp dword [ebp-0x1c],byte +0x0
000082C2  0F84C0010000      jz near 0x8488
000082C8  83EC08            sub esp,byte +0x8
000082CB  FF75E4            push dword [ebp-0x1c]
000082CE  FF750C            push dword [ebp+0xc]
000082D1  E8E491FFFF        call 0x14ba
000082D6  83C410            add esp,byte +0x10
000082D9  85C0              test eax,eax
000082DB  0F84A8010000      jz near 0x8489
000082E1  8B4508            mov eax,[ebp+0x8]
000082E4  C780A40C00000000  mov dword [eax+0xca4],0x0
         -0000
000082EE  C745F400000000    mov dword [ebp-0xc],0x0
000082F5  C745F000000000    mov dword [ebp-0x10],0x0
000082FC  E9A3000000        jmp 0x83a4
00008301  8B55F4            mov edx,[ebp-0xc]
00008304  8B45E4            mov eax,[ebp-0x1c]
00008307  01D0              add eax,edx
00008309  0FB600            movzx eax,byte [eax]
0000830C  3C0A              cmp al,0xa
0000830E  0F858C000000      jnz near 0x83a0
00008314  8B45F4            mov eax,[ebp-0xc]
00008317  2B45F0            sub eax,[ebp-0x10]
0000831A  8945D8            mov [ebp-0x28],eax
0000831D  8B45D8            mov eax,[ebp-0x28]
00008320  83C001            add eax,byte +0x1
00008323  83EC0C            sub esp,byte +0xc
00008326  50                push eax
00008327  E871E4FFFF        call 0x679d
0000832C  83C410            add esp,byte +0x10
0000832F  8945D4            mov [ebp-0x2c],eax
00008332  C745EC00000000    mov dword [ebp-0x14],0x0
00008339  EB20              jmp short 0x835b
0000833B  8B55F0            mov edx,[ebp-0x10]
0000833E  8B45EC            mov eax,[ebp-0x14]
00008341  01D0              add eax,edx
00008343  89C2              mov edx,eax
00008345  8B45E4            mov eax,[ebp-0x1c]
00008348  01D0              add eax,edx
0000834A  8B4DEC            mov ecx,[ebp-0x14]
0000834D  8B55D4            mov edx,[ebp-0x2c]
00008350  01CA              add edx,ecx
00008352  0FB600            movzx eax,byte [eax]
00008355  8802              mov [edx],al
00008357  8345EC01          add dword [ebp-0x14],byte +0x1
0000835B  8B45EC            mov eax,[ebp-0x14]
0000835E  3B45D8            cmp eax,[ebp-0x28]
00008361  7CD8              jl 0x833b
00008363  8B55D8            mov edx,[ebp-0x28]
00008366  8B45D4            mov eax,[ebp-0x2c]
00008369  01D0              add eax,edx
0000836B  C60000            mov byte [eax],0x0
0000836E  8B4508            mov eax,[ebp+0x8]
00008371  8B98A00C0000      mov ebx,[eax+0xca0]
00008377  8B4508            mov eax,[ebp+0x8]
0000837A  8B80A40C0000      mov eax,[eax+0xca4]
00008380  8D4801            lea ecx,[eax+0x1]
00008383  8B5508            mov edx,[ebp+0x8]
00008386  898AA40C0000      mov [edx+0xca4],ecx
0000838C  C1E002            shl eax,byte 0x2
0000838F  8D1403            lea edx,[ebx+eax]
00008392  8B45D4            mov eax,[ebp-0x2c]
00008395  8902              mov [edx],eax
00008397  8B45F4            mov eax,[ebp-0xc]
0000839A  83C001            add eax,byte +0x1
0000839D  8945F0            mov [ebp-0x10],eax
000083A0  8345F401          add dword [ebp-0xc],byte +0x1
000083A4  8B55F4            mov edx,[ebp-0xc]
000083A7  8B45E4            mov eax,[ebp-0x1c]
000083AA  01D0              add eax,edx
000083AC  0FB600            movzx eax,byte [eax]
000083AF  84C0              test al,al
000083B1  0F854AFFFFFF      jnz near 0x8301
000083B7  8B45F4            mov eax,[ebp-0xc]
000083BA  3B45F0            cmp eax,[ebp-0x10]
000083BD  0F8E83000000      jng near 0x8446
000083C3  8B45F4            mov eax,[ebp-0xc]
000083C6  2B45F0            sub eax,[ebp-0x10]
000083C9  8945E0            mov [ebp-0x20],eax
000083CC  8B45E0            mov eax,[ebp-0x20]
000083CF  83C001            add eax,byte +0x1
000083D2  83EC0C            sub esp,byte +0xc
000083D5  50                push eax
000083D6  E8C2E3FFFF        call 0x679d
000083DB  83C410            add esp,byte +0x10
000083DE  8945DC            mov [ebp-0x24],eax
000083E1  C745E800000000    mov dword [ebp-0x18],0x0
000083E8  EB20              jmp short 0x840a
000083EA  8B55F0            mov edx,[ebp-0x10]
000083ED  8B45E8            mov eax,[ebp-0x18]
000083F0  01D0              add eax,edx
000083F2  89C2              mov edx,eax
000083F4  8B45E4            mov eax,[ebp-0x1c]
000083F7  01D0              add eax,edx
000083F9  8B4DE8            mov ecx,[ebp-0x18]
000083FC  8B55DC            mov edx,[ebp-0x24]
000083FF  01CA              add edx,ecx
00008401  0FB600            movzx eax,byte [eax]
00008404  8802              mov [edx],al
00008406  8345E801          add dword [ebp-0x18],byte +0x1
0000840A  8B45E8            mov eax,[ebp-0x18]
0000840D  3B45E0            cmp eax,[ebp-0x20]
00008410  7CD8              jl 0x83ea
00008412  8B55E0            mov edx,[ebp-0x20]
00008415  8B45DC            mov eax,[ebp-0x24]
00008418  01D0              add eax,edx
0000841A  C60000            mov byte [eax],0x0
0000841D  8B4508            mov eax,[ebp+0x8]
00008420  8B98A00C0000      mov ebx,[eax+0xca0]
00008426  8B4508            mov eax,[ebp+0x8]
00008429  8B80A40C0000      mov eax,[eax+0xca4]
0000842F  8D4801            lea ecx,[eax+0x1]
00008432  8B5508            mov edx,[ebp+0x8]
00008435  898AA40C0000      mov [edx+0xca4],ecx
0000843B  C1E002            shl eax,byte 0x2
0000843E  8D1403            lea edx,[ebx+eax]
00008441  8B45DC            mov eax,[ebp-0x24]
00008444  8902              mov [edx],eax
00008446  8B4508            mov eax,[ebp+0x8]
00008449  8B80A40C0000      mov eax,[eax+0xca4]
0000844F  85C0              test eax,eax
00008451  7536              jnz 0x8489
00008453  8B4508            mov eax,[ebp+0x8]
00008456  8B98A00C0000      mov ebx,[eax+0xca0]
0000845C  83EC0C            sub esp,byte +0xc
0000845F  6A01              push byte +0x1
00008461  E837E3FFFF        call 0x679d
00008466  83C410            add esp,byte +0x10
00008469  8903              mov [ebx],eax
0000846B  8B4508            mov eax,[ebp+0x8]
0000846E  8B80A00C0000      mov eax,[eax+0xca0]
00008474  8B00              mov eax,[eax]
00008476  C60000            mov byte [eax],0x0
00008479  8B4508            mov eax,[ebp+0x8]
0000847C  C780A40C00000100  mov dword [eax+0xca4],0x1
         -0000
00008486  EB01              jmp short 0x8489
00008488  90                nop
00008489  8B5DFC            mov ebx,[ebp-0x4]
0000848C  C9                leave
0000848D  C3                ret
0000848E  55                push ebp
0000848F  89E5              mov ebp,esp
00008491  83EC18            sub esp,byte +0x18
00008494  C745F400000000    mov dword [ebp-0xc],0x0
0000849B  EB5D              jmp short 0x84fa
0000849D  C745F000000000    mov dword [ebp-0x10],0x0
000084A4  EB4A              jmp short 0x84f0
000084A6  C745ECFFFFFF00    mov dword [ebp-0x14],0xffffff
000084AD  837DF000          cmp dword [ebp-0x10],byte +0x0
000084B1  7412              jz 0x84c5
000084B3  837DF009          cmp dword [ebp-0x10],byte +0x9
000084B7  740C              jz 0x84c5
000084B9  837DF400          cmp dword [ebp-0xc],byte +0x0
000084BD  7406              jz 0x84c5
000084BF  837DF409          cmp dword [ebp-0xc],byte +0x9
000084C3  7507              jnz 0x84cc
000084C5  C745EC00000000    mov dword [ebp-0x14],0x0
000084CC  8B550C            mov edx,[ebp+0xc]
000084CF  8B45F4            mov eax,[ebp-0xc]
000084D2  01C2              add edx,eax
000084D4  8B4D08            mov ecx,[ebp+0x8]
000084D7  8B45F0            mov eax,[ebp-0x10]
000084DA  01C8              add eax,ecx
000084DC  83EC04            sub esp,byte +0x4
000084DF  FF75EC            push dword [ebp-0x14]
000084E2  52                push edx
000084E3  50                push eax
000084E4  E840B3FFFF        call 0x3829
000084E9  83C410            add esp,byte +0x10
000084EC  8345F001          add dword [ebp-0x10],byte +0x1
000084F0  837DF009          cmp dword [ebp-0x10],byte +0x9
000084F4  7EB0              jng 0x84a6
000084F6  8345F401          add dword [ebp-0xc],byte +0x1
000084FA  837DF409          cmp dword [ebp-0xc],byte +0x9
000084FE  7E9D              jng 0x849d
00008500  90                nop
00008501  C9                leave
00008502  C3                ret
00008503  55                push ebp
00008504  89E5              mov ebp,esp
00008506  83EC10            sub esp,byte +0x10
00008509  C745FC00000000    mov dword [ebp-0x4],0x0
00008510  EB04              jmp short 0x8516
00008512  8345FC01          add dword [ebp-0x4],byte +0x1
00008516  8B55FC            mov edx,[ebp-0x4]
00008519  8B4508            mov eax,[ebp+0x8]
0000851C  01D0              add eax,edx
0000851E  0FB600            movzx eax,byte [eax]
00008521  84C0              test al,al
00008523  75ED              jnz 0x8512
00008525  8B45FC            mov eax,[ebp-0x4]
00008528  C9                leave
00008529  C3                ret
0000852A  55                push ebp
0000852B  89E5              mov ebp,esp
0000852D  EB08              jmp short 0x8537
0000852F  83450801          add dword [ebp+0x8],byte +0x1
00008533  83450C01          add dword [ebp+0xc],byte +0x1
00008537  8B4508            mov eax,[ebp+0x8]
0000853A  0FB600            movzx eax,byte [eax]
0000853D  84C0              test al,al
0000853F  7410              jz 0x8551
00008541  8B4508            mov eax,[ebp+0x8]
00008544  0FB610            movzx edx,byte [eax]
00008547  8B450C            mov eax,[ebp+0xc]
0000854A  0FB600            movzx eax,byte [eax]
0000854D  38C2              cmp dl,al
0000854F  74DE              jz 0x852f
00008551  8B4508            mov eax,[ebp+0x8]
00008554  0FB600            movzx eax,byte [eax]
00008557  0FB6D0            movzx edx,al
0000855A  8B450C            mov eax,[ebp+0xc]
0000855D  0FB600            movzx eax,byte [eax]
00008560  0FB6C0            movzx eax,al
00008563  29C2              sub edx,eax
00008565  89D0              mov eax,edx
00008567  5D                pop ebp
00008568  C3                ret
00008569  55                push ebp
0000856A  89E5              mov ebp,esp
0000856C  83EC10            sub esp,byte +0x10
0000856F  C745FC00000000    mov dword [ebp-0x4],0x0
00008576  EB40              jmp short 0x85b8
00008578  8B45FC            mov eax,[ebp-0x4]
0000857B  69C0E80D0000      imul eax,eax,dword 0xde8
00008581  0584370200        add eax,0x23784
00008586  C70000000000      mov dword [eax],0x0
0000858C  8B45FC            mov eax,[ebp-0x4]
0000858F  69C0E80D0000      imul eax,eax,dword 0xde8
00008595  05C4370200        add eax,0x237c4
0000859A  C70000000000      mov dword [eax],0x0
000085A0  8B45FC            mov eax,[ebp-0x4]
000085A3  69C0E80D0000      imul eax,eax,dword 0xde8
000085A9  8D9080370200      lea edx,[eax+0x23780]
000085AF  8B45FC            mov eax,[ebp-0x4]
000085B2  8902              mov [edx],eax
000085B4  8345FC01          add dword [ebp-0x4],byte +0x1
000085B8  837DFC09          cmp dword [ebp-0x4],byte +0x9
000085BC  7EBA              jng 0x8578
000085BE  C705602802000000  mov dword [dword 0x22860],0x0
         -0000
000085C8  90                nop
000085C9  C9                leave
000085CA  C3                ret
000085CB  55                push ebp
000085CC  89E5              mov ebp,esp
000085CE  83EC28            sub esp,byte +0x28
000085D1  C745F400000000    mov dword [ebp-0xc],0x0
000085D8  E911020000        jmp 0x87ee
000085DD  8B45F4            mov eax,[ebp-0xc]
000085E0  69C0E80D0000      imul eax,eax,dword 0xde8
000085E6  0584370200        add eax,0x23784
000085EB  8B00              mov eax,[eax]
000085ED  85C0              test eax,eax
000085EF  0F85F5010000      jnz near 0x87ea
000085F5  8B45F4            mov eax,[ebp-0xc]
000085F8  69C0E80D0000      imul eax,eax,dword 0xde8
000085FE  0580370200        add eax,0x23780
00008603  8945E8            mov [ebp-0x18],eax
00008606  8B45E8            mov eax,[ebp-0x18]
00008609  C7400401000000    mov dword [eax+0x4],0x1
00008610  8B45E8            mov eax,[ebp-0x18]
00008613  C7404401000000    mov dword [eax+0x44],0x1
0000861A  8B45E8            mov eax,[ebp-0x18]
0000861D  8B551C            mov edx,[ebp+0x1c]
00008620  895048            mov [eax+0x48],edx
00008623  8B45E8            mov eax,[ebp-0x18]
00008626  8B5508            mov edx,[ebp+0x8]
00008629  895008            mov [eax+0x8],edx
0000862C  8B45E8            mov eax,[ebp-0x18]
0000862F  8B550C            mov edx,[ebp+0xc]
00008632  89500C            mov [eax+0xc],edx
00008635  8B45E8            mov eax,[ebp-0x18]
00008638  8B5510            mov edx,[ebp+0x10]
0000863B  895010            mov [eax+0x10],edx
0000863E  8B45E8            mov eax,[ebp-0x18]
00008641  8B5514            mov edx,[ebp+0x14]
00008644  895014            mov [eax+0x14],edx
00008647  8B45E8            mov eax,[ebp-0x18]
0000864A  C7403800000000    mov dword [eax+0x38],0x0
00008651  C745F000000000    mov dword [ebp-0x10],0x0
00008658  EB1C              jmp short 0x8676
0000865A  8B55F0            mov edx,[ebp-0x10]
0000865D  8B4518            mov eax,[ebp+0x18]
00008660  01D0              add eax,edx
00008662  0FB600            movzx eax,byte [eax]
00008665  8B4DE8            mov ecx,[ebp-0x18]
00008668  8B55F0            mov edx,[ebp-0x10]
0000866B  01CA              add edx,ecx
0000866D  83C218            add edx,byte +0x18
00008670  8802              mov [edx],al
00008672  8345F001          add dword [ebp-0x10],byte +0x1
00008676  8B55F0            mov edx,[ebp-0x10]
00008679  8B4518            mov eax,[ebp+0x18]
0000867C  01D0              add eax,edx
0000867E  0FB600            movzx eax,byte [eax]
00008681  84C0              test al,al
00008683  7406              jz 0x868b
00008685  837DF01E          cmp dword [ebp-0x10],byte +0x1e
00008689  7ECF              jng 0x865a
0000868B  8B55E8            mov edx,[ebp-0x18]
0000868E  8B45F0            mov eax,[ebp-0x10]
00008691  01D0              add eax,edx
00008693  83C018            add eax,byte +0x18
00008696  C60000            mov byte [eax],0x0
00008699  837D1C01          cmp dword [ebp+0x1c],byte +0x1
0000869D  0F8586000000      jnz near 0x8729
000086A3  8B45E8            mov eax,[ebp-0x18]
000086A6  C6404C00          mov byte [eax+0x4c],0x0
000086AA  8B45E8            mov eax,[ebp-0x18]
000086AD  C7804C0900000000  mov dword [eax+0x94c],0x0
         -0000
000086B7  8B45E8            mov eax,[ebp-0x18]
000086BA  C6804C08000000    mov byte [eax+0x84c],0x0
000086C1  8B45E8            mov eax,[ebp-0x18]
000086C4  C780500B00000000  mov dword [eax+0xb50],0x0
         -0000
000086CE  8B45E8            mov eax,[ebp-0x18]
000086D1  C780540B00000000  mov dword [eax+0xb54],0x0
         -0000
000086DB  C745E490C20100    mov dword [ebp-0x1c],0x1c290
000086E2  C745EC00000000    mov dword [ebp-0x14],0x0
000086E9  EB1C              jmp short 0x8707
000086EB  8B55EC            mov edx,[ebp-0x14]
000086EE  8B45E4            mov eax,[ebp-0x1c]
000086F1  01D0              add eax,edx
000086F3  0FB600            movzx eax,byte [eax]
000086F6  8B4DE8            mov ecx,[ebp-0x18]
000086F9  8B55EC            mov edx,[ebp-0x14]
000086FC  01CA              add edx,ecx
000086FE  83C24C            add edx,byte +0x4c
00008701  8802              mov [edx],al
00008703  8345EC01          add dword [ebp-0x14],byte +0x1
00008707  8B55EC            mov edx,[ebp-0x14]
0000870A  8B45E4            mov eax,[ebp-0x1c]
0000870D  01D0              add eax,edx
0000870F  0FB600            movzx eax,byte [eax]
00008712  84C0              test al,al
00008714  75D5              jnz 0x86eb
00008716  8B55E8            mov edx,[ebp-0x18]
00008719  8B45EC            mov eax,[ebp-0x14]
0000871C  01D0              add eax,edx
0000871E  83C04C            add eax,byte +0x4c
00008721  C60000            mov byte [eax],0x0
00008724  E9BC000000        jmp 0x87e5
00008729  837D1C02          cmp dword [ebp+0x1c],byte +0x2
0000872D  7533              jnz 0x8762
0000872F  8B45E8            mov eax,[ebp-0x18]
00008732  05580B0000        add eax,0xb58
00008737  83EC08            sub esp,byte +0x8
0000873A  6A10              push byte +0x10
0000873C  50                push eax
0000873D  E8168AFFFF        call 0x1158
00008742  83C410            add esp,byte +0x10
00008745  89C2              mov edx,eax
00008747  8B45E8            mov eax,[ebp-0x18]
0000874A  8990980C0000      mov [eax+0xc98],edx
00008750  8B45E8            mov eax,[ebp-0x18]
00008753  C7809C0C0000FFFF  mov dword [eax+0xc9c],0xffffffff
         -FFFF
0000875D  E983000000        jmp 0x87e5
00008762  837D1C03          cmp dword [ebp+0x1c],byte +0x3
00008766  7510              jnz 0x8778
00008768  83EC0C            sub esp,byte +0xc
0000876B  FF75E8            push dword [ebp-0x18]
0000876E  E8F2EFFFFF        call 0x7765
00008773  83C410            add esp,byte +0x10
00008776  EB6D              jmp short 0x87e5
00008778  837D1C04          cmp dword [ebp+0x1c],byte +0x4
0000877C  753D              jnz 0x87bb
0000877E  8B45E8            mov eax,[ebp-0x18]
00008781  C680C00C000030    mov byte [eax+0xcc0],0x30
00008788  8B45E8            mov eax,[ebp-0x18]
0000878B  C680C10C000000    mov byte [eax+0xcc1],0x0
00008792  8B45E8            mov eax,[ebp-0x18]
00008795  C780D00C00000000  mov dword [eax+0xcd0],0x0
         -0000
0000879F  8B45E8            mov eax,[ebp-0x18]
000087A2  C780D40C00000000  mov dword [eax+0xcd4],0x0
         -0000
000087AC  8B45E8            mov eax,[ebp-0x18]
000087AF  C780D80C00000100  mov dword [eax+0xcd8],0x1
         -0000
000087B9  EB2A              jmp short 0x87e5
000087BB  837D1C05          cmp dword [ebp+0x1c],byte +0x5
000087BF  7510              jnz 0x87d1
000087C1  83EC0C            sub esp,byte +0xc
000087C4  FF75E8            push dword [ebp-0x18]
000087C7  E8C7E2FFFF        call 0x6a93
000087CC  83C410            add esp,byte +0x10
000087CF  EB14              jmp short 0x87e5
000087D1  837D1C06          cmp dword [ebp+0x1c],byte +0x6
000087D5  750E              jnz 0x87e5
000087D7  83EC0C            sub esp,byte +0xc
000087DA  FF75E8            push dword [ebp-0x18]
000087DD  E8FDE8FFFF        call 0x70df
000087E2  83C410            add esp,byte +0x10
000087E5  8B45E8            mov eax,[ebp-0x18]
000087E8  EB13              jmp short 0x87fd
000087EA  8345F401          add dword [ebp-0xc],byte +0x1
000087EE  837DF409          cmp dword [ebp-0xc],byte +0x9
000087F2  0F8EE5FDFFFF      jng near 0x85dd
000087F8  B800000000        mov eax,0x0
000087FD  C9                leave
000087FE  C3                ret
000087FF  55                push ebp
00008800  89E5              mov ebp,esp
00008802  83EC28            sub esp,byte +0x28
00008805  C745F400000000    mov dword [ebp-0xc],0x0
0000880C  E9FC010000        jmp 0x8a0d
00008811  C745F000000000    mov dword [ebp-0x10],0x0
00008818  E9E2010000        jmp 0x89ff
0000881D  8B45F4            mov eax,[ebp-0xc]
00008820  C1E004            shl eax,byte 0x4
00008823  89C2              mov edx,eax
00008825  8B45F0            mov eax,[ebp-0x10]
00008828  01D0              add eax,edx
0000882A  89C2              mov edx,eax
0000882C  8B4510            mov eax,[ebp+0x10]
0000882F  01D0              add eax,edx
00008831  0FB600            movzx eax,byte [eax]
00008834  8845EB            mov [ebp-0x15],al
00008837  807DEB00          cmp byte [ebp-0x15],0x0
0000883B  0F85C8000000      jnz near 0x8909
00008841  837D1800          cmp dword [ebp+0x18],byte +0x0
00008845  0F85B0010000      jnz near 0x89fb
0000884B  837D1400          cmp dword [ebp+0x14],byte +0x0
0000884F  7407              jz 0x8858
00008851  B880000000        mov eax,0x80
00008856  EB05              jmp short 0x885d
00008858  B880800000        mov eax,0x8080
0000885D  8945E4            mov [ebp-0x1c],eax
00008860  8B45F4            mov eax,[ebp-0xc]
00008863  8D1400            lea edx,[eax+eax]
00008866  8B450C            mov eax,[ebp+0xc]
00008869  01C2              add edx,eax
0000886B  8B45F0            mov eax,[ebp-0x10]
0000886E  8D0C00            lea ecx,[eax+eax]
00008871  8B4508            mov eax,[ebp+0x8]
00008874  01C8              add eax,ecx
00008876  83EC04            sub esp,byte +0x4
00008879  FF75E4            push dword [ebp-0x1c]
0000887C  52                push edx
0000887D  50                push eax
0000887E  E80CACFFFF        call 0x348f
00008883  83C410            add esp,byte +0x10
00008886  8B45F4            mov eax,[ebp-0xc]
00008889  8D1400            lea edx,[eax+eax]
0000888C  8B450C            mov eax,[ebp+0xc]
0000888F  01C2              add edx,eax
00008891  8B45F0            mov eax,[ebp-0x10]
00008894  8D0C00            lea ecx,[eax+eax]
00008897  8B4508            mov eax,[ebp+0x8]
0000889A  01C8              add eax,ecx
0000889C  83C001            add eax,byte +0x1
0000889F  83EC04            sub esp,byte +0x4
000088A2  FF75E4            push dword [ebp-0x1c]
000088A5  52                push edx
000088A6  50                push eax
000088A7  E8E3ABFFFF        call 0x348f
000088AC  83C410            add esp,byte +0x10
000088AF  8B45F4            mov eax,[ebp-0xc]
000088B2  8D1400            lea edx,[eax+eax]
000088B5  8B450C            mov eax,[ebp+0xc]
000088B8  01D0              add eax,edx
000088BA  8D5001            lea edx,[eax+0x1]
000088BD  8B45F0            mov eax,[ebp-0x10]
000088C0  8D0C00            lea ecx,[eax+eax]
000088C3  8B4508            mov eax,[ebp+0x8]
000088C6  01C8              add eax,ecx
000088C8  83EC04            sub esp,byte +0x4
000088CB  FF75E4            push dword [ebp-0x1c]
000088CE  52                push edx
000088CF  50                push eax
000088D0  E8BAABFFFF        call 0x348f
000088D5  83C410            add esp,byte +0x10
000088D8  8B45F4            mov eax,[ebp-0xc]
000088DB  8D1400            lea edx,[eax+eax]
000088DE  8B450C            mov eax,[ebp+0xc]
000088E1  01D0              add eax,edx
000088E3  8D5001            lea edx,[eax+0x1]
000088E6  8B45F0            mov eax,[ebp-0x10]
000088E9  8D0C00            lea ecx,[eax+eax]
000088EC  8B4508            mov eax,[ebp+0x8]
000088EF  01C8              add eax,ecx
000088F1  83C001            add eax,byte +0x1
000088F4  83EC04            sub esp,byte +0x4
000088F7  FF75E4            push dword [ebp-0x1c]
000088FA  52                push edx
000088FB  50                push eax
000088FC  E88EABFFFF        call 0x348f
00008901  83C410            add esp,byte +0x10
00008904  E9F2000000        jmp 0x89fb
00008909  C745ECFFFFFF00    mov dword [ebp-0x14],0xffffff
00008910  0FB645EB          movzx eax,byte [ebp-0x15]
00008914  83F806            cmp eax,byte +0x6
00008917  7736              ja 0x894f
00008919  8B0485E0C20100    mov eax,[eax*4+0x1c2e0]
00008920  FFE0              jmp eax
00008922  C745EC00000000    mov dword [ebp-0x14],0x0
00008929  EB2C              jmp short 0x8957
0000892B  C745ECFFFFFF00    mov dword [ebp-0x14],0xffffff
00008932  EB23              jmp short 0x8957
00008934  C745ECAA000000    mov dword [ebp-0x14],0xaa
0000893B  EB1A              jmp short 0x8957
0000893D  C745ECC0C0C000    mov dword [ebp-0x14],0xc0c0c0
00008944  EB11              jmp short 0x8957
00008946  C745EC55FFFF00    mov dword [ebp-0x14],0xffff55
0000894D  EB08              jmp short 0x8957
0000894F  C745ECFFFFFF00    mov dword [ebp-0x14],0xffffff
00008956  90                nop
00008957  8B45F4            mov eax,[ebp-0xc]
0000895A  8D1400            lea edx,[eax+eax]
0000895D  8B450C            mov eax,[ebp+0xc]
00008960  01C2              add edx,eax
00008962  8B45F0            mov eax,[ebp-0x10]
00008965  8D0C00            lea ecx,[eax+eax]
00008968  8B4508            mov eax,[ebp+0x8]
0000896B  01C8              add eax,ecx
0000896D  83EC04            sub esp,byte +0x4
00008970  FF75EC            push dword [ebp-0x14]
00008973  52                push edx
00008974  50                push eax
00008975  E815ABFFFF        call 0x348f
0000897A  83C410            add esp,byte +0x10
0000897D  8B45F4            mov eax,[ebp-0xc]
00008980  8D1400            lea edx,[eax+eax]
00008983  8B450C            mov eax,[ebp+0xc]
00008986  01C2              add edx,eax
00008988  8B45F0            mov eax,[ebp-0x10]
0000898B  8D0C00            lea ecx,[eax+eax]
0000898E  8B4508            mov eax,[ebp+0x8]
00008991  01C8              add eax,ecx
00008993  83C001            add eax,byte +0x1
00008996  83EC04            sub esp,byte +0x4
00008999  FF75EC            push dword [ebp-0x14]
0000899C  52                push edx
0000899D  50                push eax
0000899E  E8ECAAFFFF        call 0x348f
000089A3  83C410            add esp,byte +0x10
000089A6  8B45F4            mov eax,[ebp-0xc]
000089A9  8D1400            lea edx,[eax+eax]
000089AC  8B450C            mov eax,[ebp+0xc]
000089AF  01D0              add eax,edx
000089B1  8D5001            lea edx,[eax+0x1]
000089B4  8B45F0            mov eax,[ebp-0x10]
000089B7  8D0C00            lea ecx,[eax+eax]
000089BA  8B4508            mov eax,[ebp+0x8]
000089BD  01C8              add eax,ecx
000089BF  83EC04            sub esp,byte +0x4
000089C2  FF75EC            push dword [ebp-0x14]
000089C5  52                push edx
000089C6  50                push eax
000089C7  E8C3AAFFFF        call 0x348f
000089CC  83C410            add esp,byte +0x10
000089CF  8B45F4            mov eax,[ebp-0xc]
000089D2  8D1400            lea edx,[eax+eax]
000089D5  8B450C            mov eax,[ebp+0xc]
000089D8  01D0              add eax,edx
000089DA  8D5001            lea edx,[eax+0x1]
000089DD  8B45F0            mov eax,[ebp-0x10]
000089E0  8D0C00            lea ecx,[eax+eax]
000089E3  8B4508            mov eax,[ebp+0x8]
000089E6  01C8              add eax,ecx
000089E8  83C001            add eax,byte +0x1
000089EB  83EC04            sub esp,byte +0x4
000089EE  FF75EC            push dword [ebp-0x14]
000089F1  52                push edx
000089F2  50                push eax
000089F3  E897AAFFFF        call 0x348f
000089F8  83C410            add esp,byte +0x10
000089FB  8345F001          add dword [ebp-0x10],byte +0x1
000089FF  837DF00F          cmp dword [ebp-0x10],byte +0xf
00008A03  0F8E14FEFFFF      jng near 0x881d
00008A09  8345F401          add dword [ebp-0xc],byte +0x1
00008A0D  837DF40F          cmp dword [ebp-0xc],byte +0xf
00008A11  0F8EFAFDFFFF      jng near 0x8811
00008A17  90                nop
00008A18  C9                leave
00008A19  C3                ret
00008A1A  55                push ebp
00008A1B  89E5              mov ebp,esp
00008A1D  83EC18            sub esp,byte +0x18
00008A20  837D1400          cmp dword [ebp+0x14],byte +0x0
00008A24  7479              jz 0x8a9f
00008A26  8B450C            mov eax,[ebp+0xc]
00008A29  8945F4            mov [ebp-0xc],eax
00008A2C  EB65              jmp short 0x8a93
00008A2E  8B4514            mov eax,[ebp+0x14]
00008A31  0FB600            movzx eax,byte [eax]
00008A34  8845F3            mov [ebp-0xd],al
00008A37  807DF30A          cmp byte [ebp-0xd],0xa
00008A3B  750C              jnz 0x8a49
00008A3D  8B45F4            mov eax,[ebp-0xc]
00008A40  89450C            mov [ebp+0xc],eax
00008A43  83451010          add dword [ebp+0x10],byte +0x10
00008A47  EB46              jmp short 0x8a8f
00008A49  8B4508            mov eax,[ebp+0x8]
00008A4C  8B5008            mov edx,[eax+0x8]
00008A4F  8B4508            mov eax,[ebp+0x8]
00008A52  8B4010            mov eax,[eax+0x10]
00008A55  01D0              add eax,edx
00008A57  83E80A            sub eax,byte +0xa
00008A5A  39450C            cmp [ebp+0xc],eax
00008A5D  7D2C              jnl 0x8a8b
00008A5F  8B4508            mov eax,[ebp+0x8]
00008A62  8B500C            mov edx,[eax+0xc]
00008A65  8B4508            mov eax,[ebp+0x8]
00008A68  8B4014            mov eax,[eax+0x14]
00008A6B  01D0              add eax,edx
00008A6D  83E810            sub eax,byte +0x10
00008A70  394510            cmp [ebp+0x10],eax
00008A73  7D16              jnl 0x8a8b
00008A75  0FBE45F3          movsx eax,byte [ebp-0xd]
00008A79  FF7518            push dword [ebp+0x18]
00008A7C  50                push eax
00008A7D  FF7510            push dword [ebp+0x10]
00008A80  FF750C            push dword [ebp+0xc]
00008A83  E8E2BDFFFF        call 0x486a
00008A88  83C410            add esp,byte +0x10
00008A8B  83450C0A          add dword [ebp+0xc],byte +0xa
00008A8F  83451401          add dword [ebp+0x14],byte +0x1
00008A93  8B4514            mov eax,[ebp+0x14]
00008A96  0FB600            movzx eax,byte [eax]
00008A99  84C0              test al,al
00008A9B  7591              jnz 0x8a2e
00008A9D  EB01              jmp short 0x8aa0
00008A9F  90                nop
00008AA0  C9                leave
00008AA1  C3                ret
00008AA2  55                push ebp
00008AA3  89E5              mov ebp,esp
00008AA5  53                push ebx
00008AA6  83EC14            sub esp,byte +0x14
00008AA9  8B450C            mov eax,[ebp+0xc]
00008AAC  8D5004            lea edx,[eax+0x4]
00008AAF  8B4508            mov eax,[ebp+0x8]
00008AB2  83C00C            add eax,byte +0xc
00008AB5  83EC0C            sub esp,byte +0xc
00008AB8  68FFFFFF00        push dword 0xffffff
00008ABD  6A20              push byte +0x20
00008ABF  6A28              push byte +0x28
00008AC1  52                push edx
00008AC2  50                push eax
00008AC3  E885ABFFFF        call 0x364d
00008AC8  83C420            add esp,byte +0x20
00008ACB  8B450C            mov eax,[ebp+0xc]
00008ACE  8D5008            lea edx,[eax+0x8]
00008AD1  8B4508            mov eax,[ebp+0x8]
00008AD4  83C010            add eax,byte +0x10
00008AD7  83EC0C            sub esp,byte +0xc
00008ADA  68AA000000        push dword 0xaa
00008ADF  6A18              push byte +0x18
00008AE1  6A20              push byte +0x20
00008AE3  52                push edx
00008AE4  50                push eax
00008AE5  E863ABFFFF        call 0x364d
00008AEA  83C420            add esp,byte +0x20
00008AED  8B450C            mov eax,[ebp+0xc]
00008AF0  8D500E            lea edx,[eax+0xe]
00008AF3  8B4508            mov eax,[ebp+0x8]
00008AF6  83C016            add eax,byte +0x16
00008AF9  68FFFFFF00        push dword 0xffffff
00008AFE  6A50              push byte +0x50
00008B00  52                push edx
00008B01  50                push eax
00008B02  E863BDFFFF        call 0x486a
00008B07  83C410            add esp,byte +0x10
00008B0A  8B450C            mov eax,[ebp+0xc]
00008B0D  8D500E            lea edx,[eax+0xe]
00008B10  8B4508            mov eax,[ebp+0x8]
00008B13  83C01E            add eax,byte +0x1e
00008B16  68FFFFFF00        push dword 0xffffff
00008B1B  6A43              push byte +0x43
00008B1D  52                push edx
00008B1E  50                push eax
00008B1F  E846BDFFFF        call 0x486a
00008B24  83C410            add esp,byte +0x10
00008B27  8B450C            mov eax,[ebp+0xc]
00008B2A  8D5012            lea edx,[eax+0x12]
00008B2D  8B4508            mov eax,[ebp+0x8]
00008B30  83C01A            add eax,byte +0x1a
00008B33  6855FF5500        push dword 0x55ff55
00008B38  6A40              push byte +0x40
00008B3A  52                push edx
00008B3B  50                push eax
00008B3C  E829BDFFFF        call 0x486a
00008B41  83C410            add esp,byte +0x10
00008B44  8B450C            mov eax,[ebp+0xc]
00008B47  8D5024            lea edx,[eax+0x24]
00008B4A  8B4508            mov eax,[ebp+0x8]
00008B4D  83C01A            add eax,byte +0x1a
00008B50  83EC0C            sub esp,byte +0xc
00008B53  68FFFFFF00        push dword 0xffffff
00008B58  6A08              push byte +0x8
00008B5A  6A0C              push byte +0xc
00008B5C  52                push edx
00008B5D  50                push eax
00008B5E  E8EAAAFFFF        call 0x364d
00008B63  83C420            add esp,byte +0x20
00008B66  8B450C            mov eax,[ebp+0xc]
00008B69  8D502C            lea edx,[eax+0x2c]
00008B6C  8B4508            mov eax,[ebp+0x8]
00008B6F  83C008            add eax,byte +0x8
00008B72  83EC0C            sub esp,byte +0xc
00008B75  68FFFFFF00        push dword 0xffffff
00008B7A  6A0C              push byte +0xc
00008B7C  6A30              push byte +0x30
00008B7E  52                push edx
00008B7F  50                push eax
00008B80  E8C8AAFFFF        call 0x364d
00008B85  83C420            add esp,byte +0x20
00008B88  C745F400000000    mov dword [ebp-0xc],0x0
00008B8F  EB31              jmp short 0x8bc2
00008B91  8B450C            mov eax,[ebp+0xc]
00008B94  8D4830            lea ecx,[eax+0x30]
00008B97  8B4508            mov eax,[ebp+0x8]
00008B9A  8D580E            lea ebx,[eax+0xe]
00008B9D  8B55F4            mov edx,[ebp-0xc]
00008BA0  89D0              mov eax,edx
00008BA2  01C0              add eax,eax
00008BA4  01D0              add eax,edx
00008BA6  C1E002            shl eax,byte 0x2
00008BA9  01D8              add eax,ebx
00008BAB  83EC0C            sub esp,byte +0xc
00008BAE  6A00              push byte +0x0
00008BB0  6A04              push byte +0x4
00008BB2  6A08              push byte +0x8
00008BB4  51                push ecx
00008BB5  50                push eax
00008BB6  E892AAFFFF        call 0x364d
00008BBB  83C420            add esp,byte +0x20
00008BBE  8345F401          add dword [ebp-0xc],byte +0x1
00008BC2  837DF402          cmp dword [ebp-0xc],byte +0x2
00008BC6  7EC9              jng 0x8b91
00008BC8  8B450C            mov eax,[ebp+0xc]
00008BCB  8D5006            lea edx,[eax+0x6]
00008BCE  8B4508            mov eax,[ebp+0x8]
00008BD1  83C00A            add eax,byte +0xa
00008BD4  83EC0C            sub esp,byte +0xc
00008BD7  6840404000        push dword 0x404040
00008BDC  6A1C              push byte +0x1c
00008BDE  6A02              push byte +0x2
00008BE0  52                push edx
00008BE1  50                push eax
00008BE2  E866AAFFFF        call 0x364d
00008BE7  83C420            add esp,byte +0x20
00008BEA  8B450C            mov eax,[ebp+0xc]
00008BED  8D5006            lea edx,[eax+0x6]
00008BF0  8B4508            mov eax,[ebp+0x8]
00008BF3  83C034            add eax,byte +0x34
00008BF6  83EC0C            sub esp,byte +0xc
00008BF9  6840404000        push dword 0x404040
00008BFE  6A1C              push byte +0x1c
00008C00  6A02              push byte +0x2
00008C02  52                push edx
00008C03  50                push eax
00008C04  E844AAFFFF        call 0x364d
00008C09  83C420            add esp,byte +0x20
00008C0C  90                nop
00008C0D  8B5DFC            mov ebx,[ebp-0x4]
00008C10  C9                leave
00008C11  C3                ret
00008C12  55                push ebp
00008C13  89E5              mov ebp,esp
00008C15  83EC08            sub esp,byte +0x8
00008C18  8B450C            mov eax,[ebp+0xc]
00008C1B  8D5004            lea edx,[eax+0x4]
00008C1E  8B4508            mov eax,[ebp+0x8]
00008C21  83C008            add eax,byte +0x8
00008C24  83EC0C            sub esp,byte +0xc
00008C27  68FFFFFF00        push dword 0xffffff
00008C2C  6A28              push byte +0x28
00008C2E  6A30              push byte +0x30
00008C30  52                push edx
00008C31  50                push eax
00008C32  E816AAFFFF        call 0x364d
00008C37  83C420            add esp,byte +0x20
00008C3A  8B450C            mov eax,[ebp+0xc]
00008C3D  8D5004            lea edx,[eax+0x4]
00008C40  8B4508            mov eax,[ebp+0x8]
00008C43  83C008            add eax,byte +0x8
00008C46  83EC0C            sub esp,byte +0xc
00008C49  68AAAA0000        push dword 0xaaaa
00008C4E  6A08              push byte +0x8
00008C50  6A30              push byte +0x30
00008C52  52                push edx
00008C53  50                push eax
00008C54  E8F4A9FFFF        call 0x364d
00008C59  83C420            add esp,byte +0x20
00008C5C  8B450C            mov eax,[ebp+0xc]
00008C5F  8D5004            lea edx,[eax+0x4]
00008C62  8B4508            mov eax,[ebp+0x8]
00008C65  83C00C            add eax,byte +0xc
00008C68  68FFFFFF00        push dword 0xffffff
00008C6D  6A54              push byte +0x54
00008C6F  52                push edx
00008C70  50                push eax
00008C71  E8F4BBFFFF        call 0x486a
00008C76  83C410            add esp,byte +0x10
00008C79  8B450C            mov eax,[ebp+0xc]
00008C7C  8D5004            lea edx,[eax+0x4]
00008C7F  8B4508            mov eax,[ebp+0x8]
00008C82  83C014            add eax,byte +0x14
00008C85  68FFFFFF00        push dword 0xffffff
00008C8A  6A45              push byte +0x45
00008C8C  52                push edx
00008C8D  50                push eax
00008C8E  E8D7BBFFFF        call 0x486a
00008C93  83C410            add esp,byte +0x10
00008C96  8B450C            mov eax,[ebp+0xc]
00008C99  8D5004            lea edx,[eax+0x4]
00008C9C  8B4508            mov eax,[ebp+0x8]
00008C9F  83C01C            add eax,byte +0x1c
00008CA2  68FFFFFF00        push dword 0xffffff
00008CA7  6A52              push byte +0x52
00008CA9  52                push edx
00008CAA  50                push eax
00008CAB  E8BABBFFFF        call 0x486a
00008CB0  83C410            add esp,byte +0x10
00008CB3  8B450C            mov eax,[ebp+0xc]
00008CB6  8D5004            lea edx,[eax+0x4]
00008CB9  8B4508            mov eax,[ebp+0x8]
00008CBC  83C024            add eax,byte +0x24
00008CBF  68FFFFFF00        push dword 0xffffff
00008CC4  6A4D              push byte +0x4d
00008CC6  52                push edx
00008CC7  50                push eax
00008CC8  E89DBBFFFF        call 0x486a
00008CCD  83C410            add esp,byte +0x10
00008CD0  8B450C            mov eax,[ebp+0xc]
00008CD3  8D5010            lea edx,[eax+0x10]
00008CD6  8B4508            mov eax,[ebp+0x8]
00008CD9  83C00C            add eax,byte +0xc
00008CDC  83EC0C            sub esp,byte +0xc
00008CDF  6A00              push byte +0x0
00008CE1  6A18              push byte +0x18
00008CE3  6A28              push byte +0x28
00008CE5  52                push edx
00008CE6  50                push eax
00008CE7  E861A9FFFF        call 0x364d
00008CEC  83C420            add esp,byte +0x20
00008CEF  8B450C            mov eax,[ebp+0xc]
00008CF2  8D5014            lea edx,[eax+0x14]
00008CF5  8B4508            mov eax,[ebp+0x8]
00008CF8  83C010            add eax,byte +0x10
00008CFB  6855FF5500        push dword 0x55ff55
00008D00  6A24              push byte +0x24
00008D02  52                push edx
00008D03  50                push eax
00008D04  E861BBFFFF        call 0x486a
00008D09  83C410            add esp,byte +0x10
00008D0C  8B450C            mov eax,[ebp+0xc]
00008D0F  8D5014            lea edx,[eax+0x14]
00008D12  8B4508            mov eax,[ebp+0x8]
00008D15  83C018            add eax,byte +0x18
00008D18  6855FF5500        push dword 0x55ff55
00008D1D  6A3E              push byte +0x3e
00008D1F  52                push edx
00008D20  50                push eax
00008D21  E844BBFFFF        call 0x486a
00008D26  83C410            add esp,byte +0x10
00008D29  8B450C            mov eax,[ebp+0xc]
00008D2C  8D5014            lea edx,[eax+0x14]
00008D2F  8B4508            mov eax,[ebp+0x8]
00008D32  83C020            add eax,byte +0x20
00008D35  68FFFFFF00        push dword 0xffffff
00008D3A  6A6C              push byte +0x6c
00008D3C  52                push edx
00008D3D  50                push eax
00008D3E  E827BBFFFF        call 0x486a
00008D43  83C410            add esp,byte +0x10
00008D46  8B450C            mov eax,[ebp+0xc]
00008D49  8D5014            lea edx,[eax+0x14]
00008D4C  8B4508            mov eax,[ebp+0x8]
00008D4F  83C028            add eax,byte +0x28
00008D52  68FFFFFF00        push dword 0xffffff
00008D57  6A73              push byte +0x73
00008D59  52                push edx
00008D5A  50                push eax
00008D5B  E80ABBFFFF        call 0x486a
00008D60  83C410            add esp,byte +0x10
00008D63  8B450C            mov eax,[ebp+0xc]
00008D66  8D5014            lea edx,[eax+0x14]
00008D69  8B4508            mov eax,[ebp+0x8]
00008D6C  83C030            add eax,byte +0x30
00008D6F  83EC0C            sub esp,byte +0xc
00008D72  68FFFFFF00        push dword 0xffffff
00008D77  6A10              push byte +0x10
00008D79  6A04              push byte +0x4
00008D7B  52                push edx
00008D7C  50                push eax
00008D7D  E8CBA8FFFF        call 0x364d
00008D82  83C420            add esp,byte +0x20
00008D85  90                nop
00008D86  C9                leave
00008D87  C3                ret
00008D88  55                push ebp
00008D89  89E5              mov ebp,esp
00008D8B  53                push ebx
00008D8C  83EC64            sub esp,byte +0x64
00008D8F  8B4508            mov eax,[ebp+0x8]
00008D92  8B4008            mov eax,[eax+0x8]
00008D95  83C002            add eax,byte +0x2
00008D98  8945E4            mov [ebp-0x1c],eax
00008D9B  8B4508            mov eax,[ebp+0x8]
00008D9E  8B400C            mov eax,[eax+0xc]
00008DA1  83C014            add eax,byte +0x14
00008DA4  8945E0            mov [ebp-0x20],eax
00008DA7  8B4508            mov eax,[ebp+0x8]
00008DAA  8B4010            mov eax,[eax+0x10]
00008DAD  83E804            sub eax,byte +0x4
00008DB0  8945DC            mov [ebp-0x24],eax
00008DB3  8B4508            mov eax,[ebp+0x8]
00008DB6  8B4014            mov eax,[eax+0x14]
00008DB9  83E816            sub eax,byte +0x16
00008DBC  8945D8            mov [ebp-0x28],eax
00008DBF  83EC0C            sub esp,byte +0xc
00008DC2  68C0C0C000        push dword 0xc0c0c0
00008DC7  FF75D8            push dword [ebp-0x28]
00008DCA  FF75DC            push dword [ebp-0x24]
00008DCD  FF75E0            push dword [ebp-0x20]
00008DD0  FF75E4            push dword [ebp-0x1c]
00008DD3  E875A8FFFF        call 0x364d
00008DD8  83C420            add esp,byte +0x20
00008DDB  8B45E4            mov eax,[ebp-0x1c]
00008DDE  83C00A            add eax,byte +0xa
00008DE1  8945D4            mov [ebp-0x2c],eax
00008DE4  8B45E0            mov eax,[ebp-0x20]
00008DE7  83C00A            add eax,byte +0xa
00008DEA  8945D0            mov [ebp-0x30],eax
00008DED  C745CC6E000000    mov dword [ebp-0x34],0x6e
00008DF4  C745C83C000000    mov dword [ebp-0x38],0x3c
00008DFB  C745F403000000    mov dword [ebp-0xc],0x3
00008E02  8B45DC            mov eax,[ebp-0x24]
00008E05  83E814            sub eax,byte +0x14
00008E08  99                cdq
00008E09  F77DCC            idiv dword [ebp-0x34]
00008E0C  8945F4            mov [ebp-0xc],eax
00008E0F  837DF400          cmp dword [ebp-0xc],byte +0x0
00008E13  7F07              jg 0x8e1c
00008E15  C745F401000000    mov dword [ebp-0xc],0x1
00008E1C  C745F000000000    mov dword [ebp-0x10],0x0
00008E23  E9B2020000        jmp 0x90da
00008E28  8B45F0            mov eax,[ebp-0x10]
00008E2B  99                cdq
00008E2C  F77DF4            idiv dword [ebp-0xc]
00008E2F  8955C4            mov [ebp-0x3c],edx
00008E32  8B45F0            mov eax,[ebp-0x10]
00008E35  99                cdq
00008E36  F77DF4            idiv dword [ebp-0xc]
00008E39  8945C0            mov [ebp-0x40],eax
00008E3C  8B45C4            mov eax,[ebp-0x3c]
00008E3F  0FAF45CC          imul eax,[ebp-0x34]
00008E43  89C2              mov edx,eax
00008E45  8B45D4            mov eax,[ebp-0x2c]
00008E48  01D0              add eax,edx
00008E4A  8945BC            mov [ebp-0x44],eax
00008E4D  8B45C0            mov eax,[ebp-0x40]
00008E50  0FAF45C8          imul eax,[ebp-0x38]
00008E54  89C2              mov edx,eax
00008E56  8B45D0            mov eax,[ebp-0x30]
00008E59  01D0              add eax,edx
00008E5B  8945B8            mov [ebp-0x48],eax
00008E5E  8B4508            mov eax,[ebp+0x8]
00008E61  8B809C0C0000      mov eax,[eax+0xc9c]
00008E67  3945F0            cmp [ebp-0x10],eax
00008E6A  0F94C0            setz al
00008E6D  0FB6C0            movzx eax,al
00008E70  8945B4            mov [ebp-0x4c],eax
00008E73  837DB400          cmp dword [ebp-0x4c],byte +0x0
00008E77  7422              jz 0x8e9b
00008E79  8B45B8            mov eax,[ebp-0x48]
00008E7C  8D50FE            lea edx,[eax-0x2]
00008E7F  8B45BC            mov eax,[ebp-0x44]
00008E82  83E802            sub eax,byte +0x2
00008E85  83EC0C            sub esp,byte +0xc
00008E88  6880000000        push dword 0x80
00008E8D  6A38              push byte +0x38
00008E8F  6A28              push byte +0x28
00008E91  52                push edx
00008E92  50                push eax
00008E93  E8B5A7FFFF        call 0x364d
00008E98  83C420            add esp,byte +0x20
00008E9B  8B45BC            mov eax,[ebp-0x44]
00008E9E  83C023            add eax,byte +0x23
00008EA1  8945B0            mov [ebp-0x50],eax
00008EA4  8B45B8            mov eax,[ebp-0x48]
00008EA7  8945AC            mov [ebp-0x54],eax
00008EAA  8B4D08            mov ecx,[ebp+0x8]
00008EAD  8B55F0            mov edx,[ebp-0x10]
00008EB0  89D0              mov eax,edx
00008EB2  C1E002            shl eax,byte 0x2
00008EB5  01D0              add eax,edx
00008EB7  C1E002            shl eax,byte 0x2
00008EBA  01C8              add eax,ecx
00008EBC  05680B0000        add eax,0xb68
00008EC1  8B00              mov eax,[eax]
00008EC3  85C0              test eax,eax
00008EC5  743D              jz 0x8f04
00008EC7  8B45AC            mov eax,[ebp-0x54]
00008ECA  83C006            add eax,byte +0x6
00008ECD  83EC0C            sub esp,byte +0xc
00008ED0  6855FFFF00        push dword 0xffff55
00008ED5  6A18              push byte +0x18
00008ED7  6A1E              push byte +0x1e
00008ED9  50                push eax
00008EDA  FF75B0            push dword [ebp-0x50]
00008EDD  E86BA7FFFF        call 0x364d
00008EE2  83C420            add esp,byte +0x20
00008EE5  83EC0C            sub esp,byte +0xc
00008EE8  6855FFFF00        push dword 0xffff55
00008EED  6A06              push byte +0x6
00008EEF  6A0C              push byte +0xc
00008EF1  FF75AC            push dword [ebp-0x54]
00008EF4  FF75B0            push dword [ebp-0x50]
00008EF7  E851A7FFFF        call 0x364d
00008EFC  83C420            add esp,byte +0x20
00008EFF  E92D010000        jmp 0x9031
00008F04  8B55F0            mov edx,[ebp-0x10]
00008F07  89D0              mov eax,edx
00008F09  C1E002            shl eax,byte 0x2
00008F0C  01D0              add eax,edx
00008F0E  C1E002            shl eax,byte 0x2
00008F11  8D90500B0000      lea edx,[eax+0xb50]
00008F17  8B4508            mov eax,[ebp+0x8]
00008F1A  01D0              add eax,edx
00008F1C  83C008            add eax,byte +0x8
00008F1F  83EC0C            sub esp,byte +0xc
00008F22  50                push eax
00008F23  E8DBF5FFFF        call 0x8503
00008F28  83C410            add esp,byte +0x10
00008F2B  8945A8            mov [ebp-0x58],eax
00008F2E  837DA804          cmp dword [ebp-0x58],byte +0x4
00008F32  7E79              jng 0x8fad
00008F34  8B45A8            mov eax,[ebp-0x58]
00008F37  8D58FD            lea ebx,[eax-0x3]
00008F3A  8B4D08            mov ecx,[ebp+0x8]
00008F3D  8B55F0            mov edx,[ebp-0x10]
00008F40  89D0              mov eax,edx
00008F42  C1E002            shl eax,byte 0x2
00008F45  01D0              add eax,edx
00008F47  C1E002            shl eax,byte 0x2
00008F4A  01C8              add eax,ecx
00008F4C  01D8              add eax,ebx
00008F4E  05580B0000        add eax,0xb58
00008F53  0FB600            movzx eax,byte [eax]
00008F56  3C42              cmp al,0x42
00008F58  7553              jnz 0x8fad
00008F5A  8B45A8            mov eax,[ebp-0x58]
00008F5D  8D58FE            lea ebx,[eax-0x2]
00008F60  8B4D08            mov ecx,[ebp+0x8]
00008F63  8B55F0            mov edx,[ebp-0x10]
00008F66  89D0              mov eax,edx
00008F68  C1E002            shl eax,byte 0x2
00008F6B  01D0              add eax,edx
00008F6D  C1E002            shl eax,byte 0x2
00008F70  01C8              add eax,ecx
00008F72  01D8              add eax,ebx
00008F74  05580B0000        add eax,0xb58
00008F79  0FB600            movzx eax,byte [eax]
00008F7C  3C49              cmp al,0x49
00008F7E  752D              jnz 0x8fad
00008F80  8B45A8            mov eax,[ebp-0x58]
00008F83  8D58FF            lea ebx,[eax-0x1]
00008F86  8B4D08            mov ecx,[ebp+0x8]
00008F89  8B55F0            mov edx,[ebp-0x10]
00008F8C  89D0              mov eax,edx
00008F8E  C1E002            shl eax,byte 0x2
00008F91  01D0              add eax,edx
00008F93  C1E002            shl eax,byte 0x2
00008F96  01C8              add eax,ecx
00008F98  01D8              add eax,ebx
00008F9A  05580B0000        add eax,0xb58
00008F9F  0FB600            movzx eax,byte [eax]
00008FA2  3C4E              cmp al,0x4e
00008FA4  7507              jnz 0x8fad
00008FA6  B801000000        mov eax,0x1
00008FAB  EB05              jmp short 0x8fb2
00008FAD  B800000000        mov eax,0x0
00008FB2  8945A4            mov [ebp-0x5c],eax
00008FB5  837DA400          cmp dword [ebp-0x5c],byte +0x0
00008FB9  743E              jz 0x8ff9
00008FBB  83EC0C            sub esp,byte +0xc
00008FBE  68FFFFFF00        push dword 0xffffff
00008FC3  6A20              push byte +0x20
00008FC5  6A1C              push byte +0x1c
00008FC7  FF75AC            push dword [ebp-0x54]
00008FCA  FF75B0            push dword [ebp-0x50]
00008FCD  E87BA6FFFF        call 0x364d
00008FD2  83C420            add esp,byte +0x20
00008FD5  8B45AC            mov eax,[ebp-0x54]
00008FD8  8D5004            lea edx,[eax+0x4]
00008FDB  8B45B0            mov eax,[ebp-0x50]
00008FDE  83C004            add eax,byte +0x4
00008FE1  83EC0C            sub esp,byte +0xc
00008FE4  68AA000000        push dword 0xaa
00008FE9  6A10              push byte +0x10
00008FEB  6A14              push byte +0x14
00008FED  52                push edx
00008FEE  50                push eax
00008FEF  E859A6FFFF        call 0x364d
00008FF4  83C420            add esp,byte +0x20
00008FF7  EB38              jmp short 0x9031
00008FF9  83EC0C            sub esp,byte +0xc
00008FFC  68FFFFFF00        push dword 0xffffff
00009001  6A20              push byte +0x20
00009003  6A18              push byte +0x18
00009005  FF75AC            push dword [ebp-0x54]
00009008  FF75B0            push dword [ebp-0x50]
0000900B  E83DA6FFFF        call 0x364d
00009010  83C420            add esp,byte +0x20
00009013  8B45B0            mov eax,[ebp-0x50]
00009016  83C010            add eax,byte +0x10
00009019  83EC0C            sub esp,byte +0xc
0000901C  68C0C0C000        push dword 0xc0c0c0
00009021  6A08              push byte +0x8
00009023  6A08              push byte +0x8
00009025  FF75AC            push dword [ebp-0x54]
00009028  50                push eax
00009029  E81FA6FFFF        call 0x364d
0000902E  83C420            add esp,byte +0x20
00009031  8B45BC            mov eax,[ebp-0x44]
00009034  83C005            add eax,byte +0x5
00009037  8945A0            mov [ebp-0x60],eax
0000903A  8B45B8            mov eax,[ebp-0x48]
0000903D  83C024            add eax,byte +0x24
00009040  89459C            mov [ebp-0x64],eax
00009043  8B55F0            mov edx,[ebp-0x10]
00009046  89D0              mov eax,edx
00009048  C1E002            shl eax,byte 0x2
0000904B  01D0              add eax,edx
0000904D  C1E002            shl eax,byte 0x2
00009050  8D90500B0000      lea edx,[eax+0xb50]
00009056  8B4508            mov eax,[ebp+0x8]
00009059  01D0              add eax,edx
0000905B  83C008            add eax,byte +0x8
0000905E  894598            mov [ebp-0x68],eax
00009061  C745EC00000000    mov dword [ebp-0x14],0x0
00009068  EB57              jmp short 0x90c1
0000906A  837DB400          cmp dword [ebp-0x4c],byte +0x0
0000906E  7407              jz 0x9077
00009070  B8FFFFFF00        mov eax,0xffffff
00009075  EB05              jmp short 0x907c
00009077  B800000000        mov eax,0x0
0000907C  8945E8            mov [ebp-0x18],eax
0000907F  837DB400          cmp dword [ebp-0x4c],byte +0x0
00009083  7407              jz 0x908c
00009085  C745E8FFFFFF00    mov dword [ebp-0x18],0xffffff
0000908C  8B55EC            mov edx,[ebp-0x14]
0000908F  8B4598            mov eax,[ebp-0x68]
00009092  01D0              add eax,edx
00009094  0FB600            movzx eax,byte [eax]
00009097  0FBEC8            movsx ecx,al
0000909A  8B55EC            mov edx,[ebp-0x14]
0000909D  89D0              mov eax,edx
0000909F  C1E002            shl eax,byte 0x2
000090A2  01D0              add eax,edx
000090A4  01C0              add eax,eax
000090A6  89C2              mov edx,eax
000090A8  8B45A0            mov eax,[ebp-0x60]
000090AB  01D0              add eax,edx
000090AD  FF75E8            push dword [ebp-0x18]
000090B0  51                push ecx
000090B1  FF759C            push dword [ebp-0x64]
000090B4  50                push eax
000090B5  E8B0B7FFFF        call 0x486a
000090BA  83C410            add esp,byte +0x10
000090BD  8345EC01          add dword [ebp-0x14],byte +0x1
000090C1  8B55EC            mov edx,[ebp-0x14]
000090C4  8B4598            mov eax,[ebp-0x68]
000090C7  01D0              add eax,edx
000090C9  0FB600            movzx eax,byte [eax]
000090CC  84C0              test al,al
000090CE  7406              jz 0x90d6
000090D0  837DEC0B          cmp dword [ebp-0x14],byte +0xb
000090D4  7E94              jng 0x906a
000090D6  8345F001          add dword [ebp-0x10],byte +0x1
000090DA  8B4508            mov eax,[ebp+0x8]
000090DD  8B80980C0000      mov eax,[eax+0xc98]
000090E3  3945F0            cmp [ebp-0x10],eax
000090E6  0F8C3CFDFFFF      jl near 0x8e28
000090EC  90                nop
000090ED  8B5DFC            mov ebx,[ebp-0x4]
000090F0  C9                leave
000090F1  C3                ret
000090F2  55                push ebp
000090F3  89E5              mov ebp,esp
000090F5  53                push ebx
000090F6  83EC24            sub esp,byte +0x24
000090F9  837D0C00          cmp dword [ebp+0xc],byte +0x0
000090FD  751D              jnz 0x911c
000090FF  8B4508            mov eax,[ebp+0x8]
00009102  8B5004            mov edx,[eax+0x4]
00009105  8B4508            mov eax,[ebp+0x8]
00009108  8B00              mov eax,[eax]
0000910A  83EC08            sub esp,byte +0x8
0000910D  52                push edx
0000910E  50                push eax
0000910F  E88EF9FFFF        call 0x8aa2
00009114  83C410            add esp,byte +0x10
00009117  E944010000        jmp 0x9260
0000911C  837D0C01          cmp dword [ebp+0xc],byte +0x1
00009120  751D              jnz 0x913f
00009122  8B4508            mov eax,[ebp+0x8]
00009125  8B5004            mov edx,[eax+0x4]
00009128  8B4508            mov eax,[ebp+0x8]
0000912B  8B00              mov eax,[eax]
0000912D  83EC08            sub esp,byte +0x8
00009130  52                push edx
00009131  50                push eax
00009132  E8DBFAFFFF        call 0x8c12
00009137  83C410            add esp,byte +0x10
0000913A  E921010000        jmp 0x9260
0000913F  837D0C03          cmp dword [ebp+0xc],byte +0x3
00009143  0F85EC000000      jnz near 0x9235
00009149  8B4508            mov eax,[ebp+0x8]
0000914C  8B4004            mov eax,[eax+0x4]
0000914F  8D5010            lea edx,[eax+0x10]
00009152  8B4508            mov eax,[ebp+0x8]
00009155  8B00              mov eax,[eax]
00009157  83C010            add eax,byte +0x10
0000915A  83EC0C            sub esp,byte +0xc
0000915D  68C0C0C000        push dword 0xc0c0c0
00009162  6A20              push byte +0x20
00009164  6A20              push byte +0x20
00009166  52                push edx
00009167  50                push eax
00009168  E8E0A4FFFF        call 0x364d
0000916D  83C420            add esp,byte +0x20
00009170  8B4508            mov eax,[ebp+0x8]
00009173  8B4004            mov eax,[eax+0x4]
00009176  8D5014            lea edx,[eax+0x14]
00009179  8B4508            mov eax,[ebp+0x8]
0000917C  8B00              mov eax,[eax]
0000917E  83C014            add eax,byte +0x14
00009181  83EC0C            sub esp,byte +0xc
00009184  680000FF00        push dword 0xff0000
00009189  6A08              push byte +0x8
0000918B  6A08              push byte +0x8
0000918D  52                push edx
0000918E  50                push eax
0000918F  E8B9A4FFFF        call 0x364d
00009194  83C420            add esp,byte +0x20
00009197  8B4508            mov eax,[ebp+0x8]
0000919A  8B4004            mov eax,[eax+0x4]
0000919D  8D5014            lea edx,[eax+0x14]
000091A0  8B4508            mov eax,[ebp+0x8]
000091A3  8B00              mov eax,[eax]
000091A5  83C01E            add eax,byte +0x1e
000091A8  83EC0C            sub esp,byte +0xc
000091AB  6800FF0000        push dword 0xff00
000091B0  6A08              push byte +0x8
000091B2  6A08              push byte +0x8
000091B4  52                push edx
000091B5  50                push eax
000091B6  E892A4FFFF        call 0x364d
000091BB  83C420            add esp,byte +0x20
000091BE  8B4508            mov eax,[ebp+0x8]
000091C1  8B4004            mov eax,[eax+0x4]
000091C4  8D501E            lea edx,[eax+0x1e]
000091C7  8B4508            mov eax,[ebp+0x8]
000091CA  8B00              mov eax,[eax]
000091CC  83C014            add eax,byte +0x14
000091CF  83EC0C            sub esp,byte +0xc
000091D2  68FF000000        push dword 0xff
000091D7  6A08              push byte +0x8
000091D9  6A08              push byte +0x8
000091DB  52                push edx
000091DC  50                push eax
000091DD  E86BA4FFFF        call 0x364d
000091E2  83C420            add esp,byte +0x20
000091E5  8B4508            mov eax,[ebp+0x8]
000091E8  8B4004            mov eax,[eax+0x4]
000091EB  8D501E            lea edx,[eax+0x1e]
000091EE  8B4508            mov eax,[ebp+0x8]
000091F1  8B00              mov eax,[eax]
000091F3  83C01E            add eax,byte +0x1e
000091F6  83EC0C            sub esp,byte +0xc
000091F9  6800FFFF00        push dword 0xffff00
000091FE  6A08              push byte +0x8
00009200  6A08              push byte +0x8
00009202  52                push edx
00009203  50                push eax
00009204  E844A4FFFF        call 0x364d
00009209  83C420            add esp,byte +0x20
0000920C  8B4508            mov eax,[ebp+0x8]
0000920F  8B4004            mov eax,[eax+0x4]
00009212  8D5010            lea edx,[eax+0x10]
00009215  8B4508            mov eax,[ebp+0x8]
00009218  8B00              mov eax,[eax]
0000921A  83C028            add eax,byte +0x28
0000921D  83EC0C            sub esp,byte +0xc
00009220  6800408000        push dword 0x804000
00009225  6A10              push byte +0x10
00009227  6A04              push byte +0x4
00009229  52                push edx
0000922A  50                push eax
0000922B  E81DA4FFFF        call 0x364d
00009230  83C420            add esp,byte +0x20
00009233  EB2B              jmp short 0x9260
00009235  8B4508            mov eax,[ebp+0x8]
00009238  8B4818            mov ecx,[eax+0x18]
0000923B  8B4508            mov eax,[ebp+0x8]
0000923E  8B5014            mov edx,[eax+0x14]
00009241  8B4508            mov eax,[ebp+0x8]
00009244  8B4004            mov eax,[eax+0x4]
00009247  8B5D08            mov ebx,[ebp+0x8]
0000924A  8B1B              mov ebx,[ebx]
0000924C  83C310            add ebx,byte +0x10
0000924F  83EC0C            sub esp,byte +0xc
00009252  6A00              push byte +0x0
00009254  51                push ecx
00009255  52                push edx
00009256  50                push eax
00009257  53                push ebx
00009258  E8A2F5FFFF        call 0x87ff
0000925D  83C420            add esp,byte +0x20
00009260  8B4508            mov eax,[ebp+0x8]
00009263  8B00              mov eax,[eax]
00009265  83C020            add eax,byte +0x20
00009268  8945EC            mov [ebp-0x14],eax
0000926B  8B4508            mov eax,[ebp+0x8]
0000926E  8B4010            mov eax,[eax+0x10]
00009271  83EC0C            sub esp,byte +0xc
00009274  50                push eax
00009275  E889F2FFFF        call 0x8503
0000927A  83C410            add esp,byte +0x10
0000927D  8945E8            mov [ebp-0x18],eax
00009280  8B45E8            mov eax,[ebp-0x18]
00009283  6BC0F6            imul eax,eax,byte -0xa
00009286  89C2              mov edx,eax
00009288  C1EA1F            shr edx,byte 0x1f
0000928B  01D0              add eax,edx
0000928D  D1F8              sar eax,1
0000928F  89C2              mov edx,eax
00009291  8B45EC            mov eax,[ebp-0x14]
00009294  01D0              add eax,edx
00009296  8945E4            mov [ebp-0x1c],eax
00009299  8B4508            mov eax,[ebp+0x8]
0000929C  8B4004            mov eax,[eax+0x4]
0000929F  83C030            add eax,byte +0x30
000092A2  8945E0            mov [ebp-0x20],eax
000092A5  8B4508            mov eax,[ebp+0x8]
000092A8  8B4010            mov eax,[eax+0x10]
000092AB  8945F4            mov [ebp-0xc],eax
000092AE  8B45E4            mov eax,[ebp-0x1c]
000092B1  8945F0            mov [ebp-0x10],eax
000092B4  EB49              jmp short 0x92ff
000092B6  8B4508            mov eax,[ebp+0x8]
000092B9  8B4018            mov eax,[eax+0x18]
000092BC  85C0              test eax,eax
000092BE  741A              jz 0x92da
000092C0  83EC0C            sub esp,byte +0xc
000092C3  6880000000        push dword 0x80
000092C8  6A10              push byte +0x10
000092CA  6A0A              push byte +0xa
000092CC  FF75E0            push dword [ebp-0x20]
000092CF  FF75F0            push dword [ebp-0x10]
000092D2  E876A3FFFF        call 0x364d
000092D7  83C420            add esp,byte +0x20
000092DA  8B45F4            mov eax,[ebp-0xc]
000092DD  0FB600            movzx eax,byte [eax]
000092E0  0FBEC0            movsx eax,al
000092E3  68FFFFFF00        push dword 0xffffff
000092E8  50                push eax
000092E9  FF75E0            push dword [ebp-0x20]
000092EC  FF75F0            push dword [ebp-0x10]
000092EF  E876B5FFFF        call 0x486a
000092F4  83C410            add esp,byte +0x10
000092F7  8345F00A          add dword [ebp-0x10],byte +0xa
000092FB  8345F401          add dword [ebp-0xc],byte +0x1
000092FF  8B45F4            mov eax,[ebp-0xc]
00009302  0FB600            movzx eax,byte [eax]
00009305  84C0              test al,al
00009307  75AD              jnz 0x92b6
00009309  90                nop
0000930A  8B5DFC            mov ebx,[ebp-0x4]
0000930D  C9                leave
0000930E  C3                ret
0000930F  55                push ebp
00009310  89E5              mov ebp,esp
00009312  83EC18            sub esp,byte +0x18
00009315  C745F400000000    mov dword [ebp-0xc],0x0
0000931C  EB1E              jmp short 0x933c
0000931E  8B45F4            mov eax,[ebp-0xc]
00009321  6BC01C            imul eax,eax,byte +0x1c
00009324  0580280200        add eax,0x22880
00009329  83EC08            sub esp,byte +0x8
0000932C  FF75F4            push dword [ebp-0xc]
0000932F  50                push eax
00009330  E8BDFDFFFF        call 0x90f2
00009335  83C410            add esp,byte +0x10
00009338  8345F401          add dword [ebp-0xc],byte +0x1
0000933C  A10C290200        mov eax,[0x2290c]
00009341  3945F4            cmp [ebp-0xc],eax
00009344  7CD8              jl 0x931e
00009346  90                nop
00009347  C9                leave
00009348  C3                ret
00009349  55                push ebp
0000934A  89E5              mov ebp,esp
0000934C  83EC10            sub esp,byte +0x10
0000934F  837D0800          cmp dword [ebp+0x8],byte +0x0
00009353  7476              jz 0x93cb
00009355  8B4508            mov eax,[ebp+0x8]
00009358  83C04C            add eax,byte +0x4c
0000935B  50                push eax
0000935C  E8A2F1FFFF        call 0x8503
00009361  83C404            add esp,byte +0x4
00009364  8945FC            mov [ebp-0x4],eax
00009367  FF750C            push dword [ebp+0xc]
0000936A  E894F1FFFF        call 0x8503
0000936F  83C404            add esp,byte +0x4
00009372  8945F4            mov [ebp-0xc],eax
00009375  8B55FC            mov edx,[ebp-0x4]
00009378  8B45F4            mov eax,[ebp-0xc]
0000937B  01D0              add eax,edx
0000937D  3DFE070000        cmp eax,0x7fe
00009382  7E0E              jng 0x9392
00009384  8B4508            mov eax,[ebp+0x8]
00009387  C6404C00          mov byte [eax+0x4c],0x0
0000938B  C745FC00000000    mov dword [ebp-0x4],0x0
00009392  8B4508            mov eax,[ebp+0x8]
00009395  8D504C            lea edx,[eax+0x4c]
00009398  8B45FC            mov eax,[ebp-0x4]
0000939B  01D0              add eax,edx
0000939D  8945F8            mov [ebp-0x8],eax
000093A0  EB17              jmp short 0x93b9
000093A2  8B550C            mov edx,[ebp+0xc]
000093A5  8D4201            lea eax,[edx+0x1]
000093A8  89450C            mov [ebp+0xc],eax
000093AB  8B45F8            mov eax,[ebp-0x8]
000093AE  8D4801            lea ecx,[eax+0x1]
000093B1  894DF8            mov [ebp-0x8],ecx
000093B4  0FB612            movzx edx,byte [edx]
000093B7  8810              mov [eax],dl
000093B9  8B450C            mov eax,[ebp+0xc]
000093BC  0FB600            movzx eax,byte [eax]
000093BF  84C0              test al,al
000093C1  75DF              jnz 0x93a2
000093C3  8B45F8            mov eax,[ebp-0x8]
000093C6  C60000            mov byte [eax],0x0
000093C9  EB01              jmp short 0x93cc
000093CB  90                nop
000093CC  C9                leave
000093CD  C3                ret
000093CE  55                push ebp
000093CF  89E5              mov ebp,esp
000093D1  83EC68            sub esp,byte +0x68
000093D4  837D0800          cmp dword [ebp+0x8],byte +0x0
000093D8  0F8423020000      jz near 0x9601
000093DE  8B4508            mov eax,[ebp+0x8]
000093E1  054C080000        add eax,0x84c
000093E6  50                push eax
000093E7  FF7508            push dword [ebp+0x8]
000093EA  E85AFFFFFF        call 0x9349
000093EF  83C408            add esp,byte +0x8
000093F2  68FCC20100        push dword 0x1c2fc
000093F7  FF7508            push dword [ebp+0x8]
000093FA  E84AFFFFFF        call 0x9349
000093FF  83C408            add esp,byte +0x8
00009402  8B4508            mov eax,[ebp+0x8]
00009405  054C080000        add eax,0x84c
0000940A  50                push eax
0000940B  E8F3F0FFFF        call 0x8503
00009410  83C404            add esp,byte +0x4
00009413  85C0              test eax,eax
00009415  0F8E5D010000      jng near 0x9578
0000941B  8B4508            mov eax,[ebp+0x8]
0000941E  8B80500B0000      mov eax,[eax+0xb50]
00009424  83F807            cmp eax,byte +0x7
00009427  0F8F8D000000      jg near 0x94ba
0000942D  C745F400000000    mov dword [ebp-0xc],0x0
00009434  EB32              jmp short 0x9468
00009436  8B4508            mov eax,[ebp+0x8]
00009439  8B88500B0000      mov ecx,[eax+0xb50]
0000943F  8B5508            mov edx,[ebp+0x8]
00009442  8B45F4            mov eax,[ebp-0xc]
00009445  01D0              add eax,edx
00009447  054C080000        add eax,0x84c
0000944C  0FB600            movzx eax,byte [eax]
0000944F  8B5508            mov edx,[ebp+0x8]
00009452  C1E106            shl ecx,byte 0x6
00009455  01D1              add ecx,edx
00009457  8B55F4            mov edx,[ebp-0xc]
0000945A  01CA              add edx,ecx
0000945C  81C250090000      add edx,0x950
00009462  8802              mov [edx],al
00009464  8345F401          add dword [ebp-0xc],byte +0x1
00009468  8B5508            mov edx,[ebp+0x8]
0000946B  8B45F4            mov eax,[ebp-0xc]
0000946E  01D0              add eax,edx
00009470  054C080000        add eax,0x84c
00009475  0FB600            movzx eax,byte [eax]
00009478  84C0              test al,al
0000947A  7406              jz 0x9482
0000947C  837DF43E          cmp dword [ebp-0xc],byte +0x3e
00009480  7EB4              jng 0x9436
00009482  8B4508            mov eax,[ebp+0x8]
00009485  8B90500B0000      mov edx,[eax+0xb50]
0000948B  8B4508            mov eax,[ebp+0x8]
0000948E  C1E206            shl edx,byte 0x6
00009491  01C2              add edx,eax
00009493  8B45F4            mov eax,[ebp-0xc]
00009496  01D0              add eax,edx
00009498  0550090000        add eax,0x950
0000949D  C60000            mov byte [eax],0x0
000094A0  8B4508            mov eax,[ebp+0x8]
000094A3  8B80500B0000      mov eax,[eax+0xb50]
000094A9  8D5001            lea edx,[eax+0x1]
000094AC  8B4508            mov eax,[ebp+0x8]
000094AF  8990500B0000      mov [eax+0xb50],edx
000094B5  E9BE000000        jmp 0x9578
000094BA  C745F000000000    mov dword [ebp-0x10],0x0
000094C1  EB58              jmp short 0x951b
000094C3  8B45F0            mov eax,[ebp-0x10]
000094C6  C1E006            shl eax,byte 0x6
000094C9  8D9050090000      lea edx,[eax+0x950]
000094CF  8B4508            mov eax,[ebp+0x8]
000094D2  01D0              add eax,edx
000094D4  8945EC            mov [ebp-0x14],eax
000094D7  8B45F0            mov eax,[ebp-0x10]
000094DA  83C001            add eax,byte +0x1
000094DD  C1E006            shl eax,byte 0x6
000094E0  8D9050090000      lea edx,[eax+0x950]
000094E6  8B4508            mov eax,[ebp+0x8]
000094E9  01D0              add eax,edx
000094EB  8945E8            mov [ebp-0x18],eax
000094EE  EB17              jmp short 0x9507
000094F0  8B55E8            mov edx,[ebp-0x18]
000094F3  8D4201            lea eax,[edx+0x1]
000094F6  8945E8            mov [ebp-0x18],eax
000094F9  8B45EC            mov eax,[ebp-0x14]
000094FC  8D4801            lea ecx,[eax+0x1]
000094FF  894DEC            mov [ebp-0x14],ecx
00009502  0FB612            movzx edx,byte [edx]
00009505  8810              mov [eax],dl
00009507  8B45E8            mov eax,[ebp-0x18]
0000950A  0FB600            movzx eax,byte [eax]
0000950D  84C0              test al,al
0000950F  75DF              jnz 0x94f0
00009511  8B45EC            mov eax,[ebp-0x14]
00009514  C60000            mov byte [eax],0x0
00009517  8345F001          add dword [ebp-0x10],byte +0x1
0000951B  837DF006          cmp dword [ebp-0x10],byte +0x6
0000951F  7EA2              jng 0x94c3
00009521  C745E400000000    mov dword [ebp-0x1c],0x0
00009528  EB24              jmp short 0x954e
0000952A  8B5508            mov edx,[ebp+0x8]
0000952D  8B45E4            mov eax,[ebp-0x1c]
00009530  01D0              add eax,edx
00009532  054C080000        add eax,0x84c
00009537  0FB600            movzx eax,byte [eax]
0000953A  8B4D08            mov ecx,[ebp+0x8]
0000953D  8B55E4            mov edx,[ebp-0x1c]
00009540  01CA              add edx,ecx
00009542  81C2100B0000      add edx,0xb10
00009548  8802              mov [edx],al
0000954A  8345E401          add dword [ebp-0x1c],byte +0x1
0000954E  8B5508            mov edx,[ebp+0x8]
00009551  8B45E4            mov eax,[ebp-0x1c]
00009554  01D0              add eax,edx
00009556  054C080000        add eax,0x84c
0000955B  0FB600            movzx eax,byte [eax]
0000955E  84C0              test al,al
00009560  7406              jz 0x9568
00009562  837DE43E          cmp dword [ebp-0x1c],byte +0x3e
00009566  7EC2              jng 0x952a
00009568  8B5508            mov edx,[ebp+0x8]
0000956B  8B45E4            mov eax,[ebp-0x1c]
0000956E  01D0              add eax,edx
00009570  05100B0000        add eax,0xb10
00009575  C60000            mov byte [eax],0x0
00009578  8B4508            mov eax,[ebp+0x8]
0000957B  8B90500B0000      mov edx,[eax+0xb50]
00009581  8B4508            mov eax,[ebp+0x8]
00009584  8990540B0000      mov [eax+0xb54],edx
0000958A  8B4508            mov eax,[ebp+0x8]
0000958D  054C080000        add eax,0x84c
00009592  83EC08            sub esp,byte +0x8
00009595  50                push eax
00009596  FF7508            push dword [ebp+0x8]
00009599  E8B0BDFFFF        call 0x534e
0000959E  83C410            add esp,byte +0x10
000095A1  83EC08            sub esp,byte +0x8
000095A4  68FEC20100        push dword 0x1c2fe
000095A9  FF7508            push dword [ebp+0x8]
000095AC  E898FDFFFF        call 0x9349
000095B1  83C410            add esp,byte +0x10
000095B4  83EC0C            sub esp,byte +0xc
000095B7  8D45A4            lea eax,[ebp-0x5c]
000095BA  50                push eax
000095BB  E8A376FFFF        call 0xc63
000095C0  83C410            add esp,byte +0x10
000095C3  83EC08            sub esp,byte +0x8
000095C6  8D45A4            lea eax,[ebp-0x5c]
000095C9  50                push eax
000095CA  FF7508            push dword [ebp+0x8]
000095CD  E877FDFFFF        call 0x9349
000095D2  83C410            add esp,byte +0x10
000095D5  83EC08            sub esp,byte +0x8
000095D8  6807C30100        push dword 0x1c307
000095DD  FF7508            push dword [ebp+0x8]
000095E0  E864FDFFFF        call 0x9349
000095E5  83C410            add esp,byte +0x10
000095E8  8B4508            mov eax,[ebp+0x8]
000095EB  C7804C0900000000  mov dword [eax+0x94c],0x0
         -0000
000095F5  8B4508            mov eax,[ebp+0x8]
000095F8  C6804C08000000    mov byte [eax+0x84c],0x0
000095FF  EB01              jmp short 0x9602
00009601  90                nop
00009602  C9                leave
00009603  C3                ret
00009604  55                push ebp
00009605  89E5              mov ebp,esp
00009607  83EC38            sub esp,byte +0x38
0000960A  8B4508            mov eax,[ebp+0x8]
0000960D  8B4008            mov eax,[eax+0x8]
00009610  83C002            add eax,byte +0x2
00009613  8945E8            mov [ebp-0x18],eax
00009616  8B4508            mov eax,[ebp+0x8]
00009619  8B400C            mov eax,[eax+0xc]
0000961C  83C014            add eax,byte +0x14
0000961F  8945E4            mov [ebp-0x1c],eax
00009622  8B4508            mov eax,[ebp+0x8]
00009625  8B4010            mov eax,[eax+0x10]
00009628  83E804            sub eax,byte +0x4
0000962B  8945E0            mov [ebp-0x20],eax
0000962E  8B4508            mov eax,[ebp+0x8]
00009631  8B4014            mov eax,[eax+0x14]
00009634  83E816            sub eax,byte +0x16
00009637  8945DC            mov [ebp-0x24],eax
0000963A  83EC0C            sub esp,byte +0xc
0000963D  6A00              push byte +0x0
0000963F  FF75DC            push dword [ebp-0x24]
00009642  FF75E0            push dword [ebp-0x20]
00009645  FF75E4            push dword [ebp-0x1c]
00009648  FF75E8            push dword [ebp-0x18]
0000964B  E8FD9FFFFF        call 0x364d
00009650  83C420            add esp,byte +0x20
00009653  8B4508            mov eax,[ebp+0x8]
00009656  8D484C            lea ecx,[eax+0x4c]
00009659  8B45E4            mov eax,[ebp-0x1c]
0000965C  8D5004            lea edx,[eax+0x4]
0000965F  8B45E8            mov eax,[ebp-0x18]
00009662  83C004            add eax,byte +0x4
00009665  83EC0C            sub esp,byte +0xc
00009668  68FFFFFF00        push dword 0xffffff
0000966D  51                push ecx
0000966E  52                push edx
0000966F  50                push eax
00009670  FF7508            push dword [ebp+0x8]
00009673  E8A2F3FFFF        call 0x8a1a
00009678  83C420            add esp,byte +0x20
0000967B  C745F400000000    mov dword [ebp-0xc],0x0
00009682  8B4508            mov eax,[ebp+0x8]
00009685  83C04C            add eax,byte +0x4c
00009688  8945F0            mov [ebp-0x10],eax
0000968B  8B45F0            mov eax,[ebp-0x10]
0000968E  8945EC            mov [ebp-0x14],eax
00009691  EB1B              jmp short 0x96ae
00009693  8B45F0            mov eax,[ebp-0x10]
00009696  0FB600            movzx eax,byte [eax]
00009699  3C0A              cmp al,0xa
0000969B  750D              jnz 0x96aa
0000969D  8345F401          add dword [ebp-0xc],byte +0x1
000096A1  8B45F0            mov eax,[ebp-0x10]
000096A4  83C001            add eax,byte +0x1
000096A7  8945EC            mov [ebp-0x14],eax
000096AA  8345F001          add dword [ebp-0x10],byte +0x1
000096AE  8B45F0            mov eax,[ebp-0x10]
000096B1  0FB600            movzx eax,byte [eax]
000096B4  84C0              test al,al
000096B6  75DB              jnz 0x9693
000096B8  8B55F0            mov edx,[ebp-0x10]
000096BB  8B45EC            mov eax,[ebp-0x14]
000096BE  29C2              sub edx,eax
000096C0  89D0              mov eax,edx
000096C2  8945D8            mov [ebp-0x28],eax
000096C5  8B45E4            mov eax,[ebp-0x1c]
000096C8  8D5004            lea edx,[eax+0x4]
000096CB  8B45F4            mov eax,[ebp-0xc]
000096CE  C1E004            shl eax,byte 0x4
000096D1  01D0              add eax,edx
000096D3  8945D4            mov [ebp-0x2c],eax
000096D6  8B45E8            mov eax,[ebp-0x18]
000096D9  8D4804            lea ecx,[eax+0x4]
000096DC  8B55D8            mov edx,[ebp-0x28]
000096DF  89D0              mov eax,edx
000096E1  C1E002            shl eax,byte 0x2
000096E4  01D0              add eax,edx
000096E6  01C0              add eax,eax
000096E8  01C8              add eax,ecx
000096EA  8945D0            mov [ebp-0x30],eax
000096ED  8B4508            mov eax,[ebp+0x8]
000096F0  054C080000        add eax,0x84c
000096F5  83EC0C            sub esp,byte +0xc
000096F8  68FFFFFF00        push dword 0xffffff
000096FD  50                push eax
000096FE  FF75D4            push dword [ebp-0x2c]
00009701  FF75D0            push dword [ebp-0x30]
00009704  FF7508            push dword [ebp+0x8]
00009707  E80EF3FFFF        call 0x8a1a
0000970C  83C420            add esp,byte +0x20
0000970F  A160280200        mov eax,[0x22860]
00009714  394508            cmp [ebp+0x8],eax
00009717  7547              jnz 0x9760
00009719  8B4508            mov eax,[ebp+0x8]
0000971C  054C080000        add eax,0x84c
00009721  83EC0C            sub esp,byte +0xc
00009724  50                push eax
00009725  E8D9EDFFFF        call 0x8503
0000972A  83C410            add esp,byte +0x10
0000972D  8945CC            mov [ebp-0x34],eax
00009730  8B55CC            mov edx,[ebp-0x34]
00009733  89D0              mov eax,edx
00009735  C1E002            shl eax,byte 0x2
00009738  01D0              add eax,edx
0000973A  01C0              add eax,eax
0000973C  89C2              mov edx,eax
0000973E  8B45D0            mov eax,[ebp-0x30]
00009741  01D0              add eax,edx
00009743  8945C8            mov [ebp-0x38],eax
00009746  83EC0C            sub esp,byte +0xc
00009749  68FFFFFF00        push dword 0xffffff
0000974E  6A0C              push byte +0xc
00009750  6A08              push byte +0x8
00009752  FF75D4            push dword [ebp-0x2c]
00009755  FF75C8            push dword [ebp-0x38]
00009758  E8F09EFFFF        call 0x364d
0000975D  83C420            add esp,byte +0x20
00009760  90                nop
00009761  C9                leave
00009762  C3                ret
00009763  55                push ebp
00009764  89E5              mov ebp,esp
00009766  83EC78            sub esp,byte +0x78
00009769  8B4508            mov eax,[ebp+0x8]
0000976C  8B4008            mov eax,[eax+0x8]
0000976F  83C014            add eax,byte +0x14
00009772  8945EC            mov [ebp-0x14],eax
00009775  8B4508            mov eax,[ebp+0x8]
00009778  8B400C            mov eax,[eax+0xc]
0000977B  83C018            add eax,byte +0x18
0000977E  8945E8            mov [ebp-0x18],eax
00009781  83EC0C            sub esp,byte +0xc
00009784  68FFFFFF00        push dword 0xffffff
00009789  6A20              push byte +0x20
0000978B  68C8000000        push dword 0xc8
00009790  FF75E8            push dword [ebp-0x18]
00009793  FF75EC            push dword [ebp-0x14]
00009796  E8B29EFFFF        call 0x364d
0000979B  83C420            add esp,byte +0x20
0000979E  8B4508            mov eax,[ebp+0x8]
000097A1  8D88C00C0000      lea ecx,[eax+0xcc0]
000097A7  8B45E8            mov eax,[ebp-0x18]
000097AA  8D5008            lea edx,[eax+0x8]
000097AD  8B45EC            mov eax,[ebp-0x14]
000097B0  83C008            add eax,byte +0x8
000097B3  83EC0C            sub esp,byte +0xc
000097B6  6A00              push byte +0x0
000097B8  51                push ecx
000097B9  52                push edx
000097BA  50                push eax
000097BB  FF7508            push dword [ebp+0x8]
000097BE  E857F2FFFF        call 0x8a1a
000097C3  83C420            add esp,byte +0x20
000097C6  8B4508            mov eax,[ebp+0x8]
000097C9  8B4008            mov eax,[eax+0x8]
000097CC  83C01C            add eax,byte +0x1c
000097CF  8945E4            mov [ebp-0x1c],eax
000097D2  C745880AC30100    mov dword [ebp-0x78],0x1c30a
000097D9  C7458C0CC30100    mov dword [ebp-0x74],0x1c30c
000097E0  C745900EC30100    mov dword [ebp-0x70],0x1c30e
000097E7  C745949DC10100    mov dword [ebp-0x6c],0x1c19d
000097EE  C7459810C30100    mov dword [ebp-0x68],0x1c310
000097F5  C7459C12C30100    mov dword [ebp-0x64],0x1c312
000097FC  C745A014C30100    mov dword [ebp-0x60],0x1c314
00009803  C745A416C30100    mov dword [ebp-0x5c],0x1c316
0000980A  C745A818C30100    mov dword [ebp-0x58],0x1c318
00009811  C745AC1AC30100    mov dword [ebp-0x54],0x1c31a
00009818  C745B01CC30100    mov dword [ebp-0x50],0x1c31c
0000981F  C745B41EC30100    mov dword [ebp-0x4c],0x1c31e
00009826  C745B820C30100    mov dword [ebp-0x48],0x1c320
0000982D  C745BC22C30100    mov dword [ebp-0x44],0x1c322
00009834  C745C024C30100    mov dword [ebp-0x40],0x1c324
0000983B  C745C426C30100    mov dword [ebp-0x3c],0x1c326
00009842  C745E028000000    mov dword [ebp-0x20],0x28
00009849  C745DC28000000    mov dword [ebp-0x24],0x28
00009850  C745D808000000    mov dword [ebp-0x28],0x8
00009857  8B45E8            mov eax,[ebp-0x18]
0000985A  83C028            add eax,byte +0x28
0000985D  8945D4            mov [ebp-0x2c],eax
00009860  C745F400000000    mov dword [ebp-0xc],0x0
00009867  E91C010000        jmp 0x9988
0000986C  C745F000000000    mov dword [ebp-0x10],0x0
00009873  E902010000        jmp 0x997a
00009878  8B55E0            mov edx,[ebp-0x20]
0000987B  8B45D8            mov eax,[ebp-0x28]
0000987E  01D0              add eax,edx
00009880  0FAF45F0          imul eax,[ebp-0x10]
00009884  89C2              mov edx,eax
00009886  8B45E4            mov eax,[ebp-0x1c]
00009889  01D0              add eax,edx
0000988B  8945D0            mov [ebp-0x30],eax
0000988E  8B55DC            mov edx,[ebp-0x24]
00009891  8B45D8            mov eax,[ebp-0x28]
00009894  01D0              add eax,edx
00009896  0FAF45F4          imul eax,[ebp-0xc]
0000989A  89C2              mov edx,eax
0000989C  8B45D4            mov eax,[ebp-0x2c]
0000989F  01D0              add eax,edx
000098A1  8945CC            mov [ebp-0x34],eax
000098A4  83EC0C            sub esp,byte +0xc
000098A7  68C0C0C000        push dword 0xc0c0c0
000098AC  FF75DC            push dword [ebp-0x24]
000098AF  FF75E0            push dword [ebp-0x20]
000098B2  FF75CC            push dword [ebp-0x34]
000098B5  FF75D0            push dword [ebp-0x30]
000098B8  E8909DFFFF        call 0x364d
000098BD  83C420            add esp,byte +0x20
000098C0  83EC0C            sub esp,byte +0xc
000098C3  68FFFFFF00        push dword 0xffffff
000098C8  6A02              push byte +0x2
000098CA  FF75E0            push dword [ebp-0x20]
000098CD  FF75CC            push dword [ebp-0x34]
000098D0  FF75D0            push dword [ebp-0x30]
000098D3  E8759DFFFF        call 0x364d
000098D8  83C420            add esp,byte +0x20
000098DB  83EC0C            sub esp,byte +0xc
000098DE  68FFFFFF00        push dword 0xffffff
000098E3  FF75DC            push dword [ebp-0x24]
000098E6  6A02              push byte +0x2
000098E8  FF75CC            push dword [ebp-0x34]
000098EB  FF75D0            push dword [ebp-0x30]
000098EE  E85A9DFFFF        call 0x364d
000098F3  83C420            add esp,byte +0x20
000098F6  8B55D0            mov edx,[ebp-0x30]
000098F9  8B45E0            mov eax,[ebp-0x20]
000098FC  01D0              add eax,edx
000098FE  83E802            sub eax,byte +0x2
00009901  83EC0C            sub esp,byte +0xc
00009904  6840404000        push dword 0x404040
00009909  FF75DC            push dword [ebp-0x24]
0000990C  6A02              push byte +0x2
0000990E  FF75CC            push dword [ebp-0x34]
00009911  50                push eax
00009912  E8369DFFFF        call 0x364d
00009917  83C420            add esp,byte +0x20
0000991A  8B55CC            mov edx,[ebp-0x34]
0000991D  8B45DC            mov eax,[ebp-0x24]
00009920  01D0              add eax,edx
00009922  83E802            sub eax,byte +0x2
00009925  83EC0C            sub esp,byte +0xc
00009928  6840404000        push dword 0x404040
0000992D  6A02              push byte +0x2
0000992F  FF75E0            push dword [ebp-0x20]
00009932  50                push eax
00009933  FF75D0            push dword [ebp-0x30]
00009936  E8129DFFFF        call 0x364d
0000993B  83C420            add esp,byte +0x20
0000993E  8B45F4            mov eax,[ebp-0xc]
00009941  8D148500000000    lea edx,[eax*4+0x0]
00009948  8B45F0            mov eax,[ebp-0x10]
0000994B  01D0              add eax,edx
0000994D  8945C8            mov [ebp-0x38],eax
00009950  8B45C8            mov eax,[ebp-0x38]
00009953  8B448588          mov eax,[ebp+eax*4-0x78]
00009957  0FB600            movzx eax,byte [eax]
0000995A  0FBEC0            movsx eax,al
0000995D  8B55CC            mov edx,[ebp-0x34]
00009960  8D4A0C            lea ecx,[edx+0xc]
00009963  8B55D0            mov edx,[ebp-0x30]
00009966  83C20F            add edx,byte +0xf
00009969  6A00              push byte +0x0
0000996B  50                push eax
0000996C  51                push ecx
0000996D  52                push edx
0000996E  E8F7AEFFFF        call 0x486a
00009973  83C410            add esp,byte +0x10
00009976  8345F001          add dword [ebp-0x10],byte +0x1
0000997A  837DF003          cmp dword [ebp-0x10],byte +0x3
0000997E  0F8EF4FEFFFF      jng near 0x9878
00009984  8345F401          add dword [ebp-0xc],byte +0x1
00009988  837DF403          cmp dword [ebp-0xc],byte +0x3
0000998C  0F8EDAFEFFFF      jng near 0x986c
00009992  90                nop
00009993  C9                leave
00009994  C3                ret
00009995  55                push ebp
00009996  89E5              mov ebp,esp
00009998  83EC38            sub esp,byte +0x38
0000999B  8B4508            mov eax,[ebp+0x8]
0000999E  8845D4            mov [ebp-0x2c],al
000099A1  A160280200        mov eax,[0x22860]
000099A6  85C0              test eax,eax
000099A8  0F84F9020000      jz near 0x9ca7
000099AE  A160280200        mov eax,[0x22860]
000099B3  8945EC            mov [ebp-0x14],eax
000099B6  8B45EC            mov eax,[ebp-0x14]
000099B9  8B4048            mov eax,[eax+0x48]
000099BC  83F801            cmp eax,byte +0x1
000099BF  0F855B020000      jnz near 0x9c20
000099C5  807DD408          cmp byte [ebp-0x2c],0x8
000099C9  7544              jnz 0x9a0f
000099CB  8B45EC            mov eax,[ebp-0x14]
000099CE  8B804C090000      mov eax,[eax+0x94c]
000099D4  85C0              test eax,eax
000099D6  0F8ECC020000      jng near 0x9ca8
000099DC  8B45EC            mov eax,[ebp-0x14]
000099DF  8B804C090000      mov eax,[eax+0x94c]
000099E5  8D50FF            lea edx,[eax-0x1]
000099E8  8B45EC            mov eax,[ebp-0x14]
000099EB  89904C090000      mov [eax+0x94c],edx
000099F1  8B45EC            mov eax,[ebp-0x14]
000099F4  8B804C090000      mov eax,[eax+0x94c]
000099FA  8B55EC            mov edx,[ebp-0x14]
000099FD  C684024C08000000  mov byte [edx+eax+0x84c],0x0
00009A05  E8CE050000        call 0x9fd8
00009A0A  E999020000        jmp 0x9ca8
00009A0F  807DD40A          cmp byte [ebp-0x2c],0xa
00009A13  7406              jz 0x9a1b
00009A15  807DD40D          cmp byte [ebp-0x2c],0xd
00009A19  7518              jnz 0x9a33
00009A1B  83EC0C            sub esp,byte +0xc
00009A1E  FF75EC            push dword [ebp-0x14]
00009A21  E8A8F9FFFF        call 0x93ce
00009A26  83C410            add esp,byte +0x10
00009A29  E8AA050000        call 0x9fd8
00009A2E  E975020000        jmp 0x9ca8
00009A33  807DD480          cmp byte [ebp-0x2c],0x80
00009A37  0F85A6000000      jnz near 0x9ae3
00009A3D  8B45EC            mov eax,[ebp-0x14]
00009A40  8B80540B0000      mov eax,[eax+0xb54]
00009A46  85C0              test eax,eax
00009A48  0F8E5A020000      jng near 0x9ca8
00009A4E  8B45EC            mov eax,[ebp-0x14]
00009A51  8B80540B0000      mov eax,[eax+0xb54]
00009A57  8D50FF            lea edx,[eax-0x1]
00009A5A  8B45EC            mov eax,[ebp-0x14]
00009A5D  8990540B0000      mov [eax+0xb54],edx
00009A63  8B45EC            mov eax,[ebp-0x14]
00009A66  8B80540B0000      mov eax,[eax+0xb54]
00009A6C  C1E006            shl eax,byte 0x6
00009A6F  8D9050090000      lea edx,[eax+0x950]
00009A75  8B45EC            mov eax,[ebp-0x14]
00009A78  01D0              add eax,edx
00009A7A  8945E4            mov [ebp-0x1c],eax
00009A7D  C745F400000000    mov dword [ebp-0xc],0x0
00009A84  EB1F              jmp short 0x9aa5
00009A86  8B55F4            mov edx,[ebp-0xc]
00009A89  8B45E4            mov eax,[ebp-0x1c]
00009A8C  01D0              add eax,edx
00009A8E  0FB600            movzx eax,byte [eax]
00009A91  8B4DEC            mov ecx,[ebp-0x14]
00009A94  8B55F4            mov edx,[ebp-0xc]
00009A97  01CA              add edx,ecx
00009A99  81C24C080000      add edx,0x84c
00009A9F  8802              mov [edx],al
00009AA1  8345F401          add dword [ebp-0xc],byte +0x1
00009AA5  8B55F4            mov edx,[ebp-0xc]
00009AA8  8B45E4            mov eax,[ebp-0x1c]
00009AAB  01D0              add eax,edx
00009AAD  0FB600            movzx eax,byte [eax]
00009AB0  84C0              test al,al
00009AB2  7409              jz 0x9abd
00009AB4  817DF4FE000000    cmp dword [ebp-0xc],0xfe
00009ABB  7EC9              jng 0x9a86
00009ABD  8B55EC            mov edx,[ebp-0x14]
00009AC0  8B45F4            mov eax,[ebp-0xc]
00009AC3  01D0              add eax,edx
00009AC5  054C080000        add eax,0x84c
00009ACA  C60000            mov byte [eax],0x0
00009ACD  8B45EC            mov eax,[ebp-0x14]
00009AD0  8B55F4            mov edx,[ebp-0xc]
00009AD3  89904C090000      mov [eax+0x94c],edx
00009AD9  E8FA040000        call 0x9fd8
00009ADE  E9C5010000        jmp 0x9ca8
00009AE3  807DD481          cmp byte [ebp-0x2c],0x81
00009AE7  0F85DE000000      jnz near 0x9bcb
00009AED  8B45EC            mov eax,[ebp-0x14]
00009AF0  8B90540B0000      mov edx,[eax+0xb54]
00009AF6  8B45EC            mov eax,[ebp-0x14]
00009AF9  8B80500B0000      mov eax,[eax+0xb50]
00009AFF  39C2              cmp edx,eax
00009B01  0F8DA1010000      jnl near 0x9ca8
00009B07  8B45EC            mov eax,[ebp-0x14]
00009B0A  8B80540B0000      mov eax,[eax+0xb54]
00009B10  8D5001            lea edx,[eax+0x1]
00009B13  8B45EC            mov eax,[ebp-0x14]
00009B16  8990540B0000      mov [eax+0xb54],edx
00009B1C  8B45EC            mov eax,[ebp-0x14]
00009B1F  8B90540B0000      mov edx,[eax+0xb54]
00009B25  8B45EC            mov eax,[ebp-0x14]
00009B28  8B80500B0000      mov eax,[eax+0xb50]
00009B2E  39C2              cmp edx,eax
00009B30  7519              jnz 0x9b4b
00009B32  8B45EC            mov eax,[ebp-0x14]
00009B35  C6804C08000000    mov byte [eax+0x84c],0x0
00009B3C  8B45EC            mov eax,[ebp-0x14]
00009B3F  C7804C0900000000  mov dword [eax+0x94c],0x0
         -0000
00009B49  EB76              jmp short 0x9bc1
00009B4B  8B45EC            mov eax,[ebp-0x14]
00009B4E  8B80540B0000      mov eax,[eax+0xb54]
00009B54  C1E006            shl eax,byte 0x6
00009B57  8D9050090000      lea edx,[eax+0x950]
00009B5D  8B45EC            mov eax,[ebp-0x14]
00009B60  01D0              add eax,edx
00009B62  8945E8            mov [ebp-0x18],eax
00009B65  C745F000000000    mov dword [ebp-0x10],0x0
00009B6C  EB1F              jmp short 0x9b8d
00009B6E  8B55F0            mov edx,[ebp-0x10]
00009B71  8B45E8            mov eax,[ebp-0x18]
00009B74  01D0              add eax,edx
00009B76  0FB600            movzx eax,byte [eax]
00009B79  8B4DEC            mov ecx,[ebp-0x14]
00009B7C  8B55F0            mov edx,[ebp-0x10]
00009B7F  01CA              add edx,ecx
00009B81  81C24C080000      add edx,0x84c
00009B87  8802              mov [edx],al
00009B89  8345F001          add dword [ebp-0x10],byte +0x1
00009B8D  8B55F0            mov edx,[ebp-0x10]
00009B90  8B45E8            mov eax,[ebp-0x18]
00009B93  01D0              add eax,edx
00009B95  0FB600            movzx eax,byte [eax]
00009B98  84C0              test al,al
00009B9A  7409              jz 0x9ba5
00009B9C  817DF0FE000000    cmp dword [ebp-0x10],0xfe
00009BA3  7EC9              jng 0x9b6e
00009BA5  8B55EC            mov edx,[ebp-0x14]
00009BA8  8B45F0            mov eax,[ebp-0x10]
00009BAB  01D0              add eax,edx
00009BAD  054C080000        add eax,0x84c
00009BB2  C60000            mov byte [eax],0x0
00009BB5  8B45EC            mov eax,[ebp-0x14]
00009BB8  8B55F0            mov edx,[ebp-0x10]
00009BBB  89904C090000      mov [eax+0x94c],edx
00009BC1  E812040000        call 0x9fd8
00009BC6  E9DD000000        jmp 0x9ca8
00009BCB  8B45EC            mov eax,[ebp-0x14]
00009BCE  8B804C090000      mov eax,[eax+0x94c]
00009BD4  3DFE000000        cmp eax,0xfe
00009BD9  0F8FC9000000      jg near 0x9ca8
00009BDF  8B45EC            mov eax,[ebp-0x14]
00009BE2  8B804C090000      mov eax,[eax+0x94c]
00009BE8  8D4801            lea ecx,[eax+0x1]
00009BEB  8B55EC            mov edx,[ebp-0x14]
00009BEE  898A4C090000      mov [edx+0x94c],ecx
00009BF4  8B55EC            mov edx,[ebp-0x14]
00009BF7  0FB64DD4          movzx ecx,byte [ebp-0x2c]
00009BFB  888C024C080000    mov [edx+eax+0x84c],cl
00009C02  8B45EC            mov eax,[ebp-0x14]
00009C05  8B804C090000      mov eax,[eax+0x94c]
00009C0B  8B55EC            mov edx,[ebp-0x14]
00009C0E  C684024C08000000  mov byte [edx+eax+0x84c],0x0
00009C16  E8BD030000        call 0x9fd8
00009C1B  E988000000        jmp 0x9ca8
00009C20  8B45EC            mov eax,[ebp-0x14]
00009C23  8B4048            mov eax,[eax+0x48]
00009C26  83F803            cmp eax,byte +0x3
00009C29  751A              jnz 0x9c45
00009C2B  0FBE45D4          movsx eax,byte [ebp-0x2c]
00009C2F  83EC08            sub esp,byte +0x8
00009C32  50                push eax
00009C33  FF75EC            push dword [ebp-0x14]
00009C36  E8C2E0FFFF        call 0x7cfd
00009C3B  83C410            add esp,byte +0x10
00009C3E  E895030000        call 0x9fd8
00009C43  EB63              jmp short 0x9ca8
00009C45  8B45EC            mov eax,[ebp-0x14]
00009C48  8B4048            mov eax,[eax+0x48]
00009C4B  83F804            cmp eax,byte +0x4
00009C4E  7558              jnz 0x9ca8
00009C50  807DD42F          cmp byte [ebp-0x2c],0x2f
00009C54  7E06              jng 0x9c5c
00009C56  807DD439          cmp byte [ebp-0x2c],0x39
00009C5A  7E36              jng 0x9c92
00009C5C  807DD42B          cmp byte [ebp-0x2c],0x2b
00009C60  7430              jz 0x9c92
00009C62  807DD42D          cmp byte [ebp-0x2c],0x2d
00009C66  742A              jz 0x9c92
00009C68  807DD42A          cmp byte [ebp-0x2c],0x2a
00009C6C  7424              jz 0x9c92
00009C6E  807DD42F          cmp byte [ebp-0x2c],0x2f
00009C72  741E              jz 0x9c92
00009C74  807DD43D          cmp byte [ebp-0x2c],0x3d
00009C78  7418              jz 0x9c92
00009C7A  807DD40A          cmp byte [ebp-0x2c],0xa
00009C7E  7412              jz 0x9c92
00009C80  807DD463          cmp byte [ebp-0x2c],0x63
00009C84  740C              jz 0x9c92
00009C86  807DD443          cmp byte [ebp-0x2c],0x43
00009C8A  7406              jz 0x9c92
00009C8C  807DD408          cmp byte [ebp-0x2c],0x8
00009C90  7516              jnz 0x9ca8
00009C92  0FBE45D4          movsx eax,byte [ebp-0x2c]
00009C96  83EC08            sub esp,byte +0x8
00009C99  50                push eax
00009C9A  FF75EC            push dword [ebp-0x14]
00009C9D  E81A090000        call 0xa5bc
00009CA2  83C410            add esp,byte +0x10
00009CA5  EB01              jmp short 0x9ca8
00009CA7  90                nop
00009CA8  C9                leave
00009CA9  C3                ret
00009CAA  55                push ebp
00009CAB  89E5              mov ebp,esp
00009CAD  53                push ebx
00009CAE  83EC14            sub esp,byte +0x14
00009CB1  8B4508            mov eax,[ebp+0x8]
00009CB4  8B4044            mov eax,[eax+0x44]
00009CB7  85C0              test eax,eax
00009CB9  0F847D020000      jz near 0x9f3c
00009CBF  8B4508            mov eax,[ebp+0x8]
00009CC2  8B5814            mov ebx,[eax+0x14]
00009CC5  8B4508            mov eax,[ebp+0x8]
00009CC8  8B4810            mov ecx,[eax+0x10]
00009CCB  8B4508            mov eax,[ebp+0x8]
00009CCE  8B500C            mov edx,[eax+0xc]
00009CD1  8B4508            mov eax,[ebp+0x8]
00009CD4  8B4008            mov eax,[eax+0x8]
00009CD7  83EC0C            sub esp,byte +0xc
00009CDA  68C0C0C000        push dword 0xc0c0c0
00009CDF  53                push ebx
00009CE0  51                push ecx
00009CE1  52                push edx
00009CE2  50                push eax
00009CE3  E86599FFFF        call 0x364d
00009CE8  83C420            add esp,byte +0x20
00009CEB  8B4508            mov eax,[ebp+0x8]
00009CEE  8B4810            mov ecx,[eax+0x10]
00009CF1  8B4508            mov eax,[ebp+0x8]
00009CF4  8B500C            mov edx,[eax+0xc]
00009CF7  8B4508            mov eax,[ebp+0x8]
00009CFA  8B4008            mov eax,[eax+0x8]
00009CFD  83EC0C            sub esp,byte +0xc
00009D00  6A00              push byte +0x0
00009D02  6A02              push byte +0x2
00009D04  51                push ecx
00009D05  52                push edx
00009D06  50                push eax
00009D07  E84199FFFF        call 0x364d
00009D0C  83C420            add esp,byte +0x20
00009D0F  8B4508            mov eax,[ebp+0x8]
00009D12  8B5010            mov edx,[eax+0x10]
00009D15  8B4508            mov eax,[ebp+0x8]
00009D18  8B480C            mov ecx,[eax+0xc]
00009D1B  8B4508            mov eax,[ebp+0x8]
00009D1E  8B4014            mov eax,[eax+0x14]
00009D21  01C8              add eax,ecx
00009D23  8D48FE            lea ecx,[eax-0x2]
00009D26  8B4508            mov eax,[ebp+0x8]
00009D29  8B4008            mov eax,[eax+0x8]
00009D2C  83EC0C            sub esp,byte +0xc
00009D2F  6A00              push byte +0x0
00009D31  6A02              push byte +0x2
00009D33  52                push edx
00009D34  51                push ecx
00009D35  50                push eax
00009D36  E81299FFFF        call 0x364d
00009D3B  83C420            add esp,byte +0x20
00009D3E  8B4508            mov eax,[ebp+0x8]
00009D41  8B4814            mov ecx,[eax+0x14]
00009D44  8B4508            mov eax,[ebp+0x8]
00009D47  8B500C            mov edx,[eax+0xc]
00009D4A  8B4508            mov eax,[ebp+0x8]
00009D4D  8B4008            mov eax,[eax+0x8]
00009D50  83EC0C            sub esp,byte +0xc
00009D53  6A00              push byte +0x0
00009D55  51                push ecx
00009D56  6A02              push byte +0x2
00009D58  52                push edx
00009D59  50                push eax
00009D5A  E8EE98FFFF        call 0x364d
00009D5F  83C420            add esp,byte +0x20
00009D62  8B4508            mov eax,[ebp+0x8]
00009D65  8B5014            mov edx,[eax+0x14]
00009D68  8B4508            mov eax,[ebp+0x8]
00009D6B  8B400C            mov eax,[eax+0xc]
00009D6E  8B4D08            mov ecx,[ebp+0x8]
00009D71  8B5908            mov ebx,[ecx+0x8]
00009D74  8B4D08            mov ecx,[ebp+0x8]
00009D77  8B4910            mov ecx,[ecx+0x10]
00009D7A  01D9              add ecx,ebx
00009D7C  83E902            sub ecx,byte +0x2
00009D7F  83EC0C            sub esp,byte +0xc
00009D82  6A00              push byte +0x0
00009D84  52                push edx
00009D85  6A02              push byte +0x2
00009D87  50                push eax
00009D88  51                push ecx
00009D89  E8BF98FFFF        call 0x364d
00009D8E  83C420            add esp,byte +0x20
00009D91  A160280200        mov eax,[0x22860]
00009D96  394508            cmp [ebp+0x8],eax
00009D99  7518              jnz 0x9db3
00009D9B  8B4508            mov eax,[ebp+0x8]
00009D9E  8B4038            mov eax,[eax+0x38]
00009DA1  85C0              test eax,eax
00009DA3  7407              jz 0x9dac
00009DA5  B840000000        mov eax,0x40
00009DAA  EB0C              jmp short 0x9db8
00009DAC  B880000000        mov eax,0x80
00009DB1  EB05              jmp short 0x9db8
00009DB3  B840404000        mov eax,0x404040
00009DB8  8945F4            mov [ebp-0xc],eax
00009DBB  8B4508            mov eax,[ebp+0x8]
00009DBE  8B4010            mov eax,[eax+0x10]
00009DC1  8D48FC            lea ecx,[eax-0x4]
00009DC4  8B4508            mov eax,[ebp+0x8]
00009DC7  8B400C            mov eax,[eax+0xc]
00009DCA  8D5002            lea edx,[eax+0x2]
00009DCD  8B4508            mov eax,[ebp+0x8]
00009DD0  8B4008            mov eax,[eax+0x8]
00009DD3  83C002            add eax,byte +0x2
00009DD6  83EC0C            sub esp,byte +0xc
00009DD9  FF75F4            push dword [ebp-0xc]
00009DDC  6A1E              push byte +0x1e
00009DDE  51                push ecx
00009DDF  52                push edx
00009DE0  50                push eax
00009DE1  E86798FFFF        call 0x364d
00009DE6  83C420            add esp,byte +0x20
00009DE9  8B4508            mov eax,[ebp+0x8]
00009DEC  8B400C            mov eax,[eax+0xc]
00009DEF  8D5008            lea edx,[eax+0x8]
00009DF2  8B4508            mov eax,[ebp+0x8]
00009DF5  8B4008            mov eax,[eax+0x8]
00009DF8  83C006            add eax,byte +0x6
00009DFB  68FFFFFF00        push dword 0xffffff
00009E00  6A20              push byte +0x20
00009E02  52                push edx
00009E03  50                push eax
00009E04  E861AAFFFF        call 0x486a
00009E09  83C410            add esp,byte +0x10
00009E0C  8B4508            mov eax,[ebp+0x8]
00009E0F  8D4818            lea ecx,[eax+0x18]
00009E12  8B4508            mov eax,[ebp+0x8]
00009E15  8B400C            mov eax,[eax+0xc]
00009E18  8D5008            lea edx,[eax+0x8]
00009E1B  8B4508            mov eax,[ebp+0x8]
00009E1E  8B4008            mov eax,[eax+0x8]
00009E21  83C00E            add eax,byte +0xe
00009E24  68FFFFFF00        push dword 0xffffff
00009E29  51                push ecx
00009E2A  52                push edx
00009E2B  50                push eax
00009E2C  E880ABFFFF        call 0x49b1
00009E31  83C410            add esp,byte +0x10
00009E34  C745F014000000    mov dword [ebp-0x10],0x14
00009E3B  8B4508            mov eax,[ebp+0x8]
00009E3E  8B5008            mov edx,[eax+0x8]
00009E41  8B4508            mov eax,[ebp+0x8]
00009E44  8B4010            mov eax,[eax+0x10]
00009E47  01D0              add eax,edx
00009E49  2B45F0            sub eax,[ebp-0x10]
00009E4C  83E806            sub eax,byte +0x6
00009E4F  8945EC            mov [ebp-0x14],eax
00009E52  8B4508            mov eax,[ebp+0x8]
00009E55  8B400C            mov eax,[eax+0xc]
00009E58  83C006            add eax,byte +0x6
00009E5B  8945E8            mov [ebp-0x18],eax
00009E5E  83EC0C            sub esp,byte +0xc
00009E61  680000AA00        push dword 0xaa0000
00009E66  FF75F0            push dword [ebp-0x10]
00009E69  FF75F0            push dword [ebp-0x10]
00009E6C  FF75E8            push dword [ebp-0x18]
00009E6F  FF75EC            push dword [ebp-0x14]
00009E72  E8D697FFFF        call 0x364d
00009E77  83C420            add esp,byte +0x20
00009E7A  8B45E8            mov eax,[ebp-0x18]
00009E7D  8D5002            lea edx,[eax+0x2]
00009E80  8B45EC            mov eax,[ebp-0x14]
00009E83  83C006            add eax,byte +0x6
00009E86  68FFFFFF00        push dword 0xffffff
00009E8B  6A78              push byte +0x78
00009E8D  52                push edx
00009E8E  50                push eax
00009E8F  E8D6A9FFFF        call 0x486a
00009E94  83C410            add esp,byte +0x10
00009E97  8B4508            mov eax,[ebp+0x8]
00009E9A  8B4048            mov eax,[eax+0x48]
00009E9D  83F801            cmp eax,byte +0x1
00009EA0  7513              jnz 0x9eb5
00009EA2  83EC0C            sub esp,byte +0xc
00009EA5  FF7508            push dword [ebp+0x8]
00009EA8  E857F7FFFF        call 0x9604
00009EAD  83C410            add esp,byte +0x10
00009EB0  E988000000        jmp 0x9f3d
00009EB5  8B4508            mov eax,[ebp+0x8]
00009EB8  8B4048            mov eax,[eax+0x48]
00009EBB  83F802            cmp eax,byte +0x2
00009EBE  7510              jnz 0x9ed0
00009EC0  83EC0C            sub esp,byte +0xc
00009EC3  FF7508            push dword [ebp+0x8]
00009EC6  E8BDEEFFFF        call 0x8d88
00009ECB  83C410            add esp,byte +0x10
00009ECE  EB6D              jmp short 0x9f3d
00009ED0  8B4508            mov eax,[ebp+0x8]
00009ED3  8B4048            mov eax,[eax+0x48]
00009ED6  83F803            cmp eax,byte +0x3
00009ED9  7510              jnz 0x9eeb
00009EDB  83EC0C            sub esp,byte +0xc
00009EDE  FF7508            push dword [ebp+0x8]
00009EE1  E831DBFFFF        call 0x7a17
00009EE6  83C410            add esp,byte +0x10
00009EE9  EB52              jmp short 0x9f3d
00009EEB  8B4508            mov eax,[ebp+0x8]
00009EEE  8B4048            mov eax,[eax+0x48]
00009EF1  83F804            cmp eax,byte +0x4
00009EF4  7510              jnz 0x9f06
00009EF6  83EC0C            sub esp,byte +0xc
00009EF9  FF7508            push dword [ebp+0x8]
00009EFC  E862F8FFFF        call 0x9763
00009F01  83C410            add esp,byte +0x10
00009F04  EB37              jmp short 0x9f3d
00009F06  8B4508            mov eax,[ebp+0x8]
00009F09  8B4048            mov eax,[eax+0x48]
00009F0C  83F805            cmp eax,byte +0x5
00009F0F  7510              jnz 0x9f21
00009F11  83EC0C            sub esp,byte +0xc
00009F14  FF7508            push dword [ebp+0x8]
00009F17  E82ACCFFFF        call 0x6b46
00009F1C  83C410            add esp,byte +0x10
00009F1F  EB1C              jmp short 0x9f3d
00009F21  8B4508            mov eax,[ebp+0x8]
00009F24  8B4048            mov eax,[eax+0x48]
00009F27  83F806            cmp eax,byte +0x6
00009F2A  7511              jnz 0x9f3d
00009F2C  83EC0C            sub esp,byte +0xc
00009F2F  FF7508            push dword [ebp+0x8]
00009F32  E8F1D2FFFF        call 0x7228
00009F37  83C410            add esp,byte +0x10
00009F3A  EB01              jmp short 0x9f3d
00009F3C  90                nop
00009F3D  8B5DFC            mov ebx,[ebp-0x4]
00009F40  C9                leave
00009F41  C3                ret
00009F42  55                push ebp
00009F43  89E5              mov ebp,esp
00009F45  83EC18            sub esp,byte +0x18
00009F48  83EC0C            sub esp,byte +0xc
00009F4B  6880800000        push dword 0x8080
00009F50  E8D995FFFF        call 0x352e
00009F55  83C410            add esp,byte +0x10
00009F58  83EC0C            sub esp,byte +0xc
00009F5B  68C0C0C000        push dword 0xc0c0c0
00009F60  6A1C              push byte +0x1c
00009F62  6800040000        push dword 0x400
00009F67  68E4020000        push dword 0x2e4
00009F6C  6A00              push byte +0x0
00009F6E  E8DA96FFFF        call 0x364d
00009F73  83C420            add esp,byte +0x20
00009F76  E894F3FFFF        call 0x930f
00009F7B  C745F400000000    mov dword [ebp-0xc],0x0
00009F82  EB46              jmp short 0x9fca
00009F84  8B45F4            mov eax,[ebp-0xc]
00009F87  69C0E80D0000      imul eax,eax,dword 0xde8
00009F8D  0584370200        add eax,0x23784
00009F92  8B00              mov eax,[eax]
00009F94  85C0              test eax,eax
00009F96  742E              jz 0x9fc6
00009F98  8B45F4            mov eax,[ebp-0xc]
00009F9B  69C0E80D0000      imul eax,eax,dword 0xde8
00009FA1  05C4370200        add eax,0x237c4
00009FA6  8B00              mov eax,[eax]
00009FA8  85C0              test eax,eax
00009FAA  741A              jz 0x9fc6
00009FAC  8B45F4            mov eax,[ebp-0xc]
00009FAF  69C0E80D0000      imul eax,eax,dword 0xde8
00009FB5  0580370200        add eax,0x23780
00009FBA  83EC0C            sub esp,byte +0xc
00009FBD  50                push eax
00009FBE  E8E7FCFFFF        call 0x9caa
00009FC3  83C410            add esp,byte +0x10
00009FC6  8345F401          add dword [ebp-0xc],byte +0x1
00009FCA  837DF409          cmp dword [ebp-0xc],byte +0x9
00009FCE  7EB4              jng 0x9f84
00009FD0  E894000000        call 0xa069
00009FD5  90                nop
00009FD6  C9                leave
00009FD7  C3                ret
00009FD8  55                push ebp
00009FD9  89E5              mov ebp,esp
00009FDB  83EC18            sub esp,byte +0x18
00009FDE  E85FFFFFFF        call 0x9f42
00009FE3  E8F095FFFF        call 0x35d8
00009FE8  E8F6C4FFFF        call 0x64e3
00009FED  8945F4            mov [ebp-0xc],eax
00009FF0  E8F8C4FFFF        call 0x64ed
00009FF5  8945F0            mov [ebp-0x10],eax
00009FF8  83EC08            sub esp,byte +0x8
00009FFB  FF75F0            push dword [ebp-0x10]
00009FFE  FF75F4            push dword [ebp-0xc]
0000A001  E888E4FFFF        call 0x848e
0000A006  83C410            add esp,byte +0x10
0000A009  8B45F4            mov eax,[ebp-0xc]
0000A00C  A350DC0100        mov [0x1dc50],eax
0000A011  8B45F0            mov eax,[ebp-0x10]
0000A014  A354DC0100        mov [0x1dc54],eax
0000A019  90                nop
0000A01A  C9                leave
0000A01B  C3                ret
0000A01C  55                push ebp
0000A01D  89E5              mov ebp,esp
0000A01F  83EC08            sub esp,byte +0x8
0000A022  A150DC0100        mov eax,[0x1dc50]
0000A027  83F8FF            cmp eax,byte -0x1
0000A02A  7419              jz 0xa045
0000A02C  8B1554DC0100      mov edx,[dword 0x1dc54]
0000A032  A150DC0100        mov eax,[0x1dc50]
0000A037  6A0A              push byte +0xa
0000A039  6A0A              push byte +0xa
0000A03B  52                push edx
0000A03C  50                push eax
0000A03D  E8E996FFFF        call 0x372b
0000A042  83C410            add esp,byte +0x10
0000A045  83EC08            sub esp,byte +0x8
0000A048  FF750C            push dword [ebp+0xc]
0000A04B  FF7508            push dword [ebp+0x8]
0000A04E  E83BE4FFFF        call 0x848e
0000A053  83C410            add esp,byte +0x10
0000A056  8B4508            mov eax,[ebp+0x8]
0000A059  A350DC0100        mov [0x1dc50],eax
0000A05E  8B450C            mov eax,[ebp+0xc]
0000A061  A354DC0100        mov [0x1dc54],eax
0000A066  90                nop
0000A067  C9                leave
0000A068  C3                ret
0000A069  55                push ebp
0000A06A  89E5              mov ebp,esp
0000A06C  83EC28            sub esp,byte +0x28
0000A06F  83EC0C            sub esp,byte +0xc
0000A072  8D45E6            lea eax,[ebp-0x1a]
0000A075  50                push eax
0000A076  E8F7C7FFFF        call 0x6872
0000A07B  83C410            add esp,byte +0x10
0000A07E  0FB645E8          movzx eax,byte [ebp-0x18]
0000A082  0FB6D0            movzx edx,al
0000A085  89D0              mov eax,edx
0000A087  C1E002            shl eax,byte 0x2
0000A08A  01D0              add eax,edx
0000A08C  C1E003            shl eax,byte 0x3
0000A08F  01D0              add eax,edx
0000A091  8D148500000000    lea edx,[eax*4+0x0]
0000A098  01D0              add eax,edx
0000A09A  66C1E808          shr ax,byte 0x8
0000A09E  C0E803            shr al,byte 0x3
0000A0A1  83C030            add eax,byte +0x30
0000A0A4  8845E0            mov [ebp-0x20],al
0000A0A7  0FB64DE8          movzx ecx,byte [ebp-0x18]
0000A0AB  0FB6D1            movzx edx,cl
0000A0AE  89D0              mov eax,edx
0000A0B0  C1E002            shl eax,byte 0x2
0000A0B3  01D0              add eax,edx
0000A0B5  C1E003            shl eax,byte 0x3
0000A0B8  01D0              add eax,edx
0000A0BA  8D148500000000    lea edx,[eax*4+0x0]
0000A0C1  01D0              add eax,edx
0000A0C3  66C1E808          shr ax,byte 0x8
0000A0C7  89C2              mov edx,eax
0000A0C9  C0EA03            shr dl,byte 0x3
0000A0CC  89D0              mov eax,edx
0000A0CE  C1E002            shl eax,byte 0x2
0000A0D1  01D0              add eax,edx
0000A0D3  01C0              add eax,eax
0000A0D5  29C1              sub ecx,eax
0000A0D7  89CA              mov edx,ecx
0000A0D9  8D4230            lea eax,[edx+0x30]
0000A0DC  8845E1            mov [ebp-0x1f],al
0000A0DF  C645E23A          mov byte [ebp-0x1e],0x3a
0000A0E3  0FB645E7          movzx eax,byte [ebp-0x19]
0000A0E7  0FB6D0            movzx edx,al
0000A0EA  89D0              mov eax,edx
0000A0EC  C1E002            shl eax,byte 0x2
0000A0EF  01D0              add eax,edx
0000A0F1  C1E003            shl eax,byte 0x3
0000A0F4  01D0              add eax,edx
0000A0F6  8D148500000000    lea edx,[eax*4+0x0]
0000A0FD  01D0              add eax,edx
0000A0FF  66C1E808          shr ax,byte 0x8
0000A103  C0E803            shr al,byte 0x3
0000A106  83C030            add eax,byte +0x30
0000A109  8845E3            mov [ebp-0x1d],al
0000A10C  0FB64DE7          movzx ecx,byte [ebp-0x19]
0000A110  0FB6D1            movzx edx,cl
0000A113  89D0              mov eax,edx
0000A115  C1E002            shl eax,byte 0x2
0000A118  01D0              add eax,edx
0000A11A  C1E003            shl eax,byte 0x3
0000A11D  01D0              add eax,edx
0000A11F  8D148500000000    lea edx,[eax*4+0x0]
0000A126  01D0              add eax,edx
0000A128  66C1E808          shr ax,byte 0x8
0000A12C  89C2              mov edx,eax
0000A12E  C0EA03            shr dl,byte 0x3
0000A131  89D0              mov eax,edx
0000A133  C1E002            shl eax,byte 0x2
0000A136  01D0              add eax,edx
0000A138  01C0              add eax,eax
0000A13A  29C1              sub ecx,eax
0000A13C  89CA              mov edx,ecx
0000A13E  8D4230            lea eax,[edx+0x30]
0000A141  8845E4            mov [ebp-0x1c],al
0000A144  C645E500          mov byte [ebp-0x1b],0x0
0000A148  C745F0B6030000    mov dword [ebp-0x10],0x3b6
0000A14F  C745ECE4020000    mov dword [ebp-0x14],0x2e4
0000A156  C745F400000000    mov dword [ebp-0xc],0x0
0000A15D  EB30              jmp short 0xa18f
0000A15F  8D55E0            lea edx,[ebp-0x20]
0000A162  8B45F4            mov eax,[ebp-0xc]
0000A165  01D0              add eax,edx
0000A167  0FB600            movzx eax,byte [eax]
0000A16A  0FBEC0            movsx eax,al
0000A16D  8B55F4            mov edx,[ebp-0xc]
0000A170  8D0CD500000000    lea ecx,[edx*8+0x0]
0000A177  8B55F0            mov edx,[ebp-0x10]
0000A17A  01CA              add edx,ecx
0000A17C  6A00              push byte +0x0
0000A17E  50                push eax
0000A17F  FF75EC            push dword [ebp-0x14]
0000A182  52                push edx
0000A183  E8E2A6FFFF        call 0x486a
0000A188  83C410            add esp,byte +0x10
0000A18B  8345F401          add dword [ebp-0xc],byte +0x1
0000A18F  837DF404          cmp dword [ebp-0xc],byte +0x4
0000A193  7ECA              jng 0xa15f
0000A195  90                nop
0000A196  C9                leave
0000A197  C3                ret
0000A198  55                push ebp
0000A199  89E5              mov ebp,esp
0000A19B  C7050C2902000000  mov dword [dword 0x2290c],0x0
         -0000
0000A1A5  A10C290200        mov eax,[0x2290c]
0000A1AA  6BC01C            imul eax,eax,byte +0x1c
0000A1AD  0580280200        add eax,0x22880
0000A1B2  C7003C000000      mov dword [eax],0x3c
0000A1B8  A10C290200        mov eax,[0x2290c]
0000A1BD  6BC01C            imul eax,eax,byte +0x1c
0000A1C0  0584280200        add eax,0x22884
0000A1C5  C7001E000000      mov dword [eax],0x1e
0000A1CB  A10C290200        mov eax,[0x2290c]
0000A1D0  6BC01C            imul eax,eax,byte +0x1c
0000A1D3  0588280200        add eax,0x22888
0000A1D8  C70020000000      mov dword [eax],0x20
0000A1DE  A10C290200        mov eax,[0x2290c]
0000A1E3  6BC01C            imul eax,eax,byte +0x1c
0000A1E6  058C280200        add eax,0x2288c
0000A1EB  C70020000000      mov dword [eax],0x20
0000A1F1  A10C290200        mov eax,[0x2290c]
0000A1F6  6BC01C            imul eax,eax,byte +0x1c
0000A1F9  0590280200        add eax,0x22890
0000A1FE  C70028C30100      mov dword [eax],0x1c328
0000A204  A10C290200        mov eax,[0x2290c]
0000A209  6BC01C            imul eax,eax,byte +0x1c
0000A20C  0594280200        add eax,0x22894
0000A211  C70080BC0100      mov dword [eax],0x1bc80
0000A217  A10C290200        mov eax,[0x2290c]
0000A21C  6BC01C            imul eax,eax,byte +0x1c
0000A21F  0598280200        add eax,0x22898
0000A224  C70000000000      mov dword [eax],0x0
0000A22A  A10C290200        mov eax,[0x2290c]
0000A22F  83C001            add eax,byte +0x1
0000A232  A30C290200        mov [0x2290c],eax
0000A237  A10C290200        mov eax,[0x2290c]
0000A23C  6BC01C            imul eax,eax,byte +0x1c
0000A23F  0580280200        add eax,0x22880
0000A244  C7003C000000      mov dword [eax],0x3c
0000A24A  A10C290200        mov eax,[0x2290c]
0000A24F  6BC01C            imul eax,eax,byte +0x1c
0000A252  0584280200        add eax,0x22884
0000A257  C70096000000      mov dword [eax],0x96
0000A25D  A10C290200        mov eax,[0x2290c]
0000A262  6BC01C            imul eax,eax,byte +0x1c
0000A265  0588280200        add eax,0x22888
0000A26A  C70020000000      mov dword [eax],0x20
0000A270  A10C290200        mov eax,[0x2290c]
0000A275  6BC01C            imul eax,eax,byte +0x1c
0000A278  058C280200        add eax,0x2288c
0000A27D  C70020000000      mov dword [eax],0x20
0000A283  A10C290200        mov eax,[0x2290c]
0000A288  6BC01C            imul eax,eax,byte +0x1c
0000A28B  0590280200        add eax,0x22890
0000A290  C70031C30100      mov dword [eax],0x1c331
0000A296  A10C290200        mov eax,[0x2290c]
0000A29B  6BC01C            imul eax,eax,byte +0x1c
0000A29E  0594280200        add eax,0x22894
0000A2A3  C70080BF0100      mov dword [eax],0x1bf80
0000A2A9  A10C290200        mov eax,[0x2290c]
0000A2AE  6BC01C            imul eax,eax,byte +0x1c
0000A2B1  0598280200        add eax,0x22898
0000A2B6  C70000000000      mov dword [eax],0x0
0000A2BC  A10C290200        mov eax,[0x2290c]
0000A2C1  83C001            add eax,byte +0x1
0000A2C4  A30C290200        mov [0x2290c],eax
0000A2C9  A10C290200        mov eax,[0x2290c]
0000A2CE  6BC01C            imul eax,eax,byte +0x1c
0000A2D1  0580280200        add eax,0x22880
0000A2D6  C7003C000000      mov dword [eax],0x3c
0000A2DC  A10C290200        mov eax,[0x2290c]
0000A2E1  6BC01C            imul eax,eax,byte +0x1c
0000A2E4  0584280200        add eax,0x22884
0000A2E9  C7000E010000      mov dword [eax],0x10e
0000A2EF  A10C290200        mov eax,[0x2290c]
0000A2F4  6BC01C            imul eax,eax,byte +0x1c
0000A2F7  0588280200        add eax,0x22888
0000A2FC  C70010000000      mov dword [eax],0x10
0000A302  A10C290200        mov eax,[0x2290c]
0000A307  6BC01C            imul eax,eax,byte +0x1c
0000A30A  058C280200        add eax,0x2288c
0000A30F  C70010000000      mov dword [eax],0x10
0000A315  A10C290200        mov eax,[0x2290c]
0000A31A  6BC01C            imul eax,eax,byte +0x1c
0000A31D  0590280200        add eax,0x22890
0000A322  C7003AC30100      mov dword [eax],0x1c33a
0000A328  A10C290200        mov eax,[0x2290c]
0000A32D  6BC01C            imul eax,eax,byte +0x1c
0000A330  0594280200        add eax,0x22894
0000A335  C70080C00100      mov dword [eax],0x1c080
0000A33B  A10C290200        mov eax,[0x2290c]
0000A340  6BC01C            imul eax,eax,byte +0x1c
0000A343  0598280200        add eax,0x22898
0000A348  C70000000000      mov dword [eax],0x0
0000A34E  A10C290200        mov eax,[0x2290c]
0000A353  83C001            add eax,byte +0x1
0000A356  A30C290200        mov [0x2290c],eax
0000A35B  A10C290200        mov eax,[0x2290c]
0000A360  6BC01C            imul eax,eax,byte +0x1c
0000A363  0580280200        add eax,0x22880
0000A368  C7003C000000      mov dword [eax],0x3c
0000A36E  A10C290200        mov eax,[0x2290c]
0000A373  6BC01C            imul eax,eax,byte +0x1c
0000A376  0584280200        add eax,0x22884
0000A37B  C70086010000      mov dword [eax],0x186
0000A381  A10C290200        mov eax,[0x2290c]
0000A386  6BC01C            imul eax,eax,byte +0x1c
0000A389  0588280200        add eax,0x22888
0000A38E  C70020000000      mov dword [eax],0x20
0000A394  A10C290200        mov eax,[0x2290c]
0000A399  6BC01C            imul eax,eax,byte +0x1c
0000A39C  058C280200        add eax,0x2288c
0000A3A1  C70020000000      mov dword [eax],0x20
0000A3A7  A10C290200        mov eax,[0x2290c]
0000A3AC  6BC01C            imul eax,eax,byte +0x1c
0000A3AF  0590280200        add eax,0x22890
0000A3B4  C7003FC30100      mov dword [eax],0x1c33f
0000A3BA  A10C290200        mov eax,[0x2290c]
0000A3BF  6BC01C            imul eax,eax,byte +0x1c
0000A3C2  0598280200        add eax,0x22898
0000A3C7  C70000000000      mov dword [eax],0x0
0000A3CD  A10C290200        mov eax,[0x2290c]
0000A3D2  83C001            add eax,byte +0x1
0000A3D5  A30C290200        mov [0x2290c],eax
0000A3DA  A10C290200        mov eax,[0x2290c]
0000A3DF  6BC01C            imul eax,eax,byte +0x1c
0000A3E2  0580280200        add eax,0x22880
0000A3E7  C7003C000000      mov dword [eax],0x3c
0000A3ED  A10C290200        mov eax,[0x2290c]
0000A3F2  6BC01C            imul eax,eax,byte +0x1c
0000A3F5  0584280200        add eax,0x22884
0000A3FA  C700FE010000      mov dword [eax],0x1fe
0000A400  A10C290200        mov eax,[0x2290c]
0000A405  6BC01C            imul eax,eax,byte +0x1c
0000A408  0588280200        add eax,0x22888
0000A40D  C70020000000      mov dword [eax],0x20
0000A413  A10C290200        mov eax,[0x2290c]
0000A418  6BC01C            imul eax,eax,byte +0x1c
0000A41B  058C280200        add eax,0x2288c
0000A420  C70020000000      mov dword [eax],0x20
0000A426  A10C290200        mov eax,[0x2290c]
0000A42B  6BC01C            imul eax,eax,byte +0x1c
0000A42E  0590280200        add eax,0x22890
0000A433  C70045C30100      mov dword [eax],0x1c345
0000A439  A10C290200        mov eax,[0x2290c]
0000A43E  6BC01C            imul eax,eax,byte +0x1c
0000A441  0598280200        add eax,0x22898
0000A446  C70000000000      mov dword [eax],0x0
0000A44C  A10C290200        mov eax,[0x2290c]
0000A451  83C001            add eax,byte +0x1
0000A454  A30C290200        mov [0x2290c],eax
0000A459  90                nop
0000A45A  5D                pop ebp
0000A45B  C3                ret
0000A45C  55                push ebp
0000A45D  89E5              mov ebp,esp
0000A45F  83EC18            sub esp,byte +0x18
0000A462  83EC08            sub esp,byte +0x8
0000A465  6A01              push byte +0x1
0000A467  6831C30100        push dword 0x1c331
0000A46C  6890010000        push dword 0x190
0000A471  6858020000        push dword 0x258
0000A476  6A64              push byte +0x64
0000A478  6A64              push byte +0x64
0000A47A  E84CE1FFFF        call 0x85cb
0000A47F  83C420            add esp,byte +0x20
0000A482  8945F4            mov [ebp-0xc],eax
0000A485  837DF400          cmp dword [ebp-0xc],byte +0x0
0000A489  740D              jz 0xa498
0000A48B  8B45F4            mov eax,[ebp-0xc]
0000A48E  A360280200        mov [0x22860],eax
0000A493  E840FBFFFF        call 0x9fd8
0000A498  90                nop
0000A499  C9                leave
0000A49A  C3                ret
0000A49B  55                push ebp
0000A49C  89E5              mov ebp,esp
0000A49E  83EC18            sub esp,byte +0x18
0000A4A1  83EC08            sub esp,byte +0x8
0000A4A4  6A02              push byte +0x2
0000A4A6  684CC30100        push dword 0x1c34c
0000A4AB  6890010000        push dword 0x190
0000A4B0  68F4010000        push dword 0x1f4
0000A4B5  6896000000        push dword 0x96
0000A4BA  6896000000        push dword 0x96
0000A4BF  E807E1FFFF        call 0x85cb
0000A4C4  83C420            add esp,byte +0x20
0000A4C7  8945F4            mov [ebp-0xc],eax
0000A4CA  837DF400          cmp dword [ebp-0xc],byte +0x0
0000A4CE  740D              jz 0xa4dd
0000A4D0  8B45F4            mov eax,[ebp-0xc]
0000A4D3  A360280200        mov [0x22860],eax
0000A4D8  E8FBFAFFFF        call 0x9fd8
0000A4DD  90                nop
0000A4DE  C9                leave
0000A4DF  C3                ret
0000A4E0  55                push ebp
0000A4E1  89E5              mov ebp,esp
0000A4E3  83EC58            sub esp,byte +0x58
0000A4E6  C745EC58C30100    mov dword [ebp-0x14],0x1c358
0000A4ED  C745F400000000    mov dword [ebp-0xc],0x0
0000A4F4  EB19              jmp short 0xa50f
0000A4F6  8B55F4            mov edx,[ebp-0xc]
0000A4F9  8B45EC            mov eax,[ebp-0x14]
0000A4FC  01D0              add eax,edx
0000A4FE  0FB600            movzx eax,byte [eax]
0000A501  8D4DA8            lea ecx,[ebp-0x58]
0000A504  8B55F4            mov edx,[ebp-0xc]
0000A507  01CA              add edx,ecx
0000A509  8802              mov [edx],al
0000A50B  8345F401          add dword [ebp-0xc],byte +0x1
0000A50F  8B55F4            mov edx,[ebp-0xc]
0000A512  8B45EC            mov eax,[ebp-0x14]
0000A515  01D0              add eax,edx
0000A517  0FB600            movzx eax,byte [eax]
0000A51A  84C0              test al,al
0000A51C  75D8              jnz 0xa4f6
0000A51E  C745F000000000    mov dword [ebp-0x10],0x0
0000A525  EB21              jmp short 0xa548
0000A527  8B45F0            mov eax,[ebp-0x10]
0000A52A  8D5001            lea edx,[eax+0x1]
0000A52D  8955F0            mov [ebp-0x10],edx
0000A530  89C2              mov edx,eax
0000A532  8B4508            mov eax,[ebp+0x8]
0000A535  8D0C02            lea ecx,[edx+eax]
0000A538  8B45F4            mov eax,[ebp-0xc]
0000A53B  8D5001            lea edx,[eax+0x1]
0000A53E  8955F4            mov [ebp-0xc],edx
0000A541  0FB611            movzx edx,byte [ecx]
0000A544  885405A8          mov [ebp+eax-0x58],dl
0000A548  8B55F0            mov edx,[ebp-0x10]
0000A54B  8B4508            mov eax,[ebp+0x8]
0000A54E  01D0              add eax,edx
0000A550  0FB600            movzx eax,byte [eax]
0000A553  84C0              test al,al
0000A555  7406              jz 0xa55d
0000A557  837DF43E          cmp dword [ebp-0xc],byte +0x3e
0000A55B  7ECA              jng 0xa527
0000A55D  8D55A8            lea edx,[ebp-0x58]
0000A560  8B45F4            mov eax,[ebp-0xc]
0000A563  01D0              add eax,edx
0000A565  C60000            mov byte [eax],0x0
0000A568  8D55A8            lea edx,[ebp-0x58]
0000A56B  8B45F4            mov eax,[ebp-0xc]
0000A56E  01D0              add eax,edx
0000A570  C60000            mov byte [eax],0x0
0000A573  83EC08            sub esp,byte +0x8
0000A576  6A03              push byte +0x3
0000A578  8D45A8            lea eax,[ebp-0x58]
0000A57B  50                push eax
0000A57C  68C2010000        push dword 0x1c2
0000A581  6858020000        push dword 0x258
0000A586  6A78              push byte +0x78
0000A588  6A78              push byte +0x78
0000A58A  E83CE0FFFF        call 0x85cb
0000A58F  83C420            add esp,byte +0x20
0000A592  8945E8            mov [ebp-0x18],eax
0000A595  837DE800          cmp dword [ebp-0x18],byte +0x0
0000A599  741E              jz 0xa5b9
0000A59B  83EC08            sub esp,byte +0x8
0000A59E  FF7508            push dword [ebp+0x8]
0000A5A1  FF75E8            push dword [ebp-0x18]
0000A5A4  E8EDDCFFFF        call 0x8296
0000A5A9  83C410            add esp,byte +0x10
0000A5AC  8B45E8            mov eax,[ebp-0x18]
0000A5AF  A360280200        mov [0x22860],eax
0000A5B4  E81FFAFFFF        call 0x9fd8
0000A5B9  90                nop
0000A5BA  C9                leave
0000A5BB  C3                ret
0000A5BC  55                push ebp
0000A5BD  89E5              mov ebp,esp
0000A5BF  83EC48            sub esp,byte +0x48
0000A5C2  C645F700          mov byte [ebp-0x9],0x0
0000A5C6  837D0C2F          cmp dword [ebp+0xc],byte +0x2f
0000A5CA  7E0E              jng 0xa5da
0000A5CC  837D0C39          cmp dword [ebp+0xc],byte +0x39
0000A5D0  7F08              jg 0xa5da
0000A5D2  8B450C            mov eax,[ebp+0xc]
0000A5D5  8845F7            mov [ebp-0x9],al
0000A5D8  EB58              jmp short 0xa632
0000A5DA  837D0C2B          cmp dword [ebp+0xc],byte +0x2b
0000A5DE  7506              jnz 0xa5e6
0000A5E0  C645F72B          mov byte [ebp-0x9],0x2b
0000A5E4  EB4C              jmp short 0xa632
0000A5E6  837D0C2D          cmp dword [ebp+0xc],byte +0x2d
0000A5EA  7506              jnz 0xa5f2
0000A5EC  C645F72D          mov byte [ebp-0x9],0x2d
0000A5F0  EB40              jmp short 0xa632
0000A5F2  837D0C2A          cmp dword [ebp+0xc],byte +0x2a
0000A5F6  7506              jnz 0xa5fe
0000A5F8  C645F72A          mov byte [ebp-0x9],0x2a
0000A5FC  EB34              jmp short 0xa632
0000A5FE  837D0C2F          cmp dword [ebp+0xc],byte +0x2f
0000A602  7506              jnz 0xa60a
0000A604  C645F72F          mov byte [ebp-0x9],0x2f
0000A608  EB28              jmp short 0xa632
0000A60A  837D0C3D          cmp dword [ebp+0xc],byte +0x3d
0000A60E  7406              jz 0xa616
0000A610  837D0C0A          cmp dword [ebp+0xc],byte +0xa
0000A614  7506              jnz 0xa61c
0000A616  C645F73D          mov byte [ebp-0x9],0x3d
0000A61A  EB16              jmp short 0xa632
0000A61C  837D0C63          cmp dword [ebp+0xc],byte +0x63
0000A620  740C              jz 0xa62e
0000A622  837D0C43          cmp dword [ebp+0xc],byte +0x43
0000A626  7406              jz 0xa62e
0000A628  837D0C08          cmp dword [ebp+0xc],byte +0x8
0000A62C  7504              jnz 0xa632
0000A62E  C645F743          mov byte [ebp-0x9],0x43
0000A632  807DF700          cmp byte [ebp-0x9],0x0
0000A636  0F8404040000      jz near 0xaa40
0000A63C  807DF72F          cmp byte [ebp-0x9],0x2f
0000A640  0F8E84000000      jng near 0xa6ca
0000A646  807DF739          cmp byte [ebp-0x9],0x39
0000A64A  7F7E              jg 0xa6ca
0000A64C  8B4508            mov eax,[ebp+0x8]
0000A64F  8B80D80C0000      mov eax,[eax+0xcd8]
0000A655  85C0              test eax,eax
0000A657  7429              jz 0xa682
0000A659  8B4508            mov eax,[ebp+0x8]
0000A65C  0FB655F7          movzx edx,byte [ebp-0x9]
0000A660  8890C00C0000      mov [eax+0xcc0],dl
0000A666  8B4508            mov eax,[ebp+0x8]
0000A669  C680C10C000000    mov byte [eax+0xcc1],0x0
0000A670  8B4508            mov eax,[ebp+0x8]
0000A673  C780D80C00000000  mov dword [eax+0xcd8],0x0
         -0000
0000A67D  E9B8030000        jmp 0xaa3a
0000A682  8B4508            mov eax,[ebp+0x8]
0000A685  05C00C0000        add eax,0xcc0
0000A68A  50                push eax
0000A68B  E873DEFFFF        call 0x8503
0000A690  83C404            add esp,byte +0x4
0000A693  8945C8            mov [ebp-0x38],eax
0000A696  837DC809          cmp dword [ebp-0x38],byte +0x9
0000A69A  0F8F9A030000      jg near 0xaa3a
0000A6A0  8B5508            mov edx,[ebp+0x8]
0000A6A3  8B45C8            mov eax,[ebp-0x38]
0000A6A6  01D0              add eax,edx
0000A6A8  8D90C00C0000      lea edx,[eax+0xcc0]
0000A6AE  0FB645F7          movzx eax,byte [ebp-0x9]
0000A6B2  8802              mov [edx],al
0000A6B4  8B45C8            mov eax,[ebp-0x38]
0000A6B7  8D5001            lea edx,[eax+0x1]
0000A6BA  8B4508            mov eax,[ebp+0x8]
0000A6BD  C68410C00C000000  mov byte [eax+edx+0xcc0],0x0
0000A6C5  E970030000        jmp 0xaa3a
0000A6CA  807DF743          cmp byte [ebp-0x9],0x43
0000A6CE  7540              jnz 0xa710
0000A6D0  8B4508            mov eax,[ebp+0x8]
0000A6D3  C780D00C00000000  mov dword [eax+0xcd0],0x0
         -0000
0000A6DD  8B4508            mov eax,[ebp+0x8]
0000A6E0  C780D40C00000000  mov dword [eax+0xcd4],0x0
         -0000
0000A6EA  8B4508            mov eax,[ebp+0x8]
0000A6ED  C780D80C00000100  mov dword [eax+0xcd8],0x1
         -0000
0000A6F7  8B4508            mov eax,[ebp+0x8]
0000A6FA  C680C00C000030    mov byte [eax+0xcc0],0x30
0000A701  8B4508            mov eax,[ebp+0x8]
0000A704  C680C10C000000    mov byte [eax+0xcc1],0x0
0000A70B  E92B030000        jmp 0xaa3b
0000A710  807DF73D          cmp byte [ebp-0x9],0x3d
0000A714  0F854C020000      jnz near 0xa966
0000A71A  C745F000000000    mov dword [ebp-0x10],0x0
0000A721  8B4508            mov eax,[ebp+0x8]
0000A724  05C00C0000        add eax,0xcc0
0000A729  8945EC            mov [ebp-0x14],eax
0000A72C  C745E801000000    mov dword [ebp-0x18],0x1
0000A733  8B45EC            mov eax,[ebp-0x14]
0000A736  0FB600            movzx eax,byte [eax]
0000A739  3C2D              cmp al,0x2d
0000A73B  7530              jnz 0xa76d
0000A73D  C745E8FFFFFFFF    mov dword [ebp-0x18],0xffffffff
0000A744  8345EC01          add dword [ebp-0x14],byte +0x1
0000A748  EB23              jmp short 0xa76d
0000A74A  8B55F0            mov edx,[ebp-0x10]
0000A74D  89D0              mov eax,edx
0000A74F  C1E002            shl eax,byte 0x2
0000A752  01D0              add eax,edx
0000A754  01C0              add eax,eax
0000A756  89C2              mov edx,eax
0000A758  8B45EC            mov eax,[ebp-0x14]
0000A75B  0FB600            movzx eax,byte [eax]
0000A75E  0FBEC0            movsx eax,al
0000A761  83E830            sub eax,byte +0x30
0000A764  01D0              add eax,edx
0000A766  8945F0            mov [ebp-0x10],eax
0000A769  8345EC01          add dword [ebp-0x14],byte +0x1
0000A76D  8B45EC            mov eax,[ebp-0x14]
0000A770  0FB600            movzx eax,byte [eax]
0000A773  84C0              test al,al
0000A775  75D3              jnz 0xa74a
0000A777  8B45F0            mov eax,[ebp-0x10]
0000A77A  0FAF45E8          imul eax,[ebp-0x18]
0000A77E  8945F0            mov [ebp-0x10],eax
0000A781  8B4508            mov eax,[ebp+0x8]
0000A784  8B80D40C0000      mov eax,[eax+0xcd4]
0000A78A  83F801            cmp eax,byte +0x1
0000A78D  751C              jnz 0xa7ab
0000A78F  8B4508            mov eax,[ebp+0x8]
0000A792  8B90D00C0000      mov edx,[eax+0xcd0]
0000A798  8B45F0            mov eax,[ebp-0x10]
0000A79B  01C2              add edx,eax
0000A79D  8B4508            mov eax,[ebp+0x8]
0000A7A0  8990D00C0000      mov [eax+0xcd0],edx
0000A7A6  E989000000        jmp 0xa834
0000A7AB  8B4508            mov eax,[ebp+0x8]
0000A7AE  8B80D40C0000      mov eax,[eax+0xcd4]
0000A7B4  83F802            cmp eax,byte +0x2
0000A7B7  7519              jnz 0xa7d2
0000A7B9  8B4508            mov eax,[ebp+0x8]
0000A7BC  8B80D00C0000      mov eax,[eax+0xcd0]
0000A7C2  2B45F0            sub eax,[ebp-0x10]
0000A7C5  89C2              mov edx,eax
0000A7C7  8B4508            mov eax,[ebp+0x8]
0000A7CA  8990D00C0000      mov [eax+0xcd0],edx
0000A7D0  EB62              jmp short 0xa834
0000A7D2  8B4508            mov eax,[ebp+0x8]
0000A7D5  8B80D40C0000      mov eax,[eax+0xcd4]
0000A7DB  83F803            cmp eax,byte +0x3
0000A7DE  751A              jnz 0xa7fa
0000A7E0  8B4508            mov eax,[ebp+0x8]
0000A7E3  8B80D00C0000      mov eax,[eax+0xcd0]
0000A7E9  0FAF45F0          imul eax,[ebp-0x10]
0000A7ED  89C2              mov edx,eax
0000A7EF  8B4508            mov eax,[ebp+0x8]
0000A7F2  8990D00C0000      mov [eax+0xcd0],edx
0000A7F8  EB3A              jmp short 0xa834
0000A7FA  8B4508            mov eax,[ebp+0x8]
0000A7FD  8B80D40C0000      mov eax,[eax+0xcd4]
0000A803  83F804            cmp eax,byte +0x4
0000A806  7520              jnz 0xa828
0000A808  837DF000          cmp dword [ebp-0x10],byte +0x0
0000A80C  7426              jz 0xa834
0000A80E  8B4508            mov eax,[ebp+0x8]
0000A811  8B80D00C0000      mov eax,[eax+0xcd0]
0000A817  99                cdq
0000A818  F77DF0            idiv dword [ebp-0x10]
0000A81B  89C2              mov edx,eax
0000A81D  8B4508            mov eax,[ebp+0x8]
0000A820  8990D00C0000      mov [eax+0xcd0],edx
0000A826  EB0C              jmp short 0xa834
0000A828  8B4508            mov eax,[ebp+0x8]
0000A82B  8B55F0            mov edx,[ebp-0x10]
0000A82E  8990D00C0000      mov [eax+0xcd0],edx
0000A834  8B4508            mov eax,[ebp+0x8]
0000A837  C780D40C00000000  mov dword [eax+0xcd4],0x0
         -0000
0000A841  8B4508            mov eax,[ebp+0x8]
0000A844  C780D80C00000100  mov dword [eax+0xcd8],0x1
         -0000
0000A84E  8B4508            mov eax,[ebp+0x8]
0000A851  8B80D00C0000      mov eax,[eax+0xcd0]
0000A857  85C0              test eax,eax
0000A859  7519              jnz 0xa874
0000A85B  8B4508            mov eax,[ebp+0x8]
0000A85E  C680C00C000030    mov byte [eax+0xcc0],0x30
0000A865  8B4508            mov eax,[ebp+0x8]
0000A868  C680C10C000000    mov byte [eax+0xcc1],0x0
0000A86F  E9C7010000        jmp 0xaa3b
0000A874  8B4508            mov eax,[ebp+0x8]
0000A877  8B80D00C0000      mov eax,[eax+0xcd0]
0000A87D  8945E4            mov [ebp-0x1c],eax
0000A880  C745E000000000    mov dword [ebp-0x20],0x0
0000A887  837DE400          cmp dword [ebp-0x1c],byte +0x0
0000A88B  790A              jns 0xa897
0000A88D  C745E001000000    mov dword [ebp-0x20],0x1
0000A894  F75DE4            neg dword [ebp-0x1c]
0000A897  C745DC00000000    mov dword [ebp-0x24],0x0
0000A89E  837DE400          cmp dword [ebp-0x1c],byte +0x0
0000A8A2  7562              jnz 0xa906
0000A8A4  8B45DC            mov eax,[ebp-0x24]
0000A8A7  8D5001            lea edx,[eax+0x1]
0000A8AA  8955DC            mov [ebp-0x24],edx
0000A8AD  C64405B830        mov byte [ebp+eax-0x48],0x30
0000A8B2  EB52              jmp short 0xa906
0000A8B4  8B4DE4            mov ecx,[ebp-0x1c]
0000A8B7  BA67666666        mov edx,0x66666667
0000A8BC  89C8              mov eax,ecx
0000A8BE  F7EA              imul edx
0000A8C0  C1FA02            sar edx,byte 0x2
0000A8C3  89C8              mov eax,ecx
0000A8C5  C1F81F            sar eax,byte 0x1f
0000A8C8  29C2              sub edx,eax
0000A8CA  89D0              mov eax,edx
0000A8CC  C1E002            shl eax,byte 0x2
0000A8CF  01D0              add eax,edx
0000A8D1  01C0              add eax,eax
0000A8D3  29C1              sub ecx,eax
0000A8D5  89CA              mov edx,ecx
0000A8D7  89D0              mov eax,edx
0000A8D9  8D4830            lea ecx,[eax+0x30]
0000A8DC  8B45DC            mov eax,[ebp-0x24]
0000A8DF  8D5001            lea edx,[eax+0x1]
0000A8E2  8955DC            mov [ebp-0x24],edx
0000A8E5  89CA              mov edx,ecx
0000A8E7  885405B8          mov [ebp+eax-0x48],dl
0000A8EB  8B4DE4            mov ecx,[ebp-0x1c]
0000A8EE  BA67666666        mov edx,0x66666667
0000A8F3  89C8              mov eax,ecx
0000A8F5  F7EA              imul edx
0000A8F7  C1FA02            sar edx,byte 0x2
0000A8FA  89C8              mov eax,ecx
0000A8FC  C1F81F            sar eax,byte 0x1f
0000A8FF  29C2              sub edx,eax
0000A901  89D0              mov eax,edx
0000A903  8945E4            mov [ebp-0x1c],eax
0000A906  837DE400          cmp dword [ebp-0x1c],byte +0x0
0000A90A  7FA8              jg 0xa8b4
0000A90C  837DE000          cmp dword [ebp-0x20],byte +0x0
0000A910  740E              jz 0xa920
0000A912  8B45DC            mov eax,[ebp-0x24]
0000A915  8D5001            lea edx,[eax+0x1]
0000A918  8955DC            mov [ebp-0x24],edx
0000A91B  C64405B82D        mov byte [ebp+eax-0x48],0x2d
0000A920  C745D800000000    mov dword [ebp-0x28],0x0
0000A927  EB22              jmp short 0xa94b
0000A929  836DDC01          sub dword [ebp-0x24],byte +0x1
0000A92D  8B45D8            mov eax,[ebp-0x28]
0000A930  8D5001            lea edx,[eax+0x1]
0000A933  8955D8            mov [ebp-0x28],edx
0000A936  8D4DB8            lea ecx,[ebp-0x48]
0000A939  8B55DC            mov edx,[ebp-0x24]
0000A93C  01CA              add edx,ecx
0000A93E  0FB60A            movzx ecx,byte [edx]
0000A941  8B5508            mov edx,[ebp+0x8]
0000A944  888C02C00C0000    mov [edx+eax+0xcc0],cl
0000A94B  837DDC00          cmp dword [ebp-0x24],byte +0x0
0000A94F  7FD8              jg 0xa929
0000A951  8B5508            mov edx,[ebp+0x8]
0000A954  8B45D8            mov eax,[ebp-0x28]
0000A957  01D0              add eax,edx
0000A959  05C00C0000        add eax,0xcc0
0000A95E  C60000            mov byte [eax],0x0
0000A961  E9D5000000        jmp 0xaa3b
0000A966  C745D400000000    mov dword [ebp-0x2c],0x0
0000A96D  8B4508            mov eax,[ebp+0x8]
0000A970  05C00C0000        add eax,0xcc0
0000A975  8945D0            mov [ebp-0x30],eax
0000A978  C745CC01000000    mov dword [ebp-0x34],0x1
0000A97F  8B45D0            mov eax,[ebp-0x30]
0000A982  0FB600            movzx eax,byte [eax]
0000A985  3C2D              cmp al,0x2d
0000A987  7530              jnz 0xa9b9
0000A989  C745CCFFFFFFFF    mov dword [ebp-0x34],0xffffffff
0000A990  8345D001          add dword [ebp-0x30],byte +0x1
0000A994  EB23              jmp short 0xa9b9
0000A996  8B55D4            mov edx,[ebp-0x2c]
0000A999  89D0              mov eax,edx
0000A99B  C1E002            shl eax,byte 0x2
0000A99E  01D0              add eax,edx
0000A9A0  01C0              add eax,eax
0000A9A2  89C2              mov edx,eax
0000A9A4  8B45D0            mov eax,[ebp-0x30]
0000A9A7  0FB600            movzx eax,byte [eax]
0000A9AA  0FBEC0            movsx eax,al
0000A9AD  83E830            sub eax,byte +0x30
0000A9B0  01D0              add eax,edx
0000A9B2  8945D4            mov [ebp-0x2c],eax
0000A9B5  8345D001          add dword [ebp-0x30],byte +0x1
0000A9B9  8B45D0            mov eax,[ebp-0x30]
0000A9BC  0FB600            movzx eax,byte [eax]
0000A9BF  84C0              test al,al
0000A9C1  75D3              jnz 0xa996
0000A9C3  8B45D4            mov eax,[ebp-0x2c]
0000A9C6  0FAF45CC          imul eax,[ebp-0x34]
0000A9CA  8945D4            mov [ebp-0x2c],eax
0000A9CD  8B4508            mov eax,[ebp+0x8]
0000A9D0  8B55D4            mov edx,[ebp-0x2c]
0000A9D3  8990D00C0000      mov [eax+0xcd0],edx
0000A9D9  837D0C2B          cmp dword [ebp+0xc],byte +0x2b
0000A9DD  750F              jnz 0xa9ee
0000A9DF  8B4508            mov eax,[ebp+0x8]
0000A9E2  C780D40C00000100  mov dword [eax+0xcd4],0x1
         -0000
0000A9EC  EB3D              jmp short 0xaa2b
0000A9EE  837D0C2D          cmp dword [ebp+0xc],byte +0x2d
0000A9F2  750F              jnz 0xaa03
0000A9F4  8B4508            mov eax,[ebp+0x8]
0000A9F7  C780D40C00000200  mov dword [eax+0xcd4],0x2
         -0000
0000AA01  EB28              jmp short 0xaa2b
0000AA03  837D0C2A          cmp dword [ebp+0xc],byte +0x2a
0000AA07  750F              jnz 0xaa18
0000AA09  8B4508            mov eax,[ebp+0x8]
0000AA0C  C780D40C00000300  mov dword [eax+0xcd4],0x3
         -0000
0000AA16  EB13              jmp short 0xaa2b
0000AA18  837D0C2F          cmp dword [ebp+0xc],byte +0x2f
0000AA1C  750D              jnz 0xaa2b
0000AA1E  8B4508            mov eax,[ebp+0x8]
0000AA21  C780D40C00000400  mov dword [eax+0xcd4],0x4
         -0000
0000AA2B  8B4508            mov eax,[ebp+0x8]
0000AA2E  C780D80C00000100  mov dword [eax+0xcd8],0x1
         -0000
0000AA38  EB01              jmp short 0xaa3b
0000AA3A  90                nop
0000AA3B  E898F5FFFF        call 0x9fd8
0000AA40  90                nop
0000AA41  C9                leave
0000AA42  C3                ret
0000AA43  55                push ebp
0000AA44  89E5              mov ebp,esp
0000AA46  83EC18            sub esp,byte +0x18
0000AA49  83EC08            sub esp,byte +0x8
0000AA4C  6A04              push byte +0x4
0000AA4E  6862C30100        push dword 0x1c362
0000AA53  6840010000        push dword 0x140
0000AA58  68F0000000        push dword 0xf0
0000AA5D  68C8000000        push dword 0xc8
0000AA62  682C010000        push dword 0x12c
0000AA67  E85FDBFFFF        call 0x85cb
0000AA6C  83C420            add esp,byte +0x20
0000AA6F  8945F4            mov [ebp-0xc],eax
0000AA72  837DF400          cmp dword [ebp-0xc],byte +0x0
0000AA76  7448              jz 0xaac0
0000AA78  8B45F4            mov eax,[ebp-0xc]
0000AA7B  C780D00C00000000  mov dword [eax+0xcd0],0x0
         -0000
0000AA85  8B45F4            mov eax,[ebp-0xc]
0000AA88  C780D40C00000000  mov dword [eax+0xcd4],0x0
         -0000
0000AA92  8B45F4            mov eax,[ebp-0xc]
0000AA95  C780D80C00000100  mov dword [eax+0xcd8],0x1
         -0000
0000AA9F  8B45F4            mov eax,[ebp-0xc]
0000AAA2  C680C00C000030    mov byte [eax+0xcc0],0x30
0000AAA9  8B45F4            mov eax,[ebp-0xc]
0000AAAC  C680C10C000000    mov byte [eax+0xcc1],0x0
0000AAB3  8B45F4            mov eax,[ebp-0xc]
0000AAB6  A360280200        mov [0x22860],eax
0000AABB  E818F5FFFF        call 0x9fd8
0000AAC0  90                nop
0000AAC1  C9                leave
0000AAC2  C3                ret
0000AAC3  55                push ebp
0000AAC4  89E5              mov ebp,esp
0000AAC6  83EC18            sub esp,byte +0x18
0000AAC9  83EC08            sub esp,byte +0x8
0000AACC  6A05              push byte +0x5
0000AACE  683FC30100        push dword 0x1c33f
0000AAD3  68C2010000        push dword 0x1c2
0000AAD8  6858020000        push dword 0x258
0000AADD  6896000000        push dword 0x96
0000AAE2  68C8000000        push dword 0xc8
0000AAE7  E8DFDAFFFF        call 0x85cb
0000AAEC  83C420            add esp,byte +0x20
0000AAEF  8945F4            mov [ebp-0xc],eax
0000AAF2  837DF400          cmp dword [ebp-0xc],byte +0x0
0000AAF6  740D              jz 0xab05
0000AAF8  8B45F4            mov eax,[ebp-0xc]
0000AAFB  A360280200        mov [0x22860],eax
0000AB00  E8D3F4FFFF        call 0x9fd8
0000AB05  90                nop
0000AB06  C9                leave
0000AB07  C3                ret
0000AB08  55                push ebp
0000AB09  89E5              mov ebp,esp
0000AB0B  83EC18            sub esp,byte +0x18
0000AB0E  83EC08            sub esp,byte +0x8
0000AB11  6A06              push byte +0x6
0000AB13  6886C10100        push dword 0x1c186
0000AB18  682C010000        push dword 0x12c
0000AB1D  6890010000        push dword 0x190
0000AB22  682C010000        push dword 0x12c
0000AB27  6890010000        push dword 0x190
0000AB2C  E89ADAFFFF        call 0x85cb
0000AB31  83C420            add esp,byte +0x20
0000AB34  8945F4            mov [ebp-0xc],eax
0000AB37  837DF400          cmp dword [ebp-0xc],byte +0x0
0000AB3B  740D              jz 0xab4a
0000AB3D  8B45F4            mov eax,[ebp-0xc]
0000AB40  A360280200        mov [0x22860],eax
0000AB45  E88EF4FFFF        call 0x9fd8
0000AB4A  90                nop
0000AB4B  C9                leave
0000AB4C  C3                ret
0000AB4D  55                push ebp
0000AB4E  89E5              mov ebp,esp
0000AB50  83EC08            sub esp,byte +0x8
0000AB53  E811DAFFFF        call 0x8569
0000AB58  E83BF6FFFF        call 0xa198
0000AB5D  E876F4FFFF        call 0x9fd8
0000AB62  90                nop
0000AB63  C9                leave
0000AB64  C3                ret
0000AB65  55                push ebp
0000AB66  89E5              mov ebp,esp
0000AB68  81ECC8000000      sub esp,0xc8
0000AB6E  C745EC00000000    mov dword [ebp-0x14],0x0
0000AB75  C745F409000000    mov dword [ebp-0xc],0x9
0000AB7C  E9F1050000        jmp 0xb172
0000AB81  8B45F4            mov eax,[ebp-0xc]
0000AB84  69C0E80D0000      imul eax,eax,dword 0xde8
0000AB8A  0580370200        add eax,0x23780
0000AB8F  8945E8            mov [ebp-0x18],eax
0000AB92  8B45E8            mov eax,[ebp-0x18]
0000AB95  8B4004            mov eax,[eax+0x4]
0000AB98  85C0              test eax,eax
0000AB9A  0F84CD050000      jz near 0xb16d
0000ABA0  8B45E8            mov eax,[ebp-0x18]
0000ABA3  8B4044            mov eax,[eax+0x44]
0000ABA6  85C0              test eax,eax
0000ABA8  0F84BF050000      jz near 0xb16d
0000ABAE  8B45E8            mov eax,[ebp-0x18]
0000ABB1  8B4008            mov eax,[eax+0x8]
0000ABB4  394508            cmp [ebp+0x8],eax
0000ABB7  0F8CB1050000      jl near 0xb16e
0000ABBD  8B45E8            mov eax,[ebp-0x18]
0000ABC0  8B5008            mov edx,[eax+0x8]
0000ABC3  8B45E8            mov eax,[ebp-0x18]
0000ABC6  8B4010            mov eax,[eax+0x10]
0000ABC9  01D0              add eax,edx
0000ABCB  394508            cmp [ebp+0x8],eax
0000ABCE  0F8F9A050000      jg near 0xb16e
0000ABD4  8B45E8            mov eax,[ebp-0x18]
0000ABD7  8B400C            mov eax,[eax+0xc]
0000ABDA  39450C            cmp [ebp+0xc],eax
0000ABDD  0F8C8B050000      jl near 0xb16e
0000ABE3  8B45E8            mov eax,[ebp-0x18]
0000ABE6  8B500C            mov edx,[eax+0xc]
0000ABE9  8B45E8            mov eax,[ebp-0x18]
0000ABEC  8B4014            mov eax,[eax+0x14]
0000ABEF  01D0              add eax,edx
0000ABF1  39450C            cmp [ebp+0xc],eax
0000ABF4  0F8F74050000      jg near 0xb16e
0000ABFA  C745EC01000000    mov dword [ebp-0x14],0x1
0000AC01  8B45E8            mov eax,[ebp-0x18]
0000AC04  A360280200        mov [0x22860],eax
0000AC09  C745E414000000    mov dword [ebp-0x1c],0x14
0000AC10  8B45E8            mov eax,[ebp-0x18]
0000AC13  8B5008            mov edx,[eax+0x8]
0000AC16  8B45E8            mov eax,[ebp-0x18]
0000AC19  8B4010            mov eax,[eax+0x10]
0000AC1C  01D0              add eax,edx
0000AC1E  2B45E4            sub eax,[ebp-0x1c]
0000AC21  83E806            sub eax,byte +0x6
0000AC24  8945E0            mov [ebp-0x20],eax
0000AC27  8B45E8            mov eax,[ebp-0x18]
0000AC2A  8B400C            mov eax,[eax+0xc]
0000AC2D  83C006            add eax,byte +0x6
0000AC30  8945DC            mov [ebp-0x24],eax
0000AC33  8B4508            mov eax,[ebp+0x8]
0000AC36  3B45E0            cmp eax,[ebp-0x20]
0000AC39  7C53              jl 0xac8e
0000AC3B  8B55E0            mov edx,[ebp-0x20]
0000AC3E  8B45E4            mov eax,[ebp-0x1c]
0000AC41  01D0              add eax,edx
0000AC43  394508            cmp [ebp+0x8],eax
0000AC46  7F46              jg 0xac8e
0000AC48  8B450C            mov eax,[ebp+0xc]
0000AC4B  3B45DC            cmp eax,[ebp-0x24]
0000AC4E  7C3E              jl 0xac8e
0000AC50  8B55DC            mov edx,[ebp-0x24]
0000AC53  8B45E4            mov eax,[ebp-0x1c]
0000AC56  01D0              add eax,edx
0000AC58  39450C            cmp [ebp+0xc],eax
0000AC5B  7F31              jg 0xac8e
0000AC5D  8B45E8            mov eax,[ebp-0x18]
0000AC60  8B4048            mov eax,[eax+0x48]
0000AC63  83F803            cmp eax,byte +0x3
0000AC66  8B45E8            mov eax,[ebp-0x18]
0000AC69  C7400400000000    mov dword [eax+0x4],0x0
0000AC70  8B45E8            mov eax,[ebp-0x18]
0000AC73  C7404400000000    mov dword [eax+0x44],0x0
0000AC7A  C705602802000000  mov dword [dword 0x22860],0x0
         -0000
0000AC84  E84FF3FFFF        call 0x9fd8
0000AC89  E9D7050000        jmp 0xb265
0000AC8E  A160280200        mov eax,[0x22860]
0000AC93  8B4048            mov eax,[eax+0x48]
0000AC96  83F805            cmp eax,byte +0x5
0000AC99  754F              jnz 0xacea
0000AC9B  A160280200        mov eax,[0x22860]
0000ACA0  8B400C            mov eax,[eax+0xc]
0000ACA3  83C048            add eax,byte +0x48
0000ACA6  39450C            cmp [ebp+0xc],eax
0000ACA9  7D19              jnl 0xacc4
0000ACAB  A160280200        mov eax,[0x22860]
0000ACB0  83EC04            sub esp,byte +0x4
0000ACB3  FF750C            push dword [ebp+0xc]
0000ACB6  FF7508            push dword [ebp+0x8]
0000ACB9  50                push eax
0000ACBA  E808C1FFFF        call 0x6dc7
0000ACBF  83C410            add esp,byte +0x10
0000ACC2  EB26              jmp short 0xacea
0000ACC4  A160280200        mov eax,[0x22860]
0000ACC9  C780E80C0000FFFF  mov dword [eax+0xce8],0xffffffff
         -FFFF
0000ACD3  A160280200        mov eax,[0x22860]
0000ACD8  83EC04            sub esp,byte +0x4
0000ACDB  FF750C            push dword [ebp+0xc]
0000ACDE  FF7508            push dword [ebp+0x8]
0000ACE1  50                push eax
0000ACE2  E8E0C0FFFF        call 0x6dc7
0000ACE7  83C410            add esp,byte +0x10
0000ACEA  8B45E8            mov eax,[ebp-0x18]
0000ACED  8B4048            mov eax,[eax+0x48]
0000ACF0  83F804            cmp eax,byte +0x4
0000ACF3  0F85D1010000      jnz near 0xaeca
0000ACF9  8B45E8            mov eax,[ebp-0x18]
0000ACFC  8B4008            mov eax,[eax+0x8]
0000ACFF  83C01C            add eax,byte +0x1c
0000AD02  8945A4            mov [ebp-0x5c],eax
0000AD05  8B45E8            mov eax,[ebp-0x18]
0000AD08  8B400C            mov eax,[eax+0xc]
0000AD0B  83C040            add eax,byte +0x40
0000AD0E  8945A0            mov [ebp-0x60],eax
0000AD11  8B4508            mov eax,[ebp+0x8]
0000AD14  3B45A4            cmp eax,[ebp-0x5c]
0000AD17  0F8C0C040000      jl near 0xb129
0000AD1D  8B450C            mov eax,[ebp+0xc]
0000AD20  3B45A0            cmp eax,[ebp-0x60]
0000AD23  0F8C00040000      jl near 0xb129
0000AD29  8B4508            mov eax,[ebp+0x8]
0000AD2C  2B45A4            sub eax,[ebp-0x5c]
0000AD2F  89459C            mov [ebp-0x64],eax
0000AD32  8B450C            mov eax,[ebp+0xc]
0000AD35  2B45A0            sub eax,[ebp-0x60]
0000AD38  894598            mov [ebp-0x68],eax
0000AD3B  8B4D9C            mov ecx,[ebp-0x64]
0000AD3E  BAABAAAA2A        mov edx,0x2aaaaaab
0000AD43  89C8              mov eax,ecx
0000AD45  F7EA              imul edx
0000AD47  C1FA03            sar edx,byte 0x3
0000AD4A  89C8              mov eax,ecx
0000AD4C  C1F81F            sar eax,byte 0x1f
0000AD4F  29C2              sub edx,eax
0000AD51  89D0              mov eax,edx
0000AD53  894594            mov [ebp-0x6c],eax
0000AD56  8B4D98            mov ecx,[ebp-0x68]
0000AD59  BAABAAAA2A        mov edx,0x2aaaaaab
0000AD5E  89C8              mov eax,ecx
0000AD60  F7EA              imul edx
0000AD62  C1FA03            sar edx,byte 0x3
0000AD65  89C8              mov eax,ecx
0000AD67  C1F81F            sar eax,byte 0x1f
0000AD6A  29C2              sub edx,eax
0000AD6C  89D0              mov eax,edx
0000AD6E  894590            mov [ebp-0x70],eax
0000AD71  8B4D9C            mov ecx,[ebp-0x64]
0000AD74  BAABAAAA2A        mov edx,0x2aaaaaab
0000AD79  89C8              mov eax,ecx
0000AD7B  F7EA              imul edx
0000AD7D  C1FA03            sar edx,byte 0x3
0000AD80  89C8              mov eax,ecx
0000AD82  C1F81F            sar eax,byte 0x1f
0000AD85  29C2              sub edx,eax
0000AD87  89D0              mov eax,edx
0000AD89  01C0              add eax,eax
0000AD8B  01D0              add eax,edx
0000AD8D  C1E004            shl eax,byte 0x4
0000AD90  29C1              sub ecx,eax
0000AD92  89CA              mov edx,ecx
0000AD94  83FA27            cmp edx,byte +0x27
0000AD97  0F9EC0            setng al
0000AD9A  0FB6C0            movzx eax,al
0000AD9D  89458C            mov [ebp-0x74],eax
0000ADA0  8B4D98            mov ecx,[ebp-0x68]
0000ADA3  BAABAAAA2A        mov edx,0x2aaaaaab
0000ADA8  89C8              mov eax,ecx
0000ADAA  F7EA              imul edx
0000ADAC  C1FA03            sar edx,byte 0x3
0000ADAF  89C8              mov eax,ecx
0000ADB1  C1F81F            sar eax,byte 0x1f
0000ADB4  29C2              sub edx,eax
0000ADB6  89D0              mov eax,edx
0000ADB8  01C0              add eax,eax
0000ADBA  01D0              add eax,edx
0000ADBC  C1E004            shl eax,byte 0x4
0000ADBF  29C1              sub ecx,eax
0000ADC1  89CA              mov edx,ecx
0000ADC3  83FA27            cmp edx,byte +0x27
0000ADC6  0F9EC0            setng al
0000ADC9  0FB6C0            movzx eax,al
0000ADCC  894588            mov [ebp-0x78],eax
0000ADCF  837D9403          cmp dword [ebp-0x6c],byte +0x3
0000ADD3  0F8F50030000      jg near 0xb129
0000ADD9  837D9003          cmp dword [ebp-0x70],byte +0x3
0000ADDD  0F8F46030000      jg near 0xb129
0000ADE3  837D8C00          cmp dword [ebp-0x74],byte +0x0
0000ADE7  0F843C030000      jz near 0xb129
0000ADED  837D8800          cmp dword [ebp-0x78],byte +0x0
0000ADF1  0F8432030000      jz near 0xb129
0000ADF7  C78544FFFFFF0AC3  mov dword [ebp-0xbc],0x1c30a
         -0100
0000AE01  C78548FFFFFF0CC3  mov dword [ebp-0xb8],0x1c30c
         -0100
0000AE0B  C7854CFFFFFF0EC3  mov dword [ebp-0xb4],0x1c30e
         -0100
0000AE15  C78550FFFFFF9DC1  mov dword [ebp-0xb0],0x1c19d
         -0100
0000AE1F  C78554FFFFFF10C3  mov dword [ebp-0xac],0x1c310
         -0100
0000AE29  C78558FFFFFF12C3  mov dword [ebp-0xa8],0x1c312
         -0100
0000AE33  C7855CFFFFFF14C3  mov dword [ebp-0xa4],0x1c314
         -0100
0000AE3D  C78560FFFFFF16C3  mov dword [ebp-0xa0],0x1c316
         -0100
0000AE47  C78564FFFFFF18C3  mov dword [ebp-0x9c],0x1c318
         -0100
0000AE51  C78568FFFFFF1AC3  mov dword [ebp-0x98],0x1c31a
         -0100
0000AE5B  C7856CFFFFFF1CC3  mov dword [ebp-0x94],0x1c31c
         -0100
0000AE65  C78570FFFFFF1EC3  mov dword [ebp-0x90],0x1c31e
         -0100
0000AE6F  C78574FFFFFF20C3  mov dword [ebp-0x8c],0x1c320
         -0100
0000AE79  C78578FFFFFF22C3  mov dword [ebp-0x88],0x1c322
         -0100
0000AE83  C7857CFFFFFF24C3  mov dword [ebp-0x84],0x1c324
         -0100
0000AE8D  C7458026C30100    mov dword [ebp-0x80],0x1c326
0000AE94  8B4590            mov eax,[ebp-0x70]
0000AE97  8D148500000000    lea edx,[eax*4+0x0]
0000AE9E  8B4594            mov eax,[ebp-0x6c]
0000AEA1  01D0              add eax,edx
0000AEA3  894584            mov [ebp-0x7c],eax
0000AEA6  8B4584            mov eax,[ebp-0x7c]
0000AEA9  8B848544FFFFFF    mov eax,[ebp+eax*4-0xbc]
0000AEB0  0FB600            movzx eax,byte [eax]
0000AEB3  0FBEC0            movsx eax,al
0000AEB6  83EC08            sub esp,byte +0x8
0000AEB9  50                push eax
0000AEBA  FF75E8            push dword [ebp-0x18]
0000AEBD  E8FAF6FFFF        call 0xa5bc
0000AEC2  83C410            add esp,byte +0x10
0000AEC5  E95F020000        jmp 0xb129
0000AECA  8B45E8            mov eax,[ebp-0x18]
0000AECD  8B4048            mov eax,[eax+0x48]
0000AED0  83F802            cmp eax,byte +0x2
0000AED3  0F8550020000      jnz near 0xb129
0000AED9  8B45E8            mov eax,[ebp-0x18]
0000AEDC  8B4008            mov eax,[eax+0x8]
0000AEDF  83C002            add eax,byte +0x2
0000AEE2  8945D8            mov [ebp-0x28],eax
0000AEE5  8B45E8            mov eax,[ebp-0x18]
0000AEE8  8B400C            mov eax,[eax+0xc]
0000AEEB  83C014            add eax,byte +0x14
0000AEEE  8945D4            mov [ebp-0x2c],eax
0000AEF1  8B45D8            mov eax,[ebp-0x28]
0000AEF4  83C00A            add eax,byte +0xa
0000AEF7  8945D0            mov [ebp-0x30],eax
0000AEFA  8B45D4            mov eax,[ebp-0x2c]
0000AEFD  83C00A            add eax,byte +0xa
0000AF00  8945CC            mov [ebp-0x34],eax
0000AF03  C745C83C000000    mov dword [ebp-0x38],0x3c
0000AF0A  C745C428000000    mov dword [ebp-0x3c],0x28
0000AF11  C745C003000000    mov dword [ebp-0x40],0x3
0000AF18  8B4508            mov eax,[ebp+0x8]
0000AF1B  2B45D0            sub eax,[ebp-0x30]
0000AF1E  8945BC            mov [ebp-0x44],eax
0000AF21  8B450C            mov eax,[ebp+0xc]
0000AF24  2B45CC            sub eax,[ebp-0x34]
0000AF27  8945B8            mov [ebp-0x48],eax
0000AF2A  837DBC00          cmp dword [ebp-0x44],byte +0x0
0000AF2E  0F88E8010000      js near 0xb11c
0000AF34  837DB800          cmp dword [ebp-0x48],byte +0x0
0000AF38  0F88DE010000      js near 0xb11c
0000AF3E  8B45BC            mov eax,[ebp-0x44]
0000AF41  99                cdq
0000AF42  F77DC8            idiv dword [ebp-0x38]
0000AF45  8945BC            mov [ebp-0x44],eax
0000AF48  8B45B8            mov eax,[ebp-0x48]
0000AF4B  99                cdq
0000AF4C  F77DC4            idiv dword [ebp-0x3c]
0000AF4F  8945B8            mov [ebp-0x48],eax
0000AF52  8B45BC            mov eax,[ebp-0x44]
0000AF55  3B45C0            cmp eax,[ebp-0x40]
0000AF58  0F8DBE010000      jnl near 0xb11c
0000AF5E  8B45B8            mov eax,[ebp-0x48]
0000AF61  0FAF45C0          imul eax,[ebp-0x40]
0000AF65  89C2              mov edx,eax
0000AF67  8B45BC            mov eax,[ebp-0x44]
0000AF6A  01D0              add eax,edx
0000AF6C  8945B4            mov [ebp-0x4c],eax
0000AF6F  837DB400          cmp dword [ebp-0x4c],byte +0x0
0000AF73  0F88A3010000      js near 0xb11c
0000AF79  8B45E8            mov eax,[ebp-0x18]
0000AF7C  8B80980C0000      mov eax,[eax+0xc98]
0000AF82  3945B4            cmp [ebp-0x4c],eax
0000AF85  0F8D91010000      jnl near 0xb11c
0000AF8B  8B45E8            mov eax,[ebp-0x18]
0000AF8E  8B809C0C0000      mov eax,[eax+0xc9c]
0000AF94  3945B4            cmp [ebp-0x4c],eax
0000AF97  0F8569010000      jnz near 0xb106
0000AF9D  8B4DE8            mov ecx,[ebp-0x18]
0000AFA0  8B55B4            mov edx,[ebp-0x4c]
0000AFA3  89D0              mov eax,edx
0000AFA5  C1E002            shl eax,byte 0x2
0000AFA8  01D0              add eax,edx
0000AFAA  C1E002            shl eax,byte 0x2
0000AFAD  01C8              add eax,ecx
0000AFAF  05680B0000        add eax,0xb68
0000AFB4  8B00              mov eax,[eax]
0000AFB6  85C0              test eax,eax
0000AFB8  7467              jz 0xb021
0000AFBA  8B55B4            mov edx,[ebp-0x4c]
0000AFBD  89D0              mov eax,edx
0000AFBF  C1E002            shl eax,byte 0x2
0000AFC2  01D0              add eax,edx
0000AFC4  C1E002            shl eax,byte 0x2
0000AFC7  8D90500B0000      lea edx,[eax+0xb50]
0000AFCD  8B45E8            mov eax,[ebp-0x18]
0000AFD0  01D0              add eax,edx
0000AFD2  83C008            add eax,byte +0x8
0000AFD5  83EC0C            sub esp,byte +0xc
0000AFD8  50                push eax
0000AFD9  E84F78FFFF        call 0x282d
0000AFDE  83C410            add esp,byte +0x10
0000AFE1  85C0              test eax,eax
0000AFE3  0F841D010000      jz near 0xb106
0000AFE9  8B45E8            mov eax,[ebp-0x18]
0000AFEC  05580B0000        add eax,0xb58
0000AFF1  83EC08            sub esp,byte +0x8
0000AFF4  6A10              push byte +0x10
0000AFF6  50                push eax
0000AFF7  E85C61FFFF        call 0x1158
0000AFFC  83C410            add esp,byte +0x10
0000AFFF  89C2              mov edx,eax
0000B001  8B45E8            mov eax,[ebp-0x18]
0000B004  8990980C0000      mov [eax+0xc98],edx
0000B00A  8B45E8            mov eax,[ebp-0x18]
0000B00D  C7809C0C0000FFFF  mov dword [eax+0xc9c],0xffffffff
         -FFFF
0000B017  E8BCEFFFFF        call 0x9fd8
0000B01C  E944020000        jmp 0xb265
0000B021  8B55B4            mov edx,[ebp-0x4c]
0000B024  89D0              mov eax,edx
0000B026  C1E002            shl eax,byte 0x2
0000B029  01D0              add eax,edx
0000B02B  C1E002            shl eax,byte 0x2
0000B02E  8D90500B0000      lea edx,[eax+0xb50]
0000B034  8B45E8            mov eax,[ebp-0x18]
0000B037  01D0              add eax,edx
0000B039  83C008            add eax,byte +0x8
0000B03C  8945B0            mov [ebp-0x50],eax
0000B03F  83EC0C            sub esp,byte +0xc
0000B042  FF75B0            push dword [ebp-0x50]
0000B045  E8B9D4FFFF        call 0x8503
0000B04A  83C410            add esp,byte +0x10
0000B04D  8945AC            mov [ebp-0x54],eax
0000B050  837DAC04          cmp dword [ebp-0x54],byte +0x4
0000B054  0F8EAC000000      jng near 0xb106
0000B05A  8B45AC            mov eax,[ebp-0x54]
0000B05D  8D50FD            lea edx,[eax-0x3]
0000B060  8B45B0            mov eax,[ebp-0x50]
0000B063  01D0              add eax,edx
0000B065  0FB600            movzx eax,byte [eax]
0000B068  3C42              cmp al,0x42
0000B06A  7551              jnz 0xb0bd
0000B06C  8B45AC            mov eax,[ebp-0x54]
0000B06F  8D50FE            lea edx,[eax-0x2]
0000B072  8B45B0            mov eax,[ebp-0x50]
0000B075  01D0              add eax,edx
0000B077  0FB600            movzx eax,byte [eax]
0000B07A  3C49              cmp al,0x49
0000B07C  753F              jnz 0xb0bd
0000B07E  8B45AC            mov eax,[ebp-0x54]
0000B081  8D50FF            lea edx,[eax-0x1]
0000B084  8B45B0            mov eax,[ebp-0x50]
0000B087  01D0              add eax,edx
0000B089  0FB600            movzx eax,byte [eax]
0000B08C  3C4E              cmp al,0x4e
0000B08E  752D              jnz 0xb0bd
0000B090  83EC08            sub esp,byte +0x8
0000B093  6800000200        push dword 0x20000
0000B098  FF75B0            push dword [ebp-0x50]
0000B09B  E81A64FFFF        call 0x14ba
0000B0A0  83C410            add esp,byte +0x10
0000B0A3  85C0              test eax,eax
0000B0A5  745F              jz 0xb106
0000B0A7  C745A800000200    mov dword [ebp-0x58],0x20000
0000B0AE  8B45A8            mov eax,[ebp-0x58]
0000B0B1  FFD0              call eax
0000B0B3  E820EFFFFF        call 0x9fd8
0000B0B8  E9A8010000        jmp 0xb265
0000B0BD  8B45AC            mov eax,[ebp-0x54]
0000B0C0  8D50FD            lea edx,[eax-0x3]
0000B0C3  8B45B0            mov eax,[ebp-0x50]
0000B0C6  01D0              add eax,edx
0000B0C8  0FB600            movzx eax,byte [eax]
0000B0CB  3C54              cmp al,0x54
0000B0CD  7537              jnz 0xb106
0000B0CF  8B45AC            mov eax,[ebp-0x54]
0000B0D2  8D50FE            lea edx,[eax-0x2]
0000B0D5  8B45B0            mov eax,[ebp-0x50]
0000B0D8  01D0              add eax,edx
0000B0DA  0FB600            movzx eax,byte [eax]
0000B0DD  3C58              cmp al,0x58
0000B0DF  7525              jnz 0xb106
0000B0E1  8B45AC            mov eax,[ebp-0x54]
0000B0E4  8D50FF            lea edx,[eax-0x1]
0000B0E7  8B45B0            mov eax,[ebp-0x50]
0000B0EA  01D0              add eax,edx
0000B0EC  0FB600            movzx eax,byte [eax]
0000B0EF  3C54              cmp al,0x54
0000B0F1  7513              jnz 0xb106
0000B0F3  83EC0C            sub esp,byte +0xc
0000B0F6  FF75B0            push dword [ebp-0x50]
0000B0F9  E8E2F3FFFF        call 0xa4e0
0000B0FE  83C410            add esp,byte +0x10
0000B101  E95F010000        jmp 0xb265
0000B106  8B45E8            mov eax,[ebp-0x18]
0000B109  8B55B4            mov edx,[ebp-0x4c]
0000B10C  89909C0C0000      mov [eax+0xc9c],edx
0000B112  E8C1EEFFFF        call 0x9fd8
0000B117  E949010000        jmp 0xb265
0000B11C  8B45E8            mov eax,[ebp-0x18]
0000B11F  C7809C0C0000FFFF  mov dword [eax+0xc9c],0xffffffff
         -FFFF
0000B129  8B45E8            mov eax,[ebp-0x18]
0000B12C  8B400C            mov eax,[eax+0xc]
0000B12F  83C014            add eax,byte +0x14
0000B132  39450C            cmp [ebp+0xc],eax
0000B135  7F2C              jg 0xb163
0000B137  8B45E8            mov eax,[ebp-0x18]
0000B13A  C7403801000000    mov dword [eax+0x38],0x1
0000B141  8B45E8            mov eax,[ebp-0x18]
0000B144  8B4008            mov eax,[eax+0x8]
0000B147  8B5508            mov edx,[ebp+0x8]
0000B14A  29C2              sub edx,eax
0000B14C  8B45E8            mov eax,[ebp-0x18]
0000B14F  89503C            mov [eax+0x3c],edx
0000B152  8B45E8            mov eax,[ebp-0x18]
0000B155  8B400C            mov eax,[eax+0xc]
0000B158  8B550C            mov edx,[ebp+0xc]
0000B15B  29C2              sub edx,eax
0000B15D  8B45E8            mov eax,[ebp-0x18]
0000B160  895040            mov [eax+0x40],edx
0000B163  E870EEFFFF        call 0x9fd8
0000B168  E9F8000000        jmp 0xb265
0000B16D  90                nop
0000B16E  836DF401          sub dword [ebp-0xc],byte +0x1
0000B172  837DF400          cmp dword [ebp-0xc],byte +0x0
0000B176  0F8905FAFFFF      jns near 0xab81
0000B17C  837DEC00          cmp dword [ebp-0x14],byte +0x0
0000B180  0F85DF000000      jnz near 0xb265
0000B186  C705602802000000  mov dword [dword 0x22860],0x0
         -0000
0000B190  C745F000000000    mov dword [ebp-0x10],0x0
0000B197  E9B6000000        jmp 0xb252
0000B19C  8B45F0            mov eax,[ebp-0x10]
0000B19F  6BC01C            imul eax,eax,byte +0x1c
0000B1A2  0580280200        add eax,0x22880
0000B1A7  8B00              mov eax,[eax]
0000B1A9  394508            cmp [ebp+0x8],eax
0000B1AC  0F8C8B000000      jl near 0xb23d
0000B1B2  8B45F0            mov eax,[ebp-0x10]
0000B1B5  6BC01C            imul eax,eax,byte +0x1c
0000B1B8  0580280200        add eax,0x22880
0000B1BD  8B00              mov eax,[eax]
0000B1BF  83C040            add eax,byte +0x40
0000B1C2  394508            cmp [ebp+0x8],eax
0000B1C5  7D76              jnl 0xb23d
0000B1C7  8B45F0            mov eax,[ebp-0x10]
0000B1CA  6BC01C            imul eax,eax,byte +0x1c
0000B1CD  0584280200        add eax,0x22884
0000B1D2  8B00              mov eax,[eax]
0000B1D4  39450C            cmp [ebp+0xc],eax
0000B1D7  7C64              jl 0xb23d
0000B1D9  8B45F0            mov eax,[ebp-0x10]
0000B1DC  6BC01C            imul eax,eax,byte +0x1c
0000B1DF  0584280200        add eax,0x22884
0000B1E4  8B00              mov eax,[eax]
0000B1E6  83C040            add eax,byte +0x40
0000B1E9  39450C            cmp [ebp+0xc],eax
0000B1EC  7D4F              jnl 0xb23d
0000B1EE  8B45F0            mov eax,[ebp-0x10]
0000B1F1  6BC01C            imul eax,eax,byte +0x1c
0000B1F4  0598280200        add eax,0x22898
0000B1F9  C70001000000      mov dword [eax],0x1
0000B1FF  837DF000          cmp dword [ebp-0x10],byte +0x0
0000B203  7505              jnz 0xb20a
0000B205  E891F2FFFF        call 0xa49b
0000B20A  837DF001          cmp dword [ebp-0x10],byte +0x1
0000B20E  7505              jnz 0xb215
0000B210  E847F2FFFF        call 0xa45c
0000B215  837DF002          cmp dword [ebp-0x10],byte +0x2
0000B219  7505              jnz 0xb220
0000B21B  E823F8FFFF        call 0xaa43
0000B220  837DF003          cmp dword [ebp-0x10],byte +0x3
0000B224  7505              jnz 0xb22b
0000B226  E898F8FFFF        call 0xaac3
0000B22B  837DF004          cmp dword [ebp-0x10],byte +0x4
0000B22F  7505              jnz 0xb236
0000B231  E8D2F8FFFF        call 0xab08
0000B236  E89DEDFFFF        call 0x9fd8
0000B23B  EB28              jmp short 0xb265
0000B23D  8B45F0            mov eax,[ebp-0x10]
0000B240  6BC01C            imul eax,eax,byte +0x1c
0000B243  0598280200        add eax,0x22898
0000B248  C70000000000      mov dword [eax],0x0
0000B24E  8345F001          add dword [ebp-0x10],byte +0x1
0000B252  A10C290200        mov eax,[0x2290c]
0000B257  3945F0            cmp [ebp-0x10],eax
0000B25A  0F8C3CFFFFFF      jl near 0xb19c
0000B260  E873EDFFFF        call 0x9fd8
0000B265  C9                leave
0000B266  C3                ret
0000B267  55                push ebp
0000B268  89E5              mov ebp,esp
0000B26A  83EC08            sub esp,byte +0x8
0000B26D  A160280200        mov eax,[0x22860]
0000B272  85C0              test eax,eax
0000B274  743F              jz 0xb2b5
0000B276  A160280200        mov eax,[0x22860]
0000B27B  8B4038            mov eax,[eax+0x38]
0000B27E  85C0              test eax,eax
0000B280  7433              jz 0xb2b5
0000B282  A160280200        mov eax,[0x22860]
0000B287  8B503C            mov edx,[eax+0x3c]
0000B28A  A160280200        mov eax,[0x22860]
0000B28F  8B4D08            mov ecx,[ebp+0x8]
0000B292  29D1              sub ecx,edx
0000B294  89CA              mov edx,ecx
0000B296  895008            mov [eax+0x8],edx
0000B299  A160280200        mov eax,[0x22860]
0000B29E  8B5040            mov edx,[eax+0x40]
0000B2A1  A160280200        mov eax,[0x22860]
0000B2A6  8B4D0C            mov ecx,[ebp+0xc]
0000B2A9  29D1              sub ecx,edx
0000B2AB  89CA              mov edx,ecx
0000B2AD  89500C            mov [eax+0xc],edx
0000B2B0  E823EDFFFF        call 0x9fd8
0000B2B5  90                nop
0000B2B6  C9                leave
0000B2B7  C3                ret
0000B2B8  55                push ebp
0000B2B9  89E5              mov ebp,esp
0000B2BB  A160280200        mov eax,[0x22860]
0000B2C0  85C0              test eax,eax
0000B2C2  740C              jz 0xb2d0
0000B2C4  A160280200        mov eax,[0x22860]
0000B2C9  C7403800000000    mov dword [eax+0x38],0x0
0000B2D0  90                nop
0000B2D1  5D                pop ebp
0000B2D2  C3                ret
0000B2D3  55                push ebp
0000B2D4  89E5              mov ebp,esp
0000B2D6  83EC28            sub esp,byte +0x28
0000B2D9  C745F400000000    mov dword [ebp-0xc],0x0
0000B2E0  837D1000          cmp dword [ebp+0x10],byte +0x0
0000B2E4  7513              jnz 0xb2f9
0000B2E6  8B45F4            mov eax,[ebp-0xc]
0000B2E9  8D5001            lea edx,[eax+0x1]
0000B2EC  8955F4            mov [ebp-0xc],edx
0000B2EF  C64405D830        mov byte [ebp+eax-0x28],0x30
0000B2F4  E98F000000        jmp 0xb388
0000B2F9  8B4510            mov eax,[ebp+0x10]
0000B2FC  8945F0            mov [ebp-0x10],eax
0000B2FF  837DF000          cmp dword [ebp-0x10],byte +0x0
0000B303  7919              jns 0xb31e
0000B305  6A0F              push byte +0xf
0000B307  6A2D              push byte +0x2d
0000B309  FF750C            push dword [ebp+0xc]
0000B30C  FF7508            push dword [ebp+0x8]
0000B30F  E85695FFFF        call 0x486a
0000B314  83C410            add esp,byte +0x10
0000B317  83450804          add dword [ebp+0x8],byte +0x4
0000B31B  F75DF0            neg dword [ebp-0x10]
0000B31E  C745EC01000000    mov dword [ebp-0x14],0x1
0000B325  EB0F              jmp short 0xb336
0000B327  8B55EC            mov edx,[ebp-0x14]
0000B32A  89D0              mov eax,edx
0000B32C  C1E002            shl eax,byte 0x2
0000B32F  01D0              add eax,edx
0000B331  01C0              add eax,eax
0000B333  8945EC            mov [ebp-0x14],eax
0000B336  8B45F0            mov eax,[ebp-0x10]
0000B339  99                cdq
0000B33A  F77DEC            idiv dword [ebp-0x14]
0000B33D  83F809            cmp eax,byte +0x9
0000B340  7FE5              jg 0xb327
0000B342  EB3E              jmp short 0xb382
0000B344  8B45F0            mov eax,[ebp-0x10]
0000B347  99                cdq
0000B348  F77DEC            idiv dword [ebp-0x14]
0000B34B  8D4830            lea ecx,[eax+0x30]
0000B34E  8B45F4            mov eax,[ebp-0xc]
0000B351  8D5001            lea edx,[eax+0x1]
0000B354  8955F4            mov [ebp-0xc],edx
0000B357  89CA              mov edx,ecx
0000B359  885405D8          mov [ebp+eax-0x28],dl
0000B35D  8B45F0            mov eax,[ebp-0x10]
0000B360  99                cdq
0000B361  F77DEC            idiv dword [ebp-0x14]
0000B364  8955F0            mov [ebp-0x10],edx
0000B367  8B4DEC            mov ecx,[ebp-0x14]
0000B36A  BA67666666        mov edx,0x66666667
0000B36F  89C8              mov eax,ecx
0000B371  F7EA              imul edx
0000B373  C1FA02            sar edx,byte 0x2
0000B376  89C8              mov eax,ecx
0000B378  C1F81F            sar eax,byte 0x1f
0000B37B  29C2              sub edx,eax
0000B37D  89D0              mov eax,edx
0000B37F  8945EC            mov [ebp-0x14],eax
0000B382  837DEC00          cmp dword [ebp-0x14],byte +0x0
0000B386  7FBC              jg 0xb344
0000B388  8D55D8            lea edx,[ebp-0x28]
0000B38B  8B45F4            mov eax,[ebp-0xc]
0000B38E  01D0              add eax,edx
0000B390  C60000            mov byte [eax],0x0
0000B393  C745E800000000    mov dword [ebp-0x18],0x0
0000B39A  EB30              jmp short 0xb3cc
0000B39C  8D55D8            lea edx,[ebp-0x28]
0000B39F  8B45E8            mov eax,[ebp-0x18]
0000B3A2  01D0              add eax,edx
0000B3A4  0FB600            movzx eax,byte [eax]
0000B3A7  0FBEC0            movsx eax,al
0000B3AA  8B55E8            mov edx,[ebp-0x18]
0000B3AD  8D0C9500000000    lea ecx,[edx*4+0x0]
0000B3B4  8B5508            mov edx,[ebp+0x8]
0000B3B7  01CA              add edx,ecx
0000B3B9  6A0F              push byte +0xf
0000B3BB  50                push eax
0000B3BC  FF750C            push dword [ebp+0xc]
0000B3BF  52                push edx
0000B3C0  E8A594FFFF        call 0x486a
0000B3C5  83C410            add esp,byte +0x10
0000B3C8  8345E801          add dword [ebp-0x18],byte +0x1
0000B3CC  8B45E8            mov eax,[ebp-0x18]
0000B3CF  3B45F4            cmp eax,[ebp-0xc]
0000B3D2  7CC8              jl 0xb39c
0000B3D4  90                nop
0000B3D5  C9                leave
0000B3D6  C3                ret
0000B3D7  55                push ebp
0000B3D8  89E5              mov ebp,esp
0000B3DA  83EC28            sub esp,byte +0x28
0000B3DD  E801B1FFFF        call 0x64e3
0000B3E2  8945F4            mov [ebp-0xc],eax
0000B3E5  E803B1FFFF        call 0x64ed
0000B3EA  8945F0            mov [ebp-0x10],eax
0000B3ED  E805B1FFFF        call 0x64f7
0000B3F2  8945EC            mov [ebp-0x14],eax
0000B3F5  8B45EC            mov eax,[ebp-0x14]
0000B3F8  83E001            and eax,byte +0x1
0000B3FB  85C0              test eax,eax
0000B3FD  7413              jz 0xb412
0000B3FF  A110290200        mov eax,[0x22910]
0000B404  83E001            and eax,byte +0x1
0000B407  85C0              test eax,eax
0000B409  7507              jnz 0xb412
0000B40B  B801000000        mov eax,0x1
0000B410  EB05              jmp short 0xb417
0000B412  B800000000        mov eax,0x0
0000B417  8945E8            mov [ebp-0x18],eax
0000B41A  8B45EC            mov eax,[ebp-0x14]
0000B41D  83E001            and eax,byte +0x1
0000B420  85C0              test eax,eax
0000B422  7513              jnz 0xb437
0000B424  A110290200        mov eax,[0x22910]
0000B429  83E001            and eax,byte +0x1
0000B42C  85C0              test eax,eax
0000B42E  7407              jz 0xb437
0000B430  B801000000        mov eax,0x1
0000B435  EB05              jmp short 0xb43c
0000B437  B800000000        mov eax,0x0
0000B43C  8945E4            mov [ebp-0x1c],eax
0000B43F  837DE800          cmp dword [ebp-0x18],byte +0x0
0000B443  7414              jz 0xb459
0000B445  83EC04            sub esp,byte +0x4
0000B448  FF75EC            push dword [ebp-0x14]
0000B44B  FF75F0            push dword [ebp-0x10]
0000B44E  FF75F4            push dword [ebp-0xc]
0000B451  E80FF7FFFF        call 0xab65
0000B456  83C410            add esp,byte +0x10
0000B459  837DE400          cmp dword [ebp-0x1c],byte +0x0
0000B45D  7405              jz 0xb464
0000B45F  E854FEFFFF        call 0xb2b8
0000B464  A158DC0100        mov eax,[0x1dc58]
0000B469  3945F4            cmp [ebp-0xc],eax
0000B46C  750E              jnz 0xb47c
0000B46E  A15CDC0100        mov eax,[0x1dc5c]
0000B473  3945F0            cmp [ebp-0x10],eax
0000B476  0F848E000000      jz near 0xb50a
0000B47C  83EC08            sub esp,byte +0x8
0000B47F  FF75F0            push dword [ebp-0x10]
0000B482  FF75F4            push dword [ebp-0xc]
0000B485  E8DDFDFFFF        call 0xb267
0000B48A  83C410            add esp,byte +0x10
0000B48D  A160280200        mov eax,[0x22860]
0000B492  85C0              test eax,eax
0000B494  7413              jz 0xb4a9
0000B496  A160280200        mov eax,[0x22860]
0000B49B  8B4038            mov eax,[eax+0x38]
0000B49E  85C0              test eax,eax
0000B4A0  7407              jz 0xb4a9
0000B4A2  E831EBFFFF        call 0x9fd8
0000B4A7  EB51              jmp short 0xb4fa
0000B4A9  A160280200        mov eax,[0x22860]
0000B4AE  85C0              test eax,eax
0000B4B0  7437              jz 0xb4e9
0000B4B2  A160280200        mov eax,[0x22860]
0000B4B7  8B4048            mov eax,[eax+0x48]
0000B4BA  83F805            cmp eax,byte +0x5
0000B4BD  752A              jnz 0xb4e9
0000B4BF  E833B0FFFF        call 0x64f7
0000B4C4  83E001            and eax,byte +0x1
0000B4C7  85C0              test eax,eax
0000B4C9  741E              jz 0xb4e9
0000B4CB  A160280200        mov eax,[0x22860]
0000B4D0  83EC04            sub esp,byte +0x4
0000B4D3  FF75F0            push dword [ebp-0x10]
0000B4D6  FF75F4            push dword [ebp-0xc]
0000B4D9  50                push eax
0000B4DA  E882BBFFFF        call 0x7061
0000B4DF  83C410            add esp,byte +0x10
0000B4E2  E8F1EAFFFF        call 0x9fd8
0000B4E7  EB11              jmp short 0xb4fa
0000B4E9  83EC08            sub esp,byte +0x8
0000B4EC  FF75F0            push dword [ebp-0x10]
0000B4EF  FF75F4            push dword [ebp-0xc]
0000B4F2  E825EBFFFF        call 0xa01c
0000B4F7  83C410            add esp,byte +0x10
0000B4FA  8B45F4            mov eax,[ebp-0xc]
0000B4FD  A358DC0100        mov [0x1dc58],eax
0000B502  8B45F0            mov eax,[ebp-0x10]
0000B505  A35CDC0100        mov [0x1dc5c],eax
0000B50A  A160280200        mov eax,[0x22860]
0000B50F  85C0              test eax,eax
0000B511  7423              jz 0xb536
0000B513  A160280200        mov eax,[0x22860]
0000B518  8B4048            mov eax,[eax+0x48]
0000B51B  83F806            cmp eax,byte +0x6
0000B51E  7516              jnz 0xb536
0000B520  A160280200        mov eax,[0x22860]
0000B525  83EC0C            sub esp,byte +0xc
0000B528  50                push eax
0000B529  E829BCFFFF        call 0x7157
0000B52E  83C410            add esp,byte +0x10
0000B531  E8A2EAFFFF        call 0x9fd8
0000B536  8B45EC            mov eax,[ebp-0x14]
0000B539  A310290200        mov [0x22910],eax
0000B53E  90                nop
0000B53F  C9                leave
0000B540  C3                ret
0000B541  0000              add [eax],al
0000B543  0000              add [eax],al
0000B545  0000              add [eax],al
0000B547  0000              add [eax],al
0000B549  0000              add [eax],al
0000B54B  0000              add [eax],al
0000B54D  0000              add [eax],al
0000B54F  0000              add [eax],al
0000B551  0000              add [eax],al
0000B553  0000              add [eax],al
0000B555  0000              add [eax],al
0000B557  0000              add [eax],al
0000B559  0000              add [eax],al
0000B55B  0000              add [eax],al
0000B55D  0000              add [eax],al
0000B55F  004B45            add [ebx+0x45],cl
0000B562  52                push edx
0000B563  4E                dec esi
0000B564  45                inc ebp
0000B565  4C                dec esp
0000B566  20454E            and [ebp+0x4e],al
0000B569  54                push esp
0000B56A  52                push edx
0000B56B  59                pop ecx
0000B56C  20504F            and [eax+0x4f],dl
0000B56F  49                dec ecx
0000B570  4E                dec esi
0000B571  54                push esp
0000B572  0A00              or al,[eax]
0000B574  48                dec eax
0000B575  45                inc ebp
0000B576  41                inc ecx
0000B577  50                push eax
0000B578  20494E            and [ecx+0x4e],cl
0000B57B  49                dec ecx
0000B57C  54                push esp
0000B57D  204F4B            and [edi+0x4b],cl
0000B580  0A00              or al,[eax]
0000B582  47                inc edi
0000B583  44                inc esp
0000B584  54                push esp
0000B585  204F4B            and [edi+0x4b],cl
0000B588  0A00              or al,[eax]
0000B58A  49                dec ecx
0000B58B  44                inc esp
0000B58C  54                push esp
0000B58D  2F                das
0000B58E  49                dec ecx
0000B58F  53                push ebx
0000B590  52                push edx
0000B591  20534B            and [ebx+0x4b],dl
0000B594  49                dec ecx
0000B595  50                push eax
0000B596  50                push eax
0000B597  45                inc ebp
0000B598  44                inc esp
0000B599  2028              and [eax],ch
0000B59B  44                inc esp
0000B59C  45                inc ebp
0000B59D  42                inc edx
0000B59E  55                push ebp
0000B59F  47                inc edi
0000B5A0  290A              sub [edx],ecx
0000B5A2  00445249          add [edx+edx*2+0x49],al
0000B5A6  56                push esi
0000B5A7  45                inc ebp
0000B5A8  52                push edx
0000B5A9  53                push ebx
0000B5AA  20494E            and [ecx+0x4e],cl
0000B5AD  49                dec ecx
0000B5AE  54                push esp
0000B5AF  20534B            and [ebx+0x4b],dl
0000B5B2  49                dec ecx
0000B5B3  50                push eax
0000B5B4  50                push eax
0000B5B5  45                inc ebp
0000B5B6  44                inc esp
0000B5B7  0A00              or al,[eax]
0000B5B9  4C                dec esp
0000B5BA  46                inc esi
0000B5BB  42                inc edx
0000B5BC  204144            and [ecx+0x44],al
0000B5BF  44                inc esp
0000B5C0  52                push edx
0000B5C1  3A20              cmp ah,[eax]
0000B5C3  307800            xor [eax+0x0],bh
0000B5C6  0A00              or al,[eax]
0000B5C8  50                push eax
0000B5C9  55                push ebp
0000B5CA  52                push edx
0000B5CB  45                inc ebp
0000B5CC  4F                dec edi
0000B5CD  53                push ebx
0000B5CE  204B45            and [ebx+0x45],cl
0000B5D1  52                push edx
0000B5D2  4E                dec esi
0000B5D3  45                inc ebp
0000B5D4  4C                dec esp
0000B5D5  202D20475241      and [dword 0x41524720],ch
0000B5DB  50                push eax
0000B5DC  48                dec eax
0000B5DD  49                dec ecx
0000B5DE  43                inc ebx
0000B5DF  53                push ebx
0000B5E0  204D4F            and [ebp+0x4f],cl
0000B5E3  44                inc esp
0000B5E4  45                inc ebp
0000B5E5  0A00              or al,[eax]
0000B5E7  49                dec ecx
0000B5E8  4E                dec esi
0000B5E9  54                push esp
0000B5EA  45                inc ebp
0000B5EB  52                push edx
0000B5EC  52                push edx
0000B5ED  55                push ebp
0000B5EE  50                push eax
0000B5EF  54                push esp
0000B5F0  53                push ebx
0000B5F1  20454E            and [ebp+0x4e],al
0000B5F4  41                inc ecx
0000B5F5  42                inc edx
0000B5F6  4C                dec esp
0000B5F7  45                inc ebp
0000B5F8  44                inc esp
0000B5F9  0A00              or al,[eax]
0000B5FB  49                dec ecx
0000B5FC  6E                outsb
0000B5FD  7374              jnc 0xb673
0000B5FF  61                popa
0000B600  6C                insb
0000B601  6C                insb
0000B602  696E6720494454    imul ebp,[esi+0x67],dword 0x54444920
0000B609  2E2E2E0A00        or al,[cs:eax]
0000B60E  52                push edx
0000B60F  656D              gs insd
0000B611  61                popa
0000B612  7070              jo 0xb684
0000B614  696E6720504943    imul ebp,[esi+0x67],dword 0x43495020
0000B61B  2E2E2E0A00        or al,[cs:eax]
0000B620  2D2D2D204D        sub eax,0x4d202d2d
0000B625  656D              gs insd
0000B627  6F                outsd
0000B628  7279              jc 0xb6a3
0000B62A  205374            and [ebx+0x74],dl
0000B62D  61                popa
0000B62E  7469              jz 0xb699
0000B630  7374              jnc 0xb6a6
0000B632  696373202D2D2D    imul esp,[ebx+0x73],dword 0x2d2d2d20
0000B639  0A00              or al,[eax]
0000B63B  54                push esp
0000B63C  6F                outsd
0000B63D  7461              jz 0xb6a0
0000B63F  6C                insb
0000B640  3A20              cmp ah,[eax]
0000B642  313420            xor [eax],esi
0000B645  4D                dec ebp
0000B646  42                inc edx
0000B647  2028              and [eax],ch
0000B649  41                inc ecx
0000B64A  7070              jo 0xb6bc
0000B64C  726F              jc 0xb6bd
0000B64E  7829              js 0xb679
0000B650  0A00              or al,[eax]
0000B652  55                push ebp
0000B653  7365              jnc 0xb6ba
0000B655  64204279          and [fs:edx+0x79],al
0000B659  7465              jz 0xb6c0
0000B65B  733A              jnc 0xb697
0000B65D  2000              and [eax],al
0000B65F  0A00              or al,[eax]
0000B661  46                inc esi
0000B662  7265              jc 0xb6c9
0000B664  65204279          and [gs:edx+0x79],al
0000B668  7465              jz 0xb6cf
0000B66A  733A              jnc 0xb6a6
0000B66C  2000              and [eax],al
0000B66E  42                inc edx
0000B66F  6C                insb
0000B670  6F                outsd
0000B671  636B73            arpl [ebx+0x73],bp
0000B674  3A20              cmp ah,[eax]
0000B676  0000              add [eax],al
0000B678  5B                pop ebx
0000B679  46                inc esi
0000B67A  53                push ebx
0000B67B  5D                pop ebp
0000B67C  20446973          and [ecx+ebp*2+0x73],al
0000B680  6B2052            imul esp,[eax],byte +0x52
0000B683  6561              gs popa
0000B685  64204572          and [fs:ebp+0x72],al
0000B689  726F              jc 0xb6fa
0000B68B  7220              jc 0xb6ad
0000B68D  6F                outsd
0000B68E  6E                outsb
0000B68F  205365            and [ebx+0x65],dl
0000B692  63746F72          arpl [edi+ebp*2+0x72],si
0000B696  2030              and [eax],dh
0000B698  0A00              or al,[eax]
0000B69A  0000              add [eax],al
0000B69C  5B                pop ebx
0000B69D  46                inc esi
0000B69E  53                push ebx
0000B69F  5D                pop ebp
0000B6A0  204E6F            and [esi+0x6f],cl
0000B6A3  20426F            and [edx+0x6f],al
0000B6A6  6F                outsd
0000B6A7  7420              jz 0xb6c9
0000B6A9  53                push ebx
0000B6AA  69676E61747572    imul esp,[edi+0x6e],dword 0x72757461
0000B6B1  6520666F          and [gs:esi+0x6f],ah
0000B6B5  756E              jnz 0xb725
0000B6B7  642E0A00          or al,[cs:eax]
0000B6BB  44                inc esp
0000B6BC  69726563746F72    imul esi,[edx+0x65],dword 0x726f7463
0000B6C3  7920              jns 0xb6e5
0000B6C5  6F                outsd
0000B6C6  662000            o16 and [eax],al
0000B6C9  0A00              or al,[eax]
0000B6CB  2020              and [eax],ah
0000B6CD  3C44              cmp al,0x44
0000B6CF  49                dec ecx
0000B6D0  52                push edx
0000B6D1  3E0A00            or al,[ds:eax]
0000B6D4  2020              and [eax],ah
0000B6D6  46                inc esi
0000B6D7  49                dec ecx
0000B6D8  4C                dec esp
0000B6D9  45                inc ebp
0000B6DA  0A00              or al,[eax]
0000B6DC  28456D            sub [ebp+0x6d],al
0000B6DF  7074              jo 0xb755
0000B6E1  7929              jns 0xb70c
0000B6E3  0A00              or al,[eax]
0000B6E5  46                inc esi
0000B6E6  53                push ebx
0000B6E7  3A20              cmp ah,[eax]
0000B6E9  44                inc esp
0000B6EA  69726563746F72    imul esi,[edx+0x65],dword 0x726f7463
0000B6F1  7920              jns 0xb713
0000B6F3  46                inc esi
0000B6F4  756C              jnz 0xb762
0000B6F6  6C                insb
0000B6F7  2E0A00            or al,[cs:eax]
0000B6FA  46                inc esi
0000B6FB  53                push ebx
0000B6FC  3A20              cmp ah,[eax]
0000B6FE  44                inc esp
0000B6FF  69736B2046756C    imul esi,[ebx+0x6b],dword 0x6c754620
0000B706  6C                insb
0000B707  2E0A00            or al,[cs:eax]
0000B70A  2F                das
0000B70B  004E6F            add [esi+0x6f],cl
0000B70E  7420              jz 0xb730
0000B710  61                popa
0000B711  20646972          and [ecx+ebp*2+0x72],ah
0000B715  6563746F72        arpl [gs:edi+ebp*2+0x72],si
0000B71A  792E              jns 0xb74a
0000B71C  0A00              or al,[eax]
0000B71E  0000              add [eax],al
0000B720  0F0000            sldt [eax]
0000B723  0000              add [eax],al
0000B725  0000              add [eax],al
0000B727  000F              add [edi],cl
0000B729  0F0000            sldt [eax]
0000B72C  0000              add [eax],al
0000B72E  0000              add [eax],al
0000B730  0F                db 0x0f
0000B731  0F                db 0x0f
0000B732  0F0000            sldt [eax]
0000B735  0000              add [eax],al
0000B737  000F              add [edi],cl
0000B739  0F                db 0x0f
0000B73A  0F                db 0x0f
0000B73B  0F0000            sldt [eax]
0000B73E  0000              add [eax],al
0000B740  0F                db 0x0f
0000B741  0F                db 0x0f
0000B742  0F                db 0x0f
0000B743  0F                db 0x0f
0000B744  0F0000            sldt [eax]
0000B747  000F              add [edi],cl
0000B749  0F                db 0x0f
0000B74A  0F0000            sldt [eax]
0000B74D  0000              add [eax],al
0000B74F  000F              add [edi],cl
0000B751  000F              add [edi],cl
0000B753  0F0000            sldt [eax]
0000B756  0000              add [eax],al
0000B758  0000              add [eax],al
0000B75A  000F              add [edi],cl
0000B75C  0000              add [eax],al
0000B75E  0000              add [eax],al
0000B760  001B              add [ebx],bl
0000B762  3132              xor [edx],esi
0000B764  33343536373839    xor esi,[esi+0x39383736]
0000B76B  302D3D080971      xor [dword 0x7109083d],ch
0000B771  7765              ja 0xb7d8
0000B773  7274              jc 0xb7e9
0000B775  7975              jns 0xb7ec
0000B777  696F705B5D0A00    imul ebp,[edi+0x70],dword 0xa5d5b
0000B77E  61                popa
0000B77F  7364              jnc 0xb7e5
0000B781  6667686A6B        push word 0x6b6a
0000B786  6C                insb
0000B787  3B27              cmp esp,[edi]
0000B789  60                pusha
0000B78A  005C7A78          add [edx+edi*2+0x78],bl
0000B78E  637662            arpl [esi+0x62],si
0000B791  6E                outsb
0000B792  6D                insd
0000B793  2C2E              sub al,0x2e
0000B795  2F                das
0000B796  002A              add [edx],ch
0000B798  0020              add [eax],ah
0000B79A  0000              add [eax],al
0000B79C  0000              add [eax],al
0000B79E  0000              add [eax],al
0000B7A0  0000              add [eax],al
0000B7A2  0000              add [eax],al
0000B7A4  0000              add [eax],al
0000B7A6  0000              add [eax],al
0000B7A8  0000              add [eax],al
0000B7AA  2D0000002B        sub eax,0x2b000000
0000B7AF  0000              add [eax],al
0000B7B1  0000              add [eax],al
0000B7B3  0000              add [eax],al
0000B7B5  0000              add [eax],al
0000B7B7  0000              add [eax],al
0000B7B9  0000              add [eax],al
0000B7BB  0000              add [eax],al
0000B7BD  0000              add [eax],al
0000B7BF  0000              add [eax],al
0000B7C1  1B21              sbb esp,[ecx]
0000B7C3  40                inc eax
0000B7C4  2324255E262A28    and esp,[0x282a265e]
0000B7CB  295F2B            sub [edi+0x2b],ebx
0000B7CE  0809              or [ecx],cl
0000B7D0  51                push ecx
0000B7D1  57                push edi
0000B7D2  45                inc ebp
0000B7D3  52                push edx
0000B7D4  54                push esp
0000B7D5  59                pop ecx
0000B7D6  55                push ebp
0000B7D7  49                dec ecx
0000B7D8  4F                dec edi
0000B7D9  50                push eax
0000B7DA  7B7D              jpo 0xb859
0000B7DC  0A00              or al,[eax]
0000B7DE  41                inc ecx
0000B7DF  53                push ebx
0000B7E0  44                inc esp
0000B7E1  46                inc esi
0000B7E2  47                inc edi
0000B7E3  48                dec eax
0000B7E4  4A                dec edx
0000B7E5  4B                dec ebx
0000B7E6  4C                dec esp
0000B7E7  3A22              cmp ah,[edx]
0000B7E9  7E00              jng 0xb7eb
0000B7EB  7C5A              jl 0xb847
0000B7ED  58                pop eax
0000B7EE  43                inc ebx
0000B7EF  56                push esi
0000B7F0  42                inc edx
0000B7F1  4E                dec esi
0000B7F2  4D                dec ebp
0000B7F3  3C3E              cmp al,0x3e
0000B7F5  3F                aas
0000B7F6  002A              add [edx],ch
0000B7F8  0020              add [eax],ah
0000B7FA  0000              add [eax],al
0000B7FC  0000              add [eax],al
0000B7FE  0000              add [eax],al
0000B800  0000              add [eax],al
0000B802  0000              add [eax],al
0000B804  0000              add [eax],al
0000B806  0000              add [eax],al
0000B808  0000              add [eax],al
0000B80A  2D0000002B        sub eax,0x2b000000
0000B80F  0000              add [eax],al
0000B811  0000              add [eax],al
0000B813  0000              add [eax],al
0000B815  0000              add [eax],al
0000B817  0000              add [eax],al
0000B819  0000              add [eax],al
0000B81B  003D3D3D3D3D      add [dword 0x3d3d3d3d],bh
0000B821  3D3D3D3D3D        cmp eax,0x3d3d3d3d
0000B826  3D3D3D3D3D        cmp eax,0x3d3d3d3d
0000B82B  3D3D3D3D3D        cmp eax,0x3d3d3d3d
0000B830  3D3D3D3D3D        cmp eax,0x3d3d3d3d
0000B835  3D0A002020        cmp eax,0x2020000a
0000B83A  2020              and [eax],ah
0000B83C  2020              and [eax],ah
0000B83E  205075            and [eax+0x75],dl
0000B841  7265              jc 0xb8a8
0000B843  4F                dec edi
0000B844  53                push ebx
0000B845  20496E            and [ecx+0x6e],cl
0000B848  666F              outsw
0000B84A  2020              and [eax],ah
0000B84C  2020              and [eax],ah
0000B84E  2020              and [eax],ah
0000B850  2020              and [eax],ah
0000B852  0A00              or al,[eax]
0000B854  4B                dec ebx
0000B855  65726E            gs jc 0xb8c6
0000B858  656C              gs insb
0000B85A  3A20              cmp ah,[eax]
0000B85C  7630              jna 0xb88e
0000B85E  2E3520283332      cs xor eax,0x32332820
0000B864  2D62697429        sub eax,0x29746962
0000B869  0A00              or al,[eax]
0000B86B  53                push ebx
0000B86C  68656C6C3A        push dword 0x3a6c6c65
0000B871  2020              and [eax],ah
0000B873  7630              jna 0xb8a5
0000B875  2E3220            xor ah,[cs:eax]
0000B878  45                inc ebp
0000B879  6E                outsb
0000B87A  68616E6365        push dword 0x65636e61
0000B87F  640A00            or al,[fs:eax]
0000B882  56                push esi
0000B883  6964656F3A202056  imul esp,[ebp+0x6f],dword 0x5620203a
0000B88B  47                inc edi
0000B88C  41                inc ecx
0000B88D  20546578          and [ebp+0x78],dl
0000B891  7420              jz 0xb8b3
0000B893  4D                dec ebp
0000B894  6F                outsd
0000B895  64650A00          or al,[gs:eax]
0000B899  49                dec ecx
0000B89A  6E                outsb
0000B89B  7465              jz 0xb902
0000B89D  7272              jc 0xb911
0000B89F  7570              jnz 0xb911
0000B8A1  7473              jz 0xb916
0000B8A3  3A20              cmp ah,[eax]
0000B8A5  45                inc ebp
0000B8A6  6E                outsb
0000B8A7  61                popa
0000B8A8  626C6564          bound ebp,[ebp+0x64]
0000B8AC  2028              and [eax],ch
0000B8AE  50                push eax
0000B8AF  49                dec ecx
0000B8B0  43                inc ebx
0000B8B1  290A              sub [edx],ecx
0000B8B3  00447269          add [edx+esi*2+0x69],al
0000B8B7  7665              jna 0xb91e
0000B8B9  723A              jc 0xb8f5
0000B8BB  204B65            and [ebx+0x65],cl
0000B8BE  7962              jns 0xb922
0000B8C0  6F                outsd
0000B8C1  61                popa
0000B8C2  7264              jc 0xb928
0000B8C4  2028              and [eax],ch
0000B8C6  49                dec ecx
0000B8C7  52                push edx
0000B8C8  51                push ecx
0000B8C9  3129              xor [ecx],ebp
0000B8CB  0A00              or al,[eax]
0000B8CD  44                inc esp
0000B8CE  7269              jc 0xb939
0000B8D0  7665              jna 0xb937
0000B8D2  723A              jc 0xb90e
0000B8D4  2054696D          and [ecx+ebp*2+0x6d],dl
0000B8D8  657220            gs jc 0xb8fb
0000B8DB  284952            sub [ecx+0x52],cl
0000B8DE  51                push ecx
0000B8DF  3029              xor [ecx],ch
0000B8E1  0A00              or al,[eax]
0000B8E3  52                push edx
0000B8E4  65626F6F          bound ebp,[gs:edi+0x6f]
0000B8E8  7469              jz 0xb953
0000B8EA  6E                outsb
0000B8EB  67207379          and [bp+di+0x79],dh
0000B8EF  7374              jnc 0xb965
0000B8F1  656D              gs insd
0000B8F3  2E2E2E0A00        or al,[cs:eax]
0000B8F8  41                inc ecx
0000B8F9  7661              jna 0xb95c
0000B8FB  696C61626C652043  imul ebp,[ecx+0x62],dword 0x4320656c
0000B903  6F                outsd
0000B904  6D                insd
0000B905  6D                insd
0000B906  61                popa
0000B907  6E                outsb
0000B908  64733A            fs jnc 0xb945
0000B90B  0A00              or al,[eax]
0000B90D  2020              and [eax],ah
0000B90F  68656C7020        push dword 0x20706c65
0000B914  2020              and [eax],ah
0000B916  2D2053686F        sub eax,0x6f685320
0000B91B  7720              ja 0xb93d
0000B91D  7468              jz 0xb987
0000B91F  6973206C697374    imul esi,[ebx+0x20],dword 0x7473696c
0000B926  0A00              or al,[eax]
0000B928  2020              and [eax],ah
0000B92A  696E666F202020    imul ebp,[esi+0x66],dword 0x2020206f
0000B931  2D20537973        sub eax,0x73795320
0000B936  7465              jz 0xb99d
0000B938  6D                insd
0000B939  20696E            and [ecx+0x6e],ch
0000B93C  666F              outsw
0000B93E  726D              jc 0xb9ad
0000B940  61                popa
0000B941  7469              jz 0xb9ac
0000B943  6F                outsd
0000B944  6E                outsb
0000B945  0A00              or al,[eax]
0000B947  2020              and [eax],ah
0000B949  636C6561          arpl [ebp+0x61],bp
0000B94D  7220              jc 0xb96f
0000B94F  202D20436C65      and [dword 0x656c4320],ch
0000B955  61                popa
0000B956  7220              jc 0xb978
0000B958  7363              jnc 0xb9bd
0000B95A  7265              jc 0xb9c1
0000B95C  656E              gs outsb
0000B95E  0A00              or al,[eax]
0000B960  2020              and [eax],ah
0000B962  6563686F          arpl [gs:eax+0x6f],bp
0000B966  2020              and [eax],ah
0000B968  202D20507269      and [dword 0x69725020],ch
0000B96E  6E                outsb
0000B96F  7420              jz 0xb991
0000B971  7465              jz 0xb9d8
0000B973  7874              js 0xb9e9
0000B975  0A00              or al,[eax]
0000B977  2020              and [eax],ah
0000B979  636F6C            arpl [edi+0x6c],bp
0000B97C  6F                outsd
0000B97D  7220              jc 0xb99f
0000B97F  202D20437963      and [dword 0x63794320],ch
0000B985  6C                insb
0000B986  6520746578        and [gs:ebp+0x78],dh
0000B98B  7420              jz 0xb9ad
0000B98D  636F6C            arpl [edi+0x6c],bp
0000B990  6F                outsd
0000B991  7273              jc 0xba06
0000B993  0A00              or al,[eax]
0000B995  2020              and [eax],ah
0000B997  7265              jc 0xb9fe
0000B999  626F6F            bound ebp,[edi+0x6f]
0000B99C  7420              jz 0xb9be
0000B99E  2D20526573        sub eax,0x73655220
0000B9A3  7461              jz 0xba06
0000B9A5  7274              jc 0xba1b
0000B9A7  205043            and [eax+0x43],dl
0000B9AA  0A00              or al,[eax]
0000B9AC  68656C7000        push dword 0x706c65
0000B9B1  696E666F00636C    imul ebp,[esi+0x66],dword 0x6c63006f
0000B9B8  6561              gs popa
0000B9BA  7200              jc 0xb9bc
0000B9BC  7265              jc 0xba23
0000B9BE  626F6F            bound ebp,[edi+0x6f]
0000B9C1  7400              jz 0xb9c3
0000B9C3  636F6C            arpl [edi+0x6c],bp
0000B9C6  6F                outsd
0000B9C7  7200              jc 0xb9c9
0000B9C9  43                inc ebx
0000B9CA  6F                outsd
0000B9CB  6C                insb
0000B9CC  6F                outsd
0000B9CD  7220              jc 0xb9ef
0000B9CF  636861            arpl [eax+0x61],bp
0000B9D2  6E                outsb
0000B9D3  6765642E0A00      or al,[cs:bx+si]
0000B9D9  6563686F          arpl [gs:eax+0x6f],bp
0000B9DD  2000              and [eax],al
0000B9DF  0A00              or al,[eax]
0000B9E1  6C                insb
0000B9E2  7300              jnc 0xb9e4
0000B9E4  6465627567        bound esi,[gs:ebp+0x67]
0000B9E9  667300            o16 jnc 0xb9ec
0000B9EC  53                push ebx
0000B9ED  6563746F72        arpl [gs:edi+ebp*2+0x72],si
0000B9F2  2030              and [eax],dh
0000B9F4  2044756D          and [ebp+esi*2+0x6d],al
0000B9F8  703A              jo 0xba34
0000B9FA  0A00              or al,[eax]
0000B9FC  53                push ebx
0000B9FD  69673A20004F45    imul esp,[edi+0x3a],dword 0x454f0020
0000BA04  4D                dec ebp
0000BA05  3A20              cmp ah,[eax]
0000BA07  00547970          add [ecx+edi*2+0x70],dl
0000BA0B  653A20            cmp ah,[gs:eax]
0000BA0E  00746F75          add [edi+ebp*2+0x75],dh
0000BA12  636820            arpl [eax+0x20],bp
0000BA15  004372            add [ebx+0x72],al
0000BA18  6561              gs popa
0000BA1A  7469              jz 0xba85
0000BA1C  6E                outsb
0000BA1D  67206669          and [bp+0x69],ah
0000BA21  6C                insb
0000BA22  653A20            cmp ah,[gs:eax]
0000BA25  0000              add [eax],al
0000BA27  000A              add [edx],cl
0000BA29  284665            sub [esi+0x65],al
0000BA2C  61                popa
0000BA2D  7475              jz 0xbaa4
0000BA2F  7265              jc 0xba96
0000BA31  207065            and [eax+0x65],dh
0000BA34  6E                outsb
0000BA35  64696E6720777269  imul ebp,[fs:esi+0x67],dword 0x69727720
0000BA3D  7465              jz 0xbaa4
0000BA3F  207375            and [ebx+0x75],dh
0000BA42  7070              jo 0xbab4
0000BA44  6F                outsd
0000BA45  7274              jc 0xbabb
0000BA47  290A              sub [edx],ecx
0000BA49  006361            add [ebx+0x61],ah
0000BA4C  7420              jz 0xba6e
0000BA4E  005265            add [edx+0x65],dl
0000BA51  61                popa
0000BA52  64696E673A200046  imul ebp,[fs:esi+0x67],dword 0x4600203a
0000BA5A  696C65206E6F7420  imul ebp,[ebp+0x20],dword 0x20746f6e
0000BA62  666F              outsw
0000BA64  756E              jnz 0xbad4
0000BA66  642E0A00          or al,[cs:eax]
0000BA6A  6D                insd
0000BA6B  656D              gs insd
0000BA6D  004865            add [eax+0x65],cl
0000BA70  61                popa
0000BA71  703A              jo 0xbaad
0000BA73  20696E            and [ecx+0x6e],ch
0000BA76  697469616C697A65  imul esi,[ecx+ebp*2+0x61],dword 0x657a696c
0000BA7E  642028            and [fs:eax],ch
0000BA81  324D42            xor cl,[ebp+0x42]
0000BA84  2D31364D42        sub eax,0x424d3631
0000BA89  290A              sub [edx],ecx
0000BA8B  00646973          add [ecx+ebp*2+0x73],ah
0000BA8F  6B2000            imul esp,[eax],byte +0x0
0000BA92  52                push edx
0000BA93  6561              gs popa
0000BA95  64696E6720646973  imul ebp,[fs:esi+0x67],dword 0x73696420
0000BA9D  6B2E2E            imul ebp,[esi],byte +0x2e
0000BAA0  2E0A00            or al,[cs:eax]
0000BAA3  52                push edx
0000BAA4  6561              gs popa
0000BAA6  64204572          and [fs:ebp+0x72],al
0000BAAA  726F              jc 0xbb1b
0000BAAC  722E              jc 0xbadc
0000BAAE  0A00              or al,[eax]
0000BAB0  7061              jo 0xbb13
0000BAB2  696E7400000045    imul ebp,[esi+0x74],dword 0x45000000
0000BAB9  7272              jc 0xbb2d
0000BABB  6F                outsd
0000BABC  723A              jc 0xbaf8
0000BABE  204361            and [ebx+0x61],al
0000BAC1  6E                outsb
0000BAC2  6E                outsb
0000BAC3  6F                outsd
0000BAC4  7420              jz 0xbae6
0000BAC6  7377              jnc 0xbb3f
0000BAC8  6974636820766964  imul esi,[ebx+0x68],dword 0x64697620
0000BAD0  656F              gs outsd
0000BAD2  206D6F            and [ebp+0x6f],ch
0000BAD5  646520696E        and [gs:ecx+0x6e],ch
0000BADA  20736F            and [ebx+0x6f],dh
0000BADD  667477            o16 jz 0xbb57
0000BAE0  61                popa
0000BAE1  7265              jc 0xbb48
0000BAE3  2E0A00            or al,[cs:eax]
0000BAE6  0000              add [eax],al
0000BAE8  50                push eax
0000BAE9  6C                insb
0000BAEA  6561              gs popa
0000BAEC  7365              jnc 0xbb53
0000BAEE  207265            and [edx+0x65],dh
0000BAF1  626F6F            bound ebp,[edi+0x6f]
0000BAF4  7420              jz 0xbb16
0000BAF6  61                popa
0000BAF7  6E                outsb
0000BAF8  64206368          and [fs:ebx+0x68],ah
0000BAFC  6F                outsd
0000BAFD  6F                outsd
0000BAFE  7365              jnc 0xbb65
0000BB00  204772            and [edi+0x72],al
0000BB03  61                popa
0000BB04  7068              jo 0xbb6e
0000BB06  696373204D6F64    imul esp,[ebx+0x73],dword 0x646f4d20
0000BB0D  652E0A00          or al,[cs:eax]
0000BB11  55                push ebp
0000BB12  6E                outsb
0000BB13  6B6E6F77          imul ebp,[esi+0x6f],byte +0x77
0000BB17  6E                outsb
0000BB18  20636F            and [ebx+0x6f],ah
0000BB1B  6D                insd
0000BB1C  6D                insd
0000BB1D  61                popa
0000BB1E  6E                outsb
0000BB1F  643A20            cmp ah,[fs:eax]
0000BB22  0000              add [eax],al
0000BB24  50                push eax
0000BB25  7572              jnz 0xbb99
0000BB27  654F              gs dec edi
0000BB29  53                push ebx
0000BB2A  205368            and [ebx+0x68],dl
0000BB2D  656C              gs insb
0000BB2F  6C                insb
0000BB30  207630            and [esi+0x30],dh
0000BB33  2E322E            xor ch,[cs:esi]
0000BB36  20547970          and [ecx+edi*2+0x70],dl
0000BB3A  652027            and [gs:edi],ah
0000BB3D  68656C7027        push dword 0x27706c65
0000BB42  2E0A00            or al,[cs:eax]
0000BB45  50                push eax
0000BB46  7572              jnz 0xbbba
0000BB48  654F              gs dec edi
0000BB4A  53                push ebx
0000BB4B  3E2000            and [ds:eax],al
0000BB4E  0820              or [eax],ah
0000BB50  0800              or [eax],al
0000BB52  0000              add [eax],al
0000BB54  3031              xor [ecx],dh
0000BB56  3233              xor dh,[ebx]
0000BB58  3435              xor al,0x35
0000BB5A  3637              ss aaa
0000BB5C  3839              cmp [ecx],bh
0000BB5E  41                inc ecx
0000BB5F  42                inc edx
0000BB60  43                inc ebx
0000BB61  44                inc esp
0000BB62  45                inc ebp
0000BB63  46                inc esi
0000BB64  0000              add [eax],al
0000BB66  0000              add [eax],al
0000BB68  0000              add [eax],al
0000BB6A  0000              add [eax],al
0000BB6C  0000              add [eax],al
0000BB6E  0000              add [eax],al
0000BB70  0000              add [eax],al
0000BB72  0000              add [eax],al
0000BB74  0000              add [eax],al
0000BB76  0000              add [eax],al
0000BB78  0000              add [eax],al
0000BB7A  0000              add [eax],al
0000BB7C  0000              add [eax],al
0000BB7E  0000              add [eax],al
0000BB80  0100              add [eax],eax
0000BB82  0000              add [eax],al
0000BB84  0000              add [eax],al
0000BB86  0000              add [eax],al
0000BB88  0000              add [eax],al
0000BB8A  0000              add [eax],al
0000BB8C  0101              add [ecx],eax
0000BB8E  0000              add [eax],al
0000BB90  0000              add [eax],al
0000BB92  0000              add [eax],al
0000BB94  0000              add [eax],al
0000BB96  0000              add [eax],al
0000BB98  0101              add [ecx],eax
0000BB9A  0100              add [eax],eax
0000BB9C  0000              add [eax],al
0000BB9E  0000              add [eax],al
0000BBA0  0000              add [eax],al
0000BBA2  0000              add [eax],al
0000BBA4  0101              add [ecx],eax
0000BBA6  0101              add [ecx],eax
0000BBA8  0000              add [eax],al
0000BBAA  0000              add [eax],al
0000BBAC  0000              add [eax],al
0000BBAE  0000              add [eax],al
0000BBB0  0101              add [ecx],eax
0000BBB2  0101              add [ecx],eax
0000BBB4  0100              add [eax],eax
0000BBB6  0000              add [eax],al
0000BBB8  0000              add [eax],al
0000BBBA  0000              add [eax],al
0000BBBC  0101              add [ecx],eax
0000BBBE  0101              add [ecx],eax
0000BBC0  0101              add [ecx],eax
0000BBC2  0000              add [eax],al
0000BBC4  0000              add [eax],al
0000BBC6  0000              add [eax],al
0000BBC8  0101              add [ecx],eax
0000BBCA  0101              add [ecx],eax
0000BBCC  0101              add [ecx],eax
0000BBCE  0100              add [eax],eax
0000BBD0  0000              add [eax],al
0000BBD2  0000              add [eax],al
0000BBD4  0101              add [ecx],eax
0000BBD6  0101              add [ecx],eax
0000BBD8  0101              add [ecx],eax
0000BBDA  0101              add [ecx],eax
0000BBDC  0000              add [eax],al
0000BBDE  0000              add [eax],al
0000BBE0  0101              add [ecx],eax
0000BBE2  0101              add [ecx],eax
0000BBE4  0101              add [ecx],eax
0000BBE6  0101              add [ecx],eax
0000BBE8  0100              add [eax],eax
0000BBEA  0000              add [eax],al
0000BBEC  0101              add [ecx],eax
0000BBEE  0101              add [ecx],eax
0000BBF0  0101              add [ecx],eax
0000BBF2  0101              add [ecx],eax
0000BBF4  0101              add [ecx],eax
0000BBF6  0000              add [eax],al
0000BBF8  0101              add [ecx],eax
0000BBFA  0101              add [ecx],eax
0000BBFC  0101              add [ecx],eax
0000BBFE  0000              add [eax],al
0000BC00  0000              add [eax],al
0000BC02  0000              add [eax],al
0000BC04  0101              add [ecx],eax
0000BC06  0000              add [eax],al
0000BC08  0101              add [ecx],eax
0000BC0A  0000              add [eax],al
0000BC0C  0000              add [eax],al
0000BC0E  0000              add [eax],al
0000BC10  0100              add [eax],eax
0000BC12  0000              add [eax],al
0000BC14  0101              add [ecx],eax
0000BC16  0100              add [eax],eax
0000BC18  0000              add [eax],al
0000BC1A  0000              add [eax],al
0000BC1C  0000              add [eax],al
0000BC1E  0000              add [eax],al
0000BC20  0001              add [ecx],al
0000BC22  0101              add [ecx],eax
0000BC24  0000              add [eax],al
0000BC26  0000              add [eax],al
0000BC28  0000              add [eax],al
0000BC2A  0000              add [eax],al
0000BC2C  0001              add [ecx],al
0000BC2E  0101              add [ecx],eax
0000BC30  0000              add [eax],al
0000BC32  0000              add [eax],al
0000BC34  0000              add [eax],al
0000BC36  0000              add [eax],al
0000BC38  0000              add [eax],al
0000BC3A  0101              add [ecx],eax
0000BC3C  0100              add [eax],eax
0000BC3E  0000              add [eax],al
0000BC40  0000              add [eax],al
0000BC42  0000              add [eax],al
0000BC44  0000              add [eax],al
0000BC46  0101              add [ecx],eax
0000BC48  0100              add [eax],eax
0000BC4A  0000              add [eax],al
0000BC4C  0000              add [eax],al
0000BC4E  0000              add [eax],al
0000BC50  0000              add [eax],al
0000BC52  0001              add [ecx],al
0000BC54  0100              add [eax],eax
0000BC56  0000              add [eax],al
0000BC58  0000              add [eax],al
0000BC5A  0000              add [eax],al
0000BC5C  0000              add [eax],al
0000BC5E  0000              add [eax],al
0000BC60  0000              add [eax],al
0000BC62  0000              add [eax],al
0000BC64  0000              add [eax],al
0000BC66  0000              add [eax],al
0000BC68  0000              add [eax],al
0000BC6A  0000              add [eax],al
0000BC6C  0000              add [eax],al
0000BC6E  0000              add [eax],al
0000BC70  0000              add [eax],al
0000BC72  0000              add [eax],al
0000BC74  0000              add [eax],al
0000BC76  0000              add [eax],al
0000BC78  0000              add [eax],al
0000BC7A  0000              add [eax],al
0000BC7C  0000              add [eax],al
0000BC7E  0000              add [eax],al
0000BC80  0000              add [eax],al
0000BC82  0000              add [eax],al
0000BC84  0000              add [eax],al
0000BC86  0000              add [eax],al
0000BC88  0000              add [eax],al
0000BC8A  0000              add [eax],al
0000BC8C  0000              add [eax],al
0000BC8E  0000              add [eax],al
0000BC90  0000              add [eax],al
0000BC92  0001              add [ecx],al
0000BC94  0101              add [ecx],eax
0000BC96  0101              add [ecx],eax
0000BC98  0101              add [ecx],eax
0000BC9A  0101              add [ecx],eax
0000BC9C  0100              add [eax],eax
0000BC9E  0000              add [eax],al
0000BCA0  0000              add [eax],al
0000BCA2  0001              add [ecx],al
0000BCA4  0303              add eax,[ebx]
0000BCA6  0303              add eax,[ebx]
0000BCA8  0303              add eax,[ebx]
0000BCAA  0301              add eax,[ecx]
0000BCAC  0201              add al,[ecx]
0000BCAE  0000              add [eax],al
0000BCB0  0000              add [eax],al
0000BCB2  0001              add [ecx],al
0000BCB4  0303              add eax,[ebx]
0000BCB6  0303              add eax,[ebx]
0000BCB8  0303              add eax,[ebx]
0000BCBA  0301              add eax,[ecx]
0000BCBC  0201              add al,[ecx]
0000BCBE  0000              add [eax],al
0000BCC0  0000              add [eax],al
0000BCC2  0001              add [ecx],al
0000BCC4  0303              add eax,[ebx]
0000BCC6  0303              add eax,[ebx]
0000BCC8  0303              add eax,[ebx]
0000BCCA  0301              add eax,[ecx]
0000BCCC  0201              add al,[ecx]
0000BCCE  0000              add [eax],al
0000BCD0  0000              add [eax],al
0000BCD2  0001              add [ecx],al
0000BCD4  0303              add eax,[ebx]
0000BCD6  0303              add eax,[ebx]
0000BCD8  0303              add eax,[ebx]
0000BCDA  0301              add eax,[ecx]
0000BCDC  0201              add al,[ecx]
0000BCDE  0000              add [eax],al
0000BCE0  0000              add [eax],al
0000BCE2  0001              add [ecx],al
0000BCE4  0101              add [ecx],eax
0000BCE6  0101              add [ecx],eax
0000BCE8  0101              add [ecx],eax
0000BCEA  0101              add [ecx],eax
0000BCEC  0201              add al,[ecx]
0000BCEE  0000              add [eax],al
0000BCF0  0000              add [eax],al
0000BCF2  0001              add [ecx],al
0000BCF4  0404              add al,0x4
0000BCF6  0404              add al,0x4
0000BCF8  0404              add al,0x4
0000BCFA  0401              add al,0x1
0000BCFC  0201              add al,[ecx]
0000BCFE  0000              add [eax],al
0000BD00  0000              add [eax],al
0000BD02  0001              add [ecx],al
0000BD04  0101              add [ecx],eax
0000BD06  0101              add [ecx],eax
0000BD08  0101              add [ecx],eax
0000BD0A  0101              add [ecx],eax
0000BD0C  0201              add al,[ecx]
0000BD0E  0000              add [eax],al
0000BD10  0000              add [eax],al
0000BD12  0000              add [eax],al
0000BD14  0000              add [eax],al
0000BD16  0000              add [eax],al
0000BD18  0101              add [ecx],eax
0000BD1A  0102              add [edx],eax
0000BD1C  0201              add al,[ecx]
0000BD1E  0000              add [eax],al
0000BD20  0000              add [eax],al
0000BD22  0000              add [eax],al
0000BD24  0000              add [eax],al
0000BD26  0000              add [eax],al
0000BD28  0102              add [edx],eax
0000BD2A  0202              add al,[edx]
0000BD2C  0201              add al,[ecx]
0000BD2E  0000              add [eax],al
0000BD30  0000              add [eax],al
0000BD32  0000              add [eax],al
0000BD34  0000              add [eax],al
0000BD36  0000              add [eax],al
0000BD38  0102              add [edx],eax
0000BD3A  0402              add al,0x2
0000BD3C  0201              add al,[ecx]
0000BD3E  0000              add [eax],al
0000BD40  0000              add [eax],al
0000BD42  0000              add [eax],al
0000BD44  0000              add [eax],al
0000BD46  0000              add [eax],al
0000BD48  0102              add [edx],eax
0000BD4A  0202              add al,[edx]
0000BD4C  0201              add al,[ecx]
0000BD4E  0000              add [eax],al
0000BD50  0000              add [eax],al
0000BD52  0000              add [eax],al
0000BD54  0000              add [eax],al
0000BD56  0000              add [eax],al
0000BD58  0101              add [ecx],eax
0000BD5A  0101              add [ecx],eax
0000BD5C  0101              add [ecx],eax
0000BD5E  0000              add [eax],al
0000BD60  0000              add [eax],al
0000BD62  0000              add [eax],al
0000BD64  0000              add [eax],al
0000BD66  0000              add [eax],al
0000BD68  0000              add [eax],al
0000BD6A  0000              add [eax],al
0000BD6C  0000              add [eax],al
0000BD6E  0000              add [eax],al
0000BD70  0000              add [eax],al
0000BD72  0000              add [eax],al
0000BD74  0000              add [eax],al
0000BD76  0000              add [eax],al
0000BD78  0000              add [eax],al
0000BD7A  0000              add [eax],al
0000BD7C  0000              add [eax],al
0000BD7E  0000              add [eax],al
0000BD80  0000              add [eax],al
0000BD82  0000              add [eax],al
0000BD84  0000              add [eax],al
0000BD86  0000              add [eax],al
0000BD88  0000              add [eax],al
0000BD8A  0000              add [eax],al
0000BD8C  0000              add [eax],al
0000BD8E  0000              add [eax],al
0000BD90  0000              add [eax],al
0000BD92  0000              add [eax],al
0000BD94  0101              add [ecx],eax
0000BD96  0101              add [ecx],eax
0000BD98  0000              add [eax],al
0000BD9A  0000              add [eax],al
0000BD9C  0000              add [eax],al
0000BD9E  0000              add [eax],al
0000BDA0  0000              add [eax],al
0000BDA2  0001              add [ecx],al
0000BDA4  06                push es
0000BDA5  06                push es
0000BDA6  06                push es
0000BDA7  06                push es
0000BDA8  0100              add [eax],eax
0000BDAA  0000              add [eax],al
0000BDAC  0000              add [eax],al
0000BDAE  0000              add [eax],al
0000BDB0  0000              add [eax],al
0000BDB2  0106              add [esi],eax
0000BDB4  06                push es
0000BDB5  06                push es
0000BDB6  06                push es
0000BDB7  06                push es
0000BDB8  06                push es
0000BDB9  0101              add [ecx],eax
0000BDBB  0101              add [ecx],eax
0000BDBD  0100              add [eax],eax
0000BDBF  0000              add [eax],al
0000BDC1  0001              add [ecx],al
0000BDC3  06                push es
0000BDC4  06                push es
0000BDC5  06                push es
0000BDC6  06                push es
0000BDC7  06                push es
0000BDC8  06                push es
0000BDC9  06                push es
0000BDCA  06                push es
0000BDCB  06                push es
0000BDCC  06                push es
0000BDCD  06                push es
0000BDCE  0100              add [eax],eax
0000BDD0  0000              add [eax],al
0000BDD2  0106              add [esi],eax
0000BDD4  06                push es
0000BDD5  06                push es
0000BDD6  06                push es
0000BDD7  06                push es
0000BDD8  06                push es
0000BDD9  06                push es
0000BDDA  06                push es
0000BDDB  06                push es
0000BDDC  06                push es
0000BDDD  06                push es
0000BDDE  0100              add [eax],eax
0000BDE0  0000              add [eax],al
0000BDE2  0106              add [esi],eax
0000BDE4  06                push es
0000BDE5  06                push es
0000BDE6  06                push es
0000BDE7  06                push es
0000BDE8  06                push es
0000BDE9  06                push es
0000BDEA  06                push es
0000BDEB  06                push es
0000BDEC  06                push es
0000BDED  06                push es
0000BDEE  0100              add [eax],eax
0000BDF0  0000              add [eax],al
0000BDF2  0106              add [esi],eax
0000BDF4  06                push es
0000BDF5  06                push es
0000BDF6  06                push es
0000BDF7  06                push es
0000BDF8  06                push es
0000BDF9  06                push es
0000BDFA  06                push es
0000BDFB  06                push es
0000BDFC  06                push es
0000BDFD  06                push es
0000BDFE  0100              add [eax],eax
0000BE00  0000              add [eax],al
0000BE02  0106              add [esi],eax
0000BE04  06                push es
0000BE05  06                push es
0000BE06  06                push es
0000BE07  06                push es
0000BE08  06                push es
0000BE09  06                push es
0000BE0A  06                push es
0000BE0B  06                push es
0000BE0C  06                push es
0000BE0D  06                push es
0000BE0E  0100              add [eax],eax
0000BE10  0000              add [eax],al
0000BE12  0106              add [esi],eax
0000BE14  06                push es
0000BE15  06                push es
0000BE16  06                push es
0000BE17  06                push es
0000BE18  06                push es
0000BE19  06                push es
0000BE1A  06                push es
0000BE1B  06                push es
0000BE1C  06                push es
0000BE1D  06                push es
0000BE1E  0100              add [eax],eax
0000BE20  0000              add [eax],al
0000BE22  0106              add [esi],eax
0000BE24  06                push es
0000BE25  06                push es
0000BE26  06                push es
0000BE27  06                push es
0000BE28  06                push es
0000BE29  06                push es
0000BE2A  06                push es
0000BE2B  06                push es
0000BE2C  06                push es
0000BE2D  06                push es
0000BE2E  0100              add [eax],eax
0000BE30  0000              add [eax],al
0000BE32  0106              add [esi],eax
0000BE34  06                push es
0000BE35  06                push es
0000BE36  06                push es
0000BE37  06                push es
0000BE38  06                push es
0000BE39  06                push es
0000BE3A  06                push es
0000BE3B  06                push es
0000BE3C  06                push es
0000BE3D  06                push es
0000BE3E  0100              add [eax],eax
0000BE40  0000              add [eax],al
0000BE42  0106              add [esi],eax
0000BE44  06                push es
0000BE45  06                push es
0000BE46  06                push es
0000BE47  06                push es
0000BE48  06                push es
0000BE49  06                push es
0000BE4A  06                push es
0000BE4B  06                push es
0000BE4C  06                push es
0000BE4D  06                push es
0000BE4E  0100              add [eax],eax
0000BE50  0000              add [eax],al
0000BE52  0101              add [ecx],eax
0000BE54  0101              add [ecx],eax
0000BE56  0101              add [ecx],eax
0000BE58  0101              add [ecx],eax
0000BE5A  0101              add [ecx],eax
0000BE5C  0101              add [ecx],eax
0000BE5E  0100              add [eax],eax
0000BE60  0000              add [eax],al
0000BE62  0000              add [eax],al
0000BE64  0000              add [eax],al
0000BE66  0000              add [eax],al
0000BE68  0000              add [eax],al
0000BE6A  0000              add [eax],al
0000BE6C  0000              add [eax],al
0000BE6E  0000              add [eax],al
0000BE70  0000              add [eax],al
0000BE72  0000              add [eax],al
0000BE74  0000              add [eax],al
0000BE76  0000              add [eax],al
0000BE78  0000              add [eax],al
0000BE7A  0000              add [eax],al
0000BE7C  0000              add [eax],al
0000BE7E  0000              add [eax],al
0000BE80  0000              add [eax],al
0000BE82  0000              add [eax],al
0000BE84  0000              add [eax],al
0000BE86  0000              add [eax],al
0000BE88  0000              add [eax],al
0000BE8A  0000              add [eax],al
0000BE8C  0000              add [eax],al
0000BE8E  0000              add [eax],al
0000BE90  0000              add [eax],al
0000BE92  0000              add [eax],al
0000BE94  0101              add [ecx],eax
0000BE96  0101              add [ecx],eax
0000BE98  0101              add [ecx],eax
0000BE9A  0100              add [eax],eax
0000BE9C  0000              add [eax],al
0000BE9E  0000              add [eax],al
0000BEA0  0000              add [eax],al
0000BEA2  0000              add [eax],al
0000BEA4  0102              add [edx],eax
0000BEA6  0202              add al,[edx]
0000BEA8  0202              add al,[edx]
0000BEAA  0101              add [ecx],eax
0000BEAC  0000              add [eax],al
0000BEAE  0000              add [eax],al
0000BEB0  0000              add [eax],al
0000BEB2  0000              add [eax],al
0000BEB4  0102              add [edx],eax
0000BEB6  0202              add al,[edx]
0000BEB8  0202              add al,[edx]
0000BEBA  0201              add al,[ecx]
0000BEBC  0000              add [eax],al
0000BEBE  0000              add [eax],al
0000BEC0  0000              add [eax],al
0000BEC2  0000              add [eax],al
0000BEC4  0102              add [edx],eax
0000BEC6  0202              add al,[edx]
0000BEC8  0202              add al,[edx]
0000BECA  0201              add al,[ecx]
0000BECC  0000              add [eax],al
0000BECE  0000              add [eax],al
0000BED0  0000              add [eax],al
0000BED2  0000              add [eax],al
0000BED4  0102              add [edx],eax
0000BED6  0101              add [ecx],eax
0000BED8  0102              add [edx],eax
0000BEDA  0201              add al,[ecx]
0000BEDC  0000              add [eax],al
0000BEDE  0000              add [eax],al
0000BEE0  0000              add [eax],al
0000BEE2  0000              add [eax],al
0000BEE4  0102              add [edx],eax
0000BEE6  0202              add al,[edx]
0000BEE8  0202              add al,[edx]
0000BEEA  0201              add al,[ecx]
0000BEEC  0000              add [eax],al
0000BEEE  0000              add [eax],al
0000BEF0  0000              add [eax],al
0000BEF2  0000              add [eax],al
0000BEF4  0102              add [edx],eax
0000BEF6  0101              add [ecx],eax
0000BEF8  0101              add [ecx],eax
0000BEFA  0201              add al,[ecx]
0000BEFC  0000              add [eax],al
0000BEFE  0000              add [eax],al
0000BF00  0000              add [eax],al
0000BF02  0000              add [eax],al
0000BF04  0102              add [edx],eax
0000BF06  0202              add al,[edx]
0000BF08  0202              add al,[edx]
0000BF0A  0201              add al,[ecx]
0000BF0C  0000              add [eax],al
0000BF0E  0000              add [eax],al
0000BF10  0000              add [eax],al
0000BF12  0000              add [eax],al
0000BF14  0102              add [edx],eax
0000BF16  0101              add [ecx],eax
0000BF18  0102              add [edx],eax
0000BF1A  0201              add al,[ecx]
0000BF1C  0000              add [eax],al
0000BF1E  0000              add [eax],al
0000BF20  0000              add [eax],al
0000BF22  0000              add [eax],al
0000BF24  0102              add [edx],eax
0000BF26  0202              add al,[edx]
0000BF28  0202              add al,[edx]
0000BF2A  0201              add al,[ecx]
0000BF2C  0000              add [eax],al
0000BF2E  0000              add [eax],al
0000BF30  0000              add [eax],al
0000BF32  0000              add [eax],al
0000BF34  0102              add [edx],eax
0000BF36  0101              add [ecx],eax
0000BF38  0101              add [ecx],eax
0000BF3A  0201              add al,[ecx]
0000BF3C  0000              add [eax],al
0000BF3E  0000              add [eax],al
0000BF40  0000              add [eax],al
0000BF42  0000              add [eax],al
0000BF44  0102              add [edx],eax
0000BF46  0202              add al,[edx]
0000BF48  0202              add al,[edx]
0000BF4A  0201              add al,[ecx]
0000BF4C  0000              add [eax],al
0000BF4E  0000              add [eax],al
0000BF50  0000              add [eax],al
0000BF52  0000              add [eax],al
0000BF54  0101              add [ecx],eax
0000BF56  0101              add [ecx],eax
0000BF58  0101              add [ecx],eax
0000BF5A  0101              add [ecx],eax
0000BF5C  0000              add [eax],al
0000BF5E  0000              add [eax],al
0000BF60  0000              add [eax],al
0000BF62  0000              add [eax],al
0000BF64  0000              add [eax],al
0000BF66  0000              add [eax],al
0000BF68  0000              add [eax],al
0000BF6A  0000              add [eax],al
0000BF6C  0000              add [eax],al
0000BF6E  0000              add [eax],al
0000BF70  0000              add [eax],al
0000BF72  0000              add [eax],al
0000BF74  0000              add [eax],al
0000BF76  0000              add [eax],al
0000BF78  0000              add [eax],al
0000BF7A  0000              add [eax],al
0000BF7C  0000              add [eax],al
0000BF7E  0000              add [eax],al
0000BF80  0000              add [eax],al
0000BF82  0000              add [eax],al
0000BF84  0000              add [eax],al
0000BF86  0000              add [eax],al
0000BF88  0000              add [eax],al
0000BF8A  0000              add [eax],al
0000BF8C  0000              add [eax],al
0000BF8E  0000              add [eax],al
0000BF90  0001              add [ecx],al
0000BF92  0101              add [ecx],eax
0000BF94  0101              add [ecx],eax
0000BF96  0101              add [ecx],eax
0000BF98  0101              add [ecx],eax
0000BF9A  0101              add [ecx],eax
0000BF9C  0101              add [ecx],eax
0000BF9E  0100              add [eax],eax
0000BFA0  0001              add [ecx],al
0000BFA2  0000              add [eax],al
0000BFA4  0000              add [eax],al
0000BFA6  0000              add [eax],al
0000BFA8  0000              add [eax],al
0000BFAA  0000              add [eax],al
0000BFAC  0000              add [eax],al
0000BFAE  0100              add [eax],eax
0000BFB0  0001              add [ecx],al
0000BFB2  0002              add [edx],al
0000BFB4  0200              add al,[eax]
0000BFB6  0000              add [eax],al
0000BFB8  0000              add [eax],al
0000BFBA  0000              add [eax],al
0000BFBC  0000              add [eax],al
0000BFBE  0100              add [eax],eax
0000BFC0  0001              add [ecx],al
0000BFC2  0000              add [eax],al
0000BFC4  0000              add [eax],al
0000BFC6  0000              add [eax],al
0000BFC8  0000              add [eax],al
0000BFCA  0000              add [eax],al
0000BFCC  0000              add [eax],al
0000BFCE  0100              add [eax],eax
0000BFD0  0001              add [ecx],al
0000BFD2  0000              add [eax],al
0000BFD4  0000              add [eax],al
0000BFD6  0000              add [eax],al
0000BFD8  0000              add [eax],al
0000BFDA  0000              add [eax],al
0000BFDC  0000              add [eax],al
0000BFDE  0100              add [eax],eax
0000BFE0  0001              add [ecx],al
0000BFE2  0000              add [eax],al
0000BFE4  0000              add [eax],al
0000BFE6  0000              add [eax],al
0000BFE8  0000              add [eax],al
0000BFEA  0000              add [eax],al
0000BFEC  0000              add [eax],al
0000BFEE  0100              add [eax],eax
0000BFF0  0001              add [ecx],al
0000BFF2  0000              add [eax],al
0000BFF4  0000              add [eax],al
0000BFF6  0000              add [eax],al
0000BFF8  0000              add [eax],al
0000BFFA  0000              add [eax],al
0000BFFC  0000              add [eax],al
0000BFFE  0100              add [eax],eax
0000C000  0001              add [ecx],al
0000C002  0000              add [eax],al
0000C004  0000              add [eax],al
0000C006  0000              add [eax],al
0000C008  0000              add [eax],al
0000C00A  0000              add [eax],al
0000C00C  0000              add [eax],al
0000C00E  0100              add [eax],eax
0000C010  0001              add [ecx],al
0000C012  0000              add [eax],al
0000C014  0000              add [eax],al
0000C016  0000              add [eax],al
0000C018  0000              add [eax],al
0000C01A  0000              add [eax],al
0000C01C  0000              add [eax],al
0000C01E  0100              add [eax],eax
0000C020  0001              add [ecx],al
0000C022  0000              add [eax],al
0000C024  0000              add [eax],al
0000C026  0000              add [eax],al
0000C028  0000              add [eax],al
0000C02A  0000              add [eax],al
0000C02C  0000              add [eax],al
0000C02E  0100              add [eax],eax
0000C030  0001              add [ecx],al
0000C032  0000              add [eax],al
0000C034  0000              add [eax],al
0000C036  0000              add [eax],al
0000C038  0000              add [eax],al
0000C03A  0000              add [eax],al
0000C03C  0000              add [eax],al
0000C03E  0100              add [eax],eax
0000C040  0001              add [ecx],al
0000C042  0101              add [ecx],eax
0000C044  0101              add [ecx],eax
0000C046  0101              add [ecx],eax
0000C048  0101              add [ecx],eax
0000C04A  0101              add [ecx],eax
0000C04C  0101              add [ecx],eax
0000C04E  0100              add [eax],eax
0000C050  0000              add [eax],al
0000C052  0000              add [eax],al
0000C054  0000              add [eax],al
0000C056  0000              add [eax],al
0000C058  0000              add [eax],al
0000C05A  0000              add [eax],al
0000C05C  0000              add [eax],al
0000C05E  0000              add [eax],al
0000C060  0000              add [eax],al
0000C062  0000              add [eax],al
0000C064  0000              add [eax],al
0000C066  0000              add [eax],al
0000C068  0000              add [eax],al
0000C06A  0000              add [eax],al
0000C06C  0000              add [eax],al
0000C06E  0000              add [eax],al
0000C070  0000              add [eax],al
0000C072  0000              add [eax],al
0000C074  0000              add [eax],al
0000C076  0000              add [eax],al
0000C078  0000              add [eax],al
0000C07A  0000              add [eax],al
0000C07C  0000              add [eax],al
0000C07E  0000              add [eax],al
0000C080  0000              add [eax],al
0000C082  0000              add [eax],al
0000C084  0000              add [eax],al
0000C086  0000              add [eax],al
0000C088  0000              add [eax],al
0000C08A  0000              add [eax],al
0000C08C  0000              add [eax],al
0000C08E  0000              add [eax],al
0000C090  0000              add [eax],al
0000C092  0101              add [ecx],eax
0000C094  0101              add [ecx],eax
0000C096  0101              add [ecx],eax
0000C098  0101              add [ecx],eax
0000C09A  0101              add [ecx],eax
0000C09C  0000              add [eax],al
0000C09E  0000              add [eax],al
0000C0A0  0000              add [eax],al
0000C0A2  0103              add [ebx],eax
0000C0A4  0303              add eax,[ebx]
0000C0A6  0303              add eax,[ebx]
0000C0A8  0303              add eax,[ebx]
0000C0AA  0301              add eax,[ecx]
0000C0AC  0000              add [eax],al
0000C0AE  0000              add [eax],al
0000C0B0  0000              add [eax],al
0000C0B2  0103              add [ebx],eax
0000C0B4  0303              add eax,[ebx]
0000C0B6  0303              add eax,[ebx]
0000C0B8  0303              add eax,[ebx]
0000C0BA  0301              add eax,[ecx]
0000C0BC  0000              add [eax],al
0000C0BE  0000              add [eax],al
0000C0C0  0000              add [eax],al
0000C0C2  0101              add [ecx],eax
0000C0C4  0101              add [ecx],eax
0000C0C6  0101              add [ecx],eax
0000C0C8  0101              add [ecx],eax
0000C0CA  0101              add [ecx],eax
0000C0CC  0000              add [eax],al
0000C0CE  0000              add [eax],al
0000C0D0  0000              add [eax],al
0000C0D2  0100              add [eax],eax
0000C0D4  0000              add [eax],al
0000C0D6  0000              add [eax],al
0000C0D8  0000              add [eax],al
0000C0DA  0001              add [ecx],al
0000C0DC  0000              add [eax],al
0000C0DE  0000              add [eax],al
0000C0E0  0000              add [eax],al
0000C0E2  0100              add [eax],eax
0000C0E4  0400              add al,0x0
0000C0E6  0400              add al,0x0
0000C0E8  0400              add al,0x0
0000C0EA  0401              add al,0x1
0000C0EC  0000              add [eax],al
0000C0EE  0000              add [eax],al
0000C0F0  0000              add [eax],al
0000C0F2  0100              add [eax],eax
0000C0F4  0400              add al,0x0
0000C0F6  0400              add al,0x0
0000C0F8  0400              add al,0x0
0000C0FA  0401              add al,0x1
0000C0FC  0000              add [eax],al
0000C0FE  0000              add [eax],al
0000C100  0000              add [eax],al
0000C102  0100              add [eax],eax
0000C104  0000              add [eax],al
0000C106  0000              add [eax],al
0000C108  0000              add [eax],al
0000C10A  0001              add [ecx],al
0000C10C  0000              add [eax],al
0000C10E  0000              add [eax],al
0000C110  0000              add [eax],al
0000C112  0100              add [eax],eax
0000C114  0400              add al,0x0
0000C116  0400              add al,0x0
0000C118  0400              add al,0x0
0000C11A  0401              add al,0x1
0000C11C  0000              add [eax],al
0000C11E  0000              add [eax],al
0000C120  0000              add [eax],al
0000C122  0100              add [eax],eax
0000C124  0400              add al,0x0
0000C126  0400              add al,0x0
0000C128  0400              add al,0x0
0000C12A  0401              add al,0x1
0000C12C  0000              add [eax],al
0000C12E  0000              add [eax],al
0000C130  0000              add [eax],al
0000C132  0100              add [eax],eax
0000C134  0000              add [eax],al
0000C136  0000              add [eax],al
0000C138  0000              add [eax],al
0000C13A  0001              add [ecx],al
0000C13C  0000              add [eax],al
0000C13E  0000              add [eax],al
0000C140  0000              add [eax],al
0000C142  0100              add [eax],eax
0000C144  0400              add al,0x0
0000C146  0400              add al,0x0
0000C148  0400              add al,0x0
0000C14A  0401              add al,0x1
0000C14C  0000              add [eax],al
0000C14E  0000              add [eax],al
0000C150  0000              add [eax],al
0000C152  0101              add [ecx],eax
0000C154  0101              add [ecx],eax
0000C156  0101              add [ecx],eax
0000C158  0101              add [ecx],eax
0000C15A  0101              add [ecx],eax
0000C15C  0000              add [eax],al
0000C15E  0000              add [eax],al
0000C160  0000              add [eax],al
0000C162  0000              add [eax],al
0000C164  0000              add [eax],al
0000C166  0000              add [eax],al
0000C168  0000              add [eax],al
0000C16A  0000              add [eax],al
0000C16C  0000              add [eax],al
0000C16E  0000              add [eax],al
0000C170  0000              add [eax],al
0000C172  0000              add [eax],al
0000C174  0000              add [eax],al
0000C176  0000              add [eax],al
0000C178  0000              add [eax],al
0000C17A  0000              add [eax],al
0000C17C  0000              add [eax],al
0000C17E  0000              add [eax],al
0000C180  53                push ebx
0000C181  697A653A005379    imul edi,[edx+0x65],dword 0x7953003a
0000C188  7374              jnc 0xc1fe
0000C18A  656D              gs insd
0000C18C  204D6F            and [ebp+0x6f],cl
0000C18F  6E                outsb
0000C190  69746F72004D656D  imul esi,[edi+ebp*2+0x72],dword 0x6d654d00
0000C198  6F                outsd
0000C199  7279              jc 0xc214
0000C19B  3A00              cmp al,[eax]
0000C19D  2F                das
0000C19E  004D42            add [ebp+0x42],cl
0000C1A1  005570            add [ebp+0x70],dl
0000C1A4  7469              jz 0xc20f
0000C1A6  6D                insd
0000C1A7  653A00            cmp al,[gs:eax]
0000C1AA  7300              jnc 0xc1ac
0000C1AC  696E7400636861    imul ebp,[esi+0x74],dword 0x61686300
0000C1B3  7200              jc 0xc1b5
0000C1B5  766F              jna 0xc226
0000C1B7  6964007265747572  imul esp,[eax+eax+0x72],dword 0x72757465
0000C1BF  6E                outsb
0000C1C0  006966            add [ecx+0x66],ch
0000C1C3  00656C            add [ebp+0x6c],ah
0000C1C6  7365              jnc 0xc22d
0000C1C8  007768            add [edi+0x68],dh
0000C1CB  696C6500666F7200  imul ebp,[ebp+0x0],dword 0x726f66
0000C1D3  7374              jnc 0xc249
0000C1D5  7275              jc 0xc24c
0000C1D7  63740074          arpl [eax+eax+0x74],si
0000C1DB  7970              jns 0xc24d
0000C1DD  6564656600696E    o16 add [gs:ecx+0x6e],ch
0000C1E4  636C7564          arpl [ebp+esi*2+0x64],bp
0000C1E8  6500646566        add [gs:ebp+0x66],ah
0000C1ED  696E650075696E    imul ebp,[esi+0x65],dword 0x6e697500
0000C1F4  7438              jz 0xc22e
0000C1F6  5F                pop edi
0000C1F7  7400              jz 0xc1f9
0000C1F9  7569              jnz 0xc264
0000C1FB  6E                outsb
0000C1FC  7433              jz 0xc231
0000C1FE  325F74            xor bl,[edi+0x74]
0000C201  006578            add [ebp+0x78],ah
0000C204  7465              jz 0xc26b
0000C206  726E              jc 0xc276
0000C208  007374            add [ebx+0x74],dh
0000C20B  61                popa
0000C20C  7469              jz 0xc277
0000C20E  6300              arpl [eax],ax
0000C210  636F6E            arpl [edi+0x6e],bp
0000C213  7374              jnc 0xc289
0000C215  006272            add [edx+0x72],ah
0000C218  6561              gs popa
0000C21A  6B0063            imul eax,[eax],byte +0x63
0000C21D  6F                outsd
0000C21E  6E                outsb
0000C21F  7469              jz 0xc28a
0000C221  6E                outsb
0000C222  7565              jnz 0xc289
0000C224  0000              add [eax],al
0000C226  0000              add [eax],al
0000C228  0000              add [eax],al
0000C22A  0000              add [eax],al
0000C22C  0000              add [eax],al
0000C22E  0000              add [eax],al
0000C230  0000              add [eax],al
0000C232  0000              add [eax],al
0000C234  0000              add [eax],al
0000C236  0000              add [eax],al
0000C238  0000              add [eax],al
0000C23A  0000              add [eax],al
0000C23C  0000              add [eax],al
0000C23E  0000              add [eax],al
0000C240  AC                lodsb
0000C241  C10100            rol dword [ecx],byte 0x0
0000C244  B0C1              mov al,0xc1
0000C246  0100              add [eax],eax
0000C248  B5C1              mov ch,0xc1
0000C24A  0100              add [eax],eax
0000C24C  BAC10100C1        mov edx,0xc10001c1
0000C251  C10100            rol dword [ecx],byte 0x0
0000C254  C4                db 0xc4
0000C255  C10100            rol dword [ecx],byte 0x0
0000C258  C9                leave
0000C259  C10100            rol dword [ecx],byte 0x0
0000C25C  CF                iret
0000C25D  C10100            rol dword [ecx],byte 0x0
0000C260  D3C1              rol ecx,cl
0000C262  0100              add [eax],eax
0000C264  DAC1              fcmovb st1
0000C266  0100              add [eax],eax
0000C268  E2C1              loop 0xc22b
0000C26A  0100              add [eax],eax
0000C26C  EAC10100F1C101    jmp 0x1c1:0xf10001c1
0000C273  00F9              add cl,bh
0000C275  C10100            rol dword [ecx],byte 0x0
0000C278  02C2              add al,dl
0000C27A  0100              add [eax],eax
0000C27C  09C2              or edx,eax
0000C27E  0100              add [eax],eax
0000C280  10C2              adc dl,al
0000C282  0100              add [eax],eax
0000C284  16                push ss
0000C285  C20100            ret 0x1
0000C288  1CC2              sbb al,0xc2
0000C28A  0100              add [eax],eax
0000C28C  0000              add [eax],al
0000C28E  0000              add [eax],al
0000C290  57                push edi
0000C291  656C              gs insb
0000C293  636F6D            arpl [edi+0x6d],bp
0000C296  6520746F20        and [gs:edi+ebp*2+0x20],dh
0000C29B  50                push eax
0000C29C  7572              jnz 0xc310
0000C29E  654F              gs dec edi
0000C2A0  53                push ebx
0000C2A1  20546572          and [ebp+0x72],dl
0000C2A5  6D                insd
0000C2A6  696E616C0A2D2D    imul ebp,[esi+0x61],dword 0x2d2d0a6c
0000C2AD  2D2D2D2D2D        sub eax,0x2d2d2d2d
0000C2B2  2D2D2D2D2D        sub eax,0x2d2d2d2d
0000C2B7  2D2D2D2D2D        sub eax,0x2d2d2d2d
0000C2BC  2D2D2D2D2D        sub eax,0x2d2d2d2d
0000C2C1  2D2D2D2D0A        sub eax,0xa2d2d2d
0000C2C6  54                push esp
0000C2C7  7970              jns 0xc339
0000C2C9  652027            and [gs:edi],ah
0000C2CC  68656C7027        push dword 0x27706c65
0000C2D1  2E0A0A            or cl,[cs:edx]
0000C2D4  50                push eax
0000C2D5  7572              jnz 0xc349
0000C2D7  654F              gs dec edi
0000C2D9  53                push ebx
0000C2DA  202F              and [edi],ch
0000C2DC  3E2000            and [ds:eax],al
0000C2DF  004F89            add [edi-0x77],cl
0000C2E2  0100              add [eax],eax
0000C2E4  228901002B89      and cl,[ecx-0x76d4ffff]
0000C2EA  0100              add [eax],eax
0000C2EC  3489              xor al,0x89
0000C2EE  0100              add [eax],eax
0000C2F0  3D8901004F        cmp eax,0x4f000189
0000C2F5  8901              mov [ecx],eax
0000C2F7  004689            add [esi-0x77],al
0000C2FA  0100              add [eax],eax
0000C2FC  0A00              or al,[eax]
0000C2FE  0A5075            or dl,[eax+0x75]
0000C301  7265              jc 0xc368
0000C303  4F                dec edi
0000C304  53                push ebx
0000C305  2000              and [eax],al
0000C307  3E2000            and [ds:eax],al
0000C30A  37                aaa
0000C30B  0038              add [eax],bh
0000C30D  0039              add [ecx],bh
0000C30F  003400            add [eax+eax],dh
0000C312  350036002A        xor eax,0x2a003600
0000C317  0031              add [ecx],dh
0000C319  0032              add [edx],dh
0000C31B  0033              add [ebx],dh
0000C31D  002D00430030      add [dword 0x30004300],ch
0000C323  003D002B0043      add [dword 0x43002b00],bh
0000C329  6F                outsd
0000C32A  6D                insd
0000C32B  7075              jo 0xc3a2
0000C32D  7465              jz 0xc394
0000C32F  7200              jc 0xc331
0000C331  54                push esp
0000C332  65726D            gs jc 0xc3a2
0000C335  696E616C004361    imul ebp,[esi+0x61],dword 0x6143006c
0000C33C  6C                insb
0000C33D  6300              arpl [eax],ax
0000C33F  50                push eax
0000C340  61                popa
0000C341  696E7400537973    imul ebp,[esi+0x74],dword 0x73795300
0000C348  4D                dec ebp
0000C349  6F                outsd
0000C34A  6E                outsb
0000C34B  004D79            add [ebp+0x79],cl
0000C34E  20436F            and [ebx+0x6f],al
0000C351  6D                insd
0000C352  7075              jo 0xc3c9
0000C354  7465              jz 0xc3bb
0000C356  7200              jc 0xc358
0000C358  45                inc ebp
0000C359  646974696E673A20  imul esi,[fs:ecx+ebp*2+0x6e],dword 0x203a67
         -00
0000C362  43                inc ebx
0000C363  61                popa
0000C364  6C                insb
0000C365  63756C            arpl [ebp+0x6c],si
0000C368  61                popa
0000C369  746F              jz 0xc3da
0000C36B  7200              jc 0xc36d
0000C36D  0000              add [eax],al
0000C36F  001400            add [eax+eax],dl
0000C372  0000              add [eax],al
0000C374  0000              add [eax],al
0000C376  0000              add [eax],al
0000C378  017A52            add [edx+0x52],edi
0000C37B  0001              add [ecx],al
0000C37D  7C08              jl 0xc387
0000C37F  011B              add [ebx],ebx
0000C381  0C04              or al,0x4
0000C383  0488              add al,0x88
0000C385  0100              add [eax],eax
0000C387  001C00            add [eax+eax],bl
0000C38A  0000              add [eax],al
0000C38C  1C00              sbb al,0x0
0000C38E  0000              add [eax],al
0000C390  083E              or [esi],bh
0000C392  FF                db 0xff
0000C393  FF1D00000000      call far [dword 0x0]
0000C399  41                inc ecx
0000C39A  0E                push cs
0000C39B  088502420D05      or [ebp+0x50d4202],al
0000C3A1  59                pop ecx
0000C3A2  C50C04            lds ecx,[esp+eax]
0000C3A5  0400              add al,0x0
0000C3A7  0018              add [eax],bl
0000C3A9  0000              add [eax],al
0000C3AB  003C00            add [eax+eax],bh
0000C3AE  0000              add [eax],al
0000C3B0  053EFFFF8C        add eax,0x8cffff3e
0000C3B5  0100              add [eax],eax
0000C3B7  0000              add [eax],al
0000C3B9  41                inc ecx
0000C3BA  0E                push cs
0000C3BB  088502420D05      or [ebp+0x50d4202],al
0000C3C1  0000              add [eax],al
0000C3C3  001400            add [eax+eax],dl
0000C3C6  0000              add [eax],al
0000C3C8  0000              add [eax],al
0000C3CA  0000              add [eax],al
0000C3CC  017A52            add [edx+0x52],edi
0000C3CF  0001              add [ecx],al
0000C3D1  7C08              jl 0xc3db
0000C3D3  011B              add [ebx],ebx
0000C3D5  0C04              or al,0x4
0000C3D7  0488              add al,0x88
0000C3D9  0100              add [eax],eax
0000C3DB  001C00            add [eax+eax],bl
0000C3DE  0000              add [eax],al
0000C3E0  1C00              sbb al,0x0
0000C3E2  0000              add [eax],al
0000C3E4  5D                pop ebp
0000C3E5  3F                aas
0000C3E6  FF                db 0xff
0000C3E7  FF9E00000000      call far [esi+0x0]
0000C3ED  41                inc ecx
0000C3EE  0E                push cs
0000C3EF  088502420D05      or [ebp+0x50d4202],al
0000C3F5  029AC50C0404      add bl,[edx+0x4040cc5]
0000C3FB  001C00            add [eax+eax],bl
0000C3FE  0000              add [eax],al
0000C400  3C00              cmp al,0x0
0000C402  0000              add [eax],al
0000C404  DB3F              fstp tword [edi]
0000C406  FF                db 0xff
0000C407  FF21              jmp [ecx]
0000C409  0000              add [eax],al
0000C40B  0000              add [eax],al
0000C40D  41                inc ecx
0000C40E  0E                push cs
0000C40F  088502420D05      or [ebp+0x50d4202],al
0000C415  5D                pop ebp
0000C416  C50C04            lds ecx,[esp+eax]
0000C419  0400              add al,0x0
0000C41B  001C00            add [eax+eax],bl
0000C41E  0000              add [eax],al
0000C420  5C                pop esp
0000C421  0000              add [eax],al
0000C423  00DC              add ah,bl
0000C425  3F                aas
0000C426  FF                db 0xff
0000C427  FF6900            jmp far [ecx+0x0]
0000C42A  0000              add [eax],al
0000C42C  00410E            add [ecx+0xe],al
0000C42F  088502420D05      or [ebp+0x50d4202],al
0000C435  0265C5            add ah,[ebp-0x3b]
0000C438  0C04              or al,0x4
0000C43A  0400              add al,0x0
0000C43C  1400              adc al,0x0
0000C43E  0000              add [eax],al
0000C440  0000              add [eax],al
0000C442  0000              add [eax],al
0000C444  017A52            add [edx+0x52],edi
0000C447  0001              add [ecx],al
0000C449  7C08              jl 0xc453
0000C44B  011B              add [ebx],ebx
0000C44D  0C04              or al,0x4
0000C44F  0488              add al,0x88
0000C451  0100              add [eax],eax
0000C453  001C00            add [eax+eax],bl
0000C456  0000              add [eax],al
0000C458  1C00              sbb al,0x0
0000C45A  0000              add [eax],al
0000C45C  0D40FFFF6A        or eax,0x6affff40
0000C461  0000              add [eax],al
0000C463  0000              add [eax],al
0000C465  41                inc ecx
0000C466  0E                push cs
0000C467  088502420D05      or [ebp+0x50d4202],al
0000C46D  0266C5            add ah,[esi-0x3b]
0000C470  0C04              or al,0x4
0000C472  0400              add al,0x0
0000C474  1C00              sbb al,0x0
0000C476  0000              add [eax],al
0000C478  3C00              cmp al,0x0
0000C47A  0000              add [eax],al
0000C47C  57                push edi
0000C47D  40                inc eax
0000C47E  FF                db 0xff
0000C47F  FF5800            call far [eax+0x0]
0000C482  0000              add [eax],al
0000C484  00410E            add [ecx+0xe],al
0000C487  088502420D05      or [ebp+0x50d4202],al
0000C48D  0254C50C          add dl,[ebp+eax*8+0xc]
0000C491  0404              add al,0x4
0000C493  001400            add [eax+eax],dl
0000C496  0000              add [eax],al
0000C498  0000              add [eax],al
0000C49A  0000              add [eax],al
0000C49C  017A52            add [edx+0x52],edi
0000C49F  0001              add [ecx],al
0000C4A1  7C08              jl 0xc4ab
0000C4A3  011B              add [ebx],ebx
0000C4A5  0C04              or al,0x4
0000C4A7  0488              add al,0x88
0000C4A9  0100              add [eax],eax
0000C4AB  001C00            add [eax+eax],bl
0000C4AE  0000              add [eax],al
0000C4B0  1C00              sbb al,0x0
0000C4B2  0000              add [eax],al
0000C4B4  7740              ja 0xc4f6
0000C4B6  FF                db 0xff
0000C4B7  FF1B              call far [ebx]
0000C4B9  0000              add [eax],al
0000C4BB  0000              add [eax],al
0000C4BD  41                inc ecx
0000C4BE  0E                push cs
0000C4BF  088502420D05      or [ebp+0x50d4202],al
0000C4C5  57                push edi
0000C4C6  C50C04            lds ecx,[esp+eax]
0000C4C9  0400              add al,0x0
0000C4CB  001C00            add [eax+eax],bl
0000C4CE  0000              add [eax],al
0000C4D0  3C00              cmp al,0x0
0000C4D2  0000              add [eax],al
0000C4D4  7240              jc 0xc516
0000C4D6  FF                db 0xff
0000C4D7  FF                db 0xff
0000C4D8  FF00              inc dword [eax]
0000C4DA  0000              add [eax],al
0000C4DC  00410E            add [ecx+0xe],al
0000C4DF  088502420D05      or [ebp+0x50d4202],al
0000C4E5  02FB              add bh,bl
0000C4E7  C50C04            lds ecx,[esp+eax]
0000C4EA  0400              add al,0x0
0000C4EC  1C00              sbb al,0x0
0000C4EE  0000              add [eax],al
0000C4F0  5C                pop esp
0000C4F1  0000              add [eax],al
0000C4F3  005141            add [ecx+0x41],dl
0000C4F6  FF                db 0xff
0000C4F7  FF19              call far [ecx]
0000C4F9  0300              add eax,[eax]
0000C4FB  0000              add [eax],al
0000C4FD  41                inc ecx
0000C4FE  0E                push cs
0000C4FF  088502420D05      or [ebp+0x50d4202],al
0000C505  031503C50C04      add edx,[dword 0x40cc503]
0000C50B  041C              add al,0x1c
0000C50D  0000              add [eax],al
0000C50F  007C0000          add [eax+eax+0x0],bh
0000C513  004A44            add [edx+0x44],cl
0000C516  FF                db 0xff
0000C517  FF06              inc dword [esi]
0000C519  0000              add [eax],al
0000C51B  0000              add [eax],al
0000C51D  41                inc ecx
0000C51E  0E                push cs
0000C51F  088502420D05      or [ebp+0x50d4202],al
0000C525  42                inc edx
0000C526  C50C04            lds ecx,[esp+eax]
0000C529  0400              add al,0x0
0000C52B  001C00            add [eax+eax],bl
0000C52E  0000              add [eax],al
0000C530  9C                pushf
0000C531  0000              add [eax],al
0000C533  0030              add [eax],dh
0000C535  44                inc esp
0000C536  FF                db 0xff
0000C537  FF640000          jmp [eax+eax+0x0]
0000C53B  0000              add [eax],al
0000C53D  41                inc ecx
0000C53E  0E                push cs
0000C53F  088502420D05      or [ebp+0x50d4202],al
0000C545  0260C5            add ah,[eax-0x3b]
0000C548  0C04              or al,0x4
0000C54A  0400              add al,0x0
0000C54C  1C00              sbb al,0x0
0000C54E  0000              add [eax],al
0000C550  BC00000074        mov esp,0x74000000
0000C555  44                inc esp
0000C556  FF                db 0xff
0000C557  FF1D00000000      call far [dword 0x0]
0000C55D  41                inc ecx
0000C55E  0E                push cs
0000C55F  088502420D05      or [ebp+0x50d4202],al
0000C565  59                pop ecx
0000C566  C50C04            lds ecx,[esp+eax]
0000C569  0400              add al,0x0
0000C56B  001400            add [eax+eax],dl
0000C56E  0000              add [eax],al
0000C570  0000              add [eax],al
0000C572  0000              add [eax],al
0000C574  017A52            add [edx+0x52],edi
0000C577  0001              add [ecx],al
0000C579  7C08              jl 0xc583
0000C57B  011B              add [ebx],ebx
0000C57D  0C04              or al,0x4
0000C57F  0488              add al,0x88
0000C581  0100              add [eax],eax
0000C583  001C00            add [eax+eax],bl
0000C586  0000              add [eax],al
0000C588  1C00              sbb al,0x0
0000C58A  0000              add [eax],al
0000C58C  59                pop ecx
0000C58D  44                inc esp
0000C58E  FF                db 0xff
0000C58F  FF29              jmp far [ecx]
0000C591  0000              add [eax],al
0000C593  0000              add [eax],al
0000C595  41                inc ecx
0000C596  0E                push cs
0000C597  088502420D05      or [ebp+0x50d4202],al
0000C59D  65C50C04          lds ecx,[gs:esp+eax]
0000C5A1  0400              add al,0x0
0000C5A3  001C00            add [eax+eax],bl
0000C5A6  0000              add [eax],al
0000C5A8  3C00              cmp al,0x0
0000C5AA  0000              add [eax],al
0000C5AC  6244FFFF          bound eax,[edi+edi*8-0x1]
0000C5B0  C400              les eax,[eax]
0000C5B2  0000              add [eax],al
0000C5B4  00410E            add [ecx+0xe],al
0000C5B7  088502420D05      or [ebp+0x50d4202],al
0000C5BD  02C0              add al,al
0000C5BF  C50C04            lds ecx,[esp+eax]
0000C5C2  0400              add al,0x0
0000C5C4  1C00              sbb al,0x0
0000C5C6  0000              add [eax],al
0000C5C8  5C                pop esp
0000C5C9  0000              add [eax],al
0000C5CB  0006              add [esi],al
0000C5CD  45                inc ebp
0000C5CE  FF                db 0xff
0000C5CF  FF5E00            call far [esi+0x0]
0000C5D2  0000              add [eax],al
0000C5D4  00410E            add [ecx+0xe],al
0000C5D7  088502420D05      or [ebp+0x50d4202],al
0000C5DD  025AC5            add bl,[edx-0x3b]
0000C5E0  0C04              or al,0x4
0000C5E2  0400              add al,0x0
0000C5E4  1C00              sbb al,0x0
0000C5E6  0000              add [eax],al
0000C5E8  7C00              jl 0xc5ea
0000C5EA  0000              add [eax],al
0000C5EC  44                inc esp
0000C5ED  45                inc ebp
0000C5EE  FF                db 0xff
0000C5EF  FF33              push dword [ebx]
0000C5F1  0100              add [eax],eax
0000C5F3  0000              add [eax],al
0000C5F5  41                inc ecx
0000C5F6  0E                push cs
0000C5F7  088502420D05      or [ebp+0x50d4202],al
0000C5FD  032F              add ebp,[edi]
0000C5FF  01C5              add ebp,eax
0000C601  0C04              or al,0x4
0000C603  0414              add al,0x14
0000C605  0000              add [eax],al
0000C607  0000              add [eax],al
0000C609  0000              add [eax],al
0000C60B  0001              add [ecx],al
0000C60D  7A52              jpe 0xc661
0000C60F  0001              add [ecx],al
0000C611  7C08              jl 0xc61b
0000C613  011B              add [ebx],ebx
0000C615  0C04              or al,0x4
0000C617  0488              add al,0x88
0000C619  0100              add [eax],eax
0000C61B  001C00            add [eax+eax],bl
0000C61E  0000              add [eax],al
0000C620  1C00              sbb al,0x0
0000C622  0000              add [eax],al
0000C624  3F                aas
0000C625  46                inc esi
0000C626  FF                db 0xff
0000C627  FF1C00            call far [eax+eax]
0000C62A  0000              add [eax],al
0000C62C  00410E            add [ecx+0xe],al
0000C62F  088502420D05      or [ebp+0x50d4202],al
0000C635  58                pop eax
0000C636  C50C04            lds ecx,[esp+eax]
0000C639  0400              add al,0x0
0000C63B  001C00            add [eax+eax],bl
0000C63E  0000              add [eax],al
0000C640  3C00              cmp al,0x0
0000C642  0000              add [eax],al
0000C644  3B46FF            cmp eax,[esi-0x1]
0000C647  FFE6              jmp esi
0000C649  0000              add [eax],al
0000C64B  0000              add [eax],al
0000C64D  41                inc ecx
0000C64E  0E                push cs
0000C64F  088502420D05      or [ebp+0x50d4202],al
0000C655  02E2              add ah,dl
0000C657  C50C04            lds ecx,[esp+eax]
0000C65A  0400              add al,0x0
0000C65C  1C00              sbb al,0x0
0000C65E  0000              add [eax],al
0000C660  5C                pop esp
0000C661  0000              add [eax],al
0000C663  0001              add [ecx],al
0000C665  47                inc edi
0000C666  FF                db 0xff
0000C667  FF08              dec dword [eax]
0000C669  0100              add [eax],eax
0000C66B  0000              add [eax],al
0000C66D  41                inc ecx
0000C66E  0E                push cs
0000C66F  088502420D05      or [ebp+0x50d4202],al
0000C675  030401            add eax,[ecx+eax]
0000C678  C50C04            lds ecx,[esp+eax]
0000C67B  041C              add al,0x1c
0000C67D  0000              add [eax],al
0000C67F  007C0000          add [eax+eax+0x0],bh
0000C683  00E9              add cl,ch
0000C685  47                inc edi
0000C686  FF                db 0xff
0000C687  FF                db 0xff
0000C688  EB02              jmp short 0xc68c
0000C68A  0000              add [eax],al
0000C68C  00410E            add [ecx+0xe],al
0000C68F  088502420D05      or [ebp+0x50d4202],al
0000C695  03E7              add esp,edi
0000C697  02C5              add al,ch
0000C699  0C04              or al,0x4
0000C69B  041C              add al,0x1c
0000C69D  0000              add [eax],al
0000C69F  009C000000B44A    add [eax+eax+0x4ab40000],bl
0000C6A6  FF                db 0xff
0000C6A7  FF6203            jmp [edx+0x3]
0000C6AA  0000              add [eax],al
0000C6AC  00410E            add [ecx+0xe],al
0000C6AF  088502420D05      or [ebp+0x50d4202],al
0000C6B5  035E03            add ebx,[esi+0x3]
0000C6B8  C50C04            lds ecx,[esp+eax]
0000C6BB  041C              add al,0x1c
0000C6BD  0000              add [eax],al
0000C6BF  00BC000000F64D    add [eax+eax+0x4df60000],bh
0000C6C6  FF                db 0xff
0000C6C7  FF03              inc dword [ebx]
0000C6C9  0300              add eax,[eax]
0000C6CB  0000              add [eax],al
0000C6CD  41                inc ecx
0000C6CE  0E                push cs
0000C6CF  088502420D05      or [ebp+0x50d4202],al
0000C6D5  03FF              add edi,edi
0000C6D7  02C5              add al,ch
0000C6D9  0C04              or al,0x4
0000C6DB  041C              add al,0x1c
0000C6DD  0000              add [eax],al
0000C6DF  00DC              add ah,bl
0000C6E1  0000              add [eax],al
0000C6E3  00D9              add cl,bl
0000C6E5  50                push eax
0000C6E6  FF                db 0xff
0000C6E7  FF0A              dec dword [edx]
0000C6E9  0100              add [eax],eax
0000C6EB  0000              add [eax],al
0000C6ED  41                inc ecx
0000C6EE  0E                push cs
0000C6EF  088502420D05      or [ebp+0x50d4202],al
0000C6F5  0306              add eax,[esi]
0000C6F7  01C5              add ebp,eax
0000C6F9  0C04              or al,0x4
0000C6FB  041C              add al,0x1c
0000C6FD  0000              add [eax],al
0000C6FF  00FC              add ah,bh
0000C701  0000              add [eax],al
0000C703  00C3              add bl,al
0000C705  51                push ecx
0000C706  FF                db 0xff
0000C707  FF2B              jmp far [ebx]
0000C709  0800              or [eax],al
0000C70B  0000              add [eax],al
0000C70D  41                inc ecx
0000C70E  0E                push cs
0000C70F  088502420D05      or [ebp+0x50d4202],al
0000C715  0327              add esp,[edi]
0000C717  08C5              or ch,al
0000C719  0C04              or al,0x4
0000C71B  041C              add al,0x1c
0000C71D  0000              add [eax],al
0000C71F  001C01            add [ecx+eax],bl
0000C722  0000              add [eax],al
0000C724  CE                into
0000C725  59                pop ecx
0000C726  FF                db 0xff
0000C727  FF20              jmp [eax]
0000C729  0300              add eax,[eax]
0000C72B  0000              add [eax],al
0000C72D  41                inc ecx
0000C72E  0E                push cs
0000C72F  088502420D05      or [ebp+0x50d4202],al
0000C735  031C03            add ebx,[ebx+eax]
0000C738  C50C04            lds ecx,[esp+eax]
0000C73B  041C              add al,0x1c
0000C73D  0000              add [eax],al
0000C73F  003C01            add [ecx+eax],bh
0000C742  0000              add [eax],al
0000C744  CE                into
0000C745  5C                pop esp
0000C746  FF                db 0xff
0000C747  FF1B              call far [ebx]
0000C749  0400              add al,0x0
0000C74B  0000              add [eax],al
0000C74D  41                inc ecx
0000C74E  0E                push cs
0000C74F  088502420D05      or [ebp+0x50d4202],al
0000C755  0317              add edx,[edi]
0000C757  04C5              add al,0xc5
0000C759  0C04              or al,0x4
0000C75B  041C              add al,0x1c
0000C75D  0000              add [eax],al
0000C75F  005C0100          add [ecx+eax+0x0],bl
0000C763  00C9              add cl,cl
0000C765  60                pusha
0000C766  FF                db 0xff
0000C767  FFC0              inc eax
0000C769  0300              add eax,[eax]
0000C76B  0000              add [eax],al
0000C76D  41                inc ecx
0000C76E  0E                push cs
0000C76F  088502420D05      or [ebp+0x50d4202],al
0000C775  03BC03C50C0404    add edi,[ebx+eax+0x4040cc5]
0000C77C  1C00              sbb al,0x0
0000C77E  0000              add [eax],al
0000C780  7C01              jl 0xc783
0000C782  0000              add [eax],al
0000C784  6964FFFFA6030000  imul esp,[edi+edi*8-0x1],dword 0x3a6
0000C78C  00410E            add [ecx+0xe],al
0000C78F  088502420D05      or [ebp+0x50d4202],al
0000C795  03A203C50C04      add esp,[edx+0x40cc503]
0000C79B  0414              add al,0x14
0000C79D  0000              add [eax],al
0000C79F  0000              add [eax],al
0000C7A1  0000              add [eax],al
0000C7A3  0001              add [ecx],al
0000C7A5  7A52              jpe 0xc7f9
0000C7A7  0001              add [ecx],al
0000C7A9  7C08              jl 0xc7b3
0000C7AB  011B              add [ebx],ebx
0000C7AD  0C04              or al,0x4
0000C7AF  0488              add al,0x88
0000C7B1  0100              add [eax],eax
0000C7B3  001C00            add [eax+eax],bl
0000C7B6  0000              add [eax],al
0000C7B8  1C00              sbb al,0x0
0000C7BA  0000              add [eax],al
0000C7BC  D7                xlatb
0000C7BD  67                a16
0000C7BE  FF                db 0xff
0000C7BF  FF5000            call [eax+0x0]
0000C7C2  0000              add [eax],al
0000C7C4  00410E            add [ecx+0xe],al
0000C7C7  088502420D05      or [ebp+0x50d4202],al
0000C7CD  024CC50C          add cl,[ebp+eax*8+0xc]
0000C7D1  0404              add al,0x4
0000C7D3  001C00            add [eax+eax],bl
0000C7D6  0000              add [eax],al
0000C7D8  3C00              cmp al,0x0
0000C7DA  0000              add [eax],al
0000C7DC  07                pop es
0000C7DD  68FFFF5400        push dword 0x54ffff
0000C7E2  0000              add [eax],al
0000C7E4  00410E            add [ecx+0xe],al
0000C7E7  088502420D05      or [ebp+0x50d4202],al
0000C7ED  0250C5            add dl,[eax-0x3b]
0000C7F0  0C04              or al,0x4
0000C7F2  0400              add al,0x0
0000C7F4  1C00              sbb al,0x0
0000C7F6  0000              add [eax],al
0000C7F8  5C                pop esp
0000C7F9  0000              add [eax],al
0000C7FB  003B              add [ebx],bh
0000C7FD  68FFFFBD00        push dword 0xbdffff
0000C802  0000              add [eax],al
0000C804  00410E            add [ecx+0xe],al
0000C807  088502420D05      or [ebp+0x50d4202],al
0000C80D  02B9C50C0404      add bh,[ecx+0x4040cc5]
0000C813  0020              add [eax],ah
0000C815  0000              add [eax],al
0000C817  007C0000          add [eax+eax+0x0],bh
0000C81B  00D8              add al,bl
0000C81D  68FFFFA500        push dword 0xa5ffff
0000C822  0000              add [eax],al
0000C824  00410E            add [ecx+0xe],al
0000C827  088502420D05      or [ebp+0x50d4202],al
0000C82D  44                inc esp
0000C82E  830302            add dword [ebx],byte +0x2
0000C831  9C                pushf
0000C832  C3                ret
0000C833  41                inc ecx
0000C834  C50C04            lds ecx,[esp+eax]
0000C837  041C              add al,0x1c
0000C839  0000              add [eax],al
0000C83B  00A000000059      add [eax+0x59000000],ah
0000C841  69FFFF650000      imul edi,edi,dword 0x65ff
0000C847  0000              add [eax],al
0000C849  41                inc ecx
0000C84A  0E                push cs
0000C84B  088502420D05      or [ebp+0x50d4202],al
0000C851  0261C5            add ah,[ecx-0x3b]
0000C854  0C04              or al,0x4
0000C856  0400              add al,0x0
0000C858  1800              sbb [eax],al
0000C85A  0000              add [eax],al
0000C85C  C00000            rol byte [eax],byte 0x0
0000C85F  009E69FFFF26      add [esi+0x26ffff69],bl
0000C865  0200              add al,[eax]
0000C867  0000              add [eax],al
0000C869  41                inc ecx
0000C86A  0E                push cs
0000C86B  088502420D05      or [ebp+0x50d4202],al
0000C871  0000              add [eax],al
0000C873  001400            add [eax+eax],dl
0000C876  0000              add [eax],al
0000C878  0000              add [eax],al
0000C87A  0000              add [eax],al
0000C87C  017A52            add [edx+0x52],edi
0000C87F  0001              add [ecx],al
0000C881  7C08              jl 0xc88b
0000C883  011B              add [ebx],ebx
0000C885  0C04              or al,0x4
0000C887  0488              add al,0x88
0000C889  0100              add [eax],eax
0000C88B  001C00            add [eax+eax],bl
0000C88E  0000              add [eax],al
0000C890  1C00              sbb al,0x0
0000C892  0000              add [eax],al
0000C894  90                nop
0000C895  6BFFFF            imul edi,edi,byte -0x1
0000C898  1400              adc al,0x0
0000C89A  0000              add [eax],al
0000C89C  00410E            add [ecx+0xe],al
0000C89F  088502420D05      or [ebp+0x50d4202],al
0000C8A5  50                push eax
0000C8A6  C50C04            lds ecx,[esp+eax]
0000C8A9  0400              add al,0x0
0000C8AB  001C00            add [eax+eax],bl
0000C8AE  0000              add [eax],al
0000C8B0  3C00              cmp al,0x0
0000C8B2  0000              add [eax],al
0000C8B4  846BFF            test [ebx-0x1],ch
0000C8B7  FF5700            call [edi+0x0]
0000C8BA  0000              add [eax],al
0000C8BC  00410E            add [ecx+0xe],al
0000C8BF  088502420D05      or [ebp+0x50d4202],al
0000C8C5  0253C5            add dl,[ebx-0x3b]
0000C8C8  0C04              or al,0x4
0000C8CA  0400              add al,0x0
0000C8CC  1C00              sbb al,0x0
0000C8CE  0000              add [eax],al
0000C8D0  5C                pop esp
0000C8D1  0000              add [eax],al
0000C8D3  00BB6BFFFF9F      add [ebx-0x60000095],bh
0000C8D9  0000              add [eax],al
0000C8DB  0000              add [eax],al
0000C8DD  41                inc ecx
0000C8DE  0E                push cs
0000C8DF  088502420D05      or [ebp+0x50d4202],al
0000C8E5  029BC50C0404      add bl,[ebx+0x4040cc5]
0000C8EB  001C00            add [eax+eax],bl
0000C8EE  0000              add [eax],al
0000C8F0  7C00              jl 0xc8f2
0000C8F2  0000              add [eax],al
0000C8F4  3A6CFFFF          cmp ch,[edi+edi*8-0x1]
0000C8F8  AA                stosb
0000C8F9  0000              add [eax],al
0000C8FB  0000              add [eax],al
0000C8FD  41                inc ecx
0000C8FE  0E                push cs
0000C8FF  088502420D05      or [ebp+0x50d4202],al
0000C905  02A6C50C0404      add ah,[esi+0x4040cc5]
0000C90B  001C00            add [eax+eax],bl
0000C90E  0000              add [eax],al
0000C910  9C                pushf
0000C911  0000              add [eax],al
0000C913  00C4              add ah,al
0000C915  6C                insb
0000C916  FF                db 0xff
0000C917  FF7500            push dword [ebp+0x0]
0000C91A  0000              add [eax],al
0000C91C  00410E            add [ecx+0xe],al
0000C91F  088502420D05      or [ebp+0x50d4202],al
0000C925  0271C5            add dh,[ecx-0x3b]
0000C928  0C04              or al,0x4
0000C92A  0400              add al,0x0
0000C92C  1C00              sbb al,0x0
0000C92E  0000              add [eax],al
0000C930  BC00000019        mov esp,0x19000000
0000C935  6D                insd
0000C936  FF                db 0xff
0000C937  FF                db 0xff
0000C938  DE00              fiadd word [eax]
0000C93A  0000              add [eax],al
0000C93C  00410E            add [ecx+0xe],al
0000C93F  088502420D05      or [ebp+0x50d4202],al
0000C945  02DA              add bl,dl
0000C947  C50C04            lds ecx,[esp+eax]
0000C94A  0400              add al,0x0
0000C94C  1C00              sbb al,0x0
0000C94E  0000              add [eax],al
0000C950  DC00              fadd qword [eax]
0000C952  0000              add [eax],al
0000C954  D7                xlatb
0000C955  6D                insd
0000C956  FF                db 0xff
0000C957  FF                db 0xff
0000C958  FE00              inc byte [eax]
0000C95A  0000              add [eax],al
0000C95C  00410E            add [ecx+0xe],al
0000C95F  088502420D05      or [ebp+0x50d4202],al
0000C965  02FA              add bh,dl
0000C967  C50C04            lds ecx,[esp+eax]
0000C96A  0400              add al,0x0
0000C96C  1C00              sbb al,0x0
0000C96E  0000              add [eax],al
0000C970  FC                cld
0000C971  0000              add [eax],al
0000C973  00B56EFFFF84      add [ebp-0x7b000092],dh
0000C979  0000              add [eax],al
0000C97B  0000              add [eax],al
0000C97D  41                inc ecx
0000C97E  0E                push cs
0000C97F  088502420D05      or [ebp+0x50d4202],al
0000C985  0280C50C0404      add al,[eax+0x4040cc5]
0000C98B  001C00            add [eax+eax],bl
0000C98E  0000              add [eax],al
0000C990  1C01              sbb al,0x1
0000C992  0000              add [eax],al
0000C994  196FFF            sbb [edi-0x1],ebp
0000C997  FF                db 0xff
0000C998  BD0F000000        mov ebp,0xf
0000C99D  41                inc ecx
0000C99E  0E                push cs
0000C99F  088502420D05      or [ebp+0x50d4202],al
0000C9A5  03B90FC50C04      add edi,[ecx+0x40cc50f]
0000C9AB  041C              add al,0x1c
0000C9AD  0000              add [eax],al
0000C9AF  003C01            add [ecx+eax],bh
0000C9B2  0000              add [eax],al
0000C9B4  B67E              mov dh,0x7e
0000C9B6  FF                db 0xff
0000C9B7  FF4701            inc dword [edi+0x1]
0000C9BA  0000              add [eax],al
0000C9BC  00410E            add [ecx+0xe],al
0000C9BF  088502420D05      or [ebp+0x50d4202],al
0000C9C5  034301            add eax,[ebx+0x1]
0000C9C8  C50C04            lds ecx,[esp+eax]
0000C9CB  041C              add al,0x1c
0000C9CD  0000              add [eax],al
0000C9CF  005C0100          add [ecx+eax+0x0],bl
0000C9D3  00DD              add ch,bl
0000C9D5  7FFF              jg 0xc9d6
0000C9D7  FF4B00            dec dword [ebx+0x0]
0000C9DA  0000              add [eax],al
0000C9DC  00410E            add [ecx+0xe],al
0000C9DF  088502420D05      or [ebp+0x50d4202],al
0000C9E5  0247C5            add al,[edi-0x3b]
0000C9E8  0C04              or al,0x4
0000C9EA  0400              add al,0x0
0000C9EC  1C00              sbb al,0x0
0000C9EE  0000              add [eax],al
0000C9F0  7C01              jl 0xc9f3
0000C9F2  0000              add [eax],al
0000C9F4  0880FFFFEB00      or [eax+0xebffff],al
0000C9FA  0000              add [eax],al
0000C9FC  00410E            add [ecx+0xe],al
0000C9FF  088502420D05      or [ebp+0x50d4202],al
0000CA05  02E7              add ah,bh
0000CA07  C50C04            lds ecx,[esp+eax]
0000CA0A  0400              add al,0x0
0000CA0C  1C00              sbb al,0x0
0000CA0E  0000              add [eax],al
0000CA10  9C                pushf
0000CA11  0100              add [eax],eax
0000CA13  00D3              add bl,dl
0000CA15  80FFFF            cmp bh,0xff
0000CA18  FF00              inc dword [eax]
0000CA1A  0000              add [eax],al
0000CA1C  00410E            add [ecx+0xe],al
0000CA1F  088502420D05      or [ebp+0x50d4202],al
0000CA25  02FB              add bh,bl
0000CA27  C50C04            lds ecx,[esp+eax]
0000CA2A  0400              add al,0x0
0000CA2C  1400              adc al,0x0
0000CA2E  0000              add [eax],al
0000CA30  0000              add [eax],al
0000CA32  0000              add [eax],al
0000CA34  017A52            add [edx+0x52],edi
0000CA37  0001              add [ecx],al
0000CA39  7C08              jl 0xca43
0000CA3B  011B              add [ebx],ebx
0000CA3D  0C04              or al,0x4
0000CA3F  0488              add al,0x88
0000CA41  0100              add [eax],eax
0000CA43  001C00            add [eax+eax],bl
0000CA46  0000              add [eax],al
0000CA48  1C00              sbb al,0x0
0000CA4A  0000              add [eax],al
0000CA4C  9A81FFFF1F0000    call 0x0:0x1fffff81
0000CA53  0000              add [eax],al
0000CA55  41                inc ecx
0000CA56  0E                push cs
0000CA57  088502420D05      or [ebp+0x50d4202],al
0000CA5D  5B                pop ebx
0000CA5E  C50C04            lds ecx,[esp+eax]
0000CA61  0400              add al,0x0
0000CA63  001C00            add [eax+eax],bl
0000CA66  0000              add [eax],al
0000CA68  3C00              cmp al,0x0
0000CA6A  0000              add [eax],al
0000CA6C  99                cdq
0000CA6D  81FFFF1D0000      cmp edi,0x1dff
0000CA73  0000              add [eax],al
0000CA75  41                inc ecx
0000CA76  0E                push cs
0000CA77  088502420D05      or [ebp+0x50d4202],al
0000CA7D  59                pop ecx
0000CA7E  C50C04            lds ecx,[esp+eax]
0000CA81  0400              add al,0x0
0000CA83  001C00            add [eax+eax],bl
0000CA86  0000              add [eax],al
0000CA88  5C                pop esp
0000CA89  0000              add [eax],al
0000CA8B  009681FFFF75      add [esi+0x75ffff81],dl
0000CA91  0000              add [eax],al
0000CA93  0000              add [eax],al
0000CA95  41                inc ecx
0000CA96  0E                push cs
0000CA97  088502420D05      or [ebp+0x50d4202],al
0000CA9D  0271C5            add dh,[ecx-0x3b]
0000CAA0  0C04              or al,0x4
0000CAA2  0400              add al,0x0
0000CAA4  1C00              sbb al,0x0
0000CAA6  0000              add [eax],al
0000CAA8  7C00              jl 0xcaaa
0000CAAA  0000              add [eax],al
0000CAAC  EB81              jmp short 0xca2f
0000CAAE  FF                db 0xff
0000CAAF  FF2400            jmp [eax+eax]
0000CAB2  0000              add [eax],al
0000CAB4  00410E            add [ecx+0xe],al
0000CAB7  088502420D05      or [ebp+0x50d4202],al
0000CABD  60                pusha
0000CABE  C50C04            lds ecx,[esp+eax]
0000CAC1  0400              add al,0x0
0000CAC3  001C00            add [eax+eax],bl
0000CAC6  0000              add [eax],al
0000CAC8  9C                pushf
0000CAC9  0000              add [eax],al
0000CACB  00EF              add bh,ch
0000CACD  81FFFF2A0000      cmp edi,0x2aff
0000CAD3  0000              add [eax],al
0000CAD5  41                inc ecx
0000CAD6  0E                push cs
0000CAD7  088502420D05      or [ebp+0x50d4202],al
0000CADD  66C50C04          lds cx,[esp+eax]
0000CAE1  0400              add al,0x0
0000CAE3  001400            add [eax+eax],dl
0000CAE6  0000              add [eax],al
0000CAE8  0000              add [eax],al
0000CAEA  0000              add [eax],al
0000CAEC  017A52            add [edx+0x52],edi
0000CAEF  0001              add [ecx],al
0000CAF1  7C08              jl 0xcafb
0000CAF3  011B              add [ebx],ebx
0000CAF5  0C04              or al,0x4
0000CAF7  0488              add al,0x88
0000CAF9  0100              add [eax],eax
0000CAFB  001C00            add [eax+eax],bl
0000CAFE  0000              add [eax],al
0000CB00  1C00              sbb al,0x0
0000CB02  0000              add [eax],al
0000CB04  E181              loope 0xca87
0000CB06  FF                db 0xff
0000CB07  FF13              call [ebx]
0000CB09  0000              add [eax],al
0000CB0B  0000              add [eax],al
0000CB0D  41                inc ecx
0000CB0E  0E                push cs
0000CB0F  088502420D05      or [ebp+0x50d4202],al
0000CB15  4F                dec edi
0000CB16  C50C04            lds ecx,[esp+eax]
0000CB19  0400              add al,0x0
0000CB1B  001C00            add [eax+eax],bl
0000CB1E  0000              add [eax],al
0000CB20  3C00              cmp al,0x0
0000CB22  0000              add [eax],al
0000CB24  D481              aam 0x81
0000CB26  FF                db 0xff
0000CB27  FF6D00            jmp far [ebp+0x0]
0000CB2A  0000              add [eax],al
0000CB2C  00410E            add [ecx+0xe],al
0000CB2F  088502420D05      or [ebp+0x50d4202],al
0000CB35  0269C5            add ch,[ecx-0x3b]
0000CB38  0C04              or al,0x4
0000CB3A  0400              add al,0x0
0000CB3C  1400              adc al,0x0
0000CB3E  0000              add [eax],al
0000CB40  0000              add [eax],al
0000CB42  0000              add [eax],al
0000CB44  017A52            add [edx+0x52],edi
0000CB47  0001              add [ecx],al
0000CB49  7C08              jl 0xcb53
0000CB4B  011B              add [ebx],ebx
0000CB4D  0C04              or al,0x4
0000CB4F  0488              add al,0x88
0000CB51  0100              add [eax],eax
0000CB53  001C00            add [eax+eax],bl
0000CB56  0000              add [eax],al
0000CB58  1C00              sbb al,0x0
0000CB5A  0000              add [eax],al
0000CB5C  0982FFFF1D00      or [edx+0x1dffff],eax
0000CB62  0000              add [eax],al
0000CB64  00410E            add [ecx+0xe],al
0000CB67  088502420D05      or [ebp+0x50d4202],al
0000CB6D  59                pop ecx
0000CB6E  C50C04            lds ecx,[esp+eax]
0000CB71  0400              add al,0x0
0000CB73  001C00            add [eax+eax],bl
0000CB76  0000              add [eax],al
0000CB78  3C00              cmp al,0x0
0000CB7A  0000              add [eax],al
0000CB7C  06                push es
0000CB7D  82                db 0x82
0000CB7E  FF                db 0xff
0000CB7F  FF18              call far [eax]
0000CB81  0000              add [eax],al
0000CB83  0000              add [eax],al
0000CB85  41                inc ecx
0000CB86  0E                push cs
0000CB87  088502420D05      or [ebp+0x50d4202],al
0000CB8D  54                push esp
0000CB8E  C50C04            lds ecx,[esp+eax]
0000CB91  0400              add al,0x0
0000CB93  001C00            add [eax+eax],bl
0000CB96  0000              add [eax],al
0000CB98  5C                pop esp
0000CB99  0000              add [eax],al
0000CB9B  00FE              add dh,bh
0000CB9D  81FFFF1A0000      cmp edi,0x1aff
0000CBA3  0000              add [eax],al
0000CBA5  41                inc ecx
0000CBA6  0E                push cs
0000CBA7  088502420D05      or [ebp+0x50d4202],al
0000CBAD  56                push esi
0000CBAE  C50C04            lds ecx,[esp+eax]
0000CBB1  0400              add al,0x0
0000CBB3  001C00            add [eax+eax],bl
0000CBB6  0000              add [eax],al
0000CBB8  7C00              jl 0xcbba
0000CBBA  0000              add [eax],al
0000CBBC  F8                clc
0000CBBD  81FFFF240000      cmp edi,0x24ff
0000CBC3  0000              add [eax],al
0000CBC5  41                inc ecx
0000CBC6  0E                push cs
0000CBC7  088502420D05      or [ebp+0x50d4202],al
0000CBCD  60                pusha
0000CBCE  C50C04            lds ecx,[esp+eax]
0000CBD1  0400              add al,0x0
0000CBD3  001C00            add [eax+eax],bl
0000CBD6  0000              add [eax],al
0000CBD8  9C                pushf
0000CBD9  0000              add [eax],al
0000CBDB  00FC              add ah,bh
0000CBDD  81FFFF170000      cmp edi,0x17ff
0000CBE3  0000              add [eax],al
0000CBE5  41                inc ecx
0000CBE6  0E                push cs
0000CBE7  088502420D05      or [ebp+0x50d4202],al
0000CBED  53                push ebx
0000CBEE  C50C04            lds ecx,[esp+eax]
0000CBF1  0400              add al,0x0
0000CBF3  001C00            add [eax+eax],bl
0000CBF6  0000              add [eax],al
0000CBF8  BC000000F3        mov esp,0xf3000000
0000CBFD  81FFFF620000      cmp edi,0x62ff
0000CC03  0000              add [eax],al
0000CC05  41                inc ecx
0000CC06  0E                push cs
0000CC07  088502420D05      or [ebp+0x50d4202],al
0000CC0D  025EC5            add bl,[esi-0x3b]
0000CC10  0C04              or al,0x4
0000CC12  0400              add al,0x0
0000CC14  1C00              sbb al,0x0
0000CC16  0000              add [eax],al
0000CC18  DC00              fadd qword [eax]
0000CC1A  0000              add [eax],al
0000CC1C  3582FFFF3F        xor eax,0x3fffff82
0000CC21  0000              add [eax],al
0000CC23  0000              add [eax],al
0000CC25  41                inc ecx
0000CC26  0E                push cs
0000CC27  088502420D05      or [ebp+0x50d4202],al
0000CC2D  7BC5              jpo 0xcbf4
0000CC2F  0C04              or al,0x4
0000CC31  0400              add al,0x0
0000CC33  001C00            add [eax+eax],bl
0000CC36  0000              add [eax],al
0000CC38  FC                cld
0000CC39  0000              add [eax],al
0000CC3B  005482FF          add [edx+eax*4-0x1],dl
0000CC3F  FF28              jmp far [eax]
0000CC41  0000              add [eax],al
0000CC43  0000              add [eax],al
0000CC45  41                inc ecx
0000CC46  0E                push cs
0000CC47  088502420D05      or [ebp+0x50d4202],al
0000CC4D  64C50C04          lds ecx,[fs:esp+eax]
0000CC51  0400              add al,0x0
0000CC53  0020              add [eax],ah
0000CC55  0000              add [eax],al
0000CC57  001C01            add [ecx+eax],bl
0000CC5A  0000              add [eax],al
0000CC5C  5C                pop esp
0000CC5D  82                db 0x82
0000CC5E  FF                db 0xff
0000CC5F  FF7201            push dword [edx+0x1]
0000CC62  0000              add [eax],al
0000CC64  00410E            add [ecx+0xe],al
0000CC67  088502420D05      or [ebp+0x50d4202],al
0000CC6D  44                inc esp
0000CC6E  830303            add dword [ebx],byte +0x3
0000CC71  6A01              push byte +0x1
0000CC73  C5                db 0xc5
0000CC74  C3                ret
0000CC75  0C04              or al,0x4
0000CC77  041C              add al,0x1c
0000CC79  0000              add [eax],al
0000CC7B  004001            add [eax+0x1],al
0000CC7E  0000              add [eax],al
0000CC80  AA                stosb
0000CC81  83FFFF            cmp edi,byte -0x1
0000CC84  3000              xor [eax],al
0000CC86  0000              add [eax],al
0000CC88  00410E            add [ecx+0xe],al
0000CC8B  088502420D05      or [ebp+0x50d4202],al
0000CC91  6C                insb
0000CC92  C50C04            lds ecx,[esp+eax]
0000CC95  0400              add al,0x0
0000CC97  001C00            add [eax+eax],bl
0000CC9A  0000              add [eax],al
0000CC9C  60                pusha
0000CC9D  0100              add [eax],eax
0000CC9F  00BA83FFFFC7      add [edx-0x3800007d],bh
0000CCA5  0000              add [eax],al
0000CCA7  0000              add [eax],al
0000CCA9  41                inc ecx
0000CCAA  0E                push cs
0000CCAB  088502420D05      or [ebp+0x50d4202],al
0000CCB1  02C3              add al,bl
0000CCB3  C50C04            lds ecx,[esp+eax]
0000CCB6  0400              add al,0x0
0000CCB8  1C00              sbb al,0x0
0000CCBA  0000              add [eax],al
0000CCBC  800100            add byte [ecx],0x0
0000CCBF  006184            add [ecx-0x7c],ah
0000CCC2  FF                db 0xff
0000CCC3  FF1B              call far [ebx]
0000CCC5  0000              add [eax],al
0000CCC7  0000              add [eax],al
0000CCC9  41                inc ecx
0000CCCA  0E                push cs
0000CCCB  088502420D05      or [ebp+0x50d4202],al
0000CCD1  57                push edi
0000CCD2  C50C04            lds ecx,[esp+eax]
0000CCD5  0400              add al,0x0
0000CCD7  001400            add [eax+eax],dl
0000CCDA  0000              add [eax],al
0000CCDC  0000              add [eax],al
0000CCDE  0000              add [eax],al
0000CCE0  017A52            add [edx+0x52],edi
0000CCE3  0001              add [ecx],al
0000CCE5  7C08              jl 0xccef
0000CCE7  011B              add [ebx],ebx
0000CCE9  0C04              or al,0x4
0000CCEB  0488              add al,0x88
0000CCED  0100              add [eax],eax
0000CCEF  001C00            add [eax+eax],bl
0000CCF2  0000              add [eax],al
0000CCF4  1C00              sbb al,0x0
0000CCF6  0000              add [eax],al
0000CCF8  44                inc esp
0000CCF9  84FF              test bh,bh
0000CCFB  FF                db 0xff
0000CCFC  3C00              cmp al,0x0
0000CCFE  0000              add [eax],al
0000CD00  00410E            add [ecx+0xe],al
0000CD03  088502420D05      or [ebp+0x50d4202],al
0000CD09  78C5              js 0xccd0
0000CD0B  0C04              or al,0x4
0000CD0D  0400              add al,0x0
0000CD0F  001C00            add [eax+eax],bl
0000CD12  0000              add [eax],al
0000CD14  3C00              cmp al,0x0
0000CD16  0000              add [eax],al
0000CD18  60                pusha
0000CD19  84FF              test bh,bh
0000CD1B  FF                db 0xff
0000CD1C  D800              fadd dword [eax]
0000CD1E  0000              add [eax],al
0000CD20  00410E            add [ecx+0xe],al
0000CD23  088502420D05      or [ebp+0x50d4202],al
0000CD29  02D4              add dl,ah
0000CD2B  C50C04            lds ecx,[esp+eax]
0000CD2E  0400              add al,0x0
0000CD30  1C00              sbb al,0x0
0000CD32  0000              add [eax],al
0000CD34  5C                pop esp
0000CD35  0000              add [eax],al
0000CD37  0018              add [eax],bl
0000CD39  85FF              test edi,edi
0000CD3B  FF540000          call [eax+eax+0x0]
0000CD3F  0000              add [eax],al
0000CD41  41                inc ecx
0000CD42  0E                push cs
0000CD43  088502420D05      or [ebp+0x50d4202],al
0000CD49  0250C5            add dl,[eax-0x3b]
0000CD4C  0C04              or al,0x4
0000CD4E  0400              add al,0x0
0000CD50  1C00              sbb al,0x0
0000CD52  0000              add [eax],al
0000CD54  7C00              jl 0xcd56
0000CD56  0000              add [eax],al
0000CD58  4C                dec esp
0000CD59  85FF              test edi,edi
0000CD5B  FFAA00000000      jmp far [edx+0x0]
0000CD61  41                inc ecx
0000CD62  0E                push cs
0000CD63  088502420D05      or [ebp+0x50d4202],al
0000CD69  02A6C50C0404      add ah,[esi+0x4040cc5]
0000CD6F  001C00            add [eax+eax],bl
0000CD72  0000              add [eax],al
0000CD74  9C                pushf
0000CD75  0000              add [eax],al
0000CD77  00D6              add dh,dl
0000CD79  85FF              test edi,edi
0000CD7B  FFCF              dec edi
0000CD7D  06                push es
0000CD7E  0000              add [eax],al
0000CD80  00410E            add [ecx+0xe],al
0000CD83  088502420D05      or [ebp+0x50d4202],al
0000CD89  03CB              add ecx,ebx
0000CD8B  06                push es
0000CD8C  C50C04            lds ecx,[esp+eax]
0000CD8F  0418              add al,0x18
0000CD91  0000              add [eax],al
0000CD93  00BC000000858C    add [eax+eax-0x737b0000],bh
0000CD9A  FF                db 0xff
0000CD9B  FF                db 0xff
0000CD9C  3D01000000        cmp eax,0x1
0000CDA1  41                inc ecx
0000CDA2  0E                push cs
0000CDA3  088502420D05      or [ebp+0x50d4202],al
0000CDA9  0000              add [eax],al
0000CDAB  001400            add [eax+eax],dl
0000CDAE  0000              add [eax],al
0000CDB0  0000              add [eax],al
0000CDB2  0000              add [eax],al
0000CDB4  017A52            add [edx+0x52],edi
0000CDB7  0001              add [ecx],al
0000CDB9  7C08              jl 0xcdc3
0000CDBB  011B              add [ebx],ebx
0000CDBD  0C04              or al,0x4
0000CDBF  0488              add al,0x88
0000CDC1  0100              add [eax],eax
0000CDC3  001C00            add [eax+eax],bl
0000CDC6  0000              add [eax],al
0000CDC8  1C00              sbb al,0x0
0000CDCA  0000              add [eax],al
0000CDCC  8E8DFFFF3F00      mov cs,[ebp+0x3fffff]
0000CDD2  0000              add [eax],al
0000CDD4  00410E            add [ecx+0xe],al
0000CDD7  088502420D05      or [ebp+0x50d4202],al
0000CDDD  7BC5              jpo 0xcda4
0000CDDF  0C04              or al,0x4
0000CDE1  0400              add al,0x0
0000CDE3  001C00            add [eax+eax],bl
0000CDE6  0000              add [eax],al
0000CDE8  3C00              cmp al,0x0
0000CDEA  0000              add [eax],al
0000CDEC  AD                lodsd
0000CDED  8D                db 0x8d
0000CDEE  FF                db 0xff
0000CDEF  FF27              jmp [edi]
0000CDF1  0000              add [eax],al
0000CDF3  0000              add [eax],al
0000CDF5  41                inc ecx
0000CDF6  0E                push cs
0000CDF7  088502420D05      or [ebp+0x50d4202],al
0000CDFD  63C5              arpl bp,ax
0000CDFF  0C04              or al,0x4
0000CE01  0400              add al,0x0
0000CE03  001C00            add [eax+eax],bl
0000CE06  0000              add [eax],al
0000CE08  5C                pop esp
0000CE09  0000              add [eax],al
0000CE0B  00B48DFFFF2E00    add [ebp+ecx*4+0x2effff],dh
0000CE12  0000              add [eax],al
0000CE14  00410E            add [ecx+0xe],al
0000CE17  088502420D05      or [ebp+0x50d4202],al
0000CE1D  6AC5              push byte -0x3b
0000CE1F  0C04              or al,0x4
0000CE21  0400              add al,0x0
0000CE23  001C00            add [eax+eax],bl
0000CE26  0000              add [eax],al
0000CE28  7C00              jl 0xce2a
0000CE2A  0000              add [eax],al
0000CE2C  C28DFF            ret 0xff8d
0000CE2F  FF                db 0xff
0000CE30  3A00              cmp al,[eax]
0000CE32  0000              add [eax],al
0000CE34  00410E            add [ecx+0xe],al
0000CE37  088502420D05      or [ebp+0x50d4202],al
0000CE3D  76C5              jna 0xce04
0000CE3F  0C04              or al,0x4
0000CE41  0400              add al,0x0
0000CE43  001C00            add [eax+eax],bl
0000CE46  0000              add [eax],al
0000CE48  9C                pushf
0000CE49  0000              add [eax],al
0000CE4B  00DC              add ah,bl
0000CE4D  8D                db 0x8d
0000CE4E  FF                db 0xff
0000CE4F  FF2C00            jmp far [eax+eax]
0000CE52  0000              add [eax],al
0000CE54  00410E            add [ecx+0xe],al
0000CE57  088502420D05      or [ebp+0x50d4202],al
0000CE5D  68C50C0404        push dword 0x4040cc5
0000CE62  0000              add [eax],al
0000CE64  1C00              sbb al,0x0
0000CE66  0000              add [eax],al
0000CE68  BC000000E8        mov esp,0xe8000000
0000CE6D  8D                db 0x8d
0000CE6E  FF                db 0xff
0000CE6F  FF5600            call [esi+0x0]
0000CE72  0000              add [eax],al
0000CE74  00410E            add [ecx+0xe],al
0000CE77  088502420D05      or [ebp+0x50d4202],al
0000CE7D  0252C5            add dl,[edx-0x3b]
0000CE80  0C04              or al,0x4
0000CE82  0400              add al,0x0
0000CE84  1C00              sbb al,0x0
0000CE86  0000              add [eax],al
0000CE88  DC00              fadd qword [eax]
0000CE8A  0000              add [eax],al
0000CE8C  1E                push ds
0000CE8D  8EFF              mov segr7,edi
0000CE8F  FF4900            dec dword [ecx+0x0]
0000CE92  0000              add [eax],al
0000CE94  00410E            add [ecx+0xe],al
0000CE97  088502420D05      or [ebp+0x50d4202],al
0000CE9D  0245C5            add al,[ebp-0x3b]
0000CEA0  0C04              or al,0x4
0000CEA2  0400              add al,0x0
0000CEA4  1400              adc al,0x0
0000CEA6  0000              add [eax],al
0000CEA8  0000              add [eax],al
0000CEAA  0000              add [eax],al
0000CEAC  017A52            add [edx+0x52],edi
0000CEAF  0001              add [ecx],al
0000CEB1  7C08              jl 0xcebb
0000CEB3  011B              add [ebx],ebx
0000CEB5  0C04              or al,0x4
0000CEB7  0488              add al,0x88
0000CEB9  0100              add [eax],eax
0000CEBB  001C00            add [eax+eax],bl
0000CEBE  0000              add [eax],al
0000CEC0  1C00              sbb al,0x0
0000CEC2  0000              add [eax],al
0000CEC4  2F                das
0000CEC5  8EFF              mov segr7,edi
0000CEC7  FF1F              call far [edi]
0000CEC9  0000              add [eax],al
0000CECB  0000              add [eax],al
0000CECD  41                inc ecx
0000CECE  0E                push cs
0000CECF  088502420D05      or [ebp+0x50d4202],al
0000CED5  5B                pop ebx
0000CED6  C50C04            lds ecx,[esp+eax]
0000CED9  0400              add al,0x0
0000CEDB  001C00            add [eax+eax],bl
0000CEDE  0000              add [eax],al
0000CEE0  3C00              cmp al,0x0
0000CEE2  0000              add [eax],al
0000CEE4  2E8EFF            cs mov segr7,edi
0000CEE7  FF1D00000000      call far [dword 0x0]
0000CEED  41                inc ecx
0000CEEE  0E                push cs
0000CEEF  088502420D05      or [ebp+0x50d4202],al
0000CEF5  59                pop ecx
0000CEF6  C50C04            lds ecx,[esp+eax]
0000CEF9  0400              add al,0x0
0000CEFB  002400            add [eax+eax],ah
0000CEFE  0000              add [eax],al
0000CF00  5C                pop esp
0000CF01  0000              add [eax],al
0000CF03  002B              add [ebx],ch
0000CF05  8EFF              mov segr7,edi
0000CF07  FF3500000000      push dword [dword 0x0]
0000CF0D  41                inc ecx
0000CF0E  0E                push cs
0000CF0F  088502420D05      or [ebp+0x50d4202],al
0000CF15  45                inc ebp
0000CF16  8703              xchg eax,[ebx]
0000CF18  83046AC3          add dword [edx+ebp*2],byte -0x3d
0000CF1C  41                inc ecx
0000CF1D  C741C50C040400    mov dword [ecx-0x3b],0x4040c
0000CF24  2400              and al,0x0
0000CF26  0000              add [eax],al
0000CF28  8400              test [eax],al
0000CF2A  0000              add [eax],al
0000CF2C  388EFFFF3500      cmp [esi+0x35ffff],cl
0000CF32  0000              add [eax],al
0000CF34  00410E            add [ecx+0xe],al
0000CF37  088502420D05      or [ebp+0x50d4202],al
0000CF3D  45                inc ebp
0000CF3E  8603              xchg al,[ebx]
0000CF40  83046AC3          add dword [edx+ebp*2],byte -0x3d
0000CF44  41                inc ecx
0000CF45  C641C50C          mov byte [ecx-0x3b],0xc
0000CF49  0404              add al,0x4
0000CF4B  001C00            add [eax+eax],bl
0000CF4E  0000              add [eax],al
0000CF50  AC                lodsb
0000CF51  0000              add [eax],al
0000CF53  00458E            add [ebp-0x72],al
0000CF56  FF                db 0xff
0000CF57  FF2F              jmp far [edi]
0000CF59  0000              add [eax],al
0000CF5B  0000              add [eax],al
0000CF5D  41                inc ecx
0000CF5E  0E                push cs
0000CF5F  088502420D05      or [ebp+0x50d4202],al
0000CF65  6BC50C            imul eax,ebp,byte +0xc
0000CF68  0404              add al,0x4
0000CF6A  0000              add [eax],al
0000CF6C  1C00              sbb al,0x0
0000CF6E  0000              add [eax],al
0000CF70  CC                int3
0000CF71  0000              add [eax],al
0000CF73  00548EFF          add [esi+ecx*4-0x1],dl
0000CF77  FF3500000000      push dword [dword 0x0]
0000CF7D  41                inc ecx
0000CF7E  0E                push cs
0000CF7F  088502420D05      or [ebp+0x50d4202],al
0000CF85  71C5              jno 0xcf4c
0000CF87  0C04              or al,0x4
0000CF89  0400              add al,0x0
0000CF8B  001C00            add [eax+eax],bl
0000CF8E  0000              add [eax],al
0000CF90  EC                in al,dx
0000CF91  0000              add [eax],al
0000CF93  00698E            add [ecx-0x72],ch
0000CF96  FF                db 0xff
0000CF97  FFAB00000000      jmp far [ebx+0x0]
0000CF9D  41                inc ecx
0000CF9E  0E                push cs
0000CF9F  088502420D05      or [ebp+0x50d4202],al
0000CFA5  02A7C50C0404      add ah,[edi+0x4040cc5]
0000CFAB  001C00            add [eax+eax],bl
0000CFAE  0000              add [eax],al
0000CFB0  0C01              or al,0x1
0000CFB2  0000              add [eax],al
0000CFB4  F4                hlt
0000CFB5  8EFF              mov segr7,edi
0000CFB7  FFA700000000      jmp [edi+0x0]
0000CFBD  41                inc ecx
0000CFBE  0E                push cs
0000CFBF  088502420D05      or [ebp+0x50d4202],al
0000CFC5  02A3C50C0404      add ah,[ebx+0x4040cc5]
0000CFCB  001400            add [eax+eax],dl
0000CFCE  0000              add [eax],al
0000CFD0  0000              add [eax],al
0000CFD2  0000              add [eax],al
0000CFD4  017A52            add [edx+0x52],edi
0000CFD7  0001              add [ecx],al
0000CFD9  7C08              jl 0xcfe3
0000CFDB  011B              add [ebx],ebx
0000CFDD  0C04              or al,0x4
0000CFDF  0488              add al,0x88
0000CFE1  0100              add [eax],eax
0000CFE3  001C00            add [eax+eax],bl
0000CFE6  0000              add [eax],al
0000CFE8  1C00              sbb al,0x0
0000CFEA  0000              add [eax],al
0000CFEC  638FFFFF4D00      arpl [edi+0x4dffff],cx
0000CFF2  0000              add [eax],al
0000CFF4  00410E            add [ecx+0xe],al
0000CFF7  088502420D05      or [ebp+0x50d4202],al
0000CFFD  0249C5            add cl,[ecx-0x3b]
0000D000  0C04              or al,0x4
0000D002  0400              add al,0x0
0000D004  1C00              sbb al,0x0
0000D006  0000              add [eax],al
0000D008  3C00              cmp al,0x0
0000D00A  0000              add [eax],al
0000D00C  90                nop
0000D00D  8F                db 0x8f
0000D00E  FF                db 0xff
0000D00F  FF4600            inc dword [esi+0x0]
0000D012  0000              add [eax],al
0000D014  00410E            add [ecx+0xe],al
0000D017  088502420D05      or [ebp+0x50d4202],al
0000D01D  0242C5            add al,[edx-0x3b]
0000D020  0C04              or al,0x4
0000D022  0400              add al,0x0
0000D024  1C00              sbb al,0x0
0000D026  0000              add [eax],al
0000D028  5C                pop esp
0000D029  0000              add [eax],al
0000D02B  00B68FFFFF34      add [esi+0x34ffff8f],dh
0000D031  0000              add [eax],al
0000D033  0000              add [eax],al
0000D035  41                inc ecx
0000D036  0E                push cs
0000D037  088502420D05      or [ebp+0x50d4202],al
0000D03D  70C5              jo 0xd004
0000D03F  0C04              or al,0x4
0000D041  0400              add al,0x0
0000D043  001C00            add [eax+eax],bl
0000D046  0000              add [eax],al
0000D048  7C00              jl 0xd04a
0000D04A  0000              add [eax],al
0000D04C  CA8FFF            retf 0xff8f
0000D04F  FF540000          call [eax+eax+0x0]
0000D053  0000              add [eax],al
0000D055  41                inc ecx
0000D056  0E                push cs
0000D057  088502420D05      or [ebp+0x50d4202],al
0000D05D  0250C5            add dl,[eax-0x3b]
0000D060  0C04              or al,0x4
0000D062  0400              add al,0x0
0000D064  2000              and [eax],al
0000D066  0000              add [eax],al
0000D068  9C                pushf
0000D069  0000              add [eax],al
0000D06B  00FE              add dh,bh
0000D06D  8F                db 0x8f
0000D06E  FF                db 0xff
0000D06F  FF9E00000000      call far [esi+0x0]
0000D075  41                inc ecx
0000D076  0E                push cs
0000D077  088502420D05      or [ebp+0x50d4202],al
0000D07D  44                inc esp
0000D07E  830302            add dword [ebx],byte +0x2
0000D081  96                xchg eax,esi
0000D082  C5                db 0xc5
0000D083  C3                ret
0000D084  0C04              or al,0x4
0000D086  0400              add al,0x0
0000D088  1C00              sbb al,0x0
0000D08A  0000              add [eax],al
0000D08C  C00000            rol byte [eax],byte 0x0
0000D08F  007890            add [eax-0x70],bh
0000D092  FF                db 0xff
0000D093  FFAF00000000      jmp far [edi+0x0]
0000D099  41                inc ecx
0000D09A  0E                push cs
0000D09B  088502420D05      or [ebp+0x50d4202],al
0000D0A1  02ABC50C0404      add ch,[ebx+0x4040cc5]
0000D0A7  001C00            add [eax+eax],bl
0000D0AA  0000              add [eax],al
0000D0AC  E000              loopne 0xd0ae
0000D0AE  0000              add [eax],al
0000D0B0  07                pop es
0000D0B1  91                xchg eax,ecx
0000D0B2  FF                db 0xff
0000D0B3  FF9500000000      call [ebp+0x0]
0000D0B9  41                inc ecx
0000D0BA  0E                push cs
0000D0BB  088502420D05      or [ebp+0x50d4202],al
0000D0C1  0291C50C0404      add dl,[ecx+0x4040cc5]
0000D0C7  001C00            add [eax+eax],bl
0000D0CA  0000              add [eax],al
0000D0CC  0001              add [ecx],al
0000D0CE  0000              add [eax],al
0000D0D0  7C91              jl 0xd063
0000D0D2  FF                db 0xff
0000D0D3  FF6900            jmp far [ecx+0x0]
0000D0D6  0000              add [eax],al
0000D0D8  00410E            add [ecx+0xe],al
0000D0DB  088502420D05      or [ebp+0x50d4202],al
0000D0E1  0265C5            add ah,[ebp-0x3b]
0000D0E4  0C04              or al,0x4
0000D0E6  0400              add al,0x0
0000D0E8  1400              adc al,0x0
0000D0EA  0000              add [eax],al
0000D0EC  0000              add [eax],al
0000D0EE  0000              add [eax],al
0000D0F0  017A52            add [edx+0x52],edi
0000D0F3  0001              add [ecx],al
0000D0F5  7C08              jl 0xd0ff
0000D0F7  011B              add [ebx],ebx
0000D0F9  0C04              or al,0x4
0000D0FB  0488              add al,0x88
0000D0FD  0100              add [eax],eax
0000D0FF  001C00            add [eax+eax],bl
0000D102  0000              add [eax],al
0000D104  1C00              sbb al,0x0
0000D106  0000              add [eax],al
0000D108  AD                lodsd
0000D109  91                xchg eax,ecx
0000D10A  FF                db 0xff
0000D10B  FF6E00            jmp far [esi+0x0]
0000D10E  0000              add [eax],al
0000D110  00410E            add [ecx+0xe],al
0000D113  088502420D05      or [ebp+0x50d4202],al
0000D119  026AC5            add ch,[edx-0x3b]
0000D11C  0C04              or al,0x4
0000D11E  0400              add al,0x0
0000D120  1C00              sbb al,0x0
0000D122  0000              add [eax],al
0000D124  3C00              cmp al,0x0
0000D126  0000              add [eax],al
0000D128  FB                sti
0000D129  91                xchg eax,ecx
0000D12A  FF                db 0xff
0000D12B  FF4D00            dec dword [ebp+0x0]
0000D12E  0000              add [eax],al
0000D130  00410E            add [ecx+0xe],al
0000D133  088502420D05      or [ebp+0x50d4202],al
0000D139  0249C5            add cl,[ecx-0x3b]
0000D13C  0C04              or al,0x4
0000D13E  0400              add al,0x0
0000D140  1C00              sbb al,0x0
0000D142  0000              add [eax],al
0000D144  5C                pop esp
0000D145  0000              add [eax],al
0000D147  0028              add [eax],ch
0000D149  92                xchg eax,edx
0000D14A  FF                db 0xff
0000D14B  FF22              jmp [edx]
0000D14D  0000              add [eax],al
0000D14F  0000              add [eax],al
0000D151  41                inc ecx
0000D152  0E                push cs
0000D153  088502420D05      or [ebp+0x50d4202],al
0000D159  5E                pop esi
0000D15A  C50C04            lds ecx,[esp+eax]
0000D15D  0400              add al,0x0
0000D15F  001C00            add [eax+eax],bl
0000D162  0000              add [eax],al
0000D164  7C00              jl 0xd166
0000D166  0000              add [eax],al
0000D168  2A92FFFF5101      sub dl,[edx+0x151ffff]
0000D16E  0000              add [eax],al
0000D170  00410E            add [ecx+0xe],al
0000D173  088502420D05      or [ebp+0x50d4202],al
0000D179  034D01            add ecx,[ebp+0x1]
0000D17C  C50C04            lds ecx,[esp+eax]
0000D17F  041C              add al,0x1c
0000D181  0000              add [eax],al
0000D183  009C0000005B93    add [eax+eax-0x6ca50000],bl
0000D18A  FF                db 0xff
0000D18B  FF0A              dec dword [edx]
0000D18D  0000              add [eax],al
0000D18F  0000              add [eax],al
0000D191  41                inc ecx
0000D192  0E                push cs
0000D193  088502420D05      or [ebp+0x50d4202],al
0000D199  46                inc esi
0000D19A  C50C04            lds ecx,[esp+eax]
0000D19D  0400              add al,0x0
0000D19F  001C00            add [eax+eax],bl
0000D1A2  0000              add [eax],al
0000D1A4  BC00000045        mov esp,0x45000000
0000D1A9  93                xchg eax,ebx
0000D1AA  FF                db 0xff
0000D1AB  FF0A              dec dword [edx]
0000D1AD  0000              add [eax],al
0000D1AF  0000              add [eax],al
0000D1B1  41                inc ecx
0000D1B2  0E                push cs
0000D1B3  088502420D05      or [ebp+0x50d4202],al
0000D1B9  46                inc esi
0000D1BA  C50C04            lds ecx,[esp+eax]
0000D1BD  0400              add al,0x0
0000D1BF  001C00            add [eax+eax],bl
0000D1C2  0000              add [eax],al
0000D1C4  DC00              fadd qword [eax]
0000D1C6  0000              add [eax],al
0000D1C8  2F                das
0000D1C9  93                xchg eax,ebx
0000D1CA  FF                db 0xff
0000D1CB  FF2500000000      jmp [dword 0x0]
0000D1D1  41                inc ecx
0000D1D2  0E                push cs
0000D1D3  088502420D05      or [ebp+0x50d4202],al
0000D1D9  61                popa
0000D1DA  C50C04            lds ecx,[esp+eax]
0000D1DD  0400              add al,0x0
0000D1DF  001C00            add [eax+eax],bl
0000D1E2  0000              add [eax],al
0000D1E4  FC                cld
0000D1E5  0000              add [eax],al
0000D1E7  003493            add [ebx+edx*4],dh
0000D1EA  FF                db 0xff
0000D1EB  FF0F              dec dword [edi]
0000D1ED  0000              add [eax],al
0000D1EF  0000              add [eax],al
0000D1F1  41                inc ecx
0000D1F2  0E                push cs
0000D1F3  088502420D05      or [ebp+0x50d4202],al
0000D1F9  4B                dec ebx
0000D1FA  C50C04            lds ecx,[esp+eax]
0000D1FD  0400              add al,0x0
0000D1FF  001C00            add [eax+eax],bl
0000D202  0000              add [eax],al
0000D204  1C01              sbb al,0x1
0000D206  0000              add [eax],al
0000D208  2393FFFF0F00      and edx,[ebx+0xfffff]
0000D20E  0000              add [eax],al
0000D210  00410E            add [ecx+0xe],al
0000D213  088502420D05      or [ebp+0x50d4202],al
0000D219  4B                dec ebx
0000D21A  C50C04            lds ecx,[esp+eax]
0000D21D  0400              add al,0x0
0000D21F  001C00            add [eax+eax],bl
0000D222  0000              add [eax],al
0000D224  3C01              cmp al,0x1
0000D226  0000              add [eax],al
0000D228  1293FFFF0F00      adc dl,[ebx+0xfffff]
0000D22E  0000              add [eax],al
0000D230  00410E            add [ecx+0xe],al
0000D233  088502420D05      or [ebp+0x50d4202],al
0000D239  4B                dec ebx
0000D23A  C50C04            lds ecx,[esp+eax]
0000D23D  0400              add al,0x0
0000D23F  001C00            add [eax+eax],bl
0000D242  0000              add [eax],al
0000D244  5C                pop esp
0000D245  0100              add [eax],eax
0000D247  0001              add [ecx],al
0000D249  93                xchg eax,ebx
0000D24A  FF                db 0xff
0000D24B  FF6A01            jmp far [edx+0x1]
0000D24E  0000              add [eax],al
0000D250  00410E            add [ecx+0xe],al
0000D253  088502420D05      or [ebp+0x50d4202],al
0000D259  036601            add esp,[esi+0x1]
0000D25C  C50C04            lds ecx,[esp+eax]
0000D25F  041C              add al,0x1c
0000D261  0000              add [eax],al
0000D263  007C0100          add [ecx+eax+0x0],bh
0000D267  004B94            add [ebx-0x6c],cl
0000D26A  FF                db 0xff
0000D26B  FFCC              dec esp
0000D26D  0000              add [eax],al
0000D26F  0000              add [eax],al
0000D271  41                inc ecx
0000D272  0E                push cs
0000D273  088502420D05      or [ebp+0x50d4202],al
0000D279  02C8              add cl,al
0000D27B  C50C04            lds ecx,[esp+eax]
0000D27E  0400              add al,0x0
0000D280  1400              adc al,0x0
0000D282  0000              add [eax],al
0000D284  0000              add [eax],al
0000D286  0000              add [eax],al
0000D288  017A52            add [edx+0x52],edi
0000D28B  0001              add [ecx],al
0000D28D  7C08              jl 0xd297
0000D28F  011B              add [ebx],ebx
0000D291  0C04              or al,0x4
0000D293  0488              add al,0x88
0000D295  0100              add [eax],eax
0000D297  001C00            add [eax+eax],bl
0000D29A  0000              add [eax],al
0000D29C  1C00              sbb al,0x0
0000D29E  0000              add [eax],al
0000D2A0  DF94FFFF1E0000    fist word [edi+edi*8+0x1eff]
0000D2A7  0000              add [eax],al
0000D2A9  41                inc ecx
0000D2AA  0E                push cs
0000D2AB  088502420D05      or [ebp+0x50d4202],al
0000D2B1  5A                pop edx
0000D2B2  C50C04            lds ecx,[esp+eax]
0000D2B5  0400              add al,0x0
0000D2B7  001C00            add [eax+eax],bl
0000D2BA  0000              add [eax],al
0000D2BC  3C00              cmp al,0x0
0000D2BE  0000              add [eax],al
0000D2C0  DD94FFFF230000    fst qword [edi+edi*8+0x23ff]
0000D2C7  0000              add [eax],al
0000D2C9  41                inc ecx
0000D2CA  0E                push cs
0000D2CB  088502420D05      or [ebp+0x50d4202],al
0000D2D1  5F                pop edi
0000D2D2  C50C04            lds ecx,[esp+eax]
0000D2D5  0400              add al,0x0
0000D2D7  001C00            add [eax+eax],bl
0000D2DA  0000              add [eax],al
0000D2DC  5C                pop esp
0000D2DD  0000              add [eax],al
0000D2DF  00E0              add al,ah
0000D2E1  94                xchg eax,esp
0000D2E2  FF                db 0xff
0000D2E3  FF06              inc dword [esi]
0000D2E5  0000              add [eax],al
0000D2E7  0000              add [eax],al
0000D2E9  41                inc ecx
0000D2EA  0E                push cs
0000D2EB  088502420D05      or [ebp+0x50d4202],al
0000D2F1  42                inc edx
0000D2F2  C50C04            lds ecx,[esp+eax]
0000D2F5  0400              add al,0x0
0000D2F7  001C00            add [eax+eax],bl
0000D2FA  0000              add [eax],al
0000D2FC  7C00              jl 0xd2fe
0000D2FE  0000              add [eax],al
0000D300  C6                db 0xc6
0000D301  94                xchg eax,esp
0000D302  FF                db 0xff
0000D303  FF1400            call [eax+eax]
0000D306  0000              add [eax],al
0000D308  00410E            add [ecx+0xe],al
0000D30B  088502420D05      or [ebp+0x50d4202],al
0000D311  50                push eax
0000D312  C50C04            lds ecx,[esp+eax]
0000D315  0400              add al,0x0
0000D317  001C00            add [eax+eax],bl
0000D31A  0000              add [eax],al
0000D31C  9C                pushf
0000D31D  0000              add [eax],al
0000D31F  00BA94FFFF0A      add [edx+0xaffff94],bh
0000D325  0000              add [eax],al
0000D327  0000              add [eax],al
0000D329  41                inc ecx
0000D32A  0E                push cs
0000D32B  088502420D05      or [ebp+0x50d4202],al
0000D331  46                inc esi
0000D332  C50C04            lds ecx,[esp+eax]
0000D335  0400              add al,0x0
0000D337  001C00            add [eax+eax],bl
0000D33A  0000              add [eax],al
0000D33C  BC000000A4        mov esp,0xa4000000
0000D341  94                xchg eax,esp
0000D342  FF                db 0xff
0000D343  FF33              push dword [ebx]
0000D345  0000              add [eax],al
0000D347  0000              add [eax],al
0000D349  41                inc ecx
0000D34A  0E                push cs
0000D34B  088502420D05      or [ebp+0x50d4202],al
0000D351  6F                outsd
0000D352  C50C04            lds ecx,[esp+eax]
0000D355  0400              add al,0x0
0000D357  001400            add [eax+eax],dl
0000D35A  0000              add [eax],al
0000D35C  0000              add [eax],al
0000D35E  0000              add [eax],al
0000D360  017A52            add [edx+0x52],edi
0000D363  0001              add [ecx],al
0000D365  7C08              jl 0xd36f
0000D367  011B              add [ebx],ebx
0000D369  0C04              or al,0x4
0000D36B  0488              add al,0x88
0000D36D  0100              add [eax],eax
0000D36F  001C00            add [eax+eax],bl
0000D372  0000              add [eax],al
0000D374  1C00              sbb al,0x0
0000D376  0000              add [eax],al
0000D378  9F                lahf
0000D379  94                xchg eax,esp
0000D37A  FF                db 0xff
0000D37B  FF2C00            jmp far [eax+eax]
0000D37E  0000              add [eax],al
0000D380  00410E            add [ecx+0xe],al
0000D383  088502420D05      or [ebp+0x50d4202],al
0000D389  68C50C0404        push dword 0x4040cc5
0000D38E  0000              add [eax],al
0000D390  1C00              sbb al,0x0
0000D392  0000              add [eax],al
0000D394  3C00              cmp al,0x0
0000D396  0000              add [eax],al
0000D398  AB                stosd
0000D399  94                xchg eax,esp
0000D39A  FF                db 0xff
0000D39B  FF29              jmp far [ecx]
0000D39D  0000              add [eax],al
0000D39F  0000              add [eax],al
0000D3A1  41                inc ecx
0000D3A2  0E                push cs
0000D3A3  088502420D05      or [ebp+0x50d4202],al
0000D3A9  65C50C04          lds ecx,[gs:esp+eax]
0000D3AD  0400              add al,0x0
0000D3AF  001C00            add [eax+eax],bl
0000D3B2  0000              add [eax],al
0000D3B4  5C                pop esp
0000D3B5  0000              add [eax],al
0000D3B7  00B494FFFF0600    add [esp+edx*4+0x6ffff],dh
0000D3BE  0000              add [eax],al
0000D3C0  00410E            add [ecx+0xe],al
0000D3C3  088502420D05      or [ebp+0x50d4202],al
0000D3C9  42                inc edx
0000D3CA  C50C04            lds ecx,[esp+eax]
0000D3CD  0400              add al,0x0
0000D3CF  001C00            add [eax+eax],bl
0000D3D2  0000              add [eax],al
0000D3D4  7C00              jl 0xd3d6
0000D3D6  0000              add [eax],al
0000D3D8  9A94FFFF110200    call 0x2:0x11ffff94
0000D3DF  0000              add [eax],al
0000D3E1  41                inc ecx
0000D3E2  0E                push cs
0000D3E3  088502420D05      or [ebp+0x50d4202],al
0000D3E9  030D02C50C04      add ecx,[dword 0x40cc502]
0000D3EF  0414              add al,0x14
0000D3F1  0000              add [eax],al
0000D3F3  0000              add [eax],al
0000D3F5  0000              add [eax],al
0000D3F7  0001              add [ecx],al
0000D3F9  7A52              jpe 0xd44d
0000D3FB  0001              add [ecx],al
0000D3FD  7C08              jl 0xd407
0000D3FF  011B              add [ebx],ebx
0000D401  0C04              or al,0x4
0000D403  0488              add al,0x88
0000D405  0100              add [eax],eax
0000D407  001C00            add [eax+eax],bl
0000D40A  0000              add [eax],al
0000D40C  1C00              sbb al,0x0
0000D40E  0000              add [eax],al
0000D410  7396              jnc 0xd3a8
0000D412  FF                db 0xff
0000D413  FF10              call [eax]
0000D415  0000              add [eax],al
0000D417  0000              add [eax],al
0000D419  41                inc ecx
0000D41A  0E                push cs
0000D41B  088502420D05      or [ebp+0x50d4202],al
0000D421  4C                dec esp
0000D422  C50C04            lds ecx,[esp+eax]
0000D425  0400              add al,0x0
0000D427  001C00            add [eax+eax],bl
0000D42A  0000              add [eax],al
0000D42C  3C00              cmp al,0x0
0000D42E  0000              add [eax],al
0000D430  6396FFFFB300      arpl [esi+0xb3ffff],dx
0000D436  0000              add [eax],al
0000D438  00410E            add [ecx+0xe],al
0000D43B  088502420D05      or [ebp+0x50d4202],al
0000D441  02AFC50C0404      add ch,[edi+0x4040cc5]
0000D447  0020              add [eax],ah
0000D449  0000              add [eax],al
0000D44B  005C0000          add [eax+eax+0x0],bl
0000D44F  00F6              add dh,dh
0000D451  96                xchg eax,esi
0000D452  FF                db 0xff
0000D453  FF8102000000      inc dword [ecx+0x2]
0000D459  41                inc ecx
0000D45A  0E                push cs
0000D45B  088502420D05      or [ebp+0x50d4202],al
0000D461  44                inc esp
0000D462  830303            add dword [ebx],byte +0x3
0000D465  7902              jns 0xd469
0000D467  C5                db 0xc5
0000D468  C3                ret
0000D469  0C04              or al,0x4
0000D46B  041C              add al,0x1c
0000D46D  0000              add [eax],al
0000D46F  008000000053      add [eax+0x53000000],al
0000D475  99                cdq
0000D476  FF                db 0xff
0000D477  FF4101            inc dword [ecx+0x1]
0000D47A  0000              add [eax],al
0000D47C  00410E            add [ecx+0xe],al
0000D47F  088502420D05      or [ebp+0x50d4202],al
0000D485  033D01C50C04      add edi,[dword 0x40cc501]
0000D48B  041C              add al,0x1c
0000D48D  0000              add [eax],al
0000D48F  00A000000074      add [eax+0x74000000],ah
0000D495  9AFFFF59010000    call 0x0:0x159ffff
0000D49C  00410E            add [ecx+0xe],al
0000D49F  088502420D05      or [ebp+0x50d4202],al
0000D4A5  035501            add edx,[ebp+0x1]
0000D4A8  C50C04            lds ecx,[esp+eax]
0000D4AB  041C              add al,0x1c
0000D4AD  0000              add [eax],al
0000D4AF  00C0              add al,al
0000D4B1  0000              add [eax],al
0000D4B3  00AD9BFFFF7E      add [ebp+0x7effff9b],ch
0000D4B9  0000              add [eax],al
0000D4BB  0000              add [eax],al
0000D4BD  41                inc ecx
0000D4BE  0E                push cs
0000D4BF  088502420D05      or [ebp+0x50d4202],al
0000D4C5  027AC5            add bh,[edx-0x3b]
0000D4C8  0C04              or al,0x4
0000D4CA  0400              add al,0x0
0000D4CC  1C00              sbb al,0x0
0000D4CE  0000              add [eax],al
0000D4D0  E000              loopne 0xd4d2
0000D4D2  0000              add [eax],al
0000D4D4  0B9CFFFF780000    or ebx,[edi+edi*8+0x78ff]
0000D4DB  0000              add [eax],al
0000D4DD  41                inc ecx
0000D4DE  0E                push cs
0000D4DF  088502420D05      or [ebp+0x50d4202],al
0000D4E5  0274C50C          add dh,[ebp+eax*8+0xc]
0000D4E9  0404              add al,0x4
0000D4EB  001C00            add [eax+eax],bl
0000D4EE  0000              add [eax],al
0000D4F0  0001              add [ecx],al
0000D4F2  0000              add [eax],al
0000D4F4  639CFFFFD10000    arpl [edi+edi*8+0xd1ff],bx
0000D4FB  0000              add [eax],al
0000D4FD  41                inc ecx
0000D4FE  0E                push cs
0000D4FF  088502420D05      or [ebp+0x50d4202],al
0000D505  02CD              add cl,ch
0000D507  C50C04            lds ecx,[esp+eax]
0000D50A  0400              add al,0x0
0000D50C  2000              and [eax],al
0000D50E  0000              add [eax],al
0000D510  2001              and [ecx],al
0000D512  0000              add [eax],al
0000D514  149D              adc al,0x9d
0000D516  FF                db 0xff
0000D517  FF                db 0xff
0000D518  3D05000000        cmp eax,0x5
0000D51D  41                inc ecx
0000D51E  0E                push cs
0000D51F  088502420D05      or [ebp+0x50d4202],al
0000D525  47                inc edi
0000D526  830303            add dword [ebx],byte +0x3
0000D529  3205C5C30C04      xor al,[dword 0x40cc3c5]
0000D52F  0420              add al,0x20
0000D531  0000              add [eax],al
0000D533  00440100          add [ecx+eax+0x0],al
0000D537  002DA2FFFFA9      add [dword 0xa9ffffa2],ch
0000D53D  0000              add [eax],al
0000D53F  0000              add [eax],al
0000D541  41                inc ecx
0000D542  0E                push cs
0000D543  088502420D05      or [ebp+0x50d4202],al
0000D549  44                inc esp
0000D54A  830302            add dword [ebx],byte +0x2
0000D54D  A1C5C30C04        mov eax,[0x40cc3c5]
0000D552  0400              add al,0x0
0000D554  2800              sub [eax],al
0000D556  0000              add [eax],al
0000D558  68010000B2        push dword 0xb2000001
0000D55D  A2FFFF6400        mov [0x64ffff],al
0000D562  0000              add [eax],al
0000D564  00410E            add [ecx+0xe],al
0000D567  088502420D05      or [ebp+0x50d4202],al
0000D56D  46                inc esi
0000D56E  8703              xchg eax,[ebx]
0000D570  860483            xchg al,[ebx+eax*4]
0000D573  050257C341        add eax,0x41c35702
0000D578  C641C741          mov byte [ecx-0x39],0x41
0000D57C  C50C04            lds ecx,[esp+eax]
0000D57F  041C              add al,0x1c
0000D581  0000              add [eax],al
0000D583  0094010000EAA2    add [ecx+eax-0x5d160000],dl
0000D58A  FF                db 0xff
0000D58B  FFA501000000      jmp [ebp+0x1]
0000D591  41                inc ecx
0000D592  0E                push cs
0000D593  088502420D05      or [ebp+0x50d4202],al
0000D599  03A101C50C04      add esp,[ecx+0x40cc501]
0000D59F  041C              add al,0x1c
0000D5A1  0000              add [eax],al
0000D5A3  00B40100006FA4    add [ecx+eax-0x5b910000],dh
0000D5AA  FF                db 0xff
0000D5AB  FFE6              jmp esi
0000D5AD  0200              add al,[eax]
0000D5AF  0000              add [eax],al
0000D5B1  41                inc ecx
0000D5B2  0E                push cs
0000D5B3  088502420D05      or [ebp+0x50d4202],al
0000D5B9  03E2              add esp,edx
0000D5BB  02C5              add al,ch
0000D5BD  0C04              or al,0x4
0000D5BF  0420              add al,0x20
0000D5C1  0000              add [eax],al
0000D5C3  00D4              add ah,dl
0000D5C5  0100              add [eax],eax
0000D5C7  0035A7FFFF99      add [dword 0x99ffffa7],dh
0000D5CD  0500000041        add eax,0x41000000
0000D5D2  0E                push cs
0000D5D3  088502420D05      or [ebp+0x50d4202],al
0000D5D9  44                inc esp
0000D5DA  830303            add dword [ebx],byte +0x3
0000D5DD  91                xchg eax,ecx
0000D5DE  05C5C30C04        add eax,0x40cc3c5
0000D5E3  0420              add al,0x20
0000D5E5  0000              add [eax],al
0000D5E7  00F8              add al,bh
0000D5E9  0100              add [eax],eax
0000D5EB  00AAACFFFFF8      add [edx-0x7000054],ch
0000D5F1  0100              add [eax],eax
0000D5F3  0000              add [eax],al
0000D5F5  41                inc ecx
0000D5F6  0E                push cs
0000D5F7  088502420D05      or [ebp+0x50d4202],al
0000D5FD  44                inc esp
0000D5FE  830303            add dword [ebx],byte +0x3
0000D601  F001C5            lock add ebp,eax
0000D604  C3                ret
0000D605  0C04              or al,0x4
0000D607  041C              add al,0x1c
0000D609  0000              add [eax],al
0000D60B  001C02            add [edx+eax],bl
0000D60E  0000              add [eax],al
0000D610  7EAE              jng 0xd5c0
0000D612  FF                db 0xff
0000D613  FF7500            push dword [ebp+0x0]
0000D616  0000              add [eax],al
0000D618  00410E            add [ecx+0xe],al
0000D61B  088502420D05      or [ebp+0x50d4202],al
0000D621  0271C5            add dh,[ecx-0x3b]
0000D624  0C04              or al,0x4
0000D626  0400              add al,0x0
0000D628  1C00              sbb al,0x0
0000D62A  0000              add [eax],al
0000D62C  3C02              cmp al,0x2
0000D62E  0000              add [eax],al
0000D630  D3AEFFFF2700      shr dword [esi+0x27ffff],cl
0000D636  0000              add [eax],al
0000D638  00410E            add [ecx+0xe],al
0000D63B  088502420D05      or [ebp+0x50d4202],al
0000D641  63C5              arpl bp,ax
0000D643  0C04              or al,0x4
0000D645  0400              add al,0x0
0000D647  001C00            add [eax+eax],bl
0000D64A  0000              add [eax],al
0000D64C  5C                pop esp
0000D64D  0200              add al,[eax]
0000D64F  00DA              add dl,bl
0000D651  AE                scasb
0000D652  FF                db 0xff
0000D653  FF                db 0xff
0000D654  3F                aas
0000D655  0000              add [eax],al
0000D657  0000              add [eax],al
0000D659  41                inc ecx
0000D65A  0E                push cs
0000D65B  088502420D05      or [ebp+0x50d4202],al
0000D661  7BC5              jpo 0xd628
0000D663  0C04              or al,0x4
0000D665  0400              add al,0x0
0000D667  001C00            add [eax+eax],bl
0000D66A  0000              add [eax],al
0000D66C  7C02              jl 0xd670
0000D66E  0000              add [eax],al
0000D670  F9                stc
0000D671  AE                scasb
0000D672  FF                db 0xff
0000D673  FF6200            jmp [edx+0x0]
0000D676  0000              add [eax],al
0000D678  00410E            add [ecx+0xe],al
0000D67B  088502420D05      or [ebp+0x50d4202],al
0000D681  025EC5            add bl,[esi-0x3b]
0000D684  0C04              or al,0x4
0000D686  0400              add al,0x0
0000D688  1C00              sbb al,0x0
0000D68A  0000              add [eax],al
0000D68C  9C                pushf
0000D68D  0200              add al,[eax]
0000D68F  003B              add [ebx],bh
0000D691  AF                scasd
0000D692  FF                db 0xff
0000D693  FF3402            push dword [edx+eax]
0000D696  0000              add [eax],al
0000D698  00410E            add [ecx+0xe],al
0000D69B  088502420D05      or [ebp+0x50d4202],al
0000D6A1  0330              add esi,[eax]
0000D6A3  02C5              add al,ch
0000D6A5  0C04              or al,0x4
0000D6A7  041C              add al,0x1c
0000D6A9  0000              add [eax],al
0000D6AB  00BC0200004FB1    add [edx+eax-0x4eb10000],bh
0000D6B2  FF                db 0xff
0000D6B3  FF1B              call far [ebx]
0000D6B5  0200              add al,[eax]
0000D6B7  0000              add [eax],al
0000D6B9  41                inc ecx
0000D6BA  0E                push cs
0000D6BB  088502420D05      or [ebp+0x50d4202],al
0000D6C1  0317              add edx,[edi]
0000D6C3  02C5              add al,ch
0000D6C5  0C04              or al,0x4
0000D6C7  041C              add al,0x1c
0000D6C9  0000              add [eax],al
0000D6CB  00DC              add ah,bl
0000D6CD  0200              add al,[eax]
0000D6CF  004AB3            add [edx-0x4d],cl
0000D6D2  FF                db 0xff
0000D6D3  FF8800000000      dec dword [eax+0x0]
0000D6D9  41                inc ecx
0000D6DA  0E                push cs
0000D6DB  088502420D05      or [ebp+0x50d4202],al
0000D6E1  0284C50C040400    add al,[ebp+eax*8+0x4040c]
0000D6E8  2000              and [eax],al
0000D6EA  0000              add [eax],al
0000D6EC  FC                cld
0000D6ED  0200              add al,[eax]
0000D6EF  00B2B3FFFF70      add [edx+0x70ffffb3],dh
0000D6F5  0100              add [eax],eax
0000D6F7  0000              add [eax],al
0000D6F9  41                inc ecx
0000D6FA  0E                push cs
0000D6FB  088502420D05      or [ebp+0x50d4202],al
0000D701  44                inc esp
0000D702  830303            add dword [ebx],byte +0x3
0000D705  6801C5C30C        push dword 0xcc3c501
0000D70A  0404              add al,0x4
0000D70C  1C00              sbb al,0x0
0000D70E  0000              add [eax],al
0000D710  2003              and [ebx],al
0000D712  0000              add [eax],al
0000D714  FE                db 0xfe
0000D715  B4FF              mov ah,0xff
0000D717  FF7601            push dword [esi+0x1]
0000D71A  0000              add [eax],al
0000D71C  00410E            add [ecx+0xe],al
0000D71F  088502420D05      or [ebp+0x50d4202],al
0000D725  037201            add esi,[edx+0x1]
0000D728  C50C04            lds ecx,[esp+eax]
0000D72B  0420              add al,0x20
0000D72D  0000              add [eax],al
0000D72F  004003            add [eax+0x3],al
0000D732  0000              add [eax],al
0000D734  54                push esp
0000D735  B6FF              mov dh,0xff
0000D737  FF6A03            jmp far [edx+0x3]
0000D73A  0000              add [eax],al
0000D73C  00410E            add [ecx+0xe],al
0000D73F  088502420D05      or [ebp+0x50d4202],al
0000D745  44                inc esp
0000D746  830303            add dword [ebx],byte +0x3
0000D749  6203              bound eax,[ebx]
0000D74B  C5                db 0xc5
0000D74C  C3                ret
0000D74D  0C04              or al,0x4
0000D74F  0420              add al,0x20
0000D751  0000              add [eax],al
0000D753  00640300          add [ebx+eax+0x0],ah
0000D757  009AB9FFFF1D      add [edx+0x1dffffb9],bl
0000D75D  0200              add al,[eax]
0000D75F  0000              add [eax],al
0000D761  41                inc ecx
0000D762  0E                push cs
0000D763  088502420D05      or [ebp+0x50d4202],al
0000D769  44                inc esp
0000D76A  830303            add dword [ebx],byte +0x3
0000D76D  1502C5C30C        adc eax,0xcc3c502
0000D772  0404              add al,0x4
0000D774  1C00              sbb al,0x0
0000D776  0000              add [eax],al
0000D778  8803              mov [ebx],al
0000D77A  0000              add [eax],al
0000D77C  93                xchg eax,ebx
0000D77D  BBFFFF3A00        mov ebx,0x3affff
0000D782  0000              add [eax],al
0000D784  00410E            add [ecx+0xe],al
0000D787  088502420D05      or [ebp+0x50d4202],al
0000D78D  76C5              jna 0xd754
0000D78F  0C04              or al,0x4
0000D791  0400              add al,0x0
0000D793  001C00            add [eax+eax],bl
0000D796  0000              add [eax],al
0000D798  A803              test al,0x3
0000D79A  0000              add [eax],al
0000D79C  AD                lodsd
0000D79D  BBFFFF8500        mov ebx,0x85ffff
0000D7A2  0000              add [eax],al
0000D7A4  00410E            add [ecx+0xe],al
0000D7A7  088502420D05      or [ebp+0x50d4202],al
0000D7AD  0281C50C0404      add al,[ecx+0x4040cc5]
0000D7B3  001C00            add [eax+eax],bl
0000D7B6  0000              add [eax],al
0000D7B8  C8030000          enter 0x3,0x0
0000D7BC  12BCFFFF360200    adc bh,[edi+edi*8+0x236ff]
0000D7C3  0000              add [eax],al
0000D7C5  41                inc ecx
0000D7C6  0E                push cs
0000D7C7  088502420D05      or [ebp+0x50d4202],al
0000D7CD  0332              add esi,[edx]
0000D7CF  02C5              add al,ch
0000D7D1  0C04              or al,0x4
0000D7D3  041C              add al,0x1c
0000D7D5  0000              add [eax],al
0000D7D7  00E8              add al,ch
0000D7D9  0300              add eax,[eax]
0000D7DB  0028              add [eax],ch
0000D7DD  BEFFFF5F01        mov esi,0x15fffff
0000D7E2  0000              add [eax],al
0000D7E4  00410E            add [ecx+0xe],al
0000D7E7  088502420D05      or [ebp+0x50d4202],al
0000D7ED  035B01            add ebx,[ebx+0x1]
0000D7F0  C50C04            lds ecx,[esp+eax]
0000D7F3  041C              add al,0x1c
0000D7F5  0000              add [eax],al
0000D7F7  0008              add [eax],cl
0000D7F9  0400              add al,0x0
0000D7FB  0067BF            add [edi-0x41],ah
0000D7FE  FF                db 0xff
0000D7FF  FF32              push dword [edx]
0000D801  0200              add al,[eax]
0000D803  0000              add [eax],al
0000D805  41                inc ecx
0000D806  0E                push cs
0000D807  088502420D05      or [ebp+0x50d4202],al
0000D80D  032E              add ebp,[esi]
0000D80F  02C5              add al,ch
0000D811  0C04              or al,0x4
0000D813  041C              add al,0x1c
0000D815  0000              add [eax],al
0000D817  0028              add [eax],ch
0000D819  0400              add al,0x0
0000D81B  0079C1            add [ecx-0x3f],bh
0000D81E  FF                db 0xff
0000D81F  FF1503000000      call [dword 0x3]
0000D825  41                inc ecx
0000D826  0E                push cs
0000D827  088502420D05      or [ebp+0x50d4202],al
0000D82D  0311              add edx,[ecx]
0000D82F  03C5              add eax,ebp
0000D831  0C04              or al,0x4
0000D833  0420              add al,0x20
0000D835  0000              add [eax],al
0000D837  004804            add [eax+0x4],cl
0000D83A  0000              add [eax],al
0000D83C  6E                outsb
0000D83D  C4                db 0xc4
0000D83E  FF                db 0xff
0000D83F  FF9802000000      call far [eax+0x2]
0000D845  41                inc ecx
0000D846  0E                push cs
0000D847  088502420D05      or [ebp+0x50d4202],al
0000D84D  44                inc esp
0000D84E  830303            add dword [ebx],byte +0x3
0000D851  90                nop
0000D852  02C5              add al,ch
0000D854  C3                ret
0000D855  0C04              or al,0x4
0000D857  041C              add al,0x1c
0000D859  0000              add [eax],al
0000D85B  006C0400          add [esp+eax+0x0],ch
0000D85F  00E2              add dl,ah
0000D861  C6                db 0xc6
0000D862  FF                db 0xff
0000D863  FF9600000000      call [esi+0x0]
0000D869  41                inc ecx
0000D86A  0E                push cs
0000D86B  088502420D05      or [ebp+0x50d4202],al
0000D871  0292C50C0404      add dl,[edx+0x4040cc5]
0000D877  001C00            add [eax+eax],bl
0000D87A  0000              add [eax],al
0000D87C  8C0400            mov [eax+eax],es
0000D87F  0058C7            add [eax-0x39],bl
0000D882  FF                db 0xff
0000D883  FF440000          inc dword [eax+eax+0x0]
0000D887  0000              add [eax],al
0000D889  41                inc ecx
0000D88A  0E                push cs
0000D88B  088502420D05      or [ebp+0x50d4202],al
0000D891  0240C5            add al,[eax-0x3b]
0000D894  0C04              or al,0x4
0000D896  0400              add al,0x0
0000D898  1C00              sbb al,0x0
0000D89A  0000              add [eax],al
0000D89C  AC                lodsb
0000D89D  0400              add al,0x0
0000D89F  007CC7FF          add [edi+eax*8-0x1],bh
0000D8A3  FF4D00            dec dword [ebp+0x0]
0000D8A6  0000              add [eax],al
0000D8A8  00410E            add [ecx+0xe],al
0000D8AB  088502420D05      or [ebp+0x50d4202],al
0000D8B1  0249C5            add cl,[ecx-0x3b]
0000D8B4  0C04              or al,0x4
0000D8B6  0400              add al,0x0
0000D8B8  1C00              sbb al,0x0
0000D8BA  0000              add [eax],al
0000D8BC  CC                int3
0000D8BD  0400              add al,0x0
0000D8BF  00A9C7FFFF2F      add [ecx+0x2fffffc7],ch
0000D8C5  0100              add [eax],eax
0000D8C7  0000              add [eax],al
0000D8C9  41                inc ecx
0000D8CA  0E                push cs
0000D8CB  088502420D05      or [ebp+0x50d4202],al
0000D8D1  032B              add ebp,[ebx]
0000D8D3  01C5              add ebp,eax
0000D8D5  0C04              or al,0x4
0000D8D7  041C              add al,0x1c
0000D8D9  0000              add [eax],al
0000D8DB  00EC              add ah,ch
0000D8DD  0400              add al,0x0
0000D8DF  00B8C8FFFFC4      add [eax-0x3b000038],bh
0000D8E5  0200              add al,[eax]
0000D8E7  0000              add [eax],al
0000D8E9  41                inc ecx
0000D8EA  0E                push cs
0000D8EB  088502420D05      or [ebp+0x50d4202],al
0000D8F1  03C0              add eax,eax
0000D8F3  02C5              add al,ch
0000D8F5  0C04              or al,0x4
0000D8F7  041C              add al,0x1c
0000D8F9  0000              add [eax],al
0000D8FB  000C0500005CCB    add [eax-0x34a40000],cl
0000D902  FF                db 0xff
0000D903  FF                db 0xff
0000D904  3F                aas
0000D905  0000              add [eax],al
0000D907  0000              add [eax],al
0000D909  41                inc ecx
0000D90A  0E                push cs
0000D90B  088502420D05      or [ebp+0x50d4202],al
0000D911  7BC5              jpo 0xd8d8
0000D913  0C04              or al,0x4
0000D915  0400              add al,0x0
0000D917  001C00            add [eax+eax],bl
0000D91A  0000              add [eax],al
0000D91C  2C05              sub al,0x5
0000D91E  0000              add [eax],al
0000D920  7BCB              jpo 0xd8ed
0000D922  FF                db 0xff
0000D923  FF4500            inc dword [ebp+0x0]
0000D926  0000              add [eax],al
0000D928  00410E            add [ecx+0xe],al
0000D92B  088502420D05      or [ebp+0x50d4202],al
0000D931  0241C5            add al,[ecx-0x3b]
0000D934  0C04              or al,0x4
0000D936  0400              add al,0x0
0000D938  1C00              sbb al,0x0
0000D93A  0000              add [eax],al
0000D93C  4C                dec esp
0000D93D  050000A0CB        add eax,0xcba00000
0000D942  FF                db 0xff
0000D943  FF                db 0xff
0000D944  DC00              fadd qword [eax]
0000D946  0000              add [eax],al
0000D948  00410E            add [ecx+0xe],al
0000D94B  088502420D05      or [ebp+0x50d4202],al
0000D951  02D8              add bl,al
0000D953  C50C04            lds ecx,[esp+eax]
0000D956  0400              add al,0x0
0000D958  1C00              sbb al,0x0
0000D95A  0000              add [eax],al
0000D95C  6C                insb
0000D95D  0500005CCC        add eax,0xcc5c0000
0000D962  FF                db 0xff
0000D963  FF8704000000      inc dword [edi+0x4]
0000D969  41                inc ecx
0000D96A  0E                push cs
0000D96B  088502420D05      or [ebp+0x50d4202],al
0000D971  038304C50C04      add eax,[ebx+0x40cc504]
0000D977  041C              add al,0x1c
0000D979  0000              add [eax],al
0000D97B  008C050000C3D0    add [ebp+eax-0x2f3d0000],cl
0000D982  FF                db 0xff
0000D983  FF8000000000      inc dword [eax+0x0]
0000D989  41                inc ecx
0000D98A  0E                push cs
0000D98B  088502420D05      or [ebp+0x50d4202],al
0000D991  027CC50C          add bh,[ebp+eax*8+0xc]
0000D995  0404              add al,0x4
0000D997  001C00            add [eax+eax],bl
0000D99A  0000              add [eax],al
0000D99C  AC                lodsb
0000D99D  05000023D1        add eax,0xd1230000
0000D9A2  FF                db 0xff
0000D9A3  FF4500            inc dword [ebp+0x0]
0000D9A6  0000              add [eax],al
0000D9A8  00410E            add [ecx+0xe],al
0000D9AB  088502420D05      or [ebp+0x50d4202],al
0000D9B1  0241C5            add al,[ecx-0x3b]
0000D9B4  0C04              or al,0x4
0000D9B6  0400              add al,0x0
0000D9B8  1C00              sbb al,0x0
0000D9BA  0000              add [eax],al
0000D9BC  CC                int3
0000D9BD  05000048D1        add eax,0xd1480000
0000D9C2  FF                db 0xff
0000D9C3  FF4500            inc dword [ebp+0x0]
0000D9C6  0000              add [eax],al
0000D9C8  00410E            add [ecx+0xe],al
0000D9CB  088502420D05      or [ebp+0x50d4202],al
0000D9D1  0241C5            add al,[ecx-0x3b]
0000D9D4  0C04              or al,0x4
0000D9D6  0400              add al,0x0
0000D9D8  1C00              sbb al,0x0
0000D9DA  0000              add [eax],al
0000D9DC  EC                in al,dx
0000D9DD  0500006DD1        add eax,0xd16d0000
0000D9E2  FF                db 0xff
0000D9E3  FF18              call far [eax]
0000D9E5  0000              add [eax],al
0000D9E7  0000              add [eax],al
0000D9E9  41                inc ecx
0000D9EA  0E                push cs
0000D9EB  088502420D05      or [ebp+0x50d4202],al
0000D9F1  54                push esp
0000D9F2  C50C04            lds ecx,[esp+eax]
0000D9F5  0400              add al,0x0
0000D9F7  001C00            add [eax+eax],bl
0000D9FA  0000              add [eax],al
0000D9FC  0C06              or al,0x6
0000D9FE  0000              add [eax],al
0000DA00  65D1FF            gs sar edi,1
0000DA03  FF02              inc dword [edx]
0000DA05  07                pop es
0000DA06  0000              add [eax],al
0000DA08  00410E            add [ecx+0xe],al
0000DA0B  088502420D05      or [ebp+0x50d4202],al
0000DA11  03FE              add edi,esi
0000DA13  06                push es
0000DA14  C50C04            lds ecx,[esp+eax]
0000DA17  041C              add al,0x1c
0000DA19  0000              add [eax],al
0000DA1B  002C06            add [esi+eax],ch
0000DA1E  0000              add [eax],al
0000DA20  47                inc edi
0000DA21  D8FF              fdivr st7
0000DA23  FF5100            call [ecx+0x0]
0000DA26  0000              add [eax],al
0000DA28  00410E            add [ecx+0xe],al
0000DA2B  088502420D05      or [ebp+0x50d4202],al
0000DA31  024DC5            add cl,[ebp-0x3b]
0000DA34  0C04              or al,0x4
0000DA36  0400              add al,0x0
0000DA38  1C00              sbb al,0x0
0000DA3A  0000              add [eax],al
0000DA3C  4C                dec esp
0000DA3D  06                push es
0000DA3E  0000              add [eax],al
0000DA40  78D8              js 0xda1a
0000DA42  FF                db 0xff
0000DA43  FF1B              call far [ebx]
0000DA45  0000              add [eax],al
0000DA47  0000              add [eax],al
0000DA49  41                inc ecx
0000DA4A  0E                push cs
0000DA4B  088502420D05      or [ebp+0x50d4202],al
0000DA51  57                push edi
0000DA52  C50C04            lds ecx,[esp+eax]
0000DA55  0400              add al,0x0
0000DA57  001C00            add [eax+eax],bl
0000DA5A  0000              add [eax],al
0000DA5C  6C                insb
0000DA5D  06                push es
0000DA5E  0000              add [eax],al
0000DA60  73D8              jnc 0xda3a
0000DA62  FF                db 0xff
0000DA63  FF0401            inc dword [ecx+eax]
0000DA66  0000              add [eax],al
0000DA68  00410E            add [ecx+0xe],al
0000DA6B  088502420D05      or [ebp+0x50d4202],al
0000DA71  0300              add eax,[eax]
0000DA73  01C5              add ebp,eax
0000DA75  0C04              or al,0x4
0000DA77  041C              add al,0x1c
0000DA79  0000              add [eax],al
0000DA7B  008C06000057D9    add [esi+eax-0x26a90000],cl
0000DA82  FF                db 0xff
0000DA83  FF6A01            jmp far [edx+0x1]
0000DA86  0000              add [eax],al
0000DA88  00410E            add [ecx+0xe],al
0000DA8B  088502420D05      or [ebp+0x50d4202],al
0000DA91  036601            add esp,[esi+0x1]
0000DA94  C50C04            lds ecx,[esp+eax]
0000DA97  0400              add al,0x0
0000DA99  0000              add [eax],al
0000DA9B  0000              add [eax],al
0000DA9D  0000              add [eax],al
0000DA9F  0000              add [eax],al
0000DAA1  0020              add [eax],ah
0000DAA3  0000              add [eax],al
0000DAA5  0000              add [eax],al
0000DAA7  0000              add [eax],al
0000DAA9  0000              add [eax],al
0000DAAB  0000              add [eax],al
0000DAAD  0000              add [eax],al
0000DAAF  0000              add [eax],al
0000DAB1  0000              add [eax],al
0000DAB3  0000              add [eax],al
0000DAB5  0000              add [eax],al
0000DAB7  0000              add [eax],al
0000DAB9  0000              add [eax],al
0000DABB  0000              add [eax],al
0000DABD  0000              add [eax],al
0000DABF  002F              add [edi],ch
0000DAC1  0000              add [eax],al
0000DAC3  0000              add [eax],al
0000DAC5  0000              add [eax],al
0000DAC7  0000              add [eax],al
0000DAC9  0000              add [eax],al
0000DACB  0000              add [eax],al
0000DACD  0000              add [eax],al
0000DACF  0000              add [eax],al
0000DAD1  0000              add [eax],al
0000DAD3  0000              add [eax],al
0000DAD5  0000              add [eax],al
0000DAD7  0000              add [eax],al
0000DAD9  0000              add [eax],al
0000DADB  0000              add [eax],al
0000DADD  0000              add [eax],al
0000DADF  0000              add [eax],al
0000DAE1  0000              add [eax],al
0000DAE3  0000              add [eax],al
0000DAE5  0000              add [eax],al
0000DAE7  0000              add [eax],al
0000DAE9  0000              add [eax],al
0000DAEB  0000              add [eax],al
0000DAED  0000              add [eax],al
0000DAEF  0000              add [eax],al
0000DAF1  0000              add [eax],al
0000DAF3  0000              add [eax],al
0000DAF5  0000              add [eax],al
0000DAF7  0000              add [eax],al
0000DAF9  0000              add [eax],al
0000DAFB  0000              add [eax],al
0000DAFD  0000              add [eax],al
0000DAFF  0000              add [eax],al
0000DB01  0000              add [eax],al
0000DB03  0000              add [eax],al
0000DB05  0000              add [eax],al
0000DB07  0000              add [eax],al
0000DB09  0000              add [eax],al
0000DB0B  0000              add [eax],al
0000DB0D  0000              add [eax],al
0000DB0F  0000              add [eax],al
0000DB11  0000              add [eax],al
0000DB13  0000              add [eax],al
0000DB15  0000              add [eax],al
0000DB17  0000              add [eax],al
0000DB19  0000              add [eax],al
0000DB1B  0000              add [eax],al
0000DB1D  0000              add [eax],al
0000DB1F  0000              add [eax],al
0000DB21  0000              add [eax],al
0000DB23  0000              add [eax],al
0000DB25  0000              add [eax],al
0000DB27  0000              add [eax],al
0000DB29  0000              add [eax],al
0000DB2B  0000              add [eax],al
0000DB2D  0000              add [eax],al
0000DB2F  0000              add [eax],al
0000DB31  0000              add [eax],al
0000DB33  0000              add [eax],al
0000DB35  0000              add [eax],al
0000DB37  0000              add [eax],al
0000DB39  0000              add [eax],al
0000DB3B  0000              add [eax],al
0000DB3D  0000              add [eax],al
0000DB3F  0000              add [eax],al
0000DB41  0000              add [eax],al
0000DB43  0000              add [eax],al
0000DB45  0000              add [eax],al
0000DB47  0000              add [eax],al
0000DB49  0000              add [eax],al
0000DB4B  0000              add [eax],al
0000DB4D  0000              add [eax],al
0000DB4F  0000              add [eax],al
0000DB51  0000              add [eax],al
0000DB53  0000              add [eax],al
0000DB55  0000              add [eax],al
0000DB57  0000              add [eax],al
0000DB59  0000              add [eax],al
0000DB5B  0000              add [eax],al
0000DB5D  0000              add [eax],al
0000DB5F  0000              add [eax],al
0000DB61  0000              add [eax],al
0000DB63  0000              add [eax],al
0000DB65  0000              add [eax],al
0000DB67  0000              add [eax],al
0000DB69  0000              add [eax],al
0000DB6B  0000              add [eax],al
0000DB6D  0000              add [eax],al
0000DB6F  0000              add [eax],al
0000DB71  0000              add [eax],al
0000DB73  0000              add [eax],al
0000DB75  0000              add [eax],al
0000DB77  0000              add [eax],al
0000DB79  0000              add [eax],al
0000DB7B  0000              add [eax],al
0000DB7D  0000              add [eax],al
0000DB7F  0000              add [eax],al
0000DB81  0000              add [eax],al
0000DB83  0000              add [eax],al
0000DB85  0000              add [eax],al
0000DB87  0000              add [eax],al
0000DB89  0000              add [eax],al
0000DB8B  0000              add [eax],al
0000DB8D  0000              add [eax],al
0000DB8F  0000              add [eax],al
0000DB91  0000              add [eax],al
0000DB93  0000              add [eax],al
0000DB95  0000              add [eax],al
0000DB97  0000              add [eax],al
0000DB99  0000              add [eax],al
0000DB9B  0000              add [eax],al
0000DB9D  0000              add [eax],al
0000DB9F  0000              add [eax],al
0000DBA1  0000              add [eax],al
0000DBA3  0000              add [eax],al
0000DBA5  0000              add [eax],al
0000DBA7  0000              add [eax],al
0000DBA9  0000              add [eax],al
0000DBAB  0000              add [eax],al
0000DBAD  0000              add [eax],al
0000DBAF  0000              add [eax],al
0000DBB1  0000              add [eax],al
0000DBB3  0000              add [eax],al
0000DBB5  0000              add [eax],al
0000DBB7  0000              add [eax],al
0000DBB9  0000              add [eax],al
0000DBBB  0000              add [eax],al
0000DBBD  0000              add [eax],al
0000DBBF  0000              add [eax],al
0000DBC1  000A              add [edx],cl
0000DBC3  0000              add [eax],al
0000DBC5  0000              add [eax],al
0000DBC7  0000              add [eax],al
0000DBC9  0000              add [eax],al
0000DBCB  0000              add [eax],al
0000DBCD  0000              add [eax],al
0000DBCF  0000              add [eax],al
0000DBD1  0000              add [eax],al
0000DBD3  0000              add [eax],al
0000DBD5  0000              add [eax],al
0000DBD7  0000              add [eax],al
0000DBD9  0000              add [eax],al
0000DBDB  0000              add [eax],al
0000DBDD  0000              add [eax],al
0000DBDF  0000              add [eax],al
0000DBE1  0000              add [eax],al
0000DBE3  00AA00000000      add [edx+0x0],ch
0000DBE9  AA                stosb
0000DBEA  0000              add [eax],al
0000DBEC  AA                stosb
0000DBED  AA                stosb
0000DBEE  0000              add [eax],al
0000DBF0  0000              add [eax],al
0000DBF2  AA                stosb
0000DBF3  00AA00AA0000      add [edx+0xaa00],ch
0000DBF9  55                push ebp
0000DBFA  AA                stosb
0000DBFB  00AAAAAA0055      add [edx+0x5500aaaa],ch
0000DC01  55                push ebp
0000DC02  55                push ebp
0000DC03  00FF              add bh,bh
0000DC05  55                push ebp
0000DC06  55                push ebp
0000DC07  0055FF            add [ebp-0x1],dl
0000DC0A  55                push ebp
0000DC0B  00FF              add bh,bh
0000DC0D  FF5500            call [ebp+0x0]
0000DC10  55                push ebp
0000DC11  55                push ebp
0000DC12  FF00              inc dword [eax]
0000DC14  FF55FF            call [ebp-0x1]
0000DC17  0055FF            add [ebp-0x1],dl
0000DC1A  FF00              inc dword [eax]
0000DC1C  FF                db 0xff
0000DC1D  FF                db 0xff
0000DC1E  FF00              inc dword [eax]
0000DC20  07                pop es
0000DC21  0000              add [eax],al
0000DC23  001F              add [edi],bl
0000DC25  0000              add [eax],al
0000DC27  00A000000064      add [eax+0x64000000],ah
0000DC2D  0000              add [eax],al
0000DC2F  0002              add [edx],al
0000DC31  0202              add al,[edx]
0000DC33  0202              add al,[edx]
0000DC35  0201              add al,[ecx]
0000DC37  0101              add [ecx],eax
0000DC39  0202              add al,[edx]
0000DC3B  0101              add [ecx],eax
0000DC3D  0102              add [edx],eax
0000DC3F  0201              add al,[ecx]
0000DC41  0101              add [ecx],eax
0000DC43  0202              add al,[edx]
0000DC45  0202              add al,[edx]
0000DC47  0202              add al,[edx]
0000DC49  0000              add [eax],al
0000DC4B  0000              add [eax],al
0000DC4D  0010              add [eax],dl
0000DC4F  00FF              add bh,bh
0000DC51  FF                db 0xff
0000DC52  FF                db 0xff
0000DC53  FF                db 0xff
0000DC54  FF                db 0xff
0000DC55  FF                db 0xff
0000DC56  FF                db 0xff
0000DC57  FF                db 0xff
0000DC58  FF                db 0xff
0000DC59  FF                db 0xff
0000DC5A  FF                db 0xff
0000DC5B  FF                db 0xff
0000DC5C  FF                db 0xff
0000DC5D  FF                db 0xff
0000DC5E  FF                db 0xff
0000DC5F  FF                db 0xff
