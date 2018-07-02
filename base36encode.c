/* https://csl.name/post/c-functions-python/ */

#include <Python.h>

#define COUNT_DIVS 0
#define PROFILE_THIS 0

struct module_state {
  PyObject *error;
};

const unsigned int n = 100;
const unsigned int n2 = 10000;

char const trans_table[36] = {
  '0','1','2','3','4','5','6','7','8','9',
  'a','b','c','d','e','f','g','h','i','j',
  'k','l','m','n','o','p','q','r','s','t',
  'u','v','w','x','y','z'
};

/*
So... trike_div is faster than trike_div2 even though it has more divs and mods
That's weird. Why?
Profiling...

It looks like trike_div is slower by about 5x. So wehre's the inefficiency? The setup?

I'm also curious to try my most recent made up metod:
  Pass in 2 64bit ints. Split it up into 2x 64bit ints with room for the remainder, and 1x 32bit with the 10 remaining bits. Do 3 loops, each one just does the divs it needs and moves to the next when its highest is 0.

ALSO! Getting occasional SEGFAULTs! Some pointer somewhere is bad. ESP can be seen when running both sidy by side.
-- FIXED! Bad references!
-- Back to WHY?! is base36encode
 */
#include <time.h>
long compute_time_delta(struct timespec *t0, struct timespec *t1) {
  return (t1->tv_sec-t0->tv_sec)*1000000 + t1->tv_nsec-t0->tv_nsec;
}

/*
Dano 2018.06.27:
I think this is the one to use, nice and simple.
It onle needs to last until python 3 because it has a native way of doing this?
Need to look through all notes and figure that out.
*/
char *trike_div1(uint64_t high, uint64_t low, char buffer[26]) {
  // base36encode(128bits) = max 25 characters + 1 null terminator
  char *p = buffer + 25;
  *p = '\0';

  // pos: 128 bits = 4321  (each 64 bit things, only 32 at a time)
  // (pos4 /% divisor) -> (quot4, mod4)
  //    then pos3 | mod4 << 32
  //    then pos2 | mod3 << 32
  //    then pos1 | mod2 << 32
  // then mod1 has remainder
  // optimization pos3 is 64 bits, then pos2 and pos1 get the rest of the treatment
  uint64_t pos1 = low & 0xffffffff;
  uint64_t pos2 = low >> 32;
  uint64_t pos34 = high;
  
  int divisor = 36;
  uint64_t quot;
  uint64_t mod;
  int num_divs = 0;
  int num_divsA = 0;
  int num_divsB = 0;
  while (pos34) {
    quot = pos34 / divisor;
    mod = pos34 % divisor;
    pos34 = quot;

    pos2 |= mod << 32;
    quot = pos2 / divisor;
    mod = pos2 % divisor;
    pos2 = quot;

    pos1 |= mod << 32;
    quot = pos1 / divisor;
    mod = pos1 % divisor;
    pos1 = quot;
    
    p -= 1;
    *p = trans_table[mod];
    num_divs += 3;
    num_divsA += 1;
  }

  pos1 = (pos2 << 32) | pos1;
  while (pos1) {
    quot = pos1 / divisor;
    mod = pos1 % divisor;
    pos1 = quot;

    p -= 1;
    *p = trans_table[mod];
    num_divs += 1;
    num_divsB += 1;
  }

  if (COUNT_DIVS) {
    printf("trike_div1(): num_divs = %d %d %d\n", num_divs, num_divsA, num_divsB);
  }
  return p;
}

