#include <python2.7/Python.h>
#include <python2.7/structmember.h>
#include <stdbool.h>

typedef struct rbnode{
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
    bool repeat;
}PyRBTree;

typedef struct stack{
    void **head;
    long size;
}stack;

typedef struct {
    PyObject_HEAD
    stack *s;
}rbtreeiterobj;

static const int SMALL_REQUEST_THRESHOLD = 512;
static const int CAPACITY = 512/sizeof(void *) - 1;

static void push(stack *s, void *p){
    void **link = s->head;
    int count = s->size/CAPACITY;
    int shift = s->size%CAPACITY;
    if (!shift){
        for (;count>1; count--){
            link = *(link+CAPACITY);
        }
        if (count == 1){
            *(link+CAPACITY) = (void **)PyObject_Malloc(SMALL_REQUEST_THRESHOLD);
        }else{
            s->head = (void **)PyObject_Malloc(SMALL_REQUEST_THRESHOLD);
        }
    }
    link = s->head;
    count = s->size/CAPACITY;
    for (;count>0; count--){
        link = *(link+CAPACITY);
    }
    *(link+shift) = p;
    s->size++;
}

static void *pop(stack *s){
    void * r = NULL;
    assert(s->size);
    s->size--;

    int shift = s->size%CAPACITY;
    int count = s->size/CAPACITY;
    void **link = s->head;
    for (;count>0; count--){
        link = *(link+CAPACITY);
    }
    r = *(link+shift);
    if (!shift){
        PyObject_Free(link);
    }
    return r;
}


rbnode nil;
static rbnode * Nil;
static PyObject *g_ID;
static PyObject *g_LEFT;
static PyObject *g_RIGHT;
static PyObject *g_KEY;
static PyObject *g_BLACK;
static PyObject *g_VALUE;
static PyObject *RBTreeError;

static PyObject *make_debug_info(rbnode *n){
    PyObject *i = PyDict_New();
    PyObject *x = PyLong_FromLong((long)n);
    PyDict_SetItem(i, g_ID, x);
    PyDict_SetItem(i, g_LEFT, n->left?PyLong_FromLong((long)n->left):Py_None);
    PyDict_SetItem(i, g_RIGHT, n->right?PyLong_FromLong((long)n->right):Py_None);
    PyDict_SetItem(i, g_BLACK, n->black?Py_True:Py_False);
    PyDict_SetItem(i, g_KEY, n->key?n->key:Py_None);
    PyDict_SetItem(i, g_VALUE, n->value?n->value:Py_None);
    return i;
}

static PyObject *debug(PyObject *self, PyObject *args)
{
    PyRBTree *t = (PyRBTree *)self;
    PyObject *list = PyList_New(0);
    stack  s = {NULL, 0};
    rbnode *n = t->root;
    while(s.size > 0 || n != Nil){
        while (n != Nil){
            push(&s, n);
            n = n->left;
        }
        n = pop(&s);
        PyList_Append(list, make_debug_info(n));
        n = n->right;
    }
    PyList_Append(list, make_debug_info(Nil));
    return list;
}

static PyObject *peekmin(PyObject *self, PyObject *args)
{
	rbnode * n = ((PyRBTree *)self)->root;
    if (n == Nil){
        return Py_None;
    }
    while (n->left != Nil){
        n = n->left;
    }
    return PyTuple_Pack(2,n->key,n->value);
}

static PyObject *peekmax(PyObject *self, PyObject *args)
{
	rbnode * n = ((PyRBTree *)self)->root;
    if (n == Nil){
        return Py_None;
    }
    while (n->right != Nil){
        n = n->right;
    }
    return PyTuple_Pack(2,n->key,n->value);
}

static int remove_node(PyObject*, PyObject*, rbnode*);

static PyObject *popmin(PyObject *self, PyObject *args)
{
	rbnode * n = ((PyRBTree *)self)->root;
    if (n == Nil){
        return Py_None;
    }
    while (n->left != Nil){
        n = n->left;
    }
    PyObject * r = PyTuple_Pack(2,n->key,n->value);
    remove_node(self, NULL, n);
    return r;
}

static PyObject *popmax(PyObject *self, PyObject *args)
{
	rbnode * n = ((PyRBTree *)self)->root;
    if (n == Nil){
        return Py_None;
    }
    while (n->right != Nil){
        n = n->right;
    }
    PyObject * r = PyTuple_Pack(2,n->key,n->value);
    remove_node(self, NULL, n);
    return r;
}

