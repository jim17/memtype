;
; Copyright � Thomas Plos, 2011.
; e-mail: <thomas.plos@iaik.tugraz.at>.
;
; This program is a free software: you can redistribute it 
; and/or modify it under the terms of the GNU General Public 
; License as published by the Free Software Foundation.
;
; It is distributed without any warranty of correctness nor
; fintess for any particular purpose. See the GNU General 
; Public License for more details.
; 
; <http://www.gnu.org/licenses/>.
;
; Description: NOEKEON encryption/descryption.
; Version 1 - June 2011.
;   
;   ------------------------------------------------------------------
;  |    User interface:                                               |
;  |------------------------------------------------------------------|
;  |(1) Load plaintext/key in SRAM at the address STATE_START         |
;  |    pointed by X register (r27:r26):                              |
;  |    -> The 16 first bytes are the plaintext/ciphertext,           |
;  |    -> The 16 last bytes are the key.                             |
;  |    (NOTE: lo8(STATE_START) must be smaller 255-32)               |
;  |------------------------------------------------------------------|
;  |(2) Reset the round counter and indicated whether encryption or   |
;  |    decryption is used:                                           |
;  |    -> encrpytion: 0x00                                           |
;  |    -> decryption: 0x80                                           |
;  |------------------------------------------------------------------|
;  |(3) Call the encrypt/decryption routine.                          |
;  |------------------------------------------------------------------|
;  |(4) After the NOEKEON call, the plaintext/cihpertext is           |
;  |    overloaded by its corresponding ciphertext/plaintext in SRAM. |
;   ------------------------------------------------------------------

;
; declaration of algorithm constants
;
.extern cipher
#define IS_DECRYPTION 7
#define STATE_START cipher
#define ROUND_CONST_MIN 0x80
#define ROUND_CONST_MAX 0xD4

;
; registers declarations
;
#define round_const R18
#define temp_1 R19
#define temp_2 R20
#define round_counter R21
#define state_0 R22
#define state_1 R23
#define state_2 R24
#define state_3 R25

#define XL R26
#define XH R27

;
; loads data from ram
;
ram_2_reg_word0:
  ld state_0, x+;
  ld state_1, x+;
ram_2_reg_word0_half:
  ld state_2, x+;
  ld state_3, x+;
  ret;

;
; stores data to ram
;
reg_2_ram_word0:
  st x+, state_0;
  st x+, state_1;
  st x+, state_2;
  st x+, state_3;
  ret;

.global noekeon_encrypt
noekeon_encrypt:
    ldi XH, hi8(STATE_START);
    ldi XL, lo8(STATE_START);
    rcall encrypt
    ret
;
; performs a noekeon encryption
;
encrypt:

    ; set round counter to zero and indicate encryption
    ldi round_counter, (0<<IS_DECRYPTION)
    ldi round_const, ROUND_CONST_MIN;
  ; executes round function 16x
  noekeon_round_function_encrypt_loop:
    rcall noekeon_round_function_encrypt;

    ; update round constant
    ldi temp_1, 0x1B;
    lsl round_const;
    brcc noekeon_round_function_encrypt_skip_xor;
    eor round_const, temp_1;
    noekeon_round_function_encrypt_skip_xor:
      inc round_counter;
      sbrs round_counter, 4;
      rjmp noekeon_round_function_encrypt_loop;

  rcall noekeon_round_const_add;
  rcall noekeon_theta_encrypt;
  ret;

.global noekeon_decrypt
noekeon_decrypt:
    ldi XH, hi8(STATE_START);
    ldi XL, lo8(STATE_START);
    rcall decrypt
    ret
;
; performs a noekeon decryption
;
.global decrypt
decrypt:

  ; set round counter to zero and indicate decryption
  ldi round_counter, (1<<IS_DECRYPTION);
  ldi round_const, ROUND_CONST_MAX;
  ; executes round function 16x
  noekeon_round_function_decrypt_loop:
    rcall noekeon_round_function_decrypt;

	; update round constant
    ldi temp_1, 0x8D;
    lsr round_const;
    brcc noekeon_round_function_decrypt_skip_xor;
    eor round_const, temp_1;
    noekeon_round_function_decrypt_skip_xor:
      inc round_counter;
      sbrs round_counter, 4;
      rjmp noekeon_round_function_decrypt_loop;

  rcall noekeon_theta_decrypt;
  rcall noekeon_round_const_add;
  ret;


