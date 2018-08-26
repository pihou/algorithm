package main

import (
	"fmt"
	"math/rand"
	"sort"
)

func quick_sort(a []int) {
	if a == nil || len(a) <= 1 {
		return
	}
	// h <head> t <tail> l <last>
	l := len(a) - 1
	pivot := a[l]
	h, t := 0, l
	for h < t {
		for h < t && a[h] <= pivot {
			h++
		}
		for h < t && a[t] >= pivot {
			t--
		}
		a[h], a[t] = a[t], a[h]
	}
	a[h], a[l] = a[l], a[h]
	quick_sort(a[:h])
	quick_sort(a[h+1:])
}

func main() {
	t := [][]int{
		{1, 2, 3},
		{1, 1, 1},
		{3, 2, 1},
		{1},
		{1, 100},
	}
	for _, d := range t {
		quick_sort(d)
		fmt.Println(d)
	}
	for i := 0; i < 12; i++ {
		d := []int{}
		for j := 0; j < 120; j++ {
			d = append(d, rand.Intn(100))
		}
		t := make([]int, len(d))
		copy(t, d)
		quick_sort(d)
		sort.Ints(t)
		for in, data := range t {
			if data != d[in] {
				fmt.Println("Not Right:")
				fmt.Println(d)
				fmt.Println(t)
				break
			}
		}
	}
}
