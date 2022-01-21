.586p

descr struc
    limit   dw 0
    base_l  dw 0
    base_m  db 0
    attr_1  db 0
    attr_2  db 0
    base_h  db 0
descr ends


idescr struc
    offs_l  dw 0
    sel     dw 0
    cntr    db 0
    attr    db 0
    offs_h  dw 0
idescr ends


stack_32 segment  para stack 'STACK'
    stack_start db  256h dup(?)
    stack_size = $-stack_start
stack_32 ends


data_32 segment para 'data'
    gdt_null  descr <0,0,0,0,0,0>
	;98h = 1001 1000
	;1000 - разрешено только исполнение (сегмент команд)
	
    gdt_code_16 descr <code_16_size-1,0,0,98h,0,0>
	;92h = 1001 0010
	;0010 - разрешены чтение и запись (сегмент данных)
	
    gdt_data_4gb descr <0FFFFh,0,0,92h,0CFh,0>
	;0CFh = 1100 1111
	
    gdt_code_32 descr <code_32_size-1,0,0,98h,40h,0>
	;40h = 0100 0000
	
    gdt_data_32 descr <data_size-1,0,0,92h,40h,0>
    gdt_stack_32 descr <stack_size-1,0,0,92h,40h,0>
    gdt_video descr <3999,8000h,0Bh,92h,0,0>

    gdt_size=$-gdt_null
    pdescr    df 0

    sel_code_16 = 8
    sel_data_4gb = 16
    sel_code_32 = 24
    sel_data_32 = 32
    sel_stack_32 = 40
    sel_video = 48

    idt label byte
	;8Fh = 1000 1111
	;1111 - шлюз ловушки (исключений)
    trap_0_12 idescr 13 dup (<0,sel_code_32,0,8Fh,0>)
	;исключение общей защиты
    trap_13 idescr <0,sel_code_32,0,8Fh,0>
    trap_14_31 idescr 18 dup (<0,sel_code_32,0,8Fh,0>)

	;8EF - шлюз прерываний
    int08 idescr <0,sel_code_32,0,8Eh,0> 
    int09 idescr <0,sel_code_32,0,8Eh,0>

    idt_size = $-idt 

    ipdescr df 0
    ipdescr_16 dw 3FFh, 0, 0

	;маска прерывания ведущего контроллера прерываний
    mask_master db 0        
	;маска прерывания ведомого контроллера прерываний
    mask_slave  db 0        

    ascii_map   db 0, 0, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 45, 61, 0, 0
    db 81, 87, 69, 82, 84, 89, 85, 73, 79, 80, 91, 93, 0, 0, 65, 83
    db 68, 70, 71, 72, 74, 75, 76, 59, 39, 96, 0, 92, 90, 88, 67
    db 86, 66, 78, 77, 44, 46, 47

    flag_enter_pr db 0
    cnt_time      db 0            

    syml_pos      dd 2 * 80 * 19 + 60

    interval=10

    mem_pos=3800
	mem_pos_protected_mode = 3900
    mem_value_pos=3818
    mb_pos=3820
    cursor_pos=80*23*2+50*2
    param=1Eh
	param_bs = 0Eh

    cursor_symb_on=48
    cursor_symb_off=49

    msg_real_mode      	db 27, '[29;44mIn real mode. ', 27, '[0m$'
    msg_wait 			db 27, '[29;44mInput any key to enter protected mode!', 27, '[0m$'
    msg_protected_mode  db 'In protected mode.'
	msg_protected_len = $-msg_protected_mode
	msg_real_again 		db 27, '[29;44mIn Real Mode again! ', 27, '[0m$'
	pm_mem_count 		db 'Memory: '
	
    data_size = $-gdt_null 
data_32 ends


code_32 segment para public 'code' use32
    assume cs:code_32, ds:data_32, ss:stack_32

