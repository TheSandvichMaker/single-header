package mud

parse :: proc "contextless" (parser: ^Parser, nodes: []Node, source: string) -> Parse_Result {
	return parse_from_string(parser, raw_data(nodes), len(nodes), source)
}

Iterator :: struct {
	current: ^Node,
}

each_node :: proc "contextless" (first: ^Node) -> Iterator {
	return Iterator{current = first}
}

each_child :: proc "contextless" (parent: ^Node) -> Iterator {
	return Iterator{current = parent.first_child}
}

each_tag :: proc "contextless" (parent: ^Node) -> Iterator {
	return Iterator{current = parent.first_tag}
}

next :: proc "contextless" (it: ^Iterator) -> (node: ^Node, ok: bool) {
	if it.current == nil || is_nil(it.current) {
		return nil, false
	}
	node = it.current
	it.current = node.next
	return node, true
}
