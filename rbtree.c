#include <python2.7/Python.h>
#include <python2.7/structmember.h>

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
static PyObject *g_id;
static PyObject *g_left;
static PyObject *g_right;
static PyObject *g_key;
static PyObject *g_black;
static PyObject *g_value;
static PyObject *RBTreeError;

static PyObject *make_debug_info(rbnode *n){
    PyObject *i = PyDict_New();
    PyObject *x = PyLong_FromLong((long)n);
    PyDict_SetItem(i, g_id, x);
    PyDict_SetItem(i, g_left, n->left?PyLong_FromLong((long)n->left):Py_None);
    PyDict_SetItem(i, g_right, n->right?PyLong_FromLong((long)n->right):Py_None);
    PyDict_SetItem(i, g_black, n->black?Py_True:Py_False);
    PyDict_SetItem(i, g_key, n->key?n->key:Py_None);
    PyDict_SetItem(i, g_value, n->value?n->value:Py_None);
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

static PyMethodDef TreeMethods[] = {
    {"debug",  debug, METH_VARARGS, "debug rbtree"},
    //{"popmin",  debug, METH_VARARGS, "debug rbtree"},
    //{"popmax",  debug, METH_VARARGS, "debug rbtree"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};


static PyMemberDef TreeMembers[] = {
    {NULL}  /* Sentinel */
};

static PyObject *
PyRBTree_New(PyTypeObject* type, PyObject* args, PyObject* kw)
{
	PyRBTree *self;
	self = (PyRBTree *)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->root = Nil;
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

static rbnode *find_node(PyObject *t, PyObject *v){
	rbnode * n = ((PyRBTree *)t)->root;
	rbnode * f = NULL;
    int compare = 0;
	while (n!=Nil){
        compare = PyObject_RichCompareBool(n->key, v, Py_EQ);
        if (compare < 0)
            return NULL;
        if (compare > 0)
            break;
        compare = PyObject_RichCompareBool(n->key, v, Py_LT);
        if (compare < 0)
            return NULL;
        n = compare ? n->right:n->left;
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

static int remove_node(PyObject *self, PyObject *v){
	rbnode *target = find_node(self, v);
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

static int add_node(PyObject *self, PyObject *v, PyObject *w){
    PyRBTree *t = (PyRBTree *)self;
    rbnode *x = PyObject_Malloc(sizeof(rbnode));
    if (!x){
        return -1;
    }
    ((PyObject *)x)->ob_refcnt = 1;
    Py_INCREF(v);
    Py_INCREF(w);
    x->key = v;
    x->value = w;
	x->parent = NULL;
	x->left = Nil;
	x->right = Nil;
	x->black = 0;
	if (!t->root || t->root == Nil){
		t->root = x;
		x->black = 1;
		return 0;
	}
    int compare = 0;
	rbnode *n = t->root;
	while (n != Nil){
        compare = PyObject_RichCompareBool(n->key, v, Py_EQ);
        if (compare < 0){
            Py_DECREF(v);
            Py_DECREF(w);
            PyObject_Free(x);
            return compare;
        }
        if (compare){
            Py_DECREF(n->value);
            Py_DECREF(v);
            n->value = w;
            PyObject_Free(x);
            return 0;
        }
        compare = PyObject_RichCompareBool(n->key, v, Py_LT);
        if (compare < 0){
            Py_DECREF(v);
            Py_DECREF(w);
            PyObject_Free(x);
            return compare;
        }
		if (compare){
			if (n->right == Nil){
				x->parent = n;
				n->right = x;
				break;
			}
			n = n->right;
		}else{
			if (n->left == Nil){
				x->parent = n;
				n->left = x;
				break;
			}
			n = n->left;
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
    PyObject *key = n->key;
    Py_INCREF(key);
    n = n->right;
    if (n == Nil){
        return key;
    }
    while(n != Nil){
        push(it->s, n);
        n = n->left;
    }
    return key;
}

static int dict_ass_sub(PyObject *self, PyObject *v, PyObject *w)
{
	if (w == NULL)
		return remove_node(self, v);
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

static PyObject * list_iter(PyObject*);

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
    list_iter,                        // tp_iter
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
list_iter(PyObject *t)
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
    g_black = PyString_FromString("black");
    g_id = PyString_FromString("id");
    g_key = PyString_FromString("key");
    g_value = PyString_FromString("value");
    g_left = PyString_FromString("left");
    g_right = PyString_FromString("right");
}

