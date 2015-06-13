          ; Barcode Blinker PIC program source
          ; V 1.0
          ; Download programs/data to HP41 calculator without HPIL
          ; Processor 16C84 with 4MHz Xtal
          ; 2001 A. R. Duell, and placed under the GPL 

          ; Pick one of the next 2 lines depending on the assembler used
          ; device pic16c84
          list c=80, n=60, p=16c84

          ; destination mode bit
W         equ 0
F         equ 1

          ; Register definitions
ind_data  equ 0 ; Indirect data
status    equ 3 ; Status register
ind_adr   equ 4 ; Indirect address
porta     equ 5 ; Port A (control lines and barcode output)
portb     equ 6 ; Port B (parallel input data)
trisa     equ 5 ; Port A tristate register (in bank 1)
incnt     equ 0xC ; Inner loop delay count
outcnt    equ 0xD ; Outer loop delay count
rdcnt     equ 0xE ; Count of characters to read from centronics port
wrcnt     equ 0xF ; Count of characters to output in barcode
bitcnt    equ 0x10 ; Bit counter for barcode character
first_dat equ 0x20 ; first location in data buffer

          ; Bit definitions 
          ; Status Register
carry     equ 0 ; Carry Flag
rp        equ 5 ; Register page select

          ; port A
barcode   equ 2 ; Barcode output
DAC       equ 3 ; Data Acknowledge line for parallel input
DAV       equ 4 ; Data Valid line for parallel input

          ; Outer loop delay counts 
narrow    equ 4 ; Narrow bar or space
wide      equ 8 ; Wide bar

          org 0 ; Reset enters here
Reset     bsf status,rp ; Select tristate register
          movlw 0x10 ; DAV input, rest outputs
          movwf trisa ; Set up port A
          bcf status, rp ; Select ports
          ; Initialise output ports
          bcf porta,barcode ; Turn off output LED
          bsf porta,DAC ; Set acknowlege output
          call senddac ; Send DAC pulse to clear busy flip-flop
          ; Main loop - read in barcode row and output it
mainlp    call centin ; Read length byte
          andlw 0x0f ; High nybble is irrelevant
          addlw 1 ; Low nybble + 1 is length
          movwf rdcnt ; Store as count of characters to read
          movwf wrcnt ; and as count of characters to write
          movlw first_dat ; Pointer to first location in buffer
          movwf ind_adr ; Set indirect address to point to start of buffer
rdlp      call centin ; Read next character from centronics port
          movwf ind_data ; Store in buffer
          incf ind_adr,F ; Increment address pointer
          decfsz rdcnt,F ; Decrement character counter
          goto rdlp ; Round again if more characters to read
          movlw first_dat ; Reset pointer to start of buffer
          movwf ind_adr
          call longdly ; Pause at start of line
          bsf porta,barcode ; Turn on output LED
          call longdly ; Wait for wand to settle
          call narbar ; Output row header (2 narrow bars)
          call narbar 
wrlp      movlw 8 ; 8 bits/character
          movwf bitcnt ; Initialize bit counter
bitlp     rlf ind_data,F ; Shift next bit into carry flag
          call bar ; Output the right type of bar
          decfsz bitcnt,F ; Decrement bit counter
          goto bitlp ; Round again if more bits to send
          incf ind_adr,F ; Point to next character in buffer
          decfsz wrcnt,F ; Decrement output counter
          goto wrlp ; Round again if more characters to send
          bsf status,carry ; Send row trailler (wide bar, narrow bar)
          call bar
          call narbar
          call longdly ; end of row pause
          bcf porta,barcode ; turn off output LED
          goto mainlp ; Round again for next row

          ; Read a character from the Centronics (Parallel) input
          ; Wait for a character to be received and exit with the 
          ; character in W
centin    nop
          nop
          nop
          btfss porta,DAV ; Is a character available ?
          goto centin ; No, loop round again
          nop
          nop
          nop
          movf portb,W ; Yes, read in character
senddac   nop
          nop
          nop
          bcf porta,DAC ; Start acknowledge pulse (and clear busy FF)
          nop ; A couple of microseconds delay
          nop
          bsf porta,DAC ; End acknowledge pulse
          nop
          nop
          nop
          return ; and exit

          ; Generate a bar
          ; On entry, carry flag is 1 for wide bar, 0 for narrow bar
bar       movlw wide ; Get wide bar delay count
          btfss status,carry ; If not carry (so narrow bar)
narbar    movlw narrow ; Get narrow bar delay count
          bcf porta,barcode ; Turn off output LED
          call delay ; Wait for bar width
          bsf porta,barcode ; Turn LED on again
          movlw narrow ; Interbar delay 
          goto delay; Wait for interbar delay time and exit

          ; A long delay time for the start and end of barcode rows
longdly   clrw ; As many outer loops as possible
          call delay; Wait a bit
          clrw 
          call delay
          clrw
          call delay
          clrw ; and fall into delay

          ; Wait for a time set by W
delay     movwf outcnt ; Store outer loop count
outlp     clrf  incnt ; Clear inner loop count (so it runs 256 times)
inlp      decfsz incnt,F ; Decrement inner loop count
          goto inlp ; Round inner loop again
          decfsz outcnt,F ; Decrement outer loop count 
          goto outlp ; Round outer loop again
          return ; At end of outer loop, exit 

          end

