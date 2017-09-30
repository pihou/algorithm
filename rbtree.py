#!-*- coding=utf8 -*-
from graphviz import Digraph

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
        n = node
        while n:
            if not n.parent:
                n.black = True
                break
            if n.parent.black:
                break
            g = n.get_grandparent()
            p = n.parent
            u = n.get_uncle()
            if not u.black:
                u.black = True
                p.black = True
                g.black = False
                n = g
                continue
            if g.left is p and p.right is n:
                self.rotate_left(p)
                n = p
            elif g.right is p and p.left is n:
                self.rotate_right(p)
                n = p
            g = n.get_grandparent()
            p = n.parent
            if g.left is p:
                self.rotate_right(g)
            else:
                self.rotate_left(g)
            p.black = True
            g.black = False
            break

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
        '''
            c: child
            n: now
            p: parent
            g: grandparent
            s: sibling
            u: uncle
        '''
        if not n:
            return
        c = n.right if n.left is Nil else n.left
        p = n.parent 
        n.parent = None
        c.parent = p 
        if not p:
            c.black = True
            self.root = c
            return
        if p.left is n:
            p.left = c
        else:
            p.right = c
        if not n.black:
            return
        if not c.black:
            c.black = True
            return
        n = c
        while n:
            if not n.parent:
                break
            p = n.parent
            s = n.get_sibling()
            if not s.black:
                if n is p.left:
                    self.rotate_left(p)
                else:
                    self.rotate_right(p)
                s.black = True
                p.black = False

            p = n.parent
            s = n.get_sibling()
            if p.black and s.black and s.left.black and s.right.black:
                s.black = False
                n = p
                continue

            if not p.black and s.black and s.left.black and s.right.black:
                p.black = True
                s.black = False
                break

            if p.right is s and s.right.black and not s.left.black:
                s.black = False
                s.left.black = True
                self.rotate_right(s)
            elif p.left is s and s.left.black and not s.right.black:
                s.black = False
                s.right.black = True
                self.rotate_left(s)

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
            break


if __name__ == "__main__":
    def print_tree(tree, dot, index):
        def get_sign(index, node):
            return str(index) + str(id(node))
        q = queue.Queue()
        q.put(tree.root)
        while not q.empty():
            node = q.get()
            sign = get_sign(index, node)
            style = None if node.black else [("fillcolor","red"), ("style", "filled")]
            dot.node(sign, "%s"%(node.value), style)
            if node.left:
                dot.edge(sign, get_sign(index, node.left))
                q.put(node.left)
            if node.right:
                dot.edge(sign, get_sign(index, node.right))
                q.put(node.right)
        return dot

    dots = Digraph()
    tree = RBTree()
    num = 24
    for i in xrange(num):
        value = random.randint(0, 10000)
        tree.add(Node(value))

    for i in xrange(num):
        node = tree._findmin()
        tree.removemin(node)
        dot = Digraph()
        print_tree(tree, dot, str(i)) 
        dots.subgraph(graph=dot)
    dots.render(view=True)