static PyMethodDef TreeMethods[] = {
    {"debug",  debug, METH_VARARGS, "debug rbtree"},
    {"peekmin",  peekmin, METH_VARARGS, "peek min key value"},
    {"peekmax",  peekmax, METH_VARARGS, "peek max key value"},
    {"popmin",  popmin, METH_VARARGS, "pop min key value as tuple"},
    {"popmax",  popmax, METH_VARARGS, "pop max key value as tuple"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};


static PyMemberDef TreeMembers[] = {
    {"repeat",  T_BOOL, offsetof(PyRBTree,repeat), RESTRICTED|READONLY},
    {NULL}  /* Sentinel */
};

static PyObject *
PyRBTree_New(PyTypeObject* type, PyObject* args, PyObject* kw)
{
	PyRBTree *self;
    PyObject *repeat = NULL;
    if (!PyArg_ParseTuple(args, "|O:ref", &repeat)){
        return NULL;
    }
	self = (PyRBTree *)type->tp_alloc(type, 0);
    if (!self){
        return (PyObject*)self;
    }
    self->root = Nil;
    if (!repeat || repeat == Py_False){
        self->repeat = false;
    }else{
        self->repeat = true;
    }
	return (PyObject *)self;
}

static void rotate_left(PyRBTree *t, rbnode *n){
	rbnode *p = n->parent;
	rbnode *r = n->right;
	assert(r != Nil);
	rbnode *rl = r->left;

	if (!p){
		t->root = r;
	}else if(p->left == n){
		p->left = r;
	}else{
		p->right = r;
	}
	r->parent = p;
	r->left = n;
	n->parent = r;
	n->right = rl;
	rl->parent = n;
}

static void rotate_right(PyRBTree *t, rbnode *n){
	rbnode *p = n->parent;
	rbnode *l = n->left;
	assert(l != Nil);
	rbnode *lr = l->right;

	if (!p){
		t->root = l;
	}else if(p->left == n){
		p->left = l;
	}else{
		p->right = l;
	}
	l->parent = p;
	n->parent = l;
	l->right = n;
	n->left = lr;
	lr->parent = n;
}

static int compare(PyObject*, PyObject*);
static rbnode *find_node(PyObject *t, PyObject *v){
    if (((PyRBTree *)t)->repeat){
		PyErr_SetString(RBTreeError, "not support this operation when rbtree repeat on");
        return NULL;
    }
	rbnode * n = ((PyRBTree *)t)->root;
	rbnode * f = NULL;
    int result = 0;
	while (n!=Nil){
        result = compare(v, n->key);
        if (result == 0){
            break;
        }else if (result == 1 || result == -1){
            n = result == 1 ? n->right:n->left;
        }else{
            return NULL;
        }
	}
	if (n == Nil){
		PyErr_SetString(RBTreeError, "not exists");
		return NULL;
	}
    return n;
}

static void balance_tree_remove(PyObject *self, rbnode *node){
    PyRBTree *t = (PyRBTree *)self;
    rbnode *n = node;
    rbnode *p = NULL;
    rbnode *s = NULL;
    while (1){
        if (!n->parent){
            break;
        }
        p = n->parent;
        s = n == p->left ? p->right: p->left;
        if (!s->black){
            if (p->left == n){
                rotate_left(t, p);
            }else{
                rotate_right(t, p);
            }
            p->black = 0;
            s->black = 1;
            continue;
        }

        if (s->left->black && s->right->black && p->black){
            s->black = 0;
            n = p;
            continue;
        }

        if (!p->black && s->right->black && s->left->black){
            p->black = 1;
            s->black = 0;
            break;
        }
        
        if (p->right == s && s->right->black){
            s->left->black =1;
            s->black = 0;
            rotate_right(t, s);
        }else if (p->left == s && s->left->black){
            s->right->black =1;
            s->black = 0;
            rotate_left(t, s);
        }
        s = n == p->left ? p->right: p->left;

        s->black = p->black;
        p->black = 1;
        if (p->right == s){
            s->right->black = 1;
            rotate_left(t, p);
        }else{
            s->left->black = 1;
            rotate_right(t, p);
        }
        break;
    }
}

static int remove_node(PyObject *self, PyObject *v, rbnode *t){
    if (((PyRBTree *)self)->repeat){
		PyErr_SetString(RBTreeError, "not support this operation when rbtree repeat on");
        return -1;
    }
    rbnode *target = NULL;
    if (!t){
        target = find_node(self, v);
    }else{
        target  = t;
    }
    if (!target){
        return -1;
    }
    if (target->left != Nil){
        rbnode *max = target->left;
        while (max->right != Nil){
            max = max->right;
        }
        Py_DECREF(target->key);
        Py_DECREF(target->value);
        target->key = max->key;
        target->value = max->value;
        target = max;
    }
    rbnode * n = target;
	rbnode * c = n->left == Nil ? n->right: n->left;
	rbnode * p = n->parent;
    if (!p){
        ((PyRBTree*)self)->root = c;
    }else if (p->left == n){
		p->left = c;
	}else{
		p->right = c;
	}
	c->parent = p;

	if (!n->black){
	}else if(!c->black){
		c->black = 1;
	}else{
        balance_tree_remove(self, c);
	}
	PyObject_Free(n);
	return 0;
}

static int compare(PyObject *a, PyObject *b){
    int result = PyObject_RichCompareBool(a, b, Py_EQ);
    if (result < 0){
        return 0xFF;
    }
    if (result){
        return 0;
    }
    result = PyObject_RichCompareBool(a, b, Py_LT);
    if (result < 0){
        return 0xFF;
    }
    if (result){
        return -1;
    }
    return 1;
}

static rbnode *make_node(PyObject *v, PyObject *w){
    rbnode *x = PyObject_Malloc(sizeof(rbnode));
    if (!x){
        return NULL;
    }
    ((PyObject *)x)->ob_refcnt = 0;
    Py_INCREF(v);
    Py_INCREF(w);
    x->key = v;
    x->value = w;
	x->parent = NULL;
	x->left = Nil;
	x->right = Nil;
	x->black = 0;
    return x;
}

static int add_node(PyObject *self, PyObject *v, PyObject *w){
    PyRBTree *t = (PyRBTree *)self;
    rbnode *x = NULL;
	if (!t->root || t->root == Nil){
        x = make_node(v, w);
        if (!x){
            return -1;
        }
		t->root = x;
		x->black = 1;
		return 0;
	}

    int result = 0;
	rbnode *n = t->root;
	while (n != Nil){
        result = compare(v, n->key);
        if (result == 1){
            if (n->right == Nil){
                x = make_node(v, w);
                x->parent = n;
                n->right = x;
                break;
            }
            n = n->right;
        }else if (result == -1 || (t->repeat && result == 0)){
            if (n->left == Nil){
                x = make_node(v, w);
                x->parent = n;
                n->left = x;
                break;
            }
            n = n->left;
        }else{
            Py_DECREF(n->key);
            Py_DECREF(n->value);
            x = make_node(v, w);
            n->key = x->key;
            n->value = x->value;
            return 0;
        }
	}
	n = x;
	rbnode *p = NULL;
	rbnode *g = NULL;
	rbnode *u = NULL;
	while (1){
		p = n->parent;
		if (!p){
			n->black = 1;
			break;
		}
		if (p->black){
			break;
		}
		g = p->parent;
		u = g->left == p ? g->right: g->left;
		if (!u->black){
			p->black = 1;
			u->black = 1;
			g->black = 0;
			n = g;
			continue;
		}
		if (g->left == p && p->right == n){
			rotate_left(t, p);
			n = p;
		}else if (g->right ==p && p->left == n){
			rotate_right(t, p);
			n = p;
		}
		p = n->parent;
		g = p->parent;
		u = g->left == p ? g->right: g->left;
		if (g->left == p){
			rotate_right(t, g);
			p->black = 1;
			g->black = 0;
		}else{
			rotate_left(t, g);
			p->black = 1;
			g->black = 0;
		}
		break;
	}
    return 0;
}


static PyObject *
rbtreeiter_next(PyObject *t)
{
    rbtreeiterobj *it = (rbtreeiterobj*)t;
    if (it->s->size <= 0){
        return NULL;
    }
    rbnode *n = pop(it->s);
	rbnode *r = n;
    n = n->right;
    if (n == Nil){
        return PyTuple_Pack(2, r->key, r->value);
    }
    while(n != Nil){
        push(it->s, n);
        n = n->left;
    }
    return PyTuple_Pack(2, r->key, r->value);
}

static int dict_ass_sub(PyObject *self, PyObject *v, PyObject *w)
{
	if (w == NULL)
		return remove_node(self, v, NULL);
    return add_node(self, v, w);
}

static PyObject *tree_sub(PyObject *self, PyObject *k)
{
    rbnode *target = find_node(self, k);
    if (!target){
        return NULL;
    }
    Py_INCREF(target->value);
    return target->value;
}

static PyMappingMethods tree_as_mapping = {
	NULL,
	tree_sub,                    /*mp_subscript*/
	(objobjargproc)dict_ass_sub, /*mp_ass_subscript*/
};

PyTypeObject PyRBTreeIter_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "rbtreeiterator",                           /* tp_name */
    sizeof(rbtreeiterobj),                      /* tp_basicsize */
    0,                                          /* tp_itemsize */
    /* methods */
    0, //(destructor)listiter_dealloc,          /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_compare */
    0,                                          /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    PyObject_GenericGetAttr,                    /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /* tp_flags */
    0,                                          /* tp_doc */
    0, //(traverseproc)listiter_traverse,       /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    PyObject_SelfIter,                          /* tp_iter */
    (iternextfunc)rbtreeiter_next,              /* tp_iternext */
    0,                                          /* tp_methods */
    0,                                          /* tp_members */
};

