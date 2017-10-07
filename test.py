from graphviz import Digraph
from rbtree import RBTree

import random
import time
import gc
import sys


def print_tree(dot, debug, i=0):
    i = str(i)
    for info in debug:
        if not info["black"]:
            _attributes = [("fillcolor", "red"), ("style", "filled")]
        else:
            _attributes = None
        dot.node(i+str(info["id"]), str(info["key"]), _attributes=_attributes)
        if info["left"]:
            dot.edge(i+str(info["id"]), i+str(info["left"]))
        if info["right"]:
            dot.edge(i+str(info["id"]), i+str(info["right"]))

def main():
    dots = Digraph(name="dots")
    t = RBTree()
    data = []
    #start = time.time()
    print(">>>>>>>> FanPrint[0].main", 111)
    for i in xrange(16):
        num = random.randint(0, 10000)
        t[num] = str(num)
        data.append(num)
    dot = Digraph()
    dots.subgraph(dot)
    print_tree(dot, t.debug())
    #end = time.time()
    #print "total time:%s" % (end-start)
    for i, d in enumerate(data):
        del t[d]
        dot = Digraph()
        print_tree(dot, t.debug(), i)
        dots.subgraph(dot)
    dots.render(view=True)

if __name__ == "__main__":
    main()

