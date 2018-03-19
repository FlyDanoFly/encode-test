/* https://csl.name/post/c-functions-python/ */

#include <Python.h>

struct module_state {
  PyObject *error;
};

char const trans_table[36] = {
  '0','1','2','3','4','5','6','7','8','9',
  'a','b','c','d','e','f','g','h','i','j',
  'k','l','m','n','o','p','q','r','s','t',
  'u','v','w','x','y','z'
};

uint64_t GetTimeStamp() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

int gangsta_div(uint64_t high, uint64_t low, uint64_t *quot_result, uint64_t *rem_result) {
  // Only works if the result fits in 64 bits, otherwise it doesn't work

  // numerator
  uint64_t a_lo = low; //0xffffffffffffffff;
  uint64_t a_hi = high; // 0xf;

  //printf("lo: %llu\n", a_lo);
  //printf("hi: %llu\n", a_hi);

  // denominator
  uint64_t b = 36;

  // quotient
  uint64_t q = a_lo << 1;

  // remainder
  uint64_t rem = a_hi;

  uint64_t carry = a_lo >> 63;
  uint64_t temp_carry = 0;
  int i;

  for(i = 0; i < 64; i++)
    {
      temp_carry = rem >> 63;
      rem <<= 1;
      rem |= carry;
      carry = temp_carry;

      if(carry == 0)
	{
	  if(rem >= b)
	    {
	      carry = 1;
	    }
	  else
	    {
	      temp_carry = q >> 63;
	      q <<= 1;
	      q |= carry;
	      carry = temp_carry;
	      continue;
	    }
	}

      rem -= b;
      rem -= (1 - carry);
      carry = 1;
      temp_carry = q >> 63;
      q <<= 1;
      q |= carry;
      carry = temp_carry;
    }

  if (quot_result) {
    *quot_result = q;
  }

  if (rem_result) {
    *rem_result = rem;
  }
  //printf("sizeof(unsigned long long) = %d\n", sizeof(unsigned long long));
  //printf("quotient = %llu\n", (long long unsigned int)q);
  //printf("remainder = %llu\n", (long long unsigned int)rem);
  //printf("lo: %llu\n", a_lo);
  //printf("hi: %llu\n", a_hi);
  return 0;
}

int trike_div(uint64_t high, uint64_t low, char **pp) {
  /*
  char buffer[30];
  for (int iii = 0; iii < 30; ++iii) {
    buffer[iii] = 0;
  }
  char *p = &buffer[29];
  */
  char *p = *pp;

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
    
    *p = trans_table[mod];
    p -= 1;
  }

  pos1 = (pos2 << 32) | pos1;
  while (pos1) {
    quot = pos1 / divisor;
    mod = pos1 % divisor;
    pos1 = quot;
    
    *p = trans_table[mod];
    p -= 1;
    
  }

  *pp = p+1;
  //printf("here: %s\n", p+1);
  assert(strcmp("6h7yzb4z112efh4n44afgst7a", *pp) == 0);
  return 0;
}

int trike_div2(uint64_t high, uint64_t low, char **pp) {
  /*
  char buffer[30];
  for (int iii = 0; iii < 30; ++iii) {
    buffer[iii] = 0;
  }
  char *p = &buffer[29];
  */
  char *p = *pp;
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
  
  const  int divisor = 36;
  uint64_t quot;
  uint64_t mod;
  while (pos34) {
    gangsta_div(0, pos34, &quot, &mod);
    //    quot = pos34 / divisor;
    //    mod = pos34 % divisor;
    pos34 = quot;

    pos2 |= mod << 32;
    gangsta_div(0, pos2, &quot, &mod);
    //    quot = pos2 / divisor;
    //    mod = pos2 % divisor;
    pos2 = quot;

    pos1 |= mod << 32;
    gangsta_div(0, pos1, &quot, &mod);
    //    quot = pos1 / divisor;
    //    mod = pos1 % divisor;
    pos1 = quot;
    
    *p = trans_table[mod];
    p -= 1;
  }

  pos1 = (pos2 << 32) | pos1;
  while (pos1) {
    gangsta_div(0, pos1, &quot, &mod);
    //    quot = pos1 / divisor;
    //    mod = pos1 % divisor;
    pos1 = quot;
    
    *p = trans_table[mod];
    p -= 1;
    
  }

  *pp = p + 1;
  //  printf("here: %s\n", p+1);
  assert(strcmp("6h7yzb4z112efh4n44afgst7a", *pp) == 0);
  return 0;
}

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif

/*
 * Function to be called from Python
 */
static PyObject* py_myFunction(PyObject* self, PyObject* args)
{
  char *s = "Hello from C!";
  return Py_BuildValue("s", s);
}

/*
 * Function to be called from Python
 */