;
; cyclic shift left of a 4-byte word
; INPUT(s):
;      XL,XH ... pointer to word that should be shifted
;      temp_1... number of left shifts
noekeon_shift_left:
  rcall ram_2_reg_word0;
  mov temp_2, state_3;
  noekeon_shift_left_loop:
    lsl temp_2;
	rol state_0;
	rol state_1;
	rol state_2;
	rol state_3;
	dec temp_1;
	brpl noekeon_shift_left_loop;
  rjmp noekeon_shift_right_left_end;

;
; cyclic shift right of a 4-byte word
; INPUT(s):
;      XL,XH ... pointer to word that should be shifted
;      temp_1... number of right shifts
noekeon_shift_right:
  rcall ram_2_reg_word0;
  mov temp_2, state_0;
  noekeon_shift_right_loop:
    lsr temp_2;
	ror state_3;
	ror state_2;
	ror state_1;
	ror state_0;
	dec temp_1;
	brpl noekeon_shift_right_loop;
    noekeon_shift_right_left_end:
    subi XL, 4
    rcall reg_2_ram_word0;
  ret;

;
; xor with round constant
;
noekeon_round_const_add:
  ldi XL, lo8(STATE_START);
  ld state_0, x;
  eor state_0, round_const;
  st x, state_0;
ret;


;
; round function for encryption
;
noekeon_round_function_encrypt:
  rcall noekeon_round_const_add;
  rcall noekeon_theta_encrypt;
  rjmp noekeon_shift_pi1;

;
; round function for decryption
;
noekeon_round_function_decrypt:
  rcall noekeon_theta_decrypt;
  rcall noekeon_round_const_add;

  ;
  ; performs cyclic shift operation 'pi1' on state
  ;
  noekeon_shift_pi1:
    ; word1 <<<= 1;
    ; move pointer to word1
    ldi XL, lo8(STATE_START+0x04);
    ldi temp_1, 0;
    rcall noekeon_shift_left;

    ; word2 <<<= 5;
    ; move pointer to word2
    ldi temp_1, 0x04;
    rcall noekeon_shift_left;

    ; word3 <<<= 2;
    ; move pointer to word3
    ldi temp_1, 1;
    rcall noekeon_shift_left;

  ;
  ; performs involutive non-linear mapping 'gamma' on state
  ;
  noekeon_gamma:
    ; word1 ^= ~(word3 | word2)
    ; word0 ^= word2 & word1)
    ; move pointer to word1
    ldi XL, lo8(STATE_START+0x04);
    rcall noekeon_gamma_byte;
    rcall noekeon_gamma_byte;
    rcall noekeon_gamma_byte;
    rcall noekeon_gamma_byte;

  ;
  ; performs cyclic shift operation 'pi2' on state
  ;
  noekeon_shift_pi2:
    ; word1 >>>= 1;
    ; move pointer to word1
    ldi XL, lo8(STATE_START+0x04);
    ldi temp_1, 0;
    rcall noekeon_shift_right;

    ; word2 >>>= 5;
    ; move pointer to word2
    ldi temp_1, 0x04;
    rcall noekeon_shift_right;

    ; word3 >>>= 2;
    ; move pointer to word3
    ldi temp_1, 1;
    rcall noekeon_shift_right;

ret;


;
; performs involutive non-linear mapping 'gamma' on byte level
;
noekeon_gamma_byte:
  ; load word1_x
  ld state_1, x;
  adiw XL, 8;
  ; load word3_x
  ld state_3, x;
  subi XL, 0x04;
  ; load word2_x
  ld state_2, x;
  subi XL, 8;
  ; load word0_x
  ld state_0, x;

  rcall noekeon_gamma_non_linear_byte;

  ; temp_1 = word3_x; word3_x = word0_x; word0_x = temp_1;
  mov temp_1, state_3;
  mov state_3, state_0;
  mov state_0, temp_1;

  ; word2_x ^= word0_x ^ word1_x ^ word3_x;
  eor temp_1, state_1;
  eor temp_1, state_3;
  eor state_2, temp_1;

  rcall noekeon_gamma_non_linear_byte;

  ; store word0_x
  st x, state_0;
  adiw XL, 12;
  ; store word3_x
  st x, state_3;
  subi XL, 0x04;
  ; store word2_x
  st x, state_2;
  subi XL, 0x04;
  ; store word1_x
  st x+, state_1;
