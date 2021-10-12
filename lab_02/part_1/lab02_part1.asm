;разрешение транслятору обрабатывать привилегированные команды
;32-х разрядных процессоров
.586P

;память под структуру здесь не выделяется
descr struct
	limit 	dw 0
	base_l 	dw 0
	base_m 	db 0
	attr_1 	db 0
	attr_2 	db 0 
	base_h 	db 0
descr ends

;use16 - в данном сегменте используются 16-битовые адреса
data segment use16 
	gdt_null descr 		<0, 0, 0, 0, 0, 0>
	
	;92h - сегмент данных с разрешением записи и чтения
	;98h- запрещено обращение с целью записи и чтения
	gdt_data descr 		<data_size-1, 0, 0, 92h, 0, 0>
	gdt_code descr 		<code_size-1, 0, 0, 98h, 0, 0>
	gdt_stack descr 	<255, 0, 0, 92h, 0, 0>
	
	;описание страницы 0 видеопамяти
	;размещается в перовм мегабайте адресного пространства
    gdt_screen descr 	<3999, 8000h, 0Bh, 92h, 0, 0>
	gdt_data_4gb descr <0FFFFh,0,0,92h,0CFh>
    gdt_size=$-gdt_null

	;псевдодескриптор: содержит
	;линейный базовый адрес таблицы(2..5 байты)
	;граница(0..1 байты)
	pdescr  df 0
	attr    db 1Eh

	msg_real     db 'Real mode'
	msg_real_len=$-msg_real

	msg_protected db 'Protected mode'
	msg_len_protected=$-msg_protected
	
	msg_real_again     db 'Real mode again!'
	msg_real_len_again=$-msg_real_again
	string db 'Memory available: ',15 dup (0),' Mb  '
	memory_value_position = 14 + 16
	mb_pos = 30 + 2
	param = 1Eh
	
	data_size=$-gdt_null
data ends
 
print	macro	msg,msg_len,row,col,color
	local	screen
	mov	di,row*160 + col*2
	mov	bx,offset msg
	mov	cx,msg_len
	mov	ah,color
screen:	mov	al,byte ptr [bx]
	inc	bx
	stosw
	loop	screen
endm
 
;use16 не запрещает использовать в программе 32-х разрядные регистры
text segment use16
    assume cs:text,ds:data
 
dword_to_str proc
				push    si
                push    di
                mov     bx,10
                xor     cx,cx
				mov     ax,dx
next_digit:
				xor dx, dx
                div     bx
                push dx
                inc     cx
                cmp ax, 0
                jne    next_digit
store_digit_loop:
                pop     ax
                add     al,'0'
                mov [di], al
				inc di
                loop    store_digit_loop
                mov     byte ptr es:[di],'$'
                pop     di
                pop     si
                ret
dword_to_str       endp

main proc
	mov ax, data
	mov ds, ax
	
	mov ax, 0B800h
	mov es, ax

	mov di, 1920
	mov bx, offset msg_real
	mov cx, msg_real_len
	mov ah, attr ; text param.
print_real_mode:
    mov al, byte ptr [bx]
    inc bx
    stosw
	loop print_real_mode

	
    xor eax, eax
    mov ax, data
    mov ds, ax

	;вычисление линейного адреса сегмента данных
	;в eax находится сегментный адрес сегмента данных
	;сдвиг этого адреса на 4 бита, образуя линейный 32-битовый адрес 
    shl eax, 4
	;сохраняем адрес в ebp, так как будем использовать его дальше
    mov ebp, eax
	
	
    mov bx, offset gdt_data
    mov [bx].base_l, ax
    shr eax, 16
	;загрузка старшей половины линейного адреса
    mov [bx].base_m, al
    xor eax, eax

	;вычисление линейного адреса сегмента кода
    mov ax, cs
    shl eax, 4
    mov bx, offset gdt_code
    mov [bx].base_l, ax
    shr eax, 16
    mov [bx].base_m, al

	;вычисление линейного адреса сегмента стека
    xor eax, eax
    mov ax, ss
    shl eax, 4
    mov bx, offset gdt_stack
    mov [bx].base_l, ax
    shr eax, 16
    mov [bx].base_m, al
	
	;вычисление линейного адреса gdt_4gb
	xor eax, eax
    mov ax, fs
    shl eax, 4
    mov bx, offset gdt_data_4gb
    mov [bx].base_l, ax
    shr eax, 16
    mov [bx].base_m, al
 
	;загрузка в регистр GDTR информации о таблице глобальных
	;дескрипторов
    mov dword ptr pdescr+2, ebp ;занесение линейного адреса GDT
    mov word  ptr pdescr, gdt_size-1 ;занесение поле границы
    lgdt pdescr
	
	push ax
    in  al, 92h
    or  al, 2
    out 92h, al 
	pop ax
	cli
	
	;подготовка возврата из защищенного режима в реальный
	;mov ax, 40h
	;mov es, ax
	;mov word ptr es :[67h], offset return ;запись смещения возврата
	;mov es:[69h], cs
	;mov al, 0Fh ;выборка байта состояния отключения
	;out 70h, al
	;mov al, 0Ah ;установка режима восстановления
	;out 71h, al
	
	
	;запрет аппаратных прерываний из-за другой обработки прерываний
	;в защищенном режиме
	
	
	mov	al,80h				; (53) Запрет NMI
	out	70h,al
	cli
	;перевод процессора в защищенный режим
    mov eax, CR0
    or eax, 1
    mov CR0, eax
 
	;загрузка в CS:IP селектор:смещение точки continue
    db 0EAh ;код команды far jmp
    dw offset continue
    dw 16 ;селектор сегмента команд

