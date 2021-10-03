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
	
	data_size=$-gdt_null
data ends
 
;use16 не запрещает использовать в программе 32-х разрядные регистры
text segment use16
    assume cs:text,ds:data
 
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
 
	;загрузка в регистр GDTR информации о таблице глобальных
	;дескрипторов
    mov dword ptr pdescr+2, ebp ;занесение линейного адреса GDT
    mov word  ptr pdescr, gdt_size-1 ;занесение поле границы
    lgdt pdescr
	
	;подготовка возврата из защищенного режима в реальный
	mov ax, 40h
	mov es, ax
	mov word ptr es :[67h], offset return ;запись смещения возврата
	mov es:[69h], cs
	mov al, 0Fh ;выборка байта состояния отключения
	out 70h, al
	mov al, 0Ah ;установка режима восстановления
	out 71h, al
	;запрет аппаратных прерываний из-за другой обработки прерываний
	;в защищенном режиме
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

	db 0Eah
	dw offset go
	dw 16
	
go:  
	mov eax, CR0
	and eax, 0FFFFFFFEh
	mov CR0, eax 
	
	db 0Eah 			; Код команды far jmp
	dw offset return 	; Смещение 
	dw text 			; Сегмент.


return:
    mov ax, data 
    mov ds, ax
    mov ax, stk
    mov ss, ax
    mov sp, 256
    sti

	mov di, 1920 + 160 * 2
	mov bx, offset msg_real
	mov cx, msg_real_len
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