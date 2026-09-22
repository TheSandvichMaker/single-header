# MUD

MUD is a small data description format designed to be hand-written. It provides
a JSON-like data model, with the addition of `@tag` annotations, inspired by Metadesk.

MUD is distributed as a single header library. Inside one source file, you
need to include the header while having `MUD_IMPL` defined.

```c
#define MUD_IMPL
#include "mud.h"
```

MUD does zero memory allocation, the only parsing API is `mud_parse_from_string` which
takes an array of nodes to use as its backing storage. All `MUD_STRING`s point back
into the original source file.

Basic usage looks like:

```c
ptrdiff_t file_size;
char const *file = read_entire_file("my_file.mud", &file_size);

#define MAX_NODE_COUNT (1 << 16)
static Mud_Node nodes[MAX_NODE_COUNT];

Mud_Parser parser;
Mud_Parse_Result result = mud_parse_from_string(&parser, nodes, MAX_NODE_COUNT, mud_string(file, file_size));

if (result.error != Mud_Error_none)
{
	// TODO: Error reporting
	exit(-1);
}

Mud_Node *root = result.root;
for (Mud_EachChild(node, root))
{
	do_stuff_with_node(node);
}
```

There are three kinds of elements that can appear at the top level of a MUD file:

```
// key-value
my_value = value

// array
my_array [
	1
	2
]

// object
my_object {
	a = 1
	b = 2
}
```

Members of arrays and objects can be delineated by commas or newlines

```
my_array  [ 1, 2 ]
my_object { a = 1, b = 2 }
```

Elements can be tagged to provide an additional dimension of metadata

```
@my_tag my_array [ 1, 2 ]
```

Tags can receive arguments, which must be a list of key-value pairs

```
@my_tag(arg1=1, arg2=2) my_array [ 1, 2 ]
```

Leading and trailing comments are attached to elements

```
// Leading comment
my_object {
	member1 = value     // Trailing comment 1
	member2 [ 1, 2, 3 ] // Trailing comment 2
	// Closing comment
} // Trailing comment 3
```

Here, the leading comment and trailing comment 3 are attached to `my_object`,
while trailing comments 1 and 2 are attached to `member1` and `member2`. A
trailing comment must sit on the same line as the element it follows;
comments on their own line lead the element below them. The closing comment
has no element below it, so it is attached to `my_object` as well, separately
from its trailing comment.

A comment that is alone on its line keeps that line's leading whitespace, so
that a multi-line comment reads as one block the consumer can reindent as a
unit. A trailing comment starts at its first slash.

There are more defines you can use to configure the library, see the header of
`mud.h` for details.

MUD uses nil nodes rather than null pointers to signify a node's non-existence.
This means you need to use `mud_is_nil()`:

```c
for (Mud_Node *node = first; !mud_is_nil(node); node = node->next)

if (!mud_is_nil(node->first_tag))
{
	printf("My node has a tag!\n");
}
```

This can be disabled by defining `MUD_NIL_IS_NULL` prior to including `mud.h` in your
source file (so wherever you are including it with `MUD_IMPL` defined)

MUD is licensed under the MIT license. See the bottom of `mud.h` for more.