unsigned long glob = 0;
static PyObject* py_myFunction2(PyObject* self, PyObject* args)
{
  char buffer[30];
  PyObject *obj;
  PyArg_ParseTuple(args, "O", &obj);
  PyObject *base36 = PyLong_FromLong(36);
  Py_INCREF(base36);
  PyObject *rem = 0;
  PyObject *div = 0;
  div = obj;
  Py_INCREF(div);
  PyObject *mod = 0;
  int i;
  for (i = 0; i < 30; ++i)
    buffer[i] = 0;
  char *p = &buffer[30-1];

  int ib = 0;
  //  while (ib < 5) {
  while (PyLong_AsSsize_t(div) != 0L) {
    PyErr_Clear();
    rem = PyNumber_Divmod(div, base36);      
    Py_DECREF(div);
    div = PyTuple_GetItem(rem, 0);
    Py_INCREF(div);
    mod = PyTuple_GetItem(rem, 1);
    *(--p) = trans_table[PyLong_AsLong(mod)];
    Py_DECREF(rem);
    ib += 1;
    glob += 1;
    //printf("glob %lu\n", glob);
  }
  Py_DECREF(div);
  Py_DECREF(base36);
  Py_DECREF(base36);
  return Py_BuildValue("s", p);
}

/*
 * Another function to be called from Python
 */
static PyObject* py_myOtherFunction(PyObject* self, PyObject* args)
{
  double x, y;
  PyArg_ParseTuple(args, "dd", &x, &y);
  return Py_BuildValue("d", x*y);
}

#if PY_MAJOR_VERSION >= 3
#else
static PyObject* py_myfn0(PyObject* self, PyObject* args)
{
  PyObject *obj;
  PyArg_ParseTuple(args, "O", &obj);
  return PyNumber_ToBase(obj, 36);
}
#endif 


static PyObject* py_myfn(PyObject* self, PyObject* args)
{
  char buffer[30];
  PyObject *obj;
  PyArg_ParseTuple(args, "O", &obj);
  PyObject *base36 = PyLong_FromLong(36);
  PyObject *rem = 0;
  PyObject *div = 0;
  div = obj;
  Py_INCREF(div);
  PyObject *mod = 0;
  int i;
  for (i = 0; i < 30; ++i)
    buffer[i] = 0;
  char *p = &buffer[30-1];
  long long whee_test;
  unsigned long long whee;
  unsigned long long whee_d;
  unsigned long long whee_m;

  while (1) {
    whee_test = PyLong_AsUnsignedLongLong(div);
    if (whee_test != -1L) {
      PyErr_Clear();
      // do it the fast way
      whee = PyLong_AsUnsignedLongLong(div);
      whee_d = whee;
      while (whee_d != 0) {
	whee = whee_d;
	whee_d = whee / 36;
	whee_m = whee % 36;
	*(--p) = trans_table[whee_m];
      }
      break;
    }
    else {
      rem = PyNumber_Divmod(div, base36);
      Py_DECREF(div);
      div = PyTuple_GetItem(rem, 0);
      Py_INCREF(div);
      mod = PyTuple_GetItem(rem, 1);
      *(--p) = trans_table[PyLong_AsLong(mod)];
      Py_DECREF(rem);
    }
  }
  Py_DECREF(base36);
  Py_DECREF(div);
  return Py_BuildValue("s", p);
}

static PyObject* py_myfn2(PyObject* self, PyObject* args)
{
  char buffer[30];
  PyObject *obj;
  PyArg_ParseTuple(args, "O", &obj);
  PyObject *base36 = PyLong_FromLong(36);
  PyObject *rem = 0;
  PyObject *div = 0;
  div = obj;
  Py_INCREF(div);
  PyObject *mod = 0;
  int i;
  for (i = 0; i < 30; ++i)
    buffer[i] = 0;
  char *p = &buffer[30-1];
  while (PyLong_AsSsize_t(div) != 0L) {
    PyErr_Clear();
    rem = PyNumber_Divmod(div, base36);
    Py_DECREF(div);
    div = PyTuple_GetItem(rem, 0);
    Py_INCREF(div);
    mod = PyTuple_GetItem(rem, 1);
    *(--p) = trans_table[PyLong_AsLong(mod)];
    Py_DECREF(rem);
  }
  Py_DECREF(div);
  Py_DECREF(base36);
  return Py_BuildValue("s", p);
}

static PyObject* py_myfn3(PyObject* self, PyObject* args)
{
  PyObject *obj;
  PyArg_ParseTuple(args, "O", &obj);
  unsigned long long low = PyLong_AsUnsignedLongLongMask(obj);
  PyObject *shifted = PyNumber_Rshift(obj, PyLong_FromLong(64));
  unsigned long long high = PyLong_AsUnsignedLongLongMask(shifted);
  Py_DECREF(shifted);

  // upper bound for the buffer will be 30 chars (for now)
  const int sz = 30;
  char buffer[sz];
  //  PyUnicodeObject *str;
  //  str = (PyUnicodeObject *) PyUnicode_FromStringAndSize((char *)0, sz);
  //  if (str == NULL)
  //    return NULL;

  //  char *p = PyUnicode_AS_STRING(str) + sz;
  char *p = buffer + sz - 1;
  *p = '\0';
  p -= 1;

  trike_div(high, low, &p);

  if (p != buffer) {
    char *q = buffer;
    assert(p > q);
    do {
    } while ((*q++ = *p++) != '\0');
    q--;
    //    _PyUnicode_Resize((PyObject **)&str,
    //		     (Py_ssize_t) (q - PyUnicode_AS_STRING(str)));
    //printf("yo2 %s\n", PyString_AS_STRING(str));
  }
  return Py_BuildValue("s", buffer);
}

