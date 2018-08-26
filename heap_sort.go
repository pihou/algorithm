package main

import (
	"fmt"
	"math/rand"
)

type Heap struct {
	data []int
}

func (h *Heap) Push(e int) {
	// i: index p: parent

	i, p := len(h.data), 0
	h.data = append(h.data, e)
	data := h.data
	for i > 0 {
		p = (i - 1) / 2
		if data[p] <= data[i] {
			break
		}
		data[p], data[i] = data[i], data[p]
		i = p
	}
}

func (p *Heap) Pop() (t int) {
	// s: size, d: data, i: index
	// l: left r: right
	s := len(p.data)
	if s == 0 {
		return
	}
	p.data[0], t = p.data[s-1], p.data[0]
	p.data = p.data[:s-1]
	d := p.data
	i := 0
	for i < s-1 {
		l := 2*i + 1
		r := 2*i + 2
		if l >= s-1 {
			// not have child
			break
		}
		if l < s-1 && r >= s-1 {
			// only left child
			if d[i] > d[l] {
				d[i], d[l] = d[l], d[i]
			}
			break
		}
		// have two child, n: next
		n := 0
		if d[l] <= d[r] {
			n = l
		} else {
			n = r
		}
		if d[i] <= d[n] {
			break
		} else {
			d[i], d[n] = d[n], d[i]
			i = n
		}
	}
	return
}

func main() {
	size := 100
	heap := Heap{}
	for i := 0; i < 1; i++ {
		for j := 0; j < size; j++ {
			heap.Push(rand.Intn(100))
		}
		for j := 0; j < size; j++ {
			fmt.Printf("%d ", heap.Pop())
		}
		fmt.Printf("\n")
	}
}
