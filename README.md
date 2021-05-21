# libparcival

Parcival is a templating engine for C.

The template files are compiled into C files that can then be compiled with the rest of the application. To access the template code the `template.h` file is used (see "Using Templates").

## Building

`make all` will build the executeable for translating the templates and the static library that includes the template registry.

## Dependencies & Compatibility

Besides a C compiler `lex` and `yacc` are also required. The `Makefile` uses `flex` and `bison` respecively but I think I only used non-specific features.

As for the C compiler support for GNU function attributes is needed. So `gcc` or `clang` should work, maybe others as well.

# Usage

## Template Format

Template files have the following syntax:

```
[meta section]
%%
[main section]
```

The meta section can contain the parameter list for the template as well as static statement blocks for optional includes, definitions, ...

The main section contains the content of the template. Here statement blocks and output blocks are allowed.

### Parameter Block

The parameter block has the following syntax:

```
{$ [parameter list ] $}
```

The parameter list is comma-seperated list of the parameters of the template including their type.

Example:
```
{$ char* title, user_t* users, size_t numberOfUsers $}
```

Note: All types that C does not provide by default have to be declared in a statement block in the meta section or in a header file included by a statement block in the meta section. In the example above this would apply to the type `user_t`.

### Statement Block

Statement blocks have the following syntax:

```
{% [C statements] %}
```

In the meta section these blocks can be used to define datatypes - even functions - and include any needed header files. Anything that got declared this way can be used as a parameter type in a parameter block.

Examples:
```
{% #include "someheader.h" %}
{%
struct s {
	int i;
};
%}
```

In the main section these blocks will open a template block. This template blocks has to be closed with `{% end %}`. Statement blocks in the main section are meant to be used for loops and conditions.

Example:
```
{% for(int i = 0; i < 10; i++) %}
This string will be printed 10 times.
{% end %}
```

Inside statement blocks in the main section the parameters as well as all local variables of parent statement blocks can be accessed.

### Output Block

Output blocks have the following syntax:

```
{{ [format string] {, format parameters} }}
```

These blocks are effectively `printf()`-calls. In the rendered templated they will be replaced by the corresponding output.

Example:
```
{{ "foo: %s - %d", "bar", 69 }}
```

### Complete Example

```
{% #include "entities.h" %}
{$ char* title, struct user* users, size_t userno $}
%%
<!DOCTYPE html>
	<head>
		<title>{{ "%s", title }}</title>
	</head>
	<body>
		<ul>
			{% for (size_t i = 0; i < userno; i++) %}
				{% if (!users[i].hidden) %}
					<li>{{ "%s", users[i].name }}</li>
				{% end %}
			{% end %}
		</ul>
	</body>
</html>
```

## Translating Template

To translate a template call the `parvical` binary using the template file as it's argument. The generated C code will be outputed to `stdout`.

The filename (and path) of the template file will set as the templates name. I might add a flag for setting custom template names later on.

Example:
```
./parvical templates/index.html.templ > gen/templates/index.tab.c
```

This will translate the template in `templates/index.html.templ` using the name "templates/index.html.templ" and write the generated code to `gen/templates/index.tab.c`.

## Compiling & Linking

Just include all generated files when compiling and linking your application.

The file `templates.h` needs to be accessable in the include-path. So make sure to add the `-Isrc/` flag (change the path to the correct location).

## Using Templates

Include the `templates.h` file into your program.
```
#include <templates.h>
```

Use the function `findTemplate()` to find the template for a given name. This function will always return a valid `template_t` pointer. In case no template with the given name is found a placeholder template is returned.

The result is a function pointer that takes a `FILE*` argument (the template will be rendered to that stream) followed by the arguments that are specified in the template file (in the same order).

Following example shows how this could look like (using the previous discussed "Complete Example" withe name specified in "Translating Template").
```
findTemplate("templates/index.html.templ")(stdout, "Page Title", userArray, userArrayLength);
```

# Contributing

Issue Reports and Pull Requests are welcome.
