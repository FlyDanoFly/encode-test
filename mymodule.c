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


uint32_t my__div64_32(uint64_t *n, uint32_t base)
{
  uint64_t rem = *n;
  uint64_t b = base;
  uint64_t res, d = 1;
  uint32_t high = rem >> 32;
  
  /* Reduce the thing a bit first */
  res = 0;
  if (high >= base) {
    high /= base;
    res = (uint64_t) high << 32;
    rem -= (uint64_t) (high*base) << 32;
  }
  
  while ((int64_t)b > 0 && b < rem) {
    b = b+b;
    d = d+d;
  }
  
  do {
    if (rem >= b) {
      rem -= b;
      res += d;
    }
    b >>= 1;
    d >>= 1;
  } while (d);
  
  *n = res;
  return rem;
}

int trike_div2(uint64_t high, uint64_t low, char **pp) {
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
    mod = my__div64_32(&pos34, 36);

    pos2 |= mod << 32;
    mod = my__div64_32(&pos2, 36);

    pos1 |= mod << 32;
    mod = my__div64_32(&pos1, 36);
    
    *p = trans_table[mod];
    p -= 1;
  }

  pos1 = (pos2 << 32) | pos1;
  while (pos1) {
    mod = my__div64_32(&pos1, 36);
    
    *p = trans_table[mod];
    p -= 1;    
  }

  *pp = p+1;
  return 0;
}

int trike_div(uint64_t high, uint64_t low, char **pp) {
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
  return 0;
}


#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif

// ERROR with 0 as the int
static PyObject* py_myfn3(PyObject* self, PyObject* args)
{
  PyObject *obj;
  PyArg_ParseTuple(args, "O", &obj);
  unsigned long long low = PyLong_AsUnsignedLongLongMask(obj);
  PyObject *shifted = PyNumber_Rshift(obj, PyLong_FromLong(64));
  unsigned long long high = PyLong_AsUnsignedLongLongMask(shifted);
  Py_DECREF(shifted);

  // The biggest buffer needed to base36 encode a 128 bit int is 26 bytes = 25 bytes + 1 terminal
  const int sz = 26;
  char buffer[sz];
  char *p = buffer + sz - 1;
  *p = '\0';
  p -= 1;

  // TODO: what's the better convention about passing in a string?
  // TODO: probably pass in buffer (& a size?), return a pointer to the first letter in the buffer or NULL on problem?
  // TODO: that would remove all the string processing out of here and into there
  trike_div(high, low, &p);

  // TODO, this isn't necessary, just use p if it is returned from the buffer properly
  if (p != buffer) {
    char *q = buffer;
    assert(p > q);
    do {
    } while ((*q++ = *p++) != '\0');
    q--;
  }
  return Py_BuildValue("s", buffer);
}

// ERROR with 0 as the int
static PyObject* py_myfn4(PyObject* self, PyObject* args)
{
  PyObject *obj;
  PyArg_ParseTuple(args, "O", &obj);
  unsigned long long low = PyLong_AsUnsignedLongLongMask(obj);
  PyObject *shifted = PyNumber_Rshift(obj, PyLong_FromLong(64));
  unsigned long long high = PyLong_AsUnsignedLongLongMask(shifted);
  Py_DECREF(shifted);

  // The biggest buffer needed to base36 encode a 128 bit int is 26 bytes = 25 bytes + 1 terminal
  const int sz = 26;
  char buffer[sz];
  char *p = buffer + sz - 1;
  *p = '\0';
  p -= 1;

  // TODO: what's the better convention about passing in a string?
  // TODO: probably pass in buffer (& a size?), return a pointer to the first letter in the buffer or NULL on problem?
  // TODO: that would remove all the string processing out of here and into there
  trike_div(high, low, &p);

  // TODO, this isn't necessary, just use p if it is returned from the buffer properly
  if (p != buffer) {
    char *q = buffer;
    assert(p > q);
    do {
    } while ((*q++ = *p++) != '\0');
    q--;
  }
  return Py_BuildValue("s", buffer);
}

/*
 * Bind Python function names to our C functions
 */
static PyMethodDef mymodule_methods[] = {
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