ret;

noekeon_gamma_non_linear_byte:
  ; word1_x ^= ~(word3_x | word2_x)
  mov temp_1, state_2;
  or temp_1, state_3;
  com temp_1;
  eor state_1, temp_1;
  ; word0_x ^= (word2_x & word1_x)
  mov temp_1, state_2;
  and temp_1, state_1;
  eor state_0, temp_1;
ret;

;
; performs the linear mapping on key and state for encryption
;
noekeon_theta_encrypt:
  noekeon_theta_part1:
    ; temp = word0 ^ word2; temp ^= temp>>>8 ^ temp<<<8
    ; word1 ^= temp;
    ; word3 ^= temp;

    ; move pointer to word0_0
    ldi XL, lo8(STATE_START + 0);
    ; set temporary pointer to word1_0
    ldi state_0, lo8(STATE_START + 0x04);
    rcall noekeon_theta_apply_xor_and_shift;

    ; check whether encryption/decryption is used
    sbrc round_counter, IS_DECRYPTION;
ret;

  noekeon_theta_part2:
    ; word0_x ^= key0_x; word1_x ^= key1_x; word2_x ^= key2_x; word3_x ^= key3_x;
    ldi XL, lo8(STATE_START + 16);
    ldi temp_1, 15;
    noekeon_theta_xor_key_loop:
      rcall noekeon_theta_xor_key;
	  dec temp_1;
	  brpl noekeon_theta_xor_key_loop;

    ; check whether encryption/decryption is used
    sbrc round_counter, IS_DECRYPTION;
    rjmp noekeon_theta_encrypt;

;
; performs the linear mapping on key and state for decryption
;
noekeon_theta_decrypt:
  noekeon_theta_part3:
    ; temp = word1 ^ word3; temp ^= temp>>>8 ^ temp<<<8
    ; word0 ^= temp;
    ; word2 ^= temp;

    ; move pointer to word1_0
    ldi XL, lo8(STATE_START + 0x04);
    ; set temporary pointer to word0_0
    ldi state_0, lo8(STATE_START + 0);
    rcall noekeon_theta_apply_xor_and_shift;

    ; check whether encryption/decryption is used
    sbrc round_counter, IS_DECRYPTION;
    rjmp noekeon_theta_part2;
ret;

noekeon_theta_apply_xor_and_shift:
  ; compute temp_0 ^ temp_1 ^ temp_2 ^ temp_3
  ldi temp_1, 0;
  rcall noekeon_theta_apply_xor_and_shift_load;
  adiw XL, 0x04;
  rcall noekeon_theta_apply_xor_and_shift_load;

  subi XL, 10;
  mov state_1, XL;
  rcall noekeon_theta_apply_xor;
  rcall noekeon_theta_apply_xor;
  subi state_1, 0x04;
  rcall noekeon_theta_apply_xor;
  rcall noekeon_theta_apply_xor;
ret;

noekeon_theta_apply_xor_and_shift_load:
  ; load wordx_0, wordx_1
  rcall ram_2_reg_word0_half;
  eor temp_1, state_2;
  eor temp_1, state_3;
  ; load wordx_2, wordx_3
  rcall ram_2_reg_word0_half;
  eor temp_1, state_2;
  eor temp_1, state_3;
ret;

noekeon_theta_apply_xor:
  mov XL, state_1;
  inc state_1;

  ld state_2, x+;
  adiw XL, 7
  ld state_3, x+;

  eor state_2, temp_1;
  eor state_2, state_3;

  ; word1_x ^= temp / word0_x ^= temp;
  mov XL, state_0;
  ld state_3, x;
  eor state_3, state_2;
  st x, state_3;

  ; word3_x ^= temp / word2_x ^= temp;
  adiw XL, 8;
  ld state_3, x;
  eor state_3, state_2;
  st x, state_3;

  subi XL, 7;
  mov state_0, XL;
  mov XL, state_1;
ret;

noekeon_theta_xor_key:
  ; applies key xor on byte level
  ; load key byte
  ld state_3, x;
  subi XL, 16;
  ; load state byte
  ld state_2, x;
  eor state_2, state_3;
  ; store state byte
  st x+, state_2;
  adiw XL, 16;
ret;
