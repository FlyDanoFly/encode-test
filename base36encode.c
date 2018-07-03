/* https://csl.name/post/c-functions-python/ */
// Py2/3 compatible packaging was taken from: https://docs.python.org/3/howto/cporting.html


#include <Python.h>

#define COUNT_DIVS 0

struct module_state {
  PyObject *error;
};


char const encoding_alphabet[36] = {
  '0','1','2','3','4','5','6','7','8','9',
  'a','b','c','d','e','f','g','h','i','j',
  'k','l','m','n','o','p','q','r','s','t',
  'u','v','w','x','y','z'
};


/*
  Take the 128 bit number passed as 2 parts, low and high
  Algorithm: High 64 bits is divided then cascades into two 32 bit
    registers until everything fits into one 64 bit register then
    finish dividing
 */
char *base36_encode_128_64_32_32(uint64_t high, uint64_t low, char *buffer) {
  // base36encode(128bits) = max 25 characters + 1 null terminator
  char *p = buffer + 25;
  *p = '\0';

  // Get the top 64 bits and the two lower 32 bits
  uint64_t pos1 = low & 0xffffffff;
  uint64_t pos2 = low >> 32;
  uint64_t pos34 = high;

  
  int divisor = 36;
  uint64_t quot;
  uint64_t mod;
  int num_divs = 0;
  int num_divsA = 0;
  int num_divsB = 0;

  // Divide the top 64 bits and cascade to the lower two
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
    *p = encoding_alphabet[mod];
    num_divs += 3;
    num_divsA += 1;
  }

  // Top 64 bits is done, combine lower two into one and finish
  pos1 = (pos2 << 32) | pos1;
  while (pos1) {
    quot = pos1 / divisor;
    mod = pos1 % divisor;
    pos1 = quot;

    p -= 1;
    *p = encoding_alphabet[mod];
    num_divs += 1;
    num_divsB += 1;
  }

  if (COUNT_DIVS) {
    printf("base36_encode_128_64_32_32(): num_divs = %d %d %d\n", num_divs, num_divsA, num_divsB);
  }
  
  return p;
}

/*
THIS is more like trike_div and less like trike_div2.
trike_div does 64,32,32 then 64
trike_div2 does 10,58,58 then 58,58 then 64 on array
trike_div3 does 10,58,58 then 58,58 then 64 on stack ints
*/
char *base36_encode_128_10_58_58(uint64_t high, uint64_t low, char buffer[26]) {
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
  while (pos34 > 0x3f) {
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
    *p = encoding_alphabet[mod];
    num_divs += 3;
    num_divsA += 1;
  }
  pos2 |= pos34 << 58;
  
  while (pos2 > 0x3f) {
    quot = pos2 / divisor;
    mod = pos2 % divisor;
    pos2 = quot;

    pos1 |= mod << 58;
    quot = pos1 / divisor;
    mod = pos1 % divisor;
    pos1 = quot;
    
    p -= 1;
    *p = encoding_alphabet[mod];
    num_divs += 2;
    num_divsB += 1;
  }
  pos1 |= pos2 << 58;

  while (pos1) {
    quot = pos1 / divisor;
    mod = pos1 % divisor;
    pos1 = quot;

    p -= 1;
    *p = encoding_alphabet[mod];
    num_divs += 1;
    num_divsC += 1;
  }

  if (COUNT_DIVS) {
    printf("base36_encode_128_10_58_58_B(): num_divs = %d %d %d %d\n", num_divs, num_divsA, num_divsB, num_divsC);
  }
  return p;
}

/*
  Take the special array of bits divided into 58 bit chunks
  Algorithm: Divide the highest 58 bits and cascade down,
    shifting the highest down one when it can fit into 64 bits

  ** This is destructive to the array! **
*/
char *base36_encode_n_58(uint64_t *ints, int len, char *buffer) {
  // Max number of chars needed is ceil(num_bits / 5) + 1 for the null terminal
  char buffer_size = len * 64 / 5 + 2;
  char *p = buffer + buffer_size - 1;
  *p = '\0';

  const int divisor = 36;
  int high = len - 1;
  int i;
  uint64_t quot;
  uint64_t mod;
  int num_divs = 0;

  // Keep looping until input bits are non-zero
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

    // Check if the highest int can be rounded into the next lower int
    if (high > 0 && ints[high] <= 0x3f) {
      ints[high - 1] |= ints[high] << 58;
      high -= 1;
    }

    p -= 1;
    *p = encoding_alphabet[mod];
  }
     
  if (COUNT_DIVS) {
    printf("base36_encode_n_58(): num_divs = %d\n", num_divs);
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
#endif
  return number;
}

