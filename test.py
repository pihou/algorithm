#from graphviz import Digraph
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
    t = RBTree()
    gc.is_tracked(t)
	
    data = []
    for i in xrange(10000):
        num = random.randint(0, 10000)
        t[num] = str(num)
        data.append(num)
    #dots = Digraph()
    #for i in data:
    #    del t[i]
    #print_tree(dots, t.debug(), i)
    #dots.render(view=True)

if __name__ == "__main__":
    main()
    #time.sleep(100)

