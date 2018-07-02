
The logic works!
And... this needs cleaning!

TODO:
	Finish profiling on profile production dyno to get an idea of speeds
	Narrow down to 1 way of doing it: current winning one is trike_div3
    Cleanup main code
    Figure out a naming scheme, maybe:
        from fastbase36 import base36encode 
        from fastbase36 import base36encode128 (only does 128 bit ints, actually could do 128+54 but will constrain) 
        from fastbase36 import base36encode128 (only does 128 bit ints, actually could do 128+54 but will constrain)
		
    x Fix pass in 0, probably if i < 36 return that encoded?
    Error checking! Pass in a string? Pass in a number that's too big?
    Add some tests
    Have the python base36encode in there? How to proprely package it?
        - no or make it slow
    Test with database exporting, old vs new way!
    Packaging!
    Public or private repository?
    Get rid of old code snapshots, not relying enough on git eh?
    Lots of comments about methods tried?
        See bottom for links



This was all the expriments to come up with a very fast base36encoder

It was successful! And my version was the best, ridiculously fast! Yay! Here's the method:

Given: convert up to 128 bit ints to base36enocode, alphabet = digits then lowercase letters

Split the 128 bit number into:
   64 bits of high in a 64 bit register
   32 bits of medium in a 64 bit register
   32 bits of low in a 64 bit register

While high > 0:
    carry = high % 36
    high = high / 36
  
    med = med | carry << 32
    carry = med % 36
    med = med / 36

    low = low | carry << 32
    remainder = low % 36
    low = low / 36

    *p++ = table[remainder]

low = low | med << 32

while low > 0:
    remainder = low % 36
    low = low / 36
  
    *p++ = table[remainder]




** LINKS **
gmpy source code
    https://github.com/aleaxit/gmpy

Heroku packages:
    https://devcenter.heroku.com/articles/stack-packages

I almost tried this library, but it wasn't simple or straight forward.
Also, once obfuscated its are first class objects, I'm not sure we need something
half the speed. My algorithm is already 17:0.3 improvement
    https://www.codeproject.com/Tips/785014/UInt-Division-Modulus

Hackers delight basiscs, I never got very far with this:
    http://www.hackersdelight.org/basics2.pdf

Some random 128 bit in C++, more complicated than my method and again, probably not worth it:
    https://mrob.com/pub/math/int128.c.txt

Another arlogithm. I used the one in the answer and it works so long as the result is 64 bits long
Also, it's 10x slower that my algorithm:
    https://codereview.stackexchange.com/questions/67962/mostly-portable-128-by-64-bit-division

Actually simple algorithm for 64bit / 32 bit (which works)
I tested this, and it's on par with my routine on my computer and on Heroku
Assuming that it's that way generally, so go with the simpler / less fancy one (mine)
    http://lxr.linux.no/linux+v2.6.22/lib/div64.c