/*
  Returns: 
  bool - true if number is > 128 bits
high and low - result of the first 128 bits of the passed in number
*/
int get_and_validate_128_bit_number(PyObject *number, unsigned long long *high, unsigned long long *low) {
  PyObject *shifted;
  PyObject *new_shifted;
  PyObject *to_shift = PyLong_FromLong(64);

  // Get the lower 64 bits
  *low = PyLong_AsUnsignedLongLongMask(number);
  shifted = PyNumber_Rshift(number, to_shift);

  // Get the higher 64 bits
  *high = PyLong_AsUnsignedLongLongMask(shifted);

  // Check if the number is too big
  new_shifted = PyNumber_Rshift(shifted, to_shift);
  Py_DECREF(to_shift);
  Py_DECREF(shifted);
  int greater_than_128_bits = PyObject_IsTrue(new_shifted);
  Py_DECREF(new_shifted);
  return greater_than_128_bits;
}


static PyObject* py_base36encode1(PyObject* self, PyObject* args) {
  // Takes up to a 128 bit int and converts it to base36encode
  PyObject *number;
  number = parseTuple(args);

  //
  // Allocate the result buffer
  
  // Max number of chars needed is ceil(num_bits / 5) + 1 for the null terminal
  // Add 2 more for a sentinal
  char buffer_size = 128 / 5 + 2 + 2;
  char *buffer = (char *) malloc(buffer_size * sizeof(char));
  buffer[0] = buffer[buffer_size - 1] = '-';
  char *encode_buffer = buffer + 26;
  *encode_buffer = '\0';
  encode_buffer -= 1;

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
    
    *encode_buffer = encoding_alphabet[idx];
    encode_buffer -= 1;
    
    Py_DECREF(number);
    Py_DECREF(rem);
    number = quot;
  } while (PyObject_IsTrue(number));
  Py_DECREF(number);
  encode_buffer += 1;

  // Check the sentinel  
  if (buffer[0] != '-' || buffer[buffer_size - 1] != '-') {
    printf("base36encode4(): BUFFER OVERRUN!\n");
    printf("base36encode4(): BUFFER OVERRUN!\n");
    printf("base36encode4(): BUFFER OVERRUN!\n");
  }
  
  PyObject *result = Py_BuildValue("s", encode_buffer);
  free(buffer);
  return result;
}

static PyObject* py_base36encode2(PyObject* self, PyObject* args) {
  // Takes up to a 128 bit int and converts it to base36encode
  PyObject *number = parseTuple(args);
  
  unsigned long long low;
  unsigned long long high;
  int greater_than_128_bits = get_and_validate_128_bit_number(number, &high, &low);
  Py_DECREF(number);
  
  if (greater_than_128_bits) {
    PyErr_SetString(PyExc_ValueError, "parameter must be be 128 bits");
    return NULL;
  }

  //
  // Allocate the result buffer
  
  // Max number of chars needed is ceil(num_bits / 5) + 1 for the null terminal
  // Add 2 more for a sentinal
  char buffer_size = 128 / 5 + 2 + 2;
  char *buffer = (char *) malloc(buffer_size * sizeof(char));
  buffer[0] = buffer[buffer_size - 1] = '-';
  char *encode_buffer = buffer + 1;

  if ((high == 0) && (low < 36)) {
    // short circuit case, the int < base
    encode_buffer[0] = encoding_alphabet[low];
    encode_buffer[1] = '\0';
  }
  else {
    encode_buffer = base36_encode_128_64_32_32(high, low, encode_buffer);
  }

  // Check the sentinel
  if (buffer[0] != '-' || buffer[buffer_size - 1] != '-') {
    printf("base36encode1(): BUFFER OVERRUN!\n");
    printf("base36encode1(): BUFFER OVERRUN!\n");
    printf("base36encode1(): BUFFER OVERRUN!\n");
  }
  
  PyObject *result = Py_BuildValue("s", encode_buffer);
  free(buffer);
  return result;
}

