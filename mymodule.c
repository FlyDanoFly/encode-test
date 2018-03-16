/* https://csl.name/post/c-functions-python/ */

#include <Python.h>

/*
 * Function to be called from Python
 */
static PyObject* py_myFunction(PyObject* self, PyObject* args)
{
  char *s = "Hello from C!";
  return Py_BuildValue("s", s);
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

/*
 * Another function to be called from Python
 */
static PyObject* py_myWowFunction(PyObject* self, PyObject* args)
{
  PyObject *obj;
  long l;
  PyArg_ParseTuple(args, "O", &obj);
  //printf("Hello world\n");
  //printf("Hello world %ld\n", l);
  //printf("Hello world %p\n", obj);
  //printf("Hello world %s\n", Py_TYPE(obj)->tp_name);
  //printf("Hello world %ld\n", Py_TYPE(obj)->tp_basicsize);
  //printf("Hello world %ld\n", Py_TYPE(obj)->tp_itemsize);
  //  printf("Hello world %ld\n", Py_SIZE(obj));
  //  printf("Hello world %ld\n", Py_SIZE(obj));
  //printf("sequence size %ld\n", PySequence_Length(obj));
  //printf("sequence check %d\n", PySequence_Check(obj));
  //printf("iter check %d\n", PyIter_Check(obj));
  //printf("number check %d\n", PyNumber_Check(obj));
  //printf("number long %p\n", Py_TYPE(obj)->tp_as_buffer);
  //printf("yo\n");

  PyObject *np;
  np = PyNumber_ToBase(obj, 36);
  //printf("PyString_Check %d\n", PyString_Check(np));
  //printf("PyString_AsString %s\n", PyString_AsString(np));
  return np;
  //char const *s2;
  //printf(" new base 36 string? %s\n", s2);
  //char *s = "Hello from C!";
  //printf("return;\n");
  //return np;
  //return Py_BuildValue("s", s);
}

/*
 * Bind Python function names to our C functions
 */
static PyMethodDef mymodule_methods[] = {
  {"myFunction", py_myFunction, METH_VARARGS},
  {"myOtherFunction", py_myOtherFunction, METH_VARARGS},
  {"myWowFunction", py_myWowFunction, METH_VARARGS},
  {NULL, NULL}
};

/*
 * Python calls this to let us initialize our module
 */
void initmymodule()
{
  (void) Py_InitModule("mymodule", mymodule_methods);
}