pm_start:
    mov ax, sel_data_32
    mov ds, ax
    mov ax, sel_video
    mov es, ax
    mov ax, sel_stack_32
    mov ss, ax
    mov eax, stack_size
    mov esp, eax

    sti 
	
	mov di, mem_pos_protected_mode
    mov ah, param
    xor esi, esi
    xor ecx, ecx
    mov cx, msg_protected_len ; Длина строки
    print_memory_msg1:
        mov al, msg_protected_mode[esi]
        stosw
        inc esi
    loop print_memory_msg1

    mov di, mem_pos
    mov ah, param
    xor esi, esi
    xor ecx, ecx
    mov cx, 8 ; Длина строки
    print_memory_msg:
        mov al, pm_mem_count[esi]
        stosw
        inc esi
    loop print_memory_msg

    call count_memory

	;ожидание нажатия клавиши enter
	;(в это время работают аппартаные прерывания)
    proccess:
        test flag_enter_pr, 1
        jz  proccess

    cli 

    db  0EAh
    dd  offset return_rm
    dw  sel_code_16


    except_1 proc
        iret
    except_1 endp


    except_13 proc
		;снятие со стека кода ошибки EIP в точке исключения
		pop eax
        pop eax
		
        iret
    except_13 endp


    new_int08 proc uses eax
        mov edi, cursor_pos
        cmp cnt_time, interval
        je X
        cmp cnt_time, interval*2
        jne skip

        mov al, cursor_symb_off
        mov cnt_time, 0
        jmp pr
    X:
        mov al, cursor_symb_on
    pr:
        mov ah, param
        stosw

    skip:
        mov  al, cnt_time
        inc al
        mov cnt_time, al
		
		;разрешение обработки прерываний с меньшим приоритетом
		;(использование в аппаратных прерываниях с меньшим приоритетом)
		;(генерация сигнала конца прерывания EOI для ведущего контроллера)
        mov al, 20h
        out 20h, al

		;в защищенном режиме аппартаное прерывание смещает стек на 6 (iretd)
        iretd
    new_int08 endp

    new_int09 proc ;uses eax ebx edx
		push eax
		push ebx
		push edx
		;чтение кода поступившего символа
        in  al, 60h
        cmp al, 1Ch

        jne what_print       
        or flag_enter_pr, 1
        jmp allow_handle_keyboard
		
	what_print:
		cmp al, 39h
		ja allow_handle_keyboard
		mov ebx, offset ascii_map
		xlatb
		
		mov ebx, syml_pos
		
		cmp al, 0
		je bs_passed
		
		cmp al, 'a'
		jb print_value
		cmp al, 'z'
		ja print_value
		
    print_value:
        mov es:[ebx], ax
		add dword ptr syml_pos, 2
		jmp short allow_handle_keyboard
		
	bs_passed:
		mov al, ' '
		sub ebx, 2
		mov ah, param_bs
		mov es:[ebx], ax
		mov syml_pos, ebx
		jmp short allow_handle_keyboard

    allow_handle_keyboard:
        in  al, 61h
		;80h = 1000 0000 - установка старшего бита
        or  al, 80h 
        out 61h, al
		;кратковременная установка старшего бита в порту 61h
		;оповещает контроллер о том, что программа извлекла
		;код символа из порта 60h
		;это разрешает контроллеру вывод в порт очередного
		;символа
		;7Fh = 0111 1111 - сброс старшего бита
        and al, 7Fh 
        out 61h, al

        mov al, 20h 
        out 20h, al

		pop edx
		pop ebx
		pop eax
        iretd
    new_int09 endp


    count_memory proc uses ds eax ebx
        mov ax, sel_data_4gb
        mov ds, ax

        mov ebx,  100001h
        mov dl,   0AEh

		;FFEF FFFF оставшиеся байты до 4Гб
        mov ecx, 0FFEFFFFEh

        iterate_through_memory:
            mov dh, ds:[ebx] 

            mov ds:[ebx], dl        
            cmp ds:[ebx], dl        

            jnz print_memory_counter        

            mov ds:[ebx], dh 
            inc ebx
        loop iterate_through_memory

    print_memory_counter:
        mov eax, ebx 
        xor edx, edx

        mov ebx, 100000h
        div ebx

        ;вывод количества доступной памяти
        mov ebx, mem_value_pos
        call print_eax

		;вывод Мб
        mov ah, param
        mov ebx, mb_pos
        mov al, 'M'
        mov es:[ebx], ax

        mov ebx, mb_pos + 2
        mov al, 'b'
        mov es:[ebx], ax
        ret
    count_memory endp



    print_eax proc uses ecx ebx edx
        mov ecx, 10
		mov dh, param
		print_memory:
			xor edx, edx
			div ecx
			add edx, '0'
			mov dh, param
			mov es:[ebx], dx
			sub bx, 2
			cmp eax, 0
			jnz print_memory

        ret
    print_eax endp

    code_32_size = $-pm_start
code_32 ends


code_16 segment para public 'CODE' use16
assume cs:code_16, ds:data_32, ss: stack_32
new_line:
	xor dx, dx
    mov ah, 2
    mov dl, 13
    int 21h    
    mov dl, 10
    int 21h
    ret