static PyObject* py_myfn4(PyObject* self, PyObject* args)
{
  PyObject *obj;
  PyArg_ParseTuple(args, "O", &obj);
  unsigned long long low = PyLong_AsUnsignedLongLongMask(obj);
  PyObject *shifted = PyNumber_Rshift(obj, PyLong_FromLong(64));
  unsigned long long high = PyLong_AsUnsignedLongLongMask(shifted);
  Py_DECREF(shifted);

  // upper bound for the buffer will be 30 chars (for now)
  const int sz = 30;
  char buffer[sz];
  //  PyUnicodeObject *str;
  //  str = (PyUnicodeObject *) PyUnicode_FromStringAndSize((char *)0, sz);
  //  if (str == NULL)
  //    return NULL;

  //  char *p = PyUnicode_AS_STRING(str) + sz;
  char *p = buffer + sz - 1;
  *p = '\0';
  p -= 1;

  trike_div2(high, low, &p);

  if (p != buffer) {
    char *q = buffer;
    assert(p > q);
    do {
    } while ((*q++ = *p++) != '\0');
    q--;
    //    _PyUnicode_Resize((PyObject **)&str,
    //		     (Py_ssize_t) (q - PyUnicode_AS_STRING(str)));
    //printf("yo2 %s\n", PyString_AS_STRING(str));
  }
  return Py_BuildValue("s", buffer);
}

#if PY_MAJOR_VERSION >= 3
#else
/*
 * Another function to be called from Python
 */
static PyObject* py_myWowFunction(PyObject* self, PyObject* args)
{
  //  printf("1\n");
  PyObject *obj;
  PyArg_ParseTuple(args, "O", &obj);

  //  printf("3 %d\n", PyNumber_Check(obj));
  //  printf("3 %d\n", PyIndex_Check(obj));
  // TODO: error/sanity checking

  // This will do a base 36 conversion into the alphabet 0123456789abcdefghijklmnopqrstuvwxyz
  // It returns a BASE# leader (e.g. 36#) which needs to be removed
  PyObject *np;
  np = PyNumber_ToBase(obj, 36);

  //  printf("5\n");
  //  printf("5 %d\n", PyUnicode_Check(np));
  //  printf("5 %d\n", PyUnicode_Check(np));
  char const *string_without_leaderC = PyString_AsString(np);
  //  printf("5x %p\n", string_without_leaderC);
  PyObject *np2a = PyString_FromString(&string_without_leaderC[3]);
  return np2a;
}
#endif

/*
 * Bind Python function names to our C functions
 */
static PyMethodDef mymodule_methods[] = {
  {"myFunction", py_myFunction, METH_VARARGS},
  {"myFunction2", py_myFunction2, METH_VARARGS},
  {"myOtherFunction", py_myOtherFunction, METH_VARARGS},
#if PY_MAJOR_VERSION >= 3
#else
  {"myWowFunction", py_myWowFunction, METH_VARARGS},
  {"myfn0", py_myfn0, METH_VARARGS},
#endif
  {"myfn", py_myfn, METH_VARARGS},
  {"myfn2", py_myfn2, METH_VARARGS},
  {"myfn3", py_myfn3, METH_VARARGS},
  {"myfn4", py_myfn4, METH_VARARGS},
  {NULL, NULL}
};

#if PY_MAJOR_VERSION >= 3

static int mymodule_traverse(PyObject *m, visitproc visit, void *arg) {
  Py_VISIT(GETSTATE(m)->error);
  return 0;
}

static int mymodule_clear(PyObject *m) {
  Py_CLEAR(GETSTATE(m)->error);
  return 0;
}

static struct PyModuleDef moduledef = {
  PyModuleDef_HEAD_INIT,
  "mymodule",
  NULL,
  sizeof(struct module_state),
  mymodule_methods,
  NULL,
  mymodule_traverse,
  mymodule_clear,
  NULL
};

#define INITERROR return NULL

PyMODINIT_FUNC
PyInit_mymodule(void)

#else
#define INITERROR return

void
  initmymodule(void)
#endif
{

#if PY_MAJOR_VERSION >= 3
  PyObject *module = PyModule_Create(&moduledef);
#else
  PyObject *module = Py_InitModule("mymodule", mymodule_methods);
#endif

  if (module == NULL)
    INITERROR;
  struct module_state *st = GETSTATE(module);

  st->error = PyErr_NewException("mymodule.Error", NULL, NULL);
  if (st->error == NULL) {
    Py_DECREF(module);
    INITERROR;
  }

#if PY_MAJOR_VERSION >= 3
  return module;
#endif
}
/*
 * Python calls this to let us initialize our module
 */
/*
void initmymodule()
{
  (void) Py_InitModuleOA("mymodule", mymodule_methods);
}
*/
