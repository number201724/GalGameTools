.486
.model flat,stdcall
option casemap:none

include windows.inc

include kernel32.inc
include user32.inc

includelib kernel32.lib
includelib user32.lib

include msvcrt.inc
includelib msvcrt.lib

.data
szMemcpy db 'memcpy',0
szNtdll db 'ntdll',0
szDrawName db '@[DrawName'
szPageBreak db '@[PageBreak]'
szMsgTest db 'Test',0
.data?
memcpy dd ?
text_buf db ?
.code

;-----------------------------------------------------------
MsgShow:
pushad
push 0
push offset szMsgTest
push offset szMsgTest
push 0
call MessageBoxA
popad
ret

;-----------------------------------------------------------
RecordText:
;edi = strings
pushad
;比较是否为绘制文本
push sizeof szDrawName
push offset szDrawName
push edi
call crt_memcmp
add esp,12
;内存是否相同
test al,al
jnz @F
mov text_buf,1
@@:

push sizeof szPageBreak
push offset szPageBreak
push edi
call crt_memcmp
add esp,12
test al,al
jnz @F
xor al,al
mov text_buf,al
@@:
popad
jmp memcpy
;-----------------------------------------------------------
CheckText1: ;check text proc
;如果是文本修正标记,则执行文本格式化
cmp al,'@'
je CheckText1_2
cmp al,';'
je CheckText1_2
cmp al,'!'
je CheckText1_2
cmp al,'<'
je CheckText1_2
cmp text_buf,1
je is_text_1
;原始汇编代码填充
db 3Ch,81h,72h,04h,3Ch,9Fh,76h,10h,3Ch,0E0h,72h,04h,3Ch,0FCh,76h,08h,3Ch,3Ch
nop
nop
nop
nop
jmp CheckText1_1
is_text_1:   ;文本格式化标记
push 00504D6Ch
ret
CheckText1_1:
jnz CheckText1_2
jmp is_text_1
CheckText1_2:
push 00505035h
ret
;-----------------------------------------------------------

;-----------------------------------------------------------
CheckText2: ;check text proc
;如果是文本修正标记,则执行文本格式化
cmp al,'@'
je CheckText2_2
cmp al,';'
je CheckText2_2
cmp al,'!'
je CheckText2_2
cmp al,'<'
je CheckText2_2
cmp text_buf,1
je is_text_2
;原始汇编代码填充
db 3Ch,81h,72h,04h,3Ch,9Fh,76h,10h,3Ch,0E0h,72h,04h,3Ch,0FCh,76h,08h,3Ch,3Ch
nop
nop
nop
nop
jmp CheckText2_1
is_text_2:   ;文本格式化标记
push 00504F63h
ret
CheckText2_1:
jnz CheckText2_2
jmp is_text_2
CheckText2_2:
push 00504FE9h
ret
;-----------------------------------------------------------


Init proc
Init endp


DllEntry proc uses esi edi ebx,hInst:HINSTANCE, nReason:DWORD, lpReserved:DWORD
	cmp nReason,DLL_PROCESS_ATTACH
	jz @F
		push 0
		call GetModuleHandleA
		push eax ; [esp]=Module Handle
		xor al,al
		mov text_buf,al
		
		mov edi,00504B2Fh  ;CALL MEMCPY
		lea esi,RecordText
		sub esi,edi
		sub esi,5
		;[esp] = 0
		;[esp+4] = Module Handle
		push 0
		push esp
		push PAGE_EXECUTE_READWRITE
		push 5
		push edi
		call VirtualProtect
		add esp,4
		mov [edi+1],esi
		
		
		push offset szNtdll
		call GetModuleHandleA
		push offset szMemcpy
		push eax
		call GetProcAddress
		mov memcpy,eax
		
		
		lea edi,CheckText1
		mov esi,00504D54h
		sub edi,esi
		sub edi,5
		
		push 0
		push esp
		push PAGE_EXECUTE_READWRITE
		push 5
		push esi
		call VirtualProtect
		mov byte ptr[esi],0E9h
		mov dword ptr[esi+1],edi
		;add esp,8
		
		lea edi,CheckText2
		mov esi,00504F47h
		sub edi,esi
		sub edi,5
		
		push 0
		push esp
		push PAGE_EXECUTE_READWRITE
		push 5
		push esi
		call VirtualProtect
		mov byte ptr[esi],0E9h
		mov dword ptr[esi+1],edi
		
		add esp,0Ch
	@@:
	mov eax, TRUE
	ret
DllEntry Endp
End DllEntry