static PyObject* py_base36encode3(PyObject* self, PyObject* args) {
  // Takes up to a 128 bit int and converts it to base36encode
  PyObject *number = parseTuple(args);

  unsigned long long low;
  unsigned long long high;
  int greater_than_128_bits = get_and_validate_128_bit_number(number, &high, &low);
  Py_DECREF(number);
  
  if (greater_than_128_bits) {
    PyErr_SetString(PyExc_ValueError, "parameter must be be 128 bits");
    return NULL;
  }

  //
  // Allocate the result buffer
  
  // Max number of chars needed is ceil(num_bits / 5) + 1 for the null terminal
  // Add 2 more for a sentinal
  char buffer_size = 128 / 5 + 2 + 2;
  char *buffer = (char *) malloc(buffer_size * sizeof(char));
  buffer[0] = buffer[buffer_size - 1] = '-';
  char *encode_buffer = buffer + 1;

  if ((high == 0) && (low < 36)) {
    // short circuit case, the int < base
    buffer[0] = encoding_alphabet[low];
    buffer[1] = '\0';
  }
  else {
      encode_buffer = base36_encode_128_10_58_58(high, low, encode_buffer);
  }

  // Check the sentinel
  if (buffer[0] != '-' || buffer[buffer_size - 1] != '-') {
    printf("base36encode3(): BUFFER OVERRUN!\n");
    printf("base36encode3(): BUFFER OVERRUN!\n");
    printf("base36encode3(): BUFFER OVERRUN!\n");
  }
  
  PyObject *result = Py_BuildValue("s", encode_buffer);
  free(buffer);
  return result;
}

static PyObject* py_base36encode4(PyObject* self, PyObject* args) {
  // Takes up to a 128 bit int and converts it to base36encode
  PyObject *number = parseTuple(args);

  unsigned long long llints[30];
  int i;
  if (0) {
  int greater_than_128_bits = get_and_validate_128_bit_number(number, &llints[1], &llints[0]);
  Py_DECREF(number);
  i = 2;
  
  if (greater_than_128_bits) {
    PyErr_SetString(PyExc_ValueError, "parameter must be be 128 bits");
    return NULL;
  }
  }
  if (1) {

  PyObject *new_shifted;
  PyObject *shifted = number;
  Py_INCREF(shifted);
  PyObject *to_shift = PyLong_FromLong(64 - 6);
  unsigned long long mask = 0x03ffffffffffffff;
  i = 0;
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
  }
  
  // The biggest buffer needed to base36 encode a 128 bit int is 26 bytes = 25 bytes + 1 terminal
  // Max number of chars needed is ceil(num_bits / 5) + 1 for the null terminal
  // Add 2 more for a sentinal
  char buffer_size = i * 64 / 5 + 2 + 2;
  char *buffer = (char *) malloc(buffer_size * sizeof(char));
  buffer[0] = buffer[buffer_size - 1] = '-';
  char *encode_buffer = buffer + 1;
  //  const int sz = 26;
  //  char big_buffer[sz+2]; // for a sentry of '-' on each end
  //  char *buffer = big_buffer + 1;
  //  char *p = buffer;
  //  big_buffer[0] = '-';
  //  big_buffer[sz+1] = '-';

  if ((i == 1) && (llints[0] < 36)) {
    // short circuit case, the int < base
    buffer[0] = encoding_alphabet[llints[0]];
    buffer[1] = '\0';
  }
  else {
    encode_buffer = base36_encode_n_58(llints, i, encode_buffer);
  }

  // Check the sentinel
  if (buffer[0] != '-' || buffer[buffer_size - 1] != '-') {
    printf("base36encode2(): BUFFER OVERRUN!\n");
    printf("base36encode2(): BUFFER OVERRUN!\n");
    printf("base36encode2(): BUFFER OVERRUN!\n");
  }

  PyObject *result = Py_BuildValue("s", encode_buffer);
  free(buffer);
  return result;
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
