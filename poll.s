; Poll counter for C programs. By Marcel Kilgus

	.text
	.even

	.globl	_poll_init

sms.achp	equ	$18		; Allocate space in Common HeaP
sms.lpol	equ	$1c		; Link in POLled action
sms.rpol	equ	$1d		; Remove POLled action

iod_iolk	equ	$0018		; l io driver linkage
iod_clos	equ	$0024		; l close routine address

chp_drlk	equ	$0004		; l pointer to DRiver LinKage (allocated space)
chp.len 	equ	$0010

counter_ptr	equ	0		; l
;space		equ	4		; l
poll_link	equ	8		; l Must be at offset 8 (iod_pllk)
poll_routine	equ	12		; l
free_routine	equ	16		; l
block.size	equ	20

; void poll_init(volatile unsigned int *counter)
_poll_init:
poll.init equ  8+4			; size of saved registers + return address
	movem.l a0/a3,-(a7)

	moveq	#sms.achp,d0
	moveq	#block.size,d1
	moveq	#-1,d2
	moveq	#0,d3
	trap	#1
	tst.l	d0
	bne	error
	move.l	a0,a3

	move.l	0+poll.init(a7),a0	; ptr to counter
	move.l	a0,counter_ptr(a3)

; This is a trick I learned from the Hotkey System:
; We need to clean up our poll routine even when the
; job is force-removed by RJOB. For this we pretend
; that our memory block is owned by a driver, which
; will automatically get notified when the memory
; block is freed upon job removal
	lea	job_removed(pc),a0
	move.l	a0,free_routine(a3)

	lea	free_routine+iod_iolk-iod_clos(a3),a0
	move.l	a0,chp_drlk-chp.len(a3) ; Fake driver entry

; Link in our own poll timer
	lea	poll(pc),a0
	move.l	a0,poll_routine(a3)
	moveq	#sms.lpol,d0
	lea	poll_link(a3),a0
	trap	#1
error:
	movem.l (a7)+,a0/a3
	rts

; Unlink our poll routine on job removal
job_removed:
	lea	iod_clos-free_routine(a3),a1	; Our stack frame
	moveq	#sms.rpol,d0
	lea	poll_link(a1),a0
	trap	#1
	rts

; By wisely chosing poll_link a3 points to our data area
poll:
	move.l	counter_ptr(a3),a0
	addq.l	#1,(a0)
	rts
