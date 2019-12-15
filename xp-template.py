#!/usr/bin/python

# Python template script
# import the print function: only required for Python 2, to disambiguate the function from the statement
from __future__ import print_function

# FOR LOOP:
# word="..Hello.."
# for letter in word:
# 	print(word,letter,sep=":",end="\n")
# 	print()
# print("it is done, innit")


# CASTING:
# number=10.12345
# inumber=10
# x= int(number)
# print("\nnumber=",number,"\ninumber=",inumber,"\nx=",x,"\n",sep="")


# FORMATTED PRINT: modulo operator separates formatting from items (in parentheses) to be formatted
# number=10.12345
# print("original: %f\tformatted: %.1f" % (number,number))
# # but apparently this is legacy and we're supposed to use str.format() instead
# # see http://www.python-course.eu/python3_formatted_output.php
# # use the default sequence
# print("Art: {:05d},  Price: {:8.2f}".format(453,59.058))
# # specify the sequence (0,1,2... etc)
# print("Art: {0:05d},  Price: {1:8.2f}, int_price: {2:.0f}".format(453,59.058,59.058))
# # explicitly define the elements referred to by formatting:
# print("Art: {a:05d},  Price: {p:8.2f}".format(a=453, p=59.058))


# SOME OTHER STRING METHODS
# word="dogs @ home. Capitalized?"
# print("capitalized=",word.capitalize())
# print("uppercase=",word.upper())
# print("lowercase=",word.lower())
# print("strip=",word.strip())
# print("replace=",word.replace("dogs","cats"))

# FILE READING
thisfunc="xp-template.p"
try:
	fhand= open(thisfunc)
except:
	print("\n\t***Error: File not found: ",thisfunc,"\n")
	exit()

for line in fhand:
	line=line.rstrip() # strip whitespace and newline from variable "line"
	if not line.startswith('#'):
		print(line)

print("Done",fhand)


## SEQUENCES:
## creating lists of numbers (from,to,increment)
#for i in range(0,10,2): print(i)
## in strings [ >= : < ]...
#word="0123456789"
#print(word[0:5])
#print(word[5:])


## NONE: specifies a variable with explicitly no memory reserved yet - will not show in variable explorer
## not sure if this is really needed
## similar to free()?
#flag=None
#if flag: print("1 flag is true")
#else: print("1 flag is false")
#flag=""
#if flag: print("2 flag is true")
#else: print("2 flag is false")
#x=None
#if x: print("x is true")
#else: print("x is false")


## TUPLES
#words=("dog","cat","mouse","piggies")
#print("elements in words():",len(words))
#for i in words: print(i,len(i),sep=":")
## ...or...
#print()
#for j in range(0,len(words),1):
#	print(words[j],len(words[j]),sep=":")