// Fewer divs when #bits > 64, slightly faster and unbound by 128 bits
// Does the overhead matter when #bits <= 64? On par, extra setup doesn't hurt much
char *trike_div2(uint64_t *ints, int len, char buffer[26]) {
  // base36encode(128bits) = max 25 characters + 1 null terminator
  char *p = buffer + 25;
  *p = '\0';

  // pos: 128 bits = 4321  (each 64 bit things, only 32 at a time)
  // (pos4 /% divisor) -> (quot4, mod4)
  //    then pos3 | mod4 << 32
  //    then pos2 | mod3 << 32
  //    then pos1 | mod2 << 32
  // then mod1 has remainder
  // optimization pos3 is 64 bits, then pos2 and pos1 get the rest of the treatment
  const int divisor = 36;
  int high = len - 1;
  int i;
  uint64_t quot;
  uint64_t mod;
  int num_divs = 0;
  while (high > 0 || ints[0]) {
    for (i = high; i >= 0; i--) {
      quot = ints[i] / divisor;
      mod = ints[i] % divisor;
      if (i > 0) {
	ints[i-1] |= mod << (64-6);
      }
      ints[i] = quot;
      ++num_divs;
    }
    if (!ints[high]) {
      high -= 1;
    }

    p -= 1;
    *p = trans_table[mod];
  }
     
  if (COUNT_DIVS) {
    printf("trike_div2(): num_divs = %d\n", num_divs);
  }
 return p;
}

/*
THIS is more like trike_div and less like trike_div2.
trike_div does 64,32,32 then 64
trike_div2 does 10,58,58 then 58,58 then 64 on array
trike_div3 does 10,58,58 then 58,58 then 64 on stack ints
*/
char *trike_div3(uint64_t high, uint64_t low, char buffer[26]) {
  // base36encode(128bits) = max 25 characters + 1 null terminator
  char *p = buffer + 25;
  *p = '\0';

  unsigned long long mask =  0x03ffffffffffffff;
  uint64_t pos1 = low & mask;
  uint64_t pos2 = ((high << 6) & mask) | (low >> (64-6));
  uint64_t pos34 = high >> (52);
  
  int divisor = 36;
  uint64_t quot;
  uint64_t mod;
  int num_divs = 0;
  int num_divsA = 0;
  int num_divsB = 0;
  int num_divsC = 0;
  while (pos34) {
    quot = pos34 / divisor;
    mod = pos34 % divisor;
    pos34 = quot;

    pos2 |= mod << 58;
    quot = pos2 / divisor;
    mod = pos2 % divisor;
    pos2 = quot;

    pos1 |= mod << 58;
    quot = pos1 / divisor;
    mod = pos1 % divisor;
    pos1 = quot;
    
    p -= 1;
    *p = trans_table[mod];
    num_divs += 3;
    num_divsA += 1;
  }

  while (pos2) {
    quot = pos2 / divisor;
    mod = pos2 % divisor;
    pos2 = quot;

    pos1 |= mod << 58;
    quot = pos1 / divisor;
    mod = pos1 % divisor;
    pos1 = quot;
    
    p -= 1;
    *p = trans_table[mod];
    num_divs += 2;
    num_divsB += 1;
  }

  pos1 = (pos2 << 32) | pos1;
  while (pos1) {
    quot = pos1 / divisor;
    mod = pos1 % divisor;
    pos1 = quot;

    p -= 1;
    *p = trans_table[mod];
    num_divs += 1;
    num_divsC += 1;
  }

  if (COUNT_DIVS) {
    printf("trike_div3(): num_divs = %d %d %d %d\n", num_divs, num_divsA, num_divsB, num_divsC);
  }
  return p;
}

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif

PyObject* parseTuple(PyObject* args) {
  PyObject *number;
  
#if PY_MAJOR_VERSION >= 3
  // Python 3 just has longs (yay!), extraction and type checking is easy
  if (!PyArg_ParseTuple(args, "O!", &PyLong_Type, &number)) {
    PyErr_SetString(PyExc_TypeError, "parameter must be an int");
    return NULL;
  }
  // TODO: when converting this to entirely Python 3, can remove this and the folling reference counting... why? add reason!
  Py_INCREF(number);
#else
  // Python 2 just has both ints and longs (boo!)
  // Extract to generic object, type check, cast an int to a long
  PyObject *obj;
  if (!PyArg_ParseTuple(args, "O", &obj)) {
    PyErr_SetString(PyExc_TypeError, "error extracting parameter");
    return NULL;
  }
  if (PyInt_Check(obj)) {
    number = PyNumber_Long(obj);
  }
  else if (PyLong_Check(obj)) {
    number = obj;
    Py_INCREF(number);
  }
  else {
    PyErr_SetString(PyExc_TypeError, "parameter must be an int");
    return NULL;    
  }
  return number;
#endif
}