start:
    mov ax, data_32
    mov ds, ax

    mov ah, 09h
    lea dx, msg_real_mode
    int 21h
	call new_line
	
    mov ah, 09h
    lea dx, msg_wait
    int 21h
    call new_line

    ;ожидание нажатия клавиши
	;10h - чтение буфера клавиатуры
    mov ah, 10h
    int 16h

    xor eax, eax

    mov ax, code_16
    shl eax, 4                        
    mov word ptr gdt_code_16.base_l, ax  
    shr eax, 16                       
    mov byte ptr gdt_code_16.base_m, al  
    mov byte ptr gdt_code_16.base_h, ah  

    mov ax, code_32
    shl eax, 4                        
    mov word ptr gdt_code_32.base_l, ax  
    shr eax, 16                       
    mov byte ptr gdt_code_32.base_m, al  
    mov byte ptr gdt_code_32.base_h, ah  

    mov ax, data_32
    shl eax, 4                        
    mov word ptr gdt_data_32.base_l, ax  
    shr eax, 16                       
    mov byte ptr gdt_data_32.base_m, al  
    mov byte ptr gdt_data_32.base_h, ah  

    mov ax, stack_32
    shl eax, 4                        
    mov word ptr gdt_stack_32.base_l, ax  
    shr eax, 16                       
    mov byte ptr gdt_stack_32.base_m, al  
    mov byte ptr gdt_stack_32.base_h, ah  

    mov ax, data_32
    shl eax, 4
    add eax, offset gdt_null

    mov dword ptr pdescr+2, eax
    mov word ptr  pdescr, gdt_size-1  
    lgdt fword ptr pdescr             

    lea eax, es:except_1
    mov trap_0_12.offs_l, ax 
    shr eax, 16             
    mov trap_0_12.offs_h, ax 

    lea eax, es:except_13
    mov trap_13.offs_l, ax 
    shr eax, 16             
    mov trap_13.offs_h, ax 

    lea eax, es:except_1
    mov trap_14_31.offs_l, ax 
    shr eax, 16             
    mov trap_14_31.offs_h, ax 


    lea eax, es:new_int08
    mov int08.offs_l, ax
    shr eax, 16
    mov int08.offs_h, ax

    lea eax, es:new_int09
    mov int09.offs_l, ax 
    shr eax, 16             
    mov int09.offs_h, ax 

    mov ax, data_32
    shl eax, 4
    add eax, offset idt

    mov  dword ptr ipdescr + 2, eax 
    mov  word ptr  ipdescr, idt_size-1 


    ; сохранение масок
    in  al, 21h                     
    mov mask_master, al             
    in  al, 0A1h                    
    mov mask_slave, al

    ; перепрограммирование ведущего контроллера
    mov al, 11h
    out 20h, al
	;32h - новый базовый вектор (до этого был 8)
    mov al, 32
    out 21h, al                     
    mov al, 4
    out 21h, al
    mov al, 1
    out 21h, al

    ; маска для ведущего контроллера
	;0FCh = 1111 1100 - разрешенение только IRQ0 и IRQ1
    mov al, 0FCh
    out 21h, al

    ; маска для ведомого контроллера (запрещаем прерывания)
	; 0FFh = 1111 1111
	; от этого контрллера прерывания заведомо поступать не будут -
	; для надежности маскируем эти прерывания
    mov al, 0FFh
    out 0A1h, al

    lidt fword ptr ipdescr                                    

    ; открытие линии А20
    in  al, 92h
    or  al, 2
    out 92h, al

    cli

    mov eax, cr0
    or eax, 1     
    mov cr0, eax

    db  66h 
    db  0EAh
    dd  offset pm_start
    dw  sel_code_32


return_rm:
    mov eax, cr0
    and al, 0FEh                
    mov cr0, eax

    db  0EAh    
    dw  offset go ;смещение
    dw  code_16 ;селектор сегмента команд

go:
    mov ax, data_32   
    mov ds, ax
    mov ax, code_32
    mov es, ax
    mov ax, stack_32   
    mov ss, ax
    mov ax, stack_size
    mov sp, ax

	;возврат базового вектора ведущего контроллера прерываний
	;(аппаратным прерываниям нужно назначить другие векторы)
    mov al, 11h
    out 20h, al
    mov al, 8 ;теперь базовый вектор 8
    out 21h, al
    mov al, 4
    out 21h, al
    mov al, 1
    out 21h, al

	;восстановление исходных масок прерываний обоих контроллеров
    mov al, mask_master
    out 21h, al
    mov al, mask_slave
    out 0A1h, al

    lidt    fword ptr ipdescr_16

    ;A20
    in  al, 70h 
    and al, 7Fh
    out 70h, al

    sti     

    ;очистка экрана
    mov ax, 3
    int 10h

    mov ah, 09h
    lea dx, msg_real_again
    int 21h
    xor dx, dx
    mov ah, 2
    mov dl, 13
    int 21h
    mov dl, 10
    int 21h

    mov ax, 4C00h
    int 21h

    code_16_size = $-start  
code_16 ends

end start