continue:
	;делаем данные адресуемыми
    mov ax, 8
    mov ds, ax
	
	;делаем адресуемым стек
    mov ax, 24
    mov ss, ax
 
	;делаем адресуемым видеобуфер
    mov ax, 32
    mov es, ax
	
	mov ax, 40
	mov gs, ax
	;sti
	;вывод на экран тестовой строки символов
	mov di, 1920 + 160 * 1 ;начальная позиция на экране
	mov bx, offset msg_protected
	mov cx, msg_len_protected ;число выводимых символов
	mov ah, attr
print_protected_mode:
    mov al, byte ptr [bx]
    inc bx
    stosw
	loop print_protected_mode
	
	;необходимо указать сегментам границу FFFFh
	;иначе теневые регистры будут иметь дескрипторы
	;защищенного режима и при обращении к сегментам в реальном
	;будет сбой
	mov gdt_data.limit, 0FFFFh
	mov gdt_code.limit, 0FFFFh
	mov gdt_stack.limit, 0FFFFh
	mov gdt_screen.limit, 0FFFFh
	mov gdt_data_4gb.limit, 0FFFFh

	;чтобы теневые регистры приняли новые значения,
	;надо перезаписать сегментные регистры, тогда
	;процессор сам перепишет теневые регистры (самим
	;обращаться к ним нельзя)
	push ds
	pop ds

	push ss
	pop ss

	push es
	pop es
	
	push fs
	pop fs
	; определение доступной памяти
	mov ecx,1048576
	mov al,0AAh

count:
	mov dl,byte ptr fs:[ecx]
	mov byte ptr fs:[ecx],al
	mov al,byte ptr fs:[ecx]
	mov byte ptr fs:[ecx],dl
	cmp al,0AAh
	jnz exit
	add ecx, 1048576
	jmp count
exit:
	sub ecx, 1048576
	mov edx, ecx
	ror edx, 16 
	mov cl, 4
	shr edx, cl
	lea di,string
	add di,18
	call dword_to_str
	print	string,38,13,23,1Eh

	db 0EAh
	dw offset go
	dw 16
	
go:  
	; закрытие линии A20 (если не закроем, то сможем адресовать еще 64кб памяти (HMA, см. сем))
    in  al, 70h 
    and al, 7Fh
    out 70h, al
	mov eax, cr0
    and al, 0FFFFFFFEh                
    mov cr0, eax
	
	db 0EAh 			; Код команды far jmp
	dw offset return 	; Смещение 
	dw text 			; Сегмент.


return:
    mov ax, data 
    mov ds, ax
    mov ax, stk
    mov ss, ax
    mov sp, 256
	
    sti
	mov	al,0				; (99) Сброс бита 7 в порте CMOS —
	out	70h,al

	mov di, 1920 + 160 * 5
	mov bx, offset msg_real_again
	mov cx, msg_real_len_again
	mov ah, attr
print_real_mode_again:
    mov al, byte ptr [bx]
    inc bx
    stosw
	loop print_real_mode_again

    ; Завершение программы.
    mov ax, 4C00h
    int 21h
main endp

code_size = $-main
text ends
 
stk segment stack use16
    db 256 dup('^')
stk ends

end main