static PyObject* py_base36encode1(PyObject* self, PyObject* args) {
  // Takes up to a 128 bit int and converts it to base36encode
  PyObject *number;

  number = parseTuple(args);
  /*
#if PY_MAJOR_VERSION >= 3
  // Python 3 just has longs (yay!), extraction and type checking is easy
  if (!PyArg_ParseTuple(args, "O!", &PyLong_Type, &number)) {
    PyErr_SetString(PyExc_TypeError, "parameter must be an int");
    return NULL;
  }
  // TODO: when converting this to entirely Python 3, can remove this and the folling reference counting
  Py_INCREF(number);
#else
  // Python 2 just has both ints and longs (boo!)
  // Extract to generic object, type check, cast an int to a long
  PyObject *obj;
  if (!PyArg_ParseTuple(args, "O", &obj)) {
    PyErr_SetString(PyExc_TypeError, "error extracting parameter");
    return NULL;
  }
  if (PyInt_Check(obj)) {
    number = PyNumber_Long(obj);
  }
  else if (PyLong_Check(obj)) {
    number = obj;
    Py_INCREF(number);
  }
  else {
    PyErr_SetString(PyExc_TypeError, "parameter must be an int");
    return NULL;    
  }
#endif
  */
  PyObject *shifted;
  PyObject *new_shifted;
  PyObject *to_shift = PyLong_FromLong(64);
  unsigned long long low = PyLong_AsUnsignedLongLongMask(number);
  shifted = PyNumber_Rshift(number, to_shift);
  Py_DECREF(number);
  unsigned long long high = PyLong_AsUnsignedLongLongMask(shifted);

  // Check if the number is too big
  new_shifted = PyNumber_Rshift(shifted, to_shift);
  Py_DECREF(to_shift);
  Py_DECREF(shifted);
  if (PyObject_IsTrue(new_shifted)) {
    PyErr_SetString(PyExc_TypeError, "parameter must be be 128 bits");
    Py_DECREF(new_shifted);
    return NULL;
  }
  Py_DECREF(new_shifted);


  // The biggest buffer needed to base36 encode a 128 bit int is 26 bytes = 25 bytes + 1 terminal
  const int sz = 26;
  char big_buffer[sz+2]; // for a sentry of '-' on each end
  char *buffer = big_buffer + 1;
  char *p = buffer;
  big_buffer[0] = '-';
  big_buffer[sz+1] = '-';

  if ((high == 0) && (low < 36)) {
    // short circuit case, the int < base
    buffer[0] = trans_table[low];
    buffer[1] = '\0';
  }
  else {
    if (PROFILE_THIS) {
      struct timespec start;
      struct timespec end;
      long result = 0;
      long result_min = 1000000000000;
      long result_max = 0;
      long result_cum = 0;
      int bad_results = 0;
      clock_gettime(CLOCK_MONOTONIC, &start);
      clock_gettime(CLOCK_MONOTONIC, &end);
      for (unsigned int yi = 0; yi < n; yi++) {
	clock_gettime(CLOCK_MONOTONIC, &start);
	for (unsigned int yi2 = 0; yi2 < n2; yi2++) {
	  p = trike_div1(high, low, buffer);
	}
	clock_gettime(CLOCK_MONOTONIC, &end);
	result = compute_time_delta(&start, &end);
	if (result > 0) {
	  result_cum += result;
	  result_min = (result < result_min) ? result : result_min;
	  result_max = (result > result_max) ? result : result_max;
	}
	else {
	  bad_results += 1;
	}
      }
      printf("base36encode1(): time (inner loop = %6d) (n - bad = total), (n, min, avg, max = total): (%6d - %6d = %6d), (%8ld, %8ld, %8ld) = %ld\n", n2, n, bad_results, (n - bad_results), result_min, result_cum / (n - bad_results), result_max, result_cum);
    } else {
      p = trike_div1(high, low, buffer);
    }
  }

  if (big_buffer[0] != '-' || big_buffer[sz+1] != '-') {
    printf("base36encode1(): BUFFER OVERRUN!\n");
    printf("base36encode1(): BUFFER OVERRUN!\n");
    printf("base36encode1(): BUFFER OVERRUN!\n");
  }
  return Py_BuildValue("s", p);
}