static PyObject * tree_iter(PyObject*);

static PyTypeObject RBTreeType = {
    PyObject_HEAD_INIT(NULL)
    0,                                // ob_size
    "rbtree.RBTree",                  // tp_name
    sizeof(PyRBTree),                 // tp_basicsize
    0,                                // tp_itemsize
    0,                                // tp_dealloc
    0,                                // tp_print
    0,                                // tp_getattr
    0,                                // tp_setattr
    0,                                // tp_compare
    0,                                // tp_repr
    0,                                // tp_as_number
    0,                                // tp_as_sequence
    &tree_as_mapping,                 // tp_as_mapping
    0,                                // tp_hash
    0,                                // tp_call
    0,                                // tp_str
    0,                                // tp_getattro
    0,                                // tp_setattro
    0,                                // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,    // tp_flags
    0,                                // tp_doc
    0,                                // tp_traverse
    0,                                // tp_clear
    0,                                // tp_richcompare
    0,                                // tp_weaklistoffset
    tree_iter,                        // tp_iter
    0,                                // tp_iternext
    TreeMethods,                      // tp_methods
    TreeMembers,                      // tp_members
    0,                                // tp_getset
    0,                                // tp_base
    0,                                // tp_dict
    0,                                // tp_descr_get
    0,                                // tp_descr_set
    0,                                // tp_dictoffset
    0,                                // tp_init
    0,                                // tp_alloc
    PyRBTree_New,                     // tp_new
};

