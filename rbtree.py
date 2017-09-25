
import queue
import random

class Node(object):
    def __init__(self, value, nil=False):
        self.left = None if nil else Nil
        self.right = None if nil else Nil
        self.parent = None
        self.black = True
        self.value = value

    def get_parent(self):
        return self.parent

    def get_sibling(self):
        parent = self.parent
        if not parent:
            return None
        if parent.left is self:
            return parent.right
        return parent.left

    def get_grandparent(self):
        parent = self.parent
        if not parent:
            return None
        return parent.parent

    def get_colour(self):
        if self.black:
            return "black"
        return "red"

    def get_uncle(self):
        parent = self.parent
        if not parent:
            return None
        if not parent.parent:
            return None
        if parent is parent.parent.left:
            return parent.parent.right
        return parent.parent.left

    def __str__(self):
        p = self.parent.value if self.parent else None
        c = "black" if self.black else "red"
        return "parent: %s, self:%s %s"%(p, self.value, c)

    def __repr__(self):
        return str(self)

Nil = Node(None, True)


class RBTree(object):
    def __init__(self):
        self.root = Nil

    def rotate_left(self, n):
        p = n.get_parent()
        r = n.right
        assert(not r is Nil)

        n.right = r.left
        if not r.left is Nil:
            r.left.parent = n
        n.parent = r
        r.left = n
        r.parent = p
        if not p:
            self.root = r
        elif p.left is n:
            p.left = r
        else:
            p.right = r

    def rotate_right(self, n):
        p = n.get_parent()
        l = n.left
        assert(not l is Nil)

        n.left = l.right
        if not l.right is Nil:
            l.right.parent = n
        n.parent = l
        l.right = n
        l.parent = p
        if not p:
            self.root = l
        elif p.left is n:
            p.left = l
        else:
            p.right = l

#---------------------------------------------------------
    def _push_node(self, node):
        n = self.root
        if n is Nil:
            self.root = node
            return
        while True:
            if node.value <= n.value:
                if n.left is Nil:
                    n.left = node
                    node.parent = n 
                    break
                n = n.left
            else:
                if n.right is Nil:
                    n.right = node
                    node.parent = n 
                    break
                n = n.right

    def _balance_tree(self, node):
        def case1(n):
            if not n.parent:
                n.black = True
                return
            case2(n)

        def case2(n):
            if n.parent.black:
                return
            case3(n)

        def case3(n):
            g = n.get_grandparent()
            p = n.parent
            u = n.get_uncle()
            if not u.black:
                u.black = True
                p.black = True
                g.black = False
                case1(g)
                return
            case4(n)

        def case4(n):
            p = n.get_parent()
            g = n.get_grandparent()
            if g.left is p and p.right is n:
                self.rotate_left(p)
                n = p
            elif g.right is p and p.left is n:
                self.rotate_right(p)
                n = p
            case5(n)

        def case5(n):
            g = n.get_grandparent()
            p = n.parent
            if g.left is p:
                self.rotate_right(g)
            else:
                self.rotate_left(g)
            p.black = True
            g.black = False

        case1(node)

    def add(self, node):
        node.black = False
        self._push_node(node)
        self._balance_tree(node)

#---------------------------------------------------------

    def _findmin(self):
        n = self.root
        if n is Nil:
            return None
        while not n.left is Nil:
            n = n.left
        return n

    def removemin(self, n):
        def case1(n):
            if not n.parent:
                return
            case2(n)

        def case2(n):
            p = n.parent
            s = n.get_sibling()
            if not s.black:
                if n is p.left:
                    self.rotate_left(p)
                else:
                    self.rotate_right(p)
                s.black = True
                p.black = False
            case3(n)

        def case3(n):
            p = n.parent
            s = n.get_sibling()
            if p.black and s.black and s.left.black and s.right.black:
                s.black = False
                case1(p)
                return
            case4(n)

        def case4(n):
            p = n.parent
            s = n.get_sibling()
            if not p.black and s.black and s.left.black and s.right.black:
                p.black = True
                s.black = False
                return
            case5(n)
            
        def case5(n):
            p = n.parent
            s = n.get_sibling()
            if p.right is s and s.right.black and not s.left.black:
                s.black = False
                s.left.black = True
                self.rotate_right(s)
            elif p.left is s and s.left.black and not s.right.black:
                s.black = False
                s.right.black = True
                self.rotate_left(s)
            case6(n)

        def case6(n):
            p = n.parent
            s = n.get_sibling()
            s.black = p.black
            p.black = True

            if p.right is s:
                s.right.black = True
                self.rotate_left(p)
            else:
                s.left.black = True
                self.rotate_right(p)

        if not n:
            return
        child  = n.right if n.left is Nil else n.left
        parent = n.parent 
        n.parent = None
        if not parent:
            child.parent = None
            child.black = True
            self.root = child
            return
        if parent.left is n:
            parent.left = child
            #if not child is Nil:
            child.parent = parent
        else:
            parent.right = child
            #if not child is Nil:
            child.parent = parent
        if not n.black:
            return
        if not child.black:
            child.black = True
            return
        case1(child)
        

if __name__ == "__main__":
    tree = RBTree()
    for i in xrange(1000):
        value = random.randint(0, 10000)
        tree.add(Node(value))

    def print_tree(tree, name):
        from graphviz import Digraph
        dot = Digraph()
        q = queue.Queue()
        q.put(tree.root)
        while not q.empty():
            x = q.get()
            style = None if x.black else [("fillcolor","red"), ("style", "filled")]
            dot.node(str(id(x)), "%s"%(x.value), style)
            if x.left:
                dot.edge(str(id(x)), str(id(x.left)))
                q.put(x.left)
            if x.right:
                dot.edge(str(id(x)), str(id(x.right)))
                q.put(x.right)
        dot.render(name, view=False)

    #for i in xrange(16):
    node = tree._findmin()
    tree.removemin(node)
    print_tree(tree, str(i)) 