static PyObject* py_base36encode2(PyObject* self, PyObject* args) {
  // Takes up to a 128 bit int and converts it to base36encode
  PyObject *number;


  number = parseTuple(args);
  /*
#if PY_MAJOR_VERSION >= 3
  // Python 3 just has longs (yay!), extraction and type checking is easy
  if (!PyArg_ParseTuple(args, "O!", &PyLong_Type, &number)) {
    PyErr_SetString(PyExc_TypeError, "parameter must be an int");
    return NULL;
  }
  // TODO: when converting this to entirely Python 3, can remove this and the folling reference counting... why? add reason!
  Py_INCREF(number);
#else
  // Python 2 just has both ints and longs (boo!)
  // Extract to generic object, type check, cast an int to a long
  PyObject *obj;
  if (!PyArg_ParseTuple(args, "O", &obj)) {
    PyErr_SetString(PyExc_TypeError, "error extracting parameter");
    return NULL;
  }
  if (PyInt_Check(obj)) {
    number = PyNumber_Long(obj);
  }
  else if (PyLong_Check(obj)) {
    number = obj;
    Py_INCREF(number);
  }
  else {
    PyErr_SetString(PyExc_TypeError, "parameter must be an int");
    return NULL;    
  }
#endif
  */
  
  unsigned long long llints[30] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
  for (int xeo = 0; xeo < 30; ++xeo) {
    llints[xeo] = 0;
  }

  PyObject *new_shifted;
  PyObject *shifted = number;
  Py_INCREF(shifted);
  PyObject *to_shift = PyLong_FromLong(64 - 6);
  unsigned long long mask = 0x03ffffffffffffff;
  int i = 0;
  do {
    llints[i] = PyLong_AsUnsignedLongLongMask(shifted) & mask;
    new_shifted = PyNumber_Rshift(shifted, to_shift);
    Py_DECREF(shifted);
    shifted = new_shifted;
    i += 1;
  } while (PyObject_IsTrue(shifted));

  Py_DECREF(to_shift);
  Py_DECREF(new_shifted);
  Py_DECREF(number);

  // for profiling
  unsigned long long *huge_llints = (unsigned long long *) malloc(3 * n2 * n * sizeof( unsigned long long));
  
  // The biggest buffer needed to base36 encode a 128 bit int is 26 bytes = 25 bytes + 1 terminal
  const int sz = 26;
  char big_buffer[sz+2]; // for a sentry of '-' on each end
  char *buffer = big_buffer + 1;
  char *p = buffer;
  big_buffer[0] = '-';
  big_buffer[sz+1] = '-';

  if ((i == 1) && (llints[0] < 36)) {
    // short circuit case, the int < base
    buffer[0] = trans_table[llints[0]];
    buffer[1] = '\0';
  }
  else {
    if (PROFILE_THIS) {
      struct timespec start;
      struct timespec end;
      long result = 0;
      long result_min = 1000000000000;
      long result_max = 0;
      long result_cum = 0;
      int bad_results = 0;
      clock_gettime(CLOCK_MONOTONIC, &start);
      clock_gettime(CLOCK_MONOTONIC, &end);
      for (unsigned int yi = 0; yi < n; yi++) {
  for (int aoetuh = 0; aoetuh < n2*n; ++aoetuh) {
    huge_llints[aoetuh * 3 + 0] = llints[0];
    huge_llints[aoetuh * 3 + 1] = llints[1];
    huge_llints[aoetuh * 3 + 2] = llints[2];
  }
	clock_gettime(CLOCK_MONOTONIC, &start);
	for (unsigned int yi2 = 0; yi2 < n2; yi2++) {
	  unsigned long long xllints[30];
	  for (int xeo = 0; xeo < 30; ++xeo) {
	    xllints[xeo] = llints[xeo];
	  }
	  p = trike_div2(huge_llints+(yi2*3), i, buffer);
	}
	clock_gettime(CLOCK_MONOTONIC, &end);
	result = compute_time_delta(&start, &end);
	if (result > 0) {
	  result_cum += result;
	  result_min = (result < result_min) ? result : result_min;
	  result_max = (result > result_max) ? result : result_max;
	}
	else {
	  bad_results += 1;
	}
      }
      printf("base36encode2(): time (inner loop = %6d) (n - bad = total), (n, min, avg, max = total): (%6d - %6d = %6d), (%8ld, %8ld, %8ld) = %ld\n", n2, n, bad_results, (n - bad_results), result_min, result_cum / (n - bad_results), result_max, result_cum);
    } else {
      p = trike_div2(llints, i, buffer);
    }
  }

  free(huge_llints);
  
  if (big_buffer[0] != '-' || big_buffer[sz+1] != '-') {
    printf("base36encode2(): BUFFER OVERRUN!\n");
    printf("base36encode2(): BUFFER OVERRUN!\n");
    printf("base36encode2(): BUFFER OVERRUN!\n");
  }

  return Py_BuildValue("s", p);
}

