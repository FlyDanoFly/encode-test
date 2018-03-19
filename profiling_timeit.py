from __future__ import print_function
import timeit
import sys


fn_setup = (
#"""
#from fun import base36encode as encode_it
#""",
"""
from mymodule import myfn as encode_it
""",
#"""
#from mymodule import myfn2 as encode_it
#""",
"""
from mymodule import myfn3 as encode_it
""",
"""
import sys
if sys.version_info[0] == 3:
    # don't run this test
    encode_it = lambda x: the_enc
else:
    from mymodule import myWowFunction as encode_it
"""

)

int_letter = '' if sys.version_info[0] == 3 else 'L'
case_setup = (
"""
the_int = 15651564330810771165{}
the_enc = "3awvki7bh5eql"
""".format(int_letter)
,
"""
the_int = 276690541156040511542851614509818852018{}
the_enc = 'cbn9ihut2vknewnwgwkylntma'
""".format(int_letter)
)
fixture = """assert encode_it(the_int) == the_enc, '"{}" != "{}"'.format(the_enc, encode_it(the_int))"""

number = 10000
repeat = 10
idx = 0
for case in case_setup:
    idx += 1
    print('='*80)
    print(case)
#    if idx == 1:
#        print('...skipping {}'.format(idx))
#        continue
    for fn in fn_setup:
        idx += 1
        print('-'*40)
        print(fn)
        print('.'*20)
        print(fixture)
#        if idx < 5:
#            print('...skipping {}'.format(idx))
#            continue
        the_time = timeit.repeat(fixture, case + fn, repeat=repeat)
        print('min / max / avg = {} {} {}'.format(min(the_time), max(the_time), sum(the_time) / repeat))
