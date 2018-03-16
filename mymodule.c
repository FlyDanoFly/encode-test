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
  PyArg_ParseTuple(args, "O", &obj);

  // TODO: error/sanity checking

  // This will do a base 36 conversion into the alphabet 0123456789abcdefghijklmnopqrstuvwxyz
  // It returns a BASE# leader (e.g. 36#) which needs to be removed
  PyObject *np;
  np = PyNumber_ToBase(obj, 36);

  // Remove the leader
  char const *string_without_leader = &PyString_AS_STRING(np)[3];
  PyObject *np2;
  np2 = PyString_FromString(&PyString_AS_STRING(np)[3]);
  return np2;
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