static PyObject* py_base36encode3(PyObject* self, PyObject* args) {
  // Takes up to a 128 bit int and converts it to base36encode
  PyObject *number;

  number = parseTuple(args);
  /*
#if PY_MAJOR_VERSION >= 3
  // Python 3 just has longs (yay!), extraction and type checking is easy
  if (!PyArg_ParseTuple(args, "O!", &PyLong_Type, &number)) {
    PyErr_SetString(PyExc_TypeError, "parameter must be an int");
    return NULL;
  }
  // TODO: when converting this to entirely Python 3, can remove this and the folling reference counting
  Py_INCREF(number);
#else
  // Python 2 just has both ints and longs (boo!)
  // Extract to generic object, type check, cast an int to a long
  PyObject *obj;
  if (!PyArg_ParseTuple(args, "O", &obj)) {
    PyErr_SetString(PyExc_TypeError, "error extracting parameter");
    return NULL;
  }
  if (PyInt_Check(obj)) {
    number = PyNumber_Long(obj);
  }
  else if (PyLong_Check(obj)) {
    number = obj;
    Py_INCREF(number);
  }
  else {
    PyErr_SetString(PyExc_TypeError, "parameter must be an int");
    return NULL;    
  }
#endif
  */
  PyObject *shifted;
  PyObject *new_shifted;
  PyObject *to_shift = PyLong_FromLong(64);
  unsigned long long low = PyLong_AsUnsignedLongLongMask(number);
  shifted = PyNumber_Rshift(number, to_shift);
  Py_DECREF(number);
  unsigned long long high = PyLong_AsUnsignedLongLongMask(shifted);

  // Check if the number is too big
  new_shifted = PyNumber_Rshift(shifted, to_shift);
  Py_DECREF(to_shift);
  Py_DECREF(shifted);
  if (PyObject_IsTrue(new_shifted)) {
    PyErr_SetString(PyExc_TypeError, "parameter must be be 128 bits");
    Py_DECREF(new_shifted);
    return NULL;
  }
  Py_DECREF(new_shifted);


  // The biggest buffer needed to base36 encode a 128 bit int is 26 bytes = 25 bytes + 1 terminal
  const int sz = 26;
  char big_buffer[sz+2]; // for a sentry of '-' on each end
  char *buffer = big_buffer + 1;
  char *p = buffer;
  big_buffer[0] = '-';
  big_buffer[sz+1] = '-';

  if ((high == 0) && (low < 36)) {
    // short circuit case, the int < base
    buffer[0] = trans_table[low];
    buffer[1] = '\0';
  }
  else {
    if (PROFILE_THIS) {
      struct timespec start;
      struct timespec end;
      long result = 0;
      long result_min = 1000000000000;
      long result_max = 0;
      long result_cum = 0;
      int bad_results = 0;
      clock_gettime(CLOCK_MONOTONIC, &start);
      clock_gettime(CLOCK_MONOTONIC, &end);
      for (unsigned int yi = 0; yi < n; yi++) {
	clock_gettime(CLOCK_MONOTONIC, &start);
	for (unsigned int yi2 = 0; yi2 < n2; yi2++) {
	  p = trike_div3(high, low, buffer);
	}
	clock_gettime(CLOCK_MONOTONIC, &end);
	result = compute_time_delta(&start, &end);
	if (result > 0) {
	  result_cum += result;
	  result_min = (result < result_min) ? result : result_min;
	  result_max = (result > result_max) ? result : result_max;
	}
	else {
	  bad_results += 1;
	}
      }
      printf("base36encode3(): time (inner loop = %6d) (n - bad = total), (n, min, avg, max = total): (%6d - %6d = %6d), (%8ld, %8ld, %8ld) = %ld\n", n2, n, bad_results, (n - bad_results), result_min, result_cum / (n - bad_results), result_max, result_cum);
    } else {
      p = trike_div3(high, low, buffer);
    }
  }

  if (big_buffer[0] != '-' || big_buffer[sz+1] != '-') {
    printf("base36encode3(): BUFFER OVERRUN!\n");
    printf("base36encode3(): BUFFER OVERRUN!\n");
    printf("base36encode3(): BUFFER OVERRUN!\n");
  }
  return Py_BuildValue("s", p);
}

