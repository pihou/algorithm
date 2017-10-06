#include <python2.7/Python.h>
#include <python2.7/structmember.h>

static PyObject *RBTreeError;

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
} PyRBTree;

rbnode nil;
rbnode * Nil;

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
        op->root = Nil;
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

static void rotate_left(PyRBTree *t, rbnode *n){
	rbnode *p = n->parent;
	rbnode *l = n->left;
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
	rbnode *r = n->right;
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

static void replace_node(PyRBTree *t, rbnode *n, rbnode *o){
	rbnode *p = o->parent;
	if (!p){
		t->root = n;
		p->left = n;
	}else{
		p->right = n;
	}
	n->parent = p;
	PyObject_Free(o);
}

static int
del_node(PyObject *self, PyObject *v){
    PyRBTree * t = (PyRBTree *)self;
	rbnode * n = t->root;
	rbnode * r = NULL;
	long h = PyObject_Hash(v);
	while (n!=Nil){
		long hash = PyObject_Hash(n->key);
		if (h == hash){
			break;
		}
		if (h > hash){
			n = n->right;
		}else{
			n = n->left;
		}
	}
	if (n == Nil){
		PyErr_SetString(RBTreeError, "not exists");
		return -1;
	}
	rbnode * d = n;
	rbnode * f = NULL;
	if (d->left == Nil && d->right == Nil){
		if (!d->parent){
			t->root = Nil;
		}else if (d->parent->left == d){
			d->parent->left = Nil;
		}else{
			d->parent->right = Nil;
		}
		PyObject_Free(d);
		return 0;
	}else if (d->left == Nil || d->right == Nil){
		f = d->left == Nil ? d->right: d->left;
		d->key = f->key;
		d->value = f->value;
	}else{
		f = d->left;
		while (f->right != Nil)
			f = f->right;
		d->key = f->key;
		d->value = f->value;
	}
	n = f;
	rbnode * c = n->left == Nil ? n->right: n->left;
	rbnode * p = n->parent;
	if (p->left == n){
		p->left = c;
	}else{
		p->right = c;
	}
	c->parent = p;
	if (!n->black){
	}else if(!c->black){
		c->black = 1;
	}else{
		n = c;
		rbnode * p = NULL;
		rbnode * s = NULL;
		while (n != Nil){
			if (!n->parent){
				break;
			}
			p = n->parent;
			s = n == p->left ? p->right: p->left;
			if (!s->black){
				if (p->left == n){
					rotate_right(t, p);
				}else{
					rotate_left(t, p);
				}
				p->black = 0;
				s->black = 1;
				break;
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

			if (p->right == s){
				s->black = p->black;
				p->black = 1;
				rotate_right(t, p);
			}else{
				s->black = p->black;
				p->black = 1;
				rotate_left(t, p);
			}
			break;
		}
	}
	PyObject_Free(f);
	return 0;
}

static int
dict_ass_sub(PyObject *self, PyObject *v, PyObject *w)
{
	if (w == NULL)
		return del_node(self, v);
    rbnode *x = PyObject_Malloc(sizeof(rbnode));
    if (!x){
        return -1;
    }
    Py_INCREF(v);
    Py_INCREF(w);
    x->key = v;
    x->value = w;
	x->parent = NULL;
	x->left = Nil;
	x->right = Nil;
	x->black = 0;
    PyRBTree * t = (PyRBTree *)self;
	if (!t->root || t->root == Nil){
		t->root = x;
		x->black = 1;
		return 0;
	}
	long hash = PyObject_Hash(v);
	rbnode *n = t->root;
	while (n != Nil){
		long h = PyObject_Hash(n->key);
		if (hash > h){
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
dict_sub(PyObject *self, PyObject *k)
{
    PyRBTree * t = (PyRBTree *)self;
	rbnode * n = t->root;
	long h = PyObject_Hash(k);
	while (n!=Nil){
		long hash = PyObject_Hash(n->key);
		if (h == hash){
			return n->value;
		}
		if (h > hash){
			n = n->right;
		}else{
			n = n->left;
		}
	}
	PyErr_SetString(RBTreeError, "not exists");
	return NULL;
}

static PyMappingMethods tree_as_mapping = {
	NULL,
	dict_sub, /*mp_subscript*/
	(objobjargproc)dict_ass_sub, /*mp_ass_subscript*/
};

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
		&tree_as_mapping,               /* tp_as_mapping */
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
	nil.black = 1;
	Nil = &nil;
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