static PyObject *
tree_iter(PyObject *t)
{
    rbtreeiterobj *it;
    if (PyObject_Type(t) != (PyObject*)&RBTreeType) {
        PyErr_BadInternalCall();
        return NULL;
    }

	it = (rbtreeiterobj *)PyRBTreeIter_Type.tp_alloc(&PyRBTreeIter_Type, 0);
    if (it == NULL)
        return NULL;
    it->s = PyObject_Malloc(sizeof(stack));
    it->s->size = 0;
    it->s->head = NULL;
    rbnode *n = ((PyRBTree *)t)->root;
    while (n != Nil){
        push(it->s, n);
        n = n->left;
    }
    return (PyObject *)it;
}

PyMODINIT_FUNC initrbtree(void)
{
    PyObject *m = Py_InitModule3("rbtree", NULL, "rbtree module");
    if (m == NULL)
        return;
    if (PyType_Ready(&RBTreeType) < 0)
        return;
    Py_INCREF(&RBTreeType);
    if (PyType_Ready(&PyRBTreeIter_Type) < 0)
        return;
    Py_INCREF(&PyRBTreeIter_Type);
    PyModule_AddObject(m, "RBTree", (PyObject*)&RBTreeType);
    RBTreeError = PyErr_NewException("rbtree.error", NULL, NULL);
    Py_INCREF(RBTreeError);
    PyModule_AddObject(m, "error", RBTreeError);

	Nil = &nil;
	Nil->black = 1;
    g_BLACK = PyString_FromString("black");
    g_ID = PyString_FromString("id");
    g_KEY = PyString_FromString("key");
    g_VALUE = PyString_FromString("value");
    g_LEFT = PyString_FromString("left");
    g_RIGHT = PyString_FromString("right");
}