static PyObject* py_base36encode4(PyObject* self, PyObject* args) {
  // Takes up to a 128 bit int and converts it to base36encode
  PyObject *number;
  number = parseTuple(args);

  const int sz = 26;
  char big_buffer[sz+2]; // for a sentry of '-' on each end
  char *buffer = big_buffer + 1;
  char *p = &buffer[25];
  *p = 0;
  p--;
  big_buffer[0] = '-';
  big_buffer[sz+1] = '-';

  PyObject *divmod_tuple;
  PyObject *quot;
  PyObject *rem;
  PyObject *base = PyLong_FromLong(36);
  int idx;
  do {
    divmod_tuple = PyNumber_Divmod(number, base);
    quot = PySequence_GetItem(divmod_tuple, 0);
    rem = PySequence_GetItem(divmod_tuple, 1);
    idx = PyLong_AsLong(rem);
    *p = trans_table[idx];
    p -= 1;
    Py_DECREF(number);
    Py_DECREF(rem);
    number = quot;
  } while (PyObject_IsTrue(number));
  Py_DECREF(number);
  ++p;
  if (big_buffer[0] != '-' || big_buffer[sz+1] != '-') {
    printf("base36encode4(): BUFFER OVERRUN!\n");
    printf("base36encode4(): BUFFER OVERRUN!\n");
    printf("base36encode4(): BUFFER OVERRUN!\n");
  }
  return Py_BuildValue("s", p);
}

/*
 * Bind Python function names to our C functions
 */
static PyMethodDef base36encode_methods[] = {
  {"base36encode1", py_base36encode1, METH_VARARGS},
  {"base36encode2", py_base36encode2, METH_VARARGS},
  {"base36encode3", py_base36encode3, METH_VARARGS},
  {"base36encode4", py_base36encode4, METH_VARARGS},
  {NULL, NULL}
};

#if PY_MAJOR_VERSION >= 3

static int base36encode_traverse(PyObject *m, visitproc visit, void *arg) {
  Py_VISIT(GETSTATE(m)->error);
  return 0;
}

static int base36encode_clear(PyObject *m) {
  Py_CLEAR(GETSTATE(m)->error);
  return 0;
}

static struct PyModuleDef moduledef = {
  PyModuleDef_HEAD_INIT,
  "base36encode",
   NULL,
  sizeof(struct module_state),
  base36encode_methods,
  NULL,
  base36encode_traverse,
  base36encode_clear,
  NULL
};

#define INITERROR return NULL

PyMODINIT_FUNC
PyInit_base36encode(void)

#else
#define INITERROR return

void
  initbase36encode(void)
#endif
{

#if PY_MAJOR_VERSION >= 3
  PyObject *module = PyModule_Create(&moduledef);
#else
  PyObject *module = Py_InitModule("base36encode", base36encode_methods);
#endif

  if (module == NULL)
    INITERROR;
  struct module_state *st = GETSTATE(module);

  st->error = PyErr_NewException("base36encode.Error", NULL, NULL);
  if (st->error == NULL) {
    Py_DECREF(module);
    INITERROR;
  }

#if PY_MAJOR_VERSION >= 3
  return module;
#endif
}
