#!-*- coding=utf8 -*-
from graphviz import Digraph

import queue
import random

class Node(object):
    def __init__(self, value):
        self.left = None 
        self.right = None
        self.factor = 0
        self.parent = None
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
        return "parent: %s self:%s factor:%s"%(p, self.value, self.factor)

    def __repr__(self):
        return str(self)


class AVLTree(object):
    def __init__(self):
        self.root = None

    def rotate_left(self, n):
        p = n.get_parent()
        r = n.right
        assert(not r is None)

        n.right = r.left
        if r.left:
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
        assert(not l is None)

        n.left = l.right
        if l.right:
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
        if not n:
            self.root = node
            return
        while True:
            if node.value <= n.value:
                if n.left is None:
                    n.left = node
                    node.parent = n 
                    break
                n = n.left
            else:
                if n.right is None:
                    n.right = node
                    node.parent = n 
                    break
                n = n.right

    def _balance_tree(self, node):
        n = node
        if n.get_sibling():
            n.parent.factor = 0
            return
        while n.parent:
            p = n.parent
            if p.left is n:
                p.factor -= 1
            else:
                p.factor += 1
            if not p.factor:
                break
            if abs(p.factor) <= 1:
                n = p
                continue
            if p.factor < -1 and n.factor < 0: 
                n.factor = 0
                p.factor = 0
                self.rotate_right(p)
                break
            if p.factor > 1 and n.factor > 0:
                n.factor = 0
                p.factor = 0
                self.rotate_left(p)
                break
            if p.factor < -1 and n.factor > 0:
                r = n.right
                factor = r.factor
                n.factor = 0 if factor <= 0 else -1
                self.rotate_left(n)
                r.factor = 0
                p.factor = 0 if factor >= 0 else 1
                self.rotate_right(p)
                break
            if p.factor > 1 and n.factor < 0:
                l = n.left
                factor = l.factor
                n.factor = 0 if factor >= 0 else 1
                self.rotate_right(n)
                l.factor = 0
                p.factor = 0 if factor <= 0 else -1
                self.rotate_left(p)
                break
            raise Erro

    def add(self, node):
        self._push_node(node)
        self._balance_tree(node)


if __name__ == "__main__":
    def print_tree(tree, dot, index):
        def get_sign(index, node):
            return str(index) + str(id(node))
        q = queue.Queue()
        q.put(tree.root)
        while not q.empty():
            node = q.get()
            sign = get_sign(index, node)
            dot.node(sign, "%s(%s)"%(node.value, node.factor))
            if node.left:
                dot.edge(sign, get_sign(index, node.left))
                q.put(node.left)
            if node.right:
                dot.edge(sign, get_sign(index, node.right))
                q.put(node.right)
        return dot

    dots = Digraph()
    tree = AVLTree()
    for i, value in enumerate(xrange(24)):
        value = random.randint(0, 10000)
        tree.add(Node(value))
        dot = Digraph()
        print_tree(tree, dot, i)
        dots.subgraph(graph=dot)
    dots.render(view=True)


