#include "Python.h"
#include "structmember.h"

static PyObject *RBTreeError;

typedef struct{
    PyObject *key;
    PyObject *value;
    int black;
    struct rbnode *parent;
    struct rbnode *left;
    struct rbnode *right;
}rbnode;

typedef struct {
    PyObject_HEAD
    rbnode *root;
} PyRBTree;

static PyObject *setitem(PyObject *self, PyObject *args)
{
    PyObject *key = NULL;
    PyObject *value = NULL;
    if (!PyArg_UnpackTuple(args, "ref", 2, 2, &key, &value))
        return PyLong_FromLong(3);
    rbnode *n = PyObject_Malloc(sizeof(rbnode));
    if (!n){
        return PyLong_FromLong(1);
    }
    n->key = key;
    n->value = value;
    PyRBTree * t = (PyRBTree *)self;
    t->root = n;
    return PyLong_FromLong(1);
}

static PyObject *pop(PyObject *self, PyObject *args)
{
    //PyRBTree * t = (PyRBTree *)self;
    //PyObject * data = t->root->data;
    //PyObject_Free(t->root);
    return NULL;
}

static PyMethodDef TreeMethods[] = {
    {"__setitem__",  setitem, METH_VARARGS, "add a node in rbtree"},
    {"pop",  pop, METH_VARARGS, "pop node in rbtree"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyTypeObject *RBTreeType = NULL;

static PyMemberDef TreeMembers[] = {
    {NULL}  /* Sentinel */
};

static PyObject *
PyRBTree_New()
{
    PyRBTree *op = PyObject_GC_New(PyRBTree, RBTreeType);
    if (op != NULL) {
        op->root = NULL;
    }
    else
        return NULL;
    Py_INCREF(op);
    _PyObject_GC_TRACK(op);
    return (PyObject *)op;
}

static PyObject *
PyRBTree_new(PyTypeObject* type, PyObject* args, PyObject* kw)
{
    PyObject *newfunc;
    newfunc = (PyObject *)PyRBTree_New();
    if (newfunc == NULL)
        return NULL;
    return (PyObject *)newfunc;
}

static PyObject *
rbtree_iter(PyObject *seq)
{
    //PyTypeObject * type = NULL;
    //PyRBTree *a = (rbtree *)seq;
    //type = (PyTypeObject *)PyObject_Type(a->list);
    return PyLong_FromLong(1);
    //return type->tp_iter(a->list);
}

PyMODINIT_FUNC inittree(void)
{
    static PyTypeObject _TreeType = {
        //PyVarObject_HEAD_INIT(NULL, 0)
        PyObject_HEAD_INIT(NULL)
        0,                                // ob_size
        "tree.RBTree",                // tp_name
        sizeof(PyRBTree),                // tp_basicsize
        0,                                // tp_itemsize
        0,        // tp_dealloc
        0,                                // tp_print
        0,                                // tp_getattr
        0,                                // tp_setattr
        0,                                // tp_compare
        0,            // tp_repr
        0,                                // tp_as_number
        0,                                // tp_as_sequence
        0,                                // tp_as_mapping
        0,                                // tp_hash
        0,                                // tp_call
        0,            // tp_str
        0,                                // tp_getattro
        0,                                // tp_setattro
        0,                                // tp_as_buffer
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE ,    // tp_flags
        0,                    // tp_doc
        0,                                // tp_traverse
        0,                                // tp_clear
        0,                                // tp_richcompare
        0,                                // tp_weaklistoffset
        rbtree_iter,                          /* tp_iter */
        0,                                // tp_iternext
        TreeMethods,                    // tp_methods
        TreeMembers,        /* tp_members */
        0,                // tp_getset
        0,                                // tp_base
        0,                                // tp_dict
        0,                                // tp_descr_get
        0,                                // tp_descr_set
        0,                                // tp_dictoffset
        0,            // tp_init
        0,                                // tp_alloc
        PyRBTree_new,                // tp_new
    };

    PyObject *m;

    m = Py_InitModule("tree", NULL);
    if (m == NULL)
        return;
    if (PyType_Ready(&_TreeType) < 0)
        return;
    RBTreeType = &_TreeType;
    Py_INCREF(RBTreeType);
    PyModule_AddObject(m, "RBTree", (PyObject*)RBTreeType);
    RBTreeError = PyErr_NewException("tree.error", NULL, NULL);
    Py_INCREF(RBTreeError);
    PyModule_AddObject(m, "error", RBTreeError);
}

