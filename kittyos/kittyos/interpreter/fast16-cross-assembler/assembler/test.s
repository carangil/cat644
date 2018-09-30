ldi a @out_dev
offset
syscall 1   #find device
swp b       #b has the device




ldi a @hello
offset

nextchar:
ldc a  #get character *a to c
swp c # c has ptr, a has char  

#jump to halt if low byte of A is zero
ldi d $00ff
and d
jaz @done

syscall 2	#write reg A to device in B

swp c #get pointer

ldi d 1
add d
jmpr @nextchar

done:
# register B has output device
swp B  # A has output
swp D  # D has output device

ldi a @in_dev
offset
syscall 1 #get input device
swp b  # b as input device

again:
syscall 3 # read a char
#char in a , input in B, output in D

swp b  
#char in b, input in A, output in D

swp d
#char in b, input in D, output in A

swp b  
#char in A, input in D, output in B

syscall 2 #write char

#zero in A, input in D, output in B

swp b

swp d

swp b

jmpr @again

halt:
jmpr @halt

out_dev:
string 'scr
byte 0

in_dev:
string 'key
byte 0

string ' 
hello:
string 'Hello World
byte 0
byte 